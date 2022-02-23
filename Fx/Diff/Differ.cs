using System.Text.RegularExpressions;

namespace Fx.Diff
{
    public interface IDiffer
    {
        Diff Transform(string input);
    }

    public class DifferCreator
    {
        public DifferCreator()
        {
            Pattern = string.Empty;
            Template = string.Empty;
            EnableRegexp = false;
            EnableIgnoreCase = true;
        }

        public IDiffer Create()
        {
            if (string.IsNullOrEmpty(Pattern))
                return new EmptyDiffer();

            if (EnableRegexp)
                return new RegexDiffer(Pattern, Template, EnableIgnoreCase);

            return new PlainDiffer(Pattern, Template, EnableIgnoreCase);
        }

        public string Pattern { get; set; }

        public string Template { get; set; }

        public bool EnableRegexp { get; set; }

        public bool EnableIgnoreCase { get; set; }
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
            var output = new Diff(1 + matches.Count * 3);
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
                output.Add(Action.Insert, input[index..]);

            output.Capacity = output.Count;
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

            while (index < total)
            {
                var match = input.IndexOf(pattern, index, option);
                if (match == -1)
                    break;

                if (match != index)
                    output.Add(Action.Retain, input[index..match]);

                output.Add(Action.Delete, pattern);

                if (template.Length != 0)
                    output.Add(Action.Insert, template);

                index = match + pattern.Length;
            }

            if (index != total)
                output.Add(Action.Insert, input[index..]);

            output.Capacity = output.Count;
            return output;
        }
    }

    internal class EmptyDiffer : IDiffer
    {
        public Diff Transform(string input)
        {
            return new Diff(1)
            {
                { Action.Retain, input }
            };
        }
    }
}
