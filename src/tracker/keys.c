/**
* @file keys.c
* @author louis.holleville
* @version 0.1
* @date 14-12-2018
* Generation of keys for tracking request
*/

#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <time.h>

#include "integrity.h"
#include "tracker.h"
#include "parser.h"

long get_portL(void)
{
    return PEER_PORT;
}

char *get_tracker_request(struct metainfo *meta)
{
    char *str = calloc(200, sizeof(char));
    if (!str)
        err(1, "get_tracker_request: cannot calloc");

    strcpy(str, "/announce?peer_id=");
    strcat(str, meta->peer_id);
    strcat(str, "&info_hash=");
    strcat(str, meta->info_hash);
    strcat(str, "&port=");
    char tmp2[5];
    sprintf(tmp2, "%d", PEER_PORT);
    strcat(str, tmp2);
    char tmp3[256];
    strcat(str, "&left=");
    sprintf(tmp3, "%lu", get_total_size(meta));
    strcat(str, tmp3);
    strcat(str, "&downloaded=0&uploaded=0&compact=1");
    return str;
}
