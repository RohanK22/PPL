// A simple farm example where each worker computes a factorial of a number
#include <iostream>
#include "../../src/FarmManager.hpp"

using namespace std;

class FactorialEmitter : public Node<void *>
{
public:
    FactorialEmitter(int num_tasks)
    {
        this->num_tasks = num_tasks;
    }

    void *run(void *_) override
    {
        if (curr == num_tasks)
        {
            std::cout << "Generator Done" << std::endl;
            return nullptr;
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return (void *)new int(curr);
    }

private:
    int num_tasks;
    int curr = 0;
};

class FactorialWorker : public Node<void *>
{
public:
    void *run(void *task) override
    {
        int num = *((int *)task);
        long long result = 1;
        for (int i = 1; i <= num; i++)
        {
            result *= i;
        }
        return (void *)new long long(result);
    }
};

class FactorialCollector : public Node<void *>
{
public:
    void *run(void *task) override
    {
        std::cout << "Collector received task " << receive_count << std::endl;
        std::cout << "Factorial is " << *((long long *)task) << std::endl;
        receive_count++;
        return task;
    }

private:
    int receive_count = 0;
};

int main()
{
    FarmManager<void *> *farm = new FarmManager<void *>();
    int num_tasks = 5;
    int num_workers = 5;

    FactorialEmitter *emitter = new FactorialEmitter(num_tasks);
    FactorialCollector *collector = new FactorialCollector();

    farm->add_emitter(emitter);
    farm->add_collector(collector);
    for (int i = 0; i < num_workers; i++)
    {
        FactorialWorker *worker = new FactorialWorker();
        farm->add_worker(worker);
    }

    farm->run_until_finish();
    delete farm;
    return 0;
}
