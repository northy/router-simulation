#pragma once

#include <structures.h>
#include <pthread.h>
#include <semaphore.h>

//exports global variables

//router
extern int router_id;
extern unsigned short router_port;
extern char router_ip[16];

//links
extern char link_cost[MAX_NEIGHBORS];
extern char link_enabled[MAX_NEIGHBORS];
extern int neighbors_c, neighbors[MAX_NEIGHBORS];

//external route
extern int total_router_c;
extern unsigned short external_router_port[MAX_NEIGHBORS];
extern char external_router_ip[MAX_NEIGHBORS][16];

//dv
extern pthread_mutex_t dv_mutex;
extern int dv_source[MAX_NEIGHBORS];
extern char dv_valid[MAX_NEIGHBORS];
extern time_t dv_recv_time[MAX_NEIGHBORS];
extern char distance_vector[MAX_NEIGHBORS][MAX_NEIGHBORS];

//socket
extern int socket_descriptor;

//received_message
extern r_message received_messages[QUEUE_MAX];
extern int received_messages_c;
extern pthread_mutex_t received_messages_mutex;

//received_dv
extern char received_dvs[QUEUE_MAX];
extern int received_dvs_c;
extern pthread_mutex_t received_dvs_mutex;

//queue
extern message_queue *process_queue, *send_queue;
extern pthread_mutex_t process_queue_mutex, send_queue_mutex;
extern TAILQ_HEAD(_message_queue_h, _message_queue) process_queue_head, send_queue_head;
extern int process_queue_c, send_queue_c;

//packet_handler
extern sem_t packet_handler_sem, sender_sem;

//terminal
extern pthread_mutex_t terminal_mutex;
