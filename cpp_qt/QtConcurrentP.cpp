#include <iostream>
#include <QtDebug>
#include <QTextStream>
#include <QString>
#include <QMap>
#include <QtConcurrent>
#include <QFile>
#include <QCoreApplication>

#include "q_clean_words.hpp"

//! Немає сенсу переписувати допоміжні інструменти на Qt
#include "../cxx/aux_tools.hpp"

using map_type = QMap<QString, unsigned int>;
using string_list_type = QStringList;
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

using work_interval = QPair<string_list_type::iterator, string_list_type::iterator>;

QVector<work_interval> splitWork(string_list_type& data_lst, size_t threads) {
    QVector<work_interval> general;
    auto part_length = data_lst.size() / threads;
    auto start = data_lst.begin();
    auto finish = start;
    std::advance(finish, part_length);
    general.append( qMakePair(start, finish) );
    for(size_t i = 0; i<threads-1;++i){
        start = finish;
        std::advance(finish, part_length);
        general.append(qMakePair(start, finish));
    }
    general.append(qMakePair(finish, data_lst.end()));
    return general;
}

map_type mapper(const work_interval& work) {
    map_type local_dictionary;
    for (auto a=work.first; a<work.second; a++) {
        qtCleanWord(*a);
        ++local_dictionary[*a];
    }
    return local_dictionary;
}

void reducer(map_type &words, const map_type& local_dictionary) {
    for(auto itr=local_dictionary.cbegin(); itr!=local_dictionary.cend(); ++itr)
    {
        words[itr.key()]+=itr.value();
    }
}

int main(int argc, char *argv[])
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

    string_list_type words_lst = qtReadData(infile);
    if (words_lst.isEmpty()) {
        std::cerr << "No data in the file or mistake in configuration" << std::endl;
        return -1;
    }
    Q_ASSERT_X( words_lst.size() >= threads_n, "main", "Too small file -- should contain more words than threads started.");

    auto work_parts = splitWork(words_lst, threads_n);

    auto indexing_start_time = get_current_time_fenced();

    auto resultFuture = QtConcurrent::mappedReduced(work_parts, mapper, reducer);

    map_type wordsMap = resultFuture.result();

    auto indexing_done_time = get_current_time_fenced();
    //=============================================================
    auto indexing_time = to_us(indexing_done_time - indexing_start_time);
    //auto reading_time = to_us(indexing_start_time - reading_start_time);
    auto total_time = to_us(indexing_done_time - reading_start_time);

    std::cout << "Total time    : " << total_time << std::endl;
    std::cout << "Analisys time : " << indexing_time << std::endl;

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



