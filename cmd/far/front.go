package main

import (
	"log"
	"time"

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
	_, err := glib.IdleAdd(func() {
		t := time.Now()
		if a != nil && a.view != nil && a.view.list != nil {
			m := (*list)(a.view.list)
			for _, o := range differed {
				if o == nil {
					m.Clear()
				} else if err := m.Append(o); err != nil {
					log.Printf("failed to append %s: %v\n", o.Path, err)
				}
			}
		}
		log.Println("time cost - append", time.Since(t), "count", len(differed))
	})
	if err != nil {
		log.Printf("failed to idle_add: %v\n", err)
	}
}

func (a *Front) OnItemsRediffer() {
	if a != nil && a.view != nil && a.view.list != nil {
		a.fall.Stop()
		m := (*list)(a.view.list)
		out := append([]string{""}, m.Paths()...)

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
		t := time.Now()
		(*list)(a.view.list).Clear()
		log.Println("time cost - clear", time.Since(t))
	}
}
