package free

/////////////////////////////////////////////////////////////////////////////

type fileNode struct {
	isUsing    bool
	fileName   string
	rootNode   fileNodeCore
	parentNode fileNodeCore
	childNodes map[string]fileNodeCore // should never contains nil node.
}

/// GETTERS ///

func (n *fileNode) name() (r string) {
	if n != nil {
		r = n.fileName
	}
	return
}

func (n *fileNode) using() (r bool) {
	if n != nil {
		r = n.isUsing
	}
	return
}

func (n *fileNode) root() (r fileNodeCore) {
	if n != nil {
		r = n.rootNode
	}
	return
}

func (n *fileNode) parent() (r fileNodeCore) {
	if n != nil {
		r = n.parentNode
	}
	return
}

func (n *fileNode) child(name string) (r fileNodeCore) {
	if n != nil && n.childNodes != nil {
		if node, find := n.childNodes[name]; find {
			r = node
		}
	}
	return
}

func (n *fileNode) countChildNodes() (r int) {
	if n != nil {
		r = len(n.childNodes)
	}
	return
}

func (n *fileNode) traverseChildNodes(f func(string, fileNodeCore) bool) {
	if n != nil && f != nil {
		for name, node := range n.childNodes {
			if !f(name, node) {
				return
			}
		}
	}
}

/// SETTERS ///

func (n *fileNode) setName(name string) {
	if n != nil {
		n.fileName = name
	}
}

func (n *fileNode) setUsing(using bool) {
	if n != nil {
		n.isUsing = using
	}
}

func (n *fileNode) setParent(parent fileNodeCore) {
	if n != nil {
		n.parentNode = parent
	}
}

func (n *fileNode) putChildNode(m fileNodeCore) bool {
	if n != nil && m != nil && n.root() == m.root() {
		if n.childNodes == nil {
			n.childNodes = map[string]fileNodeCore{}
		}

		if _, ok := n.childNodes[m.name()]; !ok {
			n.childNodes[m.name()] = m
			return true
		}
	}

	return false
}

func (n *fileNode) createChildNode(name string) (it fileNodeCore) {
	if n != nil {
		it = &fileNode{
			fileName:   name,
			rootNode:   n.rootNode,
			parentNode: n,
		}

		if n.childNodes == nil {
			n.childNodes = make(map[string]fileNodeCore)
		}

		n.childNodes[name] = it
	}
	return
}

func (n *fileNode) deleteChildNode(name string) {
	if n != nil {
		// it's ok to delete nil map.
		delete(n.childNodes, name)
	}

}

func (n *fileNode) deleteChildNodes() {
	if n != nil {
		n.childNodes = nil
	}
}
