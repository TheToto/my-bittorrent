#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <jansson.h>

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

static struct be_node *path_to_list(char *path)
{
    size_t capacity = 8;
    size_t size = 0;
    struct be_node **list = malloc(capacity * sizeof(struct be_node*));

    size_t i = 0;
    for (; path[i]; i++)
    {
        if (size == capacity - 1)
        {
            capacity *= 2;
            list = realloc(list, capacity * sizeof(struct be_node*));
        }
        char *tmp = calloc(256, sizeof(char));;
        for (size_t j = 0; path[i] && path[i] != '/'; i++, j++)
        {
            tmp[j] = path[i];
        }
        list[size] = create_str(tmp);
        free(tmp);
        size++;
    }

    list[size] = NULL;
    return create_list(list);
}

static struct be_node **add_dir(struct be_node **list, size_t *size,
        size_t *capacity, char *path)
{
    DIR *dir;
    if (strlen(path))
        dir = opendir(path);
    else
        dir = opendir(".");
    struct dirent *dp;
    while ((dp = readdir(dir)) != NULL)
    {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            continue;
        char *new_path = calloc(strlen(path) + 257, sizeof(char));
        if (strlen(path))
        {
            strcat(new_path, path);
            strcat(new_path, "/");
        }
        strcat(new_path, dp->d_name);

        if (dp->d_type & DT_DIR)
        {
            list = add_dir(list, size, capacity, new_path);
        }
        else
        {
            if (*capacity - 1 == *size)
            {
                *capacity *= 2;
                list = realloc(list, *capacity * sizeof(struct be_node*));
            }
            struct be_dict **file = malloc(sizeof(struct be_dict) * 3);
            struct be_node *length = create_int(get_size_file(new_path));
            struct be_node *path = path_to_list(new_path);
            file[0] = create_dict_item("length", length);
            file[1] = create_dict_item("path", path);
            file[2] = NULL;
            list[*size] = create_dict(file);
            (*size)++;
        }
        free(new_path);
    }
    closedir(dir);
    return list;
}

static struct be_node *create_info(char *path)
{
    struct be_dict **info = malloc(sizeof(struct be_dict*) * 5);
    struct stat s;
    stat(path, &s);
    if (S_ISDIR(s.st_mode))
    {
        size_t capacity = 8;
        size_t size = 0;
        struct be_node **list = malloc(capacity * sizeof(struct be_node*));
        char *backup = get_current_dir_name();
        chdir(path);
        list = add_dir(list, &size, &capacity, "");
        chdir(backup);
        free(backup);
        list[size] = NULL;
        info[0] = create_dict_item("files", create_list(list));
    }
    else
    {
        struct be_node *length = create_int(get_size_file(path));
        info[0] = create_dict_item("length", length);
    }
    struct be_node *piece_len = create_int(262144);
    struct be_node *name = create_str(basename(path));
    struct be_node *tmp = create_str("tmp");
    info[1] = create_dict_item("piece length", piece_len);
    info[2] = create_dict_item("name", name);
    info[3] = create_dict_item("pieces", tmp);;
    info[4] = NULL;
    return create_dict(info);
}

static struct be_node *create_root(char *path)
{
    struct be_node *announce = create_str("http://localhost:6969/announce");
    struct be_node *comment = create_str("Thetoto < SnathanP");
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

static void create_hash(struct be_node *r, char *path)
{
    // Compute hash (a bit ugly)
    json_t *json = to_json(r, NULL);
    struct metainfo *meta = create_meta(json, 0, 0);
    free_json(json);
    size_t s;
    char *h = compute_integrity(path, meta, &s);
    free(r->element.dict[4]->val->element.dict[3]->val->element.str->content);
    r->element.dict[4]->val->element.dict[3]->val->element.str->content = h;
    r->element.dict[4]->val->element.dict[3]->val->element.str->length = s;
    free_metainfo(meta);
}

void mktorrent(char *path)
{
    if (access(path, F_OK) == -1)
        errx(1, "File/Folder %s does not exist", path);
    if (path[strlen(path) - 1] == '/')
        path[strlen(path) - 1] = '\0';
    char *torrent_path = calloc(strlen(path) + 20, 1);
    strcat(torrent_path, basename(path));
    strcat(torrent_path, ".torrent");
    if (access(torrent_path, F_OK) != -1)
    {
        free(torrent_path);
        errx(1, "File %s.torrent already exist", path);
    }
    struct be_node *r = create_root(path);

    create_hash(r, path);

    size_t size;
    char *enc = be_encode(r, &size);
    be_free(r);
    if (!enc)
        errx(1, "Can't encode be_node");

    FILE *f = fopen(torrent_path, "w");
    free(torrent_path);
    if (!f)
        err(1, "Can't open file %s", path);
    fwrite(enc, sizeof(char), size, f);
    free(enc);
    fclose(f);
    exit(0);
}
