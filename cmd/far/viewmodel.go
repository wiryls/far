package main

import (
	"errors"
	"log"
	"sort"

	"github.com/lxn/walk"

	"github.com/wiryls/far/pkg/fall"
)

func NewViewModel() (m *ViewModel) {
	m = &ViewModel{}
	m.fall = fall.New(m)
	return
}

type ViewModel struct {
	// view
	view View

	// model
	walk.TableModelBase
	walk.SorterBase
	fall *fall.Fall

	// ui
	// text texts
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
//// Callbacks GUI

func (a *ViewModel) OnTextPatternChanged() {
	switch err := a.fall.Far(a.view.pattern.Text(), a.view.template.Text()); {
	case err != nil:
		a.view.pattern.SetToolTipText(err.Error())
	case a.view.pattern.ToolTipText() != "":
		a.view.pattern.SetToolTipText("")
	}
}

func (a *ViewModel) OnTextTemplateChanged() {
	switch err := a.fall.Far(a.view.pattern.Text(), a.view.template.Text()); {
	case err != nil:
		a.view.template.SetToolTipText(err.Error())
	case a.view.template.ToolTipText() != "":
		a.view.template.SetToolTipText("")
	}
}

func (a *ViewModel) OnRename() {

}

func (a *ViewModel) OnImport(list []string) {
	log.Println("OnImport", list)
	a.fall.Input(list)
}

func (a *ViewModel) OnDelete() {
	a.fall.Delete(a.view.preview.SelectedIndexes())
}

func (a *ViewModel) OnClear() {
	a.fall.Reset()
}

func (a *ViewModel) OnExit() {
	a.fall.Reset()
	a.view.window.Close()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Table View

func (a *ViewModel) RowCount() (count int) {
	a.fall.ReadonlyAccess(func(is []*fall.Item) { count = len(is) })
	return count
}

func (a *ViewModel) Value(row, col int) (v interface{}) {
	a.fall.ReadonlyAccess(func(is []*fall.Item) {
		item := is[row].Load()
		switch col {
		case 0:
			v = item.Stat
		case 1:
			v = item.Base
		case 2:
			v = item.Path
		}
	})
	return
}

func (a *ViewModel) StyleCell(style *walk.CellStyle) {

}

func (a *ViewModel) Sort(col int, order walk.SortOrder) error {
	a.fall.WritableAccess(func(list []*fall.Item) {
		comp := (func(i int, j int) bool)(nil)
		switch col {
		default:
			fallthrough
		case 1:
			comp = func(i int, j int) bool {
				return (order == walk.SortAscending) !=
					(list[i].Load().Base < list[j].Load().Base)
			}
		case 0:
			comp = func(i int, j int) bool {
				return (order == walk.SortAscending) !=
					(list[i].Load().Stat < list[j].Load().Stat)
			}
		case 2:
			comp = func(i int, j int) bool {
				return (order == walk.SortAscending) !=
					(list[i].Load().Path < list[j].Load().Path)
			}
		}
		sort.SliceStable(list, comp)
	})
	return a.SorterBase.Sort(col, order)
}

func (a *ViewModel) ResetRows() {
	a.fall.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Fall

func (a *ViewModel) OnItemUpdate(index int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			a.PublishRowChanged(index)
		})
	}
}

func (a *ViewModel) OnItemsUpdate(from, to int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			a.PublishRowsChanged(from, to)
		})
	}
}

func (a *ViewModel) OnItemsInsert(from, to int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			a.PublishRowsInserted(from, to)
		})
	}
}

func (a *ViewModel) OnItemsDelete(from, to int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			a.PublishRowsRemoved(from, to)
		})
	}
}

func (a *ViewModel) OnItemsReset() {
	if a.view.window != nil {
		a.view.window.Synchronize(a.PublishRowsReset)
	}
}
