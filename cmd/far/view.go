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

func BuildView(res Resource) (err error) {

	var app *gtk.Application
	if err == nil {
		app, err = gtk.ApplicationNew("wiryls.github.com", glib.APPLICATION_FLAGS_NONE)
	}

	var vvv *view
	if err == nil {
		vvv = &view{app: app, res: res}
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
	res Resource
	err error
}

func (v *view) BuildMainWindow() {

	var err = v.err
	var top *gtk.HeaderBar
	if err == nil {
		top, err = v.BuildHeaderBar()
	}

	var mid *gtk.Grid
	if err == nil {
		mid, err = v.BuildInput()
	}

	var bot *gtk.ScrolledWindow
	if err == nil {
		var tree *gtk.TreeView
		if err == nil {
			bot, err = gtk.ScrolledWindowNew(nil, nil)
		}

		if err == nil {
			tree, err = v.BuildTable()
		}

		if err == nil {
			bot.Add(tree)
			bot.SetShadowType(gtk.SHADOW_ETCHED_IN)
			// [GtkShadowType]
			// (http://gtk.php.net/manual/en/html/gtk/gtk.enum.shadowtype.html)
		}
	}

	var box *gtk.Box
	if err == nil {
		box, err = gtk.BoxNew(gtk.ORIENTATION_VERTICAL, 4)

		if err == nil {
			box.SetMarginTop(6)
			box.SetMarginStart(6)
			box.SetMarginBottom(6)
			box.SetMarginEnd(6)
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
		win.SetTitle(v.res("main_title"))
		win.SetIconName("application-x-executable")
		win.SetTitlebar(top)
		win.SetDefaultSize(720, 480)
		win.ShowAll()
	}

	v.err = err
}

func (v *view) BuildHeaderBar() (head *gtk.HeaderBar, err error) {

	if err == nil {
		head, err = gtk.HeaderBarNew()

		if err == nil {
			head.SetShowCloseButton(true)
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
			head.Add(menu)
			head.SetTitle(v.res("main_title"))
		}
	}
	return
}

func (v *view) BuildInput() (grid *gtk.Grid, err error) {
	if err == nil {
		grid, err = gtk.GridNew()
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

			rename.SetLabel(v.res("main_rename"))

			grid.SetRowHomogeneous(true)
			grid.SetColumnSpacing(4)
			grid.SetRowSpacing(4)

			grid.Attach(pattern, 0, 0, 1, 1)
			grid.AttachNextTo(template, pattern, gtk.POS_BOTTOM, 1, 1)
			grid.AttachNextTo(rename, pattern, gtk.POS_RIGHT, 1, 1)
		}
	}
	return
}

func (v *view) BuildTable() (tree *gtk.TreeView, err error) {

	var stat *gtk.TreeViewColumn
	if err == nil {
		var cell *gtk.CellRendererText
		if err == nil {
			cell, err = gtk.CellRendererTextNew()
		}

		if err == nil {
			stat, err = gtk.TreeViewColumnNewWithAttribute(
				v.res("main_column_name"), cell, "text", 0)
		}

		if err == nil {
			stat.SetTitle(v.res("main_column_stat"))
			stat.SetClickable(true)
			stat.SetMinWidth(32)
			stat.SetSortColumnID(0)
		}
	}

	var name *gtk.TreeViewColumn
	if err == nil {

		var cell *gtk.CellRendererText
		if err == nil {
			cell, err = gtk.CellRendererTextNew()
		}

		if err == nil {
			name, err = gtk.TreeViewColumnNewWithAttribute(
				v.res("main_column_name"), cell, "text", 1)
		}

		if err == nil {
			name.SetClickable(true)
			name.SetResizable(true)
			name.SetMinWidth(64)
			name.SetFixedWidth(256)
			name.SetSortColumnID(1)
		}
	}

	var path *gtk.TreeViewColumn
	if err == nil {

		var cell *gtk.CellRendererText
		if err == nil {
			cell, err = gtk.CellRendererTextNew()
		}

		if err == nil {
			path, err = gtk.TreeViewColumnNewWithAttribute(
				v.res("main_column_path"), cell, "text", 2)
		}

		if err == nil {
			path.SetClickable(true)
			path.SetResizable(true)
			path.SetSizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
			path.SetMinWidth(64)
			path.SetSortColumnID(2)
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
	}

	return
}
