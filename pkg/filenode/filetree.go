package filenode

// // NewFileTree is a collection of FileNode.
// func NewFileTree() FileTree {
// 	tree := new(defaultFileTree)
// 	tree.root = tree
// 	return tree
// }

// type defaultFileTree struct {
// 	defaultFileNode
// 	lock sync.RWMutex
// }

// func (t *defaultFileTree) Get(path string) FileNode {
// 	defer t.lock.RUnlock()
// 	/*_*/ t.lock.RLock()

// 	if node, ok := t.defaultFileNode.find(Splite(path)); ok {
// 		return node
// 	}
// 	return nil
// }

// func (t *defaultFileTree) Put(path string) (FileNode, bool) {
// 	defer t.lock.Unlock()
// 	/*_*/ t.lock.Lock()

// 	node, _ := t.findOrMake(Splite(path))
// 	ok := node.stat != nodeActive
// 	node.stat = nodeActive
// 	return node, ok
// }

// func (t *defaultFileTree) Move(old, new string) (FileNode, bool) {
// 	defer t.lock.Unlock()
// 	/*_*/ t.lock.Lock()

// 	// check if lhs
// 	lhs := Splite(old)
// 	rhs := Splite(new)
// 	if len(lhs) < len(rhs) {
// 		hit := false

// 		for i := range lhs {
// 			hit = hit || lhs[i] != rhs[i]
// 		}

// 		if hit == false {
// 			panic("cannot move `" + old + "` to `" + new + "`")
// 		}
// 	}

// 	src, swap := t.find(lhs)
// 	dst, override := t.findOrMake(rhs)

// 	switch {
// 	case src == dst:
// 		// no changes

// 	case swap && src.owner != nil:
// 		// fast swap and purge old
// 		src.owner.child[src.name], dst.owner.child[dst.name] = dst, src
// 		src.owner, dst.owner = dst.owner, src.owner
// 		src.name, dst.name = dst.name, src.name
// 		src.stat, dst.stat = dst.stat, src.stat

// 		src, dst = dst, src
// 		src.purge()

// 	case swap:
// 		// swap child and purge old
// 		dst.cut()
// 		dst.child, src.child = src.child, nil
// 		for _, node := range dst.child {
// 			node.owner = dst
// 		}
// 		src.purge()

// 	case override:
// 		// clean dst
// 		dst.cut()
// 	}

// 	dst.stat = nodeActive
// 	return dst
// }

// func (t *defaultFileTree) List() []FileNode {
// 	defer t.lock.RUnlock()
// 	/*_*/ t.lock.RLock()

// 	var nodes []FileNode
// 	var visit func(n *defaultFileNode)
// 	visit = func(n *defaultFileNode) {
// 		if n == nil {
// 			return
// 		}

// 		if n.stat == nodeActive {
// 			nodes = append(nodes, n)
// 		}

// 		for _, node := range n.child {
// 			visit(node)
// 		}
// 	}
// 	visit(&t.defaultFileNode)

// 	return nodes
// }
