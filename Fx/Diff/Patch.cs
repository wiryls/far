using System.Collections;

namespace Fx.Diff
{
    public enum Action
    {
        Retain,
        Insert,
        Delete,
    }

    public readonly struct Operation
    {
        public readonly Action Type;
        public readonly string Text;

        public Operation(Action type, string text)
        {
            Type = type;
            Text = text;
        }
    }

    public readonly struct Patch : IComparer<Patch>, IEnumerable<Operation>
    {
        private readonly string source;
        private readonly bool matched;
        private readonly bool changed;
        private readonly IReadOnlyCollection<Operation> operations;

        public Patch(string input, bool match = false)
        {
            source = input;
            matched = match;
            changed = false;
            operations = Array.Empty<Operation>();
        }

        public Patch(string input, List<Operation> ops)
        {
            source = input;
            matched = ops.Count is not 0;
            changed = ops.Any(x => x.Type is not Action.Retain) && !Enumerable.SequenceEqual(ToSource(ops), ToTarget(ops));
            operations = changed ? ops.ToArray() : Array.Empty<Operation>();
        }

        public bool Matched { get => matched; }

        public bool Changed { get => changed; }

        public string Source { get => source; }

        public string Target
        {
            get => changed
                ? string.Concat(operations.Where(x => x.Type is not Action.Delete).Select(x => x.Text))
                : source
                ;
        }

        public int Compare(Patch x, Patch y) =>
            string.Compare(x.source, y.source);

        public IEnumerator<Operation> GetEnumerator() =>
            operations.AsEnumerable().GetEnumerator();

        IEnumerator IEnumerable.GetEnumerator() =>
            GetEnumerator();

        public static readonly Patch Empty = new (string.Empty);

        private static IEnumerable<char> ToSource(IEnumerable<Operation> x) =>
            x.Where(x => x.Type is not Action.Insert).SelectMany(x => x.Text.AsEnumerable());

        private static IEnumerable<char> ToTarget(IEnumerable<Operation> x) =>
            x.Where(x => x.Type is not Action.Delete).SelectMany(x => x.Text.AsEnumerable());
    }

    internal class PatchBuilder
    {
        private readonly List<Operation> os;

        public PatchBuilder(int capacity = 1)
        {
            os = new (capacity);
        }

        public void Retain(ReadOnlySpan<char> text) => os.Add(new (Action.Retain, text.ToString()));

        public void Insert(ReadOnlySpan<char> text) => os.Add(new (Action.Insert, text.ToString()));

        public void Delete(ReadOnlySpan<char> text) => os.Add(new (Action.Delete, text.ToString()));

        // Note:
        // As span or memory may be better than string, I may optimize it someday.

        public Patch Build(string source) => new (source, os);
    }
}
