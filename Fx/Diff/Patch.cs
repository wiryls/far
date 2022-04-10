using System.Collections;

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

    public readonly struct Patch : IComparer<Patch>, IEnumerable<Operation>
    {
        private readonly string source;
        private readonly bool matched;
        private readonly bool changed;
        private readonly Operation[] operations;

        internal Patch(string input, List<Operation> ops)
        {
            source = input;
            matched = ops.Count is not 0;
            changed = ops.Any(x => x.Type is not Action.Retain) && !Enumerable.SequenceEqual(SourceChars(ops), TargetChars(ops));
            operations = changed ? ops.ToArray() : Array.Empty<Operation>();
        }

        internal Patch(string input)
        {
            source = input;
            matched = true;
            changed = false;
            operations = Array.Empty<Operation>();
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

        private static IEnumerable<char> SourceChars(IEnumerable<Operation> x) =>
            x.Where(x => x.Type is not Action.Insert).SelectMany(x => x.Text.AsEnumerable());

        private static IEnumerable<char> TargetChars(IEnumerable<Operation> x) =>
            x.Where(x => x.Type is not Action.Delete).SelectMany(x => x.Text.AsEnumerable());
    }

    internal class PatchBuilder : List<Operation>
    {
        public PatchBuilder(int capacity = 1)
            : base(capacity)
        { }

        public void Retain(ReadOnlySpan<char> text) => Add(new() { Type = Action.Retain, Text = text.ToString() });

        public void Insert(ReadOnlySpan<char> text) => Add(new() { Type = Action.Insert, Text = text.ToString() });

        public void Delete(ReadOnlySpan<char> text) => Add(new() { Type = Action.Delete, Text = text.ToString() });

        // Note:
        // As span is probably better than string, I may optimize it someday.

        public Patch Build(string source) => new (source, this);
    }
}
