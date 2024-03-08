// A simple farm example where each worker computes a factorial of a number

// mpiexec -np 5 ./cmake-build-debug/nested_farm_in_mpi_farm

#include <iostream>
#include "../MPIFarmManager.hpp"
//#include "../MPINode.hpp"
#include "../FarmManager.hpp"
#include "../Timer.hpp"
#include <algorithm>
#include <boost/mpi.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;

class FactorialEmitter: public Node {
public:
    FactorialEmitter(int num_tasks) {
        this->num_tasks = num_tasks;
    }

    string run(string _) override {
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

class FactorialWorker: public Node {
public:
    string run(string task) override {
        int num = std::stoi(task);
        long long *result = new long long(1);
        for (int i = 1; i <= num; i++) {
            *result *= i;
        }
        cout << "Worker sending result " << *result << endl;
        return std::to_string(*result);
    }
};

class FactorialCollector: public Node {
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
            return string("EOS");
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

    MPIFarmManager *mpi_farm = new MPIFarmManager(&env, &world);
    int num_tasks = 50;
    int num_workers = 2; // Number of free cores on machine - 3 since 0 is master, 1 is emitter, 2 is collector
    FactorialEmitter *emitter = new FactorialEmitter(num_tasks);
    FactorialCollector *collector = new FactorialCollector(num_tasks);

    mpi_farm->add_emitter(emitter);
    mpi_farm->add_collector(collector);

    // Node level farm
    FarmManager *node_farm = new FarmManager(true);
    for (int i = 0; i < num_workers; i++) {
        FactorialWorker *worker = new FactorialWorker();
        node_farm->add_worker(worker);
    }

    // The MPI farm has one worker that is a node level farm
    mpi_farm->add_worker(node_farm);

    // Add node farm 2
    FarmManager *node_farm_2 = new FarmManager(true);
    for (int i = 0; i < num_workers; i++) {
        FactorialWorker *worker = new FactorialWorker();
        node_farm_2->add_worker(worker);
    }

    // The MPI farm has one worker that is a node level farm
    mpi_farm->add_worker(node_farm_2);

    mpi_farm->run_until_finish();
    return 0;
}