//
// Created by Rohan Kumar on 24/10/2023.
//

#ifndef PPL_THREAD_ARGS_HPP
#define PPL_THREAD_ARGS_HPP

#include "Queue.hpp"
#include "Node.hpp"

class thread_args {
public:
    thread_args(Node *node) {
        this->node = node;
    }

    Queue<void*> *get_emitter_queue() {
        return emitterQueue;
    }

    Queue<void*> *get_collector_queue() {
        return collectorQueue;
    }

    Node *get_node() {
        return node;
    }

private:
    Node *node;
    Queue<void*> *emitterQueue;
    Queue<void*> *collectorQueue;
};


#endif //PPL_THREAD_ARGS_HPP
