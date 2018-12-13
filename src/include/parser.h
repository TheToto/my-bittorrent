#pragma once

#include <jansson.h>
#include "bencode.h"

/// Info of a torrent file for easy use
struct metainfo
{
    char *announce;     ///< Announce URL (may be empty)
    char **files;       ///< Array of files paths, NULL terminated array
    size_t *files_size; ///< Array of files size in bytes
    size_t piece_size;  ///< Size of a piece
    char *pieces;       ///< SHA1 hashs (20 bytes for each piece)
};

/// Convert a torrent path to a metainfo struct
struct metainfo *decode_torrent(char *path, int print);

/// Create metainfo struct from json
struct metainfo *create_meta(json_t *json);

/// Free the metainfo struct
void *free_metainfo(struct metainfo *meta);

/// Convert a bencode node to json
json_t *to_json(struct be_node *be);

/// Free json
void free_json(json_t *json);

void mktorrent(char *path);
