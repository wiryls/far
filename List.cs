using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Windows.Data;

namespace FAR
{
    public class List : ObservableCollection<Item>
    {
        public List() : base()
        {
            Add(new Item { Stat = Status.Todo, Path = "/user/bin", View = "bin" });
            Add(new Item { Stat = Status.Fail, Path = "/user/local", View = "local" });
            Add(new Item { Stat = Status.Fail, Path = "/", View = "" });
            Add(new Item { Stat = Status.Done, Path = "/etc/apt", View = "apt" });
            Add(new Item { Stat = Status.Fail, Path = "/etc", View = "etc" });
            Add(new Item { Stat = Status.Done, Path = "/user/local/bin", View = "bin" });
        }
    }

    public class Item
    {
        public Status Stat { get; set; }
        public string View { get; set; }
        public string Path { get; set; }
    }

    public enum Status
    {
        Todo,
        Done,
        Fail,
        Lost,
    }

    [ValueConversion(typeof(Status), typeof(string))]
    public class StatusConverter : IValueConverter
    {
        private const string todo = "TODO";
        private const string done = "DONE";
        private const string fail = "FAIL";
        private const string lost = "LOST";

        public object Convert(object value, Type target, object parameter, CultureInfo culture)
        {
            return (Status)value switch
            {
                Status.Todo => todo,
                Status.Done => done,
                Status.Fail => fail,
                Status.Lost => lost,
                _ => todo,
            };
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value as string switch
            {
                todo => Status.Todo,
                done => Status.Done,
                fail => Status.Fail,
                lost => Status.Lost,
                _ => Status.Todo,
            };
        }
    }

    public class Cahnge : List<Operation> { }

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
