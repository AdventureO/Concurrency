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
#include "../cxx/measurements.hpp"

using namespace std;

using map_type = QMap<QString, unsigned int>;
//using string_list_type = QStringList; //  QVector<QString>
using string_list_type = QVector<QString>;

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
    //! So-called universal reference -- template + &&, moves if possible.
    //! Compiles only for rvalue references, for example temporatire and move-ed variables
    void enque(T&& lines )
    {
        QMutexLocker ll(&mtx);
        que.enqueue(move(lines));
        cv.wakeOne();
    }

    void one_thread_done()
    {
        --left_threads;
        QMutexLocker ll(&mtx);
        cv.wakeAll();
    }

    //! Returns true and saves data to argument, if have one
    //! returns false otherwise.
    bool dequeue(T& lines )
    {
        QMutexLocker luk(&mtx);
        while( que.empty() && left_threads != 0 )
            cv.wait(&mtx);
        if(left_threads == 0 && que.empty() )
            return false;
        lines=move(que.head());
        que.dequeue();
        return true;
    }

    size_t size() const
    {
        return que.size();
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
    QFile inputFile(filename);

    if (inputFile.open(QIODevice::ReadOnly)) {
        string_list_type words;
        QTextStream in(&inputFile);
        in.setCodec("UTF-8");   // Ми враховуємо лише ASCII-символи, але
                                // це потрібно для коректного (ну, чи просто --
                                // такого ж, як в "голому" С++) розбиття на слова        
        while (!in.atEnd()) {
            QString word;
            in >> word;
            words.append(move(word));
            if (words.size() == blockSize) {
                blocksQue.enque(move(words));
                words.clear();
            }
        }
        if (words.size() != 0) {
            blocksQue.enque(move(words));
        }
        blocksQue.one_thread_done();
        //inputFile.close(); // Closed in destructor
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
    string_list_type v;
    while(blocksQue.dequeue(v))
    {
        map_type local_dictionary;
        for(auto& word: v){
                qtCleanWord(word);
                ++local_dictionary[word];
        }
        mapsQue.enque(move(local_dictionary));
        // local_dictionary is recreated every time -- no need for clear
    }

    mapsQue.one_thread_done();
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
    map_type local_dictionary;
    while(mapsQue.dequeue(local_dictionary))
    {
        for (auto itr = local_dictionary.cbegin(); itr != local_dictionary.cend(); ++itr) {
            wordsMap[itr.key()] += itr.value();
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

    int measurement_flags = str_to_val<int>(config["measurement_flags"]);

    cpu_measurements_provider_t cpu_measurements;
    all_measurements_t measurements(cpu_measurements, measurement_flags);
    //=============================================================
    int started_mark_idx = measurements.mark_start("Started reading");
    measurements.measure();

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

    int started_cnt_idx = measurements.mark_start("Started counting");
    //! Без динамічної пам'яті тут обійтися важко
    //! Але спростимо собі керування нею.
    //! На жаль, через "креативність" розробників Qt (https://stackoverflow.com/questions/34761327/qlist-of-qscopedpointers)
    //! оптимальнішим QScopedPointer скористатися не вдалося. Та й makeXXXPointer вони не надали...
    QVector<QSharedPointer<CountingThread>> thread_ptrs_lst;
    for (size_t el = 0; el < threads_n-2; el++) {
        thread_ptrs_lst.append(QSharedPointer<CountingThread>(new CountingThread(readBlocksQ, localDictsQ)));
        thread_ptrs_lst.back()->start(); // Start as soon as possible.
    }

    int started_mrg_idx = measurements.mark_start("Started mergin");
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
    measurements.measure();
    measurements.mark_finish(started_mark_idx);
    measurements.mark_finish(started_cnt_idx);
    measurements.mark_finish(started_mrg_idx);

    if(measurements.mts & all_measurements_t::BASE_MSM)
    {
        cout << "Total:" << endl;
        (measurements.wt[1] - measurements.wt[0]).print(cout);
    }
    if(measurements.mts & all_measurements_t::SYS_MSM)
    {
        cout << "=========System=specific=data=====================" << endl;
        cout << "Total:" << endl;
        (measurements.st[1] - measurements.st[0]).print(cout, "Process");
    }
    if(measurements.mts & all_measurements_t::CPU_MSM)
    {
        cout << "=========CPU=data===========================" << endl;
        cout << "Total:" << endl;
        print_cpu_params(measurements.ct[0], measurements.ct[1]);
        cout << "=========Auxiliary=data===========================" << endl;
    }
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
