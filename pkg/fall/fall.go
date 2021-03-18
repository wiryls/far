package fall

import (
	"sync"

	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/far/pkg/flow"
	"github.com/wiryls/far/pkg/free"
)

// New Fall.
func New() *Fall {
	return &Fall{
		farr: far.New(),
		tree: free.NewFileTree(),
		flow: flow.New(),
		lock: sync.Mutex{},
		tail: 0,
		list: []Item{},
	}
}

// Item is file path with status.
type Item struct {
	Stat uint32
	Diff far.Diffs
	Node free.FileNode
}

// Fall is actually a file tree with some operations.
type Fall struct {
	// data operator
	farr far.Far
	tree free.FileNode

	// actuator
	flow *flow.Flow

	// previews
	lock sync.Mutex
	tail int
	list []Item
}

// Find set the pattern and try to find.
func (f *Fall) Find(pattern string) (err error) {
	err = f.farr.SetPattern(pattern)

	// TODO: filter list in task goroutine.

	return
}

// Replace set the template and try to replace.
func (f *Fall) Replace(template string) (err error) {
	err = f.farr.SetTemplate(template)

	// TODO: run differ in task goroutine.

	return
}

func (f *Fall) Source(paths []string) {
	// TODO: add them in task goroutine
}

// Read access the result list.
func (f *Fall) Read(op func([]Item)) {
	defer f.lock.Unlock()
	/*_*/ f.lock.Lock()
	op(f.list[:f.tail])
}
