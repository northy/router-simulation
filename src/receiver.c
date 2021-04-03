#include <receiver.h>

#include <stdio.h>

//listens to incoming messages
void* receiver(void* args) {
    r_message buf;
    struct sockaddr_in si_other;
    int recv_len, slen = sizeof(si_other);
    
    for (;;) {
        memset(&buf, 0, sizeof(buf));

        //receive UDP packet
        if ((recv_len = recvfrom(socket_descriptor, &buf, sizeof(buf), 0, (struct sockaddr *) &si_other, &slen)) == -1) {
            perror("Receiving socket");
            exit(1);
        }

        //add item to packet_handler's queue
        message_queue* m = malloc(sizeof(message_queue));
        memcpy(&m->item, &buf, sizeof(r_message));

        pthread_mutex_lock(&process_queue_mutex);
        TAILQ_INSERT_TAIL(&process_queue_head, m, entries);
        pthread_mutex_unlock(&process_queue_mutex);
        sem_post(&packet_handler_sem);
    }
    
    pthread_exit(NULL);
}
