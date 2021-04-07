#include <sender.h>

void* sender(void* args) {
    message_queue *m;

    struct sockaddr_in si_other;
    int slen=sizeof(si_other);

    for (;;) {
        message_queue *m; //buffer
        
        int r;

        while ((r = sem_wait(&sender_sem)) == -1 && errno == EINTR)
            continue; /* Restart if interrupted by handler */
        
        #if DEBUG
            printf("Sender entered!\n");
        #endif

        pthread_mutex_lock(&send_queue_mutex);
        if (send_queue_c>0 && (m = TAILQ_FIRST(&send_queue_head))!=NULL) {
            send_queue_c--;
            TAILQ_REMOVE(&send_queue_head, m, entries);
            pthread_mutex_unlock(&send_queue_mutex);

            #if DEBUG
                printf("Sender got %s\n",m->item.payload);
            #endif

            //prepare socket address to send message
            memset((char *) &si_other, 0, sizeof(si_other));
            si_other.sin_family = AF_INET;
            si_other.sin_port = htons(external_router_port[m->item.destination_router_id]);
            if (inet_aton(external_router_ip[m->item.destination_router_id] , &si_other.sin_addr) == 0) {
                fprintf(stderr, "inet_aton() failed\n");
                exit(1);
            }

            //send the UDP packet
            if (sendto(socket_descriptor, &m->item, sizeof(m->item), 0, (struct sockaddr *) &si_other, slen)==-1) {
                perror("Sending socket");
                exit(1);
            }

            free(m);
        }
        else {
            pthread_mutex_unlock(&send_queue_mutex); 
        }
    }

    pthread_exit(NULL);
}
