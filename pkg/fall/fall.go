package fall

import (
	"path/filepath"
	"strings"

	"github.com/wiryls/far/pkg/far"
	"github.com/wiryls/pkg/flow"
)

func New(callback Callback) *Fall {
	if callback == nil {
		callback = func(os []*Output) {}
	}

	return &Fall{
		call: callback,
		farr: &far.Farsafe{Inner: &far.Faregex{}},
		flow: &flow.Feed{Mark: 1},
	}
}

type Fall struct {
	call Callback
	farr far.Far
	flow *flow.Feed
}

// SetPattern change the far pattern.
func (f *Fall) SetPattern(text string) error {
	return f.farr.SetPattern(text)
}

// SetTemplate change the far template.
func (f *Fall) SetTemplate(text string) error {
	return f.farr.SetTemplate(text)
}

// Flow will append some inputs to current task.
func (f *Fall) Flow(source []Input) {
	f.flow.Push((&differ{
		Number: 0,
		Source: source,
		Splite: limited(1024),
		Action: func(src Input) *Output {
			if src == "" {
				return nil
			}

			name := filepath.Base(src)
			diff := f.farr.See(name)
			return &Output{
				Path: src,
				Name: name,
				Next: diff.New(),
				View: toMarkup(name, diff),
				Diff: !diff.IsSame() || f.farr.Empty(),
			}
		},
		Output: (&sequencer{Latest: 0, Output: f.call}).Collect,
		Runner: f.flow.Push,
		Runnin: 1,
	}).Run)
}

// Stop all current tasks. It will block until the last task stops.
func (f *Fall) Stop() {
	f.flow.Sync(0)
	f.flow.Wait()
	f.flow.Sync(1)
}

// Stop all current tasks. But do something extra during stopping.
func (f *Fall) StopWith(action func()) {
	f.flow.Sync(0)
	f.flow.Wait()

	if action != nil {
		action()
	}

	f.flow.Wait()
	f.flow.Sync(1)
}

// this replacer is copied from "html/htmlEscaper"
var theMarkupReplacer = strings.NewReplacer(
	`&`, "&amp;",
	`'`, "&#39;",
	`<`, "&lt;",
	`>`, "&gt;",
	`"`, "&#34;")

func toMarkup(name string, diff far.Diffs) string {
	// fast path
	if diff.IsSame() {
		// I use html.EscapeString as there is no g_markup_escape_text in gotk3
		return theMarkupReplacer.Replace(name)
	}

	// slow path
	b := strings.Builder{}
	for _, d := range diff {
		switch d.Type {
		case far.DiffInsert:
			_, _ = b.WriteString(`<span foreground="#007947">`)
			_, _ = theMarkupReplacer.WriteString(&b, d.Text)
			_, _ = b.WriteString("</span>")
		case far.DiffDelete:
			_, _ = b.WriteString(`<span foreground="#DC143C" strikethrough="true">`)
			_, _ = theMarkupReplacer.WriteString(&b, d.Text)
			_, _ = b.WriteString("</span>")
		default:
			_, _ = theMarkupReplacer.WriteString(&b, d.Text)
		}
	}

	// [Pango markup]
	// (https://developer.gnome.org/pango/1.46/pango-Markup.html)
	return b.String()
}
