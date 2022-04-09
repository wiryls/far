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
        private string source;
        private readonly string parent;

        internal Item(string parent, string source)
        {
            this.status = Status.Todo;
            this.change = new();
            this.source = source;
            this.parent = parent;
        }

        public Status Stat { get => status; set => status = value; } // task status
        public Change View { get => change; set => change = value; } // preview
        public string Path { get => parent; } // path to parent directory
        public string Source { get => source; internal set => source = value; } // the source name
        public string Target { get => change.Target; } // the target name
    }
}
