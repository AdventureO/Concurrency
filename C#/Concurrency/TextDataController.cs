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

        public Dictionary<string, int> GetWordsCountInParallel(string fileName, int threadsQuantity)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGEWORDSCOUNT);

            Thread[] lels = new Thread[threadsQuantity];

            int fileDataChunckIndexStep = File.ReadLines(fileName).Count() / threadsQuantity;

            for (int i = 0; i < threadsQuantity; i++)
            {
                lels[i] = new Thread(new ThreadStart(delegate { this.ReadDataFromFileInParallel(fileName, ref wordsCount, fileDataChunckIndexStep * i); }));
                lels[i].Start();
            }

            for (int i = 0; i < threadsQuantity; i++)
            {
                lels[i].Join();
            }

            return wordsCount;
        }

        private object _lockObject = new object();
        private void ReadDataFromFileInParallel(string fileName, ref Dictionary<string, int> globalWordsCount, int startingLineIndex, Action callbackAction = null)
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

        private void ReadDataFromFile(string fileName, ref Dictionary<string, int> wordsCount, Action callbackAction = null)
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
    }
}
