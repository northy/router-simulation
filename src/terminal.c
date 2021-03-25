#include <terminal.h>

char terminal_free = 0;

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

void terminal_neighbors() {
    erase();
    for (int i=0; i<neighbors_c; ++i)
        printf("Neighbor %d: (id - %d) (cost - %d) (ip-port - %s:%hu) (status - unknown)\n", i+1, neighbors[i], link_cost[neighbors[i]], external_router_ip[neighbors[i]], external_router_port[neighbors[i]]);
    printf("\nPress enter...\n");
    get_char();
}

void terminal_send() {
    erase();
    printf("Whom to send to:\n\n");
    for (int i=1; i<total_router_c+1; ++i)
        printf("%d: (ip-port - %s:%hu) (cost - unknown)\n", i, external_router_ip[i], external_router_port[i]);
    printf("\n$ ");

    int choice;
    scanf("%d", &choice);
    flush();

    if (choice<=0 || choice>total_router_c)
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

void terminal_received() {
    erase();
    pthread_mutex_lock(&received_messages_mutex);
    while (received_messages_c--) {
        printf("%s:%hu sent: %s\n", external_router_ip[received_messages[received_messages_c].source_router_id], external_router_port[received_messages[received_messages_c].source_router_id], received_messages[received_messages_c].payload);
    }
    received_messages_c = 0;
    pthread_mutex_unlock(&received_messages_mutex);
    printf("\nPress enter...\n");
    get_char();
}

void terminal() {
    bool running = true;
    while (running) {
        terminal_headers();
        terminal_free = 1;
        char input = get_char();
        terminal_free = 0;
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
