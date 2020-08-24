package filenode

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

func (n *fileNode) createChildNode(name string) (it fileNodeCore) {

	it = &fileNode{
		fileName:   name,
		rootNode:   n.rootNode,
		parentNode: n.parentNode,
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

// func (n *fileNode) Name() string {
// 	return ""
// }

// func (n *fileNode) Path() string {
// 	return filepath.Join(n.ancestors()...)
// }

// func (n *fileNode) Dir() string {
// 	path := n.ancestors()
// 	if n := len(path); n != 0 {
// 		path = path[:n-1]
// 	}

// 	return filepath.Join(path...)
// }

// func (n *fileNode) DirAndName() (dir string, name string) {
// 	path := n.ancestors()
// 	if n := len(path); len(path) != 0 {
// 		name = path[n-1]
// 		dir = filepath.Join(path[:n-1]...)
// 	}

// 	return
// }

// func (n *fileNode) IsUsing() bool {
// 	return n.using
// }

// func (n *fileNode) Parent() FileNode {
// 	return n.owner
// }

// func (n *fileNode) Get(path string) FileNode {
// 	return nil
// }

// func (n *fileNode) GetNodesInUse() []FileNode {
// 	return nil
// }

// func (n *fileNode) Put(path string) (FileNode, bool) {
// 	return nil, false
// }

// func (n *fileNode) Move(old, new string) (FileNode, bool) {
// 	return nil, false
// }

// func (n *fileNode) Remove(path string) bool {
// 	return false
// }

// func (n *fileNode) Discard() {
// 	n.inactive()
// }

// Note; defaultFileNode:
// All public methods should use lock to ensure concurrent safe.
// All private methods should NOT use lock, they will be used by public
// methods.
