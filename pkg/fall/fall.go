package fall

import (
	"path/filepath"

	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/pkg/flow"
)

func New(callback Callback) *Fall {
	if callback == nil {
		callback = func(i int, os []Output) {}
	}

	return &Fall{
		call: callback,
		farr: &far.Farsafe{Inner: &far.Faregex{}},
		flow: &flow.Feed{Mark: 1},
	}
}

type Fall struct {
	call Callback
	farr far.Far
	flow *flow.Feed
}

// SetPattern change the far pattern.
func (f *Fall) SetPattern(text string) error {
	return f.farr.SetPattern(text)
}

// SetTemplate change the far template.
func (f *Fall) SetTemplate(text string) error {
	return f.farr.SetTemplate(text)
}

// Flow will append some inputs to current task.
func (f *Fall) Flow(source []Input) {
	f.flow.Push((&differ{
		Number: 0,
		Source: source,
		Splite: limited(1024),
		Action: func(src Input) (dst Output) {
			dst.Source = src
			dst.Target = filepath.Base(src)
			dst.Differ = f.farr.See(dst.Target)
			if !f.farr.Empty() && dst.Differ.IsSame() {
				dst.Differ = nil
			}
			return
		},
		Output: (&sequencer{Latest: 0, Output: f.call}).Collect,
		Runner: f.flow.Push,
		Runnin: 1,
	}).Run)
}

// Stop all current tasks. It will block until the last task stops.
func (f *Fall) Stop() {
	f.flow.Sync(0)
	f.flow.Wait()
	f.flow.Sync(1)
}

// Stop all current tasks. But do something extra during stopping.
func (f *Fall) StopWith(action func()) {
	f.flow.Sync(0)
	f.flow.Wait()

	if action != nil {
		action()
	}

	f.flow.Wait()
	f.flow.Sync(1)
}
