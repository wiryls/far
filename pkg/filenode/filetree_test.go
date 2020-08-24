package filenode

// func TestFileTree(t *testing.T) {
// 	assert := assert.New(t)

// 	{ // empty
// 		tree := NewFileTree()
// 		assert.Empty(tree.Name())
// 		assert.Empty(tree.Path())
// 		assert.Empty(tree.Dir())
// 	}

// 	{ // root ops
// 		tree := NewFileTree()
// 		node, ok := tree.Put("")
// 		assert.Equal(&tree.FileNode, node)
// 		assert.True(ok)

// 		all := tree.All()
// 		assert.Len(all, 1)
// 		assert.Equal(&tree.FileNode, all[0])

// 		tree.FileNode.Inactive()
// 		assert.Len(tree.All(), 0)

// 		// again
// 		node, ok = tree.Put("")
// 		assert.Equal(&tree.FileNode, node)
// 		assert.True(ok)

// 		all = tree.All()
// 		assert.Len(all, 1)
// 		assert.Equal(&tree.FileNode, all[0])

// 		node = tree.Move(`Z:\Program Files`, ``)
// 		assert.Equal(&tree.FileNode, node)

// 		tree.Inactive()
// 		assert.Len(tree.All(), 0)
// 	}

// 	{ // simple put and pop
// 		tree := NewFileTree()
// 		node, ok := tree.Put("")
// 		assert.Equal(&tree.FileNode, node)
// 		assert.True(ok)

// 		node, ok = tree.Put(`C:\Program Files (x86)\GOG Galaxy`)
// 		assert.Equal(`C:\Program Files (x86)\GOG Galaxy`, node.Path())
// 		assert.True(ok)

// 		list := []string{
// 			`C:\Program Files\7-Zip`,
// 			`C:\Program Files (x86)\Steam`,
// 			`C:\Program Files\Git`,
// 			`D:\Workspace\go\bin\rsrc.exe`,
// 			`D:\Workspace\go`,
// 		}

// 		some := []*FileNode{}
// 		for _, name := range list {
// 			node, _ = tree.Put(name)
// 			some = append(some, node)
// 		}

// 		for i := range some {
// 			assert.Equal(list[i], some[i].Path())
// 			assert.Equal(nodeActive, some[i].stat)
// 		}

// 		assert.Len(tree.All(), len(list)+2)

// 		some[4].purge()
// 		assert.Empty(some[4].Path())
// 		assert.Empty(some[3].Path())

// 		assert.Equal(some[4].stat, nodeInvalid)
// 		assert.Equal(some[3].stat, nodeInvalid)

// 		assert.Len(tree.All(), len(list))
// 	}

// 	{ //TODO: more test needed.

// 	}
// }
