#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

char *fix_string(char *str, size_t size)
{
    char *buf = calloc(size * 6 + 1, sizeof(char));
    size_t j = 0;
    for (size_t i = 0; i < size; i++, j++)
    {
        if (str[i] < 0x20 || str[i] > 0x7E)
        {
            char tmp[10] =
            {
                0
            };
            sprintf(tmp, "U+00%02hhX", str[i]);
            strcat(buf, tmp);
            j += strlen(tmp) - 1;
        }
        else
        {
            buf[j] = str[i];
        }
    }
    return buf;
}

void unfix_string(struct metainfo *meta, unsigned char *sha1, size_t nb)
{
    char *str = meta->pieces;
    size_t i = 0;
    for (size_t k = 0; k < nb * 20; k++, i++) // Go to nb picece hash
    {
        if (str[i] == 'U' && str[i + 1] == '+' && str[i + 2] == '0'
                && str[i + 3] == '0' && str[i + 4] && str[i + 5])
            i += 5;
    }
    for (size_t j = 0; j < 20; i++, j++) // Get piece "nb" hash
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
            sha1[j] = hex;
            i += 5;
        }
        else
        {
            sha1[j] = str[i];
        }
    }
}

char *to_web_hex(char *str)
{
    char *buf = calloc(20 * 4 + 1, sizeof(char));
    size_t j = 0;
    for (size_t i = 0; i < 20; i++, j++)
    {
        char tmp[10] =
        {
            0
        };
        sprintf(tmp, "%%%02hhx", str[i]);
        strcat(buf, tmp);
        j += strlen(tmp) - 1;
    }
    free(str);
    return buf;
}

char *unfix_info_hash(char *str)
{
    static char hash[20];
    size_t j = 0;
    for (size_t i = 0; j < 20; i += 3, j++)
    {
        char tmp[3] =
        {
            str[i + 1], str[i + 2], '\0'
        };
        unsigned int hex;
        sscanf(tmp, "%02X", &hex);
        hash[j] = hex;
    }
    return hash;
}
