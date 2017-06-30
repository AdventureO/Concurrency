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
#include "timing_v1.hpp"

using namespace std;

QQueue<QList<QString> > d;
QWaitCondition cv;
QWaitCondition cv1;
QMutex mux;
QMutex mux1;
QMutex mux2;
QMutex mux3;
atomic<bool> done = { false };
atomic<int> done_count;



using words_counter_t = QMap<QString, int>;
words_counter_t words;
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
    CountingThread(atomic <int> &done_count);
    void run();

protected:
    atomic <int> &done_count;
};

CountingThread::CountingThread(atomic <int> &done_count)
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
    MergingThread(atomic <int> &done_count);
    void run();

protected:
    atomic <int> &done_count;

};

MergingThread::MergingThread(atomic <int> &done_count)
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
        words[itr.key()] += itr.value();
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
    char thread[16];
    string base;
    string infile;
    string outfile;
    int num_threads = 5;
    done_count = {num_threads};

    QString in_filename = "/home/yaryna/Desktop/one.txt";
    QString out_filename = "/home/yaryna/Desktop/NEWRESULTS.txt";

    // ----------------------------------------------

    auto creating_threads_start_time = get_current_time_fenced();

    QThread* thr1 = new ReadingThread(in_filename);

    //!!!!!!!!!!!!!!!!!!!!!!!!

    QList<CountingThread*> thread_lst;
    int num_pointer = 0;
    for (int el = 0; el < num_threads; el++) {
        thread_lst.append(new CountingThread(done_count));
    }
    QThread* merging = new MergingThread(done_count);

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 //starting all the threads
    auto indexing_start_time = get_current_time_fenced();
    thr1->start();
    for (auto thread : thread_lst) {
        if (num_threads > 1) {
            thread->start(); // thread->run(); STARTS CODE IN THIS THREAD! Use start to run code in other thread.
        }
        else {
            thread->run(); // Do not use threads at all.
        }
    }
    merging->start();



//waiting and deleting
//    thr1->wait();
    for (auto thread : thread_lst) {

        thread->wait();
    }

    merging->wait();


    thr1->deleteLater();

    for (auto thread : thread_lst) {

        thread->deleteLater();
    }

   // qDebug() << map_q;

    merging->deleteLater();


    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1

    auto indexing_done_time = get_current_time_fenced();

    //________________________________________________

    int total_words = 0;
    for (auto it = words.begin(); it != words.end(); ++it) {
        total_words += it.value();
    }

    auto time_res = to_us(indexing_done_time - indexing_start_time);
    auto creating_threads_time = to_us(indexing_start_time - creating_threads_start_time);
    cout << "INDEXING TIME: " << time_res << " us " << endl;
    cout << "THREADS CREATING TIME: " << creating_threads_time << " us " << endl;

    //---------------------------------------------------------------
    QFile output_file(out_filename);
    if (!output_file.open(QIODevice::WriteOnly)) {
        cerr << "Could not write file with results." << endl;
        return -1;
    }

    QTextStream output_stream(&output_file);

    output_stream << "Total words: " << total_words << endl;
    output_stream << "Total time: " << time_res << endl;
    for (auto it = words.begin(); it != words.end(); ++it) {
        // Format output here.
        output_stream << QString("%1 : %2 \n").arg(it.key(), 10).arg(it.value(), 10);
    }
    output_file.close();
}
