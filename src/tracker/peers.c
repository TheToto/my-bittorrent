#define _DEFAULT_SOURCE
#include <curl/curl.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <jansson.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "parser.h"
#include "tracker.h"

void handshake(struct metainfo *meta)
{
    char handshake[68];
    handshake[0] = 19;
    memcpy(handshake + 1, "BitTorrent protocol", 19);
    memcpy(handshake + 20, "\0\0\0\0\0\0\0\0", 8);
    memcpy(handshake + 28, meta->info_hash, 20);
    memcpy(handshake + 48, meta->peer_id, 20);
}
