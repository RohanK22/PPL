//
// Created by Rohan Kumar on 20/01/2024.
//

#ifndef PPL_PIPELINEMANAGER_HPP
#define PPL_PIPELINEMANAGER_HPP

#include "Queue.hpp"
#include "Node.hpp"
#include <vector>

class PipelineManager : public Node {
private:
    std::vector<Node*> pipeline_nodes;
    unsigned int num_pipeline_stages;
    pthread_t thread;
    bool is_nested_node = true;

public:
    PipelineManager() {
        this->is_pipeline = true;
    }

    unsigned int get_num_pipeline_stages() {
        return this->num_pipeline_stages;
    }

    // Add stage to the pipeline
    void add_stage(Node *node) {
        node->set_is_pipeline_stage(true);
        pipeline_nodes.push_back(node);
        this->num_pipeline_stages = pipeline_nodes.size();
    }

    // TODO: Deprecate this function
    // If the pipeline is a nested node we need to have a separate distribution_thread that receives and sends tasks to the stages
    void *thread_function(void *args) override {
        std::cout << "PipelineManager distribution_thread function " << std::endl;
        while (true) {
            void *task = nullptr;
            bool success = this->get_input_queue()->try_pop(task);
            if (!success) {
                continue;
            }
            if (task == nullptr) {
                std::cout << "EOS for Pipe node" << std::endl;
                break;
            }
            pipeline_nodes[0]->get_input_queue()->push(task);
        }
        return nullptr;
    }

    static void* thread_function_helper(void *context) {
        return static_cast<PipelineManager*>(context)->thread_function(nullptr);
    }

    void start_node() override {
        // Start node
        for (int i = 0; i < num_pipeline_stages; i ++) {
            if (i == 0) {
                pipeline_nodes[i]->set_input_queue(this->get_input_queue());
            } else {
                pipeline_nodes[i]->set_input_queue(pipeline_nodes[i - 1]->get_output_queue());
            }
            pipeline_nodes[i]->start_node();
        }
        if (is_nested_node) {
            pthread_create(&this->thread, nullptr, &PipelineManager::thread_function_helper, this);
        }
    }

    void join_node() override {
        // Join node
        for (int i = 0; i < num_pipeline_stages; i ++) {
            pipeline_nodes[i]->join_node();
        }
        if (is_nested_node) {
            pthread_join(this->thread, nullptr);
        }
    }

    // Should never be called
    void* run(void *task)  {
        throw std::runtime_error("PipelineManager run should never be called");
    }

    void run_until_finish() {
        this->is_nested_node = false;

        // Start the pipeline
        this->start_node();

        // Join the pipeline
        this->join_node();
    }
};

#endif //PPL_PIPELINEMANAGER_HPP
