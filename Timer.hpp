//
// Created by rk76 on 23/11/23.
//

#ifndef PPL_TIMER_HPP
#define PPL_TIMER_HPP

// A distribution_thread safe implementation of a timer
// Should support: start, stop, reset, get_elapsed_time
// Should be able to start and stop multiple times, and it should aggregate the elapsed time

#include <chrono>
#include <pthread.h>

using namespace std::chrono;

class Timer {
private:
    steady_clock::time_point start_time;
    steady_clock::time_point stop_time;
    bool is_running;
    duration<double> elapsed_time;
    pthread_mutex_t timer_mutex;

public:
    Timer() {
        pthread_mutex_init(&timer_mutex, nullptr);
        is_running = false;
    }

    void start() {
        pthread_mutex_lock(&timer_mutex);
        if (!is_running) {
            start_time = steady_clock::now();
            is_running = true;
        }
        pthread_mutex_unlock(&timer_mutex);
    }

    void stop() {
        pthread_mutex_lock(&timer_mutex);
        if (is_running) {
            stop_time = steady_clock::now();
            elapsed_time += stop_time - start_time;
            is_running = false;
        }
        pthread_mutex_unlock(&timer_mutex);
    }

    void reset() {
        pthread_mutex_lock(&timer_mutex);
        elapsed_time = duration<double>::zero();
        is_running = false;
        pthread_mutex_unlock(&timer_mutex);
    }

    double get_elapsed_time() {
        pthread_mutex_lock(&timer_mutex);
        if (is_running) {
            stop_time = steady_clock::now();
            elapsed_time += stop_time - start_time;
            start_time = stop_time;
        }
        pthread_mutex_unlock(&timer_mutex);
        return elapsed_time.count();
    }
};

#endif //PPL_TIMER_HPP
