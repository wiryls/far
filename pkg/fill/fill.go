package fill

import (
	"os"
	"path/filepath"

	"github.com/wiryls/pkg/flow"
)

func New(output func([]string), filter func(string) bool) *Fill {
	if filter == nil {
		filter = func(s string) bool { return true }
	}
	if output == nil {
		output = func(s []string) {}
	}
	return &Fill{
		output: output,
		filter: filter,
		flow:   flow.Flow{},
	}
}

// Fill records.
type Fill struct {
	filter func(string) bool
	output func([]string)
	flow   flow.Flow
}

// Fill start a task to walk through filepath and fetch all valid.
func (f *Fill) Fill(input []string) {
	f.flow.Push((&importer{
		Source: input,
		Splite: func(i int) int {
			if i < 256 {
				return i
			}
			return 256
		},
		Action: func(path string) (string, bool) {
			var err error
			if err == nil {
				path, err = filepath.Abs(path)
			}
			var has bool
			if err == nil {
				has = f.filter(path)
			}
			if has && err == nil {
				_, err = os.Stat(path)
			}
			return path, err == nil && has
		},
		Output: f.output,
		Runner: f.flow.Push,
	}).Run)
}

type importer struct {
	Source []string
	Splite func(int) int
	Action func(string) (string, bool)
	Output func([]string)
	Runner func(func())
}

func (m *importer) Run() {
	if m == nil ||
		m.Source == nil ||
		m.Splite == nil ||
		m.Action == nil ||
		m.Output == nil ||
		m.Runner == nil {
		return
	}

	todo := m.Source
	if size := m.Splite(len(m.Source)); 0 <= size && size < len(m.Source) {
		another := *m
		another.Source, todo = m.Source[size:], m.Source[:size]
		another.Runner(another.Run)
	}

	result := make([]string, 0, len(todo))
	for _, input := range todo {
		if output, ok := m.Action(input); ok {
			result = append(result, output)
		}
	}

	if len(result) > 0 {
		m.Output(result)
	}
}
