#include <terminal.h>

pthread_mutex_t terminal_mutex = PTHREAD_MUTEX_INITIALIZER;

//helpers
void erase() {
    fprintf(stdout, "\033[2J\033[0;0f");
}

void flush() {
    char ch;
    do {
        ch = getchar();
    } while(ch != EOF && ch != '\n');
}

char get_char() {
    fflush(stdin);
    char ch = getchar();
    while (ch!='\n' && '\n'!=getchar()); //read until the buffer is clear
    return ch;
}

void terminal_headers() {
    erase();
    printf("Router simulation - Router %d\n1 - neighbors\n2 - send\n3 - received messages (%d)\n\n0 - exit\n\n$ ", router_id, received_messages_c);
}

//shows all the neighbors and the status and cost of each one
void terminal_neighbors() {
    erase();
    for (int i=0; i<neighbors_c; ++i)
        printf("Neighbor %d: (id - %d) (cost - %d) (ip-port - %s:%hu) (status - unknown)\n", i+1, neighbors[i], link_cost[neighbors[i]], external_router_ip[neighbors[i]], external_router_port[neighbors[i]]);
    printf("\nPress enter...\n");
    get_char();
}

//shows menu to select an option and send the message
void terminal_send() {
    erase();
    printf("Whom to send to:\n\n");
    for (int i=1; i<total_router_c+1; ++i) {
        if (distance_vector[i]!=-1) printf("%d: (ip-port - %s:%hu) (cost - %d)\n", i, external_router_ip[i], external_router_port[i], distance_vector[i]);
    }
    printf("\n0: go back\n\n$ ");

    //choose the address to send the message 
    int choice;
    scanf("%d", &choice);
    flush();

    if (choice<=0 || choice>total_router_c || distance_vector[choice]==-1)
        return;

    printf("\nMessage: ");
    
    r_message m;
    memset(&m, 0, sizeof(m));
    m.destination_router_id = choice;
    m.source_router_id = router_id;
    fgets(m.payload, BUF_LEN, stdin);

    if ((strlen(m.payload) > 0) && (m.payload[strlen (m.payload) - 1] == '\n'))
        m.payload[strlen (m.payload) - 1] = '\0';

    message_queue* mq = malloc(sizeof(message_queue));
    memcpy(&mq->item, &m, sizeof(r_message));

    pthread_mutex_lock(&process_queue_mutex);
    TAILQ_INSERT_TAIL(&process_queue_head, mq, entries);
    pthread_mutex_unlock(&process_queue_mutex);
    sem_post(&packet_handler_sem);

    printf("\nSent! Press enter...\n");
    get_char();
}

//shows how messages that have been received
void terminal_received() {
    erase();
    pthread_mutex_lock(&received_messages_mutex);
    while (received_messages_c--) {
        printf("%d (%s:%hu) sent: %s\n", received_messages[received_messages_c].source_router_id, external_router_ip[received_messages[received_messages_c].source_router_id], external_router_port[received_messages[received_messages_c].source_router_id], received_messages[received_messages_c].payload);
    }
    received_messages_c = 0;
    pthread_mutex_unlock(&received_messages_mutex);
    printf("\nPress enter...\n");
    get_char();
}

//menu shown in the terminal
void terminal() {
    bool running = true;
    while (running) {
        terminal_headers();
        pthread_mutex_unlock(&terminal_mutex);
        char input = get_char();
        pthread_mutex_lock(&terminal_mutex);
        switch (input) {
            case '1' :
                terminal_neighbors();
                break;
            case '2' :
                terminal_send();
                break;
            case '3' :
                terminal_received();
                break;
            case '0' :
                running = false;
                break;
            default :
                break;
        }
    }
}
