// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include <unordered_map>

using namespace std;

#define USE_UNORDERED_MAP

#ifdef USE_UNORDERED_MAP
using map_type = unordered_map<string, unsigned int>;
#else // Default
using map_type = map<string, unsigned int>;
#endif

//! Intentionally trivial.
//! Спроба перетворити таку конструкцію в повноцінну чергу
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

    //! So-called universal reference -- template + &&, moves if possible.
    //! Compiles only for rvalue references, for example temporatire and move-ed variables
    void enque(T&& lines )
    {
        {
            lock_guard<mutex> ll(mtx);
            que.push_back(move(lines));
        }
        cv.notify_one();
    }

    void one_thread_done()
    {
        --left_threads;
        cv.notify_all();
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

int fileReaderProducer(ifstream& file, SimpleQueStruct<vector<string>>& dq, unsigned blockSize) {
    string line;
    vector<string> lines;
    while(getline(file, line))
    {
        lines.push_back(move(line));
        if (lines.size() == blockSize)
        {
            dq.enque(move(lines)); // Move to avoid copy of LARGE objects
            lines.clear(); //! Usefull also after move: https://stackoverflow.com/questions/27376623/can-you-reuse-a-moved-stdstring
        }
    }
    if (lines.size() != 0)
    {
        dq.enque(move(lines));
    }

    //notify all consumers that file has "depleted"
    dq.one_thread_done();
    return 0;
}

void cleanWord(string &word)
{
    word.erase( remove_if(word.begin(), word.end(), ::ispunct), word.end() );
    transform(word.begin(), word.end(), word.begin(), ::tolower);
}

int countWordsConsumer(SimpleQueStruct<vector<string>>&dq,
                       SimpleQueStruct<map_type> &dq1) {
    while(true) {
        map_type localMap;
        unique_lock<mutex> luk(dq.mtx);
        if (!dq.que.empty()) {
            vector<string> v{move(dq.que.front())};
            dq.que.pop_front();
            luk.unlock();            

            for (const auto& in_word: v) {
                string word;
                istringstream iss(in_word);
                while (iss >> word) {
                    cleanWord(word);
                    ++localMap[word];
                }
            }
            dq1.enque(move(localMap)); // No need to clear -- recreated on next iteration. But be carefull!
        } else {
            if(dq.left_threads == 0) {
                break;
            }
            dq.cv.wait(luk);
        }
    }
    dq1.one_thread_done();
    return 0;

}

void mergeMapsConsumer(SimpleQueStruct<map_type>& dq1, map_type& wordsMap) {

    while(true) {
        unique_lock<mutex> luk1(dq1.mtx);
        if (!dq1.que.empty()) {
            map_type v1{move(dq1.que.front())};
            dq1.que.pop_front();
            luk1.unlock();

            for (auto& item: v1) {
                wordsMap[item.first] += item.second;
            }


        } else {
            if ((dq1.left_threads == 0)) {
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


//! std::map is sorted by keys. So we need other function for unorderd_map
template<typename KeyT, typename ValueT>
void write_sorted_by_key(ostream& file, const map<KeyT, ValueT>& data)
{
    for(auto& item: data)
        file << item.first << ": " << item.second << '\n';
    // '\n' does not flushes the buffers, so is faster than endl.
}


//! Or should we enforce sort for any map-like container, except std::map?
template<typename KeyT, typename ValueT>
void write_sorted_by_key(ostream& file, const unordered_map<KeyT, ValueT>& data)
{
    using VectorOfItemsT = vector< std::pair<KeyT, ValueT> >;
    VectorOfItemsT VectorOfItems;
    for(auto& item: data)
    {
        VectorOfItems.emplace_back(item);
    }
    sort(VectorOfItems.begin(), VectorOfItems.end());

    for(auto& item: VectorOfItems)
        file << item.first << ": " << item.second << '\n';
}


//! Sorting any map here, so need just complete map type --
//! no need to overload over map/unordered_map
template<typename MapT>
void write_sorted_by_value(ostream& file, const MapT& data)
{
    using VectorOfItemsT = vector< std::pair<typename MapT::key_type,
                                             typename MapT::mapped_type> >;
    VectorOfItemsT VectorOfItems;
    for(auto& item: data)
    {
        VectorOfItems.emplace_back(item);
    }
    sort(VectorOfItems.begin(), VectorOfItems.end(),
         [] (const typename VectorOfItemsT::value_type &a,
             const typename VectorOfItemsT::value_type &b) {
                return a.second > b.second;
          }
    );

    for(auto& item: VectorOfItems)
        file << item.first << ": " << item.second << '\n';
}


//! Compares two files, ignoring spaces.
//! IT IS BAD FUNCTION. IT PRINTS...
//! As for now I'm just too lazy to create corresponing
//! comparison results structure.
//! Взагалі, ця функція видається мені потворною, але поки переписувати лінь.
//! Для спрощення прочитати обидва файли у вектори і там порівнювати?
//! Якщо посортувати -- можна буде ще й різного сортування файли порівнювати.
//! Але і мінусів при тому багато -- наприклад, губляться рядки відмінносте.
bool compareFiles(const string& file1, const string& file2)
{
    std::ifstream f1(file1);
    std::ifstream f2(file2);

    if (f1.fail()) {
        throw std::runtime_error("Failed to open file for comparison: " + file1);
    }
    if (f2.fail()) {
        throw std::runtime_error("Failed to open file for comparison: " + file2);
    }
    string str1, str2;
    size_t cur_line = 0;
    while(getline(f1, str1), getline(f2, str2), f1 && f2)
    {
        str1.erase( remove_if(str1.begin(), str1.end(), ::isspace), str1.end() );
        str2.erase( remove_if(str2.begin(), str2.end(), ::isspace), str2.end() );
        if(str1 != str2)
        {
            cerr << "Difference at line " << cur_line << endl;
            cerr << "\t First  file: |" << str1 << "|" << endl;
            cerr << "\t Second file: |" << str2 << "|" << endl;
            return false;
        }
        ++cur_line;
    }
    // Remove empty lines at the end
    if( !f1.eof() )
    {
        do
        {
            str1.erase( remove_if(str1.begin(), str1.end(), ::isspace), str1.end() );
            if(!str1.empty())
            {
                cerr << "Excess line in file 1: " << str1 << endl;
                return false;
            }
        }
        while(getline(f1, str1));
    }
    if( !f2.eof() )
    {
        do
        {
            str1.erase( remove_if(str2.begin(), str2.end(), ::isspace), str2.end() );
            if(!str2.empty())
            {
                cerr << "Excess line in file 1: " << str2 << endl;
                return false;
            }
        }
        while(getline(f2, str2));
    }

    if(f1.eof() && f2.eof() )
    {
        return true;
    }
    else
    {
        if( f2.eof() )
        { // First file is not finished
            cerr << "First file is longer." << endl;
        }else
        { // Second file is not finished
            cerr << "Second file is longer." << endl;
        }
        return false;
    }
}

int main() {
    auto config = read_config("data_input_conc.txt");


    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];
    int    blockSize = str_to_val<unsigned>(config["blockSize"]);
    int    threads_n = str_to_val<unsigned>(config["threads"]);

    string etalon_a_file  = config["etalon_a_file"];

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

    map_type wordsMap;
    SimpleQueStruct<vector<string>> readBlocksQ{1};
    SimpleQueStruct<map_type> localDictsQ{threads_n-2};

    ifstream data_file(infile);
    if (!data_file.is_open()) {
        cerr << "Error reading from file: " << infile << endl;
        return 1;
    }

    threads.emplace_back(fileReaderProducer, ref(data_file), ref(readBlocksQ), blockSize);

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



    ofstream file(out_by_a);
    if (!file) {
        cerr << "Could not open file."<< endl;
        return 1;
    }
    write_sorted_by_key(file, wordsMap);
    file.close();

    //Write in file words by alphabet
    ofstream file2(out_by_n);
    write_sorted_by_value(file2, wordsMap);
    file2.close();

    auto finishConsumer = get_current_time_fenced();
    auto total = finishConsumer - startProducer;

    cout << "Time: " << to_us(total) << endl;

    bool are_correct = true;
    if( !etalon_a_file.empty() )
    {
        are_correct = compareFiles(out_by_a, etalon_a_file);
    }
    return !are_correct;
}
