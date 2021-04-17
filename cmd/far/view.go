package main

import (
	"encoding/xml"
	"os"
	"strings"

	"github.com/gotk3/gotk3/gdk"
	"github.com/gotk3/gotk3/glib"
	"github.com/gotk3/gotk3/gtk"
	"github.com/wiryls/far/pkg/fall"
	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/pkg/errors/cerrors"
)

type Callbacks interface {

	// for Other View
	OnActionPatternChanged()
	OnActionTemplateChanged()
	OnActionRename()
	OnActionImport([]string)
	OnActionDelete()
	OnActionClear()
	OnActionExit()

	// settings
	OnSettingImportRecursively()
}

type view struct {
	app      *gtk.Application
	list     *gtk.ListStore
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
		_, err = app.Connect("activate", bui.BuildMainWindow)

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
			tree, err = b.BuildTree()
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
		_, err = win.Connect("destroy", b.cbs.OnActionExit)
	}
	if err == nil {
		win.Add(box)
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
		_, err = pattern.Connect("changed", b.cbs.OnActionPatternChanged)

		// [The “changed” signal]
		// (https://developer.gnome.org/gtk3/unstable/GtkEditable.html#GtkEditable-changed)
	}
	if err == nil {
		template.SetHExpand(true)
		template.SetHAlign(gtk.ALIGN_FILL)
		template.SetPlaceholderText(b.res("main_input_template"))
		_, err = template.Connect("changed", b.cbs.OnActionTemplateChanged)
	}
	if err == nil {
		rename.SetLabel(b.res("main_rename"))
		_, err = rename.Connect("clicked", b.cbs.OnActionRename)
	}

	if err == nil {
		grid.SetRowHomogeneous(true)
		grid.SetColumnSpacing(4)
		grid.SetRowSpacing(4)

		grid.Attach(pattern, 0, 0, 1, 1)
		grid.AttachNextTo(template, pattern, gtk.POS_BOTTOM, 1, 1)
		grid.AttachNextTo(rename, pattern, gtk.POS_RIGHT, 1, 1)

		b.out.pattern = pattern
		b.out.template = template
		b.out.rename = rename
	}

	return
}

func (b *builder) BuildTree() (tree *gtk.TreeView, err error) {

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
		_, err = delete.Connect("activate", b.cbs.OnActionDelete)
	}
	if err == nil {
		separator, err = gtk.SeparatorMenuItemNew()
	}
	if err == nil {
		clear, err = gtk.MenuItemNewWithLabel(b.res("main_tree_menu_clear"))
	}
	if err == nil {
		_, err = clear.Connect("activate", b.cbs.OnActionClear)
	}
	if err == nil {
		menu.Add(delete)
		menu.Add(separator)
		menu.Add(clear)
	}

	if err == nil {
		tree, err = gtk.TreeViewNew()
	}
	if err == nil {
		err = b.BindList(tree)
	}
	if err == nil {
		_, err = tree.Connect("button-press-event", func(tree *gtk.TreeView, source *gdk.Event) bool {
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
	}
	if err == nil {
		_, err = tree.Connect("drag-data-received", func(tree *gtk.TreeView, ctx *gdk.DragContext, x, y int, data *gtk.SelectionData, m int, t uint) {
			src := strings.ReplaceAll(string(data.GetData()), "\r\n", "\n")
			dst := strings.Split(src, "\n")
			out := make([]string, 0, len(dst))
			for _, str := range dst {
				str = strings.TrimPrefix(str, "file:///")
				str = strings.TrimSpace(str)
				str = strings.TrimFunc(str, func(r rune) bool { return r == 0 })
				if len(str) > 0 {
					out = append(out, str)
				}
			}
			b.cbs.OnActionImport(out)
		})
	}
	var selection *gtk.TreeSelection
	if err == nil {
		selection, err = tree.GetSelection()
	}
	if err == nil {
		tree.DragDestSet(gtk.DEST_DEFAULT_ALL, entries, gdk.ACTION_COPY)
		tree.SetVExpand(true)
		tree.SetVAlign(gtk.ALIGN_FILL)
		tree.SetGridLines(gtk.TREE_VIEW_GRID_LINES_VERTICAL)

		selection.SetMode(gtk.SELECTION_MULTIPLE)

		b.out.tree = tree
	}

	return
}

func (b *builder) BindList(tree *gtk.TreeView) (err error) {

	const (
		ColumnStat = iota
		ColumnName
		ColumnPath
	)

	var list *gtk.ListStore
	if err == nil {
		list, err = gtk.ListStoreNew(
			glib.TYPE_STRING,
			glib.TYPE_STRING,
			glib.TYPE_STRING,
			glib.TYPE_OBJECT)
	}

	var (
		cell *gtk.CellRendererText
		stat *gtk.TreeViewColumn
		name *gtk.TreeViewColumn
		path *gtk.TreeViewColumn
	)
	if err == nil {
		cell, err = gtk.CellRendererTextNew()
	}
	if err == nil {
		stat, err = gtk.TreeViewColumnNewWithAttribute(
			b.res("main_tree_column_stat"), cell, "text", ColumnStat)
	}
	if err == nil {
		name, err = gtk.TreeViewColumnNewWithAttribute(
			b.res("main_tree_column_name"), cell, "markup", ColumnName)
	}
	if err == nil {
		path, err = gtk.TreeViewColumnNewWithAttribute(
			b.res("main_tree_column_path"), cell, "text", ColumnPath)
	}
	if err == nil {
		stat.SetSortColumnID(ColumnStat)
		stat.SetClickable(true)
		stat.SetMinWidth(32)

		name.SetSortColumnID(ColumnName)
		name.SetClickable(true)
		name.SetResizable(true)
		name.SetMinWidth(64)
		name.SetFixedWidth(256)

		path.SetSortColumnID(ColumnPath)
		path.SetClickable(true)
		path.SetResizable(true)
		path.SetSizing(gtk.TREE_VIEW_COLUMN_AUTOSIZE)
		path.SetMinWidth(64)

		tree.SetModel(list)
		tree.AppendColumn(stat)
		tree.AppendColumn(name)
		tree.AppendColumn(path)

		b.out.list = list
	}

	return
}

type list gtk.ListStore

func (l *list) Append(o fall.Output) (err error) {
	m := (*gtk.ListStore)(l)
	return m.Set(m.Append(), []int{
		0, 1, 2, 3,
	}, []interface{}{
		"Preview", toMarkup(o), o.Source, o,
	})
}

func (l *list) Paths() (out []string) {
	m := (*gtk.ListStore)(l)
	m.ForEach(func(model *gtk.TreeModel, path *gtk.TreePath, iter *gtk.TreeIter, u ...interface{}) bool {
		v, err := model.GetValue(iter, 2)
		if err == nil {
			s, err := v.GetString()
			if err == nil {
				out = append(out, s)
			}
		}
		return err != nil
	})
	return
}

func (l *list) Delete(path *gtk.TreePath) error {
	m := (*gtk.ListStore)(l)
	i, err := m.GetIter(path)
	if err == nil {
		m.Remove(i)
	}
	return err
}

func (l *list) Clear() {
	(*gtk.ListStore)(l).Clear()
}

func toMarkup(o fall.Output) string {
	// fast path
	if o.Differ.IsSame() && !strings.ContainsAny(o.Target, "><") {
		return o.Target
	}

	// slow path
	b := strings.Builder{}
	for _, d := range o.Differ {
		switch d.Type {
		case far.DiffInsert:
			b.WriteString(`<span foreground="#007947">`)
			_ = xml.EscapeText(&b, []byte(d.Text))
			b.WriteString("</span>")
		case far.DiffDelete:
			b.WriteString(`<span foreground="#DC143C" strikethrough="true">`)
			_ = xml.EscapeText(&b, []byte(d.Text))
			b.WriteString("</span>")
		default:
			// I use xml.EscapeText as there is no g_markup_escape_text in gotk3
			_ = xml.EscapeText(&b, []byte(d.Text))
		}
	}

	// [Pango markup]
	// (https://developer.gnome.org/pango/1.46/pango-Markup.html)
	return b.String()
}
