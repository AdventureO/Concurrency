#include <iostream>
#include <QTextStream>
#include <QString>
#include <QMap>
#include <cmath>
#include <iterator>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include "timing_v1.hpp"
#include <QtConcurrent/QtConcurrentRun>
#include <QFile>
#include <QCoreApplication>


//COMMAND LINE ARGUMENTS:  5 /home/yaryna/Desktop/ sec.txt RESULT_FOR_TEST.txt 9 "LAST"


using namespace std;

using words_counter_t = QMap<QString, int>;
words_counter_t words;
QMutex mutex;
QWaitCondition bufferNotEmpty;


QStringList reading(const QString& filename, int mode_trig) { //mode trigger == 1 - we remove all punctuation
    QStringList lst;
    QFile inputFile(filename);
    QString l;
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
          if (mode_trig == 1) {

              l = in.readAll().toLower();

          for (QChar el : l) {
              if (el.isPunct()) {l.remove(el);}
          }
}
          else {
               l = in.readAll();
              for (QChar el : l) {
                  if ((el == (QChar)'=')|| (el == (QChar)'\xa')) {
                      l.replace('=', ' ');
                      l.replace('\xa',  ' ');
                      l.replace('"',  ' ');
}
          }
          }
          lst = l.simplified().split(' ', QString::SkipEmptyParts);

       inputFile.close();

    }
    return lst;
}

QList<int> lst_division(QStringList& data_lst, int threads) {
    QList<int> general;
    int pointer = 0;
  //  if da
    int division = std::ceil((float)data_lst.size()/threads);
        while (pointer+division < data_lst.size()) {

            general.append(pointer);
            general.append(pointer+division);

            pointer += division + 1;
        }

        if (pointer < data_lst.size()) {
            general.append(pointer);
            general.append(data_lst.size()-1);
            pointer = data_lst.size();
        }
    return general;
}

words_counter_t mapper(int num_start, int num_fin, QStringList& data_lst) {
    words_counter_t words_local;
    for (int a=num_start; a<=num_fin; a++) {
            ++words_local[data_lst[a]]; //maye buty lokalnyy slovnyk
        }
    return words_local;
}

void reducer(words_counter_t &words, const words_counter_t& local_dictionary) {
    for(auto itr=local_dictionary.cbegin(); itr!=local_dictionary.cend(); ++itr)
        {
            words[itr.key()]+=itr.value();
        }
}

int main(int argc, char *argv[])
{
   // ----------------------------------------------
   // reading from command promt

    QCoreApplication app(argc, argv);

    QString in_filename, out_filename;
    int num_threads;

    QString myargfile("/home/yaryna/data_input.txt");
    QFile myFile();

    QStringList lst_arg = reading(myargfile, 0);
      //  sscanf(argv[1], "%d", &num_threads);

    //    QString base_path(argv[2]);

        in_filename = lst_arg[1];
        out_filename = lst_arg[3];
        num_threads = lst_arg[5].toInt();


    // ----------------------------------------------

    auto creating_threads_start_time = get_current_time_fenced();

    QStringList words_lst = reading(in_filename, 1); //skip punctuation

    QMap<QString, int> result_futures;
    if (words_lst.isEmpty()) {
       cerr << "No data in the file or mistake in configuration"<< endl;
       return -1;
    }
    if (words_lst.size()<num_threads) num_threads=words_lst.size()-1; //!!!
    QList<int> num_lst = lst_division(words_lst, num_threads);


    QList<QFuture<words_counter_t>> future_list;

    int num_pointer = 0;
    auto indexing_start_time = get_current_time_fenced();

    for (int el=0; el<num_threads; el++) {
       // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
       if (num_pointer+1 < num_lst.size()) {
       future_list << QtConcurrent::run(mapper, num_lst[num_pointer], num_lst[num_pointer+1], words_lst);
               num_pointer += 2;
       }
    }



    for (size_t el=0; el<future_list.size(); el++) {
        reducer(words, future_list[el]);
        //words << future_list[el].result();
    }



   //---------------------------------------------------------------
   // Це все ще не дуже хороший спосіб виміру! Потім мусимо переключитися
   // на Performance counters, але це буде потім, а так -- краще, ніж QTimer.
   // Performance counters -- див. PAPI тут:
   // Архітектура комп'ютерних систем (CS.02.17) --> Практична 2. Розпаралелення задач із явним використанням потоків ОС --> Вимірювання часу
   // (Пряме посилання не даю, з міркувань безпеки).

   auto indexing_done_time = get_current_time_fenced();

   int total_words = 0;
   for(auto it = words.begin(); it != words.end(); ++it) {
       total_words += it.value();
   }

   auto threading_time = to_us(indexing_done_time - indexing_start_time);
   auto reading_time = to_us(indexing_start_time - creating_threads_start_time);
   auto total_time = to_us(indexing_done_time - creating_threads_start_time);

   cout << "INDEXING TIME: " << reading_time << " us " << endl;
   cout << "THREADS CREATING TIME: " << threading_time << " us " << endl;
   if( words_lst.size() != total_words )
   {
       cout << "words" << words_lst.size() << " " << total_words << endl;
       cerr << "Something wrong -- words count before and after indexing, differs!" << endl;
   }
//   //---------------------------------------------------------------
   QFile output_file(out_filename);
   if (!output_file.open(QIODevice::WriteOnly|QIODevice::Append)) {
       cerr << "Could not write file with results." << endl;
       return -1;
   }

   QTextStream output_stream(&output_file);
  if( words_lst.size() != total_words )
   {
       output_stream << "Something wrong -- words count before and after indexing, differs!" << endl;
   }


   output_stream << "Reading time: " << reading_time << endl;
   output_stream << "Threading time: " << threading_time << endl;
   output_stream << "Total time: " << total_time << endl;

   output_file.close();
}


