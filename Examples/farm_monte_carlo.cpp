// Monte Carlo Pi calculation using Farm pattern

#include <iostream>
#include <cmath>
#include "../FarmManager.hpp"

#define ll long long

class MonteCarloPiTask {
public:
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

private:
    ll num_samples;
    ll inside_circle;
};

class Emitter : public Node {
public:
    Emitter(ll num_samples, ll num_workers) : num_samples(num_samples), num_workers(num_workers) {}

    void *run(void *_) override {
        ll samples_per_worker = num_samples / num_workers;
        ll residual = num_samples % num_workers;

        for (ll i = 0; i < num_workers; ++i) {
            ll worker_samples = samples_per_worker;
            MonteCarloPiTask *task = new MonteCarloPiTask(worker_samples);
            this->get_output_queue()->push(static_cast<void*>(task));
        }
        if (residual > 0) {
            MonteCarloPiTask *task = new MonteCarloPiTask(residual);
            this->get_output_queue()->push(static_cast<void*>(task));
        }

        return nullptr;
    }

private:
    ll num_samples;
    ll num_workers;
};

class Worker : public Node {
public:
    void *run(void *task) override {
        MonteCarloPiTask *pi_task = static_cast<MonteCarloPiTask*>(task);
        pi_task->perform_simulation();
        return pi_task;
    }
};

class Collector : public Node {
public:
    Collector(ll num_samples, ll num_workers) : num_samples(num_samples), num_workers(num_workers) {
        this->total_inside_circle = 0;
        this->curr = 0;
    }

    void *run(void *task) override {
        MonteCarloPiTask *pi_task = static_cast<MonteCarloPiTask*>(task);
        total_inside_circle += pi_task->get_inside_circle_count();
        curr++;

        if (curr == num_workers) {
            double estimated_pi = 4.0 * total_inside_circle / num_samples;
            std::cout << "Estimated Pi: " << estimated_pi << std::endl;
        }

        return static_cast<void*>(pi_task);
    }

private:
    ll num_samples;
    ll num_workers;
    ll total_inside_circle;
    ll curr;
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <num_samples> <num_workers>" << std::endl;
        return 1;
    }

    srand(100);

    ll num_samples = std::stoll(argv[1]);
    ll num_workers = std::stoll(argv[2]);

    Emitter *emitter = new Emitter(num_samples, num_workers);
    Collector *collector = new Collector(num_samples, num_workers);

    FarmManager *farm_manager = new FarmManager();
    farm_manager->add_emitter(emitter);

    for (ll i = 0; i < num_workers; i++) {
        Worker *worker = new Worker();
        farm_manager->add_worker(worker);
    }

    farm_manager->add_collector(collector);

    farm_manager->run_until_finish();

    return 0;
}
