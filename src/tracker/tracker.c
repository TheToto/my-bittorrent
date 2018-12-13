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
#include <stdio.h>

static size_t write_callback(char *ptr, size_t size, size_t nmemb,
        void *userdata)
{
    size_t t_size = size * nmemb;
    char *str = userdata;
    for (size_t i = 0; i < t_size; i++)
    {
        printf("%c", str[i]);
    }
    printf("\n");
    fflush(stdout);
    return t_size;
}

char *init_tracker(char *url)
{
    if (!url)
        return NULL;
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        err(1, "init_tracker: cannot init curl-easy");
    }
    char *buf;
    char errbuff[CURL_ERROR_SIZE];

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errbuff);

    char *res;
    if (curl_easy_perform(curl) == CURLE_OK)
    {
        res = strdup(buf);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return res;
    }
    else
    {
        warnx("%s", errbuff);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return NULL;
    }
}
