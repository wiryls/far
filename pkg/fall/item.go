package fall

import (
	"os"
	"path/filepath"
	"sync/atomic"

	"github.com/wiryls/far/pkg/far"
)

// Items status
const (
	StatIgnored = iota
	StatImport
	StatDiffer
	StatRename
)

// Item is file path with status.
type Item atomic.Value

func (i *Item) Load() (x Data) {
	v := (*atomic.Value)(i).Load()
	x, _ = v.(Data)
	return
}

func (i *Item) Store(x Data) {
	(*atomic.Value)(i).Store(x)
}

type Data struct {
	Stat int
	Base string
	Path string
	Diff far.Diffs
}

// FromPathToItem create an item from
func FromPathToItem(path string) (item *Item, err error) {
	if err == nil {
		path, err = filepath.Abs(path)
	}
	if err == nil {
		_, err = os.Stat(path)
	}
	if err == nil {
		base := filepath.Base(path)
		item = new(Item)
		item.Store(Data{
			Stat: StatImport,
			Base: base,
			Path: path,
		})
	}
	return
}
