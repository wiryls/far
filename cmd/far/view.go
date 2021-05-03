package main

import (
	"regexp"
	"strings"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
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
	app fyne.App
	win fyne.Window
}

func (v *view) Run() {
	v.win.Show()
	v.app.Run()
}

func BuildView(res Resource, cbs Callbacks) (out *view) {
	b := theViewBuilder{out: &view{}, res: res, cbs: cbs}
	b.Build()
	return b.out
}

type theViewBuilder struct {
	out *view
	res Resource
	cbs Callbacks
}

func (b *theViewBuilder) Build() {
	if b == nil || b.out == nil {
		return
	}

	app := app.New()
	win := app.NewWindow(b.res("main_title"))
	win.Resize(fyne.NewSize(640, 480))
	win.SetContent(
		container.NewVBox(
			b.BuildInput(),
			b.BuildTable(),
		),
	)

	b.out.app = app
	b.out.win = win
}

func (b *theViewBuilder) BuildInput() *fyne.Container {
	pattern := widget.NewEntry()
	pattern.SetPlaceHolder(b.res("main_input_pattern"))
	pattern.Validator = func(text string) (err error) {
		_, err = regexp.Compile(text)
		return
	}

	template := widget.NewEntry()
	template.SetPlaceHolder(b.res("main_input_template"))

	rename := widget.NewButton(b.res("main_rename"), func() {})

	return container.NewBorder(
		nil, nil, nil, rename,
		container.NewVBox(
			pattern,
			template,
		),
	)
}

func (b *theViewBuilder) BuildTable() (t *widget.Table) {
	t = widget.NewTable(
		func() (int, int) { return 1, 3 },
		func() fyne.CanvasObject {
			return widget.NewLabel(strings.Repeat("1", 12))
		},
		func(i widget.TableCellID, o fyne.CanvasObject) {
			switch i.Col {
			case 0:
				o.(*widget.Label).SetText("1")
			case 1:
				o.(*widget.Label).SetText("short")
			case 2:
				o.(*widget.Label).SetText("long long long text")
			}
		})
	return
}
