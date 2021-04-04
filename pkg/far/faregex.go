package far

import "regexp"

// Faregex is a finder and replacer base on regexp.
type Faregex struct {
	pattern  *regexp.Regexp
	template string
}

func (f *Faregex) See(text string) (diff Diffs) {

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

func (f *Faregex) Empty() bool {
	return f.pattern == nil
}

func (f *Faregex) Pattern() string {
	if f.pattern != nil {
		return f.pattern.String()
	}
	return ""
}

func (f *Faregex) Template() string {
	return f.template
}

func (f *Faregex) SetPattern(text string) (err error) {
	var pat *regexp.Regexp
	if text == "" {
		pat = nil
	} else {
		pat, err = regexp.Compile(text)
	}

	if err == nil {
		f.pattern = pat
	}
	return
}

func (f *Faregex) SetTemplate(text string) (err error) {
	f.template = text
	return
}
