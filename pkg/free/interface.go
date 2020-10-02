package free

// MoveResult is the result of the move operations.
type MoveResult uint8

// Enums for MoveResult
const (
	MoveResultDone MoveResult = iota
	MoveResultTargetExists
	MoveResultTargetIsChild
	MoveResultNodeUnmovable
	MoveResultTargetInvalid
)

// FileNode used to maintain a tree of files and directories.
type FileNode interface {

	// Name of file or directory.
	Name() string

	// Full path of this file or directory.
	Path() string

	// Dir is the parent directory of this FileNode.
	//  - empty if no parent.
	Dir() string

	// DirAndName return both dir and name of this node.
	DirAndName() (dir string, name string)

	// Find a child node by path.
	Find(path string) (node FileNode)

	// Put a new node by path. Return the node and whether it is created.
	//  - return old node and false if it exists.
	Put(path string) (node FileNode, created bool)

	// Move this node to target path.
	Move(path string) MoveResult

	// Disuse this node.
	// There is a situation when a node is not used by outside, but may still
	// be used by other nodes. If so, this node will be marked as unused and
	// still here, instead of deleted.
	Disuse()
}

// Only the most basic functions are included.
type fileNodeCore interface {
	fileNodeGetter
	fileNodeSetter
}

type fileNodeGetter interface {

	// name of this file or directory.
	name() string

	// is this node used by outside.
	using() bool

	// get the root node.
	//  - return itself if it is root.
	root() fileNodeCore

	// get the parent node.
	//  - return itself if it is root.
	parent() fileNodeCore

	// search a child node by name.
	//  - return nil if not found.
	child(name string) fileNodeCore

	// count the number of child nodes.
	countChildNodes() int

	// apply a for-range operation to child nodes.
	//  - the iteration stops if f return false.
	traverseChildNodes(f func(name string, node fileNodeCore) bool)
}

type fileNodeSetter interface {

	// set the name of this node.
	setName(name string)

	// set if this node is in use.
	setUsing(using bool)

	// set parent node.
	setParent(node fileNodeCore)

	// put a node as its child node.
	//  - false if node has a different root or node is nil or name conflict.
	putChildNode(node fileNodeCore) bool

	// create a child node.
	createChildNode(name string) (it fileNodeCore)

	// delete a child node by its name.
	deleteChildNode(name string)

	// deleta all child nodes.
	deleteChildNodes()
}
