#define _GNU_SOURCE
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/timerfd.h>

#include "parser.h"
#include "tracker.h"
#include "epoll.h"

#define MAX_EVENTS 500
#define READ_SIZE 262144

void init_epoll(struct peer_list *peers)
{
    peers->epoll = epoll_create1(0);
    int tfd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = tfd;
    peers->tfd = tfd;
    epoll_ctl (peers->epoll, EPOLL_CTL_ADD, tfd, &event);

    int flags = 0;
    struct itimerspec new_timer;
    new_timer.it_interval.tv_sec = 3;
    new_timer.it_interval.tv_nsec = 0;
    new_timer.it_value.tv_sec = 3;
    new_timer.it_value.tv_nsec = 0;
    struct itimerspec old_timer;
    timerfd_settime (tfd, flags, &new_timer, &old_timer);
    // DO NOT FORGET TO CLOSE THIS FD
}

void add_peer_to_epoll(struct peer_list *peers, struct peer *peer)
{
    peer->sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(peer->port);
    if (inet_pton(AF_INET, peer->ip, &serv_addr.sin_addr) <= 0)
    {
        warnx("Unsuported IP : %s\n", peer->ip);
        close(peer->sockfd);
        peer->sockfd = -1;
        remove_peers_to_epoll(peers, peer);
        return;
    }
    if (connect(peer->sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        warnx("Connect failed : %s\n", peer->ip);
        close(peer->sockfd);
        peer->sockfd = -1;
        remove_peers_to_epoll(peers, peer);
        return;
    }

    struct epoll_event event;
    event.events = EPOLLIN; // EPOLLIN : read, EPOLLOUT : write
    event.data.ptr = peer;
    epoll_ctl(peers->epoll, EPOLL_CTL_ADD, peer->sockfd, &event);
}

void remove_peers_to_epoll(struct peer_list *peers, struct peer *peer)
{
    size_t index = 0;
    for (; index < peers->size && peers->list[index] != peer; index++);
    if (index >= peers->size)
        return;
    peers->list[index] = peers->list[peers->size - 1];
    peers->size -= 1;
    peers->list[peers->size] = 0;
    epoll_ctl(peers->epoll, EPOLL_CTL_DEL, peer->sockfd, NULL);
    free(peer->ip);
    free(peer->have);
    free(peer);
}

static int handle_type_req(struct metainfo *meta, struct peer *peer,
        char *str, size_t bytes)
{
    if (peer->handshaked == 0)
    {
        handle_handshake(meta, peer, str, bytes);
        return 1;
    }
    for (size_t i = 0; i < bytes; i++)
    {
        printf("%02hhX ", str[i]);
    }
    printf("\n");
    void *tmp = str;
    uint32_t *len = tmp;
    return switch_events(meta, peer, str, ntohl(*len));
}

static ssize_t check_size(char *buf, ssize_t size, struct peer *peer)
{
    if (peer->handshaked == 0)
        return buf[0] + 49;
    void *tmp = buf;
    uint32_t *len = tmp;
    uint32_t real_len = ntohl(*len);
    printf("%zd bytes read, expedted %u. Type %d\n\n",
            size, real_len + 4, buf[4]);
    if (real_len + 4 <= size)
        return real_len + 4;
    return 0;
}

static void check_peers(struct metainfo *meta)
{
    if (meta->peers->size * 4 <= (meta->peers->initial_nb))
        init_tracker(meta->announce, meta);
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
            if (events[i].data.fd == meta->peers->tfd)
            {
                size_t s = 0;
                read (meta->peers->tfd, &s, sizeof (s));
                printf("Timer event : %zu !\n", s);
                continue;
            }
            struct peer *peer = events[i].data.ptr;
            printf("Reading file descriptor '%d' -- ", peer->sockfd);

            // only peek here to check if download is complete
            ssize_t bytes_read = recv(peer->sockfd, read_buffer,
                    READ_SIZE, MSG_PEEK);
            if (bytes_read == -1)
            {
                perror("Recv fail");
                continue;
            }
            else if (bytes_read == 0)
            {
                remove_peers_to_epoll(meta->peers, peer);
                continue;
            }
            for (ssize_t i = bytes_read; i < READ_SIZE + 1; i++)
                read_buffer[i] = '\0';

            ssize_t to_read = check_size(read_buffer, bytes_read, peer);
            if (to_read == 0)
            {
                printf("Uncomplete msg...\n");
                continue;
            }
            // Really take data here
            bytes_read = read(peer->sockfd, read_buffer, to_read);
            if (bytes_read == -1)
            {
                perror("Read fail");
                continue;
            }
            for (ssize_t i = bytes_read; i < READ_SIZE + 1; i++)
                read_buffer[i] = '\0';
            if (bytes_read != to_read)
                errx(1, "NOOOOOO");
            if (!handle_type_req(meta, peer, read_buffer, bytes_read))
                return;
        }
        check_peers(meta);
    }
}
