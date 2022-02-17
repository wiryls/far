using System.Collections.Generic;
using System.Linq;

namespace Fx.Diff
{

    public class Change : List<Operation>, IComparer<Change>
    {
        public int Compare(Change x, Change y)
        {
            var l = x
                .Where(x => x.Type != Operation.Action.Delete)
                .SelectMany(x => x.Text.AsEnumerable());
            var r = y
                .Where(x => x.Type != Operation.Action.Delete)
                .SelectMany(x => x.Text.AsEnumerable());
            return Enumerable
                .Zip(l, r, (l, r) => l - r)
                .FirstOrDefault(x => x != 0);
        }
    }

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
}
