/**
* @file epoll.h
* @author thomas.lupin
* @version 0.1
* @date 13-12-2018
* Header of epoll management functions
*/

#pragma once

#include <stdint.h>
#include "parser.h"

/// Init epoll fd
void init_epoll(struct peer_list *peers);

/// Add a peer to the epoll
char add_peer_to_epoll(struct peer_list *peers, struct peer *peer,
        char *torrent_id);

/// Remove a peer from epoll
void remove_peers_to_epoll(struct peer_list *peers, struct peer *peer,
        char *torrent_id);

/// Main wait epoll function
void wait_event_epoll(struct metainfo *meta);

/// Handle handshake message
void handle_handshake(struct metainfo *meta, struct peer *peer,
        char *str, int bytes);

///Handling of message ID 5
void handle_bfill(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer);

///Handling have messages
void handle_have(struct metainfo *meta, uint32_t len,
        char *str, struct peer *peer);

///Handling piece messages
int handle_piece(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer);

///Switching functions between differents response ID of peers
int switch_events(struct metainfo *meta, struct peer *peer, char *str,
        uint32_t len);

/// Send a request to peer
int request(struct metainfo *meta, struct peer *peer);

/// Send a handshake to peer
char handshake(struct metainfo *meta, struct peer *peer);

/// Send a interest request to peer
void interested(struct metainfo *meta, struct peer *peer);

/// Send a not interest request to peer
void not_interested(struct metainfo *meta, struct peer *peer);

/// Send a keep alive request
void keep_alive(struct metainfo *meta, struct peer *peer);
