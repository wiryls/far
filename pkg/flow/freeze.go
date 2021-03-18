package flow

// New create a flow to process some tasks.
func NewFreeze() *Freeze {
	return &Freeze{}
}

// Freeze groups something with a cancellable.
type Freeze struct {
}
