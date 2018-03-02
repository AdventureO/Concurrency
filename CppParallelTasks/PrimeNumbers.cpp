#include <iostream>
#include <mutex>
#include <thread>

using namespace std;
mutex mtx;

void findPrimeNumbers(int start, int end, int &primeNumbers) {
    int num,i,count;
    int localNumber = 0;
    for(num = start;num<=end;num++) {
        count = 0;
        for (i = 2; i <= num / 2; i++) {
            if (num % i == 0) {
                count++;
                break;
            }
        }

        if (count == 0 && num != 1) {
            localNumber++;
        }
    }

    lock_guard<mutex> ll(mtx);
    primeNumbers += localNumber;

}

int main(){

    int primeNumbers = 0;
    thread th1 = thread(findPrimeNumbers, 1, 3, ref(primeNumbers));
    thread th2 = thread(findPrimeNumbers, 4, 10, ref(primeNumbers));
    th1.join();
    th2.join();

    cout << "Number of prime munbers: " << primeNumbers;
    return 0;
}
