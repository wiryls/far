package main

import (
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
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

func BuildMainWindow(main fyne.Window) error {
	hello := widget.NewLabel("Hello Fyne!")
	main.SetContent(container.NewVBox(
		hello,
		widget.NewButton("Hi!", func() {
			hello.SetText("Welcome :)")
		}),
	))
	return nil
}
