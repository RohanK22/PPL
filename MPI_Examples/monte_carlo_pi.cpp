// Program computes PI using Monte Carlo method
// It uses parallelism at two levels
// An MPI farm is used to distribute the work to Node level farms

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
    Emitter(ll num_samples, ll num_workers) : num_samples(num_samples), num_workers(num_workers) {}

    string run(string) override {
        ll samples_per_worker = num_samples / num_workers;
        ll residual = num_samples % num_workers;

        for (ll i = 0; i < num_workers; ++i) {
            ll worker_samples = samples_per_worker;
            MonteCarloPiTask task = MonteCarloPiTask(worker_samples);
            this->get_output_queue_string()->push(serialise_montercarlo_pi_task(task));
        }
        if (residual > 0) {
            MonteCarloPiTask task = MonteCarloPiTask(residual);
            this->get_output_queue_string()->push(serialise_montercarlo_pi_task(task));
        }

        return "EOS";
    }

private:
    ll num_samples;
    ll num_workers;
};

class Worker : public Node {
public:
    string run(string task_str) override {
        MonteCarloPiTask task = deserialise_monte_carlo_pi_task(task_str);
        task.perform_simulation();
        return serialise_montercarlo_pi_task(task);
    }
};

class Collector : public Node {
public:
    Collector(ll num_samples, ll num_workers) : num_samples(num_samples), num_workers(num_workers) {
        this->total_inside_circle = 0;
        this->curr = 0;
    }

    string run(string task_str) override {
        MonteCarloPiTask task = deserialise_monte_carlo_pi_task(task_str);
        total_inside_circle += task.get_inside_circle_count();
        curr++;

        if (curr == num_workers) {
            double estimated_pi = 4.0 * (total_inside_circle / num_samples);
            std::cout << "Estimated Pi: " << estimated_pi << std::endl;
            return "EOS";
        }

        return "CONTINUE";
    }

private:
    ll num_samples;
    ll num_workers;
    ll total_inside_circle;
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

    ll num_samples = std::stoi(argv[1]);
    ll num_mpi_farm_workers = std::stoi(argv[2]);
    ll num_node_farm_workers = std::stoi(argv[3]);

    // We are going to have these many farm threads as workers in total across all processes
    ll num_workers = num_mpi_farm_workers * num_node_farm_workers;

    // Create the MPI farm manager
    MPIFarmManager *mpi_farm_manager = new MPIFarmManager(&env, &world);

    mpi_farm_manager->add_emitter(new Emitter(num_samples, num_workers));
    for (ll i = 0; i < num_mpi_farm_workers; i++) {
        FarmManager * node_farm_manager = new FarmManager();
        for (ll j = 0; j < num_node_farm_workers; j++) {
            node_farm_manager->add_worker(new Worker());
        }
        mpi_farm_manager->add_worker(node_farm_manager);
    }
    mpi_farm_manager->add_collector(new Collector(num_samples, num_workers));

    mpi_farm_manager->run_until_finish();

    return 0;
}