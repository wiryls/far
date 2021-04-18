package fall

// Input is string.
type Input = string

// Output is a pair of Input and Differ result.
type Output struct {
	Path string
	Name string
	Next string
	View string
	Diff bool
}

// Callback is fired when output arrives.
type Callback func(os []*Output)
