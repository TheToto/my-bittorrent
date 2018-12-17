#define _DEFAULT_SOURCE
#include <curl/curl.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <jansson.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "parser.h"
#include "tracker.h"
#include "epoll.h"

static char *unfix_info_hash(char *str)
{
    static char hash[20];
    size_t j = 0;
    for (size_t i = 0; j < 20; i += 3, j++)
    {
        char tmp[3] =
        {
            str[i + 1], str[i + 2], '\0'
        };
        unsigned int hex;
        sscanf(tmp, "%02X", &hex);
        hash[j] = hex;
    }
    return hash;
}

static char *build_handshake(struct metainfo *meta)
{
    static char hand[68];
    hand[0] = 19;
    memcpy(hand + 1, "BitTorrent protocol", 19);
    memcpy(hand + 20, "\0\0\0\0\0\0\0\0", 8);
    memcpy(hand + 28, unfix_info_hash(meta->info_hash), 20);
    memcpy(hand + 48, meta->peer_id, 20);
    return hand;
}

void handshake(struct metainfo *meta, struct peer *peer)
{
    if (peer->sockfd == -1)
        return;
    send(peer->sockfd, build_handshake(meta), 68, 0);
    printf("Handshake sent to %s !\n", peer->ip);
}

static char *build_request(struct metainfo *meta)
{

    return NULL;
}

void request(struct metainfo *meta, struct peer *peer)
{
    struct piece *piece = meta->cur_piece;
    if (piece->buf == NULL)
    {
        // INIT PIECE
    }
    // REQUEST A PIECE
}
