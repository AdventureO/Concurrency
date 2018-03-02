#include <iostream>
#include <mutex>
#include <thread>
#include <math.h>

using namespace std;

mutex mtx;

void calSumGP(double a_0, int start, int end, double q, double &sum) {
    double temp = a_0;
    double loc_sum = 0;

    for (int i = start; i <= end; i++) {
        lock_guard<mutex> lg(mtx);
        cout << "loc" << loc_sum << endl;
        loc_sum = loc_sum + temp;
        //cout << temp << endl;
        temp = temp * q;
    }
    //cout << "---------" << endl;
    cout << loc_sum << endl;
    lock_guard<mutex> lg(mtx);
    sum += loc_sum;

}

int main() {

 
    double sum;
    thread th1 = thread(calSumGP, 1, 1, 3, 1, ref(sum));
    thread th2 = thread(calSumGP, 1, 4, 6, 1, ref(sum));
    th1.join();
    th2.join();
    std::cout << "Sum of geometric progression : " << sum << std::endl;

}
