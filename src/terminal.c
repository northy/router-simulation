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
    fputs("Router simulation\n1 - ping\n\n0 - exit\n\n$ ", stdout);
}

void terminal_pongs() {
    erase();
    printf("Pong %d!\n\nPress enter...", (pongs++));
    get_char();
}

void terminal() {
    bool running = true;
    while (running) {
        terminal_headers();
        char input = get_char();
        switch (input) {
            case '1' :
                terminal_pongs();
                break;
            case '0' :
                running = false;
                break;
            default :
                break;
        }
    }
}
