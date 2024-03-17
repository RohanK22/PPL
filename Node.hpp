/*
 * Node.hpp
 * Each node is a distribution_thread of execution that can be part of a pipeline or a farm
 * The user must implement the run function to define the behavior of the node when it receives a task
 */
#ifndef PPL_NODE_HPP
#define PPL_NODE_HPP

// Debugging
#define DEBUG_NODE
#ifdef DEBUG_NODE
#define DEBUG_NODE_PRINT(x) std::cout << x << std::endl;
#else
#define DEBUG_NODE_PRINT(x)
#endif

#include "Queue.hpp"

using namespace std;

enum class NodeType {
    None,
    Pipeline,
    PipelineEmitter,
    PipelineStage,
    Farm,
    FarmWorker,
    FarmEmitter,
    FarmCollector
};

// Default to void*, void*
template <typename T, typename U>
class Node {
private:
    pthread_t worker_thread;
    Queue<T> *input_queue;
    Queue<U> *output_queue;
    int collector_EOS_count;

    // EOS is dependent on the type of the queue
//    inline static T EOS = (std::is_same<T, std::string>::value ? "EOS" : nullptr);

protected:
    // TODO: Move state to farm manager
    int num_farm_worker_nodes;
    Node *farm_node;
    Node *pipeline_node;
    NodeType node_type;

    // EOS handling
    static T get_EOS() {
        if constexpr (std::is_same<T, std::string>::value) {
            return std::string("EOS"); // Special handling for string
        } else {
            return nullptr;
        }
    }

public:
    // Constructor
    Node(NodeType node_type = NodeType::None) {
        this->node_type = node_type;
        DEBUG_NODE_PRINT("Node created with type " << (int)node_type);
    }

    bool get_is_farm() {
        return this->is_farm;
    }

    Queue<T> *get_input_queue() {
        return this->input_queue;
    }

    Queue<U> *get_output_queue() {
        return this->output_queue;
    }

    void set_input_queue(Queue<T> *iq) {
        this->input_queue = iq;
    }

    void set_output_queue(Queue<U> *oq) {
        this->output_queue = oq;
    }

    void set_type(NodeType node_type) {
        this->node_type = node_type;
    }

    template <typename V, typename W>
    void set_farm_node(Node<V, W> *farm_node) {
        this->farm_node = farm_node;
    }

    void set_nested_node(bool is_nested_node) {
        this->is_nested_node = is_nested_node;
    }

    Node *get_farm_node() {
        return this->farm_node;
    }

    Node *get_pipeline_node() {
        return this->pipeline_node;
    }

    // Loop function for FarmEmitter nodes
    void farm_emitter_loop(Queue<T>* eq, Queue<U>* cq) {
        auto EOS = get_EOS();
        DEBUG_NODE_PRINT("Emitter EOS is nullptr ? " + std::to_string(EOS == nullptr));
        while (true) {
            U result = this->run(EOS);
            DEBUG_NODE_PRINT("Emitter " << pthread_self() << " generated task " << result);
            if (result == EOS) {
                DEBUG_NODE_PRINT("EOS for Farm Emitter");
                DEBUG_NODE_PRINT("NUMWORKERS: " + this->get_farm_node()->num_farm_worker_nodes);
                for (unsigned int i = 0; i < this->get_farm_node()->num_farm_worker_nodes; i++) {
                    cq->push(EOS);
                    DEBUG_NODE_PRINT("Emitter " << pthread_self() << " pushing EOS to worker " << i);
                }
                break;
            }
            cq->push(result);
        }
    }

    // Loop function for FarmWorker nodes
    void farm_worker_loop(Queue<T>* eq, Queue<U>* cq) {
        auto EOS = get_EOS();
        while (true) {
            T task;
            if (!eq->try_pop(task)) {
                continue;
            }
            if (task == EOS) {
                cq->push(task);
                DEBUG_NODE_PRINT("Worker " << pthread_self() << " pushing EOS to collector");
                break;
            }
            U result = this->run(task);
            cq->push(result);
        }
    }

    // Loop function for FarmCollector nodes
    void farm_collector_loop(Queue<T>* eq, Queue<U>* cq) {
        auto EOS = get_EOS();
        while (true) {
            T task;
            if (!eq->try_pop(task)) {
                continue;
            }
            if (task == EOS) {
                collector_EOS_count++;
                if (collector_EOS_count == this->get_farm_node()->num_farm_worker_nodes) {
                    cq->push(task);
                    DEBUG_NODE_PRINT("Collector " << pthread_self() << " pushing EOS to emitter");
                    break;
                }
            } else {
                U result = this->run(task);
                cq->push(result);
            }
        }
    }

    // Loop function for PipelineEmitter nodes
    void pipeline_emitter_loop(Queue<T>* eq, Queue<U>* cq) {
        auto EOS = get_EOS();
        while (true) {
            U result = this->run(EOS);
            if (result == EOS) {
                cq->push(EOS);
                break;
            }
            cq->push(result);
        }
    }

    // Loop function for PipelineStage nodes
    void pipeline_stage_loop(Queue<T>* eq, Queue<U>* cq) {
        auto EOS = get_EOS();
        while (true) {
            T task;
            if (!eq->try_pop(task)) {
                continue;
            }
            if (task == EOS) {
                cq->push(task);
                DEBUG_NODE_PRINT("Stage " << pthread_self() << " pushing EOS to collector");
                break;
            }
            U result = this->run(task);
            cq->push(result);
        }
    }

    virtual void* thread_function(void *args) {
        Queue<T> *eq = this->get_input_queue();
        Queue<U> *cq = this->get_output_queue();

        switch (this->node_type) {
            case NodeType::FarmEmitter:
                farm_emitter_loop(eq, cq);
                break;
            case NodeType::FarmWorker:
                farm_worker_loop(eq, cq);
                break;
            case NodeType::FarmCollector:
                farm_collector_loop(eq, cq);
                break;
            case NodeType::PipelineEmitter:
                pipeline_emitter_loop(eq, cq);
                break;
            case NodeType::PipelineStage:
                pipeline_stage_loop(eq, cq);
                break;
            default:
                break;
        }
    }

    static void *thread_function_helper(void *args) {
        Node *node = static_cast<Node*>(args);
        node->thread_function(node);
        return nullptr;
    }

    virtual void start_node() {
        pthread_create(&this->worker_thread, nullptr, &Node::thread_function_helper, this);
    }

    void stop_node() {
        pthread_cancel(this->worker_thread);
    }

    virtual void join_node() {
        pthread_join(this->worker_thread, nullptr);
    }

    // Run the task (to be implemented by the user)
    virtual U run(T task) = 0;
};

#endif //PPL_NODE_HPP
