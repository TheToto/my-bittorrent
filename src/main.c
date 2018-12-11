#include <stdio.h>

#include "parser.h"

int main(int argc, char **argv)
{
    if (argc <= 1)
        fprintf(stderr,
                "my-bittorrent: Usage: %s [options] [files]\n", argv[0]);
    else
        decode_torrent(argv[1]);
}
