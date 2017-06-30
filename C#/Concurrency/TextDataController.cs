using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Threading;
using System.IO;
using System.Diagnostics;

namespace Concurrency
{
    public class TextDataController
    {
        public const int AVERAGE_WORDS_COUNT = 9000;

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

        public Dictionary<string, int> GetWordsCountInParallel(string fileName, int threadsQuantity, out long calculationTime)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGE_WORDS_COUNT);

            Thread[] counting_threads = new Thread[threadsQuantity];

            int fileDataChunkIndexStep = File.ReadLines(fileName).Count() / threadsQuantity;

            Stopwatch watch = Stopwatch.StartNew();

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_threads[i] = new Thread(new ThreadStart(delegate { this.CountWordsFromFileInParallel(fileName, ref wordsCount, fileDataChunkIndexStep * i); }));
                counting_threads[i].Start();
            }

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_threads[i].Join();
            }

            calculationTime = watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L));
            watch.Stop();

            return wordsCount;
        }

        private void CountWordsFromArrayInParallel(ref List<string> fileLines, ref Dictionary<string, int> globalWordsCount, int startingLineIndex, int endingLineIndex, Action callbackAction = null)
        {
            Dictionary<string, int> localWordsCount = new Dictionary<string, int>(globalWordsCount.Count);
            int startingWordCount = 1;


            string[] fileLineWordsBuffer;

            for (int j = startingLineIndex; j < fileLines.Count && j < endingLineIndex; j++)
            {
                fileLineWordsBuffer = fileLines[j].Split(' ');
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

        public Dictionary<string, int> GetWordsCountInParallel(List<string> lines, int threadsQuantity, out long calculationTime)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGE_WORDS_COUNT);

            Thread[] counting_threads = new Thread[threadsQuantity];

            int fileDataChunkIndexStep = lines.Count / threadsQuantity;

            Stopwatch watch = Stopwatch.StartNew();

            for (int i = 0; i < threadsQuantity; i++)
            {
                int startingIndex = fileDataChunkIndexStep * i;
                int endingIndex = fileDataChunkIndexStep * (i + 1);
                counting_threads[i] = new Thread(new ThreadStart(delegate { this.CountWordsFromArrayInParallel(ref lines, ref wordsCount, startingIndex, endingIndex); }));
                counting_threads[i].Start();
            }

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_threads[i].Join();
            }

            calculationTime = watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L));
            watch.Stop();

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
                Dictionary<string, int> localWordsCount = new Dictionary<string, int>(AVERAGE_WORDS_COUNT);
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

        public Dictionary<string, int> GetWordsCountAsync(string fileName, int threadsQuantity, out long calculationTime, Action callbackAction = null)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGE_WORDS_COUNT);

            Task<Dictionary<string, int>>[] counting_tasks = new Task<Dictionary<string, int>>[threadsQuantity];

            int fileDataChunkIndexStep = File.ReadLines(fileName).Count() / threadsQuantity;

            Stopwatch watch = Stopwatch.StartNew();

            for (int i = 0; i < threadsQuantity; i++)
            {
                counting_tasks[i] = this.CountWordsFromFileAsync(fileName, fileDataChunkIndexStep * i, callbackAction);
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
                calculationTime = watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L));
                watch.Stop();
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
                calculationTime = 0;
            }

            return wordsCount;
        }

        private Task<Dictionary<string, int>> CountWordsFromArrayAsync(List<string> fileLines, int startingLineIndex, int endingLineIndex, Action callbackAction = null)
        {
            return Task.Factory.StartNew(() =>
            {
                Dictionary<string, int> localWordsCount = new Dictionary<string, int>(AVERAGE_WORDS_COUNT);
                int startingWordCount = 1;

                string[] fileLineWordsBuffer;

                for (int j = startingLineIndex; j < fileLines.Count && j < endingLineIndex; j++)
                {
                    fileLineWordsBuffer = fileLines[j].Split(' ');
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

                callbackAction?.Invoke();

                return localWordsCount;
            });
        }

        public Dictionary<string, int> GetWordsCountAsync(List<string> lines, int threadsQuantity, out long calculationTime)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGE_WORDS_COUNT);

            Task<Dictionary<string, int>>[] counting_tasks = new Task<Dictionary<string, int>>[threadsQuantity];

            int fileDataChunkIndexStep = lines.Count / threadsQuantity;

            Stopwatch watch = Stopwatch.StartNew();

            for (int i = 0; i < threadsQuantity; i++)
            {
                int startingIndex = fileDataChunkIndexStep * i;
                int endingIndex = fileDataChunkIndexStep * (i + 1);
                counting_tasks[i] = this.CountWordsFromArrayAsync(lines, startingIndex, endingIndex);
            }

            Task<Dictionary<string, int>[]> results = Task.WhenAll(counting_tasks);

            if (results.Status == TaskStatus.RanToCompletion)
            {
                calculationTime = watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L));
                watch.Stop();
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
                calculationTime = 0;
            }

            return wordsCount;
        }

        public static void CountWordsFromArray(List<string> fileLines, ref Dictionary<string, int> wordsCount, Action callbackAction = null)
        {
            int startingWordCount = 1;

            string[] fileLineWordsBuffer;

            for (int j = 0; j < fileLines.Count; j++)
            {
                fileLineWordsBuffer = fileLines[j].Split(' ');
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

            callbackAction?.Invoke();
        }
    }
}
