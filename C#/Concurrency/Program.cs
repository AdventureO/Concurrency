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

            Dictionary<string, int> wordsCount = new Dictionary<string, int>(TextDataController.AVERAGEWORDSCOUNT);

            textDataController.ReadDataInParallel("Words Processing Data.txt", ref wordsCount);

            foreach (var item in wordsCount)
            {
                Console.WriteLine(item);
            }

            Console.WriteLine(wordsCount.Keys.Count);

            Console.ReadLine();
        }
    }
}
