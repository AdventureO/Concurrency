// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include <algorithm>
#include "aux_tools.hpp"
#include "clean_words.hpp"

#include "measurements.hpp"


using namespace std;

#ifdef USE_UNORDERED_MAP
using map_type = unordered_map<string, unsigned int>;
#else // Default
using map_type = map<string, unsigned int>;
#endif

int main()
{
    //! Щоб не втонути в національних налаштуваннях, вирішив
    //! всюди використовувати C-локаль, і викидати всі символи,
    //! які не є ASCII-літерами чи цифрами.
    //! (Тому фільтрую по !isalnum() а не ispunct()).
    //! Нам потрібна портабельна програма -- яка однаково працюватиме
    //! на всіх платформах, однак, підтримка національних символів
    //! і коректна підтримка локалізації на кожній платформі
    //! а) не особливо потрібна,
    //! б) вимагатиме неміряно ресурсів.
    setlocale(LC_ALL,"C");

    auto config = read_config("data_input_conc.txt");

    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];

    string etalon_a_file  = config["etalon_a_file"];

    int measurement_flags = str_to_val<int>(config["measurement_flags"]);

    cpu_measurements_provider_t cpu_measurements;
    all_measurements_t measurements(cpu_measurements, measurement_flags);
    //=============================================================
    int started_mark_idx = measurements.mark_start("Started reading");
    measurements.measure();

    map_type wordsMap;

    ifstream data_file(infile);
    if (!data_file.is_open()) {
        cerr << "Error reading from file: " << infile << endl;
        return 1;
    }

    string word;
    while(data_file >> word)
    {
        cleanWord(word);
        ++wordsMap[word];
    }
    //=============================================================
    measurements.measure();
    measurements.mark_finish(started_mark_idx);

    write_sorted_by_key(out_by_a, wordsMap);
    write_sorted_by_value(out_by_n, wordsMap);
    if( !etalon_a_file.empty() ){
        write_sorted_by_key(etalon_a_file, wordsMap);
    }


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

    return 0;
}
