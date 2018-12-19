/**
* @file clean.c
* @author louis.holleville
* @version 0.1
* @date 19-12-2018
* Freeing what has to be freed
*/

#include <stdlib.h>
#include <unistd.h>
#include "parser.h"
#include "epoll.h"

static void unleash_peers(struct peer_list *peers)
{
    if (!peers)
        return;
    size_t max = peers->size;
    for (size_t i = 0; i < max; i++)
        remove_peers_to_epoll(peers, peers->list[0]);
    free(peers->list);
    close(peers->tfd);
    close(peers->epoll);
    free(peers);
}

static void unleash_files(char **files)
{
    if (!files)
        return;
    for (size_t i = 0; files[i]; i++)
        free(files[i]);
    free(files);
}

static void clear_piece(struct piece *cur_piece)
{
    if (!cur_piece)
        return;
    free(cur_piece->buf);
    free(cur_piece->have);
    free(cur_piece);
}

void unleash_void(struct metainfo *meta)
{
   if (!meta)
       return;
   unleash_peers(meta->peers);
   unleash_files(meta->files);
   clear_piece(meta->cur_piece);
   free(meta->announce);
   free(meta->peer_id);
   free(meta->files_size);
   free(meta->pieces);
   free(meta->have);
   free(meta->info_hash);
   free(meta);
}
