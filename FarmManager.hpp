#ifndef FARMMANAGER_HPP
#define FARMMANAGER_HPP

#include "Queue.hpp"
#include "Task.hpp"
#include "thread_args.hpp"
#include "Node.hpp"
#include <vector>

template <typename T>

class FarmManager {
private:
    Queue<T> *emitter_queue;
    Queue<T> *collector_queue;
    std::vector<Node<T>> farm_nodes;
    unsigned int num_worker_threads;

public:
    // Constructor
    FarmManager() = default;

    // Constructor with emitter queue and collector queue
    FarmManager(Queue<T> *eq, Queue<T> *cq) {
        this->emitter_queue = eq;
        this->collector_queue = cq;
    }

    // Emitter queue has a tasks, the collector queue collects results from those tasks
    // FarmManager creates a farm of threads and distributes tasks to them
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
        // Add EOS tasks to the emitter queue for each worker thread
        for (int i = 0; i < num_worker_threads; i++) {
            T task;
            task.set_is_eos(true);
            emitter_queue->push(task);
        }

        // create nodes
        for (int i = 0; i < num_worker_threads; i++) {
            // Create a node
            Node<T> node = Node<T>(emitter_queue, collector_queue);
            node.set_is_farm(true);

            node.start_node();
            farm_nodes.push_back(node);
        }

        // join threads
        for (int i = 0; i < num_worker_threads; i++) {
            farm_nodes[i].join_node();
        }
    }
};

#endif //FARMMANAGER_HPP