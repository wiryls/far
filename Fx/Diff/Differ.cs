using System.Text.RegularExpressions;

namespace Fx.Diff
{
    public delegate Patch Differ(string input);

    public static class DifferCreator
    {
        public static Differ Create(string pattern, string template, bool enableIgnoreCase, bool enableRegex)
        {
            var differ = null as IDiffer;
            if /**/ (string.IsNullOrEmpty(pattern) || (!enableIgnoreCase && !enableRegex && pattern == template))
                differ = new EmptyDiffer();
            else if (enableRegex)
                differ = new RegexDiffer(pattern, template, enableIgnoreCase);
            else
                differ = new PlainDiffer(pattern, template, enableIgnoreCase);

            return differ.Transform;
        }
    }

    internal interface IDiffer
    {
        Patch Transform(string input);
    }

    internal class RegexDiffer : IDiffer
    {
        private readonly Regex pattern;
        private readonly string template;

        public RegexDiffer(string pattern, string template, bool ignoreCase)
        {
            var o = (ignoreCase ? RegexOptions.IgnoreCase : RegexOptions.None)
                | RegexOptions.Compiled
                | RegexOptions.CultureInvariant;

            this.pattern = new Regex(pattern, o);
            this.template = template;
        }

        public Patch Transform(string input)
        {
            var matches = pattern.Matches(input);
            var build = new PatchBuilder(1 + 3 * matches.Count);
            var index = 0;
            foreach (Match match in matches)
            {
                var insert = match.Result(template);
                if (string.Compare(input, match.Index, insert, 0, match.Length) == 0)
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

            if (index != input.Length)
                build.Retain(input.AsSpan(index));

            return build.Build(input);
        }
    }

    internal class PlainDiffer : IDiffer
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

        public Patch Transform(string input)
        {
            var build = new PatchBuilder();
            var index = 0;
            var total = input.Length;

            for (var match = 0; match < total; match += pattern.Length)
            {
                match = input.IndexOf(pattern, match, option);
                if (match == -1)
                    break;

                if (string.Compare(input, match, template, 0, template.Length) == 0)
                    continue;

                if (match != index)
                    build.Retain(input.AsSpan(index, match - index));

                build.Delete(pattern);

                if (template.Length != 0)
                    build.Insert(template);

                index = match + pattern.Length;
            }

            if (index != total)
                build.Retain(input.AsSpan(index));

            return build.Build(input);
        }
    }

    internal class EmptyDiffer : IDiffer
    {
        public Patch Transform(string input) => new (input);
    }
}
