#include <packet_handler.h>
#include <terminal.h>

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
            //TODO: Send distance vector
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

        pthread_mutex_lock(&process_queue_mutex);
        m = TAILQ_FIRST(&process_queue_head);
        if (m!=NULL) {
            #if DEBUG
                printf("Handler got %s\n",m->item.payload);
            #endif
            TAILQ_REMOVE(&process_queue_head, m, entries);
            pthread_mutex_unlock(&process_queue_mutex);
            if (m->item.type) { //control
                //TODO: process
                free(m);
            }
            else if (m->item.destination_router_id==router_id) { //data for this router
                pthread_mutex_lock(&received_messages_mutex);
                memcpy(&received_messages[received_messages_c++], &m->item, sizeof(r_message));
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
                TAILQ_INSERT_TAIL(&send_queue_head, m, entries);
                pthread_mutex_unlock(&send_queue_mutex);
                sem_post(&sender_sem);
            }
        }
        else {
            pthread_mutex_unlock(&process_queue_mutex); 
        }
    }

    pthread_exit(NULL);
}
