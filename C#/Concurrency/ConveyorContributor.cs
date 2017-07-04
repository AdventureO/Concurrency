using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Concurrency
{
    class ConveyorContributor
    {
        public void Contribute(string filePath, int step)
        {
            using (StreamReader streamReader = new StreamReader(filePath))
            {
                string fileLine = streamReader.ReadLine();

                List<string> fileLines = new List<string>(step);

                while (fileLine != null)
                {
                    fileLine = new string(fileLine.Where(c => !char.IsPunctuation(c)).ToArray());

                    if (fileLine.Trim() != string.Empty)
                    {
                        fileLines.Add(fileLine);
                        if (fileLines.Count >= step)
                        {
                            Conveyor.DataChunks.Enqueue(fileLines);
                            Conveyor.DataChunksState.Pulse();

                            Console.WriteLine("Data contributed and notification sent");

                            fileLines = new List<string>(step);
                        }
                    }

                    fileLine = streamReader.ReadLine();
                }

                if (fileLines.Count > 0)
                {
                    Conveyor.DataChunks.Enqueue(fileLines);
                    Conveyor.DataChunksState.Pulse();
                }

                Conveyor.AllDataRead = true;
            }
        }

        public ConveyorContributor()
        {

        }
    }
}
