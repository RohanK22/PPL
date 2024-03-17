#ifndef PPL_MPIFARMMANAGER_HPP
#define PPL_MPIFARMMANAGER_HPP

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <string>
#include "Node.hpp"

namespace mpi = boost::mpi;
using namespace std;

class MPIFarmManager {
private:
    mpi::environment *env;
    mpi::communicator *world;

    // Worker function callbacks
    Node *emitter_node;
    Node *collector_node;
    vector<Node*> worker_nodes;

    // Scheduling state
    int num_workers;
    int rr_index = 0;

    pthread_t thread;
    pthread_t result_thread;

    bool is_farm;

public:
    MPIFarmManager(mpi::environment *env, mpi::communicator *world) : env(env), world(world) {
        this->is_farm = true;
    }

    void add_emitter(Node *node) {
        this->emitter_node = node;
    }

    void add_collector(Node *node) {
        this->collector_node = node;
    }

    void add_worker(Node *node) {
        this->worker_nodes.push_back(node);
        this->num_workers++;
    }

    // thread_function2 - used to read results from farm output queue and push to collector
    void *thread_function2(void *args) {
        MPIFarmManager *manager = static_cast<MPIFarmManager*>(args);
        Node *worker_node = manager->worker_nodes[world->rank() - 3];
        auto output_queue = worker_node->get_output_queue_string();
        if (!output_queue) {
            throw runtime_error("------------------------Fatal error: Output queue is null");
        }

        while (true) {
            string result;
            bool success = output_queue->try_pop(result);
            if (!success) {
                continue;
            }
            world->send(2, 0, result);

            if (result == "EOS") {

                cout << "awhopivawnvuoaewrhpiausrghawiouehgpowahsfgpiouasurghs" << endl;
                break;
            }
        }
        return nullptr;
    }

    static void* thread_function_helper2(void *context) {
        // Pass args to distribution_thread function
        return static_cast<MPIFarmManager*>(context)->thread_function2(context);
    }

    void run_until_finish() {
//        int i = 0;
//        while (!i) {
//            sleep(5);
//        }

        int size = world->size();
        if (world->rank() == 0) {
            // Master Process that manages the farm
            cout << "Master says hello" << endl;
            cout << "Number of workers: " << num_workers << endl;

            // Wait for response from collector
            while (true) {
                string response;
                world->recv(2, 0, response);
                cout << "Master received response from collector: " << response << endl;
                if (response == "EOS") {
                    cout << "Master done" << endl;
                    break;
                }
            }
        } else if (world->rank() == 1) {
            while(true) {
                string task = emitter_node->run("");
                if (task == "EOS") {
                    cout << "Emitter done" << endl;

//                     Send EOS to workers
                    for (int i = 0; i < size - 3; i++) {
                        world->send(i + 3, 0, string("EOS"));
                        cout << "Sending worker " << to_string(i + 3) << " EOS" << endl;
                    }

                    break;
                }
                cout << "Sending task " << task << " to worker " << to_string(rr_index + 3) << endl;
                // Distribute task to worker
                world->send(rr_index + 3, 0, task);
                rr_index = (rr_index + 1) % (size - 3);
            }
        } else if (world->rank() == 2) {
            int collector_eos_count = 0;
            while(true) {
                string result;
                world->recv(mpi::any_source, 0, result);

                if (result == "EOS") {
                    collector_eos_count++;

                    cout << "Collector got " << to_string(collector_eos_count) << " EOS counts out of " << to_string(num_workers) << endl;

                    if (collector_eos_count == num_workers) {
                        cout << "Collector done" << endl;

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
        } else {
            Node *worker_node = worker_nodes[world->rank() - 3];

            if (worker_node->get_is_farm()) {
                worker_node->set_nested_node(true);
                // Pass the worker node to the distribution_thread

                worker_node->start_node();
                pthread_t result_collector_thread;

                // Wait two seconds for the distribution_thread to start
//                sleep(2);
                pthread_create(&result_collector_thread, nullptr, thread_function_helper2, this);

                while(true) {
                    string task;
                    world->recv(1, 0, task);

                    cout << "Emitter sent task " << task << " to worker " << world->rank() << endl;
                    if (task == "EOS") {
                        cout << "Worker " << world->rank() << " done" << endl;

                        // Push EOS to worker node
                        // TODO: Clean
                        worker_node->get_input_queue_string()->push("EOS");

                        break;
                    }

                    cout << "MPI Worker " << world->rank() << " received task " << task << endl;

                    // Send task to farm
                    worker_node->run(task);
                }
                pthread_join(result_collector_thread, nullptr);

                worker_node->join_node();
            } else {
                while(true) {
                    string task;
                    world->recv(1, 0, task);
                    if (task == "EOS") {

                        // Push EOS to worker node
                        // TODO: Clean
                        worker_node->get_input_queue_string()->push("EOS");
                        worker_node->get_input_queue()->push(nullptr);

                        worker_node->join_node();

                        // Send EOS to collector
                        world->send(2, 0, string("EOS"));

                        break;
                    }
                    // Send task to farm
                    worker_node->run(task);

                    // Send result to collector
//                    world->send(2, 0, result);
                }
            }
        }
    }

    string run(string task) {
        return "";
    }
};

#endif //PPL_MPIFARMMANAGER_HPP
