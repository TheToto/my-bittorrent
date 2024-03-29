#define _GNU_SOURCE
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <jansson.h>
#include <time.h>

#include "tracker.h"
#include "bencode.h"
#include "parser.h"

static char *concat_path (json_t *j_path, const char *name)
{
    char *res = calloc(strlen(name) + 1, sizeof(char));
    strcpy(res, name);

    size_t i;
    json_t *val;
    json_array_foreach(j_path, i, val)
    {
        if (!val || !json_is_string(val))
            return res;
        const char *tmp = json_string_value(val);
        res = realloc(res, strlen(res) + strlen(tmp) + 2);
        strcat(res, "/");
        strcat(res, tmp);
    }
    return res;
}

static struct metainfo *fill_multiple_files(struct metainfo *meta,
        json_t *j_files, const char *name)
{
    size_t nb_files = json_array_size(j_files);
    meta->files = calloc(nb_files + 1, sizeof(char*));
    meta->files_size = calloc(nb_files + 1, sizeof(size_t));

    size_t i;
    json_t *cur;
    json_array_foreach(j_files, i, cur)
    {
        json_t *j_path = json_object_get(cur, "path");
        if (!j_path || !json_is_array(j_path))
            return free_metainfo(meta);
        meta->files[i] = concat_path(j_path, name);

        json_t *j_size = json_object_get(cur, "length");
        if (!j_size || !json_is_integer(j_size))
            return free_metainfo(meta);
        meta->files_size[i] = json_integer_value(j_size);
    }
    return meta;
}

static struct metainfo *fill_files(struct metainfo *meta, json_t *j_info)
{
    json_t *j_files = json_object_get(j_info, "files");
    if (!j_files || !json_is_array(j_files))
    {
        // Unique file
        json_t *j_name = json_object_get(j_info, "name");
        if (!j_name || !json_is_string(j_name))
            return free_metainfo(meta);
        meta->files = calloc(2, sizeof(char*));
        meta->files[0] = strdup(json_string_value(j_name));

        json_t *j_size = json_object_get(j_info, "length");
        if (!j_size || !json_is_integer(j_size))
            return free_metainfo(meta);
        meta->files_size = calloc(2, sizeof(size_t));
        meta->files_size[0] = json_integer_value(j_size);

        return meta;
    }
    // Handle multiple files
    if (j_files && json_is_array(j_files))
    {
        json_t *j_name = json_object_get(j_info, "name");
        if (!j_name || !json_is_string(j_name))
            return free_metainfo(meta);

        return fill_multiple_files(meta, j_files, json_string_value(j_name));
    }
    return free_metainfo(meta);
}

static struct peer_list *init_peer_list(void)
{
    struct peer_list *peers = malloc(sizeof(struct peer_list));
    peers->capacity = 8;
    peers->size = 0;
    peers->list = malloc(sizeof(struct peer*) * 8);
    return peers;
}

static char *get_peerID(void)
{
    char *peer = calloc(21, sizeof(char));
    strcpy(peer, PEER_PREFIX);
    char str[13] =
    {
        0
    };

    srand(time(NULL));
    for (int i = 0; i < 12; i++)
        str[i] = rand() % 26 + 'a';
    strcat(peer, str);
    return peer;
}

struct metainfo *create_meta(json_t *json, int dump_peers, int verbose)
{
    struct metainfo *meta = calloc(1, sizeof(struct metainfo));
    meta->verbose = verbose;
    meta->dump_peers = dump_peers;
    meta->peers = init_peer_list();
    meta->peer_id = get_peerID();
    meta->cur_piece = calloc(1, sizeof(struct piece));

    json_t *j_an = json_object_get(json, "announce");
    if (!j_an || !json_is_string(j_an))
        warnx("No announce url");
    else
        meta->announce = strdup(json_string_value(j_an));

    json_t *j_info = json_object_get(json, "info");
    if (!j_info || !json_is_object(j_info))
        return free_metainfo(meta);

    json_t *j_pieces = json_object_get(j_info, "pieces");
    if (!j_pieces || !json_is_string(j_pieces))
        meta->pieces = NULL;
    else
        meta->pieces = strdup(json_string_value(j_pieces));

    json_t *j_size = json_object_get(j_info, "piece length");
    if (!j_size || !json_is_integer(j_size))
        return free_metainfo(meta);
    meta->piece_size = json_integer_value(j_size);

    return fill_files(meta, j_info);
}

void *free_metainfo(struct metainfo *meta)
{
    if (!meta)
        return NULL;
    if (meta->cur_piece->have)
        free(meta->cur_piece->have);
    if (meta->announce)
        free(meta->announce);
    if (meta->files)
    {
        for (size_t i = 0; meta->files[i]; i++)
            free(meta->files[i]);
        free(meta->files);
    }
    if (meta->peer_id)
        free(meta->peer_id);
    if (meta->files_size)
        free(meta->files_size);
    if (meta->pieces)
        free(meta->pieces);
    if (meta->info_hash)
        free(meta->info_hash);
    if (meta->have)
        free(meta->have);
    free(meta);
    return NULL;
}
