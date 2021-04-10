package fall

import (
	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/pkg/flow"
)

func New(callback Callback) *Fall {
	if callback == nil {
		callback = func(os []Output) {}
	}

	flee := &flow.Flow{}
	return &Fall{
		call: callback,
		farr: &far.Farsafe{Inner: &far.Faregex{}},
		flow: flee,
		feed: &flow.Feed{Flow: flee, Sync: 1},
	}
}

type Fall struct {
	call Callback
	farr far.Far
	flow *flow.Flow
	feed *flow.Feed
}

func (f *Fall) SetPattern(text string) error {
	return f.farr.SetPattern(text)
}

func (f *Fall) SetTemplate(text string) error {
	return f.farr.SetTemplate(text)
}

// Flow will append some inputs to current task.
func (f *Fall) Flow(source []Input) {
	f.feed.Push((&differ{
		Number: 0,
		Source: source,
		Splite: limited(1024),
		Action: func(src Input) (dst Output) {
			dst.Source = src
			dst.Differ = f.farr.See(src)
			if !f.farr.Empty() && dst.Differ.IsSame() {
				dst.Differ = nil
			}
			return
		},
		Output: (&sequencer{Latest: 0, Output: f.call}).Collect,
		Runner: f.feed.Push,
		Runnin: 1,
	}).Run)
}

// Stop all current tasks. It will block until the last task stops.
func (f *Fall) Stop() {
	f.feed.Send(0)
	f.feed.Wait()
	f.feed.Send(1)
}

// Stop all current tasks. But do something extra during stopping.
func (f *Fall) StopWith(action func()) {
	f.feed.Send(0)
	f.feed.Wait()

	if action != nil {
		action()
	}

	f.feed.Wait()
	f.feed.Send(1)
}
