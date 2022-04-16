using System;
using System.Collections;
using System.Linq;
using System.Runtime.CompilerServices;
[assembly: InternalsVisibleTo("FxTests")]

namespace Fx.List
{
    public class Tree : IEnumerable<Node>
    {
        private readonly SortedDictionary<string, ValueTuple<Tree?, Node?>> data;
        
        /// <summary>
        /// Create a Tree of Nodes.
        /// </summary>
        public Tree()
        {
            data = new ();
        }

        /// <summary>
        /// Add an absolute file path to Tree.
        ///
        /// Root directories and paths with trailing '\' are not supported.
        /// </summary>
        /// <param name="path">an absolute file path</param>
        /// <returns>item when succeed, or null if existed or filepath not supported</returns>
        public Node? Add(string path)
        {
            var node = new Node(path);
            var name = node.Name.ToString();
            if (string.IsNullOrEmpty(name))
                return null;

            var tree = node.Directories.Select(x => x.ToString()).Aggregate(this, (u, v) => u.Make(v));
            if (tree.data.TryGetValue(name, out var pair) is false)
                tree.data.Add(name, (null, node));
            else if (pair.Item2 is null)
                tree.data[name] = (pair.Item1, node);
            else
                return null;

            return node;
        }

        /// <summary>
        /// Remove a item from tree
        /// </summary>
        /// <param name="item">item to removed</param>
        /// <returns>true if done, otherwise not found</returns>
        public bool Remove(Node item)
        {
            if (Locate(item, out var list) is false)
                return false;

            Remove(list);
            return true;
        }

        /// <summary>
        /// Rename a item with a new name. If target name is aleady exists, remove it
        /// and its children, populate them all into `drop`.
        /// </summary>
        /// <param name="item">item to rename</param>
        /// <param name="name">the new name</param>
        /// <param name="drop">any node that be overwritten</param>
        /// <returns></returns>
        public bool Rename(ref Node item, string name, out IEnumerable<Node> drop)
        {
            drop = Enumerable.Empty<Node>();
            if (item.Name.Equals(name.AsSpan(), StringComparison.Ordinal))
                return false; // new name == old name

            var that = new Node(Path.Join(item.Directory, name));
            var what = that.Name.ToString();
            if (string.IsNullOrEmpty(what))
                return false; // new name is empty

            if (Locate(item, out var list) is false)
                return false; // cannot find old node

            var tree = list[^1].Item1;
            var copy = list[^1].Item3;

            if (tree.data.TryGetValue(what, out var pair))
                tree.data[what] = (copy, that);
            else
                tree.data.Add(what, (copy, that));

            Remove(list);

            if (pair.Item2 is Node node)
                drop = drop.Append(node);
            if (pair.Item1 is Tree next)
                drop = drop.Concat(Walk(next));

            item = that;
            return true;
        }

        public void Clear()
        {
            data.Clear();
        }

        public IEnumerator<Node> GetEnumerator()
        {
            return Walk(this).GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        private static IEnumerable<Node> Walk(Tree it)
        {
            var list = new List<IEnumerator<KeyValuePair<string, ValueTuple<Tree?, Node?>>>>
            {
                it.data.GetEnumerator()
            };

            while (list.Count is not 0)
            {
                var last = list.Last();
                if (last.MoveNext())
                {
                    var (tree, item) = last.Current.Value;

                    if (item is Node o)
                        yield return o;

                    if (tree is not null)
                        list.Add(tree.data.GetEnumerator());
                }
                else
                {
                    list.RemoveAt(list.Count - 1);
                }
            }
        }

        private bool Locate(Node item, out List<(Tree, string, Tree?, Node?)> position)
        {
            // construct ValueTuple of (tree, child name, child tree, child item)

            var tree = this;
            var dirs = item.Directories.Select(x => x.ToString()).ToArray();
            position = new (dirs.Length + 1);
            foreach (var dir in dirs.Append(item.Name.ToString()))
            {
                if (tree.data.TryGetValue(dir, out var pair))
                {
                    position.Add((tree, dir, pair.Item1, pair.Item2));
                    tree = pair.Item1;
                }
                if (tree is null)
                {
                    break;
                }
            }

            return position.Capacity == position.Count;
        }

        private static void Remove(List<(Tree, string, Tree?, Node?)> list)
        {
            var (tree, name, next, _) = list[^1];
            if (next is not null)
                tree.data[name] = (next, null);
            else if (tree.data.Remove(name))
                foreach (var (t, n, c, i) in list.SkipLast(1).Reverse())
                    if (c.data.Count is 0 && i is null)
                        t.data.Remove(n);
                    else
                        break;
        }

        private Tree Make(string name)
        {
            if (data.TryGetValue(name, out var pair))
            {
                var tree = pair.Item1 ?? new Tree();
                if (pair.Item1 == null)
                {
                    pair.Item1 = tree;
                    data[name] = pair;
                }
                return tree;
            }
            else
            {
                var tree = new Tree();
                data.Add(name, (tree, null));
                return tree;
            }
        }
    }
}
