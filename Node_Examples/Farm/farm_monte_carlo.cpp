// Monte Carlo Pi calculation using Farm pattern

#include <iostream>
#include <cmath>
#include "../../src/FarmManager.hpp"
#include <iomanip>
#include <cstdlib>

#define ull unsigned long long
#define ld long double

class MonteCarloPiTask
{
public:
    MonteCarloPiTask(ull num_samples) : num_samples(num_samples), inside_circle(0) {}

    void perform_simulation()
    {
        struct drand48_data rand_data;
        srand48_r(rand(), &rand_data); // Seed the random number generator

        for (ull i = 0; i < num_samples; ++i)
        {
            // Use drand48_r() instead
            double x, y;
            drand48_r(&rand_data, &x);
            drand48_r(&rand_data, &y);

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

private:
    ull num_samples;
    ull inside_circle;
};

class Emitter : public Node<void *>
{
public:
    Emitter(ull num_samples, ull num_workers) : num_samples(num_samples), num_workers(num_workers)
    {
        ull samples_per_worker = num_samples / num_workers;
        ull residual = num_samples % num_workers;

        for (ull i = 0; i < num_workers; ++i)
        {
            ull worker_samples = samples_per_worker;
            MonteCarloPiTask *task = new MonteCarloPiTask(worker_samples);
            tasks.push_back(task);
        }
        if (residual > 0)
        {
            MonteCarloPiTask *task = new MonteCarloPiTask(residual);
            tasks.push_back(task);
        }
        num_tasks = tasks.size();
    }

    void *run(void *_) override
    {
        if (curr < num_tasks)
        {
            return (void *)tasks[curr++];
        }
        return EOS;
    }

private:
    ull num_samples;
    ull num_workers;

    ull curr = 0;
    ull num_tasks;
    vector<MonteCarloPiTask *> tasks;
};

class Worker : public Node<void *>
{
public:
    void *run(void *task) override
    {
        MonteCarloPiTask *pi_task = static_cast<MonteCarloPiTask *>(task);
        pi_task->perform_simulation();
        return pi_task;
    }
};

class Collector : public Node<void *>
{
public:
    Collector(ull num_samples, ull num_workers) : num_samples(num_samples), num_workers(num_workers)
    {
        this->total_inside_circle = 0;
        this->curr = 0;
        this->total_tasks = num_workers + (num_samples % num_workers != 0);
    }

    void *run(void *task) override
    {
        MonteCarloPiTask *pi_task = static_cast<MonteCarloPiTask *>(task);
        total_inside_circle += pi_task->get_inside_circle_count();
        curr++;

        if (curr == total_tasks)
        {
            ld estimated_pi = (4.0 * total_inside_circle) / num_samples;
            std::cout << "Estimated Pi (Farm): " << std::setprecision(10) << estimated_pi << std::endl;
            return EOS;
        }

        return static_cast<void *>(pi_task);
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
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <num_samples> <num_workers>" << std::endl;
        return 1;
    }

    ull num_samples = std::stoull(argv[1]);
    ull num_workers = std::stoull(argv[2]);

    Emitter *emitter = new Emitter(num_samples, num_workers);
    Collector *collector = new Collector(num_samples, num_workers);

    FarmManager<void *> *farm_manager = new FarmManager<void *>();
    farm_manager->add_emitter(emitter);

    for (ull i = 0; i < num_workers; i++)
    {
        Worker *worker = new Worker();
        farm_manager->add_worker(worker);
    }

    farm_manager->add_collector(collector);

    farm_manager->run_until_finish();

    return 0;
}
