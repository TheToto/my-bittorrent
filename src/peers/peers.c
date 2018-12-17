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

static char *build_request(struct piece *piece, size_t i)
{
    uint32_t len = htonl(13);
    uint32_t index = htonl(piece->id_piece);
    uint32_t begin = htonl(i * 16384);
    uint32_t length = htonl(16384);
    if (i == piece->nb_blocks - 1 && piece->piece_size % 16384 != 0)
        length = htonl(piece->piece_size % 16384);

    static char buf[17];

    memcpy(buf, &len, 4);
    buf[4] = 6;
    memcpy(buf + 5, &index, 4);
    memcpy(buf + 9, &begin, 4);
    memcpy(buf + 13, &length, 4);
    return buf;
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

void request(struct metainfo *meta, struct peer *peer)
{
    struct piece *piece = meta->cur_piece;
    if (piece->buf == NULL)
    {
        size_t i = 0;
        for (; i < meta->nb_piece; i++)
            if (meta->have[i] == 0)
                break;
        if (i == meta->nb_piece)
            errx(0, "Download finised");
        piece->id_piece = i;
        piece->piece_size = get_piece_size(meta, i);
        piece->nb_blocks = piece->piece_size / 16384;
        if (piece->piece_size % 16384 != 0)
            piece->nb_blocks += 1;
        piece->buf = malloc(piece->piece_size * sizeof(char));
        piece->have = calloc(piece->nb_blocks, sizeof(char));
    }
    size_t i = 0;
    for (; i < piece->nb_blocks; i++)
    {
        if (piece->have[i] == 0)
            break;
    }
    if (i == piece->nb_blocks)
    {
        errx(1, "Nope, you are funcking something");
        return;
    }
    send(peer->sockfd, build_request(piece, i), 17, 0);
    printf("Request piece %ld, block %ld sent to %s !\n", piece->id_piece,
            i, peer->ip);
}
