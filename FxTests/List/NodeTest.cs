using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using Fx.List;
using System;
using System.Text;

namespace FxTests.List
{
    [TestClass]
    public class NodeTest
    {
        private static readonly Random random = new();
        private static readonly string chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ" +
            "abcdefghijklmnopqrstuvwxyz" +
            "1234567890" +
            " ._-";
        private readonly static char[] separators = new[]
        {
            Path.DirectorySeparatorChar,
            Path.AltDirectorySeparatorChar
        };

        private static string GeneratePath()
        {
            var buffer = new StringBuilder(@"C:").Append(separators[0]);
            for (var i = random.Next(10); i --> 0; )
                buffer.Append(Name(1, 16)).Append(separators[0]);
            return buffer.Append(Name(1, 16)).ToString();

            static string Name(int min, int max) => string.Concat(Enumerable
                .Repeat(chars, random.Next(1, 16))
                .Select(x => x[random.Next(chars.Length)]));
        }

        public static IEnumerable<object[]> CaseRandomPath()
        {
            for (var i = 100; i-- > 0; )
                yield return new [] { GeneratePath() };
        }

        [TestMethod]
        [DynamicData(nameof(CaseRandomPath), DynamicDataSourceType.Method)]
        public void RandomPathTest(string path)
        {
            var node = new Node(path);
            Assert.AreEqual(path, node.AbsolutePath.ToString());
            Assert.AreEqual(Path.GetFileName(path), node.Name.ToString());

            var name = Path.GetDirectoryName(path)?.TrimEnd(separators[0]);
            var join = Path.Combine(node.Directories.Select(x => x.ToString()).ToArray());
            Assert.AreEqual(name, join);
        }
    }
}
