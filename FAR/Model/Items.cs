using System.Collections.ObjectModel;
using Fx.Diff;
using Change = Fx.Diff.Diff;

namespace Far
{
    public class Items : ObservableCollection<Item>
    {
        public void Add(string dir, Change view)
        {
            Add(new Item
            {
                Stat = Status.Todo,
                View = view,
                Path = dir,
                Source = view.Source,
                Target = view.Target,
            });
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

    // references:
    //
    // - https://docs.microsoft.com/en-us/dotnet/desktop/wpf/data/how-to-create-and-bind-to-an-observablecollection
    // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.specialized.inotifycollectionchanged
    // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.ilist
}
