#ifndef __SOCKET_IMPL

#define __SOCKET_IMPL

#include <iostream>

#include <log_collector.h>

class LogBroadcaster {
    int socketfd;
    struct sockaddr_in address_memory;

    public :
        LogBroadcaster(const char * host, int port);
        void broadcast_data(logs_t * log, int index_start);
};

#endif