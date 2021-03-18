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
    printf("Router simulation - Router %d\n1 - neighbors\n\n0 - exit\n\n$ ", router_id);
}

void terminal_neighbors() {
    erase();
    for (int i=0; i<neighbors_c; ++i)
        printf("Neighbor %d: (id - %d) (cost - %d) (ip-port - unknown:unknown) (status - unknown)\n", i+1, neighbors[i], link_cost[neighbors[i]]);
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
            case '0' :
                running = false;
                break;
            default :
                break;
        }
    }
}
