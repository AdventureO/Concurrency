using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace Concurrency
{
    public static class StreamReaderUtility
    {
        public static void SkipToLine(StreamReader streamReader, int lineIndex)
        {
            for (int i = 0; i < lineIndex; i++)
            {
                streamReader.ReadLine();
            }
        }
    }
}
