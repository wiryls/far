package filenode

// FileNode is a basic element of a FileTree.
// type FileNode interface {
// fileNodeGetter
// fileNodeSetter

// Name of this file or directory.
// Name() string

// Full path of this file or directory.
// Path() string

// Dir is the parent directory of this `FileNode`.
//  - empty if no parent.
// Dir() string

// DirAndName return both dir and name of this node.
// DirAndName() (dir string, name string)

// Put a filepath into current FileTree. Return the FileNode and whether
// it is inserted.
//  - return old `FileNode` and false if it already exists.
// Put(path string) (FileNode, bool)

// Move the `old` path to 'new` path.
//  - If `new` already exists, it and all child nodes will be purged.
//  - If `old` do not exist, an empty and active `FileNode` will be
//    placed to `new`.
//  - If `old` is an ancestor of `new`, do nothing.
// Move(old, new string) (FileNode, bool)

// Remove a FileNode by filepath.
//  - return true if done, otherwise false.
// Remove(path string) bool

// Discard this `FileNode`.
// There is a situation when a `FileNode` is not used by outside, but may
// be still used by other `FileNode`. If so, this node will be marked as
// inactive and still here, instead of deleted.
// Discard()
// }

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

	// create a child node.
	createChildNode(name string) (it fileNodeCore)

	// delete a child node by its name.
	deleteChildNode(name string)

	// deleta all child nodes.
	deleteChildNodes()
}
