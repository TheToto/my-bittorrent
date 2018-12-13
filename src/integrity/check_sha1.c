#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "parser.h"
#include "integrity.h"

static void unfix_string(struct metainfo *meta, unsigned char *sha1, size_t nb)
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
    free(piece);
    return memcmp(hash, sha1, 20) == 0; // Compare hashs
}

int check_piece(struct metainfo *meta, size_t nb)
{
    unsigned char *piece = malloc(meta->piece_size * sizeof(unsigned char));
    size_t size = get_piece(meta, piece, nb);
    return check_piece_string(meta, nb, piece, size);
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
