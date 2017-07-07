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
#include <algorithm>
#include <cassert>

#include <list>


#include <unordered_map>

#include "aux_tools.hpp"
#include "clean_words.hpp"

using namespace std;

#ifdef USE_UNORDERED_MAP
using map_type = unordered_map<string, unsigned int>;
#else // Default
using map_type = map<string, unsigned int>;
#endif

//! Rather simple.
//! Не втримався, каюся, таки написав чергу...
//! TODO: Add list support
template<typename T, typename Container=deque<T>>
class SimpleQueStruct
{
private:
    Container que;
    condition_variable cv;
    mutex mtx;
    atomic<size_t> left_threads;
public:
    SimpleQueStruct(size_t thr): left_threads(thr){}

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

    //! Returns true and saves data to argument, if have one
    //! returns false otherwise.
    bool dequeue(T& lines )
    {
        unique_lock<mutex> luk(mtx);
        // cv.wait() checks condition before waiting.
        cv.wait(luk, [this](){return !que.empty() || left_threads == 0;});
        if(left_threads == 0 && que.empty() )
            return false;
        lines=move(que.front());
        que.pop_front();
        return true;
    }

    size_t size() const
    {
        return que.size();
    }

};

void fileReaderProducer(ifstream& file, SimpleQueStruct<vector<string>>& dq, size_t blockSize) {
    string line;
    vector<string> lines;
    while(file >> line)
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

    dq.one_thread_done();
}

void countWordsConsumer(SimpleQueStruct<vector<string>>&dq,
                       SimpleQueStruct<map_type> &dq1) {
    vector<string> v;
    while(dq.dequeue(v))
    {
        map_type localMap;
        for (auto& word: v) {
                cleanWord(word);
                ++localMap[word];
        }
        dq1.enque(move(localMap)); // No need to clear -- recreated on next iteration. But be carefull!
    }
    dq1.one_thread_done();
   }

void mergeMapsConsumer(SimpleQueStruct<map_type>& dq1, map_type& wordsMap) {
    map_type v1;
    while(dq1.dequeue(v1))
    {
        for (auto& item: v1) {
            wordsMap[item.first] += item.second;
        }
    }

}


int main() {
    setlocale(LC_ALL,"C");

    auto config = read_config("data_input_conc.txt");

    string infile    = config["infile"];
    string out_by_a  = config["out_by_a"];
    string out_by_n  = config["out_by_n"];
    size_t blockSize = str_to_val<size_t>(config["blockSize"]);
    size_t threads_n = str_to_val<size_t>(config["threads"]);

    string etalon_a_file  = config["etalon_a_file"];

    //=============================================================
    auto startProducer = get_current_time_fenced();

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

    for( size_t i = 0; i < threads_n-2; ++i ){
        threads.emplace_back( countWordsConsumer, ref(readBlocksQ), ref(localDictsQ) );
    }

    // threads.emplace_back(mergeMapsConsumer, ref(localDictsQ), ref(wordsMap));
    mergeMapsConsumer(localDictsQ, wordsMap);

    for (auto& th : threads) {
        th.join();
    }

    //=============================================================
	auto finishConsumer = get_current_time_fenced();

    write_sorted_by_key(out_by_a, wordsMap);
    write_sorted_by_value(out_by_n, wordsMap);

    auto total = finishConsumer - startProducer;

    cout << "Total time: " << to_us(total) << endl;

    bool are_correct = true;
    if( !etalon_a_file.empty() )
    {
        are_correct = compareFiles(out_by_a, etalon_a_file);
    }
#ifdef _MSC_VER
	system("pause");
#endif
    return !are_correct;
}
