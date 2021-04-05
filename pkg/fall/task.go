package fall

import (
	"sync/atomic"
)

// TaskImportItemsFromPaths is a task to import something and output it.
type TaskImportItemsFromPaths struct {
	Source []string
	Splite func(int) int
	Action func(string) *Item
	Output func([]*Item)
	Runner func(func())
}

func (t *TaskImportItemsFromPaths) Execute() {
	switch {
	case t.Splite == nil:
		return
	case t.Action == nil:
		return
	case t.Output == nil:
		return
	case t.Runner == nil:
		return
	}

	todo := t.Source
	if size := t.Splite(len(t.Source)); 0 <= size && size < len(t.Source) {
		task := *t
		task.Source, todo = t.Source[size:], t.Source[:size]
		task.Runner(task.Execute)
	}

	result := make([]*Item, 0, len(t.Source))
	for _, input := range todo {
		if output := t.Action(input); output != nil {
			result = append(result, output)
		}
	}

	t.Output(result)
}

type TaskDifferItems struct {
	Source []*Item
	Splite func(int) int
	Action func(*Item) *Item
	Number int
	Output func(int, []*Item)
	Runner func(func(*uint32))
	Runnin uint32
}

func (t *TaskDifferItems) Execute(sync *uint32) {
	switch {
	case t.Splite == nil:
		return
	case t.Action == nil:
		return
	case t.Output == nil:
		return
	case t.Runner == nil:
		return
	case sync == nil || atomic.LoadUint32(sync) != t.Runnin:
		return
	}

	todo := t.Source
	if size := t.Splite(len(t.Source)); 0 <= size && size < len(t.Source) {
		task := *t
		task.Source, todo = t.Source[size:], t.Source[:size]
		task.Number++
		task.Runner(task.Execute)
	}

	result := make([]*Item, 0, len(t.Source))
	for _, input := range todo {
		if output := t.Action(input); output != nil {
			result = append(result, output)
		}
	}

	if sync == nil || atomic.LoadUint32(sync) == t.Runnin {
		t.Output(t.Number, result)
	}
}
