/**
* @file events.c
* @author louis.holleville
* @version 0.1
* @date 17-12-2018
* Handling of events in epoll
*/

#include "epoll.h"

void handle_bfill(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer)
{
    char cur = str[0];
    size_t b_len = (len - 1) * 8;
    size_t max = b_len - meta->nb_piece;
    size_t j = 0;
    for (uint32_t i = b_len; i > max; i--, j++)
    {
        peer->have[j] = ((cur >> (b_len - j) & 1));
        if (j % 8 == 0)
            cur = str[j / 8];
    }
}

void handle_have(uint32_t len, char *str, struct peer *peer) 
{
    if (len != 5)//err
        return;
    void *tmp = str;
    uint32_t *index_p = tmp;
    peer->have[ntohl(*index_p)] = 1;
}
