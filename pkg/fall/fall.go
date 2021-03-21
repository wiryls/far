package fall

import (
	"sort"
	"sync"

	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/far/pkg/flow"
)

// New Fall.
func New() *Fall {
	return &Fall{
		farr: far.New(),
		flow: flow.New(),
		head: 0,
		tail: 0,
		done: make(chan int, 1),
		todo: make(chan int, 1),
		lock: sync.RWMutex{},
		list: []Item{},
	}
}

// Item is file path with status.
type Item struct {
	Stat uint32
	Base int
	Path string
	Diff far.Diffs
}

// ItemsListener is used to notify items have been ready.
type ItemsListener = func(done, todo int)

// Fall is actually a file tree with some operations.
type Fall struct {
	// model
	farr far.Far

	// task
	flow *flow.Flow
	noti ItemsListener

	// item
	head int
	tail int
	done chan int
	todo chan int
	lock sync.RWMutex
	list []Item
}

/////////////////////////////////////////////////////////////////////////////
// Getters

// Read access the result list.
// Please do not do anything may block.
func (f *Fall) Read(r func([]Item)) {
	defer f.lock.RUnlock()
	/*_*/ f.lock.RLock()

	r(f.list[f.head:f.tail])
}

/////////////////////////////////////////////////////////////////////////////
// Setters

func (f *Fall) SetItemsStatusListener(l ItemsListener) {
	defer f.lock.Unlock()
	/*_*/ f.lock.Lock()
	f.noti = l
}

// sort by what
const (
	ByStat = iota
	ByPath
	ByName
)

// Sort the list[head:tail]
func (f *Fall) Sort(isAscending bool, by int) {
	defer f.lock.RUnlock()
	/*_*/ f.lock.RLock()

	list := f.list[f.head:f.tail]
	comp := (func(i int, j int) bool)(nil)
	switch by {
	default:
		fallthrough
	case ByStat:
		comp = func(i int, j int) bool {
			return isAscending !=
				(list[i].Stat < list[j].Stat)
		}
	case ByPath:
		comp = func(i int, j int) bool {
			return isAscending !=
				(list[i].Path < list[j].Path)
		}
	case ByName:
		comp = func(i int, j int) bool {
			return isAscending !=
				(list[i].Path[list[i].Base:] < list[j].Path[list[j].Base:])
		}
	}
	sort.SliceStable(list, comp)
}

/////////////////////////////////////////////////////////////////////////////
// Tasks

// Input appends more paths as tasks.
func (f *Fall) Input(paths []string) {
	f.flow.Append((&input{
		Limits: 100,
		Source: paths,
		Output: func(list []Item) {
			f.lock.Lock()
			f.list = append(f.list, list...)
			last := len(f.list)
			f.lock.Unlock()

		loop:
			for {
				select {
				case f.todo <- last:
					break loop
				case cur := <-f.todo:
					if cur > last {
						last = cur
					}
				}
			}
		},
		Runner: f.flow.Append,
		Cancel: make(chan struct{}),
		Groups: &sync.WaitGroup{},
	}).DivideAndConquer)
}

// Far performs FaR on the list.
func (f *Fall) Far(pattern, template string) (err error) {
	err = f.farr.SetTemplate(template)

	if err == nil {
		defer f.lock.RUnlock()
		/*_*/ f.lock.RLock()

		// done := <-f.done

		// select {}

		// todo := <-f.todo
		// done := <-f.done
		// f.done <- done

		// for i := done; i < todo; i++ {
		// 	x := &f.list[i]
		// 	x.Diff = f.farr.See(x.Path[x.Base:])
		// 	atomic.AddUint32(&x.Stat, 1)
		// }

		// f.todo <- done
	}
	return
}
