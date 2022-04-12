using System.Collections;
using System.Runtime.CompilerServices;
[assembly: InternalsVisibleTo("FxTests")]

namespace Fx.List
{
    public class Tree : IEnumerable<Item>
    {
        private readonly Pool pool;
        private readonly SortedDictionary<string, ValueTuple<Tree?, Item?>> data;
        private readonly static char[] separators = new[]
        {
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar
        };

        public Tree()
            : this(new ())
        { }

        private Tree(Pool pool)
        {
            this.pool = pool;
            this.data = new ();
        }

        /// <summary>
        /// Add an absolute file path to Tree.
        ///
        /// Root directories and paths with trailing '\' are not supported.
        /// </summary>
        /// <param name="path">an absolute file path</param>
        /// <returns>item when succeed, or null if existed or filepath not supported</returns>
        public Item? Add(string path)
        {
            var name = Path.GetFileName(path);
            if (string.IsNullOrEmpty(name))
                return null;

            var head = Path.GetDirectoryName(path) ?? string.Empty;
            var dirs = head.Split(separators, StringSplitOptions.RemoveEmptyEntries) ?? Array.Empty<string>();
            var node = dirs.Aggregate(this, (cur, dir) => cur.Make(dir));

            if (node.data.TryGetValue(name, out var pair) is false)
            {
                var item = CreateItem(head, name);
                node.data.Add(name, (null, item));
                return item;
            }
            else if (pair.Item2 is null)
            {
                pair.Item2 = CreateItem(head, name);
                node.data[name] = pair;
                return pair.Item2;
            }
            else
            {
                return null;
            }
        }

        public bool Remove(Item item)
        {
            if (pool.Has(item.Path) is false)
                return false;

            var dirs = item.Path.Split(separators, StringSplitOptions.RemoveEmptyEntries);
            if (Walk(dirs, item.Source, out var list) is false)
                return false;

            // remove that item
            var (tree, name, node, view) = list[^1];
            if (view != item)
                return false;

            pool.Pop(item.Path);
            if (node is not null)
                tree.data[name] = (node, null);
            else if (tree.data.Remove(name))
                foreach (var (t, n, c, i) in list.SkipLast(1).Reverse())
                    if (c.data.Count is 0 && i is null)
                        t.data.Remove(n);
                    else
                        break;

            return true;
        }

        public bool Rename(Item item)
        {
            if (item.View.Changed is false || pool.Has(item.Path) is false)
                return false;

            var dirs = item.Path.Split(separators, StringSplitOptions.RemoveEmptyEntries);
            if (Walk(dirs, item.Source, out var list) is false)
                return false;

            var (tree, name, node, that) = list[^1];
            if (that != item)
                return false;

            var next = item.Target;
            if (tree.data.ContainsKey(next))
                return false;

            if (tree.data.Remove(name) is false)
                return false;

            that.Source = next;
            tree.data.Add(next, (node, that));
            return true;
        }

        public void Clear()
        {
            pool.Clear();
            data.Clear();
        }

        public IEnumerator<Item> GetEnumerator()
        {
            var list = new List<IEnumerator<KeyValuePair<string, (Tree?, Item?)>>>
            {
                data.GetEnumerator()
            };

            while (list.Count is not 0)
            {
                var it = list.Last();
                if (it.MoveNext())
                {
                    var (tree, item) = it.Current.Value;

                    if (item is not null)
                        yield return item;

                    if (tree is not null)
                        list.Add(tree.data.GetEnumerator());
                }
                else
                {
                    list.RemoveAt(list.Count - 1);
                }
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        internal int PoolSize
        {
            get => pool?.Count ?? 0;
        }

        private Item CreateItem(string path, string name)
        {
            return new Item(pool.Put(path), name);
        }

        private bool Walk(string[] dirs, string name, out List<(Tree, string, Tree?, Item?)> list)
        {
            // construct tuple of (tree, child name, child tree, child item)
            list = new List<(Tree, string, Tree?, Item?)>(dirs.Length + 1);

            var tree = this;
            foreach (var dir in dirs.Append(name))
            {
                if (tree.data.TryGetValue(dir, out var pair))
                {
                    list.Add((tree, dir, pair.Item1, pair.Item2));
                    tree = pair.Item1;
                }
                if (tree is null)
                {
                    break;
                }
            }

            return list.Capacity == list.Count;
        }

        private Tree Make(string name)
        {
            if (data.TryGetValue(name, out var pair))
            {
                var tree = pair.Item1 ?? new Tree(pool);
                if (pair.Item1 == null)
                {
                    pair.Item1 = tree;
                    data[name] = pair;
                }
                return tree;
            }
            else
            {
                var tree = new Tree(pool);
                data.Add(name, (tree, null));
                return tree;
            }
        }
    }

    internal class Pool
    {
        private readonly Dictionary<string, ValueTuple<string, uint>> dict = new ();

        public bool Has(string value)
        {
            return dict.ContainsKey(value);
        }

        public string Put(string value)
        {
            if (dict.TryGetValue(value, out var pair))
            {
                pair.Item2 += 1;
            
                dict[value] = pair;
                return pair.Item1;
            }
            else
            {
                dict.TryAdd(value, (value, 1));
                return value;
            }
        }

        public void Pop(string value)
        {
            if (dict.TryGetValue(value, out var pair))
            {
                if (pair.Item2 > 1)
                    dict[value] = (pair.Item1, pair.Item2 - 1);
                else
                    dict.Remove(value);
            }
        }

        public void Clear()
        {
            dict.Clear();
        }

        public int Count
        {
            get => dict.Count;
        }
    }
}
