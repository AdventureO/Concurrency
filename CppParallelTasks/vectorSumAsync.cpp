#include <iostream>
#include <future>
#include <time.h>
#include <vector>

int f(int* arr1, int* arr2, int numbofelements)
{

    //srand();
    int sum = 0;

    for (int i = 0; i < numbofelements; i++)
    {

        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        std::cout << arr1[i] << " " << arr2[i] << std::endl;
        sum += arr1[i] + arr2[i];
    }
    return sum;
};

int main() {
    int arr1[5], arr2[5];
    int offset = 0, main_sum = 0, numbofthreads = 2, elementsperthread = 5 / numbofthreads;
    std::vector<std::future<int>> futures;
    for(int i = 0; i < numbofthreads - 1; ++i) {

        futures.push_back(std::async(f, &arr1[offset], &arr2[offset], elementsperthread));
        offset += elementsperthread;
    }
    futures.push_back(std::async(f, &arr1[offset], &arr2[offset], elementsperthread + 5 % numbofthreads));

    for(auto &e : futures) {
        main_sum += e.get();
    }

    std::cout << main_sum << std::endl;



}
