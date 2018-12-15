/**
* @file tracker.c
* @author louis.holleville
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


static unsigned char *decode_peers(size_t nb, const char *str)
{
    size_t i = 0;
    for (size_t k = 0; str[i] && k < nb * 6; k++, i++) // Go to nb picece hash
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

static size_t write_callback(char *ptr, size_t size, size_t nmemb,
        void *userdata)
{
    size_t all = size * nmemb;
    printf("RAW RESPONSE :\n");
    fwrite(ptr, size, nmemb, stdout);
    printf("\n");
    struct be_node *be = be_decode(ptr, all);
    if (!be)
    {
        printf("FAILED TO DECODE BENCODE\n");
    }
    json_t *json = to_json(be, NULL);
    json_t *j_peers = json_object_get(json, "peers");
    const char *peers = json_string_value(j_peers);
    unsigned char *peer;
    printf("DECODED IPS :\n");
    for (size_t i = 0; (peer = decode_peers(i, peers)); i++)
    {
        void *tmp = peer;
        struct in_addr *ip = tmp;
        tmp = peer + 4;
        uint16_t *port = tmp;
        char *s_ip = inet_ntoa(*ip);
        int real_port = ntohs(*port);
        printf("%s - %d\n", s_ip, real_port);
        free(peer);
    }
    be_free(be);
    dump_json(json);
    free_json(json);

    return all;
}

size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    size_t res = fread(buffer, size, nitems, userdata);
    return res;
}

char *init_tracker(char *url, struct metainfo *meta)
{
    if (!url)
        return NULL;
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        err(1, "init_tracker: cannot init curl-easy");
    }
    char *buf;
    char *request = get_tracker_request(meta);
    char errbuff[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_URL, url);
    //curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    //curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_REQUEST_TARGET, request);
    //curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    //curl_easy_setopt(curl, CURLOPT_READDATA, request);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errbuff);

    //char *res;
    free(request);
    if (curl_easy_perform(curl) == CURLE_OK)
    {
        //res = strdup(buf);

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }
    else
    {
        warnx("%s", errbuff);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }
}
