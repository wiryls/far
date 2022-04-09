using System.Collections;
using System.Runtime.CompilerServices;
[assembly: InternalsVisibleTo("FxTests")]

namespace Fx.List
{
    public class Tree : IEnumerable<Item>
    {
        private readonly Pool pool;
        private readonly Turn turn;
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
            this.turn = new ();
        }

        /// <summary>
        /// Add an absolute file path to Tree. The '\' at the end will be ignored.
        /// </summary>
        /// <param name="path">an absolute file path</param>
        /// <returns>item when succeed, or null if existed</returns>
        public Item? Add(string path)
        {
            var name = Path.GetFileName(path);
            if (string.IsNullOrEmpty(name))
                return null;

            var head = Path.GetDirectoryName(path) ?? string.Empty;
            var dirs = head.Split(separators, StringSplitOptions.RemoveEmptyEntries) ?? Array.Empty<string>();
            var node = dirs.Aggregate(this, (cur, dir) => cur.Make(dir));

            if (node.turn.TryGetValue(name, out var pair) is false)
            {
                var item = CreateItem(head, name);
                node.turn.Add(name, (null, item));
                return item;
            }
            else if (pair.Item2 is null)
            {
                pair.Item2 = CreateItem(head, name);
                node.turn[name] = pair;
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

            pool.Pop(item.Source);
            if (node is not null)
                tree.turn[name] = (node, null);
            else if (tree.turn.Remove(name))
                foreach (var (t, n, c, i) in list.SkipLast(1).Reverse())
                    if (c.turn.Count is 0 && i is null)
                        t.turn.Remove(n);

            return true;
        }

        public bool Rename(Item from, string to)
        {
            if (pool.Has(from.Path) is false)
                return false;

            var dirs = from.Path.Split(separators, StringSplitOptions.RemoveEmptyEntries);
            if (Walk(dirs, from.Source, out var list) is false)
                return false;

            var (tree, name, node, item) = list[^1];
            if (item != from)
                return false;

            if (tree.turn.ContainsKey(to))
                return false;

            if (tree.turn.Remove(name) is false)
                return false;

            item.Source = to;
            tree.turn.Add(to, (node, item));
            return true;
        }

        public void Clear()
        {
            pool.Clear();
            turn.Clear();
        }

        public IEnumerator<Item> GetEnumerator()
        {
            var list = new List<IEnumerator<KeyValuePair<string, (Tree?, Item?)>>>
            {
                turn.GetEnumerator()
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
                        list.Add(tree.turn.GetEnumerator());
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
                if (tree.turn.TryGetValue(dir, out var pair))
                {
                    list.Add((tree, dir, pair.Item1, pair.Item2));
                    tree = pair.Item1;
                }
                if (tree is null)
                {
                    return false;
                }
            }

            return true;
        }

        private Tree Make(string name)
        {
            if (turn.TryGetValue(name, out var pair))
            {
                var tree = pair.Item1 ?? new Tree(pool);
                if (pair.Item1 == null)
                {
                    pair.Item1 = tree;
                    turn[name] = pair;
                }
                return tree;
            }
            else
            {
                var tree = new Tree(pool);
                turn.Add(name, (tree, null));
                return tree;
            }
        }
    }

    internal class Turn : SortedDictionary<string, ValueTuple<Tree?, Item?>>
    {

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
    }
}
