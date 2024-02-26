#ifndef PPL_MPIFARMMANAGER_HPP
#define PPL_MPIFARMMANAGER_HPP

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <vector>
#include <string>

namespace mpi = boost::mpi;
using namespace std;

class MPIFarmManager {
public:
    MPIFarmManager(mpi::environment *env, mpi::communicator *world) : env(env), world(world) {}

    void set_emitter_callback(string (*emitter_callback)(string)) {
        this->emitter_callback = emitter_callback;
    }

    void set_worker_callback(string (*worker_callback)(string)) {
        this->worker_callback = worker_callback;
    }

    void set_collector_callback(string (*collector_callback)(string)) {
        this->collector_callback = collector_callback;
    }

    void run() {
        // Number of workers is world size minus 3 (master, emitter, collector)
        num_workers = world->size() - 3;
        rr_index = 0;

        if (world->rank() == 0) {
            // Master Process that manages the farm

        } else if (world->rank() == 1) {
            // Emitter Process
            while (true) {
                string task = emitter_callback("");
                if (task == "EOS") {
                    break;
                }

                // Send to worker
                world->send(rr_index + 3, 0, task);
                rr_index = (rr_index + 1) % num_workers;
            }
        } else if (world->rank() == 2) {
            // Collector Process
            while (true) {
                string result;
                world->recv(mpi::any_source, 0, result);
                if (result == "EOS") {
                    break;
                }
//                collector_callback(result);
                cout << "Collector: " << result << endl;
            }
        } else {
            // Worker Loop
            while (true) {
                string task;
                world->recv(0, 0, task);
                if (task == "EOS") {
                    break;
                }
                string result = worker_callback(task);
                world->send(2, 0, result);
            }
        }
    }

private:
    mpi::environment *env;
    mpi::communicator *world;

    // Worker function callbacks
    string (*emitter_callback)(string);
    string (*worker_callback)(string);
    string (*collector_callback)(string);

    // Scheduling state
    int num_workers;
    int rr_index;
};

#endif //PPL_MPIFARMMANAGER_HPP
