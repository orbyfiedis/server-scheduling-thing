#include "server.h"

/**
 * Global server instance.
 */
static ServerBase* G_server;

//////////////////////////////////////////

int main(int argc, char** argv) {

    G_server = new ServerBase();
    G_server->setup();
    G_server->set_active(true)->start();

    G_server->get_thread()->join();

};