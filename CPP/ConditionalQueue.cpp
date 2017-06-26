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
#include <cassert>
#include <cctype>

using namespace std;




//! Intentionally trivial.
//! Спробе перетворити таку конструкцію в повноцінну чергу
//! мала б сенс в реальному коді, але тут вноситиме додаткову
//! неоднозначність -- "А ми вже достатньо добре її реалізували?"
template<typename T, typename Container=deque<T>>
struct SimpleQueStruct
{
    Container que;
    condition_variable cv;
    mutex mtx;
    atomic<int> left_threads;

    SimpleQueStruct(int thr): left_threads(thr){}

    ~SimpleQueStruct(){
        // Just sanity check.
        assert( left_threads == 0 && "Que destroyed before all users stopped.");
    }
};




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

int fileReaderProducer(ifstream& file, SimpleQueStruct<vector<string>>& dq) {
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
                lock_guard<mutex> guard(dq.mtx);
                dq.que.push_back(lines);
            }
            dq.cv.notify_one();
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
            lock_guard<mutex> ll(dq.mtx);
            dq.que.push_back(lines);
        }
        dq.cv.notify_one();
    }

    //notify all consumers that file has "depleted"
    --dq.left_threads;
    dq.cv.notify_all();
    return 0;
}

int countWordsConsumer(SimpleQueStruct<vector<string>>&dq,
                       SimpleQueStruct<map<string, int>> &dq1) {
    while(true) {
        map<string, int> localMap;

        unique_lock<mutex> luk(dq.mtx);
        if (!dq.que.empty()) {
            vector<string> v{dq.que.front()};
            dq.que.pop_front();
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
                    // lock_guard<mutex> lg(mtx2); // WTF?! It's local!
                    ++localMap[word];
                }
            }

            {
                lock_guard<mutex> lg(dq1.mtx);
                dq1.que.push_back(localMap);

            }
            dq1.cv.notify_one();


        } else {
            if(dq.left_threads == 0) {
                break;
            }
            dq.cv.wait(luk);
        }
    }
    --dq1.left_threads;
    dq1.cv.notify_all();
    return 0;

}

void mergeMapsConsumer(SimpleQueStruct<map<string, int>>& dq1, map<string, int>& wordsMap) {

    while(true) {
        unique_lock<mutex> luk1(dq1.mtx);
        if (!dq1.que.empty()) {
            map<string, int> v1{dq1.que.front()};
            dq1.que.pop_front();
            luk1.unlock();

            for (map<string,int>::iterator it=v1.begin(); it!=v1.end(); ++it) {
                //lock_guard<mutex> lg(mtx3); // Не обов'язкове -- пише один потік!
                wordsMap[it->first] += it->second;
            }


        } else {
            if ((dq1.left_threads == 0)&&(dq1.que.empty())) {
                break;
            }
            dq1.cv.wait(luk1);
        }
    }


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

int main() {
    auto config = read_config("data_input_conc.txt");


    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];
    int    threads_n = str_to_val<int>(config["threads"]);

#ifdef CHECK_READ_CONFIGURATION
    for(auto& option: config)
    {
        cout << option.first << "\t" << option.second << endl;
    }
    cout << "infile: " << infile << endl;
    cout << "out_by_a: " << out_by_a << endl;
    cout << "out_by_n: " << out_by_n << endl;
    cout << "threads_n: " << threads_n << endl;
#endif

    auto startProducer = get_current_time_fenced(); //<===

    vector<thread> threads;

    map<string, int> wordsMap;
    SimpleQueStruct<vector<string>> readBlocksQ{1};
    SimpleQueStruct<map<string, int>> localDictsQ{threads_n-2};

    ifstream data_file(infile);
    if (!data_file.is_open()) {
        cerr << "Error reading from file: " << infile << endl;
        return 1;
    }

    threads.emplace_back(fileReaderProducer, ref(data_file), ref(readBlocksQ));

    int thrIter = 1;
    while(thrIter != threads_n-1){
        threads.emplace_back( countWordsConsumer, ref(readBlocksQ), ref(localDictsQ) );
        thrIter++;
    }

    // threads.emplace_back(mergeMapsConsumer, ref(localDictsQ), ref(wordsMap));
    mergeMapsConsumer(localDictsQ, wordsMap);

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
