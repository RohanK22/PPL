// Program computes PI using Monte Carlo method
// It uses parallelism at two levels
// An MPI farm is used to distribute the work to Node level farms

//  time ./farm_monte_carlo 100000000 1
// mpirun -np 4 ./nested_farm_in_mpi_farm_monte_carlo 1000000 1 6

//./farm_monte_carlo 100000000 1  8.52s user 0.04s system 193% cpu 4.432 total
// mpirun -np 4 ./nested_farm_in_mpi_farm_monte_carlo 100000000 1 8  6.16s user 0.17s system 381% cpu 1.660 total

#include <iostream>
#include <cmath>
#include "../FarmManager.hpp"
#include "../MPIFarmManager.hpp"
#include <boost/mpi.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>


#define ll long long

using namespace std;
namespace mpi = boost::mpi;

class MonteCarloPiTask {
public:
    MonteCarloPiTask() = default;

    MonteCarloPiTask(ll num_samples) : num_samples(num_samples), inside_circle(0) {}

    void perform_simulation() {
        for (ll i = 0; i < num_samples; ++i) {
            double x = static_cast<double>(rand()) / RAND_MAX;
            double y = static_cast<double>(rand()) / RAND_MAX;

            if (std::sqrt(x * x + y * y) <= 1.0) {
                inside_circle++;
            }
        }
    }

    ll get_inside_circle_count() const {
        return inside_circle;
    }

    // Boost serialization
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar & num_samples;
        ar & inside_circle;
    }

private:
    ll num_samples;
    ll inside_circle;
};

string serialise_montercarlo_pi_task(MonteCarloPiTask task) {
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << task;
    return ss.str();
}

MonteCarloPiTask deserialise_monte_carlo_pi_task(string task_str) {
    stringstream ss(task_str);
    boost::archive::text_iarchive ia(ss);
    MonteCarloPiTask task;
    ia >> task;
    return task;
}

class Emitter : public Node {
public:
    Emitter(ll num_samples, ll num_workers) : num_samples(num_samples), num_workers(num_workers) {
        ll samples_per_worker = num_samples / num_workers;
        ll residual = num_samples % num_workers;

        for (ll i = 0; i < num_workers; ++i) {
            ll worker_samples = samples_per_worker;
            MonteCarloPiTask task = MonteCarloPiTask(worker_samples);
            tasks.push_back(task);
        }
        if (residual > 0) {
            MonteCarloPiTask task = MonteCarloPiTask(residual);
            tasks.push_back(task);
        }
    }

    string run(string) override {
        if (curr == num_workers) {
            std::cout << "Generator Done" << std::endl;
            return string("EOS");
        }
        string task = serialise_montercarlo_pi_task(tasks[curr]);
        curr++;
        return task;
    }

private:
    ll num_samples;
    ll num_workers;

    int num_tasks;
    int curr = 0;
    vector<MonteCarloPiTask> tasks;
};

class Worker : public Node {
public:
    string run(string task_str) override {
        MonteCarloPiTask task = deserialise_monte_carlo_pi_task(task_str);
        task.perform_simulation();
        return string(serialise_montercarlo_pi_task(task));
    }
};

class Collector : public Node {
public:
    Collector(ll num_samples, ll num_workers) : num_samples(num_samples), num_workers(num_workers) {
        this->total_inside_circle = 0;
        this->curr = 0;
        this->total_tasks = num_workers + (num_samples % num_workers != 0);
    }

    string run(string task_str) override {
        MonteCarloPiTask task = deserialise_monte_carlo_pi_task(task_str);
        total_inside_circle += task.get_inside_circle_count();
        curr++;

        if (curr == total_tasks) {
            double estimated_pi = 4.0 * total_inside_circle / num_samples;
            std::cout << "Estimated Pi: " << estimated_pi << std::endl;
            return string("EOS");
        }

        return string("CONTINUE");
    }

private:
    ll num_samples;
    ll num_workers;
    ll total_inside_circle;
    ll total_tasks;
    ll curr;
};

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <num_samples> <num_mpi_farm_workers> <num_node_farm_workers>" << std::endl;
        return 1;
    }

    // Initialise MPI environment
    mpi::environment env;
    mpi::communicator world;

    ll num_samples = std::stoll(argv[1]);
    ll num_mpi_farm_workers = std::stoll(argv[2]);
    ll num_node_farm_workers = std::stoll(argv[3]);

    // We are going to have these many farm threads as workers in total across all processes
    ll num_workers = num_mpi_farm_workers * num_node_farm_workers;

    // Create the MPI farm manager
    MPIFarmManager *mpi_farm_manager = new MPIFarmManager(&env, &world);

    mpi_farm_manager->add_emitter(new Emitter(num_samples, num_workers));
    mpi_farm_manager->add_collector(new Collector(num_samples, num_workers));

    for (ll i = 0; i < num_mpi_farm_workers; i++) {
        cout << "Adding node farm manager " << i << endl;
        FarmManager *node_farm_manager = new FarmManager(true);
        for (ll j = 0; j < num_node_farm_workers; j++) {
            node_farm_manager->add_worker(new Worker());
        }
        mpi_farm_manager->add_worker(node_farm_manager);
    }

    mpi_farm_manager->run_until_finish();

    return 0;
}