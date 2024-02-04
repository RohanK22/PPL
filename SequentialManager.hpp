//
// Created by rk76 on 23/11/23.
//

#ifndef PPL_SEQUENTIALMANAGER_HPP
#define PPL_SEQUENTIALMANAGER_HPP

// Similar to farm manager except that it runs all tasks sequentially on the main thread
// Assuming the tasks are still passed through a queue

#include "Queue.hpp"

template <typename T>
class SequentialManager {
private:
    Queue<T> *emitter_queue;
    Queue<T> *collector_queue;

public:
    SequentialManager() = default;

    SequentialManager(Queue<T> *eq, Queue<T> *cq) {
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

    void run() {
        while (!emitter_queue->empty()) {
//            std::cout << "Thread " << pthread_self() << " received a task " << std::endl;

            T task;
            bool success = emitter_queue->try_pop(task);

            if (!success) {
                // Stop thread if there are no more tasks
                break;
            }

            task.run();
            collector_queue->push(task);
        }
    }
};

#endif //PPL_SEQUENTIALMANAGER_HPP
