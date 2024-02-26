#include <boost/mpi.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include "MPIFarmManager.hpp"
#include "MPINode.hpp"

namespace mpi = boost::mpi;

class FactorialEmitter: public MPINode  {
public:
    FactorialEmitter(int num_tasks) {
        this->num_tasks = num_tasks;
    }

    string run(string task) override {
        if (curr == num_tasks) {
            std::cout << "Generator Done" << std::endl;
            return "EOS";
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return std::to_string(curr);
    }

private:
    int num_tasks;
    int curr = 0;
};

class FactorialWorker: public MPINode  {
public:
    string run(string task) override {
        int num = std::stoi(task);
        long long result = 1;
        for (int i = 1; i <= num; i++) {
            result *= i;
        }
        receive_count++;
        return std::to_string(result);
    }

private:
    int receive_count = 0;
};

class FactorialCollector: public MPINode  {
public:
    string run(string task) override {
        long long num = std::stoll(task);
        std::cout << "Factorial is " << num << std::endl;
        receive_count++;
        return "EOS";
    }

private:
    int receive_count = 0;
};

int main()
{
    mpi::environment env;
    mpi::communicator world;

    auto *farmManager = new MPIFarmManager(&env, &world);
    auto *emitter = new FactorialEmitter(5);
    auto *collector = new FactorialCollector();

    farmManager->add_emitter(emitter);
    farmManager->add_collector(collector);

    for (int i = 0; i < world.size() - 3; i++) {
        auto *worker = new FactorialWorker();
        farmManager->add_worker(worker);
    }

    farmManager->run("");

    free (farmManager);
    return 0;
}