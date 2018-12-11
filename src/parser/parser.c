#define _GNU_SOURCE
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <jansson.h>

#include "bencode.h"
#include "parser.h"

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

static struct metainfo *fill_files(struct metainfo *meta, json_t *j_info)
{
    json_t *j_files = json_object_get(j_info, "files");
    if (!j_files || !json_is_string(j_files))
    {
        // Unique file
        json_t *j_name = json_object_get(j_info, "name");
        if (!j_name || !json_is_string(j_name))
            return free_metainfo(meta);
        meta->files = calloc(2, sizeof(char*));
        meta->files[0] = strdup(json_string_value(j_name));

        json_t *j_size = json_object_get(j_info, "length");
        if (!j_size || !json_is_integer(j_size))
            return free_metainfo(meta);
        meta->files_size = calloc(2, sizeof(size_t));
        meta->files_size[0] = json_integer_value(j_size);

        return meta;
    }
    // Handle multiple files
    return free_metainfo(meta);
}

static struct metainfo *create_meta(json_t *json)
{
    struct metainfo *meta = calloc(1, sizeof(struct metainfo));

    json_t *j_an = json_object_get(json, "announce");
    if (!j_an || !json_is_string(j_an))
        return free_metainfo(meta);
    meta->announce = strdup(json_string_value(j_an));

    json_t *j_info = json_object_get(json, "info");
    if (!j_info || !json_is_object(j_info))
        return free_metainfo(meta);

    json_t *j_pieces = json_object_get(j_info, "pieces");
    if (!j_pieces || !json_is_string(j_pieces))
        return free_metainfo(meta);
    meta->pieces = strdup(json_string_value(j_pieces));

    json_t *j_size = json_object_get(j_info, "piece length");
    if (!j_size || !json_is_integer(j_size))
        return free_metainfo(meta);
    meta->piece_size = json_integer_value(j_size);

    return fill_files(meta, j_info);
}

void *free_metainfo(struct metainfo *meta)
{
    if (!meta)
        return NULL;
    if (meta->announce)
        free(meta->announce);
    if (meta->files)
    {
        for (size_t i = 0; meta->files[i]; i++)
            free(meta->files[i]);
        free(meta->files);
    }
    if (meta->files_size)
        free(meta->files_size);
    if (meta->pieces)
        free(meta->pieces);
    free(meta);
    return NULL;
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

    json_t *json = to_json(be);
    be_free(be);
    if (print)
    {
        char *s = json_dumps(json, 0);
        puts(s);
        free(s);
    }

    struct metainfo *meta = create_meta(json);

    free_json(json);

    return meta;
}
