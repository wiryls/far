namespace Fx.Diff
{
    public enum Action
    {
        Retain,
        Insert,
        Delete,
    }

    public struct Operation
    {
        public Action Type;
        public string Text;
    }

    public class Diff : List<Operation>, IComparer<Diff>
    {
        public Diff() : base() { }

        public Diff(int capacity) : base(capacity) { }

        public void Add(Action action, string text) =>
            Add(new Operation { Type = action, Text = text });

        public int Compare(Diff? x, Diff? y) => Enumerable
            .Zip(RetainInsertToChars(x), RetainInsertToChars(y), (l, r) => l - r)
            .FirstOrDefault(x => x != 0);

        public string Source
        { 
            get => string.Concat(this.Where(x => x.Type != Action.Insert).Select(x => x.Text));
        }

        public string Target
        {
            get => string.Concat(this.Where(x => x.Type != Action.Delete).Select(x => x.Text));
        }

        private static IEnumerable<char> RetainInsertToChars(Diff? x) => x?
            .Where(x => x.Type != Action.Delete)
            .SelectMany(x => x.Text.AsEnumerable()) ?? Enumerable.Empty<char>();

    }
}
