// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>

#include <QtDebug>
#include <QCoreApplication>
#include <QMap>
#include <QFile>
#include <QException>

#include "../cxx/aux_tools.hpp"
#include "q_clean_words.hpp"

using map_type = QMap<QString, unsigned int>;
using string_list_type = QStringList; //  QVector<QString>

class CCException: public QException
{
    QString m_message;
public:
    QString message() const { return m_message; }
    CCException(const QString& s): m_message(s){}
};



int main(int argc, char* argv[])
{
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
    QFile inputFile(infile);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Error reading file: " + infile;
        throw CCException("Error reading file: " + infile );
    }
    QTextStream in(&inputFile);
    in.setCodec("UTF-8");   // Ми враховуємо лише ASCII-символи, але
                            // це потрібно для коректного (ну, чи просто --
                            // такого ж, як в "голому" С++) розбиття на слова
    QString word;
    while (!in.atEnd()) {
        in >> word;
        qtCleanWord(word);
        ++wordsMap[word];
    }

    //=============================================================
    auto indexing_done_time = get_current_time_fenced();

    auto time_res = to_us(indexing_done_time - creating_threads_start_time);
    std::cout << "Total time : " << time_res << std::endl;

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
