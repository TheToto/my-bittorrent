#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "parser.h"

static size_t get_total_size(struct metainfo *meta)
{
    size_t acu = 0;
    for (size_t i = 0; meta->files[i]; i++)
    {
        acu += meta->files_size[i];
    }
    return acu;
}

static int check_piece(struct metainfo *meta, size_t nb)
{
    size_t start = nb * meta->piece_size;
    if (strlen(meta->pieces) < (nb + 1) * 20)
        return 0;
    unsigned char sha1[20];
    memcpy(sha1, meta->pieces + nb * 20, 20); // Get sha1 of piece
    size_t ptr = 0;
    size_t i = 0;
    for (size_t cur = 0; meta->files[i]; i++) // Get position of piece in files
    {
        ptr = 0;
        for (size_t j = 0; j < meta->files_size[i]; j++, cur++, ptr++)
        {
            if (start == cur)
                break;
        }
        if (start == cur)
            break;
    }
    unsigned char *piece = calloc(meta->piece_size + 1, sizeof(unsigned char));
    for (size_t k = 0; k < meta->piece_size; k++) // Get piece content
    {
        for (; k < meta->piece_size && meta->files[i]; i++)
        {
            int fd = open(meta->files[i], O_RDONLY);
            for (unsigned char c; k < meta->piece_size && read(fd, &c, 1); k++)
                piece[k] = c;
            close(fd);
        }
    }
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(piece, meta->piece_size, hash);
    free(piece);
    return memcmp(hash, sha1, 20) == 0;
}

int check_integrity(struct metainfo *meta)
{
    size_t total = get_total_size(meta);
    for (size_t i = 0; i * meta->piece_size < total; i++)
    {
        int res = check_piece(meta, i);
        if (!res)
            return 0;
    }
    return 1;
}
