#include <iostream>
//#include "../FarmManager.hpp"
#include "../src/PipelineManager.hpp"
#include <vector>
#include <numeric>

using namespace std;

class Generator: public Node  {
public:
    Generator(int num_tasks) {
        this->num_tasks = num_tasks;

        // Set as emitter
        this->set_is_pipeline_emitter(true);
    }

    void* run(void*) override {
        if (curr == num_tasks) {
            std::cout << "Generator Done" << std::endl;
            return nullptr;
        }
        std::cout << "Generated task " << curr << std::endl;
        curr++;
        return (void*) new vector<int>(curr, 1);
    }

private:
    int curr = 0;
    int num_tasks;
};

class Print: public Node  {
public:
    void* run(void* task) override {
        vector<int> *v = (vector<int> *) task;
        std::cout << "Stage 2 received task " << receive_count << std::endl;
        std::cout << "Sum: " << std::accumulate(v->begin(), v->end(), 0) << std::endl;
        receive_count++;
        return nullptr;
    }

    int receive_count = 0;
};

int main() {
    PipelineManager *pipeline = new PipelineManager();
    Generator *generator = new Generator(10);
    Print *print = new Print();

    pipeline->add_stage(generator);
    pipeline->add_stage(print);

    std::cout << "Number of stages: " << pipeline->get_num_pipeline_stages() << std::endl;

    pipeline->run_until_finish();
    return 0;
}
