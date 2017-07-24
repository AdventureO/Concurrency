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
#include "../cxx/measurements.hpp"

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

    int measurement_flags = str_to_val<int>(config["measurement_flags"]);

    cpu_measurements_provider_t cpu_measurements;
    all_measurements_t measurements(cpu_measurements, measurement_flags);
    //=============================================================
    int started_mark_idx = measurements.mark_start("Started reading");
    measurements.measure();

    string_list_type words_lst = qtReadData(infile);
    if (words_lst.isEmpty()) {
        std::cerr << "No data in the file or mistake in configuration" << std::endl;
        return -1;
    }
    Q_ASSERT_X( words_lst.size() >= threads_n, "main", "Too small file -- should contain more words than threads started.");

    auto work_parts = splitWork(words_lst, threads_n);

    int started_cnt_idx = measurements.mark_start("Started counting");
    measurements.measure();

    auto resultFuture = QtConcurrent::mappedReduced(work_parts, mapper, reducer);

    map_type wordsMap = resultFuture.result();

    //=============================================================
    measurements.measure();
    measurements.mark_finish(started_mark_idx);
    measurements.mark_finish(started_cnt_idx);

    if(measurements.mts & all_measurements_t::BASE_MSM)
    {
        std::cout << "Total:" << std::endl;
        (measurements.wt[2] - measurements.wt[0]).print(std::cout);
        std::cout << "Analisys:" << std::endl;
        (measurements.wt[2] - measurements.wt[1]).print(std::cout);
    }
    if(measurements.mts & all_measurements_t::SYS_MSM)
    {
        std::cout << "=========System=specific=data=====================" << std::endl;
        std::cout << "Total:" << std::endl;
        (measurements.st[2] - measurements.st[0]).print(std::cout, "Process");
        std::cout << "Analisys:" << std::endl;
        (measurements.st[2] - measurements.st[1]).print(std::cout, "Process");
    }
    if(measurements.mts & all_measurements_t::CPU_MSM)
    {
        std::cout << "=========CPU=data===========================" << std::endl;
        std::cout << "Total:" << std::endl;
        print_cpu_params(measurements.ct[0], measurements.ct[2]);
        std::cout << "Analisys:" << std::endl;
        print_cpu_params(measurements.ct[1], measurements.ct[2]);
        std::cout << "=========Auxiliary=data===========================" << std::endl;
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



