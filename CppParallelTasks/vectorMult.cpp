#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

int sum = 1;
mutex mtx;

void worker(int start, int end, int (&lst)[2]){
    for(int i = start; i < end; i++){
        lock_guard<mutex> ll(mtx);
        sum *= lst[i];
    }
}


int main() {
    int myints1[2], myints2[2];
    int i = 0;
    int a = 0;
    while (i != 2) {
        int rv = rand();
        myints1[i] = rv;
        cout<<"1: "<<myints1[i]<<endl;
        i++;
    }
    while (a != 2) {
        int rv1 = rand();
        myints2[a] = rv1;
        cout<<"2: "<<myints2[i]<<endl;
        a++;
    }

    thread th1 = thread(worker, 0, 1, ref(myints1));
    thread th2 = thread(worker, 1, 2, ref(myints1));
    thread th3 = thread(worker, 0, 1, ref(myints2));
    thread th4 = thread(worker, 1, 2, ref(myints2));
    th1.join();
    th2.join();
    th3.join();
    th4.join();

    cout<<sum<<endl;

    return 0;
}
