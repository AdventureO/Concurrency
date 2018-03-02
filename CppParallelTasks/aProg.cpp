#include <iostream>
#include <mutex>
#include <thread>


using namespace std;

mutex mtx;



void worker(int start, int end, int d, int &sum){
    for(int i = start; i < end; i++){
        int temp = d+i;
        lock_guard<mutex> lg(mtx);
        sum += temp;
    }


}

int main() {
    int a_0 = 0;
    int sum = a_0;
    thread th1 = thread(worker, 0, 3, 1, ref(sum));
    thread th2 = thread(worker, 3, 6, 1, ref(sum));
    th1.join();
    th2.join();
    cout << "The sum is: "<<sum << endl;
}
