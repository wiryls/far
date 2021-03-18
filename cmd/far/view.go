package main

import (
	"github.com/lxn/walk"
	ui "github.com/lxn/walk/declarative"
)

type farView struct {
	closer **walk.MainWindow
	runner *ui.MainWindow
}

func (v *farView) Run() (err error) {
	_, err = v.runner.Run()
	return
}

func (v *farView) Close() (err error) {
	(*v.closer).Synchronize(func() { err = (*v.closer).Close() })
	return
}

func makeViewFromViewModel(vm *farViewModel) (vr *farView) {
	var (
		win *walk.MainWindow
		lef *walk.LineEdit
		ler *walk.LineEdit
		pbr *walk.PushButton
		tvm *walk.TableView
	)

	return &farView{
		closer: &win,
		runner: &ui.MainWindow{
			AssignTo: &win,
			Title:    "FAR",
			MinSize:  ui.Size{Width: 640, Height: 480},
			Layout:   ui.VBox{},
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
							OnTriggered: func() { win.Close() },
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
							AssignTo:      &lef,
							CueBanner:     "Find",
							Text:          ui.Bind("Pattern"),
							ToolTipText:   ui.Bind("PatternTips"),
							OnTextChanged: func() {},
						},
						ui.LineEdit{
							ColumnSpan:    2,
							AssignTo:      &ler,
							CueBanner:     "Replace",
							ToolTipText:   "?",
							OnTextChanged: func() {},
						},
						ui.PushButton{
							Alignment: ui.AlignHFarVFar,
							Text:      "Rename",
							AssignTo:  &pbr,
							OnClicked: func() {},
						},
					},
				},
				ui.TableView{
					AssignTo:            &tvm,
					LastColumnStretched: true,
					ContextMenuItems: []ui.MenuItem{
						ui.Action{
							Text: "&Delete",
							OnTriggered: func() {
							},
						},
						ui.Separator{},
						ui.Action{
							Text:        "&Clear",
							OnTriggered: func() {},
						},
					},
					Columns: []ui.TableViewColumn{
						{Title: "Stat", Width: 32},
						{Title: "Name", Width: 256},
						{Title: "Path", Alignment: ui.AlignDefault},
					},
				},
			},
			OnDropFiles: func(files []string) {},
		},
	}
}
