//
// Created by Rohan Kumar on 24/10/2023.
//

#ifndef PPL_THREAD_ARGS_HPP
#define PPL_THREAD_ARGS_HPP

#include "Queue.hpp"
#include "Node.hpp"

template <typename T>

class thread_args {
public:
    thread_args(Node<T> *node) {
        this->node = node;
    }

    Queue<T> *get_emitter_queue() {
        return emitterQueue;
    }

    Queue<T> *get_collector_queue() {
        return collectorQueue;
    }

    Node<T> *get_node() {
        return node;
    }

private:
    Node<T> *node;
    Queue<T> *emitterQueue;
    Queue<T> *collectorQueue;
};


#endif //PPL_THREAD_ARGS_HPP
