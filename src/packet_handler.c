#include <packet_handler.h>
#include <terminal.h>

void send_dv() {
    r_message m;
    memset(&m, 0, sizeof(m));
    m.type = 1;
    m.source_router_id = router_id;

    for (int i=1; i<MAX_NEIGHBORS; ++i) {
        if (distance_vector[router_id][i]!=-1) {
            uint32_t nl = htonl(i); //convert and store destination as BIG ENDIAN
            m.payload[m.offset*5+0] = (nl>>0)&0xFF;
            m.payload[m.offset*5+1] = (nl>>8)&0xFF;
            m.payload[m.offset*5+2] = (nl>>16)&0xFF;
            m.payload[m.offset*5+3] = (nl>>24)&0xFF;
            m.payload[m.offset*5+4] = distance_vector[router_id][i]; //store cost
            //printf("%d %d %d %d\n", m.payload[m.offset*5+0], m.payload[m.offset*5+1], m.payload[m.offset*5+2], m.payload[m.offset*5+3]);
            m.offset++;
        }

        if ((m.offset+1)*5>=BUF_LEN || i==MAX_NEIGHBORS-1) {
            for (int n=0; n<neighbors_c; ++n) {
                #if DEBUG
                    printf("Sending DV to neighbor %d\n", neighbors[n]);
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
            m.offset = 0;
        }
    }
}

void receive_dv(r_message *m) {
    #if DEBUG
        printf("Received DV from %d\n", m->source_router_id);
    #endif

    for (int i=0; i<m->offset; ++i) {
        uint32_t nl=0;
        nl|=(m->payload[i*5+0]<<0);
        nl|=(m->payload[i*5+1]<<8);
        nl|=(m->payload[i*5+2]<<16);
        nl|=(m->payload[i*5+3]<<24);
        int dest = ntohl(nl);
        printf("%d %d\n", dest, m->payload[i*5+4]);
        distance_vector[m->source_router_id][dest] = m->payload[i*5+4];
    }
    dv_valid[m->source_router_id] = 1;
    
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
