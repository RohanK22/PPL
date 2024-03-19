#ifndef PPL_PIPELINEMANAGER_HPP
#define PPL_PIPELINEMANAGER_HPP

#include "Queue.hpp"
#include "Node.hpp"
#include <vector>

template <typename T>
class PipelineManager : public Node <T> {
private:
    std::vector<Node<T> *> pipeline_nodes;
    unsigned int num_pipeline_stages;

    // EOS type
    T EOS = this->get_EOS();

public:
    PipelineManager() {
        this->set_type(NodeType::Pipeline);
    }

    unsigned int get_num_pipeline_stages() {
        return this->num_pipeline_stages;
    }

    // Add stage to the pipeline
    void add_stage(Node<T> *node) {
        // node->set_is_pipeline_stage(true);
        if (node->get_node_type() != NodeType::PipelineEmitter) {
            node->set_type(NodeType::PipelineStage);
        }
        pipeline_nodes.push_back(node);
        this->num_pipeline_stages = pipeline_nodes.size();
    }

    void start_node() override {
        this->set_input_queue(new Queue<T>());
        this->set_output_queue(new Queue<T>());

        for (int i = 0; i < num_pipeline_stages; i ++) {
            if (i == 0) {
                pipeline_nodes[i]->set_input_queue(this->get_input_queue());
                pipeline_nodes[i]->set_output_queue(new Queue<T>());
            } else if (i == num_pipeline_stages - 1) {
                pipeline_nodes[i]->set_input_queue(pipeline_nodes[i - 1]->get_output_queue());
                pipeline_nodes[i]->set_output_queue(this->get_output_queue());
            } else {
                pipeline_nodes[i]->set_input_queue(pipeline_nodes[i - 1]->get_output_queue());
                pipeline_nodes[i]->set_output_queue(new Queue<T>());
            }
            pipeline_nodes[i]->start_node();
        }
    }

    void join_node() override {
        // Join node
        for (int i = 0; i < num_pipeline_stages; i ++) {
            pipeline_nodes[i]->join_node();
        }
    }

    T run(T task) {
        // Task from MPI
        this->get_input_queue()->push(task);
    }

    void run_until_finish() {
        try {
            this->start_node();
            this->join_node();
        } catch (std::exception &e) {
            std::cout << "Exception in PipelineManager run_until_finish: " << e.what() << std::endl;
        }
    }
};

#endif //PPL_PIPELINEMANAGER_HPP
