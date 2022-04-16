using Fx.Diff;
using Fx.List;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using Change = Fx.Diff.Patch;
using Marker = Fx.List.Node;

namespace Far.ViewModel
{
    public enum Status
    {
        Todo,
        Done,
        Fail,
        Lost,
        Gone,
    }

    public class Item
    {
        private Status status;
        private Marker marker;
        private Change change;

        public Item(Marker mark)
        {
            status = Status.Todo;
            marker = mark;
            change = new (mark.Name.ToString());
        }

        public Status Status
        {
            get => status;
            set => status = value;
        }

        public Change Preview
        {
            get => change;
            set => change = value;
        }

        public string Directory => marker.Directory.ToString();

        internal Marker Marker => marker;

        internal string Source => change.Source;

        internal string Target => change.Target;
    }

    internal struct Items
    {
        // status
        private IDiffer differ;
        private readonly Tree source;
        private ObservableCollection<Item> viewed;

        // buffer
        private List<Item> sorted;
        private List<Item> wanted;
        private bool update;

        public Items()
        {
            //var i = new System.ComponentModel.BindingList<Item>();

            differ = DifferCreator.Create();
            source = new();
            viewed = new();
            sorted = new();
            wanted = new();
            update = false;
        }

        public bool Add(string path)
        {
            if (source.Add(path) is not Marker mark)
                return false;

            var item = new Item(mark);
            var diff = differ.Match(item.Source);
            item.Preview = diff;
            sorted.Add(item);

            if (item.Preview.Matched)
                wanted.Add(item);

            if (item.Preview.Changed || differ.Empty)
                viewed.Add(item);

            return true;
        }

        public bool Remove(int index)
        {
            if (viewed.Count <= index || index < 0)
                return false;

            var item = viewed[index];
            item.Status = Status.Gone;

            viewed.RemoveAt(index);
            source.Remove(item.Marker);
            update = true;
            return true;
        }

        public void Clear()
        {
            source.Clear();
            sorted.Clear();
            wanted.Clear();
            viewed.Clear();
        }

        public bool Rename()
        {
            if (differ.Empty is false)
                return false;

            foreach (var item in viewed)
            {
                var info = new FileInfo(Path.Combine(item.Directory, item.Source));
                if (info.Exists is false)
                {
                    item.Status = Status.Lost;
                    continue;
                }

                try
                {
                    // TODO: test it
                    // info.MoveTo(Path.Combine(item.Path, item.Source));
                    item.Status = Status.Done;
                }
                catch (Exception)
                {
                    item.Status = Status.Fail;
                    continue;
                }

                //if (source.Rename(item) is false)
                //{
                //    item.Status = Status.Fail;
                //}
            }

            return true;
        }

        public bool Differ(IDiffer target)
        {
            if (update)
            {
                sorted = sorted.Where(x => x.Status is not Status.Gone).ToList();
                wanted = wanted.Where(x => x.Status is not Status.Gone).ToList();
                update = false;
            }

            var listed
                = Equals(differ.GetType(), target.GetType()) && differ.Pattern == target.Pattern
                ? wanted
                : sorted
                ;

            differ = target;
            viewed
                = differ.Empty
                ? new(sorted)
                : new(listed.Where(x => (x.Preview = target.Match(x.Source)).Changed))
                ;

            return true;
        }

        public bool Sort(/* by */)
        {
            // TODO:

            return false;
        }

        public ObservableCollection<Item> View { get => viewed; }
    }
}
