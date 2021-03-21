package flow

import (
	"runtime"
	"sync"
	"sync/atomic"
	"time"
)

// New create a flow to process some tasks.
func New() *Flow {
	return &Flow{
		count: 0,
		limit: int32(runtime.NumCPU()),
		mutex: sync.RWMutex{},
		tasks: []func(){},
	}
}

// Flow process something.
type Flow struct {
	count int32
	limit int32
	mutex sync.RWMutex
	tasks []func()
}

// Append a task to the executor.
func (f *Flow) Append(task func()) {
	if f != nil && f.tasks != nil && task != nil {
		{
			f.mutex.Lock()
			f.tasks = append(f.tasks, task)
			f.mutex.Unlock()
		}
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
loop:
	for {
		f.mutex.RLock()
		gotcha := len(f.tasks) > 0
		if gotcha {
			task, f.tasks = f.tasks[0], f.tasks[1:]
		}
		f.mutex.RUnlock()

		if gotcha {
			task()
		} else {
			break loop
		}
	}
}
