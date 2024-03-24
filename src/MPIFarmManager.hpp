#ifndef PPL_MPIFARMMANAGER_HPP
#define PPL_MPIFARMMANAGER_HPP

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <string>
#include "Node.hpp"

namespace mpi = boost::mpi;
using namespace std;

class MPIFarmManager
{
private:
    mpi::environment *env;
    mpi::communicator *world;

    // Worker function callbacks
    Node<string> *emitter_node;
    Node<string> *collector_node;
    vector<Node<string> *> worker_nodes;

    // Scheduling state
    int num_workers;
    int rr_index = 0;

    pthread_t thread;
    pthread_t result_thread;

    bool is_farm;

public:
    MPIFarmManager(mpi::environment *env, mpi::communicator *world) : env(env), world(world)
    {
        this->is_farm = true;
    }

    void add_emitter(Node<string> *node)
    {
        this->emitter_node = node;
    }

    void add_collector(Node<string> *node)
    {
        this->collector_node = node;
    }

    void add_worker(Node<string> *node)
    {
        this->worker_nodes.push_back(node);
        this->num_workers++;
    }

    // thread_function2 - used to read results from farm output queue and push to collector
    void *thread_function2(void *args)
    {
        MPIFarmManager *manager = static_cast<MPIFarmManager *>(args);
        Node<string> *worker_node = manager->worker_nodes[world->rank() - 3];
        auto output_queue = worker_node->get_output_queue();
        if (!output_queue)
        {
            throw runtime_error("Fatal error: Distributed Node Ouptput queue is null");
        }

        while (true)
        {
            string result;
            bool success = output_queue->try_pop(result);
            if (!success)
            {
                continue;
            }
            world->send(2, 0, result);

            if (result == "EOS")
            {
                break;
            }
        }
        return nullptr;
    }

    static void *thread_function_helper2(void *context)
    {
        return static_cast<MPIFarmManager *>(context)->thread_function2(context);
    }

    void run_until_finish()
    {
        int size = world->size();
        if (world->rank() == 0)
        {
            // Master Process that manages the farm
            DEBUG_NODE_PRINT("MPI Farm Manager: Starting farm manager with " + to_string(num_workers) + " workers");

            // Wait for response from collector
            while (true)
            {
                string response;
                world->recv(2, 0, response);
                if (response == "EOS")
                {
                    DEBUG_NODE_PRINT("MPI Farm Manager: Received EOS from collector");
                    break;
                }
            }
        }
        else if (world->rank() == 1)
        {
            while (true)
            {
                string task = emitter_node->run("");
                if (task == "EOS")
                {
                    for (int i = 0; i < size - 3; i++)
                    {
                        // Send EOS to all workers
                        world->send(i + 3, 0, string("EOS"));
                    }

                    break;
                }
                // Distribute task to worker -- round robin
                world->send(rr_index + 3, 0, task);
                rr_index = (rr_index + 1) % (size - 3);
            }
        }
        else if (world->rank() == 2)
        {
            int collector_eos_count = 0;
            while (true)
            {
                string result;
                world->recv(mpi::any_source, 0, result);

                if (result == "EOS")
                {
                    collector_eos_count++;
                    if (collector_eos_count == num_workers)
                    {
                        // Send EOS to master
                        world->send(0, 0, string("EOS"));
                        break;
                    }
                    continue; // Wait for more EOS
                }

                // Process result
                string response = collector_node->run(result);

                // Send response to master
                world->send(0, 0, response);
            }
        }
        else
        {
            Node<string> *worker_node = worker_nodes[world->rank() - 3];

            if (worker_node->get_node_type() == NodeType::Farm || worker_node->get_node_type() == NodeType::Pipeline)
            {
                // Farm node
                worker_node->start_node();
                pthread_t result_collector_thread;
                pthread_create(&result_collector_thread, nullptr, thread_function_helper2, this);

                while (true)
                {
                    string task;
                    world->recv(1, 0, task);

                    if (task == "EOS")
                    {
                        worker_node->get_input_queue()->push("EOS");
                        break;
                    }

                    // Send task to farm
                    worker_node->run(task);
                }
                pthread_join(result_collector_thread, nullptr);

                worker_node->join_node();
            }
            else
            {
                // Regular node
                while (true)
                {
                    string task;
                    world->recv(1, 0, task);
                    if (task == "EOS")
                    {
                        worker_node->get_input_queue()->push("EOS");
                        worker_node->join_node();

                        // Send EOS to collector
                        world->send(2, 0, string("EOS"));

                        break;
                    }
                    // Send task to node
                    worker_node->get_input_queue()->push(task);
                }
            }
        }
    }
};

#endif // PPL_MPIFARMMANAGER_HPP
