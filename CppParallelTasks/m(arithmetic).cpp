#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
using namespace std;
void work_for_threads(int from, int to,int b,int a0,mutex &m,int &result){
    int sum = 0;
    for (int i = from; i <=to ; ++i) {
        sum += a0 + (i -1)*b;
    }
    m.lock();
    result += sum;
    m.unlock();
}
void join(vector<thread> &threads){
    for(int i = 0; i < threads.size(); i++){
        threads[i].join();
    }
}
int threading(int n,int a0,int to,int b,mutex &m){
    int result = 0;
    vector<thread> threads;
    int thread_to = to / n;
    int from = 1;
    for (int i = 0; i < n ; ++i) {
        threads.push_back(thread(work_for_threads,from,thread_to,b,a0,ref(m),ref(result)));
        from = thread_to + 1;
        thread_to = thread_to + n;
        if(i == n-1){
            threads.push_back(thread(work_for_threads,from,to,b,a0,ref(m),ref(result)));
        }
    }
    join(threads);
    return result;
}

int main() {
    mutex m;
    int a = threading(3,12,100,2,ref(m));
    cout << a << endl;
    return 0;
}
