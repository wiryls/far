using Microsoft.VisualStudio.TestTools.UnitTesting;
using Fx.Diff;

namespace FxTests
{
    [TestClass]
    public class DIfferTest
    {
        [DataRow("", "", "", "")]
        [TestMethod]
        public void TestDifferWithRegex(string pattern, string template, string input, string expected)
        {
            var creator = new DifferCreator
            {
                Pattern = pattern,
                Template = template,
                EnableRegexp = true,
                EnableIgnoreCase = false,
            };

            var differ = creator.Create();
            var output = differ.Transform(input);
            Assert.AreEqual(expected, output.Target);
        }
    }

    // References:
    // https://docs.microsoft.com/en-us/visualstudio/test/walkthrough-creating-and-running-unit-tests-for-managed-code
    // https://docs.microsoft.com/en-us/dotnet/core/testing/unit-testing-with-mstest
}
