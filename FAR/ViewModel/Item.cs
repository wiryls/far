using Fx.Diff;
using Fx.List;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Change = Fx.Diff.Patch;
using Marker = Fx.List.Node<Far.ViewModel.Item>;

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

    public class Item : ViewModelBase
    {
        private Status status = Status.Todo;
        private Marker marker = null;
        private Change change = Change.Empty;

        private Item()
        {
            Selected = false;
        }

        public Item Rediff(IDiffer differ)
        {
            SetProperty(ref change, differ.Match(change.Source), nameof(Preview));
            return this;
        }

        public bool Rename(Tree<Item> tree, string name, out IEnumerable<Marker> drop)
        {
            var done = tree.Rename(marker, name, out drop);
            if (done)
                SetProperty(ref change, new (marker.Name), nameof(Preview));
            return done;
        }

        public Status Status
        {
            get => status;
            set => SetProperty(ref status, value);
        }

        public Change Preview => change;

        public string Directory => Path.Join(marker.Directories.ToArray()); // inefficiently

        public string Source => change.Source;

        internal string Target => change.Target; // inefficiently

        internal Marker Marker => marker;

        internal bool Changed => change.Changed;

        internal bool Matched => change.Matched;

        internal bool Selected { get; set; }

        public static bool Create(Tree<Item> tree, IDiffer diff, string path, out Item item)
        {
            item = new Item();
            if (tree.Add(path, item) is Marker mark)
            {
                item.marker = mark;
                item.change = diff.Match(mark.Name);
                return true;
            }

            item = null;
            return false;
        }

        // References:
        // https://stackoverflow.com/a/46038091
    }

    internal struct Items
    {
        private struct ItemList : IEnumerable<Item>
        {
            private List<Item> data;

            public ItemList()
            {
                data = new ();
                Outdate = false;
            }

            public void Add(Item item)
            {
                data.Add(item);
            }

            public void Add(IEnumerable<Item> items)
            {
                data.AddRange(items);
            }

            public void Clear()
            {
                data.Clear();
                Outdate = false;
            }

            public void Clean()
            {
                if (Outdate)
                {
                    data = data.Where(x => x.Status is not Status.Gone).ToList();
                    Outdate = false;
                }
            }

            public IEnumerator<Item> GetEnumerator() => data.GetEnumerator();

            IEnumerator IEnumerable.GetEnumerator() => data.GetEnumerator();

            public bool Outdate { private get; set; }

        }

        // status
        private IDiffer differ;
        private readonly Tree<Item> source;
        private readonly ObservableCollection<Item> viewed;

        // buffer
        private ItemList sorted;
        private ItemList wanted;

        public Items()
        {
            differ = DifferCreator.Create();
            source = new ();
            viewed = new ();
            sorted = new ();
            wanted = new ();
        }

        public bool Add(string path)
        {
            if (Item.Create(source, differ, path, out var item) is false)
                return false;

            sorted.Add(item);

            if (item.Matched)
                wanted.Add(item);

            if (item.Changed || differ.IsEmpty)
                viewed.Add(item);

            return true;
        }

        public void RemoveSelected()
        {
            var count = 0;
            var total = viewed.Count;
            foreach (var item in viewed.Where(x => x.Selected))
            {
                item.Status = Status.Gone;
                if (source.Remove(item.Marker))
                    count++;
            }

            if (count is 0)
                return;

            sorted.Outdate = true;
            wanted.Outdate = true;
            viewed.Clear();
            if (count == total)
                return;

            if (differ.IsEmpty)
            {
                sorted.Clean();
                foreach (var item in sorted)
                    viewed.Add(item);
            }
            else
            {
                wanted.Clean();
                foreach (var item in wanted.Where(x => x.Changed))
                    viewed.Add(item);
            }
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
            if (differ.IsEmpty is false)
                return false;

            var dropped = Enumerable.Empty<IEnumerable<Marker>>();
            foreach (var item in viewed)
            {
                var dirs = item.Directory;
                var name = item.Target;
                var that = Path.Join(dirs, name);

                var info = null as FileInfo;
                try
                {
                    info = new (Path.Join(dirs, item.Source));
                }
                catch
                {
                    item.Status = Status.Fail;
                    continue;
                }
                if (info.Exists is false)
                {
                    item.Status = Status.Lost;
                    continue;
                }

                try
                {
                    // TODO:
                    // info.MoveTo(that);
                    Debug.WriteLine($"Rename '${item.Source}' to '${name}'");
                }
                catch
                {
                    item.Status = Status.Fail;
                    continue;
                }

                if (item.Rename(source, name, out var drop) is false)
                {
                    item.Status = Status.Lost;
                    continue;
                }

                item.Status = Status.Done;
                if (drop is not null)
                    dropped = dropped.Append(drop);
            }

            foreach (var node in dropped.SelectMany(x => x).Select(x => x.Data))
            {
                if (viewed.Remove(node))
                    node.Status = Status.Gone;

                Debug.WriteLine($"Remove '${node.Source}'");
            }

            return true;
        }

        public bool Differ(IDiffer target)
        {
            viewed.Clear();
            if (target.IsEmpty)
            {
                sorted.Clean();
                foreach (var item in sorted)
                    viewed.Add(item.Rediff(target));
            }
            else if (Equals(differ.GetType(), target.GetType()) && differ.Pattern == target.Pattern)
            {
                wanted.Clean();
                foreach (var item in wanted.Where(x => x.Rediff(target).Changed))
                    viewed.Add(item);
            }
            else
            {
                wanted.Clear();
                wanted.Add(sorted.Where(x => x.Rediff(target).Matched));
                foreach (var item in wanted.Where(x => x.Changed))
                    viewed.Add(item);
            }

            differ = target;
            return true;
        }

        public bool Sort(/* by */)
        {
            // TODO:

            return false;
        }

        public ObservableCollection<Item> View => viewed;

        public bool IsEmpty => source.IsEmpty;
    }
}
