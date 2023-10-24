#ifndef FARMMANAGER_HPP
#define FARMMANAGER_HPP

#include "Queue.hpp"
#include "task.hpp"
#include "thread_args.hpp"

template <typename T>

class FarmManager {
private:
    Queue<T> *emitter_queue;
    Queue<T> *collector_queue;
    unsigned int num_worker_threads;

public:
    // Constructor
    FarmManager() = default;

    // Constructor with emitter queue and collector queue
    FarmManager(Queue<T> *eq, Queue<T> *cq) {
        this->emitter_queue = eq;
        this->collector_queue = cq;
    }

    // Emitter queue has a tasks, the collector queue collects results from those tasks
    // FarmManager creates a farm of threads and distributes tasks to them
    bool set_emitter_queue(Queue<T> *eq){
        this->emitter_queue = eq;
    }

    bool set_collector_queue(Queue<T> *cq) {
        this->collector_queue = cq;
    }

    Queue<T> *get_emitter_queue() {
        return this->emitter_queue;
    }

    Queue<T> *get_collector_queue() {
        return this->collector_queue;
    }

    // Set worker threads
    void set_num_worker_threads(unsigned int num_worker_threads) {
        this->num_worker_threads = num_worker_threads;
    }

    static void *thread_function(void *args) {
        auto *thread_args1 = (thread_args<T> *) args;
        Queue<T> *eq = thread_args1->get_emitter_queue();
        Queue<T> *cq = thread_args1->get_collector_queue();
        while (!eq->empty()) {
            std::cout << "Thread " << pthread_self() << " received a task " << std::endl;
            T task = eq->wait_and_pop();
            task.run();
            cq->push(task);
            std :: cout << "Thread " << pthread_self() << " finished a task " << std::endl;
        }
    }

    void run() {
        // make a list of threads
        pthread_t threads[num_worker_threads];

        thread_args<T> thread_args1(emitter_queue, collector_queue);

        // create threads
        for (int i = 0; i < num_worker_threads; i++) {
            pthread_create(&threads[i], nullptr, thread_function, (void *) &thread_args1);
        }

        // wait for all threads to finish
        for (int i = 0; i < num_worker_threads; i++) {
            pthread_join(threads[i], nullptr);
        }
    }
};

#endif //FARMMANAGER_HPP