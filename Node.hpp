//
// Created by Rohan Kumar on 20/01/2024.
//

// Each node is a thread of computation that reads tasks from its input queue and inserts completed tasks into
#ifndef PPL_NODE_HPP
#define PPL_NODE_HPP

#include "Queue.hpp"
#include "thread_args.hpp"

template <typename T>
class Node {
private:
    pthread_t worker_thread;
    Queue<T> *input_queue;
    Queue<T> *output_queue;

    unsigned int stage;
    bool is_farm;
    bool is_pipline;

public:
    // Constructor
    Node() = default;

    // Constructor with emitter queue and collector queue
    Node(Queue<T> *iq, Queue<T> *oq) {
        this->input_queue = iq;
        this->output_queue = oq;
    }

    bool set_input_queue(Queue<T> *iq){
        this->input_queue = iq;
    }

    bool set_output_queue(Queue<T> *oq) {
        this->output_queue = oq;
    }

    Queue<T> *get_input_queue() {
        return this->input_queue;
    }

    Queue<T> *get_output_queue() {
        return this->output_queue;
    }

    // Set worker threads
    void set_stage(unsigned int stage) {
        this->stage = stage;
    }

    void set_is_farm(bool is_farm) {
        this->is_farm = is_farm;
    }

    void set_is_pipeline(bool is_pipeline) {
        this->is_pipline = is_pipeline;
    }

    // TODO: Move to farm / pipeline manager
    static void *thread_function(void *args) {
        // Get the arguments
        Node<T> *node = (Node<T> *) args;

        if (node == nullptr) {
            std::cout << "Node is null" << std::endl;
            return nullptr;
        }

        Queue<T> *eq = node->get_input_queue();
        Queue<T> *cq = node->get_output_queue();
//        while (!eq->empty()) {
        while (true) {
//            std::cout << "Thread " << pthread_self() << " received a task " << std::endl;

            T task;
            bool success = eq->try_pop(task);

            // Check if T is an EOS task
//            if (task.get_is_eos()) {
//                std::cout << "Thread " << pthread_self() << " received an EOS task " << std::endl;
//                cq->push(task);
//                break;
//            }

            if (!success) {
                continue;
            }

            // Run the task and store results
            void *result = node->run(task);

//            cq->push(result);
//            std :: cout << "Thread " << pthread_self() << " finished a task " << std::endl;
        }
    }

    void start_node() {
        pthread_create(&this->worker_thread, nullptr, thread_function, (void *) this);
    }

    void join_node() {
        pthread_join(this->worker_thread, nullptr);
    }

    // The function inteface that performs the actual work
    virtual void* run(void *task) = 0;
};

#endif //PPL_NODE_HPP
