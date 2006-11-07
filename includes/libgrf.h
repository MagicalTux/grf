/* libgrf.h : main header file
 */

#ifndef __LIBGRF_H_INCLUDED
#define __LIBGRF_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __cplusplus
#if __STDC_VERSION__ >= 199901L
/* we have a C99 compiler, and we're compiling for C99 */
#include <stdint.h>
#ifndef __bool_true_false_are_defined
#include <stdbool.h>
#endif /* __bool_true_false_are_defined */
#else /* __STDC_VERSION__ >= 199901L */
/* We're most likely using ANSI C (C89), which does not provide bool, etc..
 * Let's use typedef for a few things... */
typedef unsigned int uint32_t;
#ifndef __bool_true_false_are_defined
typedef int bool;
#define true 1
#define false 0
#endif
#endif /* __STDC_VERSION__ >= 199901L */
#endif /* !__cplusplus */

#ifndef GRFEXPORT
#define GRFEXPORT
#define GRFEXPORT_TMP_DEF
#endif

/* only move data, do not care about what we move */
#define GRF_REPACK_FAST 1
/* if we're about to move encrypted data, decrypt it on the fly while moving it */
#define GRF_REPACK_DECRYPT 2
/* why we repack, extract each file, recompress them, and write them. NB: If the
 * recompressed data is larger than the initial data, the initial data will be
 * kept. */
#define GRF_REPACK_RECOMPRESS 3

GRFEXPORT uint32_t grf_version(void); /* main.c */
GRFEXPORT char *grf_versionstring(void); /* main.c */
GRFEXPORT char *grf_versionstring_r(char *, size_t); /* main.c */
GRFEXPORT void *grf_new(const char *, bool); /* grf.c */
GRFEXPORT void *grf_load(const char *, bool); /* grf.c */
GRFEXPORT bool grf_save(void *); /* grf.c */
GRFEXPORT void grf_free(void *); /* grf.c */
GRFEXPORT uint32_t grf_filecount(void *); /* grf.c */
GRFEXPORT uint32_t grf_wasted_space(void *); /* grf.c */
GRFEXPORT void *grf_get_file(void *, const char *); /* grf.c */
GRFEXPORT bool grf_file_delete(void *); /* grf.c */
GRFEXPORT void *grf_file_add(void *, char *, void *, size_t); /* grf.c */
GRFEXPORT void *grf_file_add_path(void *, char *, char *); /* grf.c */
GRFEXPORT const char *grf_file_get_filename(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_size(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_contents(void *, void *); /* grf.c */
GRFEXPORT void grf_create_tree(void *); /* grf.c */
GRFEXPORT void **grf_get_file_list(void *); /* grf.c */
GRFEXPORT void *grf_get_file_next(void *); /* grf.c */
GRFEXPORT void *grf_get_file_prev(void *); /* grf.c */
GRFEXPORT void *grf_get_file_first(void *); /* grf.c */
GRFEXPORT void grf_set_compression_level(void *, int); /* grf.c */
GRFEXPORT char *euc_kr_to_utf8(char *); /* euc_kr.c */

#ifdef GRFEXPORT_TMP_DEF
#undef GRFEXPORT_TMP_DEF
#undef GRFEXPORT
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LIBGRF_H_INCLUDED */

