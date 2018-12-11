#pragma once

#include <jansson.h>
#include "bencode.h"

struct metainfo
{
    char *announce;
    char **files; // paths, NULL terminated array
    size_t *files_size; // Linked with above array
    size_t piece_size; // Size of a piece
    char *pieces; // SHA1 hashs (20 bytes each)
};

struct metainfo *decode_torrent(char *path, int print);
struct metainfo *create_meta(json_t *json);
void *free_metainfo(struct metainfo *meta);

json_t *to_json(struct be_node *be);
void free_json(json_t *json);
