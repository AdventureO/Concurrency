#include <QFile>
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
#include <QCoreApplication>
#include <QDebug>
using namespace std;

using words_counter_t = QMap<QString, int>;
words_counter_t words;
QMutex mutex;
QWaitCondition bufferNotEmpty;


QStringList reading(const QString& filename) {
    QStringList lst;
    QFile inputFile(filename);
    if (inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
          QString l = in.readAll().toLower();
          for (QChar el : l) {
              if (el.isPunct()) {l.remove(el);}
          }

          lst = l.simplified().split(' ', QString::SkipEmptyParts);

       inputFile.close();

    }
    return lst;
}

QList<int> lst_division(QStringList& data_lst, int threads) {
    QList<int> general;
    int pointer = 0;
    int division = std::ceil((float)data_lst.size()/threads);
    cout << division << "div " << endl;
        while (pointer+division < data_lst.size()) {

            general.append(pointer);
            general.append(pointer+division-1);

            pointer += division;
        }

        if (pointer < data_lst.size()) {
            general.append(pointer);
            general.append(data_lst.size()-1);
            pointer = data_lst.size();
        }

    return general;
}


class CountingThread : public QThread {

    public:
        CountingThread(const QStringList& data_lst, int num_start_i, int num_fin_i);
        void run();

    protected:
        const QStringList& data;
        const int num_start; // Нагадайте -- анекдот розкажу, про "справжній посередині, повертаю"
        const int num_fin;   // Крім того, start -- то такий метод QThread, а Ви його...



 };


CountingThread::CountingThread(const QStringList& data_lst,\
                               int num_start_i, int num_fin_i):
    data (data_lst), num_start (num_start_i), num_fin(num_fin_i)
{
}

// #define USE_STUPID_PARALLELIZATION
#ifdef USE_STUPID_PARALLELIZATION
void CountingThread::run() {
    for (int a=num_start; a<=num_fin; a++) {
        QMutexLocker locker(&mutex);
        ++words[data[a]];
    }
}
#else
void CountingThread::run() {
    words_counter_t local_dictionary;

    for (int a=num_start; a<=num_fin; a++) {
        ++local_dictionary[data[a]];
    }
    QMutexLocker locker(&mutex);
    for(auto itr=local_dictionary.cbegin(); itr!=local_dictionary.cend(); ++itr)
    {
        words[itr.key()]+=itr.value();
    }
}
#endif

int main(int argc, char *argv[], char**env)
{
    // ----------------------------------------------
   // reading from script of command prommt
  // command prommt input example : 3 /home/yaryna/Desktop/ sec.txt RESULT_FOR_TEST.txt 9 "LAST"

    QCoreApplication app(argc, argv, **env);
    int num_threads;
    sscanf(argv[1], "%d", &num_threads);
    QString base_path(argv[2]);
    QString inpfile(argv[3]);
    QString outpfile(argv[4]);

   QString out_filename{base_path + outpfile};
   QString in_filename {base_path + inpfile};
   // ----------------------------------------------

   QStringList words_lst = reading(in_filename);
   if (words_lst.isEmpty()) {
       cerr << "No data in the file or mistake in configuration"<< endl;
       return -1;
   }
   if (words_lst.size()<num_threads) num_threads=words_lst.size()-1; //!!!
   QList<int> num_lst = lst_division(words_lst, num_threads);

   cout << "PROGRAM DESCRIPTION" << endl;
   cout << "TOTAL QUANTITY OF WORDS: " << words_lst.size() << endl;
   cout << "Threads: " << num_threads << endl;

   auto creating_threads_start_time = get_current_time_fenced();

   QList<CountingThread*> thread_lst;
   int num_pointer = 0;
   for (int el=0; el<num_threads; el++) {
               thread_lst.append(new CountingThread(\
                                     words_lst, num_lst[num_pointer], num_lst[num_pointer+1]));
               num_pointer += 2;
   }


   //---------------------------------------------------------------
   // Це все ще не дуже хороший спосіб виміру! Потім мусимо переключитися
   // на Performance counters, але це буде потім, а так -- краще, ніж QTimer.
   // Performance counters -- див. PAPI тут:
   // Архітектура комп'ютерних систем (CS.02.17) --> Практична 2. Розпаралелення задач із явним використанням потоків ОС --> Вимірювання часу
   // (Пряме посилання не даю, з міркувань безпеки).
   auto indexing_start_time = get_current_time_fenced();

   for (auto thread: thread_lst) {
       if(num_threads>1)
       {
           thread->start(); // thread->run(); STARTS CODE IN THIS THREAD! Use start to run code in other thread.
       }else{
           thread->run(); // Do not use threads at all.
       }



   }

   for (auto thread: thread_lst){

        thread->wait();
   }

   auto indexing_done_time = get_current_time_fenced();

   //---------------------------------------------------------------
   for(auto x: thread_lst)
       delete x;

   int total_words = 0;
   qDebug() << words;
   for(auto it = words.begin(); it != words.end(); ++it) {
       total_words += it.value();
   }

   auto time_res = to_us(indexing_done_time - indexing_start_time);
   auto creating_threads_time = to_us(indexing_start_time - creating_threads_start_time);
   cout << "INDEXING TIME: " << time_res << " us " << endl;
   cout << "THREADS CREATING TIME: " << creating_threads_time << " us " << endl;

   if( words_lst.size() != total_words )
   {
       cout << words_lst.size() << " s " << total_words << " t " << endl;
       cerr << "Something wrong -- words count before and after indexing, differs!" << endl;
   }
   //---------------------------------------------------------------
   QFile output_file(out_filename);
   if (!output_file.open(QIODevice::WriteOnly)) {
       cerr << "Could not write file with results." << endl;
       return -1;
   }

   QTextStream output_stream(&output_file);
   if( words_lst.size() != total_words )
   {
       output_stream << "Something wrong -- words count before and after indexing, differs!" << endl;
   }
   output_stream << "Total words: " << total_words << endl;
   output_stream << "Total time: " << time_res << endl;
   for (auto it = words.begin(); it != words.end(); ++it) {
       // Format output here.
       output_stream << QString("%1 : %2 \n").arg(it.key(), 10).arg(it.value(), 10);
   }
   output_file.close();

}

