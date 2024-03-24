// Vector dot product example

// ./farm_dot_product <size> <num_workers>

#include <iostream>
#include <vector>
#include "../../src/FarmManager.hpp"

class VectorDotProductTask
{
public:
    VectorDotProductTask(double *v1, double *v2, int start_index, int end_index)
    {
        this->v1 = v1;
        this->v2 = v2;
        this->start_index = start_index;
        this->end_index = end_index;
        this->partial_sum = 0.0;
    }

    double compute_partial_dot_product()
    {
        for (int i = start_index; i < end_index; i++)
        {
            partial_sum += v1[i] * v2[i];
        }
    }

    double get_partial_sum() const
    {
        return partial_sum;
    }

private:
    double *v1;
    double *v2;
    int start_index;
    int end_index;
    double partial_sum;
};

class Emitter : public Node<void *>
{
public:
    Emitter(int size, int num_workers)
    {
        this->size = size;
        this->num_workers = num_workers;
    }

    void *run(void *_) override
    {
        double *v1 = new double[size];
        double *v2 = new double[size];
        for (int i = 0; i < size; i++)
        {
            v1[i] = 12.3;
            v2[i] = 13.4;
        }

        int step = size / num_workers;
        int residual = size % num_workers;
        for (int i = 0; i < num_workers; i++)
        {
            VectorDotProductTask *vdp_task = new VectorDotProductTask(v1, v2, i * step, (i + 1) * step);
            this->get_output_queue()->push((void *)vdp_task);
        }
        if (residual != 0)
        {
            VectorDotProductTask *vdp_task = new VectorDotProductTask(v1, v2, num_workers * step, size);
            this->get_output_queue()->push((void *)vdp_task);
        }
        return nullptr;
    }

private:
    int num_workers;
    int size;
};

class Worker : public Node<void *>

{
public:
    void *run(void *task) override
    {
        VectorDotProductTask *vdp_task = (VectorDotProductTask *)task;
        vdp_task->compute_partial_dot_product();
        return vdp_task;
    }
};

class Collector : public Node<void *>
{
public:
    Collector(int size, int num_workers)
    {
        this->num_tasks = num_workers + (size % num_workers != 0);
        this->total_sum = 0.0;
    }

    void *run(void *task) override
    {
        VectorDotProductTask *vdp_task = (VectorDotProductTask *)task;
        total_sum += vdp_task->get_partial_sum();
        curr++;
        if (curr == num_tasks)
        {
            std::cout << "Total sum: " << total_sum << std::endl;
        }
        return (void *)vdp_task;
    }

private:
    double total_sum;
    int num_tasks;
    int curr = 0;
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " <size> <num_workers>" << std::endl;
        return 1;
    }
    int size = std::stoi(argv[1]);
    int num_workers = std::stoi(argv[2]);

    if (num_workers > size)
    {
        std::cout << "Number of workers cannot be greater than the size of the vectors" << std::endl;
        return 1;
    }

    Emitter *emitter = new Emitter(size, num_workers);
    Collector *collector = new Collector(size, num_workers);

    FarmManager<void *> *farm_manager = new FarmManager<void *>();
    for (int i = 0; i < num_workers; i++)
    {
        Worker *worker = new Worker();
        farm_manager->add_worker(worker);
    }
    farm_manager->add_collector(collector);

    farm_manager->run_until_finish();
    return 0;
}