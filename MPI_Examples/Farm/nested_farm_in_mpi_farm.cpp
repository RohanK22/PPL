// Multi-level farm example for computing factorial of a number
// Uses two node-level farms as workers in an MPI farm

// mpiexec -np 5 ./cmake-build-debug/nested_farm_in_mpi_farm

#include <iostream>
#include "../../src/MPIFarmManager.hpp"
#include "../../src/FarmManager.hpp"
#include <algorithm>
#include <boost/mpi.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;

class FactorialEmitter : public Node<string>
{
public:
    FactorialEmitter(int num_tasks)
    {
        this->num_tasks = num_tasks;
    }

    string run(string _) override
    {
        if (curr == num_tasks)
        {
            std::cout << "Generator Done" << std::endl;
            return EOS;
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return std::to_string(curr);
    }

private:
    int num_tasks;
    int curr = 0;
};

class FactorialWorker : public Node<string>
{
public:
    string run(string task) override
    {
        int num = std::stoi(task);
        long long *result = new long long(1);
        for (int i = 1; i <= num; i++)
        {
            *result *= i;
        }
        return std::to_string(*result);
    }
};

class FactorialCollector : public Node<string>
{
public:
    FactorialCollector(int num_tasks)
    {
        this->num_tasks = num_tasks;
    }

    string run(string task) override
    {
        long long num = std::stoll(task);
        std::cout << "Factorial is " << num << std::endl;
        receive_count++;
        if (receive_count == num_tasks)
        {
            cout << "Collector Sending EOS" << endl;
            return EOS;
        }
        return string("CONTINUE");
    }

private:
    int receive_count = 0;
    int num_tasks;
};

int main()
{
    mpi::environment env;
    mpi::communicator world;

    MPIFarmManager *mpi_farm = new MPIFarmManager(&env, &world);
    int num_tasks = 5;
    int num_workers = 2;

    FactorialEmitter *emitter = new FactorialEmitter(num_tasks);
    FactorialCollector *collector = new FactorialCollector(num_tasks);
    mpi_farm->add_emitter(emitter);
    mpi_farm->add_collector(collector);

    // Node farm 1
    FarmManager<string> *node_farm = new FarmManager<string>();
    for (int i = 0; i < num_workers; i++)
    {
        FactorialWorker *worker = new FactorialWorker();
        node_farm->add_worker(worker);
    }
    mpi_farm->add_worker(node_farm);

    // Add node farm 2
    FarmManager<string> *node_farm_2 = new FarmManager<string>();
    for (int i = 0; i < num_workers; i++)
    {
        FactorialWorker *worker = new FactorialWorker();
        node_farm_2->add_worker(worker);
    }
    mpi_farm->add_worker(node_farm_2);

    mpi_farm->run_until_finish();
    return 0;
}