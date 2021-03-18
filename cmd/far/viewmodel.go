package main

import (
	"errors"

	"github.com/lxn/walk"
)

// Errors of this application.
var (
	ErrSeparatorInTemplate = errors.New("separator should not be here")
	ErrInvalidPattern      = errors.New("pattern is invalid")
	ErrUnavailable         = errors.New("method unavailable")
)

/////////////////////////////////////////////////////////////////////////////

func makeViewModel() *farViewModel {
	return &farViewModel{
		texts: texts{},
	}
}

type farViewModel struct {
	// view model
	walk.TableModelBase
	walk.SorterBase

	// resources
	texts texts

	// data
	isRenamable    bool
	isInputable    bool
	patternNotice  string
	templateNotice string
}

// Sort is called by the TableView to sort the items.
// [writable]
func (m *farViewModel) Sort(col int, order walk.SortOrder) (err error) {
	return
}

// RowCount counts the number of rows. Called by the TableView from SetModel
// and every time the model publishes a RowsReset event.
// [readonly]
func (m *farViewModel) RowCount() int {
	return 0
}

// Value is the (row, column) elements of the table. Used by TableView.
// [readonly]
func (m *farViewModel) Value(row, col int) interface{} {
	return 0
}

// StyleCell defines style of each cells.
// [readonly]
func (m *farViewModel) StyleCell(style *walk.CellStyle) {
	// TODO:
}
