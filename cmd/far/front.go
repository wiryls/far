package main

import (
	"io/fs"
	"log"
	"path/filepath"

	"github.com/wiryls/far/pkg/fall"
)

func NewFront() (m *Front, err error) {
	m = &Front{}
	m.fall = fall.New(m)
	m.sets.ImportRecursively = true
	return
}

type Front struct {
	// view
	// main fyne.App
	// view fyne.Window

	// Settings
	sets Settings

	// model
	fall *fall.Fall

	// ui
	// text texts
}

func (a *Front) Run() error {
	r := Resource(nil)
	return BuildView(r.Set("zh"))
}

func (a *Front) Close() error {
	return nil
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks GUI

func (a *Front) OnTextPatternChanged() {

}

func (a *Front) OnTextTemplateChanged() {
}

func (a *Front) OnRename() {

}

func (a *Front) OnImport(list []string) {

	var input []string
	if !a.sets.ImportRecursively {
		input = list
	} else {
		for _, file := range list {
			err := filepath.WalkDir(file, func(path string, d fs.DirEntry, err error) error {
				input = append(input, path)
				return err
			})
			if err != nil {
				log.Println(err)
			}
		}
	}

	a.fall.Input(input)
}

func (a *Front) OnDelete() {
	a.fall.Delete( /*TODO*/ []int{})
}

func (a *Front) OnClear() {
	a.fall.Reset()
}

func (a *Front) OnExit() {
	a.fall.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Table View

func (a *Front) ResetRows() {
	a.fall.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks settings

func (a *Front) OnSettingImportRecursively() {
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Fall

func (a *Front) OnItemUpdate(index int) {

}

func (a *Front) OnItemsUpdate(from, to int) {

}

func (a *Front) OnItemsInsert(from, to int) {

}

func (a *Front) OnItemsDelete(from, to int) {

}

func (a *Front) OnItemsReset() {

}
