//
// Created by Rohan Kumar on 20/01/2024.
//

// Each node is a thread of computation that reads tasks from its input queue and inserts completed tasks into
#ifndef PPL_NODE_HPP
#define PPL_NODE_HPP

#include "Queue.hpp"

using namespace std;

class Node {
private:
    pthread_t worker_thread;
    Queue<void*> *input_queue;
    Queue<void*> *output_queue;

    Queue<string> *input_queue_string;
    Queue<string> *output_queue_string;

    bool is_pipeline_emitter;
    bool is_pipline;
    bool is_pipeline_stage;
    bool is_farm_worker;
    bool is_farm_emitter;
    bool is_farm_collector;

    int collector_EOS_count;

protected:
    Node *farm_node;
    unsigned int num_farm_worker_nodes;
    Node *pipeline_node;
    unsigned int num_pipeline_stages;
    bool is_farm;
    bool is_pipeline; // Is set to true if the node is a PipelineManager
    bool is_distributed;
    bool is_nested_node;

public:
    // Constructor
    Node(bool is_distributed =false) {
        this->is_distributed = is_distributed;
    }
//    Node() {
//        // Malloc new queues for input and output
//        this->input_queue = new Queue<void*>();
//        this->output_queue = new Queue<void*>();
//
//        this->input_queue_string = new Queue<string>();
//        this->output_queue_string = new Queue<string>();
//    }

//    // Constructor with emitter queue and collector queue
//    Node(Queue<void*> *iq, Queue<void*> *oq) {
//        this->input_queue = iq;
//        this->output_queue = oq;
//    }

    bool get_is_farm() {
        return this->is_farm;
    }

    bool get_is_pipeline() {
        return this->is_pipline;
    }

    void set_input_queue(Queue<void*> *iq){
        this->input_queue = iq;
    }

    void set_output_queue(Queue<void*> *oq) {
        this->output_queue = oq;
    }

    void set_is_dist(bool is_d) {
        this->is_distributed = is_d;
    }

    // TODO: Parameterize Node itself to take in a template type
    void set_input_queue_string(Queue<string> *iq){
        this->input_queue_string = iq;
    }

    void set_output_queue_string(Queue<string> *oq) {
        this->output_queue_string = oq;
    }

    Queue<void*> *get_input_queue() {
        return this->input_queue;
    }

    Queue<string> *get_input_queue_string() {
        return this->input_queue_string;
    }

    Queue<void*> *get_output_queue() {
        return this->output_queue;
    }

    Queue<string> *get_output_queue_string() {
        return this->output_queue_string;
    }

    void set_is_pipeline_emitter(bool is_pipeline_emitter) {
        this->is_pipeline_emitter = is_pipeline_emitter;
    }

    void set_is_pipeline_stage(bool is_pipeline_stage) {
        this->is_pipeline_stage = is_pipeline_stage;
    }

    void set_is_farm_worker(bool is_farm_worker) {
        this->is_farm_worker = is_farm_worker;
    }

    void set_is_farm_emitter(bool is_farm_emitter) {
        this->is_farm_emitter = is_farm_emitter;
    }

    void set_is_farm_collector(bool is_farm_collector) {
        this->is_farm_collector = is_farm_collector;
    }

    void set_farm_node(Node *farm_node) {
        this->farm_node = farm_node;
    }

    void set_nested_node(bool is_nested_node) {
        this->is_nested_node = is_nested_node;
    }

    Node *get_farm_node() {
        return this->farm_node;
    }

    int get_num_farm_worker_nodes() {
        return this->num_farm_worker_nodes;
    }

    Node *get_pipeline_node() {
        return this->pipeline_node;
    }

    virtual void *thread_function(void *args) {
        // Get the arguments
        Node *node = static_cast<Node*>(args);

        if (node == nullptr) {
            std::cout << "Node is null" << std::endl;
            throw std::runtime_error("Node argument is null that is passed to the thread function");
        }

        if (!node->is_distributed) {
            Queue<void*> *eq = node->get_input_queue();
            Queue<void*> *cq = node->get_output_queue();

            while (true) {
                if (node->is_farm_emitter) {
                    void *result = node->run(nullptr);
                    if (result == nullptr) {
//                    std::cout << "Farm emitter output null result (EOS)" << std::endl;
                        // Push EOS for each worker
                        for (int i = 0; i < node->get_farm_node()->num_farm_worker_nodes; i++) {
                            cq->push(nullptr);
                        }
                        break;
                    }
                    cq->push(result);
                    continue;
                } else if (node->is_pipeline_emitter) {
                    void *result = node->run(nullptr);
                    if (result == nullptr) {
                        cq->push(nullptr);
                        break;
                    }
                    cq->push(result);
                    continue;
                }

                void *task = nullptr;
                bool success = eq->try_pop(task);

                if (!success) {
                    // Busy wait only stops if an EOS is received
                    continue;
                }

                if (task == nullptr) {
                    if (node->is_farm_collector) {
                        node->collector_EOS_count++;
//                    std::cout << "Collector received EOS " << node->collector_EOS_count << std::endl;
                        if (node->collector_EOS_count == node->get_farm_node()->num_farm_worker_nodes) {
//                        std::cout << "Collector received EOS from all workers" << std::endl;
                            cq->push(nullptr);
                            break;
                        }
                        continue;
                    } else if (node->is_pipeline_stage) {
                        // Make sure that the EOS is pushed to the output queue so that the next stage can receive it
                        std::cout << "Stage received EOS" << std::endl;
                        cq->push(task);
                        break;
                    } else if (node->is_farm_worker) {
                        // Push EOS to collector
                        cq->push(task);
                        std::cout << "Worker " << pthread_self() << " pushing EOS to collector" << std::endl;
                    }
                    break;
                }

                // Run the task and store results
                void *result = node->run(task);

                // Push the result to the output queue
                cq->push(result);
            }

        } else {
            Queue<string> *eq = node->input_queue_string;
            Queue<string> *cq = node->output_queue_string;

            while (true) {
                if (node->is_farm_emitter) {
                    string result = node->run(string(""));
                    if (result == "EOS") {
//                    std::cout << "Farm emitter output null result (EOS)" << std::endl;
                        // Push EOS for each worker
                        for (int i = 0; i < node->get_farm_node()->num_farm_worker_nodes; i++) {
                            cq->push(string("EOS"));
                        }
                        break;
                    }
                    cq->push(result);
                    continue;
                } else if (node->is_pipeline_emitter) {
                    string result = node->run(string(""));
                    cout << "Pipeline emitter result " << result << endl;
                    if (result == "EOS") {
                        cq->push(string("EOS"));
                        break;
                    }
                    cq->push(result);
                    continue;
                }

                string task = "";
                bool success = eq->try_pop(task);

                if (!success) {
                    // Busy wait only stops if an EOS is received
                    continue;
                }

                if (task == "EOS") {
                    if (node->is_farm_collector) {
                        node->collector_EOS_count++;
//                    std::cout << "Collector received EOS " << node->collector_EOS_count << std::endl;
                        if (node->collector_EOS_count == node->get_farm_node()->num_farm_worker_nodes) {
//                        std::cout << "Collector received EOS from all workers" << std::endl;
                            cq->push(string("EOS"));
                            break;
                        }
                        continue;
                    } else if (node->is_pipeline_stage) {
                        // Make sure that the EOS is pushed to the output queue so that the next stage can receive it
                        std::cout << "Stage received EOS" << std::endl;
                        cq->push(task);
                        break;
                    } else if (node->is_farm_worker) {
                        // Push EOS to collector
                        cq->push(task);
                        std::cout << "Worker " << pthread_self() << " pushing EOS to collector" << std::endl;
                    }
                    break;
                }
                // Run the task and store results
                string result = node->run(task);
                // Push the result to the output queue
                cq->push(result);
            }
        }
    }

    static void *thread_function_helper(void *args) {
        Node *node = static_cast<Node*>(args);
        node->thread_function(node);
        return nullptr;
    }

    virtual void start_node() {
        pthread_create(&this->worker_thread, nullptr, &Node::thread_function_helper, this);
    }

    void stop_node() {
        pthread_cancel(this->worker_thread);
    }

    virtual void join_node() {
        pthread_join(this->worker_thread, nullptr);
    }

    // TODO Fix
    // Run the task (to be implemented by the user)
    virtual void* run(void *task) {
        return nullptr;
    }

    virtual string run(string task) {
        return "";
    }
};

#endif //PPL_NODE_HPP
