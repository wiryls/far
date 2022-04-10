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

        public int Compare(Diff? x, Diff? y) => Enumerable
            .Zip(ReservedToChars(x), ReservedToChars(y), (l, r) => l - r)
            .FirstOrDefault(x => x != 0);

        public string Source
        {
            get => Count is 1 && this[0].Type is Action.Retain
                ? this[0].Text
                : Count is 0
                ? string.Empty
                : string.Concat(this.Where(x => x.Type != Action.Insert).Select(x => x.Text))
                ;
        }

        public string Target
        {
            get => string.Concat(this.Where(x => x.Type != Action.Delete).Select(x => x.Text));
        }

        internal void Add(Action action, string text) =>
            Add(new () { Type = action, Text = text });

        internal bool Unchanged
        {
            get => Enumerable.SequenceEqual
                ( this.Where(x => x.Type is not Action.Delete).SelectMany(x => x.Text.AsEnumerable())
                , this.Where(x => x.Type is not Action.Insert).SelectMany(x => x.Text.AsEnumerable()));
        }

        private static IEnumerable<char> ReservedToChars(Diff? x) => x?
            .Where(x => x.Type is not Action.Delete).SelectMany(x => x.Text.AsEnumerable())
            ?? Enumerable.Empty<char>();
    }
}
