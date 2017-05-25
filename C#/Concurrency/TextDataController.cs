using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Concurrency
{
    class TextDataController
    {
        public const int AVERAGEWORDSCOUNT = 8000;

        public Dictionary<string, int> GetWordsCountInParallel(string fileName, int threadsQuantity)
        {
            Dictionary<string, int> wordsCount = new Dictionary<string, int>(AVERAGEWORDSCOUNT);

            this.ReadDataInParallel(fileName, ref wordsCount);

            return wordsCount;
        }

        public void ReadDataInParallel(string fileName, ref Dictionary<string, int> globalWordsCount, Action callbackAction = null)
        {
            Dictionary<string, int> localWordsCount = new Dictionary<string, int>(globalWordsCount.Count);
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

            callbackAction?.Invoke();
        }
    }
}
