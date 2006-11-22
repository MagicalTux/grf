/* libgrf.h : main header file
 *
 * Please include this file in your C/C++ programs to be able to use the GRF
 * functions.
 * Remember to always call grf_free() after using a GRF file to avoid memory
 * leaks.
 *
 * This file is compatible and tested with C89(ANSI)/C99/C++
 */

#ifndef __LIBGRF_H_INCLUDED
#define __LIBGRF_H_INCLUDED

/* If we're running in C++, give the compiler a hint that the following
 * definitions are in C style */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Detect version of C compiler, and do the required stuff to stay compatible
 */
#ifndef __cplusplus
#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
#ifndef __bool_true_false_are_defined
#include <stdbool.h>
#endif /* __bool_true_false_are_defined */
#else /* __STDC_VERSION__ >= 199901L */
/* We're most likely using ANSI C (C89), which does not provide bool, etc..
 * Let's use typedef for a few things... */
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#ifndef __bool_true_false_are_defined
typedef int bool;
#define true 1
#define false 0
#endif
#endif /* __STDC_VERSION__ >= 199901L */
#endif /* !__cplusplus */

/* Since we also use this file for building the grf library, we have to export
 * symbols only when building the library. If we're not building the lib, the
 * GRFEXPORT define must be empty.
 */
#ifndef GRFEXPORT
#define GRFEXPORT
#define GRFEXPORT_TMP_DEF
#endif

#ifndef __LIBGRF_HAS_TYPEDEF
typedef void * grf_handle;
typedef void * grf_node;
#endif

/* Some defines used by grf_merge() and grf_repack() : 
 *  - GRF_REPACK_FAST
 *    only move data, do not care about what we move
 *  - GRF_REPACK_DECRYPT
 *    if we're about to move encrypted data, decrypt it on the fly
 *  - GRF_REPACK_RECOMPRESS
 *    why we repack, extract each file, recompress them, and write them. NB: If
 *    the recompressed data is larger than the initial data, the initial data
 *    will be kept.
 */

#define GRF_REPACK_FAST 1
#define GRF_REPACK_DECRYPT 2
#define GRF_REPACK_RECOMPRESS 3

/*****************************************************************************
 *************************** BASE FUNCTIONS **********************************
 ****************************************************************************/

/* (int) grf_version()
 * Returns the current GRF version as an integer.
 * You can get major/minor/revision values with the following math:
 * major = (version >> 16) & 0xff
 * minor = (version >> 8) & 0xff
 * revision = version & 0xff
 */
GRFEXPORT uint32_t grf_version(void); /* main.c */

/* (char *) grf_versionstring()
 * (char *) grf_versionstring_r(char *buffer, size_t len)
 * Returns the version as a string, which includes a linebreak (for the version
 * of zlib)
 * You can use the reentrant version of this function for thread-safe usage.
 */
GRFEXPORT char *grf_versionstring(void); /* main.c */
GRFEXPORT char *grf_versionstring_r(char *, size_t); /* main.c */

/* (grf_handle) grf_new(const char *filename, bool allow_write)
 * (grf_handle) grf_new_by_fd(int fd, bool allow_write)
 * Creates a new GRF file called filename. You will usually want to set
 * allow_write to true, as creating a new grf for read-only use isn't really
 * useful, however no write happens until you call grf_save(), so you can
 * use grf_new(), assign a callback, then call grf_load_from_new() to have
 * a callback while the loading happens.
 * grf_new_by_fd() is useful on windows for example, as grf_new() does not
 * support unicode names, you can open the file yourself, then pass it to
 * grf_new_by_fd().
 */
GRFEXPORT grf_handle grf_new(const char *, bool); /* grf.c */
GRFEXPORT grf_handle grf_new_by_fd(int, bool); /* grf.c */

/* (grf_handle) grf_load(const char filename, bool allow_write)
 * Well ... This just loads file filename, and returns a handle to the GRF.
 */
GRFEXPORT grf_handle grf_load(const char *, bool); /* grf.c */

/* (grf_handle) grf_load_from_new(grf_handle handle)
 * Loads GRF data from an handle returned by grf_new() or grf_new_by_fd()
 * If a problem happens, the grf is free()d and the function returns NULL.
 */
GRFEXPORT grf_handle grf_load_from_new(grf_handle); /* grf.c */

/* (bool) grf_save(grf_handle handle)
 * Write the grf's files table to disk.
 */
GRFEXPORT bool grf_save(grf_handle); /* grf.c */

/* grf_free(grf_handle handle)
 * Free a GRF file and all the memory used by it, and closes the grf's fd.
 * If any change was made to the grf, grf_free() will call grf_save() just
 * before freeing everything. If grf_save() fails, the failure is ignored
 * and the ressources freeing happens anyway. If you need to check for result
 * of save, call grf_save() before grf_free().
 */
GRFEXPORT void grf_free(grf_handle); /* grf.c */

/* grf_set_callback(grf_handle handle, callback, callback_param)
 * Callback: bool callback(void *param, grf_handle handle, int position,
 * 		int max, const char *filename)
 * Defines a callback function for all the long operations. This includes 
 * loading, repack, merge, etc...
 */
GRFEXPORT void grf_set_callback(grf_handle, bool (*)(void *, grf_handle, int, int, const char *), void *); /* grf.c */

/* grf_set_compression_level(grf_handle handle, int level)
 * Sets the compression level used on this GRF file. This affects newly added
 * files, repack/merge when using GRF_REPACK_RECOMPRESS mode, and files table.
 * The level is directly passed to zlib, and should be between 0 (no
 * compression) and 9. */
GRFEXPORT void grf_set_compression_level(grf_handle, int); /* grf.c */

/* (unsigned int) grf_filecount(grf_handle handle)
 * Returns the number of files currently in the GRF. Directory entries are
 * excluded from this count.
 */
GRFEXPORT uint32_t grf_filecount(grf_handle); /* grf.c */

/* (unsigned int) grf_wasted_space(void *handle)
 * Returns the amount of data (in bytes) that would be theorically saved if the
 * file gets repacked.
 * A good application would be to check this value each time the patch client is
 * run, and ask the user about repacking if the value is over 20MB.
 */
GRFEXPORT uint32_t grf_wasted_space(grf_handle); /* grf.c */

/*****************************************************************************
 **************************** FILES FUNCTIONS ********************************
 ****************************************************************************/

/* (grf_node) grf_file_add(grf_handle, const char *name, void *buffer, size_t size)
 * (grf_node) grf_file_add_path(grf_handle, const char *name, const char *file)
 * Add a file to the specified GRF file (opened for writing) and return the
 * pointer to the created file.
 */
GRFEXPORT grf_node grf_file_add(grf_handle, const char *, void *, size_t); /* grf.c */
GRFEXPORT void *grf_file_add_path(void *, const char *, const char *); /* grf.c */

/* (grf_node) grf_get_file(grf_handle handle, const char *filename)
 * Returns a node handle to the specified file inside the GRF file. If the
 * function fails, NULL is returned.
 */
GRFEXPORT void *grf_get_file(void *, const char *); /* grf.c */

GRFEXPORT const char *grf_file_get_filename(void *); /* grf.c */
GRFEXPORT const char *grf_file_get_basename(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_size(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_storage_pos(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_storage_size(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_contents(void *, void *); /* grf.c */
GRFEXPORT uint32_t grf_file_put_contents_to_fd(void *, int); /* grf.c */
GRFEXPORT bool grf_put_contents_to_file(void *, const char *); /* grf.c */
GRFEXPORT bool grf_file_delete(void *); /* grf.c */

/*****************************************************************************
 ******************************* TREE FUNCTIONS ******************************
 ****************************************************************************/

GRFEXPORT void grf_create_tree(void *); /* grf.c */
GRFEXPORT void *grf_tree_get_root(void *); /* grf.c */
GRFEXPORT void **grf_tree_list_node(void *); /* grf.c */
GRFEXPORT bool grf_tree_is_dir(void *); /* grf.c */
GRFEXPORT const char *grf_tree_get_name(void *); /* grf.c */
GRFEXPORT void *grf_tree_get_file(void *); /* grf.c */
GRFEXPORT void *grf_tree_get_parent(void *); /* grf.c */
GRFEXPORT void *grf_file_get_tree(void *); /* grf.c */
GRFEXPORT bool grf_tree_dir_count_files(void *); /* grf.c */

/*****************************************************************************
 ************************** FILE LISTING FUNCTIONS ***************************
 ****************************************************************************/

GRFEXPORT void **grf_get_file_list(void *); /* grf.c */
GRFEXPORT void *grf_get_file_first(void *); /* grf.c */
GRFEXPORT void *grf_get_file_next(void *); /* grf.c */
GRFEXPORT void *grf_get_file_prev(void *); /* grf.c */

/*****************************************************************************
 ***************************** FILE ID FUNCTIONS *****************************
 ****************************************************************************/

GRFEXPORT uint32_t grf_file_get_id(void *); /* grf.c */
GRFEXPORT void *grf_get_file_by_id(void *, uint32_t); /* grf.c */

/*****************************************************************************
 *************************** GRF MASS OPERATIONS *****************************
 ****************************************************************************/

GRFEXPORT bool grf_repack(void *, uint8_t);
GRFEXPORT bool grf_merge(void *, void *, uint8_t);

/*****************************************************************************
 **************************** CHARSET FUNCTIONS ******************************
 ****************************************************************************/

GRFEXPORT char *euc_kr_to_utf8(const char *); /* euc_kr.c */
GRFEXPORT char *euc_kr_to_utf8_r(const char *orig, uint8_t *res); /* euc_kr.c */

#ifdef GRFEXPORT_TMP_DEF
#undef GRFEXPORT_TMP_DEF
#undef GRFEXPORT
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LIBGRF_H_INCLUDED */

