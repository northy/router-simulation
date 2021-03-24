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

pthread_t receiver_thread;

int router_id;
unsigned short router_port;
char router_ip[16];

int total_router_c=0;
unsigned short external_router_port[MAX_NEIGHBORS];
char external_router_ip[MAX_NEIGHBORS][16];

int neighbors_c=0, neighbors[MAX_NEIGHBORS];
int link_cost[MAX_NEIGHBORS];

int socket_descriptor;

r_message received_messages[QUEUE_MAX];
int received_messages_c = 0;
pthread_mutex_t received_messages_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
    if (argc<2) {
        fprintf(stderr, "Please use \"./Router <id>\"\n");
        exit(1);
    }

    router_id = atoi(argv[1]);

    setvbuf(stdout, NULL, _IONBF, -1); //set the STDOUT buffer to flush immediately

    parse_link();
    parse_router();

    if ((socket_descriptor=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Socket creation error");
        exit(1);
    }

    struct sockaddr_in si_me;
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(router_port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(socket_descriptor, (struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
        perror("Socket bind error");
        exit(1);
    }

    //populate by hand one message
    received_messages_c=1;
    received_messages[0].type=0;
    received_messages[0].source_router_id = 2;
    received_messages[0].destination_router_id = 1;
    strcpy(received_messages[0].payload, "Hello world!");

    terminal();

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_join(receiver_thread, NULL);
    printf("Receiver returned\n");

    int true_v = 1;
    setsockopt(socket_descriptor,SOL_SOCKET,SO_REUSEADDR,&true_v,sizeof(int));
    close(socket_descriptor);

    return 0;
}
