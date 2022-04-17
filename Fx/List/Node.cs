namespace Fx.List
{
    public class Node<T> : IComparable<Node<T>>
    {
        public Node(string name, Node<T>? parent = null)
        {
            Name = name;
            Last = parent;
            Used = false;
        }

        public string Name { get; internal set; }

        public T? Data { get; internal set; }

        internal bool Used { get; set; }

        internal Node<T>? Last { get; set; }

        internal SortedSet<Node<T>>? Tree { get; set; }

        public IEnumerable<string> Directories => Rise(Last).Reverse();

        public bool Equals(Node<T> node) => Name == node.Name;

        public int CompareTo(Node<T>? other) => Name.CompareTo(other?.Name);

        public override int GetHashCode() => Name.GetHashCode();

        public override bool Equals(object? o) => o is Node<T> node && Equals(node);

        internal Node<T>? Step(IEnumerable<string> keys)
        {
            return Walk(this, keys, Step);
        }

        internal Node<T>? Make(IEnumerable<string> keys)
        {
            return Walk(this, keys, Make);
        }

        internal static Node<T>? Step(Node<T> node, string name)
        {
            if (node.Tree is null)
                return null;

            var that = new Node<T>(name, node);
            if (node.Tree.TryGetValue(that, out var next))
                return next;

            return null;
        }

        internal static Node<T>? Make(Node<T> node, string name)
        {
            if (node.Tree is null)
                node.Tree = new();

            var that = new Node<T>(name, node);
            if (node.Tree.TryGetValue(that, out var next))
                return next;

            if (node.Tree.Add(that))
                return that;

            return null; // should never happend
        }

        private static Node<T>? Walk(Node<T> from, IEnumerable<string> keys, Func<Node<T>, string, Node<T>?> func)
        {
            var next = keys.GetEnumerator();
            var node = from;
            while (node is not null && next.MoveNext())
                node = func(node, next.Current);
            return node;
        }

        private static IEnumerable<string> Rise(Node<T>? node)
        {
            while (node is not null && string.IsNullOrEmpty(node.Name) is false)
            {
                yield return node.Name;
                node = node.Last;
            }
        }
    }
}
