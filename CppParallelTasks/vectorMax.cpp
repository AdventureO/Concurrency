#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;


mutex mtx1;
vector<int> finalMaxVector;

int worker(int start, int end, int (&myints)[10]){
    int max = myints[start];

    for(int i = start; i < end; i++){

        if(myints[start] < myints[i]){
            max = myints[i];
        }
    }
    lock_guard<mutex> ll(mtx1);
    finalMaxVector.push_back(max);


    return 0;
}


int main() {
    int myints[10];
    int i = 0;

    while (i != 10) {
        myints[i] = rand();
        cout<<myints[i]<<endl;
        i++;
    }
    thread th1 = thread(worker, 0, 5, ref(myints));
    thread th2 = thread(worker, 5, 10, ref(myints));
    th1.join();
    th2.join();
    int finalMax = finalMaxVector[0];
    for(int a = 0; a < 1; a++){
        if(finalMax < finalMaxVector[a]){
            finalMax = finalMaxVector[a];
        }
    }
    cout<<"Final Max: "<<finalMax<<endl;
    return 0;
}
