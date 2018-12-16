#pragma once

#include <jansson.h>
#include "bencode.h"

/// Info of a torrent file for easy use
struct metainfo
{
    char *announce;         ///< Announce URL (may be empty)
    char **files;           ///< Array of files paths, NULL terminated array
    size_t *files_size;     ///< Array of files size in bytes
    size_t piece_size;      ///< Size of a piece
    char *pieces;           ///< SHA1 hashs (20 bytes for each piece)
    char *info_hash;        ///< 20 bytes hash of info dict
    char *peer_id;
    struct peer_list *peers;///< The peer list struct
};

/// List of peers attached to a metainfo struct
struct peer_list
{
    struct peer **list;
    size_t size;            ///< The size of the list
    size_t capacity;        ///< The capacity of the list
};

struct peer
{
    char *ip;
    int port;
    //char *peer_id;
    //enum state;
};

/// Convert a torrent path to a metainfo struct
struct metainfo *decode_torrent(char *path, int print);

/// Create metainfo struct from json
struct metainfo *create_meta(json_t *json);

/// Free the metainfo struct
void *free_metainfo(struct metainfo *meta);

/// Convert a bencode node to json
json_t *to_json(struct be_node *be, struct be_node **info);

/// Print json
void dump_json(json_t *json);

/// Free json
void free_json(json_t *json);

/// mktorrent fonction (path can be file or directory)
void mktorrent(char *path);
