#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <deque>
#include <functional>
#include <iostream>
#include <sstream>

class ThreadPool;

class Worker{
public:
    Worker(ThreadPool &s) : pool(s){}
    void operator()();
private:
    ThreadPool& pool;
};

class ThreadPool{
public:
    ThreadPool(size_t);
    template<class F> void enqueue(F f);
    ~ThreadPool();
private:
    friend class Worker;
    std::vector<std::thread> workers;
    std::deque<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

void Worker::operator()()
{
    std::function<void()> task;
    while(true)
    {
        {
            std::unique_lock<std::mutex> lock(pool.queue_mutex);
            while(!pool.stop && pool.tasks.empty())
            {
                pool.condition.wait(lock);
            }
            if(pool.stop)return;
            task = pool.tasks.front();
            pool.tasks.pop_front();
        }
        task();
    }
}

ThreadPool::ThreadPool(size_t threads)
{
    for(size_t i = 0; i < threads; ++i)
    {
        workers.push_back(std::thread(Worker(*this)));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    std::cout<<"Desctruct Threadpool, notified all, stop = " << stop << std::endl;
    for(size_t i = 0; i < workers.size(); ++i)
    {
        workers[i].join();
    }
}

template<class F> void ThreadPool::enqueue(F f)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.push_back(std::function<void()>(f));
    }
    condition.notify_one();
    std::cout<<"enqueue one task, notify_one"<<std::endl;
}

void job()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::ostringstream ost;
    ost<<std::this_thread::get_id() 
             <<", sleep for " << 0.5 << " second.\n";
    std::cout << ost.str() << std::endl;
}

int main()
{
    ThreadPool pool(5);
    for(int i = 0; i < 10; ++i)
    {
        pool.enqueue(job);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(6000));
    return 0;
}