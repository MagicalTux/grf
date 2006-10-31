/* grf.h : main grf include
 * Blavla
 *
 */

#ifndef __GRF_H_INCLUDED
#define __GRF_H_INCLUDED

#define _LARGEFILE_SOURCE

#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include "hash_tables.h"

#ifndef GRF_NO_EXPORT
#ifdef __WIN32
#define GRFEXPORT __declspec(dllexport)
#else
#define GRFEXPORT extern
#endif
#else
#define GRFEXPORT
#endif

#define VERSION_MAJOR 0
#define VERSION_MINOR 1

#ifdef __WIN32
#define VERSION_TYPE "Win32"
#else
#define VERSION_TYPE "Unix"
#endif

#ifdef __USE_LARGEFILE
#define VERSION_EXTRA ", LargeFile"
#else
#define VERSION_EXTRA
#endif

#ifdef __DEBUG
#define VERSION_EXTRA2 ", Debug"
#else
#define VERSION_EXTRA2
#endif

#define VERSION_STRING "Gravity Ragnarok Files lib v%d.%d (" VERSION_TYPE VERSION_EXTRA VERSION_EXTRA2 ") by MagicalTux <MagicalTux@gmail.com>\nBundled zlib version %s"

struct grf_node {
	struct grf_node *prev, *next;
	struct grf_handler *parent;
	char *filename, type;
	uint32_t size, len, len_aligned, pos;
};

struct grf_treenode {
	bool is_dir;
	hash_table *subdir;
	struct grf_node *ptr;
};

struct grf_handler {
	uint32_t filecount, table_offset, wasted_space;
	int fd;
	bool need_save;
	struct grf_node *first_node;
	hash_table *fast_table;
	struct grf_treenode *root;
};

#define GRF_HEADER_SIZE 0x2e /* sizeof(grf_header) */
#define GRF_HEADER_MAGIC "Master of Magic"
#define GRF_FILE_OUTPUT_VERISON 0x200
#define GRF_HASH_TABLE_SIZE 128
#define GRF_TREE_HASH_SIZE 16

// should we use pragma pack ?
struct grf_header {
	char header_magic[16] __attribute__ ((__packed__)); // "Master of Magic" + 0x00
	char header_key[14] __attribute__ ((__packed__)); // 0x01 -> 0x0e
	uint32_t offset __attribute__ ((__packed__)); // offset of file table
	uint32_t seed __attribute__ ((__packed__));
	uint32_t filecount __attribute__ ((__packed__)); // Real filecount = filecount - seed - 7
	uint32_t version __attribute__ ((__packed__)); // we only know about 0x200
};

struct grf_table_entry_data {
	// file name (char + 0x00)
	uint32_t len __attribute__ ((__packed__)); // packed len
	uint32_t len_aligned __attribute__ ((__packed__)); // same, but with the alignment (?)
	uint32_t size __attribute__ ((__packed__)); // real file size (unpacked)
	uint8_t type __attribute__ ((__packed__)); // type seems to be 1, 3 or 5
	uint32_t pos __attribute__ ((__packed__)); // position in the grf
};

int zlib_buffer_inflate(void *, int, void *, int); /* private: zlib.c */
int zlib_buffer_deflate(void *, int, void *, int); /* private: zlib.c */

#include "libgrf.h"

#endif /* __GRF_H_INCLUDED */

