package main

import (
	_ "embed"
	"errors"
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

//go:embed ui.xml
var ui string

func (v *view) BuildMainWindow() {

	var b *gtk.Builder
	var err = v.err
	if err == nil {
		b, err = gtk.BuilderNewFromString(ui)
	}

	var win *gtk.ApplicationWindow
	if err == nil {
		var o glib.IObject
		o, err = b.GetObject("window")
		if err == nil {
			var ok bool
			win, ok = o.(*gtk.ApplicationWindow)
			if !ok {
				err = errors.New("invalid ApplicationWindow")
			}
		}
	}

	if err == nil {
		win.SetApplication(v.app)
		win.SetTitle("FAR")
		win.SetDefaultSize(720, 480)
		win.ShowAll()
	}

	v.err = err
}
