#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "parser.h"
#include "tracker.h"

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
        peer->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

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
        event.data.fd = peer->sockfd;
        epoll_ctl(peers->epoll, EPOLL_CTL_ADD, peer->sockfd, &event);
    }
}

void wait_event_epoll(struct metainfo *meta)
{
    int running = 1, event_count, i;
    size_t bytes_read;
    char read_buffer[READ_SIZE + 1];
    struct epoll_event event, events[MAX_EVENTS];
    while (1)
    {
        printf("\nPolling for input...\n");
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 30000);
        printf("%d ready events\n", event_count);
        for(i = 0; i < event_count; i++)
        {
            printf("Reading file descriptor '%d' -- ", events[i].data.fd);
            bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
            printf("%zd bytes read.\n", bytes_read);
            read_buffer[bytes_read] = '\0';
            printf("Read '%s'\n", read_buffer);

            if(!strncmp(read_buffer, "stop\n", 5))
                running = 0;
        }
    }
}
