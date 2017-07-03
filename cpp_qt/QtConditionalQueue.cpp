// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>

#include <QtDebug>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QFile>
#include <QMap>
#include <QThread>
#include <QCoreApplication>
#include <QSharedPointer>
#include <QAtomicInteger>
#include <QException>

#include "q_clean_words.hpp"

//! Немає сенсу переписувати допоміжні інструменти на Qt
#include "../cxx/aux_tools.hpp"

using namespace std;

using map_type = QMap<QString, unsigned int>;
using string_list_type = QStringList; //  QVector<QString>

template<typename T, typename Container=QQueue<T>>
class QSimpleQueStruct
{
public:
    Container que;
    QWaitCondition cv;
    QMutex mtx;
    QAtomicInteger<size_t> left_threads;

    QSimpleQueStruct(size_t thr): left_threads(thr){}

    ~QSimpleQueStruct(){
        // Just sanity check.
        Q_ASSERT_X( left_threads == 0, "~QSimpleQueStruct()",
                      "Que destroyed before all users stopped.");
    }
};

class CCException: public QException
{
    QString m_message;
public:
    QString message() const { return m_message; }
    CCException(const QString& s): m_message(s){}
};


class ReadingThread : public QThread { //appends to queue

public:
    ReadingThread(const QString& filename_, size_t blockSize_, QSimpleQueStruct<string_list_type>& blocksQue_);
    void run();

protected:
    QString filename;
    size_t blockSize;
    QSimpleQueStruct<string_list_type>& blocksQue;
};

ReadingThread::ReadingThread(const QString& filename_, size_t blockSize_, QSimpleQueStruct<string_list_type>& blocksQue_)
    : filename(filename_), blockSize(blockSize_), blocksQue(blocksQue_)
{
}

void ReadingThread::run()
{
    string_list_type words;
    QFile inputFile(filename);

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");   // Ми враховуємо лише ASCII-символи, але
                                // це потрібно для коректного (ну, чи просто --
                                // такого ж, як в "голому" С++) розбиття на слова
        QString word;
        while (!in.atEnd()) {
            in >> word;
            words.append(word);
            if (words.size() == blockSize) {
                QMutexLocker lck(&blocksQue.mtx);
                blocksQue.que.enqueue(words);
                blocksQue.cv.wakeOne();
                lck.unlock();
                words.clear();
            }
        }
        if (words.size() != 0) {

            QMutexLocker lck(&blocksQue.mtx);
            blocksQue.que.append(words);
            blocksQue.cv.wakeOne();
            lck.unlock();
        }
        inputFile.close();
        QMutexLocker lck(&blocksQue.mtx);
        blocksQue.cv.wakeAll();
        --blocksQue.left_threads;
    }
    else {
        qDebug() << "Error reading file: " + filename;
        throw CCException("Error reading file: " + filename );
    }
}


class CountingThread : public QThread { //takes from queue and adds to map

public:
    CountingThread(QSimpleQueStruct<string_list_type>& blocksQue_,
                   QSimpleQueStruct<map_type>& mapsQue_);
    void run();

protected:
    QSimpleQueStruct<string_list_type>& blocksQue;
    QSimpleQueStruct<map_type>& mapsQue;
};

CountingThread::CountingThread(QSimpleQueStruct<string_list_type>& blocksQue_,
                               QSimpleQueStruct<map_type>& mapsQue_)
    : blocksQue(blocksQue_), mapsQue(mapsQue_){}

void CountingThread::run()
{

    while (true) {
        QMutexLocker lk(&blocksQue.mtx);
        if (!blocksQue.que.empty()) {
            string_list_type v;
            v.append(blocksQue.que.head());
            blocksQue.que.dequeue();
            lk.unlock();
            map_type local_dictionary;
            for(auto& word: v){
                    qtCleanWord(word);
                    ++local_dictionary[word];
            }
            QMutexLocker other_lk(&mapsQue.mtx);
            mapsQue.que.enqueue(local_dictionary);
            mapsQue.cv.wakeOne();
        }
        if (blocksQue.que.empty()) {
            if (blocksQue.left_threads == 0) {
                break;
            }
            else {
                blocksQue.cv.wait(&blocksQue.mtx);
            }
        }
    }
    --mapsQue.left_threads;
    mapsQue.cv.wakeAll();
}


class MergingThread : public QThread { //appends to queue

public:
    MergingThread(QSimpleQueStruct<map_type>& mapsQue_, map_type& wordsMap_);
    void run();

protected:
    QSimpleQueStruct<map_type>& mapsQue;
    map_type& wordsMap;
};

MergingThread::MergingThread(QSimpleQueStruct<map_type>& mapsQue_, map_type& wordsMap_)
    : mapsQue(mapsQue_), wordsMap(wordsMap_) {}



void MergingThread::run()
{
    while (true) {
        QMutexLocker luk1(&mapsQue.mtx);
        if (mapsQue.que.size() != 0) {
            map_type local_dictionary{mapsQue.que.head()};
            mapsQue.que.dequeue();
            luk1.unlock();
            for (auto itr = local_dictionary.cbegin(); itr != local_dictionary.cend(); ++itr) {
                wordsMap[itr.key()] += itr.value();
            }
        }else{
            if ( mapsQue.left_threads == 0) {
                break;
            }else{
                mapsQue.cv.wait(&mapsQue.mtx);
            }
        }
    }
}


int main(int argc, char* argv[])
{
    setlocale(LC_ALL,"C");

    QCoreApplication app(argc, argv);

    auto config = read_config("data_input_conc.txt");

    QString infile      = QString::fromStdString(config["infile"]);
    QString out_by_a    = QString::fromStdString(config["out_by_a"]);
    QString out_by_n    = QString::fromStdString(config["out_by_n"]);
    size_t blockSize    = str_to_val<size_t>(config["blockSize"]);
    size_t threads_n    = str_to_val<size_t>(config["threads"]);

    QString etalon_a_file  = QString::fromStdString(config["etalon_a_file"]);

    //=============================================================
    auto creating_threads_start_time = get_current_time_fenced();

    map_type wordsMap;
    QSimpleQueStruct<string_list_type> readBlocksQ{1};
    QSimpleQueStruct<map_type> localDictsQ{threads_n-2};

    //! Перевизначення run() тут є прийнятним -- воно найближче до c++11::thread
    //! але див. https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
    //! та й тут: http://doc.qt.io/qt-5/qthread.html
    //! Важливо, що про мотиви використовувати "неканонічне" рішення
    //! слід буде наголосити в тексті.
    ReadingThread reader_thr(infile, blockSize, readBlocksQ);
    reader_thr.start(); // Start as soon as possible.

    //! Без динамічної пам'яті тут обійтися важко
    //! Але спростимо собі керування нею.
    //! На жаль, через "креативність" розробників Qt (https://stackoverflow.com/questions/34761327/qlist-of-qscopedpointers)
    //! оптимальнішим QScopedPointer скористатися не вдалося. Та й makeXXXPointer вони не надали...
    QVector<QSharedPointer<CountingThread>> thread_ptrs_lst;
    for (size_t el = 0; el < threads_n-2; el++) {
        thread_ptrs_lst.append(QSharedPointer<CountingThread>(new CountingThread(readBlocksQ, localDictsQ)));
        thread_ptrs_lst.back()->start(); // Start as soon as possible.
    }

    MergingThread merging_thr(localDictsQ, wordsMap);

    // QThread::run() runs code in this thread,
    // QThread::start() -- in target thread
    merging_thr.run(); // Можемо собі дозволити працювати в цьому потоці -- новий не потрібен.

    reader_thr.wait();
    for (auto& thread : thread_ptrs_lst) {
        thread->wait();
    }

    merging_thr.wait();

    //=============================================================

    auto indexing_done_time = get_current_time_fenced();

    auto time_res = to_us(indexing_done_time - creating_threads_start_time);
    cout << "Total time : " << time_res << endl;

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
