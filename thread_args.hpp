//
// Created by Rohan Kumar on 24/10/2023.
//

#ifndef PPL_THREAD_ARGS_HPP
#define PPL_THREAD_ARGS_HPP

#include "Queue.hpp"

template <typename T>

class thread_args {
public:
    thread_args(Queue<T> *emitterQueue, Queue<T> *collectorQueue) {
        this->emitterQueue = emitterQueue;
        this->collectorQueue = collectorQueue;
    }

    Queue<T> *get_emitter_queue() {
        return emitterQueue;
    }

    Queue<T> *get_collector_queue() {
        return collectorQueue;
    }

private:
    Queue<T> *emitterQueue;
    Queue<T> *collectorQueue;
};


#endif //PPL_THREAD_ARGS_HPP
