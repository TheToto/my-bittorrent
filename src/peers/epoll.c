#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "parser.h"
#include "tracker.h"
#include "epoll.h"

#define MAX_EVENTS 500
#define READ_SIZE 262144

void init_epoll(struct peer_list *peers)
{
    peers->epoll = epoll_create1(0);
}

void add_peers_to_epoll(struct peer_list *peers)
{
    for (size_t i = 0; i < peers->size; i++)
    {
        struct peer *peer = peers->list[i];
        peer->sockfd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(peer->port);
        if (inet_pton(AF_INET, peer->ip, &serv_addr.sin_addr) <= 0)
        {
            warnx("Unsuported IP : %s\n", peer->ip);
            close(peer->sockfd);
            peer->sockfd = -1;
            // REMOVE PEER
            continue;
        }
        if (connect(peer->sockfd, (struct sockaddr *)&serv_addr,
                    sizeof(serv_addr)) < 0)
        {
            warnx("Connect failed : %s\n", peer->ip);
            close(peer->sockfd);
            peer->sockfd = -1;
            // REMOVE PEER
            continue;
        }

        struct epoll_event event;
        event.events = EPOLLIN; // EPOLLIN : read, EPOLLOUT : write
        event.data.ptr = peer;
        epoll_ctl(peers->epoll, EPOLL_CTL_ADD, peer->sockfd, &event);
    }
}

static void handle_type_req(struct metainfo *meta, struct peer *peer,
        char *str, size_t bytes)
{
    if (peer->handshaked == 0)
    {
        handle_handshake(meta, peer, str, bytes);
        return;
    }
    for (size_t i = 0; i < bytes; i++)
    {
        printf("%02hhX ", str[i]);
    }
    printf("\n");
    void *tmp = str;
    uint32_t *len = tmp;
    switch_events(meta, peer, str, ntohl(*len));
}

static size_t check_size(char *buf, size_t size, struct peer *peer)
{
    if (size == 0)
        return 0;
    if (peer->handshaked == 0)
        return buf[0] + 49;
    void *tmp = buf;
    uint32_t *len = tmp;
    uint32_t real_len = ntohl(*len);
    printf("%zd bytes read, expedted %u.\n\n", size, real_len + 4);
    if (real_len + 4 <= size)
        return real_len + 4;
    return 0;
}

void wait_event_epoll(struct metainfo *meta)
{
    int event_count;
    char read_buffer[READ_SIZE + 1];
    struct epoll_event events[MAX_EVENTS];
    while (1)
    {
        printf("\nWaiting for event...\n");
        event_count = epoll_wait(meta->peers->epoll, events, MAX_EVENTS, 30000);
        printf("%d ready events\n", event_count);
        for(int i = 0; i < event_count; i++)
        {
            struct peer *peer = events[i].data.ptr;
            printf("Reading file descriptor '%d' -- ", peer->sockfd);

            // only peek here to check if download is complete
            size_t bytes_read = recv(peer->sockfd, read_buffer,
                    READ_SIZE, MSG_PEEK);
            for (size_t i = bytes_read; i < READ_SIZE + 1; i++)
                read_buffer[i] = '\0';

            size_t to_read = check_size(read_buffer, bytes_read, peer);
            if (to_read == 0)
            {
                printf("Uncomplete msg...\n");
                continue;
            }
            // Really take data here
            bytes_read = read(peer->sockfd, read_buffer, to_read);
            for (size_t i = bytes_read; i < READ_SIZE + 1; i++)
                read_buffer[i] = '\0';
            if (bytes_read != to_read)
                errx(1, "NOOOOOO");
            handle_type_req(meta, peer, read_buffer, bytes_read);
        }
    }
}
