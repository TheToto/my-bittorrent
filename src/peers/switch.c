/**
* @file switch.c
* @author louis.holleville
* @version 0.1
* @date 17-12-2018
* Management of events thanks to a switch
*/

#include <err.h>
#include "parser.h"
#include "epoll.h"

static int switch_events_next(struct metainfo *meta, struct peer *peer,
        char *str, uint32_t len)
{
    switch (str[4])
    {
    case 5:
        handle_bfill(meta, len, str + 5, peer);
        return 1;

    case 6:
        return 1;

    case 7:
        handle_piece(meta, len, str, peer);
        return 1;

    case 8:
        return 1;

    default:
        warnx("Unknow request: %c", str[4]);
        remove_peers_to_epoll(meta->peers, peer);
        return 1;
    }
    return 1;
}

int switch_events(struct metainfo *meta, struct peer *peer, char *str,
        uint32_t len)
{
    switch (str[4])
    {

    case 0:
        printf("Peer %s id alive !\n", peer->ip);
        return 1;//handle timeout
    case 1:
        peer->state = !peer->state;
        printf("New choked update (%s) of %s !\n",
                peer->state ? "choked" : "unchoked", peer->ip);
        if (peer->state == 0)
            return request(meta, peer);
        return 1;

    case 2:
        return 1;

    case 3:
        return 1;

    case 4:
        handle_have(meta, len, str, peer);
        return 1;

    default:
        return switch_events_next(meta, peer, str, len);
    }
    return 1;
}
