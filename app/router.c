#include <global.h>
#include <terminal.h>
#include <receiver.h>
#include <sender.h>
#include <packet_handler.h>
#include <parse.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

//initialization of variables
//referenced globally in include/global.h
pthread_t receiver_thread, packet_handler_thread, sender_thread;

int router_id;
unsigned short router_port;
char router_ip[16];

int total_router_c=0;
unsigned short external_router_port[MAX_NEIGHBORS];
char external_router_ip[MAX_NEIGHBORS][16];

int neighbors_c=0, neighbors[MAX_NEIGHBORS];
int link_cost[MAX_NEIGHBORS];

int distance_vector[MAX_NEIGHBORS];

int socket_descriptor;

r_message received_messages[QUEUE_MAX];
int received_messages_c = 0;
pthread_mutex_t received_messages_mutex = PTHREAD_MUTEX_INITIALIZER;

message_queue *process_queue, *send_queue;
pthread_mutex_t process_queue_mutex = PTHREAD_MUTEX_INITIALIZER, send_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
struct _message_queue_h process_queue_head, send_queue_head;

sem_t packet_handler_sem, sender_sem;

int main(int argc, char *argv[]) {
    if (argc<2) { //it is necessary to run the Router with an id
        fprintf(stderr, "Please use \"./Router <id>\"\n");
        exit(1);
    }

    router_id = atoi(argv[1]);

    setvbuf(stdout, NULL, _IONBF, -1); //set the STDOUT buffer to flush immediately

    memset(distance_vector,-1,sizeof(distance_vector));

    distance_vector[router_id] = 0;

    //parse the config files
    parse_link(); //link.config
    parse_router(); //router.config

    //create the UDP socket
    if ((socket_descriptor=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Socket creation error");
        exit(1);
    }

    //bind the socket
    struct sockaddr_in si_me;
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(router_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socket_descriptor, (struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
        perror("Socket bind error");
        exit(1);
    }
    
    //inititate semaphores
    sem_init(&packet_handler_sem, 0, 0);
    sem_init(&sender_sem, 0, 0);

    //initiate queues
    TAILQ_INIT(&process_queue_head);
    TAILQ_INIT(&send_queue_head);

    //create threads
    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_create(&packet_handler_thread, NULL, packet_handler, NULL);
    pthread_create(&sender_thread, NULL, sender, NULL);
    terminal();

    //terminate threads
    pthread_cancel(receiver_thread);
    pthread_cancel(packet_handler_thread);
    pthread_cancel(sender_thread);

    pthread_join(receiver_thread, NULL);
    pthread_join(packet_handler_thread, NULL);
    pthread_join(sender_thread, NULL);

    //close the socket
    int true_v = 1;
    setsockopt(socket_descriptor,SOL_SOCKET,SO_REUSEADDR,&true_v,sizeof(int));
    close(socket_descriptor);

    return 0;
}
