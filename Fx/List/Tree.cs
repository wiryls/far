namespace Fx.List
{
    public class Tree
    {
        private readonly SortedDictionary<Node, Item?> nodes;
        private readonly Cache cache;
        private readonly static char[] separators = new[]
        {
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar
        };

        public Tree()
            : this(new ())
        { }

        internal Tree(Cache cache)
        {
            this.nodes = new SortedDictionary<Node, Item?>();
            this.cache = cache;
        }

        public void Remove(string path, string name)
        {
            // TODO:
            //nodes.Remove(path);
        }

        public void Rename(string path, string from, string to)
        {

        }

        public void Add(string path)
        {
            Append(path.Split(separators).Where(n => !string.IsNullOrEmpty(n)).GetEnumerator());
        }

        private void Append(IEnumerator<string> names)
        {
            // TODO:
            //var name = names.Current;
        }
    }

    internal struct Node
    {
        private readonly string name;
        private Tree? tree;

        public Node()
        {
            name = string.Empty;
            tree = null;
        }

        // transfer
    }

    internal class Cache
    {
        private readonly Dictionary<string, uint> cache;

        public Cache()
        {
            cache = new Dictionary<string, uint>();
        }

        public bool this[string value]
        {
            get => cache.ContainsKey(value);
        }

        public void Put(string value)
        {
            if (cache.TryAdd(value, 1) is false)
                cache[value] = 1;
        }

        public void Pop(string value)
        {
            if (cache.TryGetValue(value, out uint count) is false)
            {
                if (count > 1)
                    cache[value] = count - 1;
                else
                    cache.Remove(value);
            }
        }
    }
}
