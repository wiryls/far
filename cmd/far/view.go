package main

import (
	"github.com/lxn/walk"
	ui "github.com/lxn/walk/declarative"
)

type View struct {
	window   *walk.MainWindow
	rename   *walk.PushButton
	pattern  *walk.LineEdit
	template *walk.LineEdit
	preview  *walk.TableView
}

type Callback interface {

	// for TableView
	RowCount() int
	Value(row, col int) interface{}
	StyleCell(style *walk.CellStyle)
	Sort(col int, order walk.SortOrder) error
	ResetRows()

	// for Other View
	OnTextReplaceChanged()
	OnTextTemplateChanged()
	OnRename()
	OnImport([]string)
	OnDelete()
	OnClear()
	OnExit()
}

func BindCallbackToView(v *View, c Callback) interface{ Run() error } {
	if v == nil || c == nil {
		return nil
	}
	return (*WindowToRunnerAdapter)(&ui.MainWindow{
		AssignTo:    &v.window,
		Title:       "FAR",
		Layout:      ui.VBox{},
		MinSize:     ui.Size{Width: 640, Height: 480},
		OnDropFiles: c.OnImport,
		MenuItems: []ui.MenuItem{
			ui.Menu{
				Text: "&File",
				Items: []ui.MenuItem{
					ui.Action{
						Text:        "&Open",
						OnTriggered: func() {},
					},
					ui.Separator{},
					ui.Action{
						Text:        "&Exit",
						OnTriggered: c.OnExit,
					},
				},
			},
			ui.Menu{
				Text: "&Help",
				Items: []ui.MenuItem{
					ui.Action{
						Text:        "About",
						OnTriggered: func() {},
					},
				},
			},
		},
		Children: []ui.Widget{
			ui.Composite{
				Border: false,
				Layout: ui.Grid{
					Rows:        2,
					MarginsZero: true,
				},
				Children: []ui.Widget{
					ui.LineEdit{
						AssignTo:      &v.template,
						CueBanner:     "Find",
						OnTextChanged: c.OnTextTemplateChanged,
					},
					ui.LineEdit{
						ColumnSpan:    2,
						AssignTo:      &v.pattern,
						CueBanner:     "Replace",
						OnTextChanged: c.OnTextReplaceChanged,
					},
					ui.PushButton{
						Alignment: ui.AlignHFarVFar,
						Text:      "Rename",
						AssignTo:  &v.rename,
						OnClicked: c.OnRename,
					},
				},
			},
			ui.TableView{
				Model:               c,
				AssignTo:            &v.preview,
				AlternatingRowBG:    true,
				ColumnsOrderable:    true,
				LastColumnStretched: true,
				MultiSelection:      true,
				ContextMenuItems: []ui.MenuItem{
					ui.Action{
						Text:        "&Delete",
						OnTriggered: c.OnDelete,
					},
					ui.Separator{},
					ui.Action{
						Text:        "&Clear",
						OnTriggered: c.OnClear,
					},
				},
				Columns: []ui.TableViewColumn{
					{Title: "Stat", Width: 32},
					{Title: "Name", Width: 256},
					{Title: "Path", Alignment: ui.AlignDefault},
				},
				StyleCell: c.StyleCell,
			},
		},
	})
}

type WindowToRunnerAdapter ui.MainWindow

func (w *WindowToRunnerAdapter) Run() error {
	_, err := (*ui.MainWindow)(w).Run()
	return err
}
