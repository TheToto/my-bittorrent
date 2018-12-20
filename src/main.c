#include <stdio.h>
#include <getopt.h>
#include <err.h>

#include "parser.h"
#include "integrity.h"
#include "tracker.h"
#include "epoll.h"

static struct option long_opt[] =
{
    {
        "pretty-print-torrent-file", required_argument, 0, 0
    },
    {
        "mktorrent", required_argument, 0, 0
    },
    {
        "check-integrity", required_argument, 0, 0
    },
    {
        "dump-peers", no_argument, 0, 0
    },
    {
        "verbose", no_argument, 0, 0
    },
    {
        0, 0, 0, 0
    }
};

static void opt_check_integrity(char *optarg)
{
    struct metainfo *meta = decode_torrent(optarg, 0, 0, 0);
    int ret = check_integrity(meta);
    unleash_void(meta);
    exit(ret == 0);
}

static int handle_options(int argc, char **argv, int *dump_peers, int *verbose)
{
    while (1)
    {
        int option_index = 0;

        char c = getopt_long(argc, argv, "p:m:c:dv", long_opt, &option_index);
        if (c == -1)
            return 1;//OK
        if (c == 'p' || (c == 0 && option_index == 0))
        {
            struct metainfo *meta = decode_torrent(optarg, 1, 0, 0);
            if (!meta)
                exit(1);
            unleash_void(meta);
            exit(0);
        }
        else if (c == 'm' || (c == 0 && option_index == 1))
            mktorrent(optarg);
        else if (c == 'c' || (c == 0 && option_index == 2))
            opt_check_integrity(optarg);
        else if (c == 'd' || (c == 0 && option_index == 3))
            *dump_peers = 1;
        else if (c == 'v' || (c == 0 && option_index == 4))
            *verbose = 1;
    }
}

static void print_resume(struct metainfo *meta)
{
    size_t acu = 0;
    for (size_t i = 0; i < meta->nb_piece; i++)
    {
        if (meta->have[i])
            acu++;
    }
    if (acu != 0)
    {
        printf("%6s: resumed from %zu/%zu\n", meta->torrent_id,
                acu, meta->nb_piece);
    }
}

int meta_handling(struct metainfo *meta)
{
    create_files(meta);
    init_epoll(meta->peers);
    if (check_integrity(meta)) //For resuming (But too slow on big files)
    {
        printf("%6s: torrent is already complete\n", meta->torrent_id);
        return 1;
    }
    print_resume(meta);
    int ret;
    if ((ret = init_tracker(meta->announce, meta)))
    {
        wait_event_epoll(meta);
    }
    return ret;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
        errx(1, "Usage: %s [options] [files]\n", argv[0]);

    int verbose = 0;
    int dump_peers = 0;
    if (!handle_options(argc, argv, &dump_peers, &verbose))
        return 0;
    struct metainfo *meta = decode_torrent(argv[optind], 0,
            dump_peers, verbose);
    int ret = 1;
    if (meta)
    {
        ret = meta_handling(meta);
        unleash_void(meta);
    }
    else
        warnx("Failed to decode .torrent file");
    if (ret)
        return 0;
    return 1;
}
