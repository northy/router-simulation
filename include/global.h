#pragma once

#include <structures.h>
#include <pthread.h>
#include <semaphore.h>

extern int router_id;
extern unsigned short router_port;
extern char router_ip[16];

extern char link_cost[MAX_NEIGHBORS];
extern char link_enabled[MAX_NEIGHBORS];
extern int neighbors_c, neighbors[MAX_NEIGHBORS];

extern int total_router_c;
extern unsigned short external_router_port[MAX_NEIGHBORS];
extern char external_router_ip[MAX_NEIGHBORS][16];

extern pthread_mutex_t dv_mutex;
extern int dv_source[MAX_NEIGHBORS];
extern char dv_valid[MAX_NEIGHBORS];
extern time_t dv_recv_time[MAX_NEIGHBORS];
extern char distance_vector[MAX_NEIGHBORS][MAX_NEIGHBORS];

extern int socket_descriptor;

extern r_message received_messages[QUEUE_MAX];
extern int received_messages_c;
extern pthread_mutex_t received_messages_mutex;

extern message_queue *process_queue, *send_queue;
extern pthread_mutex_t process_queue_mutex, send_queue_mutex;
extern TAILQ_HEAD(_message_queue_h, _message_queue) process_queue_head, send_queue_head;
extern int process_queue_c, send_queue_c;

extern sem_t packet_handler_sem, sender_sem;

extern pthread_mutex_t terminal_mutex;
