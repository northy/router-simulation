#pragma once

#include <structures.h>

extern int router_id;
extern unsigned short router_port;
extern char router_ip[16];

extern int link_cost[MAX_NEIGHBORS];
extern int neighbors_c, neighbors[MAX_NEIGHBORS];

extern int total_router_c;
extern unsigned short external_router_port[MAX_NEIGHBORS];
extern char external_router_ip[MAX_NEIGHBORS][16];
