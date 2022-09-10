//
// Created by atomf on 9/5/2022.
//

#ifndef SERVER_SERVER_SCHEDULING_H
#define SERVER_SERVER_SCHEDULING_H

#include "utilh.h"

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>

/* -------------------------- */
/* Scheduling                 */
/* -------------------------- */

/**
 * An object which counts down until it
 * reaches zero, and then executes an operation.
 */
class Timer {
private:
    // the name of this timer
    std::string& _name;
    // the time left
    float _time     = -1.0f;
    // is it currently enabled
    bool _active   = false;
    // has is completed
    bool _complete = false;

    // functions
    std::function<void()> _on_complete = nullptr;
    std::function<void()> _on_tick     = nullptr;

public:
    explicit Timer(std::string& name) : _name(name) { }

    // getters
    std::string& name();
    [[nodiscard]] bool is_complete() const;
    [[nodiscard]] bool is_active() const;

    // modify
    Timer* set_time_left(float f);
    Timer* set_active(bool b);
    Timer* on_tick(std::function<void()> func);
    Timer* on_complete(std::function<void()> func);

    // tick
    void tick(float elapsed);

};

/**
 * Object for scheduling tasks and timers
 * on the server. Primarily tick based.
 */
class ServerScheduler {
private:
    // synchronous timers
    std::vector<Timer*> _sync_tick_timers = std::vector<Timer*>();
    std::vector<Timer*> _sync_rt_timers   = std::vector<Timer*>();

    // TODO: asynchronous timers
    std::vector<Timer*> _async_tick_timers = std::vector<Timer*>();
    std::vector<Timer*> _async_rt_timers   = std::vector<Timer*>();

    // run next tick
    std::vector<std::function<void()>>* _run_next_tick = new std::vector<std::function<void()>>();
    std::vector<std::function<void()>>* _run_this_tick = new std::vector<std::function<void()>>();

public:
    // timers
    Timer* create_sync_tick_timer(std::string name);
    Timer* create_sync_real_timer(std::string name);
    std::vector<Timer*>& get_sync_tick_timers();
    std::vector<Timer*>& get_sync_real_timers();

    // run next or this tick
    ServerScheduler* run_soon(std::function<void()> func);
    ServerScheduler* run_next_tick(std::function<void()> func);

    // tick
    void update_tick(float fElapsedTime, float fPeriodTime);
};

#endif //SERVER_SERVER_SCHEDULING_H
