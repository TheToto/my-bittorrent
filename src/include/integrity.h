#pragma once

#include "parser.h"

/// Check integrity of all pieces
int check_integrity(struct metainfo *meta);

/// Check integrity of a piece
int check_piece(struct metainfo *meta, size_t nb);

/// Get the total size of a torrent in bytes
size_t get_total_size(struct metainfo *meta);
