package free

import (
	"path/filepath"
)

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

// don't modify during visiting.
func visitAllChildNodes(node fileNodeCore, f func(fileNodeGetter) bool) {
	node.traverseChildNodes(func(_ string, node fileNodeCore) (goon bool) {
		if goon = f(node); goon {
			visitAllChildNodes(node, f)
		}
		return
	})
}

// find a node by path. (note: "." and ".." is not supported.)
//
//  - the node of divergence if not found.
//  - itself if path is nil or empty.
//  - nil if node is empty.
func findNodeByPath(node fileNodeCore, path []string) (fileNodeCore, []string) {

	switch {

	case len(path) == 0:
		path = nil

	case len(path) == 1 && len(path[0]) == 1 && path[0][0] == '.':
		path = nil

	case node == nil || node.root() == nil:
		node = nil

	default:
		if filepath.IsAbs(path[0]) {
			node = node.root()
		}

		for len(path) != 0 {
			if next := node.child(path[0]); next != nil {
				node = next
				path = path[1:]
			} else {
				break
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

func moveNode(node fileNodeCore, path []string, putin bool) (code MoveResult) {

	switch {
	case len(path) == 0 || path[0] == "." || path[0] == "..":
		code = MoveResultTargetInvalid
	case node == nil || node.root() == nil || node.parent() == nil:
		code = MoveResultNodeUnmovable
	case node.root() == node || node.parent() == node:
		code = MoveResultNodeUnmovable
	}

	var lhs, rhs []string
	if code == MoveResultDone {
		lhs = nameOfAncestors(node)
		rhs = path
		if putin {
			rhs = append(rhs, lhs[len(lhs)-1])
		}
	}

	done := code == MoveResultDone && len(lhs) <= len(rhs)
	if done {
		i, n := 0, len(lhs)
		for ; i < n; i++ {
			if lhs[i] != rhs[i] {
				break
			}
		}

		if i == n-1 && n == len(rhs) {
			this := lhs[len(lhs)-1]
			that := rhs[len(rhs)-1]
			next := node.parent()
			if next.child(that) == nil {
				node.setName(that)
				if next.putChildNode(node) {
					next.deleteChildNode(this)
				} else {
					node.setName(this)
					code = MoveResultNodeUnmovable
				}
			} else {
				code = MoveResultTargetExists
			}
		} else if i == n && n < len(rhs) {
			code = MoveResultTargetIsChild
		} else {
			done = false
		}
	}

	if !done {
		this := lhs[len(lhs)-1]
		that := rhs[len(rhs)-1]
		next, find := findOrMakeNodeByPath(node, rhs[:len(rhs)-1])
		if find && next.child(that) != nil {
			code = MoveResultTargetExists
		} else {
			last := node.parent()
			node.setName(that)
			if next.putChildNode(node) {
				node.setParent(next)
				last.deleteChildNode(this)
			} else {
				node.setName(this)
				code = MoveResultNodeUnmovable
			}
		}
	}

	return
}
