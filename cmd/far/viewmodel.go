package main

import (
	"errors"
	"log"

	"github.com/lxn/walk"

	"github.com/wiryls/far/pkg/fall"
)

// Errors of this application.
var (
	ErrSeparatorInTemplate = errors.New("separator should not be here")
	ErrInvalidPattern      = errors.New("pattern is invalid")
	ErrUnavailable         = errors.New("method unavailable")
)

/////////////////////////////////////////////////////////////////////////////

func NewViewModel() *ViewModel {
	a := &ViewModel{
		view: View{},
		fall: fall.New(nil), // TODO: callback
		text: texts{},
	}

	// a.fall.SetItemResetListener(func() {
	// 	a.view.window.Synchronize(a.PublishRowsReset)
	// })
	// a.fall.SetItemsInsertListener(func(from, to int) {
	// 	a.view.window.Synchronize(func() {
	// 		log.Println("Insert", from, "to", to)
	// 		a.PublishRowsInserted(from, to)
	// 	})
	// })
	return a
}

type ViewModel struct {
	// view
	view View

	// model
	walk.TableModelBase
	walk.SorterBase
	fall *fall.Fall

	// ui
	text texts
	// uiPatternErrorInfo  string
	// uiTemplateErrorInfo string
}

func (a *ViewModel) Run() error {
	if r := BindCallbackToView(&a.view, a); r != nil {
		return r.Run()
	}
	return errors.New("failed to init view")
}

func (a *ViewModel) Close() error {
	switch {
	case a == nil:
		fallthrough
	case a.view.window == nil:
		return nil
	}

	c := make(chan error)
	a.view.window.Synchronize(func() { c <- a.view.window.Close() })
	return <-c
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks

func (a *ViewModel) OnTextReplaceChanged() {
	a.fall.Far(a.view.pattern.Text(), a.view.template.Text())
}

func (a *ViewModel) OnTextTemplateChanged() {
	a.fall.Far(a.view.pattern.Text(), a.view.template.Text())
}

func (a *ViewModel) OnRename() {

}

func (a *ViewModel) OnImport(list []string) {
	log.Println("input", list)
	a.fall.Input(list)
}

func (a *ViewModel) OnDelete() {

}

func (a *ViewModel) OnClear() {

}

func (a *ViewModel) OnExit() {
	a.view.window.Close()
}

func (a *ViewModel) RowCount() (count int) {
	a.fall.ReadonlyAccess(func(is []fall.Item) { count = len(is) })
	return count
}

func (a *ViewModel) Value(row, col int) (v interface{}) {
	a.fall.ReadonlyAccess(func(is []fall.Item) {
		switch col {
		case 0:
			v = is[row].Stat
		case 1:
			v = is[row].Base
		case 2:
			v = is[row].Path
		}
	})
	return
}

func (a *ViewModel) StyleCell(style *walk.CellStyle) {

}

func (a *ViewModel) Sort(col int, order walk.SortOrder) error {
	return nil
}

func (a *ViewModel) ResetRows() {

}
