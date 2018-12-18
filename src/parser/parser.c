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
#include "misc.h"

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

void dump_json(json_t *json)
{
    char *s = json_dumps(json, 4);
    puts(s);
    free(s);
}

static void compute_info_hash(struct be_node *b_info, struct metainfo *meta)
{
    size_t l_info;
    void *s_info = be_encode(b_info, &l_info);
    void *h_info = malloc(20 * sizeof(unsigned char));
    SHA1(s_info, l_info, h_info);
    meta->info_hash = to_web_hex(h_info);
    free(s_info);

    size_t total = get_total_size(meta);
    size_t nb_piece = total / meta->piece_size;
    if (total > nb_piece * meta->piece_size)
        nb_piece++;
    meta->nb_piece = nb_piece;

    meta->have = calloc(meta->nb_piece, sizeof(char));
}

struct metainfo *decode_torrent(char *path,
        int print, int dump_peers, int verbose)
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

    struct metainfo *meta = create_meta(json, dump_peers, verbose);
    compute_info_hash(b_info, meta);

    free_json(json);
    be_free(be);

    return meta;
}
