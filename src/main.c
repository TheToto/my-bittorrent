#include <stdio.h>

#include "parser.h"

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr,
                "my-bittorrent: Usage: %s [options] [files]\n", argv[0]);
        return 1;
    }
    struct metainfo *meta = decode_torrent(argv[1], 1);
    free_metainfo(meta);
    return 0;
}
