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

	actionDragRecursively *walk.Action
}

type Callback interface {

	// for TableView
	RowCount() int
	Value(row, col int) interface{}
	StyleName(style *walk.CellStyle)
	Sort(col int, order walk.SortOrder) error
	ResetRows()

	// for Other View
	OnTextPatternChanged()
	OnTextTemplateChanged()
	OnRename()
	OnImport([]string)
	OnDelete()
	OnClear()
	OnExit()

	// settings
	OnSettingDragRecursively()
}

func BindCallbackToView(v *View, c Callback) interface{ Run() error } {
	if v == nil || c == nil {
		return nil
	}
	return (*WindowToRunnerAdapter)(&ui.MainWindow{
		AssignTo:    &v.window,
		Title:       "FAR",
		Layout:      ui.VBox{},
		MinSize:     ui.Size{Width: 480, Height: 320},
		Persistent:  false,
		OnDropFiles: c.OnImport,
		MenuItems: []ui.MenuItem{
			ui.Menu{
				Text: "&File",
				Items: []ui.MenuItem{
					ui.Action{
						Text:        "&Open",
						Enabled:     false,
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
				Text: "&Settings",
				Items: []ui.MenuItem{
					ui.Action{
						AssignTo:    &v.actionDragRecursively,
						Text:        "Drag &recursively",
						Checkable:   true,
						Checked:     true,
						OnTriggered: c.OnSettingDragRecursively,
					},
				},
			},
			ui.Menu{
				Text: "&Help",
				Items: []ui.MenuItem{
					ui.Action{
						Text:        "About",
						Enabled:     false,
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
						AssignTo:      &v.pattern,
						CueBanner:     "Find",
						OnTextChanged: c.OnTextPatternChanged,
					},
					ui.LineEdit{
						ColumnSpan:    2,
						AssignTo:      &v.template,
						CueBanner:     "Replace",
						OnTextChanged: c.OnTextTemplateChanged,
					},
					ui.PushButton{
						Alignment: ui.AlignHFarVFar,
						Enabled:   false,
						Text:      "Rename",
						AssignTo:  &v.rename,
						OnClicked: c.OnRename,
					},
				},
			},
			ui.TableView{
				Model:                    c,
				AssignTo:                 &v.preview,
				ColumnsOrderable:         false,
				NotSortableByHeaderClick: true,
				MultiSelection:           true,
				Persistent:               true,
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
					{Title: "Name", Width: 256, StyleCell: c.StyleName},
					{Title: "Path", Width: 512, Alignment: ui.AlignDefault},
				},
			},
		},
	})
}

type WindowToRunnerAdapter ui.MainWindow

func (w *WindowToRunnerAdapter) Run() error {
	_, err := (*ui.MainWindow)(w).Run()
	return err
}
