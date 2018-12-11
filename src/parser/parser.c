#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>

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

// Will return a torrent struct ?
void decode_torrent(char *path)
{
    size_t size;
    char *buf = file_to_string(path, &size);
    if (buf == NULL)
    {
        warn("Failed to read file");
        return;
    }
    struct be_node *be = be_decode(buf, size);
    if (be == NULL)
    {
        warn("Failed to decode bencode");
        return;
    }
    print_json(be);
    be_free(be);
}
