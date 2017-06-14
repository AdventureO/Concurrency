using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.IO;

namespace Concurrency
{
    public class TextDataController
    {
        public const int AVERAGEWORDSCOUNT = 9000;

        private object _lockObject = new object();
        private void CountWordsFromFileInParallel(string fileName, ref Dictionary<string, int> globalWordsCount, int startingLineIndex, Action callbackAction = null)
        {
            Dictionary<string, int> localWordsCount = new Dictionary<string, int>(globalWordsCount.Count);
            int startingWordCount = 1;

            try
            {
                using (StreamReader streamReader = new StreamReader(fileName))
                {
                    StreamReaderUtility.SkipToLine(streamReader, startingLineIndex);

                    string fileLine = streamReader.ReadLine();
                    string[] fileLineWordsBuffer;
                    while (fileLine != null)
                    {
                        fileLine = new string(fileLine.Where(c => !char.IsPunctuation(c)).ToArray());
                        if (fileLine.Trim() != string.Empty)
                        {
                            fileLineWordsBuffer = fileLine.Split(' ');
                            for (int i = 0; i < fileLineWordsBuffer.Length; i++)
                            {
                                if (localWordsCount.ContainsKey(fileLineWordsBuffer[i]))
                                {
                                    localWordsCount[fileLineWordsBuffer[i]]++;
                                }
                                else
                                {
                                    localWordsCount.Add(fileLineWordsBuffer[i], startingWordCount);
                                }
                            }
                        }

                        fileLine = streamReader.ReadLine();
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("The file could not be read");
                Console.WriteLine(e.Message);
                throw;
            }

            lock (this._lockObject)
            {
                foreach (string word in localWordsCount.Keys)
                {
                    if (globalWordsCount.ContainsKey(word))
                    {
                        globalWordsCount[word] += localWordsCount[word];
                    }
                    else
                    {
                        globalWordsCount.Add(word, localWordsCount[word]);
                    }
                }
            }

            callbackAction?.Invoke();
        }

        public Dictionary<string, int> GetWordsCountInParallel(string fileName, int threadsQuantity)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGEWORDSCOUNT);

            Thread[] counting_threads = new Thread[threadsQuantity];

            int fileDataChunkIndexStep = File.ReadLines(fileName).Count() / threadsQuantity;

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_threads[i] = new Thread(new ThreadStart(delegate { this.CountWordsFromFileInParallel(fileName, ref wordsCount, fileDataChunkIndexStep * i); }));
                counting_threads[i].Start();
            }

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_threads[i].Join();
            }

            return wordsCount;
        }

        private void CountWordsFromFile(string fileName, ref Dictionary<string, int> wordsCount, Action callbackAction = null)
        {
            int startingWordCount = 1;

            try
            {
                using (StreamReader streamReader = new StreamReader(fileName))
                {
                    string fileLine = streamReader.ReadLine();
                    string[] fileLineWordsBuffer;
                    while (fileLine != null)
                    {
                        fileLine = new string(fileLine.Where(c => !char.IsPunctuation(c)).ToArray());
                        if (fileLine.Trim() != string.Empty)
                        {
                            fileLineWordsBuffer = fileLine.Split(' ');
                            for (int i = 0; i < fileLineWordsBuffer.Length; i++)
                            {
                                if (wordsCount.ContainsKey(fileLineWordsBuffer[i]))
                                {
                                    wordsCount[fileLineWordsBuffer[i]]++;
                                }
                                else
                                {
                                    wordsCount.Add(fileLineWordsBuffer[i], startingWordCount);
                                }
                            }
                        }

                        fileLine = streamReader.ReadLine();
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine("The file could not be read");
                Console.WriteLine(e.Message);
                throw;
            }

            callbackAction?.Invoke();
        }

        private Task<Dictionary<string, int>> CountWordsFromFileAsync(string fileName, int startingLineIndex, Action callbackAction = null)
        {
            return Task.Factory.StartNew(() =>
            {
                Dictionary<string, int> localWordsCount = new Dictionary<string, int>(AVERAGEWORDSCOUNT);
                int startingWordCount = 1;

                try
                {
                    using (StreamReader streamReader = new StreamReader(fileName))
                    {
                        StreamReaderUtility.SkipToLine(streamReader, startingLineIndex);

                        string fileLine = streamReader.ReadLine();
                        string[] fileLineWordsBuffer;
                        while (fileLine != null)
                        {
                            fileLine = new string(fileLine.Where(c => !char.IsPunctuation(c)).ToArray());
                            if (fileLine.Trim() != string.Empty)
                            {
                                fileLineWordsBuffer = fileLine.Split(' ');
                                for (int i = 0; i < fileLineWordsBuffer.Length; i++)
                                {
                                    if (localWordsCount.ContainsKey(fileLineWordsBuffer[i]))
                                    {
                                        localWordsCount[fileLineWordsBuffer[i]]++;
                                    }
                                    else
                                    {
                                        localWordsCount.Add(fileLineWordsBuffer[i], startingWordCount);
                                    }
                                }
                            }

                            fileLine = streamReader.ReadLine();
                        }
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine("The file could not be read");
                    Console.WriteLine(e.Message);
                    throw;
                }

                callbackAction?.Invoke();

                return localWordsCount;
            });
        }

        public Dictionary<string, int> GetWordsCountAsync(string fileName, int threadsQuantity)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGEWORDSCOUNT);

            Task<Dictionary<string, int>>[] counting_tasks = new Task<Dictionary<string, int>>[threadsQuantity];

            int fileDataChunkIndexStep = File.ReadLines(fileName).Count() / threadsQuantity;

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_tasks[i] = this.CountWordsFromFileAsync(fileName, fileDataChunkIndexStep * i);
            }

            Task<Dictionary<string, int>[]> results = Task.WhenAll(counting_tasks);

            try
            {
                results.Wait();
            }
            catch (AggregateException)
            {}

            if (results.Status == TaskStatus.RanToCompletion)
            {
                foreach (var result in results.Result)
                {
                    foreach (string word in result.Keys)
                    {
                        if (wordsCount.ContainsKey(word))
                        {
                            wordsCount[word] += result[word];
                        }
                        else
                        {
                            wordsCount.Add(word, result[word]);
                        }
                    }
                }
            }
            else
            {
                foreach (var t in counting_tasks)
                    Console.WriteLine("Task {0}: {1}", t.Id, t.Status);
            }

            return wordsCount;
        }
    }
}
