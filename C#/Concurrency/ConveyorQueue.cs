using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Concurrency
{
    class ConveyorQueue<T>
    {
        private readonly object _accessMutex = new object();

        private Queue<T> _items;
        public int Count { get { return this._items.Count; } }

        public void Enqueue(T item)
        {
            lock (this._accessMutex)
            {
                this._items.Enqueue(item);
            }
        }

        public T Dequeue()
        {
            lock (this._accessMutex)
            {
                return this._items.Dequeue();
            }
        }

        public T Peek()
        {
            lock (this._accessMutex)
            {
                return this._items.Peek();
            }
        }

        public ConveyorQueue(int capacity)
        {
            this._items = new Queue<T>(capacity);
        }
    }
}
