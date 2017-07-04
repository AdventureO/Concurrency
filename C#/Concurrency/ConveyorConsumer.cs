using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Concurrency
{
    class ConveyorConsumer
    {
        public void Consume(ref ConveyorQueue<Dictionary<string, int>> container, ref List<string> fileLines)
        {
            Dictionary<string, int> localWordsCount = new Dictionary<string, int>(TextDataController.AVERAGE_WORDS_COUNT);

            TextDataController.CountWordsFromArray(fileLines, ref localWordsCount);

            container.Enqueue(localWordsCount);
        }

        public ConveyorConsumer()
        {
            
        }
    }
}
