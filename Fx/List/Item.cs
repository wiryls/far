namespace Fx.List
{
    using Change = Diff.Diff;

    public enum Status
    {
        Todo,
        Done,
        Fail,
        Lost,
    }

    public class Item
    {
        private Status status;
        private Change change;
        private readonly string parent;

        internal Item(string parent)
        {
            this.status = Status.Todo;
            this.change = new();
            this.parent = parent;
        }

        public Status Stat { get => status; set => status = value; } // task status
        public Change View { get => change; set => change = value; } // preview
        public string Path { get => parent; } // path to parent directory
        public string Source { get => change.Source; } // the source name
        public string Target { get => change.Target; } // the target name
    }
}
