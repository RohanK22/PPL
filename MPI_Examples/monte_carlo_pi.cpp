// Program computes PI using Monte Carlo method
// It uses parallelism at two levels
// An MPI farm is used to distribute the work to Node level farms

// rk76@pc7-003-l:.../rk76/Documents/PPL/build $ time ./seq_monte_carlo 1000000000
// Estimated Pi (Sequential): 3.14155

// real    0m29.076s
// user    0m29.038s
// sys     0m0.003s

//  time ./farm_monte_carlo 100000000 1
// mpirun -np 4 ./nested_farm_in_mpi_farm_monte_carlo 1000000 1 6

//./farm_monte_carlo 100000000 1  8.52s user 0.04s system 193% cpu 4.432 total
// mpirun -np 4 ./nested_farm_in_mpi_farm_monte_carlo 100000000 1 8  6.16s user 0.17s system 381% cpu 1.660 total

// Estimated Pi: 3.14161
// ./farm_monte_carlo 1000000000 1  81.70s user 0.16s system 287% cpu 28.502 total

// mpirun -np 6 ./nested_farm_in_mpi_farm_monte_carlo 1000000000 3 6  45.46s user 0.44s system 700% cpu 6.553 total

// mpirun -np 6 /cs/home/rk76/Documents/PPL/build/nested_farm_in_mpi_farm_monte_carlo 100000 3 6

// mpirun -np 4 --hostfile /cs/home/rk76/Documents/OMPI/hostfile /cs/home/rk76/Documents/PPL/build/nested_farm_in_mpi_farm_monte_carlo 100000 1 8

// mpirun -n 4 --host pc7-003-l:1,pc7-005-l:1,pc7-007-l:1,pc7-009-l:1 /cs/home/rk76/Documents/PPL/build/nested_farm_in_mpi_farm_monte_carlo 100000 1 8

// rk76@pc7-003-l:.../rk76/Documents/PPL/build $ time ./seq_monte_carlo 10000000000
// Estimated Pi (Sequential): 3.14163

// real    4m50.520s
// user    4m50.178s
// sys     0m0.004s

// time mpirun -n 6 --host pc7-003-l:2,pc7-005-l:1,pc7-007-l:1,pc7-009-l:1,pc7-011-l:1 /cs/home/rk76/Documents/PPL/build/nested_farm_in_mpi_farm_monte_carlo 10000000000 3 6

#include <iostream>
#include <cmath>
#include "../FarmManager.hpp"
#include "../MPIFarmManager.hpp"
#include <boost/mpi.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>

#define ull unsigned long long
#define ld long double

using namespace std;
namespace mpi = boost::mpi;

class MonteCarloPiTask
{
public:
    MonteCarloPiTask() = default;

    MonteCarloPiTask(ull num_samples) : num_samples(num_samples), inside_circle(0) {}

    void perform_simulation()
    {
        for (ull i = 0; i < num_samples; ++i)
        {
            double x = static_cast<double>(rand()) / RAND_MAX;
            double y = static_cast<double>(rand()) / RAND_MAX;
            if (std::sqrt(x * x + y * y) <= 1.0)
            {
                inside_circle++;
            }
        }
    }

    ull get_inside_circle_count() const
    {
        return inside_circle;
    }

    // Boost serialization
    template <class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & num_samples;
        ar & inside_circle;
    }

private:
    ull num_samples;
    ull inside_circle;
};

string serialise_montercarlo_pi_task(MonteCarloPiTask task)
{
    stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << task;
    return ss.str();
}

MonteCarloPiTask deserialise_monte_carlo_pi_task(string task_str)
{
    stringstream ss(task_str);
    boost::archive::text_iarchive ia(ss);
    MonteCarloPiTask task;
    ia >> task;
    return task;
}

class Emitter : public Node<string>
{
public:
    Emitter(ull num_samples, ull num_workers) : num_samples(num_samples), num_workers(num_workers)
    {
        ull samples_per_worker = num_samples / num_workers;
        ull residual = num_samples % num_workers;

        for (ull i = 0; i < num_workers; ++i)
        {
            ull worker_samples = samples_per_worker;
            MonteCarloPiTask task = MonteCarloPiTask(worker_samples);
            tasks.push_back(task);
        }
        if (residual > 0)
        {
            MonteCarloPiTask task = MonteCarloPiTask(residual);
            tasks.push_back(task);
        }
        num_tasks = tasks.size();
    }

    string run(string) override
    {
        if (curr == num_tasks)
        {
            std::cout << "Generator Done" << std::endl;
            return string("EOS");
        }
        string task = serialise_montercarlo_pi_task(tasks[curr]);
        curr++;
        return task;
    }

private:
    ull num_samples;
    ull num_workers;

    ull num_tasks;
    ull curr = 0;
    vector<MonteCarloPiTask> tasks;
};

class Worker : public Node<string>
{
public:
    string run(string task_str) override
    {
        MonteCarloPiTask task = deserialise_monte_carlo_pi_task(task_str);
        task.perform_simulation();
        return string(serialise_montercarlo_pi_task(task));
    }
};

class Collector : public Node<string>
{
public:
    Collector(ull num_samples, ull num_workers) : num_samples(num_samples), num_workers(num_workers)
    {
        this->total_inside_circle = 0;
        this->curr = 0;
        this->total_tasks = num_workers + (num_samples % num_workers != 0);
    }

    string run(string task_str) override
    {
        MonteCarloPiTask task = deserialise_monte_carlo_pi_task(task_str);
        total_inside_circle += task.get_inside_circle_count();
        curr++;

        if (curr == total_tasks)
        {
            cout << "Total inside circle: " << total_inside_circle << '\n';
            cout << "Total samples: " << num_samples << '\n';
            ld estimated_pi = (4.0 * total_inside_circle) / num_samples;
            std::cout << "Estimated Pi: " << std::setprecision(10) << estimated_pi << std::endl;
            return string("EOS");
        }

        return string("CONTINUE");
    }

private:
    ull num_samples;
    ull num_workers;
    ull total_inside_circle;
    ull total_tasks;
    ull curr;
};

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        std::cout << "Usage: " << argv[0] << " <num_samples> <num_mpi_farm_workers> <num_node_farm_workers>" << std::endl;
        return 1;
    }

    // Initialise MPI environment
    mpi::environment env;
    mpi::communicator world;

    ull num_samples = std::stoull(argv[1]);
    ull num_mpi_farm_workers = std::stoull(argv[2]);
    ull num_node_farm_workers = std::stoull(argv[3]);

    // We are going to have these many farm threads as workers in total across all processes
    ull num_workers = num_mpi_farm_workers * num_node_farm_workers;

    // Create the MPI farm manager
    MPIFarmManager *mpi_farm_manager = new MPIFarmManager(&env, &world);

    mpi_farm_manager->add_emitter(new Emitter(num_samples, num_workers));
    mpi_farm_manager->add_collector(new Collector(num_samples, num_workers));

    for (ull i = 0; i < num_mpi_farm_workers; i++)
    {
        // cout << "Adding node farm manager " << i << endl;
        FarmManager<string> *node_farm_manager = new FarmManager<string>();
        for (ull j = 0; j < num_node_farm_workers; j++)
        {
            node_farm_manager->add_worker(new Worker());
        }
        mpi_farm_manager->add_worker(node_farm_manager);
    }

    mpi_farm_manager->run_until_finish();

    return 0;
}