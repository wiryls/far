package fall

import (
	"sort"
	"sync"

	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/pkg/flow"
)

// New Fall.
func New(c Callback) *Fall {
	f := &flow.Flow{}
	if c == nil {
		c = &doNothing{}
	}

	return &Fall{
		call: c,
		flow: f,
		feed: &flow.Feed{Flow: f, Sync: 1},
		farr: &far.Farsafe{Inner: &far.Faregex{}},
	}
}

// Fall is actually a file tree with some operations.
type Fall struct {
	// methods
	call Callback
	flow *flow.Flow
	feed *flow.Feed
	farr far.Far

	// items
	keeper sync.RWMutex
	source []*Item
	output []*Item
}

/////////////////////////////////////////////////////////////////////////////
// Getters & Setters

// ReadonlyAccess access the result list.
// Please do not do anything may block.
func (f *Fall) ReadonlyAccess(access func([]*Item)) {
	defer f.keeper.RUnlock()
	/*_*/ f.keeper.RLock()
	access(f.output)
}

// WritableAccess access the result list.
// Please do not do anything may block.
func (f *Fall) WritableAccess(access func([]*Item)) {
	defer f.keeper.Unlock()
	/*_*/ f.keeper.Lock()
	access(f.output)
}

func (f *Fall) Delete(some []int) {
	f.keeper.Lock()

	// standardise some
	if !sort.IntsAreSorted(some) {
		sort.Ints(some)
	}
	some = IntsUnique(some)

	// mark item stat as deleted
loop:
	for i, k := range some {
		switch {
		case k < 0:

		case k < len(f.output):
			item := f.output[k].Load()
			item.Stat = StatIgnored
			f.output[k].Store(item)

		default:
			some = some[:i]
			break loop
		}
	}

	// remove from output
	f.output = RemoveItemByIndexes(f.output, some)

	// remove from source
	f.source = RemoveItemByCondition(f.source, func(i *Item) bool {
		return i.Load().Stat == 0
	})

	f.keeper.Unlock()

	// notify
	for _, i := range IntsPairs(some) {
		f.call.OnItemsDelete(i[0], i[1])
	}
}

// Reset all
func (f *Fall) Reset() {
	f.feed.Send(0)
	f.feed.Wait()

	defer f.call.OnItemsReset()
	defer f.keeper.Unlock()
	/*_*/ f.keeper.Lock()
	f.source = nil
	f.output = nil
	f.feed.Wait()
	f.feed.Send(1)
}

/////////////////////////////////////////////////////////////////////////////
// Tasks

// Input appends more paths as tasks.
func (f *Fall) Input(source []string) {
	f.flow.Push((&TaskImportItemsFromPaths{
		Source: source,
		Splite: Limited(512),
		Action: func(s string) *Item {
			item, _ := FromPathToItem(s)
			return item
		},
		Output: func(list []*Item) {
			defer f.keeper.Unlock()
			/*_*/ f.keeper.Lock()
			f.source = append(f.source, list...)
			f.diffing(list)
		},
		Runner: f.flow.Push,
	}).Execute)
}

// Far performs FaR on the list.
func (f *Fall) Far(pattern, template string) (err error) {

	if template != f.farr.Template() {
		err = f.farr.SetTemplate(template)
	}
	if pattern != f.farr.Pattern() {
		err = f.farr.SetPattern(pattern)
	}

	if err == nil {
		// stop the last FaR task.
		f.feed.Send(0)
		f.feed.Wait()

		// start a new FaR task.
		defer f.keeper.RUnlock()
		defer f.diffing(f.source)
		defer f.keeper.RLock()
		defer f.call.OnItemsReset()

		defer f.keeper.Unlock()
		/*_*/ f.keeper.Lock()
		f.feed.Wait()
		f.feed.Send(1)
		f.output = nil
	}

	return
}

func (f *Fall) diffing(list []*Item) {
	s := sequencer{
		Latest: 0,
		Output: func(list []*Item) {
			if len(list) == 0 {
				return
			}

			f.keeper.Lock()
			from, delta := len(f.output), len(list)
			f.output = append(f.output, list...)
			f.keeper.Unlock()

			f.call.OnItemsInsert(from, from+delta)
		},
	}
	f.feed.Push((&TaskDifferItems{
		Source: list,
		Splite: Limited(1024),
		Action: func(src *Item) (dst *Item) {
			item := src.Load()
			item.Diff = f.farr.See(item.Base)
			switch {
			default:
			case item.Stat == StatIgnored:
				// ignore it
			case f.farr.Empty():
				item.Stat = StatImport
				dst = src
			case !item.Diff.IsSame():
				item.Stat = StatDiffer
				dst = src
			}
			if dst != nil {
				dst.Store(item)
			}
			return
		},
		Number: 0,
		Output: s.Collect,
		Runner: f.feed.Push,
		Runnin: 1,
	}).Execute)
}
