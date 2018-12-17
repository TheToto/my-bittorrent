#pragma once

#include "parser.h"

void init_epoll(struct peer_list *peers);
void add_peers_to_epoll(struct peer_list *peers);
