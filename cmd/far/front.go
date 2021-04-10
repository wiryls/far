package main

import (
	"io/fs"
	"log"
	"path/filepath"

	"github.com/gotk3/gotk3/glib"
	"github.com/gotk3/gotk3/gtk"
	"github.com/wiryls/far/pkg/fall"
	"github.com/wiryls/far/pkg/fill"
	"github.com/wiryls/far/pkg/filter"
)

func NewFront() (m *Front, err error) {
	m = &Front{}
	m.text.Set("zh")

	m.view, err = BuildView(m.text, m)
	m.sets.ImportRecursively = true

	m.fall = fall.New(nil)
	m.fill = fill.New(nil, m.hope.Has)
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
	fall *fall.Fall
	fill *fill.Fill
}

func (a *Front) Run() (err error) {
	a.view, err = BuildView(a.text, a)
	a.view.Run()
	return
}

func (a *Front) Close() error {
	a.view.app.Quit()
	return nil
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks GUI

func (a *Front) OnTextPatternChanged() {
	var (
		err     error
		pattern string
	)
	if err == nil {
		pattern, err = a.view.pattern.GetText()
	}
	if err == nil {
		err = a.fall.SetPattern(pattern)
	}
	if err != nil {
		a.view.pattern.SetTooltipText(err.Error())
	}
}

func (a *Front) OnTextTemplateChanged() {
	var (
		err      error
		template string
	)
	if err == nil {
		template, err = a.view.template.GetText()
	}
	if err == nil {
		err = a.fall.SetTemplate(template)
	}
	if err != nil {
		a.view.template.SetTooltipText(err.Error())
	}
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

	if len(input) > 0 {
		a.fall.Flow(input)
	}
}

func (a *Front) OnDelete() {

	var (
		err       error
		model     gtk.ITreeModel
		selection *gtk.TreeSelection
		rows      *glib.List
		list      []int
	)
	if err == nil {
		model, err = a.view.tree.GetModel()
	}
	if err == nil {
		selection, err = a.view.tree.GetSelection()
	}

	if err == nil {
		// attention: https://github.com/gotk3/gotk3/issues/590
		rows = selection.GetSelectedRows(model)
	}

	if rows != nil {
		rows.Foreach(func(i interface{}) {
			if v, ok := i.(*gtk.TreePath); ok {
				if is := v.GetIndices(); len(is) > 0 {
					list = append(list, is[0])
				}
			}
		})
	}

	if len(list) > 0 {
		// a.fall.Delete(list)
	}
}

func (a *Front) OnClear() {
}

func (a *Front) OnExit() {
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
