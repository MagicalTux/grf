#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <libgrf.h>

/* open directory "data" in current context, and recursively add all files */

void recurse_scan_add(void *grf, char *dname) {
	DIR *d;
	char name[NAME_MAX];
	struct stat s;

	printf("opendir([%p]%s)\n", dname, dname);
	d = opendir(dname);
	if (d == NULL) return; /* argh */
	while(1) {
		struct dirent *de = readdir(d);
		if (de == NULL) break;
		if (de->d_name[0]=='.') continue; /* skip "invisible" files */
		sprintf(name, "%s/%s", dname, de->d_name);
		stat(name, &s);
		if (S_ISDIR(s.st_mode)) {
			recurse_scan_add(grf, name);
		} else if (S_ISREG(s.st_mode)) {
			grf_file_add_path(grf, name, name);
		}
	}
	closedir(d);
}

int main(int argc, char *argv[]) {
	void *grf;
	if (argc != 2) {
		fprintf(stderr, "Arguments: %s out_file\n", argv[0]);
		return 1;
	}
	grf = grf_new(argv[1], true);
	if (grf == NULL) {
		fprintf(stderr, "Could not write to %s\n", argv[1]);
		return 2;
	}
	recurse_scan_add(grf, "data");
	grf_free(grf);
	return 0;
}

