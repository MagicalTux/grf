#include <grf.h>
#include <stdio.h>
#include <zlib.h>
#include <stdint.h>

GRFEXPORT uint32_t grf_version(void) {
	return (VERSION_MAJOR << 16) | (VERSION_MINOR << 8) | VERSION_REVISION;
}

GRFEXPORT char *grf_versionstring(void) {
	static char vstr[128];
	snprintf((char *)&vstr, 128, VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, zlibVersion());
	return vstr;
}

GRFEXPORT char *grf_versionstring_r(char *vstr, size_t buflen) {
	snprintf(vstr, buflen, VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, zlibVersion());
	return vstr;
}

