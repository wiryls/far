using System.Text.RegularExpressions;

namespace Fx.Diff
{
    public enum Strategy
    {
        None,
        Normal,
        Regexp,
    }

    public interface IDiffer
    {
        Patch Match(string input);

        string Pattern { get; }

        string Template { get; }

        Strategy Strategy { get; }
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

    internal readonly struct RegexDiffer : IDiffer
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

            if (matches.Count is not 0 && index != input.Length)
                build.Retain(input.AsSpan(index));

            return build.Build(input);
        }

        string IDiffer.Pattern => pattern;

        string IDiffer.Template => template;

        Strategy IDiffer.Strategy => Strategy.Regexp;
    }

    internal readonly struct PlainDiffer : IDiffer
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
            var found = false;

            // assume pattern.Length is not zero.
            for (var match = 0; match < total; match += pattern.Length)
            {
                match = input.IndexOf(pattern, match, option);
                if (match == -1)
                    break;

                if (found is false)
                    found = true;

                if (template.AsSpan().Equals(input.AsSpan(match, pattern.Length), StringComparison.Ordinal))
                    continue;

                if (match != index)
                    build.Retain(input.AsSpan(index, match - index));

                build.Delete(input.AsSpan(match, pattern.Length));

                if (template.Length != 0)
                    build.Insert(template);

                index = match + pattern.Length;
            }

            if (found && index != total)
                build.Retain(input.AsSpan(index));

            return build.Build(input);
        }

        string IDiffer.Pattern => pattern;

        string IDiffer.Template => template;

        Strategy IDiffer.Strategy => Strategy.Normal;
    }

    internal readonly struct EmptyDiffer : IDiffer
    {
        private readonly string pattern;

        public EmptyDiffer(string pattern)
        {
            this.pattern = pattern;
        }

        public Patch Match(string input) => new (input, true);

        string IDiffer. Pattern => string.Empty;

        string IDiffer. Template => pattern;

        Strategy IDiffer. Strategy => Strategy.None;
    }
}
