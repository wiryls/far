using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using Fx.Diff;

namespace FxTests.Diff
{
    [TestClass]
    public class DifferTest
    {
        public static IEnumerable<object[]> CaseDifferWithRegex()
        {
            var xss = new (string, string, (string, string)[])[]
            {
                ( "", "", new[] {
                    ( "", "" ),
                    ( "?", "?" ),
                    ( "!!", "!!" ) }
                ),
                ( "log", "ln", new[] {
                    ( "ln(e) = 1", "ln(e) = 1" ),
                    ( "\tlog.d(\"?\")", "\tln.d(\"?\")" ),
                    ( "log(😅) = 💧log(😄)", "ln(😅) = 💧ln(😄)" ) }
                ),
                ( @"(\d)", "$1.618", new[] {
                    ( "", "" ),
                    ( "0", "0.618" ),
                    ( "0+0", "0.618+0.618" ),
                    ( "eof", "eof" ) }
                ),
                ( @"^(.*)(\r?\n\1)+$", @"$1", new [] {
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
            var differ = DifferCreator.Create(x.pattern, x.template, false, true);
            Assert.AreEqual(x.pattern, differ.Pattern);
            Assert.AreEqual(x.template, differ.Template);

            var output = differ.Match(x.input);
            Assert.AreEqual(x.input, output.Source);
            Assert.AreEqual(x.expected, output.Target);
        }

        public static IEnumerable<object[]> CaseDifferWithPlain()
        {
            var xss = new (string, string, (string, string)[])[]
            {
                ( "", "", new[] { ( "", "" ) }),
                ( "true", "false", new[] {
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
            var differ = DifferCreator.Create(x.pattern, x.template, false, false);
            Assert.AreEqual(x.pattern, differ.Pattern);
            Assert.AreEqual(x.template, differ.Template);

            var output = differ.Match(x.input);
            Assert.AreEqual(x.input, output.Source);
            Assert.AreEqual(x.expected, output.Target);
        }
    }

    // References:
    // https://docs.microsoft.com/en-us/visualstudio/test/walkthrough-creating-and-running-unit-tests-for-managed-code
    // https://docs.microsoft.com/en-us/dotnet/core/testing/unit-testing-with-mstest
    // https://stackoverflow.com/a/59806993
}
