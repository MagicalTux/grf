#include <grf.h>
#include <stdio.h>
#include <zlib.h>
#include <stdint.h>

GRFEXPORT uint32_t grf_version(void) {
	return (VERSION_MAJOR << 8) | VERSION_MINOR;
}

GRFEXPORT char *grf_versionstring(void) {
	static char vstr[128];
	snprintf((char *)&vstr, 128, VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, zlibVersion());
	return vstr;
}

GRFEXPORT char *grf_versionstring_r(char *vstr, size_t buflen) {
	snprintf(vstr, buflen, VERSION_STRING, VERSION_MAJOR, VERSION_MINOR, zlibVersion());
	return vstr;
}

