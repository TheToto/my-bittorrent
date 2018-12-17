/**
* @file events.c
* @author louis.holleville thomas.lupin
* @version 0.1
* @date 17-12-2018
* Handling of events in epoll
*/

#include <arpa/inet.h>
#include <string.h>
#include "epoll.h"
#include "integrity.h"

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
    request(meta, peer);
}

void handle_have(uint32_t len, char *str, struct peer *peer)
{
    if (len != 5)//err
        return;
    void *tmp = str;
    uint32_t *index_p = tmp;
    peer->have[ntohl(*index_p)] = 1;
    printf("Peer %s have now piece %d !\n", peer->ip, ntohl(*index_p));
}

void handle_piece(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer)
{
    uint32_t f_len = len - 9;
    void *tmp = str + 1;
    uint32_t *id = tmp;
    if (meta->cur_piece->id_piece != ntohl(*id))//err
        return;
    tmp = str + 5;
    uint32_t *offset_BE = tmp;
    uint32_t offset = ntohl(*offset_BE);
    tmp = str + 9;
    char *piece = tmp;
    memcpy(meta->cur_piece->buf + offset, piece, f_len);
    meta->cur_piece->have[offset / 16384] = 2;
    printf("Receive piece %d, block %d from %s !",
            ntohl(*id), offset / 16384, peer->ip);

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
    }
    else
    {
        printf("Piece %ld failed integrity !\n", meta->cur_piece->id_piece);
    }

    free(meta->cur_piece->buf);
    free(meta->cur_piece->have);
    meta->cur_piece->buf = NULL;
    request(meta, peer);
}
