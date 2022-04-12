using Change = Fx.Diff.Patch;

namespace Fx.List
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
        private Change change;
        private readonly string parent;

        internal Item(string parent, string source)
        {
            this.status = Status.Todo;
            this.change = new Change(source);
            this.parent = parent;
        }

        public Status Stat
        {
            get => status;
            set => status = value;
        }

        public Change View
        {
            get => change;
            set => change = value;
        }

        public string Path
        {
            get => parent;
        }

        public string Source
        {
            get => change.Source;
            internal set => change = new Change(value);
        }
        
        public string Target
        {
            get => change.Target;
        }
    }
}
