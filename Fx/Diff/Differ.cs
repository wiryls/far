using System;
using System.Text.RegularExpressions;

namespace Fx.Diff
{
    internal interface IDiffer
    {
        Diff Transform(string input);
    }

    internal class RegexDiffer : IDiffer
    {
        private readonly Regex pattern;
        private readonly string tempalte;

        public RegexDiffer(string pattern, string template, bool ignoreCase)
        {
            var o = (ignoreCase ? RegexOptions.IgnoreCase : RegexOptions.None)
                | RegexOptions.Compiled;

            this.pattern = new Regex(pattern, o);
            tempalte = template;
        }

        public Diff Transform(string input)
        {
            var output = new Diff();
            var index = 0;
            foreach (Match match in pattern.Matches(input))
            {
                var insert = match.Result(tempalte);
                if (string.Compare(input, match.Index, insert, 0, match.Length) == 0)
                    continue;

                var retain = input.Substring(index, match.Index);
                if (retain.Length != 0)
                    output.Add(new Operation { Text = retain, Type = Operation.Action.Retain });

                var delete = input.Substring(match.Index, match.Length);
                if (delete.Length != 0)
                    output.Add(new Operation { Text = delete, Type = Operation.Action.Delete });

                if (insert.Length != 0)
                    output.Add(new Operation { Text = insert, Type = Operation.Action.Insert });

                index = match.Index + match.Length;
            }

            if (index != input.Length)
                output.Add(new Operation { Text = input[index..], Type = Operation.Action.Insert });

            return output;
        }
    }

    internal class PlainDiffer : IDiffer
    {
        public Diff Transform(string input)
        {
            throw new NotImplementedException();
        }
    }
}
