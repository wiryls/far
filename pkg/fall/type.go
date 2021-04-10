package fall

import "github.com/wiryls/far/pkg/far"

// Input is string.
type Input = string

// Output is a pair of Input and Differ result.
type Output struct {
	Source Input
	Differ far.Diffs
}

// Callback is fired when output arrives.
type Callback func(os []Output)
