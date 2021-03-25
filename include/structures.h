#include <config.h>

#include <stdlib.h>
#include <sys/queue.h>

//received message
typedef struct _r_message {
    char type; //0 = data; 1 = control
    int source_router_id;
    int destination_router_id;
    char payload[BUF_LEN];

} r_message;

typedef struct _message_queue {
    r_message item;
    TAILQ_ENTRY(_message_queue) entries;
} message_queue;
