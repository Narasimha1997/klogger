#include <sys/sysinfo.h>
#include <time.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <sys_uptime.h>

static inline void check_for_errros(int code, std::string message) {
    if(code < 0) {
        std::cout<<message<<std::endl;
        exit(0);
    }
}

time_t get_current_time() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t( now );
}

time_t get_uptime() {
    struct sysinfo info;
    check_for_errros(sysinfo(&info), "Failed to get system uptime");
    return info.uptime;
}

float get_system_boot_time() {
    time_t uptime = get_uptime();
    time_t current_time = get_current_time();
    return (float)current_time - (float)uptime;
}

