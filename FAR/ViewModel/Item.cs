using Fx.Diff;
using Fx.List;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
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

    public class Item
    {
        private Status status = Status.Todo;
        private Marker marker = Marker.Empty;
        private Change change = Change.Empty;

        public Item Rediff(IDiffer differ)
        {
            change = differ.Match(change.Source);
            return this;
        }

        public bool Rename(Tree<Item> tree, string name, out IEnumerable<Marker> drop)
        {
            var done = tree.Rename(ref marker, name, out drop);
            if (done)
                change = new(marker.Name.ToString());
            return done;
        }

        public Status Status
        {
            get => status;
            set => status = value;
        }

        public Change Preview => change;

        public string Directory => marker.Directory.ToString();

        internal Marker Marker => marker;

        internal string Source => change.Source;

        internal string Target => change.Target;

        public static bool Create(Tree<Item> tree, IDiffer diff, string path, out Item item)
        {
            item = new Item();
            if (tree.Add(path, item) is Marker mark)
            {
                item.marker = mark;
                item.change = diff.Match(mark.Name.ToString());
                return true;
            }
            return false;
        }
    }

    internal struct Items
    {
        // status
        private IDiffer differ;
        private readonly Tree<Item> source;
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
            if (Item.Create(source, differ, path, out var item) is false)
                return false;

            sorted.Add(item);

            if (item.Preview.Matched)
                wanted.Add(item);

            if (item.Preview.Changed || differ.IsEmpty)
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
            if (differ.IsEmpty is false)
                return false;

            var dropped = Enumerable.Empty<IEnumerable<Marker>>();
            foreach (var item in viewed)
            {
                var last = item.Marker.AbsolutePath.ToString();
                var name = item.Target;
                var next = Path.Join(item.Directory, name);

                var info = null as FileInfo;
                try
                {
                    info = new (last);
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
                    // info.MoveTo(next);
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

            foreach (var node in dropped.SelectMany(x => x).Select(x => x.Value))
            {
                if (viewed.Remove(node))
                    node.Status = Status.Gone;

                Debug.WriteLine($"Remove '${node.Source}'");
            }

            return true;
        }

        public bool Differ(IDiffer target)
        {
            if (update)
            {
                sorted = sorted.Where(NotGone).ToList();
                wanted = wanted.Where(NotGone).ToList();
                update = false;
                static bool NotGone(Item item) => item.Status is not Status.Gone;
            }

            if (target.IsEmpty)
            {
                viewed.Clear();
                viewed = new (sorted);
            }
            else if (Equals(differ.GetType(), target.GetType()) && differ.Pattern == target.Pattern)
            {

            }

            var listed
                = Equals(differ.GetType(), target.GetType()) && differ.Pattern == target.Pattern
                ? wanted
                : sorted
                ;

            differ = target;
            viewed.Clear();
            foreach (var item in differ.IsEmpty ? sorted : listed.Where(x => x.Rediff(target).Preview.Changed))
                viewed.Add(item);

            wanted = viewed.ToList();
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
