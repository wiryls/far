using System.Text.RegularExpressions;

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

    public class Change : List<Operation>, IComparer<Change>
    {
        public int Compare(Change? x, Change? y)
        {
            static IEnumerable<char> toChars(Change? x) => x?
                .Where(x => x.Type != Operation.Action.Delete)
                .SelectMany(x => x.Text.AsEnumerable()) ?? Enumerable.Empty<char>();

            return Enumerable
                .Zip(toChars(x), toChars(y), (l, r) => l - r)
                .FirstOrDefault(x => x != 0);
        }
    }

    internal interface IMatcher
    {

    }

    public class Differ
    {
        private string pattern;
        private string template;
    }

}
