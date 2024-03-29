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
#include "integrity.h"
#include "misc.h"

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

static char *build_request(struct piece *piece, size_t i, struct peer *peer,
        struct metainfo *meta)
{
    uint32_t len = htonl(13);
    uint32_t index = htonl(piece->id_piece);
    uint32_t begin_p = i * 16384;
    uint32_t begin = htonl(begin_p);
    uint32_t length = htonl(16384);
    if (i == piece->nb_blocks - 1 && piece->piece_size % 16384 != 0)
        length = htonl(piece->piece_size % 16384);

    static char buf[17];

    memcpy(buf, &len, 4);
    buf[4] = 6;
    memcpy(buf + 5, &index, 4);
    memcpy(buf + 9, &begin, 4);
    memcpy(buf + 13, &length, 4);
    if (meta->verbose)
        printf("%6s: msg: send: %s:%d: request %ld %d %d\n", meta->torrent_id,
                peer->ip, peer->port, piece->id_piece, begin_p, ntohl(length));

    return buf;
}

static char *build_interested(void)
{
    static char buf[5];
    uint32_t len = htonl(1);

    memcpy(buf, &len, 4);
    buf[4] = 2;
    return buf;
}


char handshake(struct metainfo *meta, struct peer *peer)
{
    if (peer->sockfd == -1)
        return 0;
    send(peer->sockfd, build_handshake(meta), 68, 0);
    if (meta->verbose)
        printf("%6s: msg: send: %s:%d: handshake\n", meta->torrent_id, peer->ip,
                peer->port);
    return 1;
}

static size_t get_piece_size(struct metainfo *meta, size_t nb)
{
    if (nb == meta->nb_piece - 1)
    {
        size_t mod = get_total_size(meta) % meta->piece_size;
        if (mod != 0)
            return mod;
    }
    return meta->piece_size;
}

static int init_request(struct metainfo *meta, struct peer *peer)
{
    struct piece *piece = meta->cur_piece;
    size_t i = 0;
    for (; i < meta->nb_piece; i++)
    {
        if (meta->have[i] == 0 && peer->have[i] == 1)
            break;
    }
    if (i == meta->nb_piece) // Test if download is complete
    {
        for (i = 0; i < meta->nb_piece; i++)
        {
            if (meta->have[i] == 0)
            {
                not_interested(meta, peer);
                return 1; // No complete : Peer have no piece needed
            }
        }
        return 0;
    }
    piece->id_piece = i;
    piece->piece_size = get_piece_size(meta, i);
    piece->nb_blocks = piece->piece_size / 16384;
    if (piece->piece_size % 16384 != 0)
        piece->nb_blocks += 1;
    piece->buf = malloc(piece->piece_size * sizeof(char));
    piece->have = calloc(piece->nb_blocks, sizeof(char));
    for (size_t k = 0; k < meta->peers->size; k++)
    {
        struct peer *cur_p = meta->peers->list[k];
        if (cur_p->have[i] && !cur_p->interested)
            interested(meta, cur_p);
    }
    request(meta, peer);
    return 1;
}

int request(struct metainfo *meta, struct peer *peer)
{
    if (peer->state)
        return 1;
    struct piece *piece = meta->cur_piece;
    if (piece->buf == NULL)
        return init_request(meta, peer);
    if (peer->have[piece->id_piece] == 0)
    {
        not_interested(meta, peer);
        return 1;
    }
    size_t i = 0;
    for (; i < piece->nb_blocks; i++)
    {
        if (piece->have[i] == 0)
            break;
    }
    if (i == piece->nb_blocks)
    {
        for (i = 0; i < piece->nb_blocks; i++)
        {
            if (piece->have[i] == 1) //ReAsk missing parts
                break;
        }
    }
    send(peer->sockfd, build_request(piece, i, peer, meta), 17, 0);
    piece->have[i] = 1;
    return 1;
}

void interested(struct metainfo *meta, struct peer *peer)
{
    size_t i = 0;
    for (; i < meta->nb_piece; i++)
    {
        if (meta->have[i] == 0 && peer->have[i] == 1)
            break;
    }
    if (i == meta->nb_piece)
    {
        return;
    }
    send(peer->sockfd, build_interested(), 5, 0);
    peer->interested = 1;
    if (meta->verbose)
        printf("%6s: msg: send: %s:%d: interested\n", meta->torrent_id,
                peer->ip, peer->port);
}

void not_interested(struct metainfo *meta, struct peer *peer)
{
    static char buf[5];
    uint32_t len = htonl(1);

    memcpy(buf, &len, 4);
    buf[4] = 3;
    send(peer->sockfd, buf, 5, 0);
    peer->interested = 0;
    if (meta->verbose)
        printf("%6s: msg: send: %s:%d: not interested\n", meta->torrent_id,
                peer->ip, peer->port);

}

void keep_alive(struct metainfo *meta, struct peer *peer)
{
    static char buf[4];
    uint32_t len = htonl(0);

    memcpy(buf, &len, 4);
    send(peer->sockfd, buf, 4, 0);
    if (meta->verbose)
        printf("%6s: msg: send: %s:%d: keep-alive\n", meta->torrent_id,
                peer->ip, peer->port);

}
