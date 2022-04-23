using System.Text.RegularExpressions;

namespace Fx.Diff
{
    public interface IDiffer
    {
        Patch Match(string input);

        string Pattern { get; }

        string Template { get; }

        bool IsEmpty { get; }
    }

    public static class DifferCreator
    {
        public static IDiffer Create()
        {
            return new EmptyDiffer();
        }

        public static IDiffer Create(string pattern, string template, bool enableIgnoreCase, bool enableRegex)
        {
            IDiffer differ;
            if /**/ (string.IsNullOrEmpty(pattern))
                differ = new EmptyDiffer(template);
            else if (enableRegex)
                differ = new RegexDiffer(pattern, template, enableIgnoreCase);
            else
                differ = new PlainDiffer(pattern, template, enableIgnoreCase);

            return differ;
        }
    }

    internal struct RegexDiffer : IDiffer
    {
        private readonly Regex regex;
        private readonly string pattern;
        private readonly string template;

        public RegexDiffer(string pattern, string template, bool ignoreCase)
        {
            var o = (ignoreCase ? RegexOptions.IgnoreCase : RegexOptions.None)
                | RegexOptions.Compiled
                | RegexOptions.CultureInvariant;

            this.regex = new Regex(pattern, o);
            this.pattern = pattern;
            this.template = template;
        }

        public Patch Match(string input)
        {
            var matches = regex.Matches(input);
            var build = new PatchBuilder(1 + 3 * matches.Count);
            var index = 0;
            foreach (Match match in matches)
            {
                var insert = match.Result(template);
                if (input.AsSpan(match.Index, match.Length).Equals(insert.AsSpan(), StringComparison.Ordinal))
                    continue;

                var retain = input.AsSpan(index, match.Index - index);
                if (retain.Length != 0)
                    build.Retain(retain);

                var delete = input.AsSpan(match.Index, match.Length);
                if (delete.Length != 0)
                    build.Delete(delete);

                if (insert.Length != 0)
                    build.Insert(insert);

                index = match.Index + match.Length;
            }

            if (index is not 0 && index != input.Length)
                build.Retain(input.AsSpan(index));

            return build.Build(input);
        }

        public string Pattern { get => pattern; }

        public string Template { get => template; }

        public bool IsEmpty { get => false; }
    }

    internal struct PlainDiffer : IDiffer
    {
        private readonly string pattern;
        private readonly string template;
        private readonly StringComparison option;

        public PlainDiffer(string pattern, string template, bool ignoreCase)
        {
            // assume pattern is different from template
            this.pattern = pattern;
            this.template = template;
            option
                = ignoreCase
                ? StringComparison.OrdinalIgnoreCase
                : StringComparison.Ordinal;
        }

        public Patch Match(string input)
        {
            var build = new PatchBuilder();
            var index = 0;
            var total = input.Length;

            // assume pattern.Length is not zero.
            for (var match = 0; match < total; match += pattern.Length)
            {
                match = input.IndexOf(pattern, match, option);
                if (match == -1)
                    break;

                if (string.Compare(input, match, template, 0, pattern.Length) is 0)
                    continue;

                if (match != index)
                    build.Retain(input.AsSpan(index, match - index));

                build.Delete(input.AsSpan(match, pattern.Length));

                if (template.Length != 0)
                    build.Insert(template);

                index = match + pattern.Length;
            }

            if (index is not 0 && index != total)
                build.Retain(input.AsSpan(index));

            return build.Build(input);
        }

        public string Pattern { get => pattern; }

        public string Template { get => template; }

        public bool IsEmpty { get => false; }
    }

    internal struct EmptyDiffer : IDiffer
    {
        private readonly string pattern;

        public EmptyDiffer(string pattern)
        {
            this.pattern = pattern;
        }

        public Patch Match(string input) => new (input, true);

        public string Pattern { get => string.Empty; }

        public string Template { get => pattern; }

        public bool IsEmpty { get => true; }
    }
}
