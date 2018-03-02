#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;
template <typename T>

class Queue
{
public:

    T pop()
    {
        unique_lock<std::mutex> mlock(mtx);
        while (queue_.empty())
        {
            cv.wait(mlock);
        }
        auto item = queue_.front();
        queue_.pop();
        return item;
    }

//    void pop(T& item)
//    {
//        std::unique_lock<std::mutex> mlock(mutex_);
//        while (queue_.empty())
//        {
//            cond_.wait(mlock);
//        }
//        item = queue_.front();
//        queue_.pop();
//    }

    void push(const T& item)
    {
        unique_lock<mutex> mlock(mtx);
        queue_.push(item);
        mlock.unlock();
        cv.notify_one();
    }

    T size(){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        unique_lock<mutex> mlock(mtx);
        auto res = queue_.size();
        mlock.unlock();
        cv.notify_one();
        return res;
    }

    void empty(){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        unique_lock<mutex> mlock(mtx);
        while(!queue_.empty())
            queue_.pop();
    }



private:
    queue<T> queue_;
    mutex mtx;
    condition_variable cv;
};

void produce(Queue<int>& q) {
    for (int i = 0; i< 2; ++i) {
        cout << "Pushing " << i << endl;

        q.push(i);
    }
}

void consume(Queue<int>& q) {
    //for (int i = 0; i< 2; ++i) {

        auto item = q.size();
        cout << "Size " << item << endl;
    //}
}


int main() {
    Queue<int> q;

    //using namespace std::placeholders;

    // producer thread
    thread th1 =  thread (produce, ref(q));

    // consumer threads
   // thread th1 = thread(q.push, cref(1));
    thread th3 = thread(consume, std::ref(q));
    //thread th4 = thread(consume, std::ref(q));
    //thread th1 =  thread (push, ref(q));
    th1.join();
    th3.join();
    //th3.join();
    //th4.join();
    return 0;
}
