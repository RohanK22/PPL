
#ifndef PPL_MPIPIPELINEMANAGER_HPP
#define PPL_MPIPIPELINEMANAGER_HPP

#include "MPINode.hpp"
#include <boost/mpi.hpp>
#include "Node.hpp"

using namespace std;
namespace mpi = boost::mpi;

// TODO: Might want to drop the "Manager" suffix
class MPIPipelineManager : public Node {
private:
    mpi::environment *env;
    mpi::communicator *world;

    // Stages
    vector<Node*> pipline_nodes;

    int num_pipeline_stages = 0;

public:
    MPIPipelineManager(mpi::environment *env, mpi::communicator *world) : env(env), world(world) {}

    void add_pipeline_node(Node *node) {
        this->pipline_nodes.push_back(node);
        this->num_pipeline_stages++;
    }

    int get_num_pipeline_stages() {
        return this->num_pipeline_stages;
    }

    string run(string _) override {
        int size = world->size();
        int first_pipeline_node = 1;
        int last_pipeline_node = first_pipeline_node + num_pipeline_stages - 1;

        // Master waits for EOS from last pipeline node
        if (world->rank() == 0) {
            cout << "Master says hello" << endl;

            // Wait for response from collector
            while (true) {
                string response;
                world->recv(last_pipeline_node, 0, response);
                cout << "Master received response from collector: " << response << endl;
                if (response == "EOS") {
                    cout << "Master done" << endl;
                    break;
                }
            }
        } else if (world->rank() == first_pipeline_node) {
            // Emitter node
            cout << "Emitter says hello" << endl;

            while (true) {
                // Run the emitter node
                string task = pipline_nodes[0]->run("");
                if (task == "EOS") {
                    cout << "Emitter done" << endl;

                    // EOS to next stage
                    world->send(first_pipeline_node + 1, 0, string("EOS"));
                    break;
                }

                // Send task to next stage
                world->send(first_pipeline_node + 1, 0, task);
            }
        } else if (world->rank() == last_pipeline_node) {
            // Collector node
            cout << "Collector says hello" << endl;

            while (true) {
                string task;
                world->recv(last_pipeline_node - 1, 0, task);
                if (task == "EOS") {
                    cout << "Collector done" << endl;
                    world->send(0, 0, string("EOS"));
                    break;
                }

                // Run the collector node
                string result = pipline_nodes[num_pipeline_stages - 1]->run(task);
                world->send(0, 0, result);
            }
        } else {
            // Worker nodes
            cout << "Worker " << world->rank() << " says hello" << endl;
            while (true) {
                string task;
                world->recv(world->rank() - 1, 0, task);
                if (task == "EOS") {
                    cout << "Worker " << world->rank() << " done" << endl;

                    // EOS to next stage
                    world->send(world->rank() + 1, 0, string("EOS"));
                    break;
                }

                // Run the worker node
                string result = pipline_nodes[world->rank() - first_pipeline_node]->run(task);
                world->send(world->rank() + 1, 0, result);
            }
        }
    }
};

#endif //PPL_MPIPIPELINEMANAGER_HPP
