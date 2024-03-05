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
    bool is_nested_node;

public:
    FarmManager() {
        this->is_farm = true;
        this->is_nested_node = true;
    }


    // Add emitter node
    void add_emitter(Node *node) {
        node->set_is_farm_emitter(true);
        node->set_input_queue(this->get_input_queue());
        node->set_farm_node(this);
        this->emitter_node = node;
    }

    // Add collector node
    void add_collector(Node *node) {
        node->set_output_queue(this->get_output_queue());
        node->set_farm_node(this);
        node->set_is_farm_collector(true);
        this->collector_node = node;
    }

    // Add worker nodes
    void add_worker(Node *node) {
        node->set_is_farm_worker(true);
        node->set_farm_node(this);
        worker_nodes.push_back(node);
        this->num_farm_worker_nodes = worker_nodes.size();
    }

    void *thread_function(void *args) override {
        std::cout << "Farm Manager thread function" << std::endl;
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
                std::cout << "EOS for farm" << std::endl;

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
        return nullptr;
    }

    static void* thread_function_helper(void *context) {
        return static_cast<FarmManager*>(context)->thread_function(nullptr);
    }

    void *collector_thread_function(void *args) {
        std::cout << "Farm Manager collector thread function" << std::endl;
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

                    if (eos_counts == num_farm_worker_nodes) {
                        std::cout << "Farm node shutting down as Collector received all EOS" << std::endl;
                        this->get_output_queue()->push(nullptr);
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
        return nullptr;
    }

    static void* collector_thread_function_helper(void *context) {
        return static_cast<FarmManager*>(context)->collector_thread_function(nullptr);
    }

    void start_node() override {
        std::cout << "Starting Farm Manager" << std::endl;
        // Start nodes
        if (this->emitter_node != nullptr) { // TODO: Connect
            this->emitter_node->start_node();
        }
        for (int i = 0; i < num_farm_worker_nodes; i++) {
            std::cout << "Farm Input queue: " << this->get_input_queue() << std::endl;
            std::cout << "Farm Output queue: " << this->get_output_queue() << std::endl;
            std::cout << worker_nodes[i]->get_input_queue() << std::endl;
            std::cout << worker_nodes[i]->get_output_queue() << std::endl;

            worker_nodes[i]->start_node();
        }
        if (this->collector_node != nullptr) {
            this->collector_node->start_node();
        }

        std::cout << "Farm Manager started" << std::endl;

        if (this->is_nested_node) {
            pthread_create(&this->thread, nullptr, &FarmManager::thread_function_helper, this);
            pthread_create(&this->collector_thread, nullptr, &FarmManager::collector_thread_function_helper, this);
        }
    }

    void join_node() override {
        // join threads
        if (this->emitter_node != nullptr) {
            this->emitter_node->join_node();
        }
        for (int i = 0; i < num_farm_worker_nodes; i++) {
            worker_nodes[i]->join_node();

            std::cout << "Worker " << i << " joined" << std::endl;
        }
        if (this->collector_node != nullptr) {
            this->collector_node->join_node();
        }

        if (this->is_nested_node) {
            pthread_join(this->thread, nullptr);
            pthread_join(this->collector_thread, nullptr);
        }
    }

    // Should never be called
    void* run(void *task) override {
        throw std::runtime_error("FarmManager run should never be called");
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