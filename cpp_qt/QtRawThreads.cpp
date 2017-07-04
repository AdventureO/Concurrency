#include <QFile>
#include <iostream>
#include <QTextStream>
#include <QString>
#include <QMap>
#include <QVector>
#include <QSharedPointer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include <QCoreApplication>
#include <QDebug>
using namespace std;

// #define USE_STUPID_PARALLELIZATION

#include "q_clean_words.hpp"

//! Немає сенсу переписувати допоміжні інструменти на Qt
#include "../cxx/aux_tools.hpp"

using map_type = QMap<QString, unsigned int>;
using string_list_type = QStringList; //  QVector<QString>
//using string_list_type = QVector<QString>;


string_list_type qtReadData(const QString& filename) {
    string_list_type lst;
    QFile inputFile(filename);
    QString word;
    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");   // Ми враховуємо лише ASCII-символи, але
                                // це потрібно для коректного (ну, чи просто --
                                // такого ж, як в "голому" С++) розбиття на слова
        while (!in.atEnd()) {
            in >> word;
            lst.append(word);
        }
    }
    return lst;
}

QVector<string_list_type::iterator> splitWork(string_list_type& data_lst, size_t threads) {
    QVector<string_list_type::iterator> general;
    auto part_length = data_lst.size() / threads;
    auto pointer = data_lst.begin();
    general.append(pointer);
    for(size_t i = 0; i<threads;++i){
        advance(pointer, part_length);
        general.append(pointer);
        //std::cout << general.back() - data_lst.begin() << std::endl;
    }
    general.append(data_lst.end());
    //std::cout << general.back() - data_lst.begin() << std::endl;
    //data_lst.size() << std::endl;
    return general;
}



class CountingThread : public QThread {
    public:
        CountingThread(string_list_type::iterator num_start_i,
                       string_list_type::iterator num_fin_i,
                       map_type& wordsMap_i,
                       QMutex& mtx_i
                       );
        void run();

    private:
        string_list_type::iterator num_start;
        string_list_type::iterator num_fin;
        map_type& wordsMap;
        QMutex &mtx;

 };


CountingThread::CountingThread(string_list_type::iterator num_start_i,
                               string_list_type::iterator num_fin_i,
                               map_type& wordsMap_i,
                               QMutex& mtx_i):
   num_start (num_start_i), num_fin(num_fin_i), wordsMap(wordsMap_i), mtx(mtx_i)
{
}

void CountingThread::run() {
    map_type local_dictionary;
    for (auto a=num_start; a<num_fin; a++) {
        qtCleanWord(*a);
        ++local_dictionary[*a];
    }
#ifdef USE_STUPID_PARALLELIZATION
    for(auto itr=local_dictionary.cbegin(); itr!=local_dictionary.cend(); ++itr)
    {
        QMutexLocker locker(&mtx);
        wordsMap[itr.key()]+=itr.value();
    }
#else
    QMutexLocker locker(&mtx);
    for(auto itr=local_dictionary.cbegin(); itr!=local_dictionary.cend(); ++itr)
    {
        wordsMap[itr.key()]+=itr.value();
    }
#endif

}

int main(int argc, char *argv[], char**env)
{
    setlocale(LC_ALL,"C");

    QCoreApplication app(argc, argv);

    auto config = read_config("data_input_conc.txt");

    QString infile      = QString::fromStdString(config["infile"]);
    QString out_by_a    = QString::fromStdString(config["out_by_a"]);
    QString out_by_n    = QString::fromStdString(config["out_by_n"]);
    size_t threads_n    = str_to_val<size_t>(config["threads"]);

    QString etalon_a_file  = QString::fromStdString(config["etalon_a_file"]);

    //=============================================================
    auto reading_start_time = get_current_time_fenced();

    map_type wordsMap;
    QMutex mutex;

    string_list_type words_lst = qtReadData(infile);
    if (words_lst.isEmpty()) {
           cerr << "No data in the file or mistake in configuration" << endl;
        return -1;
    }
    Q_ASSERT_X( words_lst.size() >= threads_n, "main", "Too small file -- should contain more words than threads started.");

    auto work_parts = splitWork(words_lst, threads_n);

    auto indexing_start_time = get_current_time_fenced();

    QVector<QSharedPointer<CountingThread>> thread_ptrs_lst;
    for (auto a = work_parts.begin(); a < work_parts.end()-1; ++a) {
        thread_ptrs_lst.append(QSharedPointer<CountingThread>(
                                   new CountingThread(*a, *(a + 1), wordsMap, mutex)));
        thread_ptrs_lst.back()->start(); // Start as soon as possible.
    }

    for (auto thread: thread_ptrs_lst){
         thread->wait();
    }
    auto indexing_done_time = get_current_time_fenced();

    //=============================================================
    auto indexing_time = to_us(indexing_done_time - indexing_start_time);
    //auto reading_time = to_us(indexing_start_time - reading_start_time);
    auto total_time = to_us(indexing_done_time - reading_start_time);

    cout << "Total time    : " << total_time << endl;
    cout << "Analisys time : " << indexing_time << endl;

    //=============================================================
    //! Чисто з ліні -- щоб не переносити функції збереження під Qt
    std::map<std::string, unsigned int> cpp_map;
    for(auto iter = wordsMap.begin(); iter != wordsMap.end(); ++iter)
    {
        cpp_map[iter.key().toStdString()] = iter.value();
    }

    write_sorted_by_key(out_by_a.toStdString(), cpp_map);
    write_sorted_by_value(out_by_n.toStdString(), cpp_map);

    bool are_correct = true;
    if( !etalon_a_file.isEmpty() )
    {
        are_correct = compareFiles(out_by_a.toStdString(), etalon_a_file.toStdString());
    }
    return !are_correct;
}

