
#ifndef PPL_QUEUE_HPP
#define PPL_QUEUE_HPP

#include <iostream>
#include <pthread.h>
#include <queue>

template <typename T>
class Queue {
private:
    std::queue<T> q;
    pthread_mutex_t mutex;
    pthread_cond_t non_empty_queue_cond;
public:
    Queue() {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&non_empty_queue_cond, nullptr);
    }

    void push(T value) {
        pthread_mutex_lock(&mutex);
        bool wasEmpty = q.empty();
        q.push(value);
        if (wasEmpty) {
            pthread_cond_signal(&non_empty_queue_cond); // Signaling the condition variable
        }
        pthread_mutex_unlock(&mutex);
    }

    T pop() {
        pthread_mutex_lock(&mutex);
        while (q.empty()) {
            // Wait for
            pthread_cond_wait(&non_empty_queue_cond, &mutex);
        }
        T popped_value = q.front();
        q.pop();
        pthread_mutex_unlock(&mutex);
        return popped_value;
    }

    // For a non-blocking pop, we can use the following function
    bool try_pop(T &popped_value) {
        pthread_mutex_lock(&mutex);
        if (q.empty()) {
            pthread_mutex_unlock(&mutex);
            return false;
        }
        popped_value = q.front();
        q.pop();
        pthread_mutex_unlock(&mutex);
        return true;
    }

    int size() {
        pthread_mutex_lock(&mutex);
        int size = q.size();
        pthread_mutex_unlock(&mutex);
        return size;
    }

    ~Queue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&non_empty_queue_cond); // Destroying the renamed condition variable
    }
};
#endif //PPL_QUEUE_HPP