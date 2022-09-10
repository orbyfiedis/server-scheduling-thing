//
// Created by atomf on 9/5/2022.
//

#include "server-scheduling.h"

/*
 * ---------------------------
 * Scheduling : Timer
 * ---------------------------
 */

std::string&       Timer::name()              { return _name;}
[[nodiscard]] bool Timer::is_complete() const { return _complete; }
[[nodiscard]] bool Timer::is_active()   const { return _active; }

Timer* Timer::set_time_left(float f) {
    this->_time = f;
    return this;
}

Timer* Timer::set_active(bool b) {
    this->_active = b;
    return this;
}

Timer* Timer::on_tick(std::function<void()> func) {
    this->_on_tick = std::move(func);
    return this;
}

Timer* Timer::on_complete(std::function<void()> func) {
    this->_on_complete = std::move(func);
    return this;
}

void Timer::tick(float elapsed) {
    // check active
    if (!_active)
        return;

    // remove elapsed time
    _time -= elapsed;

    // call on tick
    if (_on_tick != nullptr)
        _on_tick();

    // check complete
    if (_time <= 0) {
        // deactivate and mark
        _active   = false;
        _complete = true;

        // call on complete
        if (_on_complete != nullptr)
            _on_complete();
    }
}

/*
 * ---------------------------
 * Scheduling : ServerScheduler
 * ---------------------------
 */

Timer* ServerScheduler::create_sync_tick_timer(std::string name) {
    auto* timer = new Timer(name);
    _sync_tick_timers.push_back(timer);
    return timer;
}

Timer* ServerScheduler::create_sync_real_timer(std::string name) {
    auto* timer = new Timer(name);
    _sync_rt_timers.push_back(timer);
    return timer;
}

std::vector<Timer*>& ServerScheduler::get_sync_tick_timers() {
    return _sync_tick_timers;
}

std::vector<Timer*>& ServerScheduler::get_sync_real_timers() {
    return _sync_rt_timers;
}

ServerScheduler* ServerScheduler::run_soon(std::function<void()> func) {
    _run_this_tick->push_back(func);
    return this;
}

ServerScheduler* ServerScheduler::run_next_tick(std::function<void()> func) {
    _run_next_tick->push_back(func);
    return this;
}

/**
 * Tick.
 */
void ServerScheduler::update_tick(float fElapsedTime, float fPeriodTime) {
    // update tick timers
    {
        std::vector<Timer*> new_timers;

        unsigned long long l = _sync_tick_timers.size();
        for (int i = 0; i < l; i++) {
            Timer* timer = _sync_tick_timers[i];
            timer->tick(/* 1 tick */ 1.0f);
            if (timer->is_complete()) {
                delete timer;
            } else {
                new_timers.push_back(timer);
            }
        }

        _sync_tick_timers = new_timers;
    }

    // update real time timers
    {
        std::vector<Timer*> new_timers;

        unsigned long long l = _sync_rt_timers.size();
        for (int i = 0; i < l; i++) {
            Timer* timer = _sync_rt_timers[i];
            timer->tick(/* time elapsed */ fPeriodTime);
            if (timer->is_complete()) {
                delete timer;
            } else {
                new_timers.push_back(timer);
            }
        }

        _sync_rt_timers = new_timers;
    }

    // run tick tasks
    for (const auto& f1 : *_run_this_tick) {
        f1();
    }

    _run_this_tick = _run_next_tick;

    for (const auto& f2 : *_run_this_tick) {
        f2();
    }

    // schedule next tick tasks
    _run_next_tick = new std::vector<std::function<void()>>();
}