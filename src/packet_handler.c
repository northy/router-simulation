#include <packet_handler.h>
#include <terminal.h>

void send_dv() {
    r_message m;
    memset(&m, 0, sizeof(m));
    m.type = 1;
    m.source_router_id = router_id;

    //MAX_NEIGHBORS/BUF_LEN must be exact
    for (int i=0; i<MAX_NEIGHBORS/BUF_LEN; ++i) {
        m.offset = BUF_LEN*i;
        #if DEBUG
            printf("Preparing DV from %d to %d\n", m.offset, m.offset+BUF_LEN-1);
        #endif
        memcpy(m.payload, &distance_vector[router_id][m.offset], BUF_LEN);

        for (int n=0; n<neighbors_c; ++n) {
            #if DEBUG
                printf("Sending to neighbor %d\n", neighbors[n]);
            #endif
            m.destination_router_id = neighbors[n];
            message_queue* mq = malloc(sizeof(message_queue));
            memcpy(&mq->item, &m, sizeof(r_message));

            pthread_mutex_lock(&send_queue_mutex);
            if (send_queue_c<QUEUE_MAX) {
                send_queue_c++;
                TAILQ_INSERT_TAIL(&send_queue_head, mq, entries);
                pthread_mutex_unlock(&send_queue_mutex);
                sem_post(&sender_sem);
            }
            pthread_mutex_unlock(&send_queue_mutex);
        }
    }
}

void receive_dv(r_message *m) {
    memcpy(&distance_vector[m->source_router_id][m->offset], m->payload, BUF_LEN);
    dv_valid[m->source_router_id] = 1;
    #if DEBUG
        printf("Received DV [%d-%d] from %d\n", m->offset, m->offset+BUF_LEN-1, m->source_router_id);
    #endif
    //TODO: process
}

void* packet_handler(void* args) {
    struct timespec ts;
    time_t now, then;

    clock_gettime(CLOCK_REALTIME, &ts);
    time(&then);
    ts.tv_sec += WAIT_DELAY;

    for (;;) {
        message_queue *m; //buffer
        
        //check if enough time has passed to send the distance vector
        time(&now);
        if ((int)difftime(now,then)>WAIT_DELAY-1) {
            send_dv();
            clock_gettime(CLOCK_REALTIME, &ts);
            time(&then);
            ts.tv_sec += WAIT_DELAY;
        }
        
        int r;

        while ((r = sem_timedwait(&packet_handler_sem, &ts)) == -1 && errno == EINTR)
            continue; /* Restart if interrupted by handler */
        
        // Check what happened, if there's an error
        #if DEBUG
            if (r == -1) {
                if (errno == ETIMEDOUT)
                    printf("sem_timedwait() timed out\n");
                else
                    perror("sem_timedwait");
            }
            else printf("sem_timedwait() succeeded\n");
        #endif

        if (r == -1 && errno == ETIMEDOUT)
            continue; //Skip queue processing if woke up by a timeout

        pthread_mutex_lock(&process_queue_mutex);
        if (process_queue_c>0 && (m = TAILQ_FIRST(&process_queue_head))!=NULL) {
            #if DEBUG
                printf("Handler got %s for %d\n", m->item.payload, m->item.destination_router_id);
            #endif
            process_queue_c--;
            TAILQ_REMOVE(&process_queue_head, m, entries);
            pthread_mutex_unlock(&process_queue_mutex);
            if (m->item.type) { //control
                receive_dv(&m->item);
                free(m);
            }
            else if (m->item.destination_router_id==router_id) { //data for this router
                pthread_mutex_lock(&received_messages_mutex);
                if (received_messages_c<QUEUE_MAX) {
                    memcpy(&received_messages[received_messages_c++], &m->item, sizeof(r_message));
                }
                pthread_mutex_unlock(&received_messages_mutex);
                if (pthread_mutex_trylock(&terminal_mutex)==0) {
                    terminal_headers();
                    pthread_mutex_unlock(&terminal_mutex);
                }
                free(m);
            }
            else { //data to forward
                //add item to sender's queue
                pthread_mutex_lock(&send_queue_mutex);
                if (send_queue_c<QUEUE_MAX) {
                    send_queue_c++;
                    TAILQ_INSERT_TAIL(&send_queue_head, m, entries);
                    pthread_mutex_unlock(&send_queue_mutex);
                    sem_post(&sender_sem);
                }
                else { //no space, discard message
                    free(m);
                    pthread_mutex_unlock(&send_queue_mutex);
                }
            }
        }
        else {
            pthread_mutex_unlock(&process_queue_mutex); 
        }
    }

    pthread_exit(NULL);
}
