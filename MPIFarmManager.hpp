#ifndef PPL_MPIFARMMANAGER_HPP
#define PPL_MPIFARMMANAGER_HPP

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <string>
#include "MPINode.hpp"

namespace mpi = boost::mpi;
using namespace std;

class MPIFarmManager : public MPINode {
public:
    MPIFarmManager(mpi::environment *env, mpi::communicator *world) : env(env), world(world) {}

    void add_emitter(MPINode *node) {
        this->emitter_node = node;
    }

    void add_collector(MPINode *node) {
        this->collector_node = node;
    }

    void add_worker(MPINode *node) {
        this->worker_nodes.push_back(node);
        this->num_workers++;
    }

    string run(string _) override {
        int size = world->size();
        if (world->rank() == 0) {
            // Master Process that manages the farm
            cout << "Master says hello" << endl;
            cout << "Number of workers: " << num_workers << endl;

            // Wait response from collector
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
            while(true) {
                string task;
                world->recv(1, 0, task);
//                cout << "Emitter sent task " << task << " to worker " << world->rank() << endl;
                if (task == "EOS") {
                    cout << "Worker " << world->rank() << " done" << endl;

                    // Send EOS to collector
                    world->send(2, 0, string("EOS"));

                    break;
                }
                string result = worker_nodes[world->rank() - 3]->run(task);
                world->send(2, 0, result);
            }
        }
    }

private:
    mpi::environment *env;
    mpi::communicator *world;

    // Worker function callbacks
    MPINode *emitter_node;
    MPINode *collector_node;
    vector<MPINode*> worker_nodes;

    // Scheduling state
    int num_workers;
    int rr_index = 0;
};

#endif //PPL_MPIFARMMANAGER_HPP
