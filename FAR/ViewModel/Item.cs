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
            if (SetProperty(ref change, differ.Match(change.Source), nameof(Preview)))
                OnPropertyChanged(nameof(Description));
            return this;
        }

        public bool Rename(Tree<Item> tree, string name, out IEnumerable<Marker> drop)
        {
            var done = tree.Rename(marker, name, out drop);
            if (done && SetProperty(ref change, new (marker.Name), nameof(Preview)))
                OnPropertyChanged(nameof(Description));
            return done;
        }

        public Status Status
        {
            get => status;
            set => SetProperty(ref status, value);
        }

        public Change Preview => change;

        public string Description => change.Changed ? $"{change.Source} → {change.Target}" : change.Source;

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

    internal class ItemList
    {
        public enum OrderBy
        {
            Stat,
            View,
            Path,
        }

        private class Items : IEnumerable<Item>
        {
            private List<Item> data;

            public Items()
            {
                data = new ();
                Outdate = false;
            }

            public Items Add(Item item)
            {
                data.Add(item);
                return this;
            }

            public Items Add(IEnumerable<Item> items)
            {
                data.AddRange(items);
                return this;
            }

            public Items Reset(List<Item> list = null)
            {
                if (list is null)
                    data.Clear();
                else
                    data = list;

                Outdate = false;
                return this;
            }

            public Items Sort(OrderBy order, bool isAscending)
            {
                Clean();
                data.Sort(order switch
                {
                    OrderBy.Stat => isAscending
                        ? (l, r) => l.Status - r.Status
                        : (r, l) => l.Status - r.Status,
                    OrderBy.Path => isAscending
                        ? (l, r) => CompareStrings(l.Marker.Directories, r.Marker.Directories)
                        : (r, l) => CompareStrings(l.Marker.Directories, r.Marker.Directories),
                    _ => isAscending
                        ? (l, r) => string.Compare(l.Marker.Name, r.Marker.Name)
                        : (r, l) => string.Compare(l.Marker.Name, r.Marker.Name),
                });
                return this;

                static int CompareStrings(IEnumerable<string> lhs, IEnumerable<string> rhs)
                {
                    var l = lhs.GetEnumerator();
                    var r = rhs.GetEnumerator();
                    while (true)
                    {
                        var u = l.MoveNext();
                        var v = r.MoveNext();
                        if (u is false || v is false)
                            return u ? 1 : v ? -1 : 0; // (int)u - (int)v

                        var c = string.Compare(l.Current, r.Current);
                        if (c != 0)
                            return c;
                    }
                }
            }

            public Items Clean()
            {
                if (Outdate)
                {
                    data = data.Where(x => x.Status is not Status.Gone).ToList();
                    Outdate = false;
                }
                return this;
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
        private readonly Items sorted;
        private readonly Items wanted;
        private bool           rename; // if the last operation is Rename

        public ItemList()
        {
            differ = DifferCreator.Create();
            source = new ();
            viewed = new ();
            sorted = new ();
            wanted = new ();
            rename = false;
        }

        public bool Add(string path)
        {
            if (Item.Create(source, differ, path, out var item) is false)
                return false;

            sorted.Add(item);

            if (differ.Strategy is not Strategy.None && item.Matched)
                wanted.Add(item);

            if (differ.Strategy is Strategy.None || item.Changed)
                viewed.Add(item);

            if (rename)
                Differ(differ);

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

            if (differ.Strategy is Strategy.None)
                sorted.Clean().AddTo(viewed);
            else
                wanted.Clean().Where(x => x.Changed).AddTo(viewed);
        }

        public void Clear()
        {
            source.Clear();
            sorted.Reset();
            wanted.Reset();
            viewed.Clear();
            rename = false;
        }

        public bool Rename()
        {
            if (differ.Strategy is Strategy.None)
                return false;

            var dropped = Enumerable.Empty<IEnumerable<Marker>>();
            foreach (var item in viewed)
            {
                var dirs = item.Directory;
                var name = item.Target;
                var prev = Path.Join(dirs, item.Source);
                var next = Path.Join(dirs, name);

                try
                {
                    Directory.Move(prev, next);
                }
                catch (DirectoryNotFoundException e)
                {
                    item.Status = Status.Lost;
                    continue;
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

            rename = true;
            return true;
        }

        public void Differ(IDiffer target)
        {
            if (rename)
            {
                rename = false;
                foreach (var item in viewed)
                    if (item.Status is Status.Done)
                        item.Status = Status.Todo;
            }

            viewed.Clear();

            if (target.Strategy is Strategy.None)
                sorted
                    .Clean()
                    .Select(x => x.Rediff(target))
                    .AddTo(viewed);
            else if (differ.Strategy == target.Strategy && differ.Pattern == target.Pattern)
                wanted
                    .Clean()
                    .Where(x => x.Rediff(target).Changed)
                    .AddTo(viewed);
            else if (differ.Strategy is Strategy.Normal && target.Strategy is Strategy.Normal && target.Pattern.StartsWith(differ.Pattern))
                wanted
                    .Reset(wanted.Where(x => x.Rediff(target).Matched).ToList())
                    .Where(x => x.Changed)
                    .AddTo(viewed);
            else
                wanted
                    .Reset()
                    .Add(sorted.Where(x => x.Rediff(target).Matched))
                    .Where(x => x.Changed)
                    .AddTo(viewed);

            differ = target;
        }

        public void Sort(OrderBy order, bool isAscending)
        {
            sorted.Sort(order, isAscending);
            viewed.Clear();
            if (differ.Strategy is Strategy.None)
                sorted
                    .AddTo(viewed);
            else if (rename)
                sorted
                    .Where(x => x.Status is not Status.Todo)
                    .AddTo(viewed);
            else
                wanted
                    .Reset()
                    .Add(sorted.Where(x => x.Matched))
                    .Where(x => x.Changed)
                    .AddTo(viewed);
        }

        public ObservableCollection<Item> View => viewed;

        public bool IsEmpty => source.IsEmpty;

        public bool IsRenamable
            => rename is false
            && differ.Strategy is not Strategy.None
            && viewed.Count is not 0;

        // References:
        // https://docs.microsoft.com/windows/communitytoolkit/controls/datagrid_guidance/group_sort_filter
    }

    internal static class EnumerableExtension
    {
        internal static void AddTo<T>(this IEnumerable<T> it, ICollection<T> that)
        {
            foreach(var one in it)
                that.Add(one);
        }
    }
}
