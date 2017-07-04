using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Concurrency
{
    class Conveyor
    {
        public const int NUMBER_OF_SPLIT_WORK = 10;
        public static ConveyorQueue<List<string>> DataChunks { get; set; } = new ConveyorQueue<List<string>>(NUMBER_OF_SPLIT_WORK);

        private ConveyorContributor _contributor = new ConveyorContributor();
        private List<ConveyorConsumer> _consumers;

        private void MergeDictionary<K>(ref ConveyorQueue<Dictionary<K, int>> container, ref Dictionary<K, int> mainDict) 
        {
            while (container.Count > 0)
            {
                Dictionary<K, int> innerDict = container.Dequeue();
                foreach (K key in innerDict.Keys)
                {
                    if (mainDict.ContainsKey(key))
                    {
                        mainDict[key] += innerDict[key];
                    }
                    else
                    {
                        mainDict.Add(key, innerDict[key]);
                    }
                }
            }
        }

        private object _conditionalDataChunksMutex = new object();
        public static ConditionVariable DataChunksState = new ConditionVariable();
        public static bool AllDataRead { get; set; } = false;
        public void Run(ref Dictionary<string, int> globalWordsCount)
        {
            ConveyorQueue<Dictionary<string, int>> wordsCount = new ConveyorQueue<Dictionary<string, int>>(NUMBER_OF_SPLIT_WORK);

            Thread[] counting_threads = new Thread[this._consumers.Count + 1];

            // Instead of 1000 there should be length of the file divided by number of consumers
            counting_threads[0] = new Thread(new ThreadStart(delegate { this._contributor.Contribute("data50MB.txt", 100000); }));
            counting_threads[0].Start();

            Stopwatch watch = Stopwatch.StartNew();

            Console.WriteLine("Consumering process started...");

            int free_thread_slot_index = 1;
            while (free_thread_slot_index < counting_threads.Length)
            {
                lock (this._conditionalDataChunksMutex)
                {
                    if (DataChunks.Count == 0)
                    {
                        Console.WriteLine("Waiting for data to be contributed");
                        DataChunksState.Wait(this._conditionalDataChunksMutex);
                    }

                    Console.WriteLine($"Data was contibuted { DataChunks.Count }");

                    List<string> consumableDataChunk = Conveyor.DataChunks.Dequeue();
                    counting_threads[free_thread_slot_index] = new Thread(
                                                                    new ThreadStart(
                                                                        delegate {
                                                                            this._consumers[free_thread_slot_index - 1].Consume(
                                                                                ref wordsCount, 
                                                                                ref consumableDataChunk
                                                                                );
                                                                        }));
                    counting_threads[free_thread_slot_index].Start();
                    ++free_thread_slot_index;

                    Console.WriteLine($"Data was consumed { DataChunks.Count }");
                }

                if (AllDataRead)
                {
                    break;
                }
            }

            for (int i = 0; i < free_thread_slot_index; i++)
            {
                counting_threads[i].Join();
            }

            this.MergeDictionary<string>(ref wordsCount, ref globalWordsCount);

            long calculationTime = watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L));
            watch.Stop();

            Console.WriteLine($"Total calculation time: { calculationTime }ms");
        }

        public Conveyor(int numberOfConsumers)
        {
            this._consumers = new List<ConveyorConsumer>((int)numberOfConsumers);
            for (int i = 0; i < numberOfConsumers; i++)
            {
                _consumers.Add(new ConveyorConsumer());
            }
        }
    }
}
