#include <packet_handler.h>
#include <terminal.h>

//send a message that we deleted something.
void send_nullable(int i) {
    r_message m;
    memset(&m, 0, sizeof(m));
    m.type = 1;
    m.source_router_id = router_id;

    uint32_t nl = htonl(i);
    m.payload[m.offset*5+0] = (nl>>0)&0xFF;
    m.payload[m.offset*5+1] = (nl>>8)&0xFF;
    m.payload[m.offset*5+2] = (nl>>16)&0xFF;
    m.payload[m.offset*5+3] = (nl>>24)&0xFF;
    m.payload[m.offset*5+4] = -1;
    m.offset++;

    for (int n=0; n<neighbors_c; ++n) {
        if (!link_enabled[neighbors[n]]) continue; //ignore disabled links' neighbors
        m.destination_router_id = neighbors[n];
        m.offset = 1;

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

char update_dv(int sender) {
    pthread_mutex_lock(&dv_mutex);
    char updateflag = 0;

    //checks if a DV we were using changed.
    if (sender!=-1) {
        for (int i=0; i<MAX_NEIGHBORS; ++i) {
            if (dv_source[i]==sender) {
                //sender told us that he deleted this route.
                if (distance_vector[sender][i]==-1) {
                    distance_vector[router_id][i] = -1;
                    dv_source[i] = -1;
                    send_nullable(i);
                    continue;
                }
                
                char d = distance_vector[sender][i]+link_cost[sender];
                if (d>=WEIGHT_INFINITE || d<0) d = -1;
                if (distance_vector[router_id][i] != d) {
                    distance_vector[router_id][i] = d;
                    if (d==-1){
                        distance_vector[sender][i] = -1;
                        dv_source[i] = -1;
                        send_nullable(i);
                    }
                    updateflag = 1;
                }
            }
        }
    }

    //check if some source is now invalid
    for (int i=0; i<MAX_NEIGHBORS; ++i) {
        if (dv_source[i]!=-1 && (!dv_valid[dv_source[i]] || !link_enabled[dv_source[i]])) {
            distance_vector[router_id][i] = -1;
            dv_source[i] = -1;
            send_nullable(i);
        }
    }
    
    for (int n=0; n<neighbors_c; ++n) {
        if (!dv_valid[neighbors[n]] || !link_enabled[neighbors[n]]) continue;

        //check neighbor's distance
        for (int i=0; i<MAX_NEIGHBORS; ++i) {
            if (distance_vector[neighbors[n]][i]==-1) continue;

            char d = distance_vector[neighbors[n]][i]+link_cost[neighbors[n]];
            if (d>=WEIGHT_INFINITE || d<0) {
                continue;
            }
            if (distance_vector[router_id][i]==-1 || distance_vector[router_id][i] > d) {
                updateflag = 1;
                distance_vector[router_id][i] = d;
                dv_source[i] = neighbors[n];
            }
        }

        //check own distance
        char d = link_cost[neighbors[n]];
        if (d==-1) continue;

        if (distance_vector[router_id][neighbors[n]]==-1 || distance_vector[router_id][neighbors[n]] > d) {
            updateflag = 1;
            distance_vector[router_id][neighbors[n]] = d;
            dv_source[neighbors[n]] = neighbors[n];
        }
    }
    pthread_mutex_unlock(&dv_mutex);

    return updateflag;
}

void send_dv() {
    int last_offset = 0;

    r_message m;
    memset(&m, 0, sizeof(m));
    m.type = 1;
    m.source_router_id = router_id;

    pthread_mutex_lock(&dv_mutex);
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
                if (!link_enabled[neighbors[n]]) continue; //ignore disabled links' neighbors
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
            last_offset = m.offset;
            m.offset = 0;
        }
    }

    //send one last 0 objects DV if needed.
    if ((last_offset+1)*5>=BUF_LEN) {
        for (int n=0; n<neighbors_c; ++n) {
            if (!link_enabled[neighbors[n]]) continue; //ignore disabled links' neighbors
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
    }

    pthread_mutex_unlock(&dv_mutex);
}

void receive_dv(r_message *m) {
    if (!link_enabled[m->source_router_id])
        return; //ignore disabled link's messages

    #if DEBUG
        printf("Received DV from %d\n", m->source_router_id);
    #endif

    pthread_mutex_lock(&dv_mutex);
    for (int i=0; i<m->offset; ++i) {
        uint32_t nl=0;
        nl|=(m->payload[i*5+0]<<0);
        nl|=(m->payload[i*5+1]<<8);
        nl|=(m->payload[i*5+2]<<16);
        nl|=(m->payload[i*5+3]<<24);
        int dest = ntohl(nl);
	#if DEBUG
		printf("Distance from %d: %d\n", dest, m->payload[i*5+4]);
	#endif
        distance_vector[m->source_router_id][dest] = m->payload[i*5+4];
    }

    //it was the last DV to be sent.
    if ((m->offset+1)*5<BUF_LEN) {
        dv_valid[m->source_router_id] = 1;
        time(&dv_recv_time[m->source_router_id]);
        pthread_mutex_unlock(&dv_mutex);

        pthread_mutex_lock(&received_dvs_mutex);
        if (received_dvs_c<QUEUE_MAX) { //checks if the limit of dvs in the queue has been reached
            received_dvs[received_dvs_c++] = m->source_router_id;
        }
        pthread_mutex_unlock(&received_dvs_mutex);

        if (pthread_mutex_trylock(&terminal_mutex)==0) {
            terminal_headers();
            pthread_mutex_unlock(&terminal_mutex);
        }

        if (update_dv(m->source_router_id)) {
            send_dv();
        }

        return;
    }
    
    pthread_mutex_unlock(&dv_mutex);
}

//check if the dv is disabled
void check_disabled() {
    char updateflag=0;

    time_t now;
    time(&now);

    pthread_mutex_lock(&dv_mutex);

    for (int n=0; n<neighbors_c; ++n) {
        if (dv_valid[neighbors[n]] && (int)difftime(now,dv_recv_time[neighbors[n]])>WAIT_DELAY*DV_SEND_ROUNDS_TOLERANCE) {
            dv_valid[neighbors[n]] = 0;
            updateflag = 1;
        }
    }

    pthread_mutex_unlock(&dv_mutex);

    if (updateflag && update_dv(-1)) {
        send_dv();
    }
}

void* packet_handler(void* args) {
    struct timespec ts;
    time_t now, then;

    update_dv(-1);

    clock_gettime(CLOCK_REALTIME, &ts);
    time(&then);
    ts.tv_sec += WAIT_DELAY;

    for (;;) {
        message_queue *m; //buffer
        
        //check if enough time has passed to send the distance vector
        time(&now);
        if ((int)difftime(now,then)>WAIT_DELAY-1) {
            send_dv();
            check_disabled();
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
                if (!m->item.type) printf("Handler got %s for %d\n", m->item.payload, m->item.destination_router_id);
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
