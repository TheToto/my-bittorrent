#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <libgen.h>

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

static size_t get_size_file(char *path)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return 0;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fclose(f);
    return size;
}

static void rec_mkdir(const char *dir)
{
    char *tmp = strdup(dir);
    tmp = dirname(tmp);
    size_t len = strlen(tmp);

    if(tmp[len - 1] == '/')
        tmp[len - 1] = '\0';

    for(char *p = tmp + 1; *p; p++)
    {
        if(*p == '/')
        {
            *p = '\0';
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);
    free(tmp);
}

void create_files(struct metainfo *meta)
{
    char zero = '0';
    for (size_t i = 0; meta->files[i]; i++)
    {
        // Check if file exist with correct size
        if (access(meta->files[i], F_OK) == 0
                && get_size_file(meta->files[i]) == meta->files_size[i])
        {
            continue;
        }
        rec_mkdir(meta->files[i]);
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
