#ifndef PPL_MPINODE_HPP
#define PPL_MPINODE_HPP

#include <boost/mpi.hpp>
#include <boost/serialization/vector.hpp>
#include <string>

using namespace boost::mpi;
using namespace std;
namespace mpi = boost::mpi;

// Represents a process in the MPI environment
class MPINode {
private:
    int rank;
    int size;
    int rr_index;

    bool is_farm_emitter;

    // MPI environment used for communication
    environment *env;
    communicator *world;

public:
    // Default constructor
    MPINode() = default;

    // Constructor
    MPINode(environment *env, communicator *world) {
        this->rank = world->rank();
        this->size = world->size();
        this->env = env;
        this->world = world;
        this->rr_index = 0;
    }

    // Set the MPI environment
    void set_env(environment *env) {
        this->env = env;
    }

    // Set the MPI communicator
    void set_world(communicator *world) {
        this->world = world;
        this->rank = world->rank();
        this->size = world->size();
    }

    // Virtual run method that represents work done by each process
    virtual string run(string task) = 0;
};

#endif //PPL_MPINODE_HPP
