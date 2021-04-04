package far

// Far is to Find and Replace.
type Far interface {
	// See if the text matches.
	See(text string) Diffs

	// is it empty.
	Empty() bool

	// Get pattern.
	Pattern() string

	// Get template.
	Template() string

	// Set the pattern to search.
	SetPattern(text string) error

	// Set the template to fill.
	SetTemplate(text string) error
}
