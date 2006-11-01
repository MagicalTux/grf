#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> /* free() */
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libgrf.h>

void do_mkdir(char *name) {
	char *n = strdup(name);
	char *b = strdup(dirname(n));
	struct stat s;
	memset(&s, 0, sizeof(s));
	free(n);
	stat(b, &s);
	if ((s.st_mode & S_IFDIR) == S_IFDIR) { free(b); return; } /* already good */
	if (mkdir(b, 0755) == 0) {
		free(b);
		return;
	}
	do_mkdir(b);
	if (mkdir(b, 0755) != 0) {
		printf("Failed to create %s\n", b);
		exit(1);
	}
	free(b);
}

int main(int argc, char *argv[]) {
	void *grf;
	void *cur_file;
	int i;
	char *name;
	size_t len, size;
	void *ptr;
	FILE *f;
	if (argc != 2) {
		fprintf(stderr, "Call: %s /path/to/file.grf\n", argv[0]);
		return 1;
	}
	printf("Trying to open `%s'...\n", argv[1]);
	grf = grf_load(argv[1], false); /* readonly */
	if (grf==NULL) {
		printf("Failed! Please check that `%s' is a valid Gravity Ragnarok File.\n", argv[1]);
		return 1;
	}
	/* get files list */
	cur_file = grf_get_file_first(grf);
	while(cur_file != NULL) {
		name = strdup(grf_file_get_filename(cur_file));
		len = strlen(name);
		size = grf_file_get_size(cur_file);
		for(i=0;i<len;i++) if (*(name+i)=='\\') *(name+i) = '/';
		printf("Extracting: %s: (%d bytes)                  \r", name, size);
		do_mkdir(name);
		f = fopen(name, "wb");
		if (f==NULL) {
			printf("\nError (fopen)\n");
			free(name);
			cur_file = grf_get_file_next(cur_file);
			continue;
		}
		ptr = malloc(size);
		if ((ptr==NULL) || (grf_file_get_contents(cur_file, ptr) != size)) {
			printf("\nError (get_contents)\n");
			free(ptr);
			free(name);
			fclose(f);
			cur_file = grf_get_file_next(cur_file);
			continue;
		}
		fwrite(ptr, size, 1, f);
		free(ptr);
		fclose(f);
		free(name);
		cur_file = grf_get_file_next(cur_file);
	}
	printf("\nFinished!\n");
	grf_free(grf);
	return 0;
}

