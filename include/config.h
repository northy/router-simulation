#pragma once

#define BUF_LEN 512 //received messages buffer
#define MAX_NEIGHBORS 512 //maximum number of routers in the network graph
#define QUEUE_MAX 1024 //maximum size of the internal queues
#define WAIT_DELAY 5 //time in seconds to sleep until sending control messages
#define DV_SEND_ROUNDS_TOLERANCE 3 //if a DV isn't received in WAIT_DELAY*DV_SEND_ROUNDS_TOLERANCE, a link will be considered down
#define DEBUG 0
#define WEIGHT_INFINITE 100
