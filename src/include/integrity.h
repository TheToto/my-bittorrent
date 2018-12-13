#pragma once

#include "parser.h"

/// Compute pieces hashs of torrent (from files)
char *compute_integrity(char *path, struct metainfo *meta, size_t *size);

/// Check integrity of all pieces
int check_integrity(struct metainfo *meta);

/// Check integrity of a piece (from files)
int check_piece(struct metainfo *meta, size_t nb);

/// Check integrity of a piece (from piece string)
int check_piece_string(struct metainfo *meta, size_t nb,
        unsigned char *piece, size_t piece_size);

/// Get the total size of a torrent in bytes (from metainfo)
size_t get_total_size(struct metainfo *meta);

/// Get the REAL size of a file (different from stat.st_size)
size_t get_size_file(char *path);

/// Get piece "nb" and put in buf "piece", return number of bytes read
size_t get_piece(struct metainfo *meta, unsigned char *piece, size_t nb);

/// Write the piece "nb" into files
void write_piece(struct metainfo *meta, unsigned char *piece, size_t nb);

/// Create empty files (placeholders) (call this before write_piece)
void create_files(struct metainfo *meta);
