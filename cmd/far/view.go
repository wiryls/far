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

	actionImportRecursively *walk.Action
}

type Callback interface {

	// for TableView
	RowCount() int
	Value(row, col int) interface{}
	Sort(col int, order walk.SortOrder) error
	ResetRows()
	StyleName(style *walk.CellStyle)

	// for Other View
	OnTextPatternChanged()
	OnTextTemplateChanged()
	OnRename()
	OnImport([]string)
	OnDelete()
	OnClear()
	OnExit()

	// settings
	OnSettingImportRecursively()
}

func BindCallbackToView(v *View, c Callback) interface{ Run() error } {
	if v == nil || c == nil {
		return nil
	}
	return (*WindowToRunnerAdapter)(&ui.MainWindow{
		AssignTo:    &v.window,
		Title:       "FAR",
		Layout:      ui.VBox{},
		Size:        ui.Size{Width: 720, Height: 480},
		MinSize:     ui.Size{Width: 320, Height: 240},
		Persistent:  true,
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
						AssignTo:    &v.actionImportRecursively,
						Text:        "Import &recursively",
						Checkable:   true,
						Checked:     true,
						OnTriggered: c.OnSettingImportRecursively,
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
				Persistent:               true,
				MultiSelection:           true,
				AlwaysConsumeSpace:       true,
				LastColumnStretched:      true,
				NotSortableByHeaderClick: false,
				ColumnsOrderable:         false,
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
					{Title: "Path"},
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
