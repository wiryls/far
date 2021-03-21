package fall

import (
	"os"
	"path/filepath"
	"sync"
)

type input struct {
	Limits int
	Source []string
	Output func([]Item)
	Runner func(task func())
	Cancel chan struct{}
	Groups *sync.WaitGroup
}

func (i *input) DivideAndConquer() {

	if i.Groups != nil {
		defer i.Groups.Done()
		/*_*/ i.Groups.Add(1)
	}

	select {
	default:
	case <-i.Cancel:
		return
	}

	if 0 < i.Limits && i.Limits < len(i.Source) {
		f := &input{
			Limits: i.Limits,
			Source: i.Source[i.Limits:],
			Output: i.Output,
			Runner: i.Runner,
			Cancel: i.Cancel,
			Groups: i.Groups,
		}
		i.Source = i.Source[:i.Limits]
		i.Runner(f.DivideAndConquer)
	}

	list := make([]Item, 0, len(i.Source))
	for _, path := range i.Source {
		var err error
		if err == nil {
			path, err = filepath.Abs(path)
		}
		if err == nil {
			_, err = os.Stat(path)
		}
		if err == nil {
			list = append(list, Item{
				Stat: 0,
				Base: len(path) - len(filepath.Base(path)),
				Path: path,
				Diff: nil,
			})
		}
	}

	select {
	default:
	case <-i.Cancel:
		return
	}

	i.Output(list)
}
