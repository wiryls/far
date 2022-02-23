using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using Fx.Diff;

namespace FxTests
{
    [TestClass]
    public class DIfferTest
    {
        public static IEnumerable<object[]> CaseDifferWithRegex()
        {
            var xss = new List<(string, string, List<(string, string)>)>
            {
                ( "", "", new() { ( "", "" ) }),
                ( "log", "ln", new() {
                    ( "ln(e) = 1", "ln(e) = 1" ),
                    ( "\tlog.d(\"?\")", "\tln.d(\"?\")" ),
                    ( "log(😅) = 💧log(😄)", "ln(😅) = 💧ln(😄)" ) }
                ),
                ( @"^(.*)(\r?\n\1)+$", @"$1", new () {
                    ( "だんご だんご だんご だんご だんご だんご 大家族\n"
                    + "だんご だんご だんご だんご だんご だんご 大家族"
                    , "だんご だんご だんご だんご だんご だんご 大家族" ),
                    ( "不要回答！\n不要回答！\n不要回答！", "不要回答！" )}
                ),
            };

            foreach (var xs in xss)
                foreach (var x in xs.Item3)
                    yield return new object[] { (xs.Item1, xs.Item2, x.Item1, x.Item2) };
        }

        [TestMethod]
        [DynamicData(nameof(CaseDifferWithRegex), DynamicDataSourceType.Method)]
        public void TestDifferWithRegex((string pattern, string template, string input, string expected) x)
        {
            var creator = new DifferCreator
            {
                Pattern = x.pattern,
                Template = x.template,
                EnableRegexp = true,
                EnableIgnoreCase = false,
            };

            var differ = creator.Create();
            var output = differ.Transform(x.input);
            Assert.AreEqual(x.expected, output.Target);
        }

        public static IEnumerable<object[]> CaseDifferWithPlain()
        {
            var xss = new List<(string, string, List<(string, string)>)>
            {
                ( "", "", new() { ( "", "" ) }),
                ( "true", "false", new() {
                    ( "return true", "return false" ),
                    ( "if (true == false) {", "if (false == false) {" ) }
                ),
            };

            foreach (var xs in xss)
                foreach (var x in xs.Item3)
                    yield return new object[] { (xs.Item1, xs.Item2, x.Item1, x.Item2) };
        }

        [TestMethod]
        [DynamicData(nameof(CaseDifferWithPlain), DynamicDataSourceType.Method)]
        public void TestDifferWithPlain((string pattern, string template, string input, string expected) x)
        {
            var creator = new DifferCreator
            {
                Pattern = x.pattern,
                Template = x.template,
                EnableRegexp = false,
                EnableIgnoreCase = false,
            };

            var differ = creator.Create();
            var output = differ.Transform(x.input);
            Assert.AreEqual(x.expected, output.Target);
        }
    }

    // References:
    // https://docs.microsoft.com/en-us/visualstudio/test/walkthrough-creating-and-running-unit-tests-for-managed-code
    // https://docs.microsoft.com/en-us/dotnet/core/testing/unit-testing-with-mstest
    // https://stackoverflow.com/a/59806993
}
