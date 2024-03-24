#include <iostream>
#include "../../src/PipelineManager.hpp"
#include <vector>
#include <numeric>

using namespace std;

class Generator : public Node<void *>
{
public:
    Generator(int num_tasks)
    {
        this->num_tasks = num_tasks;
    }

    void *run(void *) override
    {
        if (curr == num_tasks)
        {
            return EOS;
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return (void *)new vector<int>(curr, 1);
    }

private:
    int curr = 0;
    int num_tasks;
};

class Print : public Node<void *>
{
public:
    void *run(void *task) override
    {
        vector<int> *v = (vector<int> *)task;
        std::cout << "Sum: " << std::accumulate(v->begin(), v->end(), 0) << std::endl;
        return nullptr;
    }
};

int main()
{
    PipelineManager<void *> *pipeline = new PipelineManager<void *>();
    Generator *generator = new Generator(10);
    Print *print = new Print();

    pipeline->add_stage(generator);
    pipeline->add_stage(print);

    std::cout << "Number of stages: " << pipeline->get_num_pipeline_stages() << std::endl;

    pipeline->run_until_finish();
    return 0;
}
