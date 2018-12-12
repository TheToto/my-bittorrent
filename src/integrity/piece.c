#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>

#include "parser.h"
#include "integrity.h"

size_t get_piece(struct metainfo *meta, unsigned char *piece, size_t nb)
{
    size_t start = nb * meta->piece_size;
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
    size_t k = 0;
    for (; k < meta->piece_size && meta->files[i]; i++) // Get piece content
    {
        int fd = open(meta->files[i], O_RDONLY);
        if (fd == -1)
        {
            warn("Cannot open file %s", meta->files[i]);
            return 0;
        }
        if (ptr)
        {
            lseek(fd, ptr, SEEK_SET);
            ptr = 0;
        }
        for (unsigned char c; k < meta->piece_size && read(fd, &c, 1); k++)
            piece[k] = c;
        close(fd);
    }
    return k;
}

void write_piece(struct metainfo *meta, unsigned char *piece, size_t nb)
{
    size_t start = nb * meta->piece_size;
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
    for (size_t k = 0; k < meta->piece_size && meta->files[i]; i++)
    {
        int fd = open(meta->files[i], O_WRONLY);
        if (fd == -1)
        {
            warn("Cannot open file %s", meta->files[i]);
            return;
        }
        if (ptr)
        {
            lseek(fd, ptr, SEEK_SET);
            ptr = 0;
        }
        for (size_t cur = 0; k < meta->piece_size
                && cur < meta->files_size[i]; k++, cur++)
        {
            write(fd, piece + k, 1);
        }
        close(fd);
    }
}

void create_files(struct metainfo *meta)
{
    char zero = '0';
    for (size_t i = 0; meta->files[i]; i++) // Get position of piece in files
    {
        int fd = open(meta->files[i], O_WRONLY | O_CREAT | O_TRUNC, 00644);
        if (fd == -1)
        {
            warn("Cannot open file %s", meta->files[i]);
            return;
        }
        for (size_t j = 0; j < meta->files_size[i]; j++)
        {
            write(fd, &zero, 1);
        }
    }
}
