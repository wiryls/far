package main

import (
	"os"
	"strings"

	"github.com/gotk3/gotk3/gdk"
	"github.com/gotk3/gotk3/glib"
	"github.com/gotk3/gotk3/gtk"
	"github.com/wiryls/pkg/errors/cerrors"
)

type Callbacks interface {

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

type view struct {
	app      *gtk.Application
	tree     *gtk.TreeView
	pattern  *gtk.Entry
	template *gtk.Entry
	rename   *gtk.Button
}

func (v *view) Run() {
	if v != nil && v.app != nil {
		v.app.Run(os.Args)
	}
}

func BuildView(res Resource, cbs Callbacks) (out *view, err error) {

	if cbs == nil {
		err = cerrors.NilArgument("cbs Callbacks")
	}

	if res == nil {
		res.Set("en")
	}

	var app *gtk.Application
	if err == nil {
		app, err = gtk.ApplicationNew("wiryls.github.com", glib.APPLICATION_FLAGS_NONE)
	}

	var bui *builder
	if err == nil {
		bui = &builder{out: &view{app: app}, res: res, cbs: cbs}
		app.Connect("activate", bui.BuildMainWindow)

		// [signals]
		// (https://wiki.gnome.org/HowDoI/GtkApplication)
	}

	if err == nil {
		out = bui.out
		err = bui.err
	}

	return
}

type builder struct {
	out *view
	res Resource
	cbs Callbacks
	err error
}

func (b *builder) BuildMainWindow() {

	var err = b.err
	var top *gtk.HeaderBar
	if err == nil {
		top, err = b.BuildHeaderBar()
	}

	var mid *gtk.Grid
	if err == nil {
		mid, err = b.BuildInput()
	}

	var bot *gtk.ScrolledWindow
	if err == nil {
		var tree *gtk.TreeView
		if err == nil {
			bot, err = gtk.ScrolledWindowNew(nil, nil)
		}

		if err == nil {
			tree, err = b.BuildTable()
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
		win, err = gtk.ApplicationWindowNew(b.out.app)
	}

	if err == nil {
		win.Add(box)

		win.Connect("destroy", b.cbs.OnExit)

		win.SetTitle(b.res("main_title"))
		win.SetIconName("application-x-executable")
		win.SetTitlebar(top)
		win.SetDefaultSize(720, 480)
		win.ShowAll()
	}

	b.err = err
}

func (b *builder) BuildHeaderBar() (head *gtk.HeaderBar, err error) {

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
			head.SetTitle(b.res("main_title"))
		}
	}
	return
}

func (b *builder) BuildInput() (grid *gtk.Grid, err error) {
	if err == nil {
		grid, err = gtk.GridNew()
	}

	var (
		pattern  *gtk.Entry
		template *gtk.Entry
		rename   *gtk.Button
	)

	if err == nil {

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
			pattern.SetPlaceholderText(b.res("main_input_pattern"))
			pattern.Connect("changed", b.cbs.OnTextPatternChanged)
			// [The “changed” signal]
			// (https://developer.gnome.org/gtk3/unstable/GtkEditable.html#GtkEditable-changed)

			template.SetHExpand(true)
			template.SetHAlign(gtk.ALIGN_FILL)
			template.SetPlaceholderText(b.res("main_input_template"))
			template.Connect("changed", b.cbs.OnTextTemplateChanged)

			rename.SetLabel(b.res("main_rename"))
			rename.Connect("clicked", b.cbs.OnRename)

			grid.SetRowHomogeneous(true)
			grid.SetColumnSpacing(4)
			grid.SetRowSpacing(4)

			grid.Attach(pattern, 0, 0, 1, 1)
			grid.AttachNextTo(template, pattern, gtk.POS_BOTTOM, 1, 1)
			grid.AttachNextTo(rename, pattern, gtk.POS_RIGHT, 1, 1)
		}
	}

	if err == nil {
		b.out.pattern = pattern
		b.out.template = template
		b.out.rename = rename
	}

	return
}

func (b *builder) BuildTable() (tree *gtk.TreeView, err error) {

	const (
		ColumnStat = iota
		ColumnName
		ColumnPath
	)

	var cell *gtk.CellRendererText
	if err == nil {
		cell, err = gtk.CellRendererTextNew()
	}

	var stat *gtk.TreeViewColumn
	if err == nil {

		if err == nil {
			stat, err = gtk.TreeViewColumnNewWithAttribute(
				b.res("main_tree_column_stat"), cell, "text", ColumnStat)
		}

		if err == nil {
			stat.SetSortColumnID(ColumnStat)
			stat.SetClickable(true)
			stat.SetMinWidth(32)
		}
	}

	var name *gtk.TreeViewColumn
	if err == nil {

		if err == nil {
			name, err = gtk.TreeViewColumnNewWithAttribute(
				b.res("main_tree_column_name"), cell, "markup", ColumnName)
		}

		if err == nil {
			name.SetSortColumnID(ColumnName)
			name.SetClickable(true)
			name.SetResizable(true)
			name.SetMinWidth(64)
			name.SetFixedWidth(256)
		}
	}

	var path *gtk.TreeViewColumn
	if err == nil {

		if err == nil {
			path, err = gtk.TreeViewColumnNewWithAttribute(
				b.res("main_tree_column_path"), cell, "text", ColumnPath)
		}

		if err == nil {
			path.SetSortColumnID(ColumnPath)
			path.SetClickable(true)
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
			cols := []int{ColumnStat, ColumnName, ColumnPath}
			model.Set(model.Append(), cols, []interface{}{"Foo", "?", "Bar"})
			model.Set(model.Append(), cols, []interface{}{"Foo", "<b>Bar</b>", "Bar"})
		}
	}

	var entries []gtk.TargetEntry
	if err == nil {
		var entry *gtk.TargetEntry
		if err == nil {
			entry, err = gtk.TargetEntryNew("text/uri-list", gtk.TARGET_OTHER_APP, 0)
		}
		if err == nil {
			entries = append(entries, *entry)
		}
		if err == nil {
			entry, err = gtk.TargetEntryNew("text/plain", gtk.TARGET_OTHER_APP, 0)
		}
		if err == nil {
			entries = append(entries, *entry)
		}
	}

	var (
		menu      *gtk.Menu
		delete    *gtk.MenuItem
		separator *gtk.SeparatorMenuItem
		clear     *gtk.MenuItem
	)
	if err == nil {
		menu, err = gtk.MenuNew()
	}
	if err == nil {
		delete, err = gtk.MenuItemNewWithLabel(b.res("main_tree_menu_delete"))
	}
	if err == nil {
		separator, err = gtk.SeparatorMenuItemNew()
	}
	if err == nil {
		clear, err = gtk.MenuItemNewWithLabel(b.res("main_tree_menu_clear"))
	}
	if err == nil {
		delete.Connect("activate", b.cbs.OnDelete)
		clear.Connect("activate", b.cbs.OnClear)

		menu.Add(delete)
		menu.Add(separator)
		menu.Add(clear)
	}

	if err == nil {
		tree, err = gtk.TreeViewNewWithModel(model)
	}

	if err == nil {
		var sel *gtk.TreeSelection
		sel, err = tree.GetSelection()
		if err == nil {
			sel.SetMode(gtk.SELECTION_MULTIPLE)
		}
	}

	if err == nil {
		tree.Connect("button-press-event", func(tree *gtk.TreeView, source *gdk.Event) bool {
			event := gdk.EventButtonNewFromEvent(source)
			model, err := tree.GetModel()
			if err == nil &&
				event.Type() == gdk.EVENT_BUTTON_PRESS &&
				event.Button() == gdk.BUTTON_SECONDARY {

				selection, err := tree.GetSelection()
				delete.SetSensitive(err == nil && selection.CountSelectedRows() > 0)

				_, ok := model.ToTreeModel().GetIterFirst()
				clear.SetSensitive(ok)

				menu.ShowAll()
				menu.PopupAtPointer(source)
				return true
			}

			return false
		})

		tree.DragDestSet(gtk.DEST_DEFAULT_ALL, entries, gdk.ACTION_COPY)
		tree.Connect("drag-data-received", func(tree *gtk.TreeView, ctx *gdk.DragContext, x, y int, data *gtk.SelectionData, m int, t uint) {
			src := strings.ReplaceAll(string(data.GetData()), "\r\n", "\n")
			dst := strings.Split(src, "\n")
			b.cbs.OnImport(dst)
		})

		tree.AppendColumn(stat)
		tree.AppendColumn(name)
		tree.AppendColumn(path)

		tree.SetVExpand(true)
		tree.SetVAlign(gtk.ALIGN_FILL)
		tree.SetGridLines(gtk.TREE_VIEW_GRID_LINES_VERTICAL)
	}

	if err == nil {
		b.out.tree = tree
	}

	return
}
