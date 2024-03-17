#ifndef FARMMANAGER_HPP
#define FARMMANAGER_HPP

#include "Queue.hpp"
#include "Node.hpp"
#include <vector>
#include <pthread.h>
#include <iostream>
#include <set>

template <typename T, typename U>
class FarmManager : public Node <T, U> {
protected:
    // Farm nodes
    Node<T, T> *emitter_node;
    std::vector<Node<T, U>*> worker_nodes;
    Node<U, U> *collector_node;

    // Thread for moving tasks from input queue to worker nodes and from worker nodes to output queue
    pthread_t distribution_thread;
    pthread_t collector_thread;

    // State variables
    int rr_index = 0;

    // EOS type
//    inline static T EOS = (std::is_same<T, std::string>::value ? "EOS" : nullptr);

public:
    FarmManager() {
        this->node_type = NodeType::Farm;
    }

    // Add emitter node
    void add_emitter(Node<T, T> *node) {
        node->set_type(NodeType::FarmEmitter);
        node->set_farm_node(this);
        this->emitter_node = node;
    }

    // Add collector node
    void add_collector(Node<U, U> *node) {
        node->set_type(NodeType::FarmCollector);
        node->set_farm_node(this);
        this->collector_node = node;
    }

    // Add worker nodes
    void add_worker(Node<T, U> *node) {
        node->set_type(NodeType::FarmWorker);
        node->set_farm_node(this);
        worker_nodes.push_back(node);
        this->num_farm_worker_nodes = worker_nodes.size();
    }

    int get_num_farm_worker_nodes() {
        return this->num_farm_worker_nodes;
    }

    void *thread_function(void *args) {
        DEBUG_NODE_PRINT("Node Farm distribution thread started");
        auto EOS = this->get_EOS();
        auto queue = this->get_input_queue();
        if (this->emitter_node != nullptr) {
            queue = this->emitter_node->get_output_queue();
        }
        while (true) {
            T task;
            bool success = queue->try_pop(task);
            if (!success) {
                continue;
            }
            DEBUG_NODE_PRINT("Node Farm distribution thread received task: " + std::to_string(task == nullptr));
            if (task == EOS) {
                for (int i = 0; i < this->num_farm_worker_nodes; i++) {
                    this->worker_nodes[i]->get_input_queue()->push(EOS);
                }
                cout << "EYOOOOOOOOOOOOEYOOOOOOOOOOOOEYOOOOOOOOOOOOEYOOOOOOOOOOOOEYOOOOOOOOOOOOEYOOOOOOOOOOOO" <<endl;
                break;
            }
            this->worker_nodes[rr_index]->get_input_queue()->push(task);
            DEBUG_NODE_PRINT("Node Farm distribution thread pushed task to worker node " + std::to_string(rr_index));
            rr_index = (rr_index + 1) % this->num_farm_worker_nodes;
        }
        DEBUG_NODE_PRINT("Node Farm distribution thread ended");
        return nullptr;
    }

    static void* thread_function_helper(void *context) {
        return static_cast<FarmManager*>(context)->thread_function(context);
    }

    void *collector_thread_function(void *args) {
        auto EOS = this->get_EOS();
        int eos_counts = 0;
        std::set<int> finished_workers;
        bool end = false;
        auto queue = this->get_output_queue();
        if (this->collector_node != nullptr) {
            queue = this->collector_node->get_input_queue();
        }
        while (true) {
            for (int i = 0; i < this->num_farm_worker_nodes; i++) {
                if (finished_workers.find(i) != finished_workers.end()) {
                    continue;
                }
                U task;
                bool success = this->worker_nodes[i]->get_output_queue()->try_pop(task);
                if (!success) {
                    continue;
                }
                if (task == EOS) {
                    eos_counts++;
                    finished_workers.insert(i);
                    if (this->collector_node != nullptr) {
                        this->collector_node->get_input_queue()->push(EOS);
                    }
                    if (eos_counts == this->num_farm_worker_nodes) {
                        if (this->collector_node == nullptr)
                            queue->push(EOS);
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
        return static_cast<FarmManager*>(context)->collector_thread_function(context);
    }

    void start_node() override {
        this->set_input_queue(new Queue<T>());
        this->set_output_queue(new Queue<U>());

        if (this->emitter_node != nullptr) {
            this->emitter_node->set_input_queue(this->get_input_queue());
            this->emitter_node->set_output_queue(new Queue<T>());
            this->emitter_node->start_node();
        }
        for (int i = 0; i < this->num_farm_worker_nodes; i++) {
            worker_nodes[i]->set_input_queue(new Queue<T>());
            worker_nodes[i]->set_output_queue(new Queue<U>());
            worker_nodes[i]->start_node();
        }
        if (this->collector_node != nullptr) {
            this->collector_node->set_input_queue(new Queue<U>());
            this->collector_node->set_output_queue(this->get_output_queue());
            this->collector_node->start_node();
        }

        pthread_create(&this->distribution_thread, nullptr, &FarmManager::thread_function_helper, this);
        pthread_create(&this->collector_thread, nullptr, &FarmManager::collector_thread_function_helper, this);
    }

    void join_node() override {
        if (this->emitter_node != nullptr) {
            this->emitter_node->join_node();
        }
        for (int i = 0; i < this->num_farm_worker_nodes; i++) {
            worker_nodes[i]->join_node();
        }
        if (this->collector_node != nullptr) {
            this->collector_node->join_node();
        }
        pthread_join(this->distribution_thread, nullptr);
        pthread_join(this->collector_thread, nullptr);
    }

//    // Receive task from MPI
//    string run(string task) {
//        cout << "Farm is receiving task from MPI" << endl;
//        this->get_input_queue_string()->push(task);
//        return string("Node received task from MPI");
//    }

    U run(T task) {
        cout << "Farm is receiving task from MPI" << endl;
        this->get_input_queue()->push(task);
        return U();
    }

    void run_until_finish() {
//        this->is_nested_node = false;
        this->start_node();
        this->join_node();
    }
};

#endif //FARMMANAGER_HPP