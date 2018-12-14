#define _GNU_SOURCE
#include <string.h>
#include <sys/mman.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <jansson.h>

#include "bencode.h"
#include "parser.h"
#include "integrity.h"

static char *file_to_string(char *path, size_t *size)
{
    int fd = open(path, O_RDONLY);
    if (fd == -1)
        return NULL;
    *size = lseek(fd, 0, SEEK_END);
    char *data = mmap(0, *size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (data == MAP_FAILED)
        return NULL;
    return data;
}

static void dump_json(json_t *json)
{
    char *s = json_dumps(json, 4);
    puts(s);
    free(s);
}

static char *to_web_hex(char *str)
{
    char *buf = calloc(20 * 3 + 1, sizeof(char));
    size_t j = 0;
    for (size_t i = 0; i < 20; i++, j++)
    {
        if (str[i] < 0x20 || str[i] > 0x7E)
        {
            char tmp[10] =
            {
                0
            };
            sprintf(tmp, "%%%02hhx", str[i]);
            strcat(buf, tmp);
            j += strlen(tmp) - 1;
        }
        else
        {
            buf[j] = str[i];
        }
    }
    free(str);
    return buf;
}

struct metainfo *decode_torrent(char *path, int print)
{
    size_t size;
    char *buf = file_to_string(path, &size);
    if (buf == NULL)
    {
        warn("Failed to read file");
        return NULL;
    }

    struct be_node *be = be_decode(buf, size);
    munmap(buf, size);
    if (be == NULL)
    {
        warn("Failed to decode bencode");
        return NULL;
    }

    struct be_node *b_info;
    json_t *json = to_json(be, &b_info);

    if (print)
        dump_json(json);

    struct metainfo *meta = create_meta(json);

    size_t l_info;
    void *s_info = be_encode(b_info, &l_info);
    void *h_info = malloc(20 * sizeof(unsigned char));
    SHA1(s_info, l_info, h_info);
    meta->info_hash = to_web_hex(h_info);
    free(s_info);

    free_json(json);
    be_free(be);

    return meta;
}
