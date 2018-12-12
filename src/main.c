#include <stdio.h>
#include <err.h>

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
    int ret = 1;
    if (meta)
        ret = check_integrity(meta);
    else
        warnx("Failed to decode .torrent file");
    free_metainfo(meta);
    if (ret)
        printf("Integrity check : SUCCESS\n");
    else
        printf("Integrity check : FAILED\n");
    return ret == 0;
}
