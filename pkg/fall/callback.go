package fall

type Callback interface {
	OnItemUpdate(index int)
	OnItemsUpdate(from, to int)
	OnItemsInsert(from, to int)
	OnItemsDelete(from, to int)
	OnItemsReset()
}

type doNothing struct{}

func (c *doNothing) OnItemUpdate(index int)     {}
func (c *doNothing) OnItemsUpdate(from, to int) {}
func (c *doNothing) OnItemsInsert(from, to int) {}
func (c *doNothing) OnItemsDelete(from, to int) {}
func (c *doNothing) OnItemsReset()              {}
