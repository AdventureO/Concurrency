#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <thread>
#include <set>
#include <algorithm>
#include <iterator>
#include <mutex>

using namespace std;
mutex mtx;

// Counts number of each letter
void CountWords(const vector<string>& wordsVector, int start, int end, map<char, int>& wordsCount) {

    map<char, int> local_map;
    for (int i = start; i < end; i++) {
        for (int letter = 0; letter < wordsVector[i].size(); letter++) {
            char a = wordsVector[i][letter];
            ++local_map[a];
        }
    }

    for(map<char, int>::iterator i = local_map.begin(); i != local_map.end(); i++){
        lock_guard<mutex> lg(mtx);
        wordsCount[i -> first]  += i-> second;
    }

}


// Divide vector into n equally parts, n - number of threads
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


int main() {

    map<char, int> wordsCount;
    string infile = "text.txt";
    int threads = 2;


    // Read file with text
    vector<string> vectorWords;
    ifstream file;
    file.open(infile);

    if (!file.is_open()){
        cerr << "Error opening file";
    }

    string word;
    while (file >> word) {
        for (size_t i = 0, len = word.size(); i < len; i++)
        {
            if (ispunct(word[i]))
            {
                word.erase( i--, 1);
                len = word.size();
            }
        }
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        vectorWords.push_back(word);
    }
    file.close();


    vector<int> list_of_words_amount = SplitVector(vectorWords, threads);
    thread t[threads];

    for (int i = 0; i < list_of_words_amount.size() - 1; ++i) {
        t[i] = thread(CountWords, cref(vectorWords), list_of_words_amount[i], list_of_words_amount[i + 1],
                      ref(wordsCount));
    }


    for (int i = 0; i < threads; ++i) {
        t[i].join();
    }

    for(map<char, int> :: iterator i = wordsCount.begin(); i != wordsCount.end(); i++){
        cout << i -> first << "   " <<  i-> second << endl;
    }

}






