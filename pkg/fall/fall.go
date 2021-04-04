package fall

import (
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
	source []Item
	output []Item
}

/////////////////////////////////////////////////////////////////////////////
// Getters & Setters

// ReadonlyAccess access the result list.
// Please do not do anything may block.
func (f *Fall) ReadonlyAccess(access func([]Item)) {
	defer f.keeper.RUnlock()
	/*_*/ f.keeper.RLock()
	access(f.output)
}

// WritableAccess access the result list.
// Please do not do anything may block.
func (f *Fall) WritableAccess(access func([]Item)) {
	defer f.keeper.Unlock()
	/*_*/ f.keeper.Lock()
	access(f.output)
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
	f.flow.Push((&TaskFromStringsToItems{
		Source: source,
		Splite: fixedSplitter(32),
		Action: func(s string) (Item, bool) {
			item, err := FromPathToItem(s)
			return item, err == nil
		},
		Output: func(list []Item) {
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

	f.keeper.Lock()
	var list []Item
	if template != f.farr.Template() {
		list = f.output
		err = f.farr.SetTemplate(template)
	}
	if pattern != f.farr.Pattern() {
		list = f.source
		err = f.farr.SetPattern(pattern)
	}
	f.keeper.Unlock()

	if err == nil {
		// stop the last FaR task.
		f.feed.Send(0)
		f.feed.Wait()

		// start a new FaR task.
		defer f.call.OnItemsReset()
		defer f.keeper.Unlock()
		/*_*/ f.keeper.Lock()
		f.feed.Wait()
		f.feed.Send(1)
		f.output = nil
		f.diffing(list)
	}

	return
}

func (f *Fall) diffing(list []Item) {
	f.feed.Push((&TaskFromItemsToItems{
		Source: list,
		Splite: fixedSplitter(32),
		Action: func(item Item) (Item, bool) {
			item.Diff = f.farr.See(item.Base)
			return item, f.farr.Empty() || !item.Diff.IsSame()
		},
		Output: func(list []Item) {
			f.keeper.Lock()
			from, delta := len(f.output), len(list)
			f.output = append(f.output, list...)
			f.keeper.Unlock()

			f.call.OnItemsInsert(from, from+delta)
		},
		Runner: f.feed.Push,
		Runnin: 1,
	}).Execute)
}

func fixedSplitter(limit int) func(int) int {
	return func(size int) int {
		if size > limit {
			return limit
		}
		return size
	}
}
