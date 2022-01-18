using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace FAR
{
    public class List : ObservableCollection<Item>
    {
        public List() : base()
        {
            Add(new Item { Stat = Status.Todo, Path = "/user/bin", View = new Change { new Operation { Type = Operation.Action.Insert, Text = "nooo" } } });
            Add(new Item { Stat = Status.Fail, Path = "/user/local", View = new Change { new Operation { Type = Operation.Action.Insert, Text = "nooo" } } });
            Add(new Item { Stat = Status.Fail, Path = "/", View = new Change { new Operation { Type = Operation.Action.Retain, Text = "nooo" } } });
            Add(new Item { Stat = Status.Done, Path = "/etc/apt", View = new Change { new Operation { Type = Operation.Action.Retain, Text = "nooo" } } });
            Add(new Item { Stat = Status.Fail, Path = "/etc", View = new Change { new Operation { Type = Operation.Action.Delete, Text = "nooo" } } });
            Add(new Item { Stat = Status.Done, Path = "/user/local/bin", View = new Change { new Operation { Type = Operation.Action.Insert, Text = "nooo" }, new Operation { Type = Operation.Action.Delete, Text = "what" } } });
        }
    }

    public class Item
    {
        public Status Stat { get; set; } // task status
        public Change View { get; set; } // preview
        public string Path { get; set; } // path to directory
        public string Source { get; set; } // the source name
        public string Target { get; set; } // the target name
    }

    public enum Status
    {
        Todo,
        Done,
        Fail,
        Lost,
    }

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

    public class Operation
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

    // references:
    //
    // - https://docs.microsoft.com/en-us/dotnet/desktop/wpf/data/how-to-create-and-bind-to-an-observablecollection
    // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.specialized.inotifycollectionchanged
    // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.ilist
}
