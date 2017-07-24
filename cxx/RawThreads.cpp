// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <iostream>
#include <fstream>
#include <map>
#include <thread>
#include <vector>
#include <algorithm>
#include <set>
#include <mutex>
#include <chrono>
#include <atomic>
#include <sstream>

#include "measurements.hpp"


#include "aux_tools.hpp"
#include "clean_words.hpp"

#ifdef USE_UNORDERED_MAP
using map_type = std::unordered_map<std::string, unsigned int>;
#else // Default
using map_type = std::map<std::string, unsigned int>;
#endif

using namespace std;



vector<string> readData(ifstream& file) {
    vector<string> words;
    string word;
    while (file >> word) {
        words.push_back(word);
    }
    return words;
}


template<typename Iter>
void wordCounter(Iter start, Iter end, map_type& m, mutex& mtx)
{
    map_type localMp;

    for (auto i = start; i < end; ++i) {
        cleanWord(*i);
        ++localMp[*i];
    }
    lock_guard<mutex> lg(mtx);

    for(auto& item: localMp){
        m[item.first] += item.second;
    }
}


//! vec is not const, because latter it's iterators will be used to modify it's
//! elements -- see wordCounter.
vector<vector<string>::iterator> SplitVector(vector<string>& vec, size_t n) {

    vector<vector<string>::iterator> outVec;
    auto part_length = vec.size() / n;
    auto sum = vec.begin();
    outVec.push_back(vec.begin());

    for(size_t i = 0; i<n-1;++i){
        advance(sum, part_length);
        outVec.push_back(sum);
        // cout<<outVec[i]<<endl;
    }
    outVec.push_back(vec.end());
    return outVec;
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL,"C");
    auto config = read_config("data_input_conc.txt");

    string infile      = config["infile"];
    string out_by_a    = config["out_by_a"];
    string out_by_n    = config["out_by_n"];
    size_t threads_n   = str_to_val<size_t>(config["threads"]);

    string etalon_a_file  = config["etalon_a_file"];

    int measurement_flags = str_to_val<int>(config["measurement_flags"]);

    cpu_measurements_provider_t cpu_measurements;
    all_measurements_t measurements(cpu_measurements, measurement_flags);

    //=============================================================
    int started_mark_idx = measurements.mark_start("Started reading");
    measurements.measure();

    ifstream data_file(infile);
    if (!data_file.is_open()) {
        cerr << "Error reading from file: " << infile << endl;
        return 1;
    }
    vector<string> words{readData(data_file)};
    data_file.close();

    //=============================================================
    int started_cnt_idx = measurements.mark_start("Started counting");
    measurements.measure();

    mutex mtx;
    map_type wordsMap;
    vector<thread> threads;
    auto work_parts{SplitVector(words, threads_n)};

    for (auto a = work_parts.begin(); a < work_parts.end()-1; ++a) {
        threads.emplace_back( wordCounter<vector<string>::iterator>,
                              *a, *(a + 1), ref(wordsMap), ref(mtx) );
    }

    for (auto& th : threads) th.join();

    //=============================================================
    measurements.measure();
    measurements.mark_finish(started_mark_idx);
    measurements.mark_finish(started_cnt_idx);

    write_sorted_by_key(out_by_a, wordsMap);
    write_sorted_by_value(out_by_n, wordsMap);

    if(measurements.mts & all_measurements_t::BASE_MSM)
    {
        cout << "Total:" << endl;
        (measurements.wt[2] - measurements.wt[0]).print(cout);
        cout << "Analisys:" << endl;
        (measurements.wt[2] - measurements.wt[1]).print(cout);
    }
    if(measurements.mts & all_measurements_t::SYS_MSM)
    {
        cout << "=========System=specific=data=====================" << endl;
        cout << "Total:" << endl;
        (measurements.st[2] - measurements.st[0]).print(cout, "Process");
        cout << "Analisys:" << endl;
        (measurements.st[2] - measurements.st[1]).print(cout, "Process");
    }
    if(measurements.mts & all_measurements_t::CPU_MSM)
    {
        cout << "=========CPU=data===========================" << endl;
        cout << "Total:" << endl;
        print_cpu_params(measurements.ct[0], measurements.ct[2]);
        cout << "Analisys:" << endl;
        print_cpu_params(measurements.ct[1], measurements.ct[2]);
        cout << "=========Auxiliary=data===========================" << endl;
    }
    bool are_correct = true;
    if( !etalon_a_file.empty() )
    {
        are_correct = compareFiles(out_by_a, etalon_a_file);
    }
    return !are_correct;

}
