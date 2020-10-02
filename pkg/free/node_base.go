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

func (n *fileNode) name() string {
	return n.fileName
}

func (n *fileNode) using() bool {
	return n.isUsing
}

func (n *fileNode) root() fileNodeCore {
	return n.rootNode
}

func (n *fileNode) parent() fileNodeCore {
	return n.parentNode
}

func (n *fileNode) child(name string) fileNodeCore {
	if n.childNodes == nil {
		return nil
	}

	if node, find := n.childNodes[name]; find {
		return node
	}

	return nil
}

func (n *fileNode) countChildNodes() int {
	return len(n.childNodes)
}

func (n *fileNode) traverseChildNodes(f func(string, fileNodeCore) bool) {
	for name, node := range n.childNodes {
		if !f(name, node) {
			return
		}
	}
}

/// SETTERS ///

func (n *fileNode) setName(name string) {
	n.fileName = name
}

func (n *fileNode) setUsing(using bool) {
	n.isUsing = using
}

func (n *fileNode) setParent(parent fileNodeCore) {
	n.parentNode = parent
}

func (n *fileNode) putChildNode(m fileNodeCore) bool {
	if m != nil && n != nil && m.root() == n.root() {

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

	it = &fileNode{
		fileName:   name,
		rootNode:   n.rootNode,
		parentNode: n,
	}

	if n.childNodes == nil {
		n.childNodes = make(map[string]fileNodeCore)
	}

	n.childNodes[name] = it
	return
}

func (n *fileNode) deleteChildNode(name string) {
	// it's ok to delete nil map.
	delete(n.childNodes, name)
}

func (n *fileNode) deleteChildNodes() {
	n.childNodes = nil
}
