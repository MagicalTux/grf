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
typedef void * grf_treenode;
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

/* do not ask questions about that */
#define GRF_FLAG_FILE 1
#define GRF_FLAG_MIXCRYPT 2
#define GRF_FLAG_DES 4
/* extra custom-flag to delete a file while updating */
#define GRF_FLAG_DELETE 8

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
 * (grf_node) grf_file_add_fd(grf_handle, const char *name, int fd)
 * (grf_node) grf_file_add_path(grf_handle, const char *name, const char *file)
 * Add a file to the specified GRF file (opened for writing) and return the
 * pointer to the created file.
 */
GRFEXPORT grf_node grf_file_add(grf_handle, const char *, void *, size_t); /* grf.c */
GRFEXPORT void *grf_file_add_fd(void *, const char *, int); /* grf.c */
GRFEXPORT void *grf_file_add_path(void *, const char *, const char *); /* grf.c */

/* (grf_node) grf_get_file(grf_handle handle, const char *filename)
 * Returns a node handle to the specified file inside the GRF file. If the
 * function fails, NULL is returned.
 */
GRFEXPORT void *grf_get_file(void *, const char *); /* grf.c */

/* (const char *) grf_file_get_filename(grf_node)
 * Returns the full filename of a file.
 */
GRFEXPORT const char *grf_file_get_filename(grf_node); /* grf.c */

/* (const char *) grf_file_get_basename(grf_node)
 * Returns only the filename (no path info)
 */
GRFEXPORT const char *grf_file_get_basename(grf_node); /* grf.c */

/* (unsigned int) grf_file_get_size(grf_node)
 * Returns the (real) size of the file.
 */
GRFEXPORT uint32_t grf_file_get_size(grf_node); /* grf.c */

/* (unsigned int) grf_file_get_storage_pos(grf_node)
 * Returns the position of the file in the archive.
 */
GRFEXPORT uint32_t grf_file_get_storage_pos(grf_node); /* grf.c */

/* (unsigned int) grf_file_get_storage_size(grf_node)
 * Returns the real (compressed) size of the file.
 */
GRFEXPORT uint32_t grf_file_get_storage_size(grf_node); /* grf.c */

/* (unsigned int) grf_file_get_storage_flags(grf_node)
 * Test if the first bit is set to know if a node is a file, or not.
 */
GRFEXPORT uint32_t grf_file_get_storage_flags(grf_node); /* grf.c */

/* (unsigned int) grf_file_get_contents(grf_node, void *ptr)
 * Extracts the file to the provided pointer and returns the number of bytes
 * successfully extracted. The system assumes that you pre-allocated enough
 * memory in ptr by calling grf_file_get_size().
 */
GRFEXPORT uint32_t grf_file_get_contents(grf_node, void *); /* grf.c */

/* (unsigned int) grf_file_put_contents_to_fd(grf_node, int)
 * Extracts a file to the specified file descriptor. This can be a socket or
 * a regular file, no seeks are used.
 */
GRFEXPORT uint32_t grf_file_put_contents_to_fd(grf_node, int); /* grf.c */

/* (bool) grf_put_contents_to_file(grf_node, const char *filename)
 * Write the contents of the compressed file to the filesystem.
 */
GRFEXPORT bool grf_put_contents_to_file(grf_node, const char *); /* grf.c */

/* (bool) grf_file_rename(grf_node, const char *new_name)
 * Rename the given file to new_name. NB: You must provide a FULL filename,
 * including the full path of the file, eg: data\some_file.txt
 */
GRFEXPORT bool grf_file_rename(grf_node, const char *); /* grf.c */

/* (bool) grf_file_delete(grf_node)
 * Removes a file from the GRF.
 * NB: This will not immediatly reduce the size of the resulting GRF file,
 * you will have to repack the GRF to see a reduction in its size. However if
 * you add a file which is exactly the same size as the deleted file, no repack
 * will be needed.
 */
GRFEXPORT bool grf_file_delete(grf_node); /* grf.c */

/*****************************************************************************
 ******************************* TREE FUNCTIONS ******************************
 *****************************************************************************
 * Trees are a good way to display the content of a GRF file in a hierarchized
 * way. Basically, GRF files just contain a list of file, and each file has
 * its full path information attached to it. Using grf_create_tree(), you can
 * ask libgrf to build a global directories/files-based tree for the GRF,
 * allowing things such as listing the content of a directory directly,
 * without having to parse the whole files list yourself.
 */

/* grf_create_tree(grf_handle)
 * Create a tree for the specified GRF file.
 */
GRFEXPORT void grf_create_tree(grf_handle); /* grf.c */

/* (grf_treenode) grf_tree_get_root(grf_handle)
 * Returns the root node of the tree.
 */
GRFEXPORT grf_treenode grf_tree_get_root(grf_handle); /* grf.c */

/* (grf_treenode *)grf_tree_list_node(grf_treenode)
 * Returns the list of files/directories found in a specific node.
 * NB: You will have to free() this result after use.
 */
GRFEXPORT grf_treenode *grf_tree_list_node(grf_treenode); /* grf.c */

/* (bool) grf_tree_is_dir(grf_treenode)
 * Return true if this node is a tree. Return false if it's a regular file.
 * There's no other possible type for a node.
 */
GRFEXPORT bool grf_tree_is_dir(grf_treenode); /* grf.c */

/* (const char *) grf_tree_get_name(grf_treenode)
 * Returns the name of a node (no path info will be added)
 */
GRFEXPORT const char *grf_tree_get_name(grf_treenode); /* grf.c */

/* (grf_node) grf_tree_get_file(grf_treenode)
 * Return the node of the file attached to the current treenode. Will return
 * NULL for directories. (but do not rely on that, you have a function named
 * grf_tree_is_dir() to determine if a node is a dir, or not)
 */
GRFEXPORT grf_node grf_tree_get_file(grf_treenode); /* grf.c */

/* (grf_treenode) grf_tree_get_parent(grf_treenode)
 * Returns the parent node, or NULL if you call that on the root node.
 */
GRFEXPORT grf_treenode grf_tree_get_parent(grf_treenode); /* grf.c */

/* (grf_treenode) grf_file_get_tree(grf_node)
 * Returns the grf_treenode entry corresponding to a file. Will return NULL
 * if the treenode wasn't created previously.
 */
GRFEXPORT grf_treenode grf_file_get_tree(grf_node); /* grf.c */

/* (unsigned int) grf_tree_dir_count_files(grf_treenode)
 * Returns the number of files contained in a treenode dir.
 */
GRFEXPORT uint32_t grf_tree_dir_count_files(void *); /* grf.c */

/*****************************************************************************
 ************************** FILE LISTING FUNCTIONS ***************************
 ****************************************************************************/

/* (grf_node*) grf_get_file_list(grf_handle)
 * Returns all the nodes contained in the GRF files. The order is totally
 * random and files may be inserted anyway in this list, so don't rely too
 * much on that, 'kay?
 * NB: Remember to free() it after use too, m'kay?
 */
GRFEXPORT grf_node *grf_get_file_list(grf_handle); /* grf.c */

/* (grf_node) grf_get_file_first(grf_handle)
 * Returns the first file's node pointer in the GRF, or NULL if no file were
 * found.
 */
GRFEXPORT grf_node grf_get_file_first(grf_handle); /* grf.c */

/* (grf_node) grf_get_file_next(grf_node)
 * Returns next file, or NULL if file was last file.
 */
GRFEXPORT grf_node grf_get_file_next(grf_node); /* grf.c */

/* (grf_node) grf_get_file_prev(grf_node)
 * Returns previous file, or NULL if file is already the first one.
 */
GRFEXPORT grf_node grf_get_file_prev(grf_node); /* grf.c */

/*****************************************************************************
 ***************************** FILE ID FUNCTIONS *****************************
 ****************************************************************************/

/* (grf_node*) grf_get_file_id_list(grf_handle)
 * Returns the global list of files. Its order may vary each time GRF file is
 * modified.
 */
GRFEXPORT grf_node *grf_get_file_id_list(grf_handle);

/* (unsigned int) grf_file_get_id(grf_node)
 * Returns ID (index position in the list) of a file.
 */
GRFEXPORT uint32_t grf_file_get_id(void *); /* grf.c */

/* (grf_node) grf_get_file_by_id(grf_handle, unsigned int id)
 * Returns the file corresponding to the given ID.
 */
GRFEXPORT grf_node grf_get_file_by_id(grf_handle, uint32_t); /* grf.c */

/*****************************************************************************
 *************************** GRF MASS OPERATIONS *****************************
 ****************************************************************************/

#define GRF_REPACK_FAST 1
#define GRF_REPACK_DECRYPT 2
#define GRF_REPACK_RECOMPRESS 3

/* (bool) grf_repack(grf_handle, char options)
 * Repack given GRF file, to save wasted_bytes and maybe more (if using the
 * recompress option).
 * Valid options :
 * - GRF_REPACK_FAST (just move files)
 * - GRF_REPACK_DECRYPT (move files, and decrypt files if any was found
 *   encrypted)
 * - GRF_REPACK_RECOMPRESS (recompress all files, and replace if newly
 *   compressed file is smaller than the one previously stored)
 */
GRFEXPORT bool grf_repack(grf_handle, uint8_t);

/* (bool) grf_merge(grf_handle dest, grf_handle src, char options)
 * Copy files from "src" grf_handle (can be opened read-only) to "dest"
 * grf_handle (must be opened read/write). Takes the same options as
 * grf_repack with one exception : GRF_REPACK_RECOMPRESS will recompress all
 * files, even if the new one is larger than the not-recompressed one.
 */
GRFEXPORT bool grf_merge(grf_handle, grf_handle, uint8_t);

/*****************************************************************************
 **************************** CHARSET FUNCTIONS ******************************
 ****************************************************************************/

/* (char *) euc_kr_to_utf8(const char *text)
 * (char *) euc_kr_to_utf8_r(const char *text, unsigned char *ptr)
 * "Translates" EUC-KR text as encoded in GRF files to UTF-8. Available also
 * in reentrant version.
 */
GRFEXPORT char *euc_kr_to_utf8(const char *); /* euc_kr.c */
GRFEXPORT char *euc_kr_to_utf8_r(const char *, uint8_t *); /* euc_kr.c */

/* (char *) utf8_to_euc_kr(const char *text)
 * (char *) utf8_to_euc_kr_r(const char *text, unsigned char *ptr)
 * "Translates" UTF-8 text to EUC-KR as used to encode filenames in GRF files
 * Of course you need to input KOREAN/ASCII text or it'll return NULL.
 */
GRFEXPORT char *utf8_to_euc_kr(const char *); /* euc_kr.c */
GRFEXPORT char *utf8_to_euc_kr_r(const char *, uint8_t *); /* euc_kr.c */

#ifdef GRFEXPORT_TMP_DEF
#undef GRFEXPORT_TMP_DEF
#undef GRFEXPORT
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LIBGRF_H_INCLUDED */

