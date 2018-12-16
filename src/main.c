#include <stdio.h>
#include <err.h>

#include "parser.h"
#include "integrity.h"
#include "tracker.h"

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr,
                "my-bittorrent: Usage: %s [options] [files]\n", argv[0]);
        return 1;
    }
    if (argc == 3)
        mktorrent(argv[1]);
    else
    {
        struct metainfo *meta = decode_torrent(argv[1], 1);
        int ret = 1;
        if (meta)
        {
            create_files(meta);
            init_tracker(meta->announce, meta);

            for (size_t i = 0; i < meta->peers->size; i++)
            {
                handshake(meta, meta->peers->list[i]);
            }

            ret = check_integrity(meta);
            free_metainfo(meta);
            if (ret)
                printf("Integrity check : SUCCESS\n");
            else
                printf("Integrity check : FAILED\n");
        }
        else
        {
            warnx("Failed to decode .torrent file");
        }
        return ret == 0;
    }
}
