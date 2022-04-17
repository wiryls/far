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
    public class TreeTest
    {
        #region generator
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
            for (var i = random.Next(10); i-- > 0;)
                buffer.Append(Name(1, 16)).Append(separators[0]);
            return buffer.Append(Name(1, 16)).ToString();

            static string Name(int min, int max) => string.Concat(Enumerable
                .Repeat(chars, random.Next(1, 16))
                .Select(x => x[random.Next(chars.Length)]));
        }
        #endregion

        [TestMethod]
        [DataRow(@"")]
        [DataRow(@"C:\")]
        [DataRow(@"Z:\x\")]
        public void TreeWithUnsupportedPathTest(string path)
        {
            var tree = new Tree<int>();
            Assert.AreEqual(0, tree.Count());

            var item = tree.Add(path, 1);
            Assert.IsNull(item);
            Assert.AreEqual(0, tree.Count());
        }

        [TestMethod]
        [DataRow(@"C:\Windows")]
        [DataRow(@"C:\Users\🌚\Downloads")]
        public void TreeWithOnePathTest(string path)
        {
            var name = Path.GetFileName(path);
            var tree = new Tree<bool>();
            Assert.AreEqual(0, tree.Count());

            tree.Clear();
            Assert.AreEqual(0, tree.Count());

            if (tree.Add(path, false) is not Node<bool> item)
            {
                Assert.Fail();
                return;
            }

            Assert.AreEqual(name, item.Name.ToString());
            Assert.AreEqual(1, tree.Count());

            Assert.IsFalse(tree.Rename(item, name, out _));
            Assert.IsTrue(tree.Rename(item, "?", out _));
            Assert.AreEqual(1, tree.Count());

            Assert.IsTrue(tree.Remove(item));
            Assert.AreEqual(0, tree.Count());

            tree.Clear();
            Assert.AreEqual(0, tree.Count());
        }

        public static IEnumerable<object[]> CaseTreeWithPaths()
        {
            var xss = new string[][]
            {
                new []
                {
                    @"C:\Users\🌚\Documents\The Witcher 3",
                    @"C:\Users\🌚\Documents\GOG Galaxy\Screenshots",
                    @"C:\Users\🌚\Documents\GOG Galaxy\Screenshots\Iconoclasts",
                    @"C:\Users\🌚\Documents\GOG Galaxy\Screenshots\Hollow Knight",
                },
            };
            foreach (var xs in xss)
                yield return new object[] { xs };
        }

        [TestMethod]
        [DynamicData(nameof(CaseTreeWithPaths), DynamicDataSourceType.Method)]
        public void TreeWithPathsTest(string[] paths)
        {
            var tree = new Tree<string>();
            Assert.AreEqual(0, tree.Count());

            var rand = new Random();
            var list = new List<Node<string>>(paths.Length);
            foreach (var path in paths.OrderBy(_ => rand.Next()))
            {
                var item = tree.Add(path, string.Empty);
                Assert.IsNotNull(item);

                list.Add(item);
                Assert.AreEqual(list.Count, tree.Count());
            }

            foreach (var path in paths.OrderBy(_ => rand.Next()))
            {
                Assert.IsNull(tree.Add(path, string.Empty));
            }

            Assert.AreEqual(list.Capacity, list.Count);
            Assert.AreEqual(list.Capacity, tree.Count());

            foreach (var (item, index) in list.OrderBy(_ => rand.Next()).Select((x, y) => (x, y)))
            {
                Assert.IsTrue(tree.Remove(item));
                Assert.AreEqual(list.Count - 1 - index, tree.Count());
            }

            // References:
            // - shuffle: https://stackoverflow.com/a/108836
        }

        public static IEnumerable<object[]> CaseTreeWithRandomPaths()
        {
            for (var i = 10; i-- > 0;)
                yield return new[]
                {
                    Enumerable.Repeat(GeneratePath, 10).Select(x => x.Invoke()).Distinct().ToArray()
                };
        }

        [TestMethod]
        [DynamicData(nameof(CaseTreeWithRandomPaths), DynamicDataSourceType.Method)]
        public void TreeWithRandomPathsTest(string[] paths)
        {
            var tree = new Tree<bool>();
            foreach (var path in paths)
            {
                var node = tree.Add(path, true);
                Assert.IsNotNull(node);
                Assert.IsTrue(node.Data);
                Assert.AreEqual(Path.GetFileName(path), node.Name.ToString());

                var name = Path.GetDirectoryName(path)?.TrimEnd(separators[0]);
                var join = Path.Combine(node.Directories.ToArray());
                Assert.AreEqual(name, join);
            }
        }
    }

    [TestClass]
    public class Miscellaneous
    {
        [TestMethod]
        public void SpanTest()
        {
            var text = "hello world";
            var span = text.AsSpan();
            Assert.ReferenceEquals(text, span.ToString());

            var buff = new StringBuilder(text);
            text = buff.ToString();
            span = text.AsSpan(0, buff.Length);
            Assert.ReferenceEquals(text, span.ToString());

            Assert.IsTrue(string.Empty.AsSpan().IsEmpty);
        }
    }
}
