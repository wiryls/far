package filter

import "sync"

type Filter sync.Map

func (f *Filter) Has(value string) (has bool) {
	if f != nil {
		_, has = (*sync.Map)(f).Load(value)
	}
	return has
}

func (f *Filter) Put(value string) (has bool) {
	if f != nil {
		_, has = (*sync.Map)(f).LoadOrStore(value, struct{}{})
	}
	return has
}

func (f *Filter) Pop(value string) {
	if f != nil {
		(*sync.Map)(f).Delete(value)
	}
}
