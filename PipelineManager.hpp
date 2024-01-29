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

template <typename T>

class PipelineManager {
private:
    Queue<T> *emitter_queue;
    Queue<T> *collector_queue;
    std::vector<Queue<T>*> intermediate_queues;
    std::vector<Node<T>*> pipeline_nodes;
    unsigned int num_nodes; // number of steps in the pipeline

public:
    // Constructor
    PipelineManager() = default;

    // Constructor with emitter and collector queue
    PipelineManager(Queue<T> *eq, Queue<T> *cq) {
        this->emitter_queue = eq;
        this->collector_queue = cq;
    }

    bool set_emitter_queue(Queue<T> *eq){
        this->emitter_queue = eq;
    }

    bool set_collector_queue(Queue<T> *cq) {
        this->collector_queue = cq;
    }

    Queue<T> *get_emitter_queue() {
        return this->emitter_queue;
    }

    Queue<T> *get_collector_queue() {
        return this->collector_queue;
    }

    // Add stage to the pipeline
    void add_stage(Node<T> *node) {
        pipeline_nodes.push_back(node);

        // Connect the node
        if (pipeline_nodes.size() == 1) {
            pipeline_nodes[0]->set_input_queue(emitter_queue);
            pipeline_nodes[0]->set_output_queue(collector_queue);
        } else {
            // Make an intermediate queue
            Queue<T> *intermediate_queue = new Queue<T>();
            intermediate_queues.push_back(intermediate_queue);

            // Connect the node
            pipeline_nodes.back()->set_input_queue(intermediate_queues.back());
            pipeline_nodes.back()->set_output_queue(collector_queue);

            // Rewire the previous node
            pipeline_nodes[pipeline_nodes.size() - 2]->set_output_queue(intermediate_queues[intermediate_queues.size() - 1]);
        }
    }

    void run() {
        if (this->num_nodes < 1) {
            std::cout << "Error: Pipeline can not have less than one stage" << std::endl;
            return;
        }

//        // Add EOS tasks to the emitter queue for each worker thread
//        for (int i = 0; i < num_nodes; i++) {
//            T task;
//            task.set_is_eos(true);
//            emitter_queue->push(task);
//        }

        std::cout << "Number of intermediate queues: " << intermediate_queues.size() << std::endl;
        std::cout << "Number of pipeline nodes: " << pipeline_nodes.size() << std::endl;

        // Start node
        for (int i = 0; i < num_nodes; i ++) {
            pipeline_nodes[i]->start_node();
        }

        // join threads
        for (int i = 0; i < num_nodes; i++) {
            pipeline_nodes[i]->join_node();
        }
    }
};

#endif //PPL_PIPELINEMANAGER_HPP
