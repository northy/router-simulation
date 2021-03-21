#pragma once

#include <structures.h>
#include <pthread.h>

extern int router_id;
extern unsigned short router_port;
extern char router_ip[16];

extern int link_cost[MAX_NEIGHBORS];
extern int neighbors_c, neighbors[MAX_NEIGHBORS];

extern int total_router_c;
extern unsigned short external_router_port[MAX_NEIGHBORS];
extern char external_router_ip[MAX_NEIGHBORS][16];

extern int socket_descriptor;

extern r_message received_messages[QUEUE_MAX];
extern int received_messages_c;
extern pthread_mutex_t received_messages_mutex;
