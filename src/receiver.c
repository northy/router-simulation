#include <sender.h>

void* receiver(void* args) {
    printf("Receiver was called\n");

    pthread_exit(NULL);
}
