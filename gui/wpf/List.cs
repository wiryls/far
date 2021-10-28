using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace WPF
{
    using Event  = NotifyCollectionChangedEventArgs;
    using Action = NotifyCollectionChangedAction;

    public class Item
    {
        public string Stat; // or enum
        public string Path; // rich text
        public string View; // rich text
    }

    /// <summary>
    /// List is a proxy for our real item list.
    /// </summary>
    public class List : INotifyCollectionChanged, IList<Item>
    {
        // references:
        // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.specialized.inotifycollectionchanged
        // - https://docs.microsoft.com/en-us/dotnet/api/system.collections.ilist

        public Item this[int index]
        {
            get => throw new System.NotImplementedException();
            set => throw new System.NotImplementedException();
        }

        public int Count => throw new System.NotImplementedException();

        public bool IsReadOnly => throw new System.NotImplementedException();

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        public void Add(Item item)
        {
            CollectionChanged?.Invoke(this, new Event(Action.Add));

            throw new System.NotImplementedException();
        }

        public void Clear()
        {
            throw new System.NotImplementedException();
        }

        public bool Contains(Item item)
        {
            throw new System.NotImplementedException();
        }

        public void CopyTo(Item[] array, int arrayIndex)
        {
            throw new System.NotImplementedException();
        }

        public IEnumerator<Item> GetEnumerator()
        {
            throw new System.NotImplementedException();
        }

        public int IndexOf(Item item)
        {
            throw new System.NotImplementedException();
        }

        public void Insert(int index, Item item)
        {
            throw new System.NotImplementedException();
        }

        public bool Remove(Item item)
        {
            throw new System.NotImplementedException();
        }

        public void RemoveAt(int index)
        {
            throw new System.NotImplementedException();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            throw new System.NotImplementedException();
        }
    }
}
