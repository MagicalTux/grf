/* grf.h : main grf include
 * Blavla
 *
 */

#ifndef __GRF_H_INCLUDED
#define __GRF_H_INCLUDED

#define _LARGEFILE_SOURCE

#ifdef __C99
#error test
#endif

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
#define VERSION_REVISION 31

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

#define VERSION_STRING "Gravity Ragnarok Files lib v%d.%d.%d (" VERSION_TYPE VERSION_EXTRA VERSION_EXTRA2 ") by MagicalTux <MagicalTux@ooKoo.org>\nBundled zlib version %s"

typedef struct grf_handler *grf_handle;
typedef struct grf_node *grf_node;
typedef struct grf_treenode *grf_treenode;
#define __LIBGRF_HAS_TYPEDEF

struct grf_node {
	struct grf_node *prev, *next;
	struct grf_handler *parent;
	struct grf_treenode *tree_parent;
	char *filename, flags;
	uint32_t size, len, len_aligned, pos, id;
	int cycle;
};

struct grf_treenode {
	bool is_dir;
	char *name;
	hash_table *subdir;
	struct grf_node *ptr;
	struct grf_treenode *parent;
};

struct grf_handler {
	uint32_t filecount, table_offset, table_size, wasted_space;
	uint32_t version;
	int fd;
	int compression_level;
	bool need_save, write_mode;
	struct grf_node *first_node;
	hash_table *fast_table;
	struct grf_treenode *root;
	bool (* callback)(void *, grf_handle, int, int, const char *);
	void *callback_etc;
	struct grf_node **node_table;
};

#define GRF_HEADER_SIZE 0x2e /* sizeof(grf_header) */
#define GRF_HEADER_MAGIC "Master of Magic"
#define GRF_FILE_OUTPUT_VERISON 0x200
#define GRF_HASH_TABLE_SIZE 128
#define GRF_TREE_HASH_SIZE 32

/* values specific to all directories */
#define GRF_DIRECTORY_LEN 1094
#define GRF_DIRECTORY_LEN_ALIGNED 1812
#define GRF_DIRECTORY_SIZE 1372
#define GRF_DIRECTORY_OFFSET 1418

// should we use pragma pack ?
struct grf_header {
	char header_magic[16] __attribute__ ((__packed__)); // "Master of Magic" + 0x00
	char header_key[14] __attribute__ ((__packed__)); // 0x01 -> 0x0e, or 0x00 -> 0x00 (no crypt)
	uint32_t offset __attribute__ ((__packed__)); // offset of file table
	uint32_t seed __attribute__ ((__packed__));
	uint32_t filecount __attribute__ ((__packed__)); // Real filecount = filecount - seed - 7
	uint32_t version __attribute__ ((__packed__)); // 0x102 0x103 0x200 0xCACA
};

struct grf_table_entry_data {
	// file name (char + 0x00)
	uint32_t len __attribute__ ((__packed__)); // packed len
	uint32_t len_aligned __attribute__ ((__packed__)); // same, but with the alignment (?)
	uint32_t size __attribute__ ((__packed__)); // real file size (unpacked)
	uint8_t flags __attribute__ ((__packed__)); // file flags
	uint32_t pos __attribute__ ((__packed__)); // position in the grf
};

int zlib_buffer_inflate(void *, int, void *, int); /* private: zlib.c */
int zlib_buffer_deflate(void *, int, void *, int, int); /* private: zlib.c */

#define MAX(a,b) ((a>b)?a:b)

#include "libgrf.h"

#endif /* __GRF_H_INCLUDED */

