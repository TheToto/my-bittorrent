#pragma once

#include <stdint.h>
#include "parser.h"

void init_epoll(struct peer_list *peers);
void add_peers_to_epoll(struct peer_list *peers);
void wait_event_epoll(struct metainfo *meta);

///Handling of message ID 5
void handle_bfill(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer);

///Handling have messages
void handle_have(uint32_t len, char *str, struct peer *peer);

///Handling piece messages
void handle_piece(struct metainfo *meta, uint32_t len, char *str,
        struct peer *peer);

///Switching functions between differents response ID of peers
void switch_events(struct metainfo *meta, struct peer *peer, char *str,
        uint32_t len);
