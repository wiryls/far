package fall

import (
	"sync"
	"sync/atomic"
)

type sequencer struct {
	Latest int
	Output func([]Output)
	buffer map[int][]Output
	keeper sync.Mutex
}

func (s *sequencer) Collect(index int, items []Output) {
	defer s.keeper.Unlock()
	/*_*/ s.keeper.Lock()

	if s.Latest == index {
		for {
			s.Latest++
			is, ok := s.buffer[s.Latest]
			if ok {
				items = append(items, is...)
				delete(s.buffer, s.Latest)
			} else {
				break
			}
		}
		s.Output(items)

	} else if len(items) > 0 {
		if s.buffer == nil {
			s.buffer = make(map[int][]Output)
		}
		s.buffer[index] = items

	}
}

type differ struct {
	Number int
	Source []Input
	Splite func(int) int
	Action func(Input) Output
	Output func(int, []Output)
	Runner func(func(*uint32))
	Runnin uint32
}

func (t *differ) Run(sync *uint32) {
	if (sync == nil || atomic.LoadUint32(sync) != t.Runnin) ||
		t.Source == nil ||
		t.Splite == nil ||
		t.Action == nil ||
		t.Output == nil ||
		t.Runner == nil {
		return
	}

	todo := t.Source
	if size := t.Splite(len(t.Source)); 0 <= size && size < len(t.Source) {
		another := *t
		another.Source, todo = t.Source[size:], t.Source[:size]
		another.Number++
		another.Runner(another.Run)
	}

	result := make([]Output, 0, len(todo))
	for _, input := range todo {
		if output := t.Action(input); output.Differ != nil {
			result = append(result, output)
		}
	}

	if (sync == nil || atomic.LoadUint32(sync) == t.Runnin) && len(result) > 0 {
		t.Output(t.Number, result)
	}
}

func limited(limit int) func(int) int {
	return func(size int) int {
		if size > limit {
			return limit
		}
		return size
	}
}
