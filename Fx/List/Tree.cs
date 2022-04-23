using System.Collections;
using System.Runtime.CompilerServices;
[assembly: InternalsVisibleTo("FxTests")]

namespace Fx.List
{
    public class Tree<T> : IEnumerable<Node<T>>
    {
        private readonly Node<T> root = new (string.Empty);
        private readonly static char[] separators = new[]
        {
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar
        };

        /// <summary>
        /// Add an absolute file path to Tree.
        ///
        /// Root directories and paths with trailing '\' are not supported.
        /// </summary>
        /// <param name="path">an absolute file path</param>
        /// <returns>item when succeed, or null if existed or filepath not supported</returns>
        public Node<T>? Add(string path, T data)
        {
            var name = Path.GetFileName(path);
            if (string.IsNullOrEmpty(name))
                return null;

            var keys = Path.GetDirectoryName(path)?.Split(separators, StringSplitOptions.RemoveEmptyEntries);
            if (keys is null || keys.Length == 0)
                return null;

            var node = root.Make(keys.Append(name));
            if (node is null || node.Used)
                return null;

            node.Data = data;
            node.Used = true;
            return node;
        }

        /// <summary>
        /// Remove a item from tree
        /// </summary>
        /// <param name="node">item to removed</param>
        /// <returns>true if done, otherwise not found</returns>
        public bool Remove(Node<T> node)
        {
            if (node.Data is null)
                return false;

            var rise = node.Last;
            node.Last = null;
            node.Data = default;
            node.Used = false;

            var last = rise;
            while (last is not null && last.Used is false && last.Tree?.Count is 0 && last.Tree.Remove(last))
            {
                rise = last.Last;
                last.Last = null;
                last = rise;
            }
            return true;
        }

        /// <summary>
        /// Rename a item with a new name. If target name is aleady exists, remove it
        /// and its children, populate them all into `drop`.
        /// </summary>
        /// <param name="node">item to rename</param>
        /// <param name="name">the new name</param>
        /// <param name="drop">all nodes that be overwritten</param>
        /// <returns>true if succeed, otherwise false</returns>
        public bool Rename(Node<T> node, string name, out IEnumerable<Node<T>>? drop)
        {
            drop = null;
            if (node.Name.Equals(name))
                return false; // new name == old name

            var last = node.Last;
            var tree = last?.Tree;
            if (tree is null)
                return false;

            var that = new Node<T>(name, last);
            if (tree.TryGetValue(that, out var oops) && tree.Remove(oops))
                drop = Walk(oops);

            if (tree.Remove(node) is false)
                return false;

            node.Name = name;
            if (tree.Add(node) is false)
                return false; // should never happend

            return true;
        }

        /// <summary>
        /// Remove all nodes.
        /// </summary>
        public void Clear()
        {
            root.Tree = null;
        }

        public bool IsEmpty => root.Tree is null || root.Tree.Count is 0;

        public IEnumerator<Node<T>> GetEnumerator()
        {
            return Walk(root).GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        private static IEnumerable<Node<T>> Walk(Node<T> it)
        {
            if (it.Used)
                yield return it;

            if (it.Tree is null)
                yield break;

            var nodes = new Stack<SortedSet<Node<T>>.Enumerator>();
            nodes.Push(it.Tree.GetEnumerator());
            while (nodes.Count is not 0)
            {
                var current = nodes.Peek();
                if (current.MoveNext())
                {
                    var node = current.Current;

                    if (node.Used)
                        yield return node;

                    if (node.Tree is not null)
                        nodes.Push(node.Tree.GetEnumerator());
                }
                else
                {
                    nodes.Pop();
                }
            }
        }
    }
}
