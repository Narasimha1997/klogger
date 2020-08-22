#include <iostream>
#include <sys/klog.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "log_collector.h"

static inline void check_for_errros(int code, std::string message) {
    if(code < 0) {
        std::cout<<message<<std::endl;
        exit(0);
    }
}

void log_parser_with_append(std::vector<syslog_entry> * logs, syslog_t * buffer, float * boot_stamp, int offset) {
    bool isNewLine = true;
    char * buffer_data = buffer->buffer_region;
    buffer_data = buffer_data + offset;
    
    while(*buffer_data != '\0'){
        if(*buffer_data == '<' && isNewLine) {

            syslog_entry new_entry;

            //parse the log level:
            buffer_data ++;
            std::string log_level = "";
            while(*buffer_data != '>') {
                log_level.push_back(*buffer_data);
                buffer_data ++;
            }

            buffer_data ++;
            if(*buffer_data == '[') {
                buffer_data ++;
                std::string timestring = "";
                while(*buffer_data != ']') { 
                    if(*buffer_data == ' ') {
                        buffer_data ++;
                        continue;
                    }
                    timestring.push_back(*buffer_data);
                    buffer_data ++;
                }

                float timestamp = std::stof(timestring);
                timestamp = * boot_stamp + timestamp;
                new_entry.timestamp = timestamp;
            }

            buffer_data ++;

            //parse the content
            //skip initial spaces:
            while(*buffer_data == ' ') buffer_data ++;

            while(*buffer_data != '\n') {
                new_entry.data.push_back(*buffer_data);
                buffer_data ++;
            }

            buffer_data ++;
            //add the entry to vector
            new_entry.log_level = std::stoi(log_level);
            logs->push_back(new_entry);
        }
    }
}

std::vector<syslog_entry> log_parser(syslog_t * buffer, float * boot_stamp) {
    std::vector<syslog_entry> logs;
    bool isNewLine = true;
    char * buffer_data = buffer->buffer_region;
    while(*buffer_data != '\0'){
        if(*buffer_data == '<' && isNewLine) {

            syslog_entry new_entry;

            //parse the log level:
            buffer_data ++;
            std::string log_level = "";
            while(*buffer_data != '>') {
                log_level.push_back(*buffer_data);
                buffer_data ++;
            }

            buffer_data ++;
            if(*buffer_data == '[') {
                buffer_data ++;
                std::string timestring = "";
                while(*buffer_data != ']') { 
                    if(*buffer_data == ' ') {
                        buffer_data ++;
                        continue;
                    }
                    timestring.push_back(*buffer_data);
                    buffer_data ++;
                }

                float timestamp = std::stof(timestring);
                timestamp = * boot_stamp + timestamp;
                new_entry.timestamp = timestamp;
            }

            buffer_data ++;

            //parse the content
            //skip initial spaces:
            while(*buffer_data == ' ') buffer_data ++;

            while(*buffer_data != '\n') {
                new_entry.data.push_back(*buffer_data);
                buffer_data ++;
            }

            buffer_data ++;
            //add the entry to vector
            new_entry.log_level = std::stoi(log_level);
            logs.push_back(new_entry);
        }
    }

    return logs;
}

syslog_t * allocate_buffer_memory() {
    int buffer_size = klogctl(10, NULL, 0);
    check_for_errros(buffer_size, "Failed to get buffer size");

    void * buffer = (void *)malloc((buffer_size + 1)* sizeof(char));

    syslog_t * syslog = (syslog_t *)malloc(sizeof(syslog_t));
    syslog->length = buffer_size;
    syslog->buffer_region = (char *)buffer;

    return syslog;
}

void klogctl_read(syslog_t * syslog) {
    char * buffer_ptr = (char *)syslog->buffer_region;
    int result = klogctl(3, buffer_ptr, syslog->length);
    check_for_errros(result, "Failed to collect kernel message logs");
    buffer_ptr[syslog->length] = '\0';
} 

logs_t read_kernel_log(float * boot_stamp) {
    syslog_t * logs = allocate_buffer_memory();
    klogctl_read(logs);
    std::vector<syslog_entry> entries =  log_parser(logs, boot_stamp);
    logs_t log_data;
    log_data.log_length = logs->length;
    log_data.entries = entries;

    log_data.actual_length = strlen(logs->buffer_region);

    free(logs->buffer_region);
    free(logs);

    log_data.sync_lock = false;

    return log_data;
}

syslog_t * allocate_buffer_from_length(int length) {

    //allocate a log-partition

    void * buffer = (void *)malloc((length + 1) * sizeof(char));
    syslog_t * syslog = (syslog_t *)malloc(sizeof(syslog_t));
    syslog->length = length;
    syslog->buffer_region = (char *)buffer;
    return syslog;
}


void read_log_changes(logs_t * current_logs, float * boot_stamp, int * length_difference) {
    //conditional read, for log changes, this reads the length difference and reads only those values
    syslog_t * logs = allocate_buffer_memory();
    klogctl_read(logs);

    bool changed = false;

    int new_logs_length = strlen(logs->buffer_region);
    //compare string lengths and process 
    if(new_logs_length > current_logs->actual_length) {

        //set the mutex lock before modifying the buffer
        current_logs->sync_lock = true;

        int previous_length = current_logs->entries.size();
        
        log_parser_with_append(&current_logs->entries, logs, boot_stamp, current_logs->actual_length);
        current_logs->actual_length = new_logs_length;

        int new_length = current_logs->entries.size();

        * length_difference = new_length - previous_length;
        changed = true;
        
        //release the mutex lock
        current_logs->sync_lock = false;

    } else if (new_logs_length < current_logs->actual_length) {
        //destroy the exisitng buffer and create a new addition
        //This will happen if the kernel ring-buffer has refreshed data.
        
        current_logs->sync_lock = true;
        current_logs->entries.clear();
        log_parser_with_append(&current_logs->entries, logs, boot_stamp, 0);
        current_logs->actual_length = new_logs_length;
        * length_difference = current_logs->entries.size();
        
        changed = true;

        //release the mutex lock
        current_logs->sync_lock = false;
    }

    if(!changed) {
        * length_difference = 0;
    }

    //free the buffers and pointers
    free(logs->buffer_region);
    free(logs);
}
