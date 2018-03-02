#include <iostream>
#include <vector>
#include <thread>
#include <future>

using namespace std;

int worker(int start, int end, vector<int> &int_array){
    int max = int_array[start];
    for(int i = start + 1; i <= end; i++) {
        if(max < int_array[i]){
            max = int_array[i];
        }
    }
    return max;
}

int main() {
    srand(123);
    int arraySize;
    cout << "Please enter a size of array: ";
    cin >> arraySize;

    int numOfThreads;
    cout << "Please enter a number of threads: ";
    cin >> numOfThreads;

    vector<future<int>> result;
    vector<int> int_array;

    int i = 0;
    cout << "--------------------------" << endl;
    while (i != arraySize) {
        int_array.push_back( rand() % 100 + 1);
        cout << int_array[i] << endl;
        i++;
    }
    cout << "--------------------------" << endl;

    int start = 0;
    int sz = arraySize;
    int end = sz/numOfThreads;
    int end1 = end;


    cout << arraySize << endl;
    for (int j = 0; j < numOfThreads; j++) {
        if (j == numOfThreads - 1) {
            cout << start << "  " << arraySize << endl;
            result.push_back(async(worker, start, arraySize, ref(int_array)));
        } else if (sz - end >= end1) {
            cout << start << "  " << end << endl;
            result.push_back(async(worker, start, end, ref(int_array)));
            start = end + 1;
            end += end1;
        }
        //cout << start << "  " << end << endl;
    }

    int finalMax = result[0].get();
    for (int a = 1; a < numOfThreads; a++) {
        int tempMax = result[a].get();
        if(finalMax < tempMax) {
            finalMax = tempMax;
        }
    }

    cout << "--------------------------" << endl;
    cout << "Final Max: " << finalMax << endl;
}