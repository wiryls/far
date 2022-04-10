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
        [DataRow("", ".")]
        [DataRow(".", "")]
        [DataRow(".", " ")]
        public void BasicPoolTest(string it, string that)
        {
            Assert.AreNotEqual(it, that);

            var pool = new Pool();
            Assert.IsFalse(pool.Has(it));
            Assert.IsFalse(pool.Has(that));

            pool.Put(it);
            Assert.IsTrue(pool.Has(it));
            Assert.IsFalse(pool.Has(that));

            pool.Put(it);
            Assert.IsTrue(pool.Has(it));
            Assert.IsFalse(pool.Has(that));

            pool.Pop(it);
            Assert.IsTrue(pool.Has(it));
            Assert.IsFalse(pool.Has(that));

            pool.Pop(it);
            Assert.IsFalse(pool.Has(it));
            Assert.IsFalse(pool.Has(that));
        }

        [TestMethod]
        [DataRow(@"C:\")]
        [DataRow(@"Z:\x\")]
        public void TreeWithUnsupportedPathTest(string path)
        {
            var tree = new Tree();
            Assert.AreEqual(0, tree.Count());
            Assert.AreEqual(0, tree.PoolSize);

            var item = tree.Add(path);
            Assert.IsNull(item);
            Assert.AreEqual(0, tree.Count());
            Assert.AreEqual(0, tree.PoolSize);
        }

        [TestMethod]
        [DataRow(@"C:\Windows")]
        [DataRow(@"C:\Users\🌚\Downloads")]
        public void TreeWithOnePathTest(string path)
        {
            var tree = new Tree();
            Assert.AreEqual(0, tree.Count());
            Assert.AreEqual(0, tree.PoolSize);

            tree.Clear();
            Assert.AreEqual(0, tree.Count());
            Assert.AreEqual(0, tree.PoolSize);

            var item = tree.Add(path);
            Assert.IsNotNull(item);
            Assert.AreEqual(Path.GetFileName(path), item.Source);
            Assert.AreEqual(Path.GetDirectoryName(path), item.Path);
            Assert.AreEqual(1, tree.Count());
            Assert.AreEqual(1, tree.PoolSize);

            Assert.IsFalse(tree.Rename(item, Path.GetFileName(path)));
            Assert.IsTrue(tree.Rename(item, "?"));
            Assert.AreEqual(1, tree.Count());
            Assert.AreEqual(1, tree.PoolSize);

            Assert.IsTrue(tree.Remove(item));
            Assert.AreEqual(0, tree.Count());
            Assert.AreEqual(0, tree.PoolSize);

            tree.Clear();
            Assert.AreEqual(0, tree.Count());
            Assert.AreEqual(0, tree.PoolSize);
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
            Assert.AreEqual(0, tree.PoolSize);

            var rand = new Random();
            var list = new List<Item>(paths.Length);
            foreach (var path in paths.OrderBy(_ => rand.Next()))
            {
                var item = tree.Add(path);
                Assert.IsNotNull(item);

                list.Add(item);
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

            Assert.AreEqual(0, tree.PoolSize);

            // References:
            // - shuffle: https://stackoverflow.com/a/108836
        }

        [TestMethod]
        public void SpanConvertionTest()
        {
            var text = "hello world";
            var span = text.AsSpan();
            ReferenceEquals(text, span.ToString());

            var buff = new StringBuilder(text);
            text = buff.ToString();
            span = text.AsSpan(0, buff.Length);
            ReferenceEquals(text, span.ToString());
        }
    }
}
