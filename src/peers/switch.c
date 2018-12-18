/**
* @file switch.c
* @author louis.holleville
* @version 0.1
* @date 17-12-2018
* Management of events thanks to a switch
*/

#include "parser.h"
#include "epoll.h"

static void switch_events_next(struct metainfo *meta, struct peer *peer,
        char *str, uint32_t len)
{
    switch (str[4])
    {
    case 5:
        handle_bfill(meta, len, str + 5, peer);
        break;

    case 6:
        break;

    case 7:
        handle_piece(meta, len, str, peer);
        break;

    case 8:
        break;

    default:
        break;
    }
}

void switch_events(struct metainfo *meta, struct peer *peer, char *str,
        uint32_t len)
{
    switch (str[4])
    {

    case 0:
        printf("Peer %s id alive !\n", peer->ip);
        break;//handle timeout
    case 1:
        peer->state = !peer->state;
        printf("New choked update (%d) of %s !\n", peer->state, peer->ip);
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        handle_have(len, str, peer);
        break;

    default:
        switch_events_next(meta, peer, str, len);
    }
}
