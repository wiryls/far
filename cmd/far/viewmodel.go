package main

import (
	"errors"
	"io/fs"
	"log"
	"path/filepath"

	"github.com/wiryls/far/pkg/fall"
)

func NewViewModel() (m *ViewModel, err error) {
	m = &ViewModel{}
	m.fall = fall.New(m)
	m.sets.ImportRecursively = true
	return
}

type ViewModel struct {
	// view
	view View

	// Settings
	sets Settings

	// model
	fall *fall.Fall

	// ui
	// text texts
	// uiPatternErrorInfo  string
	// uiTemplateErrorInfo string
}

func (a *ViewModel) Run() error {
	return errors.New("failed to init view")
}

func (a *ViewModel) Close() error {
	return nil
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks GUI

func (a *ViewModel) OnTextPatternChanged() {

}

func (a *ViewModel) OnTextTemplateChanged() {
}

func (a *ViewModel) OnRename() {

}

func (a *ViewModel) OnImport(list []string) {

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

func (a *ViewModel) OnDelete() {
	a.fall.Delete( /*TODO*/ []int{})
}

func (a *ViewModel) OnClear() {
	a.fall.Reset()
}

func (a *ViewModel) OnExit() {
	a.fall.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Table View

func (a *ViewModel) ResetRows() {
	a.fall.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks settings

func (a *ViewModel) OnSettingImportRecursively() {
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Fall

func (a *ViewModel) OnItemUpdate(index int) {

}

func (a *ViewModel) OnItemsUpdate(from, to int) {

}

func (a *ViewModel) OnItemsInsert(from, to int) {

}

func (a *ViewModel) OnItemsDelete(from, to int) {

}

func (a *ViewModel) OnItemsReset() {

}
