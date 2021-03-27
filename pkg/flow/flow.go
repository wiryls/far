package flow

import (
	"runtime"
	"sync"
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
		defer f.mutex.Unlock()
		/*_*/ f.mutex.Lock()

		f.tasks = append(f.tasks, task)
		if f.count < f.limit {
			f.count++
			go f.low()
		}
	}
}

// Wait until all task done.
func (f *Flow) Wait() {
	for {
		f.mutex.RLock()
		running := f.count > 0
		f.mutex.RUnlock()
		if running {
			time.Sleep(time.Millisecond)
		} else {
			break
		}
	}
}

func (f *Flow) low() {
	for {
		// fast-check if any task exists
		f.mutex.RLock()
		todo := f.etch()
		f.mutex.RUnlock()

		if todo == nil {
			// slow-check if no task exists
			f.mutex.Lock()
			todo = f.etch()
			if todo == nil {
				f.count--
			}
			f.mutex.Unlock()
		}

		if todo != nil {
			todo()
		} else {
			break
		}
	}
}

func (f *Flow) etch() (fun func()) {
	if len(f.tasks) > 0 {
		fun, f.tasks = f.tasks[0], f.tasks[1:]
	}
	return
}
