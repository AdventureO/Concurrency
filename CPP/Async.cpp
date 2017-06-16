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

void printMap(const map<string, int> &m) {
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

void write_to_file(const map<string, int> &m, string path) {
    ofstream myfile;
    myfile.open(path);
    for (auto elem : m) {
        myfile << elem.first << " : " << elem.second << "\n";
    }
    myfile.close();
}

map<string, int> mapper(int l, int r, vector<string> &words) {
    map<string, int> mp;
    for (; l <= r; ++l) {
        words[l].erase(remove_if(words[l].begin(), words[l].end(), [](char x) {return !isalpha(x);} ), words[l].end() );
        transform(words[l].begin(), words[l].end(), words[l].begin(), ::tolower);
        ++mp[words[l]];
    }
    return mp;

}

void reducer( map<string, int> &master, const map<string, int>& mp){
    for (auto w: mp) {
        master[w.first] += w.second;
    }
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
        count++;
    }
    outVec.push_back(int(vec.size()));
    return outVec;
}


int main(int argc, char *argv[]) {
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
    map<string, int> m;
    vector< future<map<string, int>> > result_futures;


    words = open_read(infile);
    auto start = get_current_time_fenced();
    vector<int> list_of_words_amount = SplitVector(words, numT);

    auto startPar = get_current_time_fenced();
    for (int a = 0; a < list_of_words_amount.size()-1; ++a) {

        result_futures.push_back(
                async(std::launch::async, mapper,  list_of_words_amount[a],  list_of_words_amount[a+1] - 1, ref(words))
        );
    }

    vector<map<string, int>> results;
    for(size_t i = 0; i<result_futures.size(); ++i)
    {
        reducer(m, result_futures[i].get());
    }
    auto finishPar = get_current_time_fenced();

    auto counting = finishPar - startPar;


    write_to_file(m, "result_word.txt");
    auto finish = get_current_time_fenced();
    auto total = finish - start;


    //cout << "Read Time: " << to_us(totalRead) << endl;
    //cout << "Parallel Time: " << to_us(totalPar) << endl;
    //cout << "Total Time: " << to_us(total) << endl;
    //write_to_file(m, "result.txt");
//printMap(m);

    ofstream myfile;
    myfile.open (out_by);
    myfile << to_us(counting)<<'\n';
    myfile << to_us(total)<<'\n';
    myfile.close();

    return 0;
}

