#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "bencode.h"
#include "integrity.h"

static struct be_node *create_str(char *str)
{
    struct be_node *ret = malloc(sizeof(struct be_node));
    struct be_string *intern = malloc(sizeof(struct be_string));
    ret->type = BE_STR;
    ret->element.str = intern;
    intern->length = strlen(str);
    intern->content = strdup(str);
    return ret;
}

static struct be_node *create_int(long long int num)
{
    struct be_node *ret = malloc(sizeof(struct be_node));
    ret->type = BE_INT;
    ret->element.num = num;
    return ret;
}

static struct be_node *create_list(struct be_node **list)
{
    struct be_node *ret = malloc(sizeof(struct be_node));
    ret->type = BE_LIST;
    ret->element.list = list;
    return ret;
}

static struct be_dict *create_dict_item(char *key, struct be_node *val)
{
    struct be_string *intern = malloc(sizeof(struct be_string));
    intern->length = strlen(key);
    intern->content = strdup(key);

    struct be_dict *ret = malloc(sizeof(struct be_dict));
    ret->key = intern;
    ret->val = val;
    return ret;
}

static struct be_node *create_dict(struct be_dict **dict)
{
    struct be_node *ret = malloc(sizeof(struct be_node));
    ret->type = BE_DICT;
    ret->element.dict = dict;
    return ret;
}
static struct be_node *fill_info_multiple(char *path)
{
    path = path;
    return NULL;
}

static struct be_node *create_info(char *path)
{
    struct be_node *piece_len = create_int(262144);
    struct be_node *pieces = create_str("tmp");
    struct be_dict **info = malloc(sizeof(struct be_dict*) * 5);
    info[0] = create_dict_item("piece length", piece_len);
    info[1] = create_dict_item("pieces", pieces);
    info[3] = NULL;
    info[4] = NULL;
    struct stat s;
    stat(path, &s);
    if (S_ISDIR(s.st_mode))
    {
        info[2] = create_dict_item("files", fill_info_multiple(path));
    }
    else
    {
        struct be_node *length = create_int(get_size_file(path));
        struct be_node *name = create_str(basename(path));
        info[2] = create_dict_item("length", length);
        info[3] = create_dict_item("name", name);
    }
    return create_dict(info);
}

static struct be_node *create_root(char *path)
{
    struct be_node *announce = create_str("http://localhost:6969/announce");
    struct be_node *comment = create_str("Thetoto > SnathanP");
    struct be_node *created_by = create_str("Bible White Corp.");
    time_t timestamp = time(NULL);
    struct be_node *time = create_int(timestamp);
    struct be_dict **root = malloc(sizeof(struct be_dict*) * 6);
    root[0] = create_dict_item("announce", announce);
    root[1] = create_dict_item("comment", comment);
    root[2] = create_dict_item("created by", created_by);
    root[3] = create_dict_item("creation date", time);
    root[4] = create_dict_item("info", create_info(path));
    root[5] = NULL;
    return create_dict(root);
}

void mktorrent(char *path)
{
    if (access(path, F_OK) == -1)
        return;
    struct be_node *root = create_root(path);
    size_t size;
    char *enc = be_encode(root, &size);
    be_free(root);

    char *torrent_path = calloc(strlen(path) + 20, 1);
    strcat(torrent_path, path);
    if (torrent_path[strlen(torrent_path) - 1] == '/')
        torrent_path[strlen(torrent_path) - 1] = '\0';
    strcat(torrent_path, ".torrent");
    FILE *f = fopen(torrent_path, "w");
    free(torrent_path);
    if (!f)
    {
        warn("Can't open file %s", path);
        return;
    }
    fwrite(enc, sizeof(char), size, f);
    free(enc);
    fclose(f);
}
