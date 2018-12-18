#pragma once

#include "parser.h"

char *fix_string(char *str, size_t size);
void unfix_string(struct metainfo *meta, unsigned char *sha1, size_t nb);
char *to_web_hex(char *str);
char *unfix_info_hash(char *str);
