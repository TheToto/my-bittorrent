/**
* @file tracker.c
* @author louis.holleville thomas.lupin
* @version 0.1
* @date 13-12-2018
* Management of initial tracker connexion
*/
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
#include "epoll.h"

static unsigned char *decode_peers(size_t nb, const char *str)
{
    size_t i = 0;
    for (size_t k = 0; k < nb * 6 && str[i]; k++, i++) // Go to nb picece hash
    {
        if (str[i] == 'U' && str[i + 1] == '+' && str[i + 2] == '0'
                && str[i + 3] == '0' && str[i + 4] && str[i + 5])
            i += 5;
    }
    if (!str[i])
        return NULL;
    unsigned char *peer = calloc(6, sizeof(char));
    for (size_t j = 0; j < 6; i++, j++) // Get piece "nb" hash
    {
        if (str[i] == 'U' && str[i + 1] == '+' && str[i + 2] == '0'
                && str[i + 3] == '0' && str[i + 4] && str[i + 5])
        {
            char tmp[3] =
            {
                str[i + 4], str[i + 5], '\0'
            };
            unsigned int hex;
            sscanf(tmp, "%02X", &hex);
            peer[j] = hex;
            i += 5;
        }
        else
        {
            peer[j] = str[i];
        }
    }
    return peer;
}

static void add_to_peer_list(struct metainfo *meta, unsigned char *peer)
{
    void *tmp = peer;
    struct in_addr *ip = tmp;

    tmp = peer + 4;
    uint16_t *port = tmp;

    char *real_ip = inet_ntoa(*ip);
    int real_port = ntohs(*port);

    if (meta->dump_peers)
        printf("%s:%d\n", real_ip, real_port);


    struct peer_list *peers = meta->peers;
    if (peers->size == peers->capacity - 1)
    {
        peers->capacity *= 2;
        peers->list = realloc(peers->list,
                peers->capacity * sizeof(struct peer));
    }
    struct peer *new = malloc(sizeof(struct peer));
    new->ip = strdup(real_ip);
    new->port = real_port;
    new->sockfd = -1;
    new->handshaked = 0;
    new->interested = 0;
    new->state = 1;
    new->has_contact = 0;
    new->have = calloc(meta->nb_piece, sizeof(char));
    peers->list[peers->size] = new;
    peers->size += 1;
    if (add_peer_to_epoll(peers, new, meta->verbose ? meta->torrent_id : NULL))
        if (!handshake(meta, new))
            remove_peers_to_epoll(meta->peers, new,
                    meta->verbose ? meta->torrent_id : NULL);
}

static void jsoning(char *ptr, size_t all, struct metainfo *meta)
{
    struct be_node *be = be_decode(ptr, all);
    if (!be)
        errx(1, "FAILED TO DECODE BENCODE (announce response)");

    json_t *json = to_json(be, NULL);
    json_t *j_fail = json_object_get(json, "failure reason");
    if (j_fail)
    {
        printf("Tracker failure : %s\n", json_string_value(j_fail));
    }
    else
    {
        json_t *j_peers = json_object_get(json, "peers");
        const char *peers = json_string_value(j_peers);

        unsigned char *peer;
        for (size_t i = 0; i < 10 && (peer = decode_peers(i, peers)); i++)
        {
            add_to_peer_list(meta, peer);
            free(peer);
        }
        meta->peers->initial_nb = meta->peers->size;
    }
    be_free(be);
    free_json(json);
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb,
        void *userdata)
{
    size_t all = size * nmemb;
    struct metainfo *meta = userdata;

    if (meta->verbose)
    {
        printf("RAW RESPONSE :\n");
        fwrite(ptr, size, nmemb, stdout);
        printf("\n");
    }

    jsoning(ptr, all, meta);

    return all;
}

size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    size_t res = fread(buffer, size, nitems, userdata);
    return res;
}

char init_tracker(char *url, struct metainfo *meta)
{
    if (!url)
        return 0;
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        err(1, "init_tracker: cannot init curl-easy");
    }
    char *request = get_tracker_request(meta);
    char errbuff[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_REQUEST_TARGET, request);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, meta);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errbuff);

    free(request);
    int res = curl_easy_perform(curl) == CURLE_OK;
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return res;
}
