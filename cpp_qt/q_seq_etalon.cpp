// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>

#include <QtDebug>
#include <QCoreApplication>
#include <QMap>
#include <QFile>
#include <QException>

#include "../cxx/aux_tools.hpp"
#include "../cxx/measurements.hpp"
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

    QString etalon_a_file  = QString::fromStdString(config["etalon_a_file"]);

    int measurement_flags = str_to_val<int>(config["measurement_flags"]);

    cpu_measurements_provider_t cpu_measurements;
    all_measurements_t measurements(cpu_measurements, measurement_flags);
    //=============================================================
    int started_mark_idx = measurements.mark_start("Started reading");
    measurements.measure();

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
    measurements.measure();
    measurements.mark_finish(started_mark_idx);

    if(measurements.mts & all_measurements_t::BASE_MSM)
    {
        std::cout << "Total:" << std::endl;
        (measurements.wt[1] - measurements.wt[0]).print(std::cout);
    }
    if(measurements.mts & all_measurements_t::SYS_MSM)
    {
        std::cout << "=========System=specific=data=====================" << std::endl;
        std::cout << "Total:" << std::endl;
        (measurements.st[1] - measurements.st[0]).print(std::cout, "Process");
    }
    if(measurements.mts & all_measurements_t::CPU_MSM)
    {
        std::cout << "=========CPU=data===========================" << std::endl;
        std::cout << "Total:" << std::endl;
        print_cpu_params(measurements.ct[0], measurements.ct[1]);
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
