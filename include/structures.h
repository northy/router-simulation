#include <config.h>

//received message
typedef struct {
    char type; //0 = data; 1 = control
    int source_router_id;
    int destination_router_id;
    char payload[BUF_LEN];

} r_message;
