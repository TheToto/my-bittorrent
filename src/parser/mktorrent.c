#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "bencode.h"
#include "integrity.h"

struct be_node *create_root(char *path)
{
    struct be_node *root = be_alloc(BE_DICT);
    path = path;
    return root;
}

void mktorrent(char *path)
{
    struct be_node *root = create_root(path);
    FILE *f = fopen(path, "w");
    if (!f)
    {
        warn("Can't open file %s", path);
        return;
    }
    size_t size;
    char *enc = be_encode(root, &size);
    fwrite(enc, sizeof(char), size, f);
    fclose(f);
}
