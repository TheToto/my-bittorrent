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

char *init_tracker(char *url)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        err(1, "init_tracker: cannot init curl-easy");
    }
    char *buf;
    char errbuff[CURL_ERROR_SIZE];

    curl_easy_setopt(curl, CURLOPT_URL, url);
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
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
