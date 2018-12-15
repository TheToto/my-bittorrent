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

#include "parser.h"
#include "tracker.h"

static size_t write_callback(char *ptr, size_t size, size_t nmemb,
        void *userdata)
{
    size_t all = size * nmemb;
    fwrite(ptr, size, nmemb, stdout);
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
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, request);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errbuff);

    char *res;
    printf("KEY=\n%s\n\n", request);//DEBUG
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
