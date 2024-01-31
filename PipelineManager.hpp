//
// Created by Rohan Kumar on 20/01/2024.
//

#ifndef PPL_PIPELINEMANAGER_HPP
#define PPL_PIPELINEMANAGER_HPP

#include "Queue.hpp"
#include "Task.hpp"
#include "thread_args.hpp"
#include "Node.hpp"
#include <vector>

class PipelineManager : public Node {
private:
    std::vector<Node*> pipeline_nodes;

public:
    PipelineManager() = default;

    unsigned int get_num_pipeline_stages() {
        return this->num_pipeline_stages;
    }

    // Add stage to the pipeline
    void add_stage(Node *node) {
        node->set_is_pipeline_stage(true);
        pipeline_nodes.push_back(node);

        // Connect the node
        if (pipeline_nodes.size() == 1) {
            pipeline_nodes[0]->set_input_queue(this->get_input_queue());
            pipeline_nodes[0]->set_output_queue(this->get_output_queue());
        } else {
            // TODO: Warning: There is a slight memory leak here
            Queue<void*> *new_queue = new Queue<void*>();
            pipeline_nodes[pipeline_nodes.size() - 2]->set_output_queue(new_queue);
            pipeline_nodes.back()->set_input_queue(new_queue);
            pipeline_nodes.back()->set_output_queue(this->get_output_queue());
        }

        this->num_pipeline_stages = pipeline_nodes.size();
    }

    void* run(void *task) {
//        std::cout << "Node 1 input queue: " << pipeline_nodes[0]->get_input_queue() << std::endl;
//        std::cout << "Node 1 output queue: " << pipeline_nodes[0]->get_output_queue() << std::endl;
//        std::cout << "Node 2 input queue: " << pipeline_nodes[1]->get_input_queue() << std::endl;
//        std::cout << "Node 2 output queue: " << pipeline_nodes[1]->get_output_queue() << std::endl;
//        std::cout << "Node 3 input queue: " << pipeline_nodes[2]->get_input_queue() << std::endl;
//        std::cout << "Node 3 output queue: " << pipeline_nodes[2]->get_output_queue() << std::endl;

        if (this->num_pipeline_stages < 1) {
            throw std::runtime_error("Pipeline must have at least one stage");
        }

        // Start node
        for (int i = 0; i < num_pipeline_stages; i ++) {
            pipeline_nodes[i]->start_node();
        }

        // join threads
        for (int i = 0; i < num_pipeline_stages; i++) {
            pipeline_nodes[i]->join_node();
        }
        return nullptr;
    }
};

#endif //PPL_PIPELINEMANAGER_HPP
