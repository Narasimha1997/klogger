#include <iostream>
#include <vector>
#include <stdio.h>
#include <log_collector.h>
#include <sys_uptime.h>
#include <socket_impl.h>
#include<unistd.h>

#include <string.h>

int main(int argc, char **argv) {

    if(argc < 3) {
        std::cout<<"Provide server address and server port to trasfer data to"<<std::endl;
        exit(0);
    }

   float timestamp = get_system_boot_time();
   //printf("Time = %f\n", timestamp);

   logs_t logs = read_kernel_log(&timestamp);
   /*for(syslog_entry entry : logs.entries) {
       printf(
           "Timestamp = %f Message = %s\n",
           entry.timestamp, entry.data.c_str()
       );
   }*/

   const char * host_address = argv[1];
   int port = atoi(argv[2]);

   int changed_length = 0;
   int previous_length = logs.entries.size();

   LogBroadcaster * socketProducer = new LogBroadcaster(host_address, 5555);
   //socketProducer->broadcast_data(&logs, 0);

   while(true) {
    
       read_log_changes(&logs, &timestamp, &changed_length);
       if(changed_length != 0) {
           std::cout<<"Logs changed, new entries added = " << changed_length << std::endl;
           socketProducer->broadcast_data(&logs, previous_length);

           previous_length = logs.entries.size();

       }
       sleep(2);
   }

}