package flow

import (
	"runtime"
	"sync/atomic"
	"time"
)

// New create a flow to process some tasks.
func New() *Flow {
	stream := make(chan []func(), 1)
	stream <- []func(){}
	return &Flow{
		stream: stream,
		count:  0,
		limit:  int32(runtime.NumCPU()),
	}
}

// Flow process something.
type Flow struct {
	stream chan []func()
	count  int32
	limit  int32
}

// Append a task to the executor.
func (f *Flow) Append(task func()) {
	if f != nil && f.stream != nil {
		f.stream <- append(<-f.stream, task)
		if atomic.AddInt32(&f.count, 1) > f.limit {
			atomic.AddInt32(&f.count, -1)
		} else {
			go f.low()
		}
	}
}

// Wait until all task done.
func (f *Flow) Wait() {
	for atomic.LoadInt32(&f.count) != 0 {
		time.Sleep(time.Millisecond)
	}
}

func (f *Flow) low() {
	defer atomic.AddInt32(&f.count, -1)
	var task func()
	var list []func()
loop:
	for {
		list = <-f.stream
		switch len(list) {
		case 0:
			f.stream <- nil
			break loop

		default:
			f.stream <- list[1:]

			task = list[0]
			if task != nil {
				task()
			}
		}
	}
}
