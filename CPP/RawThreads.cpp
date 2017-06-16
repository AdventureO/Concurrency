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

using namespace std;
mutex mtx;


//for(x: lm)
//gm[x.first] += x.sp
//

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


int finder(vector<string>& myVector, int start, int end, std::map<string, int>& m)
{
    map<string, int> localMp;

    for (int i = start; i < end; i++) {
        {
            myVector[i].erase(remove_if(myVector[i].begin(), myVector[i].end(), [](char x) {return !isalpha(x);} ),myVector[i].end() );
            transform(myVector[i].begin(), myVector[i].end(), myVector[i].begin(), ::tolower);
            ++localMp[myVector[i]];
        }
    }
    lock_guard<mutex> lg(mtx);

    for(map<string, int> :: iterator i = localMp.begin(); i != localMp.end(); i++){
        m[i->first] += i-> second;

    }

    return 0;

}

void write_to_file(const map<string, int> &m, string path) {
    ofstream myfile;
    myfile.open(path);
    for (auto elem : m) {
        myfile << elem.first << "    " << elem.second << "\n";
    }
    myfile.close();
}

void printMap(const map<string, int> &m) {
    for (auto elem : m) {
        cout << elem.first << " : " << elem.second << "\n";
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
        //cout<<outVec[count]<<endl;
        count++;
    }
    outVec.push_back(int(vec.size()));
    return outVec;
}


inline std::chrono::high_resolution_clock::time_point get_current_time_fenced()
{
    std::atomic_thread_fence(memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(memory_order_seq_cst);
    return res_time;
}

template<class D>
inline long long to_us(const D& d)
{
    return std::chrono::duration_cast<chrono::microseconds>(d).count();
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
    words = open_read(infile);

    auto start = get_current_time_fenced();
    vector<pair<string, int>> VectorOfPair;
    map<string, int> m;
    thread threads[numT];
    vector<int> list_of_words_amount = SplitVector(words, numT);
    auto startPar = get_current_time_fenced();
    for (int a = 0; a < list_of_words_amount.size()-1; ++a) {

        threads[a] = thread(finder, ref(words), list_of_words_amount[a],
                            list_of_words_amount[a + 1], ref(m));
    }

    for (auto& th : threads) th.join();

    auto finishPar = get_current_time_fenced();
    auto counting = finishPar - startPar;
    write_to_file(m , "result_word.txt");
    auto finish = get_current_time_fenced();
    auto total = finish - start;
    ofstream myfile;
    myfile.open (out_by);
    myfile << to_us(counting)<<'\n';
    myfile << to_us(total)<<'\n';
    myfile.close();
    //printMap(m);

    return 0;

}