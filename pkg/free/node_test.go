package free

import (
	"math/rand"
	"runtime"
	"sync"
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestFileTree(t *testing.T) {
	assert := assert.New(t)

	{ // empty
		root := NewFileTree()
		assert.Empty(root.Name())
		assert.Empty(root.Path())
		assert.Empty(root.Dir())
	}

	{ // root ops
		root := NewFileTree()
		node, ok := root.Put("")

		assert.Equal(root.Path(), node.Path())
		assert.Equal(root.Name(), node.Name())
		assert.Equal(root.Dir(), node.Dir())
		assert.False(ok)

		// again
		node, ok = root.Put("")
		assert.Equal(root.Path(), node.Path())
		assert.Equal(root.Name(), node.Name())
		assert.Equal(root.Dir(), node.Dir())
		assert.False(ok)

		node = root.Find("")
		assert.Equal(root.Path(), node.Path())
		assert.Equal(root.Name(), node.Name())
		assert.Equal(root.Dir(), node.Dir())

		move := root.Move("")
		assert.Equal(MoveResultNodeUnmovable, move)

		move = root.Move(`C:\Program Files (x86)`)
		assert.Equal(MoveResultNodeUnmovable, move)
	}

	{ // simple put and pop
		root := NewFileTree()

		// put
		for i, o := range []struct {
			Path      string
			IsNodeNil bool
			IsCreated bool
			CouldFind bool
		}{
			{`D:\Workspace\go\bin\rsrc.exe`, false, true, true},
			{`C:\Program Files\7-Zip`, false, true, true},
			{`C:\Program Files\Git`, false, true, true},
			{`D:\Workspace\go`, false, false, true},
			{`C:\Program Files (x86)\GOG Galaxy`, false, true, true},
			{`.\..\X`, true, false, false},
		} {
			node, created := root.Put(o.Path)
			assert.Equal(o.IsNodeNil, node == nil, "case #%d", i)
			assert.Equal(o.IsCreated, created, "case #%d", i)
			assert.Equal(o.CouldFind, root.Find(o.Path) != nil, "case #%d", i)
		}

		// pop
		{
			node := root.Find(`C:\Program Files\Git`)
			assert.NotNil(node)
			node.Disuse()

			assert.Nil(root.Find(`C:\Program Files\Git`))

			node = root.Find(`C:\Program Files`)
			assert.Equal("Program Files", node.Name())
			assert.Equal(`C:\Program Files`, node.Path())
			assert.NotNil(node)
			node.Disuse()

			assert.NotNil(root.Find(`C:\Program Files`))

			node = root.Find(`C:\Program Files\7-Zip`)
			assert.NotNil(node)
			node.Disuse()

			assert.Nil(root.Find(`C:\Program Files`))

			assert.NotNil(root.Find(`C:\`))
			assert.NotNil(root.Find(`C:\Program Files (x86)`))
			assert.NotNil(root.Find(`C:\Program Files (x86)\GOG Galaxy`))

			root.Find(`C:\Program Files (x86)\GOG Galaxy`).Disuse()
			assert.Nil(root.Find(`C:\`))

			assert.NotNil(root.Find(`D:\Workspace\go\bin\rsrc.exe`))
			assert.NotNil(root.Find(`D:\Workspace\go`))

			root.Find(`D:\Workspace\go\bin\rsrc.exe`).Disuse()
			assert.Nil(root.Find(`D:\Workspace\go\bin\rsrc.exe`))
			assert.NotNil(root.Find(`D:\Workspace\go`))
		}
	}

	{ // TODO: more test needed.

	}
}

func TestNodeMove(t *testing.T) {
	assert := assert.New(t)

	// {
	// 	root := NewFileTree()
	// 	assert.NotNil(root)

	// 	foo, create := root.Put("X:\\opt\\foo\\")
	// 	assert.NotNil(foo)
	// 	assert.True(create)

	// 	node, create := root.Put("X:\\opt\\node\\")
	// 	assert.NotNil(node)
	// 	assert.True(create)

	// 	result := node.Move("X:\\opt\\node\\what")
	// 	assert.Equal(MoveResultTargetIsChild, result)

	// 	result = node.Move("X:\\opt\\foo")
	// 	assert.Equal(MoveResultTargetExists, result)

	// 	result = node.Move("X:\\opt\\foo\\bar")
	// 	assert.Equal(MoveResultDone, result)

	// 	assert.Equal("X:\\opt\\foo\\bar", node.Path())
	// 	assert.Equal("bar", node.Name())

	// 	result = node.Move("Y:\\")
	// 	assert.Equal(MoveResultDone, result)
	// 	assert.Equal("Y:\\bar", node.Path())
	// }

	{
		root := NewFileTree()
		node, create := root.Put("X:\\opt\\foo\\")
		assert.NotNil(node)
		assert.True(create)

		what, create := node.Put("X:\\opt\\bar")
		assert.NotNil(what)
		assert.True(create)
		assert.Equal("X:\\opt\\bar", what.Path())

		result := node.Move("..\\node")
		assert.Equal(MoveResultDone, result)
		assert.Equal("X:\\node", node.Path())

		result = node.Move("node\\next")
		assert.Equal(MoveResultTargetIsChild, result)
		assert.Equal("X:\\node", node.Path())
	}
}

func TestFileNodeLock(t *testing.T) {
	assert := assert.New(t)

	TotalCases := 2000 + rand.Intn(20)*100
	GroupCount := TotalCases / 100
	PathLength := 32
	NameLength := 2
	Data := make([]string, 0, TotalCases)
	Set := []rune("XYZ")
	{
		for i := 0; i < TotalCases; i++ {
			name := makeRandomPath(NameLength, NameLength, PathLength, PathLength, Set)
			Data = append(Data, string(name))
		}
	}

	{
		root := NewFileTree()
		wait := &sync.WaitGroup{}
		mark := runtime.NumGoroutine()

		for i := 0; i < GroupCount; i++ {
			wait.Add(1)
			go func(i int) {
				defer wait.Done()

				for _, path := range Data[i*100 : (i+1)*100] {
					root.Put(path)
					assert.NotNil(root.Find(path))
				}
			}(i)
		}

		wait.Wait()
		assert.Equal(mark, runtime.NumGoroutine())
	}
}
