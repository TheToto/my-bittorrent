/**
* @file parser.h
* @author thomas.lupin
* @version 0.1
* @date 13-12-2018
* Header of parsing functions
*/


#pragma once

#include <jansson.h>
#include "bencode.h"

/// Info of a torrent file for easy use
struct metainfo
{
    char *announce;         ///< Announce URL (may be empty)
    char **files;           ///< Array of files paths, NULL terminated array
    size_t *files_size;     ///< Array of files size in bytes
    char *torrent_id;       ///< Torrent ID for verbose
    size_t piece_size;      ///< Size of a piece
    size_t nb_piece;        ///< Number of pieces
    char *pieces;           ///< SHA1 hashs (20 bytes for each piece)
    char *have;             ///< Booleans if have index piece
    char *info_hash;        ///< 20 bytes hash of info dict
    char *peer_id;          ///< Peer id of the client
    struct peer_list *peers;///< The peer list struct
    char verbose;           ///< Bool if verbose is on
    char dump_peers;        ///< Bool if -d is on
    struct piece *cur_piece;///< Current download
};

/// Struct of current piece download
struct piece
{
    char *have;             ///< 0 if we have block id, size = piece_size/16KB
    size_t id_piece;        ///< Index of piece
    size_t piece_size;      ///< Size of the current piece usefull if last
    size_t nb_blocks;       ///< Number of blocks
    unsigned char *buf;     ///< Buffer of size
};

/// List of peers attached to a metainfo struct
struct peer_list
{
    struct peer **list;     ///< List of peer struct
    size_t size;            ///< The size of the list
    size_t capacity;        ///< The capacity of the list
    size_t initial_nb;      ///< Initial number of peers
    int epoll;              ///< Epoll fd
    int tfd;                ///< Timer fd
};

/// Struct of a peer in the peer list
struct peer
{
    char *ip;               ///< Ip of the peer
    int port;               ///< Port of the peer
    int sockfd;             ///< FD of socket
    char handshaked;        ///< Bool if the peer has handshaked
    char state;             ///< Flag for choking status
    char *have;             ///< Booleans if have index piece
    char interested;        ///< Has been notified of interest
    char has_contact;       ///< If we receive a request since last timeout
};

/// Convert a torrent path to a metainfo struct
struct metainfo *decode_torrent(char *path,
        int print, int dump_peers, int verbose);

/// Create metainfo struct from json
struct metainfo *create_meta(json_t *json, int dump_peers, int verbose);

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

///free all structures and close all fd
void unleash_void(struct metainfo *meta);
