#include <parse.h>

void parse_link() {
    FILE *fp = fopen("link.config", "r");

    int router1, router2, cost;
    while (!feof(fp)) {
        fscanf(fp, "%d %d %d\n", &router1, &router2, &cost);
        if (router_id==router1) {
            neighbors[(neighbors_c++)] = router2;
            link_cost[router2] = cost;
        }
        if (router_id==router2) {
            neighbors[(neighbors_c++)]=router1;
            link_cost[router1] = cost;
        }
    }

    fclose(fp);
}
