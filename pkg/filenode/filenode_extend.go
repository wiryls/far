package filenode

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

func moveNode(node fileNodeCore, path []string) (code MoveResult) {

	// maybe node is unmovable
	if node == nil || node.root() == nil || node.parent() == nil {
		code = MoveResultNodeUnmovable
	}

	var lhs, rhs []string
	if code == MoveResultDone {
		lhs = nameOfAncestors(node)
		rhs = path
	}

	// maybe target is child
	if code == MoveResultDone && len(lhs) < len(rhs) {
		hit := false

		for i := range lhs {
			hit = hit || lhs[i] != rhs[i]
		}

		if hit == false {
			code = MoveResultTargetIsChild
		}
	}

	// may exist or be invalid
	var dst fileNodeCore
	if code == MoveResultDone {
		var hit bool
		dst, hit = findOrMakeNodeByPath(node, path)
		switch {
		case dst == nil || dst.root() != node.root() || dst.parent() == nil:
			code = MoveResultTargetInvalid
		case hit:
			code = MoveResultTargetExists
		}
	}

	// maybe move
	if code == MoveResultDone {
		oldname := node.name()
		newname := dst.name()

		that := dst.parent()
		that.deleteChildNode(newname)
		that.putChildNode(node)
		node.parent().deleteChildNode(oldname)
		node.setParent(that)

		dst.setName("")
		dst.setParent(nil)
	}

	return
}
