package free

/// READONLY ///

// get names of all ancestor nodes (includes the node itself).
func nameOfAncestors(node fileNodeGetter) []string {

	path := []string{}
	root := fileNodeGetter(node.root())
	for ; node != root; node = node.parent() {
		if node == nil {
			path = path[:]
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

// don't modify during visiting.
func visitAllChildNodes(node fileNodeCore, f func(fileNodeGetter) bool) {
	node.traverseChildNodes(func(_ string, node fileNodeCore) (goon bool) {
		if goon = f(node); goon {
			visitAllChildNodes(node, f)
		}
		return
	})
}

// find a node based on current path.
//
//  - the node of divergence if not found.
//  - itself if path is nil or empty.
//  - nil if node is empty.
func findNodeByPath(node fileNodeCore, path []string) (fileNodeCore, []string) {

	switch {

	case len(path) == 0:
		path = nil

	case node == nil || node.root() == nil:
		node = nil

	default:
		root := node.root()
		next := node
		for len(path) != 0 && next != nil {
			switch {
			case path[0] == "." || len(path[0]) == 0:
				// do nothing
			case path[0] == "..":
				next = node.parent()
			default:
				next = node.child(path[0])
			}

			switch {
			case next == nil:
				// do nothing
			case next != root:
				node = next
				fallthrough
			default:
				path = path[1:]
			}
		}
	}

	return node, path
}

/// READWRITE ///

// find a node or make it if not found.
// return the node and isFound.
func findOrMakeNodeByPath(node fileNodeCore, path []string) (fileNodeCore, bool) {

	node, path = findNodeByPath(node, path)

	switch {
	case node == nil || isStringInSet(".", path) || isStringInSet("..", path):
		return nil, false

	case len(path) == 0:
		return node, true

	default:
		for _, name := range path {
			node = node.createChildNode(name)
		}
		node.setUsing(true)
		return node, false
	}
}

func renameNode(node fileNodeCore, name string) bool {
	n := node.name()
	p := node.parent()

	node.setName(name)
	ok := p.putChildNode(node)
	if !ok {
		node.setName(n)
	} else {
		p.deleteChildNode(n)
	}

	return ok
}

func disuseThisNode(node fileNodeCore) {
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

func removeNode(node fileNodeCore) {
	removeAllChildNodes(node)
	disuseThisNode(node)
}

func swapNodes(l, r fileNodeCore) bool {
	if l == nil || r == nil || l == r ||
		l.root() == nil || l.root() != r.root() ||
		l.root() == l || r.root() == r {
		return false
	}

	lp, rp := l.parent(), r.parent()
	lp.deleteChildNode(l.name())
	rp.deleteChildNode(r.name())

	{
		using := l.using()
		l.setUsing(r.using())
		r.setUsing(using)
	}
	{
		name := l.name()
		l.setName(r.name())
		r.setName(name)
	}

	l.setParent(rp)
	r.setParent(lp)
	return lp.putChildNode(r) && rp.putChildNode(l)
}
