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

static char *get_peerID(void)
{
    char *peer = calloc(21, sizeof(char));
    strcpy(peer, PEER_PREFIX);
    char str[13] =
    {
        0
    };

    srand(time(NULL));
    for (int i = 0; i < 12; i++)
        str[i] = rand() % 26 + 'a';
    strcat(peer, str);
    return peer;
}

long get_portL(void)
{
    return PEER_PORT;
}

char *get_tracker_request(struct metainfo *meta)
{
    char *str = calloc(200, sizeof(char));
    if (!str)
        err(1, "get_tracker_request: cannot calloc");

    strcpy(str, "GET /announce?peer_id=");
    char *tmp = get_peerID();
    strcat(str, tmp);
    free(tmp);
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
