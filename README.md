### KLogger
A C++ CLI tool for linux that streams kernel logs and events over a light-weight UDP socket to remote machines. 
You can use this tool for debugging while developing Linux kernel extensions and custom Linux kernel modules, especially for embedded systems that don't have built-in display. (Cross compile for aarch64 and others).

#### Architecture
The logger is simple in-design and implements a polling mechanism over `/dev/kmsg` device. It maintains a built-in table that stores timestamp ordered log data. The changes are appended to the table every 2 seconds. The table is then referenced by the broadcaster which transfers the data to the server.

#### Compiling:
We have provided a makefile that can compile and build the klogger.
```
make all
```

#### Running KLogger:
You can run KLogger by executing klogger binary and providing UDP server address and port.
```
klogger server_address server_port
```

Example:
```
klogger 192.168.0.112 7777
```

#### Writing your own produces
If you don't want UDP producer, then you can write your own producer module. Look at `src/socket_impl.h`

```C++
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
```

You can write a custom implementation for `broadcast_data` and the `constructor` to add custom producer.
Parameters:
1. `logs_t * log` : This is the reference pointer to kernel log table. This is a custom data-type.
```C++
typedef struct processed_logs {
    int log_length;      // length of log buffer
    int actual_length;   // Length of the string data
    bool sync_lock;      // mutex lock, if it is true, wait for it to be false
    std::vector<syslog_entry> entries; // The table which contains logs
} logs_t;
```

The log table entry `syslog_entry` is shown below:
```C++
typedef struct log_entry {
    float timestamp; //timestamp of the event
    int log_level;      //level of log
    std::string data;   // log data
} syslog_entry;
```

2. index_start : This integer is an index that tells the last log entry in the table from where the data has been newly added. You can optionally process table entries from this index till the last.

#### Running sample test
You can start the python sample UDP server provided in the script
```python3
python test_client.py
```

Then you can run klogger to stream changes to this server.
```
klogger localhost 5555
```