#include <global.h>
#include <terminal.h>
#include <receiver.h>
#include <sender.h>
#include <packet_handler.h>
#include <parse.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_t receiver_thread;

int router_id;
int neighbors_c=0, neighbors[MAX_NEIGHBORS];
int link_cost[MAX_NEIGHBORS];

int main(int argc, char *argv[]) {
    if (argc<2) {
        fprintf(stderr, "Please use \"./Router <id>\"\n");
        exit(1);
    }

    router_id = atoi(argv[1]);

    parse_link();

    setvbuf(stdout, NULL, _IONBF, -1); //set the STDOUT buffer to flush immediately

    terminal();

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_join(receiver_thread, NULL);
    printf("Receiver returned\n");

    return 0;
}
