#include <config.h>

//received message
typedef struct {
    char type; //0 = data; 1 = control
    char source_router_ip[16];
    unsigned short source_router_port;
    char destination_router_ip[16];
    unsigned short destination_router_port;
    char payload[BUF_LEN];

} r_message;
