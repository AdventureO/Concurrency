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


using namespace std;
//g++ read.cpp -pthread -std=c++11
using words_counter_t = map<string, int>;
words_counter_t m;


void printMap(const words_counter_t &m) {
    for (auto elem : m) {
        cout << elem.first << " : " << elem.second << "\n";
    }
}


vector<string> open_read(string path) {
    ifstream myfile;
    vector<string> words;
    string word;
    myfile.open(path);
    if (!myfile.is_open()) {
        cerr << "Error" << endl;
        return words;
    }
    string formated_word;
    while (myfile >> word) {
        words.push_back(word);
    }
    myfile.close();
    return words;
}

void write_to_file(const words_counter_t &m, string path) {
    ofstream myfile;
    myfile.open(path);
    for (auto elem : m) {
        myfile << elem.first << "    " << elem.second << "\n";
    }
    myfile.close();
}

words_counter_t mapper(int l, int r, vector<string> &words) {
    words_counter_t mp;
    for (; l <= r; ++l) {
        string y = words[l];
        y.erase(remove_if(y.begin(), y.end(), [](char x) {return !isalpha(x);} ),y.end() );
        transform(y.begin(), y.end(), y.begin(), ::tolower);
        ++mp[y];
    }
    return mp;

}

void reducer( words_counter_t &master, const words_counter_t& mp){
    for (auto w: mp) {
        master[w.first] += w.second;
    }
}


void worker2(int l, int r, vector<string> &words,
             promise<words_counter_t> p){
    auto res = mapper(l, r, words);
    p.set_value(res);
}


vector<int> SplitVector(const vector<string>& vec, int n) {

    vector<int> outVec;
    int length = int(vec.size())/ n;
    int count = 0;
    int sum = 0;

    outVec.push_back(0);
    while(count != n - 1){
        sum += length;
        outVec.push_back(sum);
        //cout<<outVec[count]<<endl;
        count++;
    }
    outVec.push_back(int(vec.size()));
    return outVec;
}

inline std::chrono::high_resolution_clock::time_point get_current_time_fenced() {
    std::atomic_thread_fence(memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D& d) {
    return std::chrono::duration_cast<chrono::microseconds>(d).count();
}

int main(int argc, char *argv[]) {  // input_file, threads, output_file
    istringstream ss1(argv[1]);
    int numT;
    ss1 >> numT;
    string input_data[2], infile, out_by;
    ifstream myFile;
    myFile.open("data_input.txt");

    for(int i = 0; i < 2; i++)
        myFile >> input_data[i];
    myFile.close();

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < input_data[i].length(); j++) {
            if (input_data[i][j] == '=')
                input_data[i][j] = ' ';
        }
        stringstream ss(input_data[i]);
        string temp;
        int k = 0;
        while (ss >> temp) {
            if (k != 0) {
                stringstream lineStream(temp);
                string s;
                getline(lineStream,s,',');
                s.erase(remove( s.begin(), s.end(), '\"' ), s.end());
                input_data[i] = s;
            }
            k++;
        }
    }

    infile = input_data[0];
    out_by = input_data[1];


    vector<string> words;
    vector< future<words_counter_t> > result_futures;


    words = open_read(infile);

    auto start = get_current_time_fenced();

    vector<int> list_of_words_amount = SplitVector(words, numT);

    vector<thread> rthreads;

    auto startPar = get_current_time_fenced();

    for (int i = 0; i < list_of_words_amount.size()-1; ++i) {
        promise<words_counter_t> rg;
        result_futures.push_back(rg.get_future());
        rthreads.push_back(thread(worker2, list_of_words_amount[i], list_of_words_amount[i+1], ref(words), move(rg)));
    }

    vector<words_counter_t> results;
    for(size_t i = 0; i<result_futures.size(); ++i)
    {
        reducer(m, result_futures[i].get());
    }

    for (int i = 0; i < numT; ++i) {
        rthreads[i].join();
    }
    auto finishPar = get_current_time_fenced();
    auto counting = finishPar - startPar;
    write_to_file(m, "result_word.txt");
    auto finish = get_current_time_fenced();
    //cout << "Total read: " << to_us(totalRead) << endl;
    //cout << "Total Parallel: " << to_us(totalPar) << endl;
    //cout << "Total time: " << to_us(total) << endl;
    auto total = finish - start;
    ofstream myfile;
    myfile.open (out_by);
    myfile << to_us(counting)<<'\n';
    myfile << to_us(total)<<'\n';
    myfile.close();
    //write_to_file(m, "A.txt");
    //printMap(m);

    return 0;
}

