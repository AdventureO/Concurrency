#include <iostream>
#include <future>
#include <vector>

using namespace std;
int findPrimeNumbers(int start, int end) {
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
    return localNumber;
}

int main(){

    vector<future<int>> futures;
    int primeNumbers = 0;
    int threadsNumb = 6;
    int to = 30;
    int elmntsbyThread = to/threadsNumb;
    int x = 1;
    while(x < to){
        futures.push_back(async(findPrimeNumbers, x, x+elmntsbyThread));
        x  = x + elmntsbyThread+1;
    }

    for(auto &e : futures) {
        primeNumbers += e.get();
    }
    cout << "Number of prime numbers: " << primeNumbers;
    return 0;
}
