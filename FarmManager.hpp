#ifndef FARMMANAGER_HPP
#define FARMMANAGER_HPP

#include "Queue.hpp"
#include "Node.hpp"
#include <vector>


class FarmManager : public Node {
private:
    Node *emitter_node;
    std::vector<Node*> worker_nodes;
    Node *collector_node;

public:
    FarmManager() = default;

    unsigned int get_num_worker_nodes() {
        return this->num_farm_worker_nodes;
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
        if (this->emitter_node != nullptr) {
            node->set_input_queue(this->emitter_node->get_output_queue());
        } else {
            node->set_input_queue(this->get_input_queue());
        }
        if (this->collector_node != nullptr) {
            node->set_output_queue(this->collector_node->get_input_queue());
        } else {
            node->set_output_queue(this->get_output_queue());
        }

        this->num_farm_worker_nodes = worker_nodes.size();
    }

    void* run(void *task) {
//        std::cout << "Emitter input queue: " << emitter_node->get_input_queue() << std::endl;
//        std::cout << "Emitter output queue: " << emitter_node->get_output_queue() << std::endl;
//        std::cout << "Worker 1 input queue: " << worker_nodes[0]->get_input_queue() << std::endl;
//        std::cout << "Worker 1 output queue: " << worker_nodes[0]->get_output_queue() << std::endl;
//        std::cout << "Collector input queue: " << collector_node->get_input_queue() << std::endl;
//        std::cout << "Collector output queue: " << collector_node->get_output_queue() << std::endl;

        // Start nodes
        emitter_node->start_node();
        for (int i = 0; i < num_farm_worker_nodes; i++) {
            worker_nodes[i]->set_is_farm_worker(true);
            worker_nodes[i]->start_node();
        }
        collector_node->start_node();

        // join threads
        emitter_node->join_node();
        for (int i = 0; i < num_farm_worker_nodes; i++) {
            worker_nodes[i]->join_node();
        }
        collector_node->join_node();
        return nullptr;
    }
};

#endif //FARMMANAGER_HPP