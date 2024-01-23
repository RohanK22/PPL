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
    std::vector<Node<T>> pipeline_nodes;
    unsigned int num_worker_threads; // number of steps in the pipeline

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

    // Set worker threads
    void set_num_worker_threads(unsigned int num_worker_threads) {
        this->num_worker_threads = num_worker_threads;
    }

    void run() {
        if (this->num_worker_threads < 1) {
            std::cout << "Error: Pipeline can not have less than one stage" << std::endl;
        } else {
            // Generate intermediate queues
            for (int i = 1; i < this->num_worker_threads; i ++) {
                intermediate_queues.push_back(new Queue<T>());
            }
        }

        // Add EOS tasks to the emitter queue for each worker thread
        for (int i = 0; i < num_worker_threads; i++) {
            T task;
            task.set_is_eos(true);
            emitter_queue->push(task);
        }

        // create nodes
        for (int i = 0; i < num_worker_threads; i++) {
            // Create a node
            Node<T> node = Node<T>();
            node.set_is_pipeline(true);
            pipeline_nodes.push_back(node);
        }

        std::cout << "Number of intermediate queues: " << intermediate_queues.size() << std::endl;
        std::cout << "Number of pipeline nodes: " << pipeline_nodes.size() << std::endl;

        // Connect with nodes with intermediate queues
        pipeline_nodes[0].set_input_queue(emitter_queue);
        pipeline_nodes[0].set_output_queue(intermediate_queues[0]);

        for (int i = 1; i < num_worker_threads - 1; i ++) {
            pipeline_nodes[i].set_input_queue(intermediate_queues[i]);
            pipeline_nodes[i].set_output_queue(intermediate_queues[i + 1]);
        }

        pipeline_nodes[num_worker_threads - 1].set_input_queue(intermediate_queues[intermediate_queues.size() - 1]);
        pipeline_nodes[num_worker_threads - 1].set_output_queue(collector_queue);

        // Start node
        for (int i = 0; i < num_worker_threads; i ++) {
            pipeline_nodes[i].start_node();
        }

        // join threads
        for (int i = 0; i < num_worker_threads; i++) {
            pipeline_nodes[i].join_node();
        }
    }
};

#endif //PPL_PIPELINEMANAGER_HPP
