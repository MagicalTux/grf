#include <grf.h>
#include <zlib.h>

int zlib_buffer_inflate(void *dest, int destlen, void *src, int srclen) {
	z_stream stream;
	int err;
	
	stream.next_in = src;
	stream.avail_in = srclen;
	
	stream.next_out = dest;
	stream.avail_out = destlen;
	
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	
	err = inflateInit(&stream);
	if (err != Z_OK) return 0;

	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		inflateEnd(&stream);
		return 0;
	}
	
	err = inflateEnd(&stream);
	if (err != Z_OK) return 0;
	return stream.total_out;
}

int zlib_buffer_deflate(void *dest, int destlen, void *src, int srclen, int level) {
	z_stream stream;
	int err;
	
	stream.next_in = src;
	stream.avail_in = srclen;
	
	stream.next_out = dest;
	stream.avail_out = destlen;
	
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	
	err = deflateInit(&stream, level);
	if (err != Z_OK) return 0;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END) {
		deflateEnd(&stream);
		return 0;
	}
	
	err = deflateEnd(&stream);
	if (err != Z_OK) return 0;
	return stream.total_out;
}

