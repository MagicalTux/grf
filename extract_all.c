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
	if ((s.st_mode & S_IFDIR) == S_IFDIR) { free(b); return; } // Good
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
	void **list;
	if (argc != 2) {
		fprintf(stderr, "Call: %s /path/to/file.grf\n", argv[0]);
		return 1;
	}
	printf("Trying to open `%s'...\n", argv[1]);
	grf = grf_load(argv[1], false); // readonly
	if (grf==NULL) {
		printf("Failed! Please check that `%s' is a valid Gravity Ragnarok File.\n", argv[1]);
		return 1;
	}
	// get files list
	list = grf_get_file_list(grf);
	for(int i=0;list[i]!=NULL;i++) {
		char *name = strdup(grf_file_get_filename(list[i]));
		size_t len = strlen(name);
		size_t size = grf_file_get_size(list[i]);
		void *ptr;
		FILE *f;
		for(int i=0;i<len;i++) if (*(name+i)=='\\') *(name+i) = '/';
		printf("Extracting: %s: (%d bytes)                  \r", name, size);
		do_mkdir(name);
		f = fopen(name, "wb");
		if (f==NULL) {
			printf("\nError (fopen)\n");
			free(name);
			continue;
		}
		ptr = malloc(size);
		if ((ptr==NULL) || (grf_file_get_contents(list[i], ptr) != size)) {
			printf("\nError (get_contents)\n");
			free(ptr);
			free(name);
			fclose(f);
			continue;
		}
		fwrite(ptr, size, 1, f);
		free(ptr);
		fclose(f);
		free(name);
	}
	printf("\nFinished!\n");
	free(list);
	grf_free(grf);
	return 0;
}

