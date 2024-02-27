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
public:
    // Default constructor
    MPINode() = default;

    // Virtual run method that represents work done by each process
    virtual string run(string task) = 0;
};

#endif //PPL_MPINODE_HPP
