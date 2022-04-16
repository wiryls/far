namespace Fx.List
{
    public readonly struct Node
    {
        private readonly string       path;
        private readonly (int, int)   name;
        private readonly (int, int)[] dirs;
        private readonly static char[] separators = new[]
        {
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar
        };

        public Node(string absolute)
        {
            var expect = absolute.Count(x => separators.Contains(x));
            var buffer = new (int, int)[expect];

            var count = 0;
            var match = 0;
            var total = absolute.Length;
            while (match < total)
            {
                var found = absolute.IndexOfAny(separators, match);
                if (found < match)
                    break;

                buffer[count++] = (match, found - match);
                match = found + 1;
            }

            path = absolute;
            name = (match, total - match);
            dirs = buffer.Where(x => x.Item2 is 0).Any()
                 ? buffer.Take(count).Where(x => x.Item2 is not 0).ToArray()
                 : buffer;
        }

        public ReadOnlySpan<char> AbsolutePath => path;

        public ReadOnlySpan<char> Name => name.Item2 is 0
            ? string.Empty.AsSpan()
            : path.AsSpan(name.Item1, name.Item2);

        public ReadOnlySpan<char> Directory => path.Length is 0
            ? string.Empty.AsSpan()
            : path.AsSpan(0, name.Item1)
            ;

        public IEnumerable<ReadOnlyMemory<char>> Directories
        {
            get
            {
                var local = path;
                return dirs.Select(x => local.AsMemory(x.Item1, x.Item2));
            }
        }

        // References for ReadOnlySpan and ReadOnlyMemory:
        // https://docs.microsoft.com/en-us/dotnet/standard/memory-and-spans/memory-t-usage-guidelines
        // https://stackoverflow.com/a/66447142
        // https://blog.ndepend.com/improve-c-code-performance-with-spant/
    }
}
