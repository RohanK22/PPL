A Parallel Patterns Library that provides parallelism through farm and pipeline patterns at the node and distributed level.

# Parallel Patterns
Parallel Patterns, also known as algorithmic skeletons, offer a structured way to represent common forms of parallel computation, communication, and interaction. 
In essense they are reusable templates to structure parallel code.

# Node-level Parallelism
Node is a MIMD machine or a multicore machine.
Node-level parallelism is achieved though the shared memory programming model using POSIX threads or pthreads.

# Distributed-level Parallelism
Distributed-level refers to a cluster of MIMD machine this library targets.
Distributed-level parallelism is achieved though message passing programming model through the use of BoostMPI.
