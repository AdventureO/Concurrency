// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <future>
#include <algorithm>

#include <map>
#include <unordered_map>

#include "aux_tools.hpp"
#include "clean_words.hpp"

using namespace std;

#ifdef USE_UNORDERED_MAP
using map_type = unordered_map<string, unsigned int>;
#else // Default
using map_type = map<string, unsigned int>;
#endif

template<typename Iter>
map_type wordCounter(Iter start, Iter end)
{
    map_type localMp;

    for (auto i = start; i < end; ++i) {
        cleanWord(*i);
        ++localMp[*i];
    }
    return localMp;
}

void reducer( map_type &master, const map_type& mp){
    for (auto w: mp) {
        master[w.first] += w.second;
    }
}

template<typename Iter>
void wordCounterWrapper(Iter start, Iter end, promise<map_type> p){
    p.set_value( wordCounter(start, end) );
}

vector<string> readData(ifstream& file) {
    vector<string> words;
    string word;
    while (file >> word) {
        words.push_back(word);
    }
    return words;
}

//! vec is not const, because latter it's iterators will be used to modify it's
//! elements -- see wordCounter.
vector<vector<string>::iterator> SplitVector(vector<string>& vec, unsigned n) {

    vector<vector<string>::iterator> outVec;
    auto part_length = vec.size() / n;
    auto sum = vec.begin();
    outVec.push_back(vec.begin());

    for(int i = 0; i<n-1;++i){
        advance(sum, part_length);
        outVec.push_back(sum);
        // cout<<outVec[i]<<endl;
    }
    outVec.push_back(vec.end());
    return outVec;
}


int main(int argc, char *argv[]) {
    auto config = read_config("data_input_conc.txt");

    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];
    int    threads_n = str_to_val<unsigned>(config["threads"]);

    string etalon_a_file  = config["etalon_a_file"];

    auto start = get_current_time_fenced();

    ifstream data_file(infile);
    if (!data_file.is_open()) {
        cerr << "Error reading from file: " << infile << endl;
        return 1;
    }
    vector<string> words{readData(data_file)};
    data_file.close();

    auto readed = get_current_time_fenced();

    map_type wordsMap;
    vector< future<map_type> > result_futures;
    vector<thread> threads;

    auto work_parts{SplitVector(words, threads_n)};

    auto startPar = get_current_time_fenced();

    for (auto a = work_parts.begin(); a < work_parts.end()-1; ++a) {
        promise<map_type> rg;
        result_futures.push_back(rg.get_future());
        threads.emplace_back(wordCounterWrapper<vector<string>::iterator>, *a, *(a + 1), move(rg) );
    }


    //TODO: Implement scan for ready futures
    for(auto& result : result_futures)
    {
        reducer(wordsMap, result.get());
    }

    for (auto& t: threads) {
        t.join();
    }

    auto finished = get_current_time_fenced();

    write_sorted_by_key(out_by_a, wordsMap);
    write_sorted_by_value(out_by_n, wordsMap);

    auto counting = finished - readed;
    auto total = finished - start;

    cout << "Total time    : " << to_us(total) << endl;
    cout << "Analisys time : " << to_us(counting) << endl;

    bool are_correct = true;
    if( !etalon_a_file.empty() )
    {
        are_correct = compareFiles(out_by_a, etalon_a_file);
    }
    return !are_correct;
}

