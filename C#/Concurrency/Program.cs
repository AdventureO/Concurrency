using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.IO;

namespace Concurrency
{
    class Program
    {
        static void Main(string[] args)
        {
            TextDataController textDataController = new TextDataController();

            Dictionary<string, int> wordsCount = textDataController.GetWordsCountInParallel("Words Processing Data.txt", 4);

            Console.WriteLine("Threads word count: " + wordsCount.Keys.Count);

            Console.ReadLine();

            wordsCount = textDataController.GetWordsCountAsync("Words Processing Data.txt", 4);

            Console.WriteLine("Async word count: " + wordsCount.Keys.Count);

            Console.ReadLine();
        }
    }
}
