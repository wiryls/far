using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;
using System.IO;
using Fx.List;

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
        [DataRow(@"C:\Users\🌚\Downloads")]
        public void BasicTreeTest(string path)
        {
            var tree = new Tree();
            Assert.AreEqual(0, tree.Count());

            tree.Clear();
            Assert.AreEqual(0, tree.Count());

            var item = tree.Add(path);
            Assert.IsNotNull(item);
            Assert.AreEqual(Path.GetFileName(path), item.Source);
            Assert.AreEqual(Path.GetDirectoryName(path), item.Path);
            Assert.AreEqual(1, tree.Count());

            Assert.IsFalse(tree.Rename(item, Path.GetFileName(path)));
            Assert.IsTrue(tree.Rename(item, "?"));
            Assert.AreEqual(1, tree.Count());

            Assert.IsTrue(tree.Remove(item));
            Assert.AreEqual(0, tree.Count());

            tree.Clear();
            Assert.AreEqual(0, tree.Count());
        }
    }
}
