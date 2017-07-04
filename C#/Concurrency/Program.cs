using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace Concurrency
{
    class Program
    {
        static void Main(string[] args)
        {
#region Raw Threads and Async
            TextDataController textDataController = new TextDataController();
            
            List<string> fileLines = new List<string>(5000);

            using (StreamReader streamReader = new StreamReader("data50MB.txt"))
            {
                string fileLine = streamReader.ReadLine();

                while (fileLine != null)
                {
                    fileLine = new string(fileLine.Where(c => !char.IsPunctuation(c)).ToArray());

                    if (fileLine.Trim() != string.Empty)
                    {
                        fileLines.Add(fileLine);
                    }

                    fileLine = streamReader.ReadLine();
                }
            }

            for (int i = 1; i < 11; i++)
            {
                long calcTime;
                Stopwatch watch = Stopwatch.StartNew();

                Dictionary<string, int> wordsCount = textDataController.GetWordsCountInParallel(fileLines, i, out calcTime);
                Console.WriteLine($"Number of Threads: {i} Calculation time: {calcTime} Total time: {watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L))}ms");
            }

            for (int i = 1; i < 11; i++)
            {
                Stopwatch watch = Stopwatch.StartNew();
                long calcTime;

                Dictionary<string, int> wordsCount = textDataController.GetWordsCountAsync("data10MB.txt", i, out calcTime);
                Console.WriteLine($"Async number of Threads: {i} Calculation time: {calcTime} Total time: {watch.ElapsedTicks / (Stopwatch.Frequency / (1000L * 1000L))}ms");

                watch.Reset();
            }
            #endregion

            #region Conveyor
            Conveyor conveyor = new Conveyor(10);

            Dictionary<string, int> wordsCountC = new Dictionary<string, int>(TextDataController.AVERAGE_WORDS_COUNT);
            conveyor.Run(ref wordsCountC);

            Console.WriteLine(wordsCountC.Count);

#endregion

            Console.ReadLine();
        }
    }
}
