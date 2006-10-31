/* libgrf.h : main header file
 */

#ifndef __LIBGRF_H_INCLUDED
#define __LIBGRF_H_INCLUDED

#ifndef __bool_true_false_are_defined
#include <stdbool.h>
#endif

#ifndef GRFEXPORT
#define GRFEXPORT
#define GRFEXPORT_TMP_DEF
#endif

GRFEXPORT uint32_t grf_version(void); /* main.c */
GRFEXPORT char *grf_versionstring(void); /* main.c */
GRFEXPORT char *grf_versionstring_r(char *, size_t); /* main.c */
GRFEXPORT void grf_set_compression_level(int); /* zlib.c */
GRFEXPORT void *grf_new(const char *, bool); /* grf.c */
GRFEXPORT void *grf_load(const char *, bool); /* grf.c */
GRFEXPORT bool grf_save(void *); /* grf.c */
GRFEXPORT void grf_free(void *); /* grf.c */
GRFEXPORT uint32_t grf_filecount(void *); /* grf.c */
GRFEXPORT uint32_t grf_wasted_space(void *); /* grf.c */
GRFEXPORT void *grf_get_file(void *, const char *); /* grf.c */
GRFEXPORT bool grf_file_delete(void *); /* grf.c */
GRFEXPORT void *grf_file_add(void *, const char *, const void *, int); /* grf.c */
GRFEXPORT const char *grf_file_get_filename(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_size(void *); /* grf.c */
GRFEXPORT uint32_t grf_file_get_contents(void *, void *); /* grf.c */
GRFEXPORT void grf_create_tree(void *); /* grf.c */
GRFEXPORT void **grf_get_file_list(void *); /* grf.c */

#ifdef GRFEXPORT_TMP_DEF
#undef GRFEXPORT_TMP_DEF
#undef GRFEXPORT
#endif

#endif /* __LIBGRF_H_INCLUDED */

