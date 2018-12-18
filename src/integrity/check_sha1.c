#define _GNU_SOURCE
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "parser.h"
#include "integrity.h"
#include "misc.h"

size_t get_total_size(struct metainfo *meta)
{
    size_t acu = 0;
    for (size_t i = 0; meta->files[i]; i++)
    {
        acu += meta->files_size[i];
    }
    return acu;
}

int check_piece_string(struct metainfo *meta, size_t nb,
        unsigned char *piece, size_t piece_size)
{
    unsigned char sha1[20];
    unfix_string(meta, sha1, nb); // Get piece nb hash

    unsigned char hash[20];
    SHA1(piece, piece_size, hash); // Compute piece hash
    return memcmp(hash, sha1, 20) == 0; // Compare hashs
}

int check_piece(struct metainfo *meta, size_t nb)
{
    unsigned char *piece = malloc(meta->piece_size * sizeof(unsigned char));
    size_t size = get_piece(meta, piece, nb);
    int ret = check_piece_string(meta, nb, piece, size);
    free(piece);
    return ret;
}

int check_integrity(struct metainfo *meta)
{
    int ret = 1;
    for (size_t i = 0; i < meta->nb_piece; i++)
    {
        int res = check_piece(meta, i);
        if (!res)
        {
            meta->have[i] = 0;
            ret = 0;
        }
        else
            meta->have[i] = 1;
    }
    return ret;
}

char *compute_integrity(char *path, struct metainfo *meta, size_t *size)
{
    char *dup = strdup(path);
    char *backup = get_current_dir_name();
    chdir(dirname(dup));

    size_t total = get_total_size(meta);
    size_t nb_piece = total / meta->piece_size;
    if (total > nb_piece * meta->piece_size)
        nb_piece++;

    meta->nb_piece = nb_piece;
    *size = meta->nb_piece * 20;
    char *pieces = malloc(*size * sizeof(char));
    for (size_t i = 0; i < meta->nb_piece; i++)
    {
        unsigned char *piece = malloc(meta->piece_size * sizeof(unsigned char));
        int piece_size = get_piece(meta, piece, i);

        unsigned char hash[20];
        SHA1(piece, piece_size, hash); // Compute piece hash
        free(piece);

        memcpy(pieces + i * 20, hash, 20); //
    }
    chdir(backup);
    free(backup);
    free(dup);
    return pieces;
}
