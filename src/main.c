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
            unleash_void(decode_torrent(optarg, 1, 0, 0));
            return 0;//EXIT
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

int meta_handling(struct metainfo *meta)
{
    create_files(meta);
    /*if (check_integrity(meta)) //For resuming (But too slow on big files)
      {
      printf("File already downloaded\n");
      exit(0);
      }*/
    init_epoll(meta->peers);
    int ret = 2;
    if (init_tracker(meta->announce, meta))
    {
        wait_event_epoll(meta);
        ret = check_integrity(meta); // A bit useless
    }
    return ret;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        fprintf(stderr,
                "my-bittorrent: Usage: %s [options] [files]\n", argv[0]);
        return 1;
    }
    int verbose = 0;
    int dump_peers = 0;
    if (!handle_options(argc, argv, &dump_peers, &verbose))
        return 0;
    struct metainfo *meta = decode_torrent(argv[optind], 0,
            dump_peers, verbose);
    int ret;
    if (meta)
    {
        if ((ret = meta_handling(meta)) == 1)
            printf("Integrity check : SUCCESS\n");
        else if (ret != 2)
            printf("Integrity check : FAILED\n");
        unleash_void(meta);
    }
    else
        warnx("Failed to decode .torrent file");
    return 0;
}
