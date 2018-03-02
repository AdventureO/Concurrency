#include <iostream>
#include <mutex>
#include <future>


using namespace std;

void worker(int start, int end, int d, int &sum){
    for(int i = start; i < end; i++){
        sum *= d;
    }

}

int main() {
    int a_0 = 5;
    int sum = a_0;
    auto t1 = async(launch::async, worker, 0, 3, 2, ref(sum));
    auto t2 = async(launch::async, worker, 3, 6, 2, ref(sum));
    t1.get();
    t2.get();
    cout << "The sum is: "<<sum << endl;
}
