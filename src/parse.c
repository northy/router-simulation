#include <parse.h>

void parse_link() {
    FILE *fp = fopen("link.config", "r");

    int router1, router2, cost;
    while (!feof(fp)) {
        fscanf(fp, "%d %d %d", &router1, &router2, &cost);
        if (router_id==router1) {
            if (link_cost[router2]!=-1) continue; //Already set, fixes file input dupe bug
            neighbors[(neighbors_c++)] = router2;
            link_cost[router2] = cost;
            dv_source[router2] = router2; //TODO: Delete when receiving dynamically
            distance_vector[router_id][router2] = cost; //TODO: Delete when receiving dynamically
        }
        if (router_id==router2) {
            if (link_cost[router1]!=-1) continue; //Already set, fixes file input dupe bug
            neighbors[(neighbors_c++)]=router1;
            link_cost[router1] = cost;
            dv_source[router1] = router1; //TODO: Delete when receiving dynamically
            distance_vector[router_id][router1] = cost; //TODO: Delete when receiving dynamically
        }
    }

    fclose(fp);
}

void parse_router() {
    FILE *fp = fopen("router.config", "r");

    int id;
    unsigned short port;
    char ip[16];
    while (!feof(fp)) {
        fscanf(fp, "%d %hu %s", &id, &port, ip);
        if (id==router_id) {
            router_port = port;
            memcpy(router_ip,ip,sizeof(ip));
        }
        if (link_cost[id]!=-1) {
            external_router_port[id] = port;
            memcpy(external_router_ip[id],ip,sizeof(ip));
            if (id-total_router_c>0) total_router_c=id;
        }
    }

    fclose(fp);
}
