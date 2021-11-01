using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;

namespace FAR
{
    public class Item
    {
        public string Stat { get; set; } // or enum
        public string View { get; set; } // rich text
        public string Path { get; set; } // rich text
    }

    public class List : ObservableCollection<Item>
    {
        public List() : base()
        {
            Add(new Item { Stat = "?", Path = "/user/bin", View = "bin" });
            Add(new Item { Stat = "x", Path = "/user/local", View = "local" });
            Add(new Item { Stat = "o", Path = "/", View = "" });
            Add(new Item { Stat = "!", Path = "/etc/apt", View = "apt" });
            Add(new Item { Stat = "f", Path = "/etc", View = "etc" });
            Add(new Item { Stat = "a", Path = "/user/local/bin", View = "bin" });
        }
    }

    // references:
    //
    // - https://docs.microsoft.com/en-us/dotnet/desktop/wpf/data/how-to-create-and-bind-to-an-observablecollection
    // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.specialized.inotifycollectionchanged
    // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.ilist
}
