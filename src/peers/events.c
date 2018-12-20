/**
* @file events.c
* @author louis.holleville thomas.lupin
* @version 0.1
* @date 17-12-2018
* Handling of events in epoll
*/

#include <arpa/inet.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include "epoll.h"
#include "misc.h"
#include "integrity.h"
#include "parser.h"

void handle_handshake(struct metainfo *meta, struct peer *peer,
        char *str, int bytes)
{
    if (bytes != 49 + str[0])
    {
        if (meta->verbose)
            printf("%6s: msg: recv: %s:%d: incorrect response\
                   (lenght : %d, expected : %d)!\n", meta->torrent_id,
                   peer->ip, peer->port, bytes, 49 + str[0]);
        close(peer->sockfd);
        peer->sockfd = -1;
        remove_peers_to_epoll(meta->peers, peer,
                meta->verbose ? meta->torrent_id : NULL);
        return;
    }
    if (memcmp(unfix_info_hash(meta->info_hash),
                str + str[0] + 9, 20) != 0)
    {
        if (meta->verbose)
            printf("%6s: msg: recv: %s:%d: incorrect response (hash_info)!\n",
                    meta->torrent_id, peer->ip, peer->port);
        close(peer->sockfd);
        peer->sockfd = -1;
        remove_peers_to_epoll(meta->peers, peer,
                meta->verbose ? meta->torrent_id : NULL);
        return;
    }
    if (meta->verbose)
        printf("%6s: msg: recv: %s:%d: handshake\n", meta->torrent_id, peer->ip,
                peer->port);
    peer->handshaked = 1;
}

void handle_bfill(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer)
{
    char cur = str[0];
    size_t b_len = (len - 1) * 8;
    size_t max = b_len - meta->nb_piece;
    size_t j = 0;
    char *bitfield = NULL;
    if (meta->verbose)
        bitfield = calloc(meta->nb_piece + 1, sizeof(char));
    for (uint32_t i = b_len; i > max; i--, j++)
    {
        peer->have[j] = ((cur >> (b_len - j) & 1));
        if (meta->verbose)
            bitfield[j] = peer->have[j] + '0';
        if (j % 8 == 0)
            cur = str[j / 8];
    }
    if (meta->verbose)
        printf("%6s: msg: recv: %s:%d: bitfield %s\n", meta->torrent_id,
                peer->ip, peer->port, bitfield);
    if (!peer->interested)
        interested(meta, peer);
    free(bitfield);
}

void handle_have(struct metainfo *meta, uint32_t len,
        char *str, struct peer *peer)
{
    if (len != 5)//err
        return;
    void *tmp = str + 5;
    uint32_t *index_p = tmp;
    peer->have[ntohl(*index_p)] = 1;
    if (meta->verbose)
        printf("%6s: msg: recv: %s:%d: have %u\n", meta->torrent_id, peer->ip,
              peer->port, ntohl(*index_p));
    if (!peer->interested)
        interested(meta, peer);
}

static int follow_piece(struct metainfo *meta, struct peer *peer)
{
    // Check if piece is complete
    for (size_t i = 0; i < meta->cur_piece->nb_blocks; i++)
    {
        if (meta->cur_piece->have[i] != 2)
        {
            return request(meta, peer);
        }
    }

    if (check_piece_string(meta, meta->cur_piece->id_piece,
                meta->cur_piece->buf, meta->cur_piece->piece_size))
    {
        write_piece(meta, meta->cur_piece->buf, meta->cur_piece->id_piece);
        meta->have[meta->cur_piece->id_piece] = 1;
        // Send have msg
    }
    // Free for next piece
    free(meta->cur_piece->buf);
    free(meta->cur_piece->have);
    meta->cur_piece->have = NULL;
    meta->cur_piece->buf = NULL;
    return request(meta, peer);
}

int handle_piece(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer)
{
    uint32_t f_len = len - 9;
    void *tmp = str + 5;
    uint32_t *id = tmp;
    if (meta->cur_piece->id_piece != ntohl(*id))//err
        return 1;
    tmp = str + 9;
    uint32_t *offset_BE = tmp;
    uint32_t offset = ntohl(*offset_BE);
    tmp = str + 13;
    char *piece = tmp;
    memcpy(meta->cur_piece->buf + offset, piece, f_len);
    meta->cur_piece->have[offset / 16384] = 2;
    if (meta->verbose)
        printf("%6s: msg: recv: %s:%d: piece %u %u\n", meta->torrent_id,
                peer->ip, peer->port, ntohl(*id), offset);
    return follow_piece(meta, peer);
}
