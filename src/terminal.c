#include <terminal.h>

void erase() {
    fprintf(stdout, "\033[2J\033[0;0f");
}

char get_char() {
    fflush(stdin);
    char ch = getchar();
    while(ch!='\n' && '\n'!=getchar()); //read until the buffer is clear
    return ch;
}

void terminal_headers() {
    erase();
    printf("Router simulation - Router %d\n1 - neighbors\n2 - send\n\n0 - exit\n\n$ ", router_id);
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
    printf("Whom to send to (not yet working):\n\n");
    for (int i=1; i<total_router_c+1; ++i)
        printf("%d: (ip-port - %s:%hu) (cost - unknown)\n", i, external_router_ip[i], external_router_port[i]);
    printf("\nPress enter...\n");
    get_char();
}

void terminal() {
    bool running = true;
    while (running) {
        terminal_headers();
        char input = get_char();
        switch (input) {
            case '1' :
                terminal_neighbors();
                break;
            case '2' :
                terminal_send();
                break;
            case '0' :
                running = false;
                break;
            default :
                break;
        }
    }
}
