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
		state: nil,
	}
}

type farViewModel struct {
	// view model
	walk.TableModelBase
	walk.SorterBase

	// context
	texts texts
	state farViewModelAction

	// data
}

// SetPattern change the pattern of the FAR.
// [writable]
func (m *farViewModel) SetPattern(le *walk.LineEdit) {
	var err error
	m.state, err = m.state.SetPattern(le.Text())

	switch err {
	case nil:
		break

	case ErrUnavailable:
		break

	case ErrSeparatorInTemplate:
		le.SetToolTipText(m.texts.PatternIsInvalid())

	default:
		// create a error dialog.
		break
	}
}

// SetTemplate change the template of the FAR.
// [writable]
func (m *farViewModel) SetTemplate(le *walk.LineEdit) {
	var err error
	m.state, err = m.state.SetPattern(le.Text())

	switch err {
	case nil:
		break

	case ErrUnavailable:
		break

	case ErrSeparatorInTemplate:
		le.SetToolTipText(m.texts.TemplateIsInvalid())

	default:
		// create a error dialog.
		break
	}
}

// Rename all matched.
// [writable]
func (m *farViewModel) Rename() {
	var err error
	m.state, err = m.state.Rename()

	switch err {
	case nil:
		break

	case ErrUnavailable:
		break

	default:
		// create a error dialog.
		break
	}
}

// ResetRows clears all.
// [writable]
func (m *farViewModel) ResetRows() {
	var err error
	m.state, err = m.state.ResetRows()

	switch err {
	case nil:
		break

	case ErrUnavailable:
		break

	default:
		// create a error dialog.
		break
	}
}

// Import some filepaths.
// [writable]
func (m *farViewModel) Import(paths []string, recursive bool) {
	var err error
	m.state, err = m.state.Import(paths, recursive)

	switch err {
	case nil:
		break

	case ErrUnavailable:
		break

	default:
		// create a error dialog.
		break
	}
}

// Delete items from list.
func (m *farViewModel) Delete(indexes []int) {
	var err error
	m.state, err = m.state.Delete(indexes)

	switch err {
	case nil:
		break

	case ErrUnavailable:
		break

	default:
		// create a error dialog.
		break
	}
}

// Sort is called by the TableView to sort the items.
// [writable]
func (m *farViewModel) Sort(col int, order walk.SortOrder) (err error) {
	err = m.state.Sort(col, order == walk.SortAscending)
	return
}

// RowCount counts the number of rows. Called by the TableView from SetModel
// and every time the model publishes a RowsReset event.
// [readonly]
func (m *farViewModel) RowCount() int {
	return m.state.RowCount()
}

// Value is the (row, column) elements of the table. Used by TableView.
// [readonly]
func (m *farViewModel) Value(row, col int) interface{} {
	return m.state.Value(row, col)
}

// StyleCell defines style of each cells.
// [readonly]
func (m *farViewModel) StyleCell(style *walk.CellStyle) {
	// TODO:
}
