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
        [TestMethod]
        [DataRow(@"")]
        [DataRow(@"C:\")]
        [DataRow(@"Z:\x\")]
        public void TreeWithUnsupportedPathTest(string path)
        {
            var tree = new Tree();
            Assert.AreEqual(0, tree.Count());

            var item = tree.Add(path);
            Assert.IsNull(item);
            Assert.AreEqual(0, tree.Count());
        }

        [TestMethod]
        [DataRow(@"C:\Windows")]
        [DataRow(@"C:\Users\🌚\Downloads")]
        public void TreeWithOnePathTest(string path)
        {
            var name = Path.GetFileName(path);
            var tree = new Tree();
            Assert.AreEqual(0, tree.Count());

            tree.Clear();
            Assert.AreEqual(0, tree.Count());

            if (tree.Add(path) is not Node item)
            {
                Assert.Fail();
                return;
            }

            Assert.AreEqual(name, item.Name.ToString());
            Assert.AreEqual(1, tree.Count());

            Assert.IsFalse(tree.Rename(ref item, name, out _));
            Assert.IsTrue(tree.Rename(ref item, "?", out _));
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
            var tree = new Tree();
            Assert.AreEqual(0, tree.Count());

            var rand = new Random();
            var list = new List<Node>(paths.Length);
            foreach (var path in paths.OrderBy(_ => rand.Next()))
            {
                var item = tree.Add(path);
                Assert.IsNotNull(item);

                list.Add(item.Value);
                Assert.AreEqual(list.Count, tree.Count());
            }

            foreach (var path in paths.OrderBy(_ => rand.Next()))
            {
                Assert.IsNull(tree.Add(path));
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
