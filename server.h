//
// Created by atomf on 9/5/2022.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>

/* -------------------------- */
/* Server                     */
/* -------------------------- */

#include "server-scheduling.h"

using namespace std::chrono;

class ServerBase {
private:
    // target ups
    float           _target_ups = 60.0f;
    duration<float> _target_t   = duration<float>(0);
    float           _target_tc  = -1;

    // threading
    std::atomic_bool _active = std::atomic_bool();
    std::thread*     _thread = nullptr;

public:
    // scheduler
    ServerScheduler* scheduler = nullptr;

    // ticks
    unsigned long long ticksElapsed = 0;
    float elapsedTime = -1;
    float ups         = -1;

    /**
     * Constructor.
     * Sets up the core.
     */
    ServerBase();

    // basic getters and setters
    ServerBase* set_target_ups(float ups);
    ServerBase* set_active(bool b);
    [[nodiscard]] float get_target_ups() const;
    [[nodiscard]] std::thread* get_thread() const;

    /**
     * Will set up the server.
     */
    void setup();

    /**
     * The run function for the server thread.
     * This will contain the main update loop
     * of the server. Very nice.
     */
    void run();

    /**
     * Creates the server thread starting the server.
     */
    void start();
};

#endif //SERVER_SERVER_H
