package far

import (
	"regexp"
	"sync"
)

// Far is to Find and Replace.
type Far interface {
	// See if the text matches.
	See(text string) Diffs
	// Get pattern.
	Pattern() string
	// Get template.
	Template() string
	// Set the pattern to search.
	SetPattern(text string) error
	// Set the template to fill.
	SetTemplate(text string) error
}

// New regex finder and replacer.
func New() Far {
	return &faregex{pattern: nil, template: ""}
}

type faregex struct {
	// As regexp.Regexp is concurrent safe, we use RWMutex.
	// See: https://golang.org/pkg/regexp/#Regexp
	lock     sync.RWMutex
	pattern  *regexp.Regexp
	template string
}

func (f *faregex) See(text string) Diffs {
	defer f.lock.RUnlock()
	/*_*/ f.lock.RLock()

	diff := Diffs(nil)

	if f.pattern != nil {

		i := 0
		for _, m := range f.pattern.FindAllStringSubmatchIndex(text, -1) {
			if len(m) < 2 {
				continue // should never reach
			}

			ret := text[i:m[0]]
			del := text[m[0]:m[1]]
			ins := string(f.pattern.ExpandString(nil, f.template, text, m))

			if del == ins {
				continue
			}
			if len(ret) != 0 {
				diff = append(diff, Diff{Type: DiffRetain, Text: ret})
			}
			if len(del) != 0 {
				diff = append(diff, Diff{Type: DiffDelete, Text: del})
			}
			if len(ins) != 0 {
				diff = append(diff, Diff{Type: DiffInsert, Text: ins})
			}

			i = m[1]
		}

		if i != len(text) {
			diff = append(diff, Diff{Type: DiffRetain, Text: text[i:]})
		}
	}

	if diff == nil {
		diff = Diffs{{Type: DiffRetain, Text: text}}
	}

	return diff
}

func (f *faregex) Pattern() string {
	defer f.lock.RUnlock()
	/*_*/ f.lock.RLock()

	return f.pattern.String()
}

func (f *faregex) Template() string {
	defer f.lock.RUnlock()
	/*_*/ f.lock.RLock()

	return f.template
}

func (f *faregex) SetPattern(text string) (err error) {
	defer f.lock.Unlock()
	/*_*/ f.lock.Lock()

	var pat *regexp.Regexp
	pat, err = regexp.Compile(text)
	if err == nil {
		f.pattern = pat
	}
	return
}

func (f *faregex) SetTemplate(text string) (err error) {
	defer f.lock.Unlock()
	/*_*/ f.lock.Lock()

	f.template = text
	return
}
