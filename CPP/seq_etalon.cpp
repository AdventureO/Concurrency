// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include <algorithm>
#include "aux_tools.hpp"
#include "clean_words.hpp"

using namespace std;

#ifdef USE_UNORDERED_MAP
using map_type = unordered_map<string, unsigned int>;
#else // Default
using map_type = map<string, unsigned int>;
#endif

int main()
{
    auto config = read_config("data_input_conc.txt");

    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];

    string etalon_a_file  = config["etalon_a_file"];

    //=============================================================
    auto start = get_current_time_fenced();

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
    auto finish = get_current_time_fenced();

    ofstream file(out_by_a);
    if (!file) {
        cerr << "Could not open file " << out_by_a << endl;
        return 1;
    }
    write_sorted_by_key(file, wordsMap);
    file.close();

    //Write in file words by alphabet
    ofstream file2(out_by_n);
    if (!file2) {
        cerr << "Could not open file " << out_by_n << endl;
        return 1;
    }
    write_sorted_by_value(file2, wordsMap);
    file2.close();

    if( !etalon_a_file.empty() )
    {
        ofstream fileEtalon(etalon_a_file);
        if (!fileEtalon) {
            cerr << "Could not open file " << etalon_a_file << endl;
            return 1;
        }
        write_sorted_by_key(fileEtalon, wordsMap);
    }

    auto total = finish - start;

    cout << "Total time: " << to_us(total) << endl;
    return 0;
}
