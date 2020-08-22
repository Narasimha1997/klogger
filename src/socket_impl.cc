#include <iostream>
#include <stdlib.h>

#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include <socket_impl.h>

static inline void handleError(int code, std::string message) {
    if(code < 0) {
        std::cerr<<message<<std::endl;
        exit(0);
    }
}

LogBroadcaster::LogBroadcaster(const char * address, int port) {
    //Create a UDP socket to stream data
    this->socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    handleError(this->socketfd, "Socket creating failed"); 

    memset(&this->address_memory, 0, sizeof(this->address_memory));

    //set UDP parameters:

    this->address_memory.sin_family = AF_INET;
    this->address_memory.sin_port = htons(port);
    this->address_memory.sin_addr.s_addr = INADDR_ANY;

    std::cout<<"UDP Streaming enabled."<<std::endl;
}

void LogBroadcaster::broadcast_data(logs_t * log_data, int start_index) {
    std::vector<syslog_entry> * entries_pointer = &log_data->entries;

    std::string broadcast_data = "{\"entries\" : [";

    for(int idx = start_index; idx < entries_pointer->size(); idx++) {
        syslog_entry * entry = &entries_pointer->at(idx);
        broadcast_data += "{\"timestamp\" : " + std::to_string(entry->timestamp) + 
            ", \"level\" : \"" + std::to_string(entry->log_level) +", \"message\":" + entry->data +
            "\"},";
    }

    broadcast_data.pop_back();
    broadcast_data += "]}";
    size_t length = broadcast_data.length();

    //send data
    int result = sendto(
            this->socketfd, 
            broadcast_data.c_str(), 
            length, 
            MSG_CONFIRM, (const sockaddr *)&this->address_memory, 
            sizeof(this->address_memory)
        );
    
    handleError(result, "Cant send message");
}