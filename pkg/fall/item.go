package fall

import (
	"os"
	"path/filepath"

	"github.com/wiryls/far/pkg/far"
)

// Item is file path with status.
type Item struct {
	Stat uint32
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
		item = &Item{
			Stat: 1,
			Base: base,
			Path: path,
		}
	}
	return
}
