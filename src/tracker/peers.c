#define _DEFAULT_SOURCE
#include <curl/curl.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <jansson.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "parser.h"
#include "tracker.h"

static char *build_handshake(struct metainfo *meta)
{
    static char hand[68];
    hand[0] = 19;
    memcpy(hand + 1, "BitTorrent protocol", 19);
    memcpy(hand + 20, "\0\0\0\0\0\0\0\0", 8);
    memcpy(hand + 28, meta->info_hash, 20);
    memcpy(hand + 48, meta->peer_id, 20);
    return hand;
}

void handshake(struct metainfo *meta, struct peer *peer)
{
    printf("Handshake to %s\n", peer->ip);
    peer->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(peer->port);
    if (inet_pton(AF_INET, peer->ip, &serv_addr.sin_addr) <= 0)
    {
        warnx("Unsuported IP : %s\n", peer->ip);
        close(peer->sockfd);
        return;
    }
    if (connect(peer->sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        warnx("Connect failed : %s\n", peer->ip);
        close(peer->sockfd);
        return;
    }
    send(peer->sockfd, build_handshake(meta), 68, 0);
    printf("Handshake sent !\n");
    char buffer[1024] = { 0 };
    read(peer->sockfd, buffer, 1024);
    printf("Response : %s\n\n", buffer);

    close(peer->sockfd);
}
