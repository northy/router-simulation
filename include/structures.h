#include <config.h>

#include <arpa/inet.h>
#include <sys/socket.h>

//received message
typedef struct {
    char type; //data = control; 1 = control
    struct sockaddr_in source_router;
    struct sockaddr_in destination_router;
    char payload[BUF_LEN];

} r_message;
