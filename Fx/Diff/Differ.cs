using System.Text.RegularExpressions;

namespace Fx.Diff
{
    public delegate Diff Differ(string input);

    public static class DifferCreator
    {
        public static Differ Create(string pattern, string template, bool enableIgnoreCase, bool enableRegex)
        {
            var differ = null as IDiffer;
            if /**/ (string.IsNullOrEmpty(pattern) || (!enableIgnoreCase && !enableRegex && pattern == template))
                differ = new EmptyDiffer();
            else if (enableRegex)
                differ = new Tidier<RegexDiffer>(new (pattern, template, enableIgnoreCase));
            else
                differ = new Tidier<PlainDiffer>(new (pattern, template, enableIgnoreCase));

            return differ.Transform;
        }
    }

    internal interface IDiffer
    {
        Diff Transform(string input);
    }

    internal class Tidier<T> : IDiffer where T : IDiffer
    {
        private readonly T differ;

        public Tidier(T differ)
        {
            this.differ = differ;
        }

        public Diff Transform(string input)
        {
            var output = differ.Transform(input);
            if (output.Count > 1)
            {
                if (output.Unchanged)
                    output = new Diff(1) { { Action.Retain, input } };
                else
                    output.Capacity = output.Count;
            }
            return output;
        }
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

        public Diff Transform(string input)
        {
            var matches = pattern.Matches(input);
            var output = new Diff(1 + 3 * matches.Count);
            var index = 0;
            foreach (Match match in matches)
            {
                var insert = match.Result(template);
                if (string.Compare(input, match.Index, insert, 0, match.Length) == 0)
                    continue;

                var retain = input[index..match.Index];
                if (retain.Length != 0)
                    output.Add(Action.Retain, retain);

                var delete = input.Substring(match.Index, match.Length);
                if (delete.Length != 0)
                    output.Add(Action.Delete, delete);

                if (insert.Length != 0)
                    output.Add(Action.Insert, insert);

                index = match.Index + match.Length;
            }

            if (index != input.Length)
                output.Add(Action.Retain, input[index..]);

            return output;
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

        public Diff Transform(string input)
        {
            var output = new Diff();
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
                    output.Add(Action.Retain, input[index..match]);

                output.Add(Action.Delete, pattern);

                if (template.Length != 0)
                    output.Add(Action.Insert, template);

                index = match + pattern.Length;
            }

            if (index != total)
                output.Add(Action.Retain, input[index..]);

            return output;
        }
    }

    internal class EmptyDiffer : IDiffer
    {
        public Diff Transform(string input) => new (1)
        {
            { Action.Retain, input }
        };
    }
}
