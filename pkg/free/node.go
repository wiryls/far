package free

import (
	"os"
	"path/filepath"
	"strings"
	"sync"
)

// NewFileTree create a root FileNode.
func NewFileTree() FileNode {
	node := &fileNode{
		isUsing: true}
	node.rootNode = node
	node.parentNode = node
	tree := &wrapper{
		lock:         &sync.RWMutex{},
		fileNodeCore: node}
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

// Note; wrapper:
// All public methods should use lock to ensure concurrent safe.
// All private methods should NOT use lock, they will be used by public
// methods.

/// GETTERS ///

func (n *wrapper) Name() string {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	return n.fileNodeCore.name()
}

func (n *wrapper) Path() string {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	return filepath.Join(nameOfAncestors(n.fileNodeCore)...)
}

func (n *wrapper) Dir() string {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	path := nameOfAncestors(n.fileNodeCore)
	if n := len(path); n != 0 {
		path = path[:n-1]
	}

	return filepath.Join(path...)
}

func (n *wrapper) DirAndName() (dir string, name string) {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	path := nameOfAncestors(n.fileNodeCore)
	if n := len(path); len(path) != 0 {
		name = path[n-1]
		dir = filepath.Join(path[:n-1]...)
	}

	return
}

func (n *wrapper) Find(path string) FileNode {
	/*_*/ n.lock.RLock()
	defer n.lock.RUnlock()

	node := n.fileNodeCore
	if filepath.IsAbs(path) {
		node = node.root()
	}

	node, rest := findNodeByPath(node, splite(path))
	if node != nil && len(rest) == 0 {
		return n.toFileNode(node)
	}

	return nil
}

/// SETTERS ///

func (n *wrapper) Put(path string) (out FileNode, put bool) {
	/*_*/ n.lock.Lock()
	defer n.lock.Unlock()

	node := n.fileNodeCore
	if filepath.IsAbs(path) {
		node = node.root()
	}

	node, find := findOrMakeNodeByPath(node, splite(path))
	if node != nil {
		node.setUsing(true)
		out = n.toFileNode(node)
		put = !find
	}

	return
}

func (n *wrapper) Move(path string) (code MoveResult) {
	/*_*/ n.lock.Lock()
	defer n.lock.Unlock()

	switch {
	case n.fileNodeCore.root() == n.fileNodeCore:
		code = MoveResultNodeUnmovable
	case path == "":
		code = MoveResultTargetInvalid
	case path == ".":
		code = MoveResultDone
	case !strings.ContainsRune(path, filepath.Separator):
		if renameNode(n.fileNodeCore, path) {
			code = MoveResultDone
		} else {
			code = MoveResultTargetExists
		}
	default:
		this := nameOfAncestors(n.fileNodeCore)
		{
			that := []string{}
			if !filepath.IsAbs(path) {
				that = append(this, "..", path)
			} else {
				that = append(that, path)
			}
			if strings.HasSuffix(path, string(os.PathSeparator)) {
				that = append(that, n.fileNodeCore.name())
			}
			path = filepath.Join(that...)
		}
		head := filepath.Join(this...)

		switch {
		case path == head:
			code = MoveResultDone
		case len(path) > len(head) && strings.HasPrefix(path, head):
			code = MoveResultTargetIsChild
		default:
			node, find := findOrMakeNodeByPath(n.fileNodeCore.root(), splite(path))
			switch {
			case find:
				code = MoveResultTargetExists
			case node == nil:
				code = MoveResultTargetInvalid
			case !swapNodes(n.fileNodeCore, node):
				code = MoveResultNodeUnmovable
			default:
				removeNode(node)
				code = MoveResultDone
			}
		}
	}

	return
}

func (n *wrapper) Disuse() {
	/*_*/ n.lock.Lock()
	defer n.lock.Unlock()

	disuseThisNode(n.fileNodeCore)
}
