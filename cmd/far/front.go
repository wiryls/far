package main

import (
	"os"
	"path/filepath"

	"github.com/wiryls/far/pkg/fall"
	"github.com/wiryls/far/pkg/fill"
	"github.com/wiryls/far/pkg/filter"
)

func NewFront() (m *Front, err error) {
	m = &Front{}
	m.text.Set("zh")

	m.view = BuildView(m.text, m)
	m.sets.ImportRecursively = true

	m.fill = fill.New(m.OnItemsImported, m.hope.Has)
	m.fall = fall.New(m.OnItemsDiffered)
	return
}

type Front struct {
	// view
	text Resource
	view *view

	// Settings
	sets Settings

	// model
	hope filter.Filter
	fill *fill.Fill
	fall *fall.Fall
}

func (a *Front) Run() (err error) {
	a.view.Run()
	return
}

func (a *Front) Close() error {
	return nil
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks View

func (a *Front) OnActionPatternChanged() {

}

func (a *Front) OnActionTemplateChanged() {

}

func (a *Front) OnActionRename() {

}

func (a *Front) OnActionImport(list []string) {
	if len(list) > 0 {
		a.fill.Fill(list, a.sets.ImportRecursively)
	}
}

func (a *Front) OnActionDelete() {
	a.DoItemsDeleteSelected()
}

func (a *Front) OnActionClear() {
	a.DoItemsReset()
	a.hope.Reset()
}

func (a *Front) OnActionExit() {
	a.DoItemsReset()
	a.hope.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks setting

func (a *Front) OnSettingImportRecursively() {

}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Items

func (a *Front) OnItemsImported(imported []string) {
	l := make([]string, 0, len(imported))
	for _, s := range imported {
		if !a.hope.Put(s) {
			l = append(l, s)
		}
	}
	a.fall.Flow(l)
}

func (a *Front) OnItemsDiffered(differed []*fall.Output) {

}

func (a *Front) OnItemsRediffer() {

}

func (a *Front) DoItemsDeleteSelected() {

}

func (a *Front) DoItemsReset() {

}

func init() {
	font := filepath.Join("C:\\", "Windows", "Fonts", "msyh.ttc")
	os.Setenv("FYNE_FONT", font)
}
