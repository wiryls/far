package fall

import "sync"

type sequencer struct {
	Latest int
	Output func(items []*Item)
	buffer map[int][]*Item
	keeper sync.Mutex
}

func (s *sequencer) Collect(index int, items []*Item) {
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

	} else {
		if s.buffer == nil {
			s.buffer = make(map[int][]*Item)
		}
		s.buffer[index] = items

	}
}
