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

#include "aux_tools.hpp"
#include "clean_words.hpp"

#ifdef USE_UNORDERED_MAP
using map_type = std::unordered_map<std::string, unsigned int>;
#else // Default
using map_type = std::map<std::string, unsigned int>;
#endif

using namespace std;
mutex mtx;


vector<string> readData(ifstream& file) {
    vector<string> words;
    string word;
    while (file >> word) {
        words.push_back(word);
    }
    return words;
}


void wordCounter(vector<string>& myVector, size_t start, size_t end, map_type& m)
{
    map_type localMp;

    for (size_t i = start; i < end; i++) {
        {
            cleanWord(myVector[i]);
            ++localMp[myVector[i]];
        }
    }
    lock_guard<mutex> lg(mtx);

    for(auto& item: localMp){
        m[item.first] += item.second;

    }
}


vector<size_t> SplitVector(const vector<string>& vec, int n) {

    vector<size_t> outVec;
    auto part_length = vec.size() / n;
    size_t sum = 0;
    outVec.push_back(0);

    for(int i = 0; i<n-1;++i){
        sum += part_length;
        outVec.push_back(sum);
        // cout<<outVec[i]<<endl;
    }
    outVec.push_back(vec.size());
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
    vector<thread> threads;
    vector<size_t> work_parts = SplitVector(words, threads_n);
    auto startPar = get_current_time_fenced();
    for (int a = 0; a < work_parts.size()-1; ++a) {

        threads.emplace_back( wordCounter, ref(words), work_parts[a],
                              work_parts[a + 1], ref(wordsMap) );
    }

    for (auto& th : threads) th.join();

    auto finished = get_current_time_fenced();

    ofstream file(out_by_a);
    if (!file) {
        cerr << "Could not open file."<< endl;
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

    return 0;

}
