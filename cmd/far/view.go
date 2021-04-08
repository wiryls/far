package main

import (
	"os"

	"github.com/gotk3/gotk3/glib"
	"github.com/gotk3/gotk3/gtk"
)

type Callback interface {

	// for TableView
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
	OnSettingImportRecursively()
}

func BuildView() (err error) {

	var app *gtk.Application
	if err == nil {
		app, err = gtk.ApplicationNew("wiryls.github.com", glib.APPLICATION_FLAGS_NONE)
	}

	var vvv *view
	if err == nil {
		vvv = &view{app: app}
		app.Connect("activate", vvv.BuildMainWindow)
		app.Run(os.Args)

		// References:
		// - [signals](https://wiki.gnome.org/HowDoI/GtkApplication)
	}

	if err == nil {
		err = vvv.err
	}

	return
}

type view struct {
	app *gtk.Application
	err error
}

func (v *view) BuildMainWindow() {

	var err = v.err
	var top *gtk.HeaderBar
	if err == nil {
		top, err = gtk.HeaderBarNew()

		if err == nil {
			top.SetShowCloseButton(true)
		}
	}

	if err == nil {
		var menu *gtk.MenuButton
		menu, err = gtk.MenuButtonNew()

		var icon *gtk.Image
		if err == nil {
			icon, err = gtk.ImageNewFromIconName("open-menu-symbolic", gtk.ICON_SIZE_MENU)
		}

		if err == nil {
			menu.SetImage(icon)
			top.Add(menu)
		}
	}

	var mid *gtk.Grid
	if err == nil {
		mid, err = gtk.GridNew()
	}

	if err == nil {
		var (
			pattern  *gtk.Entry
			template *gtk.Entry
			rename   *gtk.Button
		)

		if err == nil {
			pattern, err = gtk.EntryNew()
		}

		if err == nil {
			template, err = gtk.EntryNew()
		}

		if err == nil {
			rename, err = gtk.ButtonNew()
		}

		if err == nil {
			pattern.SetHExpand(true)
			pattern.SetHAlign(gtk.ALIGN_FILL)
			template.SetHExpand(true)
			template.SetHAlign(gtk.ALIGN_FILL)
			rename.SetLabel("Rename")

			mid.SetRowHomogeneous(true)
			mid.SetColumnSpacing(4)
			mid.SetRowSpacing(4)

			mid.Attach(pattern, 0, 0, 1, 1)
			mid.AttachNextTo(template, pattern, gtk.POS_BOTTOM, 1, 1)
			mid.AttachNextTo(rename, pattern, gtk.POS_RIGHT, 1, 1)
		}
	}

	var bot *gtk.TreeView
	if err == nil {
		bot, err = v.BuildMainWindowTreeView()
	}

	var box *gtk.Box
	if err == nil {
		box, err = gtk.BoxNew(gtk.ORIENTATION_VERTICAL, 4)

		if err == nil {
			box.SetMarginTop(8)
			box.SetMarginStart(8)
			box.SetMarginBottom(8)
			box.SetMarginEnd(8)
			box.Add(mid)
			box.Add(bot)
		}
	}

	var win *gtk.ApplicationWindow
	if err == nil {
		win, err = gtk.ApplicationWindowNew(v.app)
	}

	if err == nil {
		win.Add(box)
		win.SetTitle("FAR")
		win.SetIconName("application-x-executable")
		win.SetTitlebar(top)
		win.SetDefaultSize(720, 480)
		win.ShowAll()
	}

	v.err = err
}

func (v *view) BuildMainWindowTreeView() (tree *gtk.TreeView, err error) {

	var stat *gtk.TreeViewColumn
	if err == nil {
		stat, err = gtk.TreeViewColumnNew()
		if err == nil {
			stat.SetTitle("Stat")
		}
	}

	var name *gtk.TreeViewColumn
	if err == nil {
		name, err = gtk.TreeViewColumnNew()
		if err == nil {
			name.SetTitle("Name")
			name.SetClickable(true)
			name.SetReorderable(true)
			name.SetResizable(true)
			name.SetMinWidth(64)
		}
	}

	var path *gtk.TreeViewColumn
	if err == nil {
		path, err = gtk.TreeViewColumnNew()
		if err == nil {
			path.SetTitle("Path")
			path.SetClickable(true)
			path.SetReorderable(true)
			path.SetResizable(true)
			path.SetSizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
			path.SetMinWidth(64)
		}
	}

	var model *gtk.ListStore
	if err == nil {
		model, err = gtk.ListStoreNew(
			glib.TYPE_STRING,
			glib.TYPE_STRING,
			glib.TYPE_STRING)

		if err == nil {
			model.Set(model.Append(), []int{0, 1, 2}, []interface{}{"Foo", "Bar", "Bar"})
			model.Set(model.Append(), []int{0, 1, 2}, []interface{}{"Foo", "Bar", "Bar"})
		}
	}

	if err == nil {
		tree, err = gtk.TreeViewNewWithModel(model)
	}

	if err == nil {
		tree.AppendColumn(stat)
		tree.AppendColumn(name)
		tree.AppendColumn(path)

		tree.SetVExpand(true)
		tree.SetVAlign(gtk.ALIGN_FILL)
		tree.SetGridLines(gtk.TREE_VIEW_GRID_LINES_VERTICAL)
		// tree.SetRe
	}

	return
}
