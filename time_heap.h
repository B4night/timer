#ifndef __TIME_HEAP_H_
#define __TIME_HEAP_H_

#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <algorithm>
#include <functional>

class heap_timer {
public:
    heap_timer(int delay, void (*func)(void*), void* argument) : cb_func(func), arg(argument) {
        expire = time(NULL) + delay;
    }

    // bool operator<(const heap_timer& h) {
    //     return this->expire < h.expire;
    // }

public:
    time_t expire;
    void (*cb_func)(void*);
    void* arg;
};

bool operator< (const heap_timer& h1, const heap_timer& h2) {
    return h1.expire > h2.expire;
}

class time_heap {
private:
    int cur_size;
    std::priority_queue<heap_timer, std::vector<heap_timer>, std::less<heap_timer>> pq;
public:
    time_heap() {}
    void add_timer(int delay, void (*func)(void*), void* arg) {
        pq.push(heap_timer(delay, func, arg));
    }
    void tick() {
        if (pq.empty()) {
            fprintf(stdout, "priority queue is empty\n");
            return;
        }
        auto& tmp = pq.top();
        if (time(NULL) < tmp.expire)
            return;
        
        pq.pop();
        tmp.cb_func(tmp.arg);
    }
};

#endif