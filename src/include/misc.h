/**
* @file misc.h
* @author thomas.lupin
* @version 0.1
* @date 13-12-2018
* Header of string misc funcs
*/

#pragma once

#include "parser.h"

/// Convert a string to show unprintable char as U+00XX 
char *fix_string(char *str, size_t size);

/// Convert back a string to raw chars
void unfix_string(struct metainfo *meta, unsigned char *sha1, size_t nb);

/// COnvert a string to show unprintable char as %xx
char *to_web_hex(char *str);

/// COnvert back a string to raw chars
char *unfix_info_hash(char *str);
