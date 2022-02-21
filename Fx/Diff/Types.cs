namespace Fx.Diff
{
    public struct Operation
    {
        public enum Action
        {
            Retain,
            Insert,
            Delete,
        }

        public Action Type;
        public string Text;
    }

    public class Diff : List<Operation>, IComparer<Diff>
    {
        public int Compare(Diff? x, Diff? y)
        {
            static IEnumerable<char> toChars(Diff? x) => x?
                .Where(x => x.Type != Operation.Action.Delete)
                .SelectMany(x => x.Text.AsEnumerable()) ?? Enumerable.Empty<char>();

            return Enumerable
                .Zip(toChars(x), toChars(y), (l, r) => l - r)
                .FirstOrDefault(x => x != 0);
        }
    }
}
