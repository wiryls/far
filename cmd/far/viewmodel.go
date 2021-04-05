package main

import (
	"errors"
	"io/fs"
	"log"
	"path/filepath"
	"sort"

	"github.com/lxn/walk"

	"github.com/wiryls/far/pkg/fall"
	"github.com/wiryls/far/pkg/far"
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

		// log.Printf("access %d %d with %v", row, col, v)
	})
	return
}

func (a *ViewModel) StyleName(style *walk.CellStyle) {
	var item fall.Data
	a.fall.ReadonlyAccess(func(i []*fall.Item) { item = i[style.Row()].Load() })

	if item.Diff.IsSame() {
		return
	}

	canvas := style.Canvas()
	if canvas == nil {
		return
	}

	dash, err := walk.NewCosmeticPen(walk.PenSolid, style.TextColor)
	if err != nil {
		log.Println(err)
		return
	}

	colour := [3]walk.Color{style.TextColor, RGB2BGR(0x007947), RGB2BGR(0xDC143C)}
	bounds := style.Bounds()
	font := a.view.preview.Font()
	rect := bounds
	rect.X += 6
	rect.Y += 2
	rect.Width -= 12
	rect.Height -= 4

	for _, d := range item.Diff {
		var err error
		if int(d.Type) >= len(colour) {
			err = errors.New("unknown text color when drawing diff")
		}

		if err == nil {
			err = canvas.DrawTextPixels(d.Text, font, colour[d.Type], rect, walk.TextLeft)
		}

		var calc walk.Rectangle
		if err == nil {
			flag := walk.TextLeft | walk.TextSingleLine | walk.TextCalcRect
			calc, _, err = canvas.MeasureTextPixels(d.Text, font, rect, flag)
		}

		if err == nil && d.Type == far.DiffDelete {
			l := walk.Point{Y: calc.Y + calc.Height/2 + 1, X: calc.X}
			r := walk.Point{Y: calc.Y + calc.Height/2 + 1, X: calc.X + calc.Width}
			err = canvas.DrawLinePixels(dash, l, r)
		}

		if err == nil {
			rect.X += calc.Width
			rect.Width -= calc.Width
			if rect.Width <= 0 {
				break
			}
		}
		if err != nil {
			log.Println(err)
			break
		}
	}
}

func (a *ViewModel) Sort(col int, order walk.SortOrder) error {
	log.Println("sort", col)
	a.fall.WritableAccess(func(list []*fall.Item) {
		comp := (func(i int, j int) bool)(nil)
		switch col {
		default:
			fallthrough
		case 1:
			comp = func(i int, j int) bool {
				l := list[i].Load()
				r := list[j].Load()
				return (order == walk.SortAscending) !=
					(l.Base < r.Base || (l.Base == r.Base && l.Path < r.Path))
			}
		case 0:
			comp = func(i int, j int) bool {
				l := list[i].Load()
				r := list[j].Load()
				return (order == walk.SortAscending) !=
					(l.Stat < r.Stat || (l.Stat == r.Stat && l.Base < r.Base))
			}
		case 2:
			comp = func(i int, j int) bool {
				l := list[i].Load()
				r := list[j].Load()
				return (order == walk.SortAscending) !=
					(l.Path < r.Path || (l.Path == r.Path && l.Base < r.Base))
			}
		}
		sort.SliceStable(list, comp)
	})
	return a.SorterBase.Sort(col, order)
}

func (a *ViewModel) ColumnSortable(col int) bool {
	return col != 0
}

func (a *ViewModel) ResetRows() {
	a.fall.Reset()
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks settings

func (a *ViewModel) OnSettingImportRecursively() {
	if a.view.actionImportRecursively != nil {
		a.sets.ImportRecursively = a.view.actionImportRecursively.Checked()
	}
}

/////////////////////////////////////////////////////////////////////////////
//// Callbacks Fall

func (a *ViewModel) OnItemUpdate(index int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			log.Println("change", index)
			a.PublishRowChanged(index)
		})
	}
}

func (a *ViewModel) OnItemsUpdate(from, to int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			log.Println("change", from, to)
			a.PublishRowsChanged(from, to)
		})
	}
}

func (a *ViewModel) OnItemsInsert(from, to int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			log.Println("insert", from, to)
			a.PublishRowsInserted(from, to)
		})
	}
}

func (a *ViewModel) OnItemsDelete(from, to int) {
	if a.view.window != nil {
		a.view.window.Synchronize(func() {
			log.Println("delete", from, to)
			a.PublishRowsRemoved(from, to)
			a.PublishRowsChanged(from, to)
		})
	}
}

func (a *ViewModel) OnItemsReset() {
	if a.view.window != nil {
		log.Println("Reset")
		a.view.window.Synchronize(a.PublishRowsReset)
	}
}
