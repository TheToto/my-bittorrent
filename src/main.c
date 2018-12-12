#include <stdio.h>

#include "parser.h"
#include "integrity.h"

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr,
                "my-bittorrent: Usage: %s [options] [files]\n", argv[0]);
        return 1;
    }
    struct metainfo *meta = decode_torrent(argv[1], 1);
    int ret = check_integrity(meta);
    free_metainfo(meta);
    return ret;
}
