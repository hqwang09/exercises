
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <condition_variable>
 
std::mutex data_mutex;
std::condition_variable data_var;
bool flag = true;
int cnt = 0;
int mod = 5;
 
void printA()
{
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lck(data_mutex) ;
        data_var.wait(lck,[]{return flag;});
        std::cout<<"thread: "<< std::this_thread::get_id() << "   printf: " << "A" <<std::endl;
        flag = false;
        data_var.notify_one();
    }
}
 
void printB()
{
    while(1)
    {
        std::unique_lock<std::mutex> lck(data_mutex) ;
        data_var.wait(lck,[]{return !flag;});
        std::cout<<"thread: "<< std::this_thread::get_id() << "   printf: " << "B" <<std::endl;
        flag = true;
        data_var.notify_one();
    }
}

void print()
{
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(0.1));
        std::unique_lock<std::mutex> lck(data_mutex) ;
        data_var.wait(lck);
        std::cout<<"thread: "<< std::this_thread::get_id() << ", cnt = " << cnt <<std::endl;
        ++cnt;
        data_var.notify_one();
    }
}

void twoprint()
{
    std::thread tA(printA);
    std::thread tB(printB);
    tA.join();
    tB.join();
}

void multiprint()
{
    std::vector<std::thread> threadpool;
    for(int i = 0; i <mod; ++i)
    {
        threadpool.push_back(std::thread(print, i));
    }
    for(int i = 0; i < mod; ++i)
    {
        threadpool[i].join();
    }
}
 
int main()
{
    //twoprint();
    multiprint();
    return 0;
}