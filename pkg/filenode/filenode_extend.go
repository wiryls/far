package filenode

/// READONLY ///

// get names of all ancestor nodes (includes the node itself).
func nameOfAncestors(node fileNodeGetter) []string {

	path := []string{}
	root := fileNodeGetter(node.root())
	for ; node != root; node = node.parent() {
		if node == nil {
			path = nil
			break
		}
		path = append(path, node.name())
	}

	k := len(path)
	for i := k/2 - 1; i >= 0; i-- {
		j := k - 1 - i
		path[i], path[j] = path[j], path[i]
	}

	return path
}

func visitAllChildNodes(node fileNodeCore, f func(fileNodeGetter) bool) {
	node.traverseChildNodes(func(_ string, node fileNodeCore) (goon bool) {
		if goon = f(node); goon {
			visitAllChildNodes(node, f)
		}
		return
	})
}

// find a node by path.
//  - itself if path is nil or empty.
//  - nil if not found.
func findNodeByPath(node fileNodeCore, path []string) fileNodeCore {

	for _, name := range path {
		node = node.child(name)
		if node == nil {
			break
		}
	}

	return node
}

/// READWRITE ///

// find a node or create it if not found.
func findOrCreateNodeByPath(node fileNodeCore, path []string) (
	it fileNodeCore, find bool,
) {

	it, find = node, true
	for _, name := range path {
		var next fileNodeCore

		if find {
			next = it.child(name)
			find = next != nil
		}

		if find {
			it = next
		} else {
			it = it.createChildNode(name)
		}
	}

	if !find {
		it.setUsing(true)
	}

	return
}

func noLongerUseThisNode(node fileNodeCore) {
	// mark this node won't be used any more.
	node.setUsing(false)

	// detach from parents.
	for node != nil && !node.using() && node.countChildNodes() == 0 {
		next := node.parent()
		if next != nil {
			next.deleteChildNode(node.name())
		}

		node.setName("")
		node.setParent(nil)
		node.deleteChildNodes()
		node = next
	}
}

func removeAllChildNodes(node fileNodeCore) {
	node.traverseChildNodes(func(_ string, it fileNodeCore) bool {
		removeAllChildNodes(it)
		it.setName("")
		it.setUsing(false)
		it.setParent(nil)
		it.deleteChildNodes()
		return true
	})
	node.deleteChildNodes()
}

func deleteNode(node fileNodeCore) {
	removeAllChildNodes(node)
	noLongerUseThisNode(node)
}
