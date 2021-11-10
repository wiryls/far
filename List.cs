using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

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
            Add(new Item { Stat = Status.Done, Path = "/user/local/bin", View = new Change { new Operation { Type = Operation.Action.Insert, Text = "nooo" } } });
        }
    }

    public class Item
    {
        public Status Stat { get; set; }
        public Change View { get; set; }
        public string Path { get; set; }
    }

    public enum Status
    {
        Todo,
        Done,
        Fail,
        Lost,
    }

    public class Change : List<Operation> { }

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
