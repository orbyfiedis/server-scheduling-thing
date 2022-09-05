//
// Created by atomf on 9/5/2022.
//

#include <iostream>

#include "server.h"

ServerBase::ServerBase() {
    this->scheduler = new ServerScheduler();

    // set default ups
    this->set_target_ups(-1.0f);
}

ServerBase* ServerBase::set_target_ups(float ups) {
    if (ups > 0) {
        this->_target_ups = ups;
        this->_target_t = duration<float>(1.0f / ups);
        this->_target_tc = _target_t.count();
    } else {
        this->_target_ups = -1;
    }
    return this;
}

ServerBase* ServerBase::set_active(bool b) {
    _active = b;
    return this;
}

[[nodiscard]] float ServerBase::get_target_ups() const {
    return this->_target_ups;
}

[[nodiscard]] std::thread* ServerBase::get_thread() const {
    return this->_thread;
}

/**
 * Will set up the server.
 */
void ServerBase::setup() {

}

// WRAPPER: static wrapper for ServerBase::run()
static void w_run(ServerBase* server) {
    server->run();
}

/**
 * The run function for the server thread.
 * This will contain the main update loop
 * of the server. Very nice.
 */
void ServerBase::run() {
    using namespace std::chrono;

    // time control
    auto tp1 = Clock::now();
    auto tp2 = Clock::now();
    float fElapsedTime;
    float fPeriodTime;

    // main loop
    while (this->_active) {

        /* ----------------- */
        /* Timings           */
        /* ----------------- */

        bool syncUps = _target_ups != -1;

        // timings
        duration<float> dElapsed = tp2 - tp1;
        fElapsedTime = dElapsed.count();
        this->elapsedTime = fElapsedTime;
        tp1 = Clock::now();

        fPeriodTime = m_max(elapsedTime, _target_tc);
        periodTime  = fPeriodTime;

        if (syncUps) {
            // wait for next tick
            duration<float> w_dur = _target_t - dElapsed;
            if (w_dur.count() > 0)
                std::this_thread::sleep_for(w_dur);

            // update ups counter
            this->ups = m_min(1.0f / fElapsedTime, _target_ups);
        } else {
            // update ups counter
            this->ups = 1.0f / fElapsedTime;
        }

        /* ----------------- */
        /* Scheduling        */
        /* ----------------- */

        // update scheduler
        scheduler->update_tick(fElapsedTime, fPeriodTime);

        /* ----------------- */
        /* Tick Tail         */
        /* ----------------- */

 #ifdef TEST_DECREASE_PERFORMANCE
        {
            double a = 1000000000000;
            for (int i = 0; i < TEST_DECREASE_PERFORMANCE; i++)
                a /= 5;
            if (ticksElapsed % 10000 == 0) {
                std::cout << a << std::endl;
            }
        }
#endif

        /* ----------------- */
        /* Debug             */
        /* ----------------- */

        if (ticksElapsed % 1000000 == 0)
            std::cout << "Tick | elapsed: " << fElapsedTime << ", ups: " << ups << std::endl;

        /* ----------------- */
        /* Timings           */
        /* ----------------- */

        // register end of last tick
        tp2 = Clock::now();

        // increment tick
        ticksElapsed++;

    }
}

/**
 * Creates the server thread starting the server.
 */
void ServerBase::start() {
    _thread = new std::thread(w_run, this);
}
