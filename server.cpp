//
// Created by atomf on 9/5/2022.
//

#include <iostream>

#include "lua-util.h"
#include "server.h"

/*
 * Server Core
 */

ServerBase::ServerBase() {
    this->scheduler = new ServerScheduler();

    // set default ups
    this->set_target_ups(60.0);
}

ServerBase* ServerBase::set_target_ups(float ups) {
    if (ups > 0) {
        this->_target_ups = ups;
        this->_target_t = duration<float>(1.0f / ups);
        this->_target_tc = _target_t.count();

        this->debug_updates_per_log = _target_ups * 10;
    } else {
        this->_target_ups = -1;

        this->debug_updates_per_log = -1;
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

static int L_Server_set_ups(lua_State* L) {
    ServerBase* server = ll_self<ServerBase>(L, "Server");
    server->set_target_ups(lua_tonumber(L, 2));
    return 0;
}

static int L_Server_get_scheduler(lua_State* L) {
    ServerBase* server = ll_self<ServerBase>(L, "Server");
    lua_pushlightuserdata(L, (void*) server->scheduler);
    ll_setmtc(L, "ServerScheduler");
    return 1;
}

static int L_ServerScheduler_add_tick_timer(lua_State* L) {
    ServerScheduler* scheduler = ll_self<ServerScheduler>(L, "ServerScheduler");

    if (!lua_isstring(L, 2) || !lua_isnumber(L, 3))
        return 0;

    std::string name = std::string(lua_tostring(L, 2));
    float tl = lua_tonumber(L, 3);
    Timer* t = scheduler->create_sync_tick_timer(name)
            ->set_time_left(tl)
            ->set_active(true);

    t->on_complete([&] {
        char *name = nullptr;
        sprintf(name, "STT_complete_%s", name);
        lua_getglobal(L, name);
        if (lua_isfunction(L, -1)) {
            lua_pcall(L, 0, 0, 0);
        }
    });

    // return timer reference
    ll_pushptr(L, t);
    return 1;
}

static int L_Server_print_tick_debug(lua_State* L) {
    ll_self<ServerBase>(L, "Server")->print_tick_debug();
    return 0;
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

void ServerBase::print_tick_debug() {
    std::cout << "Tick Debug" << std::endl;
}

/**
 * The run function for the server thread.
 * This will contain the main update loop
 * of the server. Very nice.
 */
void ServerBase::run() {
    using namespace std::chrono;

    // create lua
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    ll_createmtc(L, "Server"); // ServerBase
    ll_setcfunc(L, "set_target_ups", L_Server_set_ups); // ServerBase->set_target_ups
    ll_setcfunc(L, "scheduler", L_Server_get_scheduler); // ServerBase->scheduler
    ll_setcfunc(L, "print_tick_debug", L_Server_print_tick_debug);

    ll_createmtc(L, "ServerScheduler");
    ll_setcfunc(L, "add_tick_timer", L_ServerScheduler_add_tick_timer); // ServerScheduler->add_sync_tick_timer

    lua_pushlightuserdata(L, (void*) this); // server table
    luaL_getmetatable(L, "Server");
    lua_setmetatable(L, -2);

    lua_setglobal(L, "server");

    // execute init file
    if (checklua(L, luaL_dofile(L, "../init.lua"))) {
        std::cout << "successfully executed init.lua" << std::endl;
    }

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

        lua_getglobal(L, "server_tick");
        lua_pcall(L, 0, 0, 0);

 #ifdef TEST_DECREASE_PERFORMANCE
        {
            double a = 1000000000000;
            for (int i = 0; i < TEST_DECREASE_PERFORMANCE; i++)
                a /= 5;
            if (ticksElapsed % debug_updates_per_log == 0) {
                std::cout << a << std::endl;
            }
        }
#endif

        /* ----------------- */
        /* Debug             */
        /* ----------------- */

        int dupl = debug_updates_per_log;
        if (dupl == -1)
            dupl = 1000000;

        if (ticksElapsed % dupl == 0) {
            std::cout
                << "DEBUG: Tick | dtElapsed: " << fElapsedTime
                << ", dtPeriod: " << fPeriodTime
                << ", ups: " << ups
                << std::endl;
            std::cout
                << "DEBUG: Tick:Scheduler | syncTickTimers: " << scheduler->get_sync_tick_timers().size()
                << ", syncRealTimers: " << scheduler->get_sync_real_timers().size()
                << std::endl;
        }

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
