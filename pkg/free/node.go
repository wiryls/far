package free

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
)

// NewFileTree create a root FileNode.
func NewFileTree() FileNode {
	tree := &wrapper{lock: &sync.RWMutex{}}
	tree.fileNodeCore = &fileNode{isUsing: true, rootNode: tree}
	return tree
}

/////////////////////////////////////////////////////////////////////////////

type wrapper struct {
	lock *sync.RWMutex
	fileNodeCore
}

/// other ///

func (n *wrapper) toFileNode(core fileNodeCore) FileNode {
	if node, ok := core.(FileNode); ok {
		return node
	}
	return &wrapper{lock: n.lock, fileNodeCore: core}
}

/// override ///

func (n *wrapper) createChildNode(name string) (it fileNodeCore) {

	it = &wrapper{
		lock: n.lock,
		fileNodeCore: &fileNode{
			fileName:   name,
			rootNode:   n.root(),
			parentNode: n,
		},
	}

	n.putChildNode(it)
	return
}

// Note; wrapper:
// All public methods should use lock to ensure concurrent safe.
// All private methods should NOT use lock, they will be used by public
// methods.

/// GETTERS ///

func (n *wrapper) Name() string {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	return n.name()
}

func (n *wrapper) Path() string {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	return filepath.Join(nameOfAncestors(n)...)
}

func (n *wrapper) Dir() string {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	path := nameOfAncestors(n)
	if n := len(path); n != 0 {
		path = path[:n-1]
	}

	return filepath.Join(path...)
}

func (n *wrapper) DirAndName() (dir string, name string) {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	path := nameOfAncestors(n)
	if n := len(path); len(path) != 0 {
		name = path[n-1]
		dir = filepath.Join(path[:n-1]...)
	}

	return
}

func (n *wrapper) Find(path string) FileNode {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	core, rest := findNodeByPath(n, splite(path))
	if core != nil && len(rest) == 0 {
		return n.toFileNode(core)
	}

	return nil
}

/// SETTERS ///

func (n *wrapper) Put(path string) (node FileNode, created bool) {
	/*_*/ n.lock.Lock()
	defer n.lock.Unlock()

	core, find := findOrMakeNodeByPath(n, splite(path))
	if core != nil {
		core.setUsing(true)
		node = n.toFileNode(core)
		created = !find
	}

	return
}

func (n *wrapper) Move(path string) MoveResult {
	/*_*/ n.lock.Lock()
	defer n.lock.Unlock()

	isdir := strings.HasSuffix(path, string(os.PathSeparator))
	if !filepath.IsAbs(path) {
		vec := append(nameOfAncestors(n), "..", path)
		path = filepath.Join(vec...)
	}
	return moveNode(n, splite(path), isdir)
}

func (n *wrapper) Disuse() {
	/*_*/ n.lock.Lock()
	defer n.lock.Unlock()

	disuseThisNode(n)
}
