#ifndef __LOG_COLLECTOR
#define __LOG_COLLECTOR

#include<iostream>
#include<vector>

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

typedef struct logdata {
    int length;
    char * buffer_region;
} syslog_t;

typedef struct log_entry {
    float timestamp;
    int log_level;
    std::string data;
} syslog_entry;


typedef struct processed_logs {
    int log_length;
    int actual_length;
    bool sync_lock;
    std::vector<syslog_entry> entries;
} logs_t;

logs_t read_kernel_log(float * boot_stamp);
void read_log_changes(logs_t * current_logs, float * boot_stamp, int * length_difference);

#endif