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

void handle_handshake(struct metainfo *meta, struct peer *peer,
        char *str, int bytes)
{
    if (bytes != 49 + str[0])
    {
        warnx("Incorrect response (lenght : %d, expected : %d)!\n",
                bytes, 49 + str[0]);
        close(peer->sockfd);
        peer->sockfd = -1;
        // REMOVE PEER
        return;
    }
    if (memcmp(unfix_info_hash(meta->info_hash),
                str + str[0] + 9, 20) != 0)
    {
        warnx("Incorrect response (hash_info)!\n");
        close(peer->sockfd);
        peer->sockfd = -1;
        // REMOVE PEER
        return;
    }
    printf("Handskake received from %s !\nPeer id : %s\n\n",
            peer->ip, str + str[0] + 29);
    peer->handshaked = 1;
}

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
    printf("Bitfield recv from %s !\n", peer->ip);
    if (!peer->interested)
        interested(meta, peer);
}

void handle_have(struct metainfo *meta, uint32_t len,
        char *str, struct peer *peer)
{
    if (len != 5)//err
        return;
    void *tmp = str + 5;
    uint32_t *index_p = tmp;
    peer->have[ntohl(*index_p)] = 1;
    printf("Peer %s have now piece %d !\n", peer->ip, ntohl(*index_p));
    if (!peer->interested)
        interested(meta, peer);
}

static void follow_piece(struct metainfo *meta, struct peer *peer)
{
    // Check if piece is complete
    for (size_t i = 0; i < meta->cur_piece->nb_blocks; i++)
    {
        if (meta->cur_piece->have[i] != 2)
        {
            request(meta, peer);
            return;
        }
    }

    if (check_piece_string(meta, meta->cur_piece->id_piece,
                meta->cur_piece->buf, meta->cur_piece->piece_size))
    {
        write_piece(meta, meta->cur_piece->buf, meta->cur_piece->id_piece);
        meta->have[meta->cur_piece->id_piece] = 1;
        printf("Piece %ld completed !\n", meta->cur_piece->id_piece);
        // Send have msg
    }
    else
    {
        printf("Piece %ld failed integrity !\n", meta->cur_piece->id_piece);
    }
    // Free for next piece
    free(meta->cur_piece->buf);
    free(meta->cur_piece->have);
    meta->cur_piece->buf = NULL;
    request(meta, peer);
}

void handle_piece(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer)
{
    uint32_t f_len = len - 9;
    void *tmp = str + 5;
    uint32_t *id = tmp;
    if (meta->cur_piece->id_piece != ntohl(*id))//err
        return;
    tmp = str + 9;
    uint32_t *offset_BE = tmp;
    uint32_t offset = ntohl(*offset_BE);
    tmp = str + 13;
    char *piece = tmp;
    memcpy(meta->cur_piece->buf + offset, piece, f_len);
    meta->cur_piece->have[offset / 16384] = 2;
    printf("Receive piece %d, block %d from %s !",
            ntohl(*id), offset / 16384, peer->ip);

    follow_piece(meta, peer);
}
