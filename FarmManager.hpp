#ifndef FARMMANAGER_HPP
#define FARMMANAGER_HPP

#include "Queue.hpp"
#include "Node.hpp"
#include <vector>
#include <pthread.h>
#include <iostream>
#include <set>

class FarmManager : public Node {
protected:
    Node *emitter_node;
    std::vector<Node*> worker_nodes;
    Node *collector_node;
    pthread_t thread;
    pthread_t collector_thread;
    int rr_index = 0;

public:
    FarmManager(bool distributed = false) {
        this->is_farm = true;
        this->is_nested_node = is_distributed;
        this->is_distributed = distributed;
    }


    // Add emitter node
    void add_emitter(Node *node) {
        node->set_is_farm_emitter(true);
        node->set_farm_node(this);
        this->emitter_node = node;
    }

    // Add collector node
    void add_collector(Node *node) {
        node->set_farm_node(this);
        node->set_is_farm_collector(true);
        this->collector_node = node;
    }

    // Add worker nodes
    void add_worker(Node *node) {
        node->set_is_farm_worker(true);
        node->set_farm_node(this);
        node->set_is_dist(this->is_distributed); // TODO Fix hardcoding
        worker_nodes.push_back(node);
        this->num_farm_worker_nodes = worker_nodes.size();
    }

    // TODO: Can parameterise this function
    void *thread_function(void *args) override {
        if (!is_distributed) {
//            std::cout << "Farm Manager thread function" << std::endl;
            auto queue = this->get_input_queue();
            if (this->emitter_node != nullptr) {
                queue = this->emitter_node->get_output_queue();
            }
            while (true) {
                void *task = nullptr;
                bool success = queue->try_pop(task);
                if (!success) {
//                std::cout << "No task for farm" << std::endl;
                    continue;
                }
                if (task == nullptr) {
//                    std::cout << "EOS for farm" << std::endl;

                    // Send EOS to all workers
                    for (int i = 0; i < num_farm_worker_nodes; i++) {
                        this->worker_nodes[i]->get_input_queue()->push(nullptr);
                    }
                    break;
                }

                // Assign task to worker
                this->worker_nodes[rr_index]->get_input_queue()->push(task);
                rr_index = (rr_index + 1) % num_farm_worker_nodes;
            }
        } else {
//            std::cout << "Farm Manager Dist thread function" << std::endl;
            auto queue = this->get_input_queue_string();
            if (this->emitter_node != nullptr) {
                queue = this->emitter_node->get_output_queue_string();
            }
            while (true) {
                string task = "";
                bool success = queue->try_pop(task);
//                cout << "Task is " << task << endl;
                if (!success) {
//                    std::cout << "No task for farm" << std::endl;
                    continue;
                }
                if (task == "EOS") {
                    std::cout << "EOS for farm" << std::endl;

                    // Send EOS to all workers
                    for (int i = 0; i < num_farm_worker_nodes; i++) {
                        cout << "Sending EOS to farm node worker " << i << endl;
                        this->worker_nodes[i]->get_input_queue_string()->push(string("EOS"));
                    }
                    break;
                }
                // Assign task to worker
                this->worker_nodes[rr_index]->get_input_queue_string()->push(task);
                rr_index = (rr_index + 1) % num_farm_worker_nodes;
            }
        }
        return nullptr;
    }

    static void* thread_function_helper(void *context) {
        return static_cast<FarmManager*>(context)->thread_function(context);
    }

    void *collector_thread_function(void *args) {
        if (!is_distributed) {
//            std::cout << "Farm Manager collector thread function" << std::endl;
            int eos_counts = 0;
            // Set of worker indices that have finished
            std::set<int> finished_workers;
            bool end = false;
            auto queue = this->get_output_queue();
            if (this->collector_node != nullptr) {
                queue = this->collector_node->get_input_queue();
            }
            while (true) {
                // Loop through all workers that haven't shut down
                for (int i = 0; i < num_farm_worker_nodes; i++) {
                    if (finished_workers.find(i) != finished_workers.end()) {
                        continue;
                    }
                    void *task = nullptr;
                    bool success = this->worker_nodes[i]->get_output_queue()->try_pop(task);

                    if (!success) {
                        continue;
                    }
                    if (task == nullptr) {
                        eos_counts++;
                        finished_workers.insert(i);

                        if (this->collector_node != nullptr) {
                            this->collector_node->get_input_queue()->push(nullptr);
                        }

                        if (eos_counts == num_farm_worker_nodes) {
//                            std::cout << "Farm node shutting down as Collector received all EOS" << std::endl;
                            if (this->collector_node == nullptr)
                                queue->push(nullptr);
                            end = true;
                            break;
                        }
                        continue;
                    }
                    queue->push(task);
                }
                if (end) {
                    break;
                }
            }
        } else {
            std::cout << "Dist Farm Manager collector thread function" << std::endl;
            int eos_counts = 0;
            // Set of worker indices that have finished
            std::set<int> finished_workers;
            bool end = false;
            auto queue = this->get_output_queue_string();
            if (this->collector_node != nullptr) {
                queue = this->collector_node->get_input_queue_string();
            }
            while (true) {
                    // Loop through all workers that haven't shut down
                for (int i = 0; i < num_farm_worker_nodes; i++) {
                    if (finished_workers.find(i) != finished_workers.end()) {
                        continue;
                    }
                    string task;
                    bool success = this->worker_nodes[i]->get_output_queue_string()->try_pop(task);
                    if (!success) {
                        continue;
                    }
                    cout << "Task bro " << task << endl;
                    if (task == "EOS") {
                        eos_counts++;
                        finished_workers.insert(i);

                        cout << "YOOOOOOOOO" <<endl;

                        if (eos_counts == num_farm_worker_nodes) {
                            std::cout << "Farm node shutting down as Collector received all EOS" << std::endl;
                            queue->push(string("EOS"));
                            end = true;
                            break;
                        }
                        continue;
                    }
                    queue->push(task);
                }
                if (end) {
                    break;
                }
            }
        }
        return nullptr;
    }

    static void* collector_thread_function_helper(void *context) {
        return static_cast<FarmManager*>(context)->collector_thread_function(context);
    }

    void start_node() override {
        // Set input and output queue of farm node
        this->set_input_queue(new Queue<void*>());
        this->set_output_queue(new Queue<void*>());
        this->set_input_queue_string(new Queue<string>());
        this->set_output_queue_string(new Queue<string>());

        // Start nodes
        if (this->emitter_node != nullptr) { // TODO: Connect
            this->emitter_node->set_input_queue(this->get_input_queue());
            this->emitter_node->set_output_queue(new Queue<void*>());
            this->emitter_node->set_input_queue_string(this->get_input_queue_string());
            this->emitter_node->set_output_queue_string(new Queue<string>());
            this->emitter_node->start_node();
        }
        for (int i = 0; i < num_farm_worker_nodes; i++) {

            // Each worker node has its own input and output queue
            worker_nodes[i]->set_input_queue(new Queue<void*>());
            worker_nodes[i]->set_output_queue(new Queue<void*>());
            worker_nodes[i]->set_input_queue_string(new Queue<string>());
            worker_nodes[i]->set_output_queue_string(new Queue<string>());

//            std::cout << "Farm Input queue: " << this->get_input_queue() << std::endl;
//            std::cout << "Farm Output queue: " << this->get_output_queue() << std::endl;
//            std::cout << worker_nodes[i]->get_input_queue() << std::endl;
//            std::cout << worker_nodes[i]->get_output_queue() << std::endl;

            worker_nodes[i]->start_node();
        }
        if (this->collector_node != nullptr) {
            this->collector_node->set_input_queue(new Queue<void*>());
            this->collector_node->set_output_queue(this->get_output_queue());
            this->collector_node->set_input_queue_string(new Queue<string>());
            this->collector_node->set_output_queue_string(this->get_output_queue_string());
            this->collector_node->start_node();
        }

            // This moves tasks from the input queue to the worker nodes
            pthread_create(&this->thread, nullptr, &FarmManager::thread_function_helper, this);

            // This moves tasks from the worker nodes to the output queue
            pthread_create(&this->collector_thread, nullptr, &FarmManager::collector_thread_function_helper, this);
//        }
    }

    void join_node() override {
        // join threads
        if (this->emitter_node != nullptr) {
            this->emitter_node->join_node();
        }
        for (int i = 0; i < num_farm_worker_nodes; i++) {
            worker_nodes[i]->join_node();
        }
        if (this->collector_node != nullptr) {
            this->collector_node->join_node();
        }

//        if (this->is_nested_node) {
            pthread_join(this->thread, nullptr);
            pthread_join(this->collector_thread, nullptr);
//        }
    }

    // Should never be called
    void* run(void *task) override {
        throw std::runtime_error("FarmManager run should never be called");
    }

    string run(string task) override {
        cout << "Farm is receiving task from MPI" << endl;
        this->get_input_queue_string()->push(task);
        return string("Ignore");
    }

    void run_until_finish() {
        this->is_nested_node = false;

        // Start the farm
        this->start_node();

        // Join the farm
        this->join_node();
    }
};

#endif //FARMMANAGER_HPP