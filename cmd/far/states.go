package main

/////////////////////////////////////////////////////////////////////////////

type farViewModelAction interface {
	// button
	SetPattern(text string) (farViewModelAction, error)
	SetTemplate(text string) (farViewModelAction, error)
	Rename() (farViewModelAction, error)

	// menu
	ResetRows() (farViewModelAction, error)
	Delete(indexes []int) (farViewModelAction, error)
	Import(paths []string, recursive bool) (farViewModelAction, error)

	// tableview
	Sort(col int, isAscending bool) error
	RowCount() int
	Value(row, col int) interface{}
}
