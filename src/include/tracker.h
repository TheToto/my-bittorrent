/**
* @file tracker.h
* @author louis.holleville
* @version 0.1
* @date 13-12-2018
* Header of trackers management functions
*/

#pragma once

#include "parser.h"

#define PEER_PREFIX "-MB2021-"
#define PEER_PORT 6881
#define COMPACT 1

char init_tracker(char *url, struct metainfo *meta);
char *get_tracker_request(struct metainfo *meta);
long get_portL(void);
