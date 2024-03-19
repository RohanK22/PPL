// A simple farm example where each worker computes a factorial of a number
#include <iostream>
#include "../src/MPIFarmManager.hpp"
#include "../src/Timer.hpp"
#include <algorithm>
#include <boost/mpi.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


using namespace std;

class FactorialEmitter: public MPINode {
public:
    FactorialEmitter(int num_tasks) {
        this->num_tasks = num_tasks;
    }

    string run(string task) override {
        if (curr == num_tasks) {
            std::cout << "Generator Done" << std::endl;
            return string("EOS");
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return std::to_string(curr);
    }

private:
    int num_tasks;
    int curr = 0;
};

class FactorialWorker: public MPINode {
public:
    string run(string task) override {
        int num = std::stoi(task);
        long long *result = new long long(1);
        for (int i = 1; i <= num; i++) {
            *result *= i;
        }
        return std::to_string(*result);
    }
};

class FactorialCollector: public MPINode {
public:
    FactorialCollector(int num_tasks) {
        this->num_tasks = num_tasks;
    }

    string run(string task) override {
        std::cout << "Collector received task " << receive_count << std::endl;
        long long num = std::stoll(task);
        std::cout << "Factorial is " << num << std::endl;
        receive_count++;
        if (receive_count == num_tasks) {
            cout << "Collector Sending EOS" << endl;
            return "EOS";
        }
        return string("CONTINUE");
    }

private:
    int receive_count = 0;
    int num_tasks;
};

int main() {
    mpi::environment env;
    mpi::communicator world;

    MPIFarmManager *farm = new MPIFarmManager(&env, &world);
    int num_tasks = 20;
    int num_workers = 5; // Number of free cores on machine - 3 since 0 is master, 1 is emitter, 2 is collector
    FactorialEmitter *emitter = new FactorialEmitter(num_tasks);
    FactorialCollector *collector = new FactorialCollector(num_tasks);

    farm->add_emitter(emitter);
    farm->add_collector(collector);
    for (int i = 0; i < num_workers; i++) {
        FactorialWorker *worker = new FactorialWorker();
        farm->add_worker(worker);
    }

    farm->run("");
    return 0;
}