#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "parser.h"

static void unfix_string(struct metainfo *meta, unsigned char *sha1, size_t nb)
{
    char *str = meta->pieces + nb * 20;
    size_t i = 0;
    for (size_t j = 0; j < 20; i++, j++)
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
    unsigned char sha1[20];
    unfix_string(meta, sha1, nb);
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
    unsigned char *piece = malloc(meta->piece_size * sizeof(unsigned char));
    size_t k = 0;
    while (k < meta->piece_size && meta->files[i]) // Get piece content
    {
        for (; k < meta->piece_size && meta->files[i]; i++)
        {
            int fd = open(meta->files[i], O_RDONLY);
            if (ptr)
            {
                lseek(fd, ptr, SEEK_SET);
                ptr = 0;
            }
            for (unsigned char c; k < meta->piece_size && read(fd, &c, 1); k++)
                piece[k] = c;
            close(fd);
        }
    }
    unsigned char hash[20];
    SHA1(piece, k, hash);
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
