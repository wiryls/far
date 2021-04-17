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
	a.view.app.Quit()
	return nil
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks View

func (a *Front) OnActionPatternChanged() {
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
	if err == nil {
		a.view.pattern.SetTooltipText("")
		a.OnItemsRediffer()
	} else {
		a.view.pattern.SetTooltipText(err.Error())
	}
}

func (a *Front) OnActionTemplateChanged() {
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
	if err == nil {
		a.view.template.SetTooltipText("")
		a.OnItemsRediffer()
	} else {
		a.view.template.SetTooltipText(err.Error())
	}
}

func (a *Front) OnActionRename() {

}

func (a *Front) OnActionImport(list []string) {

	var input []string
	if !a.sets.ImportRecursively {
		input = list

	} else {
		for _, file := range list {
			if err := filepath.WalkDir(file, func(path string, d fs.DirEntry, err error) error {
				input = append(input, path)
				return err
			}); err != nil {
				log.Println("OnActionImport", err)
			}
		}

	}

	if len(input) > 0 {
		a.fill.Fill(input)
	}
}

func (a *Front) OnActionDelete() {
	a.DoItemsDeleteSelected()
}

func (a *Front) OnActionClear() {
	a.DoItemsReset()
}

func (a *Front) OnActionExit() {
	a.DoItemsReset()
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

func (a *Front) OnItemsDiffered(i int, differed []fall.Output) {
	_, err := glib.IdleAdd(func() {
		if a != nil && a.view != nil && a.view.list != nil {
			m := (*list)(a.view.list)
			if i == 0 {
				m.Clear()
			}
			for _, o := range differed {
				err := m.Append(o)
				if err != nil {
					log.Printf("failed to append %s: %v\n", o.Target, err)
				}
			}
		}
	})
	if err != nil {
		log.Printf("failed to idle_add: %v\n", err)
	}
}

func (a *Front) OnItemsRediffer() {
	if a != nil && a.view != nil && a.view.list != nil {
		a.fall.Stop()
		m := (*list)(a.view.list)
		out := m.Paths()

		a.fall.Flow(out)
	}
}

func (a *Front) DoItemsDeleteSelected() {
	if a != nil && a.view != nil && a.view.list != nil {

		var (
			err       error
			selection *gtk.TreeSelection
			rows      *glib.List
		)
		if err == nil {
			selection, err = a.view.tree.GetSelection()
		}
		if err == nil {
			// attention: https://github.com/gotk3/gotk3/issues/590
			rows = selection.GetSelectedRows(a.view.list).Reverse()
		}
		if rows != nil {
			m := (*list)(a.view.list)
			// [Removing multiple rows from a Gtk TreeStore]
			// (https://stackoverflow.com/a/27933886)
			rows.Foreach(func(x interface{}) {
				v, ok := x.(*gtk.TreePath)
				if ok {
					i, err := m.GetIter(v)

					var o *glib.Value
					if err == nil {
						o, err = m.GetValue(i, 2)
					}

					var s string
					if err == nil {
						s, err = o.GetString()
					}
					if err == nil {
						a.hope.Pop(s)
					} else if err != nil {
						log.Printf("failed to pop %s: %v\n", s, err)
						err = nil
					}

					err = m.Delete(v)
					if err != nil {
						log.Printf("failed to delete %s: %v\n", s, err)
					}
				}
			})
		}

		if err != nil {
			log.Println("failed to delete selected:", err)
		}
	}
}

func (a *Front) DoItemsReset() {
	if a != nil && a.view != nil && a.view.list != nil {
		(*list)(a.view.list).Clear()
	}
}
