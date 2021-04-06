package main

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

func BuildMainWindow(interface{}) error {
	return nil
}
