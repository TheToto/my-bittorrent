#pragma once

#include "parser.h"

/// Check integrity of all pieces
int check_integrity(struct metainfo *meta);

/// Check integrity of a piece (from files)
int check_piece(struct metainfo *meta, size_t nb);

/// Get the total size of a torrent in bytes
size_t get_total_size(struct metainfo *meta);

/// Get piece "nb" and put in buf "piece", return number of bytes read
size_t get_piece(struct metainfo *meta, unsigned char *piece, size_t nb);

/// Write the piece "nb" into files
void write_piece(struct metainfo *meta, unsigned char *piece, size_t nb);

/// Create empty files (placeholders) (call this before write_piece)
void create_files(struct metainfo *meta);
