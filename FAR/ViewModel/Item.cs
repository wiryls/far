﻿using Fx.Diff;
using Fx.List;
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
            // empty
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
        // status
        private IDiffer differ;
        private readonly Tree<Item> source;
        private readonly ObservableCollection<Item> viewed;

        // buffer
        private List<Item> sorted;
        private List<Item> wanted;
        private bool remove;

        public Items()
        {
            differ = DifferCreator.Create();
            source = new();
            viewed = new();
            sorted = new();
            wanted = new();
            remove = false;
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

        public bool Remove(int index)
        {
            if (viewed.Count <= index || index < 0)
                return false;

            var item = viewed[index];
            item.Status = Status.Gone;

            remove = true;
            viewed.RemoveAt(index);
            return source.Remove(item.Marker);
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
            if (remove)
            {
                sorted = sorted.Where(NotGone).ToList();
                wanted = wanted.Where(NotGone).ToList();
                remove = false;

                static bool NotGone(Item item) => item.Status is not Status.Gone;
            }

            viewed.Clear();
            if (target.IsEmpty)
            {
                foreach (var item in sorted)
                    viewed.Add(item.Rediff(target));
            }
            else if (Equals(differ.GetType(), target.GetType()) && differ.Pattern == target.Pattern)
            {
                foreach (var item in wanted.Where(x => x.Rediff(target).Changed))
                    viewed.Add(item);
            }
            else
            {
                wanted.Clear();
                wanted.AddRange(sorted.Where(x => x.Rediff(target).Matched));
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

        public ObservableCollection<Item> View { get => viewed; }
    }
}
