#include <global.h>
#include <terminal.h>
#include <receiver.h>
#include <sender.h>
#include <packet_handler.h>

int pongs=0;

pthread_t receiver_thread;

int main() {
    setvbuf(stdout, NULL, _IONBF, -1); //set the STDOUT buffer to flush immediately

    pongs++;
    terminal();

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_join(receiver_thread, NULL);
    printf("Receiver returned\n");

    return 0;
}
