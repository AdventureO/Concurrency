#include <iostream>
#include <deque>
#include <atomic>
#include <mutex>
#include <thread>
#include <map>
#include <fstream>
#include <condition_variable>
#include <vector>
#include <sstream>
#include <algorithm>
//#include <cstring>
#include <cctype>

using namespace std;

condition_variable cv;
condition_variable cv1;
mutex mtx;
mutex mtx1;
mutex mtx2;
mutex mtx3;
mutex mtx4;
mutex mtx5;
atomic <bool> isReady = {false};
atomic <bool> isReady1 = {false};

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

int producer(const string& filename, deque<vector<string>>& dq) {
    fstream file(filename);
    if (!file.is_open()) {
        cout << "error reading from file";
        return  0;
    }
    string line;
    vector<string> lines;
    int counter = 0;
    int linesBlock = 100;
    while(getline(file, line))
    {
        lines.push_back(line);
        if (counter == linesBlock)
        {
            {
                lock_guard<mutex> guard(mtx);
                dq.push_back(lines);
            }
            cv.notify_one();
            lines.clear();
            counter = 0;
        } else
        {
            counter++;
        }
    }
    if (lines.size() != 0)
    {
        {
            lock_guard<mutex> ll(mtx);
            dq.push_back(lines);
        }
        cv.notify_one();
    }

    //notify all consumers that words have finished
    cv.notify_all();
    isReady = true;
    return 0;
}

int consumer(deque<vector<string>> &dq, deque<map<string, int>> &dq1, atomic <int> &numT) {
    map<string, int> localMap;
    while(true) {
        unique_lock<mutex> luk(mtx);
        if (!dq.empty()) {
            vector<string> v{dq.front()};
            dq.pop_front();
            luk.unlock();
            string word;

            for (int i = 0; i < v.size(); i++) {
                istringstream iss(v[i]);
                while (iss >> word) {
                    auto to = begin(word);
                    for (auto from : word)
                        if (!ispunct(from))
                            *to++ = from;
                    word.resize(distance(begin(word), to));
                    transform(word.begin(), word.end(), word.begin(), ::tolower);
                    lock_guard<mutex> lg(mtx2);
                    ++localMap[word];
                }
            }

            {
                lock_guard<mutex> lg(mtx1);
                dq1.push_back(localMap);

            }
            cv1.notify_one();


        } else {
            if(isReady) {
                break;
            }
            cv.wait(luk);
        }
    }
    cv1.notify_all();
    numT --;
  //  cout<<numT<<endl;
    return 0;

}

int finalConsumer(deque<map<string, int>>& dq1, map<string, int>& wordsMap, atomic <int> &numT) {

    while(true) {
        unique_lock<mutex> luk1(mtx1);
            //cout<<"\t" << numT<<endl;
        if (!dq1.empty()) {
            map<string, int> v1{dq1.front()};
            dq1.pop_front();
            luk1.unlock();

            for (map<string,int>::iterator it=v1.begin(); it!=v1.end(); ++it) {
                lock_guard<mutex> lg(mtx3);
                wordsMap[it->first] += it->second;
            }


        } else {
            if ((numT == 0)&&(dq1.empty())) {
                //cout<<"HERE"<<endl;
                break;
            }
            cv1.wait(luk1);
        }
    }
//    cout << "\tTipaFone\n" << endl;
    return 0;

}

map<string,string> read_config(const string& filename)
{
    map<string,string> result;
    ifstream confFile(filename);
    if( !confFile.is_open() ){
        throw std::runtime_error("Failed to open file: " + filename);
    }
    string str;
    while( getline(confFile,str) )
    {
        auto cut_pos = str.find("#");
        if(cut_pos!=string::npos)
            str.erase(cut_pos);
        auto cut_iter = remove_if(str.begin(), str.end(), ::isspace);
        str.erase( cut_iter, str.end() );
        if( str.empty() )
            continue;
        auto pos = str.find('=');
        if(pos==string::npos){
            throw std::runtime_error("Wrong option: " + str + "\n in file: " + filename);
        }
        string name {str, 0, pos};
        string value{str, pos+1, string::npos};
        cut_iter = std::remove(value.begin(), value.end(), '\"');
        value.erase(cut_iter, value.end());
        result[name] = value;
    }
    return result;

}

template<typename T>
T str_to_val(const string& s)
{
    stringstream ss(s);
    T res;
    ss >> res;
    if(!ss){
        throw std::runtime_error("Failed converting: " + s);
    }
    return res;
}

#define CHECK_READED_CONFIGURATION
int main() {
    auto config = read_config("data_input_conc.txt");

    deque<vector<string>> dq;
    deque<map<string, int>> dq1;
    map<string, int> wordsMap;

    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];
    int    threads_n = str_to_val<int>(config["threads"]);
    atomic<int> numT{threads_n-2};

#ifdef CHECK_READED_CONFIGURATION
    for(auto& option: config)
    {
        cout << option.first << "\t" << option.second << endl;
    }
    cout << "infile: " << infile << endl;
    cout << "out_by_a: " << out_by_a << endl;
    cout << "out_by_n: " << out_by_n << endl;
    cout << "threads_n: " << threads_n << endl;
#endif

    vector<thread> threads;

    auto startProducer = get_current_time_fenced(); //<===
    threads.emplace_back(producer, cref(infile), ref(dq));

    int thrIter = 1;
    while(thrIter != threads_n-1){
        threads.emplace_back( consumer, ref(dq), ref(dq1), ref(numT));
        thrIter++;
        //cout<<thrIter<<endl;
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // threads.emplace_back(finalConsumer, ref(dq1), ref(wordsMap), ref(numT));
    finalConsumer(dq1, wordsMap, numT);

    for (auto& th : threads) {
        th.join();
    }



    ofstream file;
    file.open(out_by_a);
    if (!file) {
        cerr << "Could not open file."<< endl;
        return 1;
    }
    for(map<string, int> :: iterator i = wordsMap.begin(); i != wordsMap.end(); i++){
        file << i->first << ": " << i->second << endl;
    }
    file.close();

    vector<pair<string, int>> VectorOfPair;
    for(map<string, int> :: iterator i = wordsMap.begin(); i != wordsMap.end(); i++){
        VectorOfPair.push_back(make_pair(i -> first, i-> second));
    }
    sort(VectorOfPair.begin(), VectorOfPair.end(), [] (const pair<string,int> &a, const pair<string,int> &b) {
        return a.second > b.second;
    });

    //Write in file words by alphabet
    ofstream file2;
    file2.open(out_by_n);
    for (auto p = VectorOfPair.begin(); p != VectorOfPair.end(); p++)
        file2 << p->first << ": " << p->second << endl;
    file2.close();

    auto finishConsumer = get_current_time_fenced();
    auto total = finishConsumer - startProducer;

    cout << "Time: " << to_us(total) << endl;
}
