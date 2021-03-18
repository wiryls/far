package far

import "bytes"

// DiffType shows how the text changed.
type DiffType byte

// There are three `DiffType`: retain, insert, Delete.
// Indicate how the text should be changed.
const (
	DiffRetain DiffType = iota
	DiffInsert
	DiffDelete
)

// Diff indicates whether it matches and how it matches.
//  - `Type` is a `DiffType` of this `Text`.
//  - `Text` is a plain result.
type Diff struct {
	Type DiffType
	Text string
}

// Diffs is a list of Diff.
type Diffs []Diff

// String format of this diffs.
func (ds Diffs) String() string {
	return ds.New()
}

// New string.
func (ds Diffs) New() string {
	o := bytes.Buffer{}
	for d := range ds {
		d := &ds[d]
		switch d.Type {
		case DiffRetain, DiffInsert:
			o.WriteString(d.Text)
		}
	}
	return o.String()
}

// Old string.
func (ds Diffs) Old() string {
	o := bytes.Buffer{}
	for d := range ds {
		d := &ds[d]
		switch d.Type {
		case DiffRetain, DiffDelete:
			o.WriteString(d.Text)
		}
	}
	return o.String()
}

// IsSame returns whether there is no difference between old and new string.
func (ds Diffs) IsSame() bool {
	switch {
	case len(ds) == 0:
		fallthrough
	case len(ds) == 1 && ds[0].Type == DiffRetain:
		return true
	default:
		return false
	}
}
