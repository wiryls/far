package flow

import (
	"context"
	"runtime"
	"sync/atomic"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func TestFlowAppend(t *testing.T) {
	assert := assert.New(t)

	{
		count := runtime.NumGoroutine()
		waste := func() { time.Sleep(1 * time.Microsecond) }

		f := New()
		for i := 0; i < 100; i++ {
			f.Append(waste)
		}

		delta := int(f.limit)
		if delta > 100 {
			delta = 100
		}

		assert.Equal(count+delta, runtime.NumGoroutine())

		f.Wait()
		assert.EqualValues(0, atomic.LoadInt32(&f.count))
	}

	{
		ctx, cancel := context.WithCancel(context.Background())
		waste := func() {
			select {
			case <-ctx.Done():
			case <-time.After(1 * time.Microsecond):
			}
		}

		count := runtime.NumGoroutine()

		f := New()
		for i := 0; i < 1000; i++ {
			f.Append(waste)
		}

		delta := int(f.limit)
		assert.Equal(count+delta, runtime.NumGoroutine())

		cancel()
		f.Wait()
		assert.EqualValues(0, atomic.LoadInt32(&f.count))
	}
}
