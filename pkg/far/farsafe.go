package far

import (
	"sync"

	"github.com/wiryls/pkg/errors/cerrors"
)

type Farsafe struct {
	Inner Far
	mutex sync.RWMutex
}

func (f *Farsafe) See(text string) Diffs {
	defer f.mutex.RUnlock()
	/*_*/ f.mutex.RLock()
	if f.Inner == nil {
		return nil
	}
	return f.Inner.See(text)
}

func (f *Farsafe) Empty() bool {
	defer f.mutex.RUnlock()
	/*_*/ f.mutex.RLock()
	if f.Inner == nil {
		return true
	}
	return f.Inner.Empty()
}

func (f *Farsafe) Pattern() string {
	defer f.mutex.RUnlock()
	/*_*/ f.mutex.RLock()
	if f.Inner == nil {
		return ""
	}
	return f.Inner.Pattern()
}

func (f *Farsafe) Template() string {
	defer f.mutex.RUnlock()
	/*_*/ f.mutex.RLock()
	if f.Inner == nil {
		return ""
	}
	return f.Inner.Template()
}

func (f *Farsafe) SetPattern(text string) error {
	defer f.mutex.Unlock()
	/*_*/ f.mutex.Lock()
	if f.Inner == nil {
		return cerrors.NilArgument("f.Inner")
	}
	return f.Inner.SetPattern(text)
}

func (f *Farsafe) SetTemplate(text string) error {
	defer f.mutex.Unlock()
	/*_*/ f.mutex.Lock()
	if f.Inner == nil {
		return cerrors.NilArgument("f.Inner")
	}
	return f.Inner.SetTemplate(text)
}
