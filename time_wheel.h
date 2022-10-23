#ifndef __TIME_WHEEL_H_
#define __TIME_WHEEL_H_

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <algorithm>
#include <list>

class wheel_timer {
public:
    wheel_timer(int rot, int slo, void (*func)(void*), void* argument) : 
    rotation(rot), time_slot(slo), cb_func(func), arg(argument) {}
    bool operator==(const wheel_timer& mt) {
        return this->rotation == mt.rotation &&
                this->time_slot == mt.time_slot &&
                this->cb_func == mt.cb_func && 
                this->arg == mt.arg;
    }
public:
    int rotation;
    int time_slot;
    void (*cb_func)(void*); //call back function
    void* arg;  //cb_func's argument
};

class time_wheel {
private:
    static const int slot_num = 60;
    static const int si = 1;   //slot interval: 1 second
    std::list<wheel_timer>* slots[time_wheel::slot_num];
    int cur_slot;
public:
    time_wheel() {
        cur_slot = 0;
        for (int i = 0; i < time_wheel::slot_num; i++)
            slots[i] = new std::list<wheel_timer>();
    }
    
    ~time_wheel() {
        for (int i = 0; i < time_wheel::slot_num; i++) {
            slots[i]->~list();
            // delete slots[i];
        }
    }

    bool add_timer(int timeout, void (*func)(void*), void* arg) {
        if (timeout <= 0)
            return false;
        int ticks = 0;
        if (timeout < time_wheel::si)
            timeout = time_wheel::si;
        ticks = timeout / time_wheel::si;
        int rotation = ticks / time_wheel::slot_num;
        int ts = (cur_slot + (ticks % time_wheel::slot_num)) % time_wheel::slot_num;  //target slot
        slots[ts]->push_front(wheel_timer(rotation, ts, func, arg));
        return true;
    }

    void del_timer(const wheel_timer& timer) {
        int ts = timer.time_slot;
        auto iter = std::find(slots[ts]->begin(), slots[ts]->end(), timer);
        slots[ts]->erase(iter);
    }

    void tick() {
        // everytime passed a si, call tick()
        fprintf(stdout, "current slot is %d\n", cur_slot);
        // fprintf(stdout, "this slots' size is %d\n", slots[cur_slot]->size());
        for (auto iter = slots[cur_slot]->begin(); iter != slots[cur_slot]->end(); ) {
            auto& elem = *iter;
            if (elem.rotation > 0) {
                elem.rotation--;
                iter++;
                continue;
            }
            elem.cb_func(elem.arg);
            iter++;
            del_timer(elem);
        }
        cur_slot++;
        cur_slot %= time_wheel::slot_num;
    }

};

#endif