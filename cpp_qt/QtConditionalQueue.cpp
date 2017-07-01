// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include <atomic>
#include <cstring>
#include <QQueue>
#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <QFile>
#include <cmath>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QSharedPointer>
#include <QAtomicInteger>

//! Немає сенсу переписувати допоміжні інструменти на Qt
#include "../cxx/aux_tools.hpp"

using namespace std;

QQueue<QList<QString> > d;
QWaitCondition cv;
QWaitCondition cv1;
QMutex mux;
QMutex mux1;
QMutex mux2;
QMutex mux3;
QAtomicInteger<bool> done = { false };
QAtomicInteger<size_t> done_count;



using words_counter_t = QMap<QString, int>;
words_counter_t wordsMap;
QQueue<words_counter_t> map_q;




class ReadingThread : public QThread { //appends to queue

public:
    ReadingThread(const QString& filename);
    void run();

protected:
    const QString& filename;
};

ReadingThread::ReadingThread(const QString& filename)
    : filename(filename)
{
}


void ReadingThread::run()
{
    QStringList lst;
    int counter = 1;
    int fullblock = 100;
    QFile inputFile(filename);

    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        while (!in.atEnd()) {

            QString l = in.readLine();
            lst << l;
            if (counter == fullblock) {
                QMutexLocker lck(&mux);
                d.enqueue(lst);
                lck.unlock(); ///////////////////////
                cv.wakeOne();
                lst.clear();
                counter = 0; //
            }
            else {
                counter++;
            }
        }
        if (lst.size() != 0) {

            QMutexLocker lck(&mux);
            d << lst;
            lck.unlock();
            cv.wakeOne();
        }
        inputFile.close();
        cv.wakeAll();
        done = true;
    }
    else {
        cout << "error reading from file";
    }
}




class CountingThread : public QThread { //takes from queue and adds to map

public:
    CountingThread(QAtomicInteger<size_t> &done_count);
    void run();

protected:
    QAtomicInteger<size_t> &done_count;
};

CountingThread::CountingThread(QAtomicInteger<size_t> &done_count)
    : done_count(done_count) {}

void CountingThread::run()
{
// можливо проблема через цей мютекс, але коли я його переставляю
    // в іф, програма взагалі не закінчує ранитися
    words_counter_t local_dictionary;

    while (true) {
        qDebug() << done_count << " : COUNT";
            QString l;
            QMutexLocker lk(&mux);
            if (!d.empty()) {
                QList<QString> v;
                v << d.head();
                d.dequeue();
                lk.unlock();
            for (int i = 0; i < v.size(); i++) {
                QTextStream in(&v[i]);
                while (!in.atEnd()) {
                in >> l;
                l =  l.remove(remove_if(l.begin(), l.end(), [](QChar x) {return !x.isLetter() && !x.isSpace();} )
                - l.begin(), l.size() ).toLower();
                QMutexLocker dict_lk(&mux1); //окремий мютекслокер дати
                ++local_dictionary[l];
            }
            }
            QMutexLocker other_lk(&mux2);
            map_q.enqueue(local_dictionary);
            other_lk.unlock();
            cv1.wakeOne();
        }
        if (d.empty()) {
            if (done) {

                //cv1.wakeOne(); //////////////////////////////////
                break;
            }
            else {
                cv.wait(&mux);
            }
        }
    }
    cv1.wakeAll();
    qDebug() << "here";
    done_count--;
}


class MergingThread : public QThread { //appends to queue

public:
    MergingThread(QAtomicInteger<size_t> &done_count);
    void run();

protected:
    QAtomicInteger<size_t> &done_count;

};

MergingThread::MergingThread(QAtomicInteger<size_t> &done_count)
    : done_count(done_count) {}



void MergingThread::run()
{
    while (true) {
        qDebug() << "COUNT BEFORE: " << done_count;
    QMutexLocker luk1(&mux2);
    if (map_q.size() != 0) {
        cout << "MERGE " << endl;
        words_counter_t local_dictionary = map_q.head();
        luk1.unlock();
    for (auto itr = local_dictionary.cbegin(); itr != local_dictionary.cend(); ++itr) {
        QMutexLocker lg(&mux3);
        wordsMap[itr.key()] += itr.value();
    }
    map_q.dequeue();
    }else{
        if ((done_count==0)&&(map_q.isEmpty())) {
            qDebug() << "DONE COUNT TRUE";
            break;
        }else{
            qDebug() << "DONE COUNT FALSE";
            cv1.wait(&mux2);
        }
    }
    }
}



int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    auto config = read_config("data_input_conc.txt");

    QString infile      = QString::fromStdString(config["infile"]);
    QString out_by_a    = QString::fromStdString(config["out_by_a"]);
    QString out_by_n    = QString::fromStdString(config["out_by_n"]);
    size_t threads_n   = str_to_val<size_t>(config["threads"]);

    QString etalon_a_file  = QString::fromStdString(config["etalon_a_file"]);

    //=============================================================
    auto creating_threads_start_time = get_current_time_fenced();

    done_count = {threads_n};

    //! Перевизначення run() тут є прийнятним -- воно найближче до c++11::thread
    //! але див. https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
    //! та й тут: http://doc.qt.io/qt-5/qthread.html
    //! Важливо, що про мотиви використовувати "неканонічне" рішення
    //! слід буде наголосити в тексті.
    ReadingThread reader_thr(infile);

    //! Без динамічної пам'яті тут обійтися важко
    //! Але спростимо собі керування нею.
    //! На жаль, через "креативність" розробників Qt (https://stackoverflow.com/questions/34761327/qlist-of-qscopedpointers)
    //! оптимальнішим QScopedPointer скористатися не вдалося. Та й makeXXXPointer вони не надали...
    QVector<QSharedPointer<CountingThread>> thread_ptrs_lst;
    for (int el = 0; el < threads_n; el++) {
        thread_ptrs_lst.append(QSharedPointer<CountingThread>(new CountingThread(done_count)));
    }
    MergingThread merging_thr(done_count);

    //! Замір часу створення потоків важливий, однак тут він може зіпсувати
    //! трохи результати -- вставивши бар'єри пам'яті та заважаючи компілятору
    //! оптимізувати.
    // auto indexing_start_time = get_current_time_fenced();
    reader_thr.start();
    // QThread::run() runs code in this thread,
    // QThread::start() -- in target thread
    for (auto& thread : thread_ptrs_lst) {
        if (threads_n > 1) {
            thread->start();
        }
        else {
            thread->run(); // Do not use threads at all.
        }
    }
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

    write_sorted_by_value(out_by_a.toStdString(), cpp_map);
    write_sorted_by_value(out_by_n.toStdString(), cpp_map);

    bool are_correct = true;
    if( !etalon_a_file.isEmpty() )
    {
        are_correct = compareFiles(out_by_a.toStdString(), etalon_a_file.toStdString());
    }
    return !are_correct;
}
