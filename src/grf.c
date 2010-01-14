#include <stdlib.h>
#include <zlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <grf.h>
#ifndef __WIN32
#include <libgen.h>
#endif

/* BEGIN: INCLUDE FROM GRFIO.C */

#ifdef _O_BINARY
# ifdef O_LARGEFILE
#  define OPEN_OPTIONS (O_LARGEFILE | _O_BINARY)
# else
#  define OPEN_OPTIONS _O_BINARY
# endif
#else
# ifdef O_LARGEFILE
#  define OPEN_OPTIONS O_LARGEFILE
# else
#  define OPEN_OPTIONS 0
# endif
#endif

#ifdef __WIN32
#include <windows.h>
#else
/* Since GRF is a windows-type file, we're using windows types "BYTE", "WORD" and "DWORD".
 * However, when we compile on __WIN32 machine, those types are already defined in windef.h
 * included from windows.h, so we don't need to redefine them ! */
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
#endif

// Values for encrypted grf (v0x102/0x103)
static unsigned char BitMaskTable[8] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static char BitSwapTable1[64] = {
	58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15,  7
};
static char	BitSwapTable2[64] = {
	40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
};
static char	BitSwapTable3[32] = {
	16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
     2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};

static unsigned char NibbleData[4][64]={
	{
		0xef, 0x03, 0x41, 0xfd, 0xd8, 0x74, 0x1e, 0x47,  0x26, 0xef, 0xfb, 0x22, 0xb3, 0xd8, 0x84, 0x1e,
		0x39, 0xac, 0xa7, 0x60, 0x62, 0xc1, 0xcd, 0xba,  0x5c, 0x96, 0x90, 0x59, 0x05, 0x3b, 0x7a, 0x85,
		0x40, 0xfd, 0x1e, 0xc8, 0xe7, 0x8a, 0x8b, 0x21,  0xda, 0x43, 0x64, 0x9f, 0x2d, 0x14, 0xb1, 0x72,
		0xf5, 0x5b, 0xc8, 0xb6, 0x9c, 0x37, 0x76, 0xec,  0x39, 0xa0, 0xa3, 0x05, 0x52, 0x6e, 0x0f, 0xd9,
	}, {
		0xa7, 0xdd, 0x0d, 0x78, 0x9e, 0x0b, 0xe3, 0x95,  0x60, 0x36, 0x36, 0x4f, 0xf9, 0x60, 0x5a, 0xa3,
		0x11, 0x24, 0xd2, 0x87, 0xc8, 0x52, 0x75, 0xec,  0xbb, 0xc1, 0x4c, 0xba, 0x24, 0xfe, 0x8f, 0x19,
		0xda, 0x13, 0x66, 0xaf, 0x49, 0xd0, 0x90, 0x06,  0x8c, 0x6a, 0xfb, 0x91, 0x37, 0x8d, 0x0d, 0x78,
		0xbf, 0x49, 0x11, 0xf4, 0x23, 0xe5, 0xce, 0x3b,  0x55, 0xbc, 0xa2, 0x57, 0xe8, 0x22, 0x74, 0xce,
	}, {
		0x2c, 0xea, 0xc1, 0xbf, 0x4a, 0x24, 0x1f, 0xc2,  0x79, 0x47, 0xa2, 0x7c, 0xb6, 0xd9, 0x68, 0x15,
		0x80, 0x56, 0x5d, 0x01, 0x33, 0xfd, 0xf4, 0xae,  0xde, 0x30, 0x07, 0x9b, 0xe5, 0x83, 0x9b, 0x68,
		0x49, 0xb4, 0x2e, 0x83, 0x1f, 0xc2, 0xb5, 0x7c,  0xa2, 0x19, 0xd8, 0xe5, 0x7c, 0x2f, 0x83, 0xda,
		0xf7, 0x6b, 0x90, 0xfe, 0xc4, 0x01, 0x5a, 0x97,  0x61, 0xa6, 0x3d, 0x40, 0x0b, 0x58, 0xe6, 0x3d,
	}, {
		0x4d, 0xd1, 0xb2, 0x0f, 0x28, 0xbd, 0xe4, 0x78,  0xf6, 0x4a, 0x0f, 0x93, 0x8b, 0x17, 0xd1, 0xa4,
		0x3a, 0xec, 0xc9, 0x35, 0x93, 0x56, 0x7e, 0xcb,  0x55, 0x20, 0xa0, 0xfe, 0x6c, 0x89, 0x17, 0x62,
		0x17, 0x62, 0x4b, 0xb1, 0xb4, 0xde, 0xd1, 0x87,  0xc9, 0x14, 0x3c, 0x4a, 0x7e, 0xa8, 0xe2, 0x7d,
		0xa0, 0x9f, 0xf6, 0x5c, 0x6a, 0x09, 0x8d, 0xf0,  0x0f, 0xe3, 0x53, 0x25, 0x95, 0x36, 0x28, 0xcb,
	}
};

static void NibbleSwap(BYTE *src, int len) {
	for( ; 0 < len; len--, src++) {
		*src = (*src >> 4) | (*src << 4);
	}
	return;
}

static void BitConvert(BYTE *Src, char *BitSwapTable) {
	int lop, prm;
	BYTE tmp[8];

	*(DWORD*)tmp = *(DWORD*)(tmp + 4) = 0; // use memset is slower.

	for(lop = 0; lop != 64; lop++) {
		prm = BitSwapTable[lop]-1;
		if (Src[(prm >> 3) & 7] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) & 7] |= BitMaskTable[lop & 7];
		}
	}
	*(DWORD*)Src       = *(DWORD*)tmp;
	*(DWORD*)(Src + 4) = *(DWORD*)(tmp + 4); // use memcpy is not speeder

	return;
}

static void BitConvert4(BYTE *Src) {
	int lop,prm;
	BYTE tmp[8];

	tmp[0] = ((Src[7]<<5) | (Src[4]>>3)) & 0x3f;	// ..0 vutsr
	tmp[1] = ((Src[4]<<1) | (Src[5]>>7)) & 0x3f;	// ..srqpo n
	tmp[2] = ((Src[4]<<5) | (Src[5]>>3)) & 0x3f;	// ..o nmlkj
	tmp[3] = ((Src[5]<<1) | (Src[6]>>7)) & 0x3f;	// ..kjihg f
	tmp[4] = ((Src[5]<<5) | (Src[6]>>3)) & 0x3f;	// ..g fedcb
	tmp[5] = ((Src[6]<<1) | (Src[7]>>7)) & 0x3f;	// ..cba98 7
	tmp[6] = ((Src[6]<<5) | (Src[7]>>3)) & 0x3f;	// ..8 76543
	tmp[7] = ((Src[7]<<1) | (Src[4]>>7)) & 0x3f;	// ..43210 v

	for(lop=0;lop!=4;lop++) {
		tmp[lop] = (NibbleData[lop][tmp[lop*2  ]] & 0xf0)
		         | (NibbleData[lop][tmp[lop*2+1]] & 0x0f);
	}

	*(DWORD*)(tmp+4)=0;
	for(lop=0;lop!=32;lop++) {
		prm = BitSwapTable3[lop]-1;
		if (tmp[prm >> 3] & BitMaskTable[prm & 7]) {
			tmp[(lop >> 3) + 4] |= BitMaskTable[lop & 7];
		}
	}
	*(DWORD*)Src ^= *(DWORD*)(tmp + 4); // speeder method

	return;
}

// TODO: need cleanup
static void decode_des_etc(BYTE *buf, int len, int type, int cycle) {
	int lop, cnt = 0;

	if (cycle<3) cycle=3;
	else if (cycle<5) cycle++;
	else if (cycle<7) cycle+=9;
	else cycle+=15;

	for(lop=0;lop*8<len;lop++,buf+=8) {
		if (lop < 20 || (type == 0 && lop % cycle == 0)) { // des
			BitConvert(buf, BitSwapTable1);
			BitConvert4(buf);
			BitConvert(buf, BitSwapTable2);
		} else {
			if (cnt == 7 && type == 0) {
				int a;
				BYTE tmp[8];
				*(DWORD*)tmp     = *(DWORD*)buf;
				*(DWORD*)(tmp+4) = *(DWORD*)(buf+4);
				cnt=0;
				buf[0]=tmp[3];
				buf[1]=tmp[4];
				buf[2]=tmp[6];
				buf[3]=tmp[0];
				buf[4]=tmp[1];
				buf[5]=tmp[2];
				buf[6]=tmp[5];
				a=tmp[7];
				if (a==0x00) a=0x2b;
				else if (a==0x2b) a=0x00;
				else if (a==0x01) a=0x68;
				else if (a==0x68) a=0x01;
				else if (a==0x48) a=0x77;
				else if (a==0x77) a=0x48;
				else if (a==0x60) a=0xff;
				else if (a==0xff) a=0x60;
				else if (a==0x6c) a=0x80;
				else if (a==0x80) a=0x6c;
				else if (a==0xb9) a=0xc0;
				else if (a==0xc0) a=0xb9;
				else if (a==0xeb) a=0xfe;
				else if (a==0xfe) a=0xeb;
				buf[7] = a;
			}
			cnt++;
		}
	}

	return;
}

static unsigned char * decode_filename(unsigned char *buf, int len) {
	int lop;

	for(lop=0;lop<len;lop+=8) {
		NibbleSwap(&buf[lop],8);
		BitConvert(&buf[lop],BitSwapTable1);
		BitConvert4(&buf[lop]);
		BitConvert(&buf[lop],BitSwapTable2);
	}

	return buf;
}

/* END: INCLUDE FROM GRFIO.C */

#ifdef __WIN32
/* compatibility : function missing in windows. */

static char *dirname(char *d) {
	if (d == NULL) return ".";
	for(int i=strlen(d)-1;i>=0;i--) if ((*(d+i)=='/') || (*(d+i)=='\\')) {
		*(d+i)=0;
		return d;
	}
	return ".";
}
#endif

static void prv_grf_free_node(struct grf_node *node) {
	free(node->filename);
	if (node->next != NULL) node->next->prev = node->prev;
	if (node->prev != NULL) node->prev->next = node->next;
	free(node);
}

static void prv_grf_tree_table_free_node(struct grf_treenode *node) {
	if (node->is_dir) {
		hash_free_table(node->subdir);
	}
	if (node->name != NULL) free(node->name); // does not apply to root
	free(node);
}

static void prv_grf_reg_tree_node(struct grf_treenode *root, struct grf_node *cur_node) {
	char *fn = cur_node->filename;
	char dirname[4096];
	struct grf_treenode *parent = root;
	struct grf_treenode *new;
	while(1) {
		size_t pos;
		// locate either / or \ in filename...
		for(pos=0;(*(fn+pos)!=0) && (*(fn+pos)!='/') && (*(fn+pos)!='\\');pos++);
		memcpy(&dirname, fn, pos+1);
		if (*(fn+pos)==0) break; // record file~
		// this is a directory, check if already existing...
		dirname[pos] = 0;
		fn+=pos+1;
		new = hash_lookup(parent->subdir, dirname);
		if (new != NULL) { // found it !
			if (!new->is_dir) {
				// bogus grf file: directory with same name as a file... convert to directory !
				new->is_dir = true;
				new->subdir = hash_create_table(GRF_TREE_HASH_SIZE, prv_grf_tree_table_free_node);
			}
			parent = new;
			continue;
		}
		// not found -> create a new node
		new = (struct grf_treenode *)calloc(1, sizeof(struct grf_treenode));
		memset(new, 0, sizeof(struct grf_treenode));
		new->is_dir = true;
		new->subdir = hash_create_table(GRF_TREE_HASH_SIZE, prv_grf_tree_table_free_node);
		new->parent = parent;
		new->name = strdup((char *)&dirname);
		hash_add_element(parent->subdir, (char *)&dirname, new);
		parent = new;
	}
	// record file
	new = (struct grf_treenode *)calloc(1, sizeof(struct grf_treenode));
	memset(new, 0, sizeof(struct grf_treenode));
	new->is_dir = false;
	new->ptr = cur_node;
	new->parent = parent;
	cur_node->tree_parent = new;
	new->name = strdup((char *)&dirname);
	hash_add_element(parent->subdir, (char *)&dirname, new);
}

GRFEXPORT grf_handle grf_new_by_fd(int fd, bool writemode) {
	grf_handle handler;

	if (fd < 0) return NULL;

	handler = (grf_handle)calloc(1, sizeof(struct grf_handler));
	if (handler == NULL) { close(fd); return NULL; }
	memset(handler, 0, sizeof(grf_handle));
	handler->fast_table = hash_create_table(GRF_HASH_TABLE_SIZE, prv_grf_free_node);
	handler->fd = fd;
	handler->need_save = writemode; // file should be new (flag will be unset by prv_grf_load)
	handler->write_mode = writemode;
	handler->compression_level = 5; /* default ZLIB compression level */
	handler->version = GRF_FILE_OUTPUT_VERISON; /* default version */
	return handler;
}

grf_node prv_grf_find_free_space(grf_handle handler, size_t size, grf_node inode) {
	// find a "leak" between two files, to put our own file
	// our files are sorted, that's a good thing:)
	// We just have to return the node where the space is available, the other func will
	// insert his new node just after this one, so everything stays cool
	grf_node cur = handler->first_node;
	// case: inode is the first node (and is not null)
	if ((cur == inode) && (cur != NULL)) cur = cur->next;
	// case: there's no (other) file
	if (cur == NULL) return NULL; /* special case : nothing in the grf */
	while(cur->next != NULL) {
		struct grf_node *next = cur->next;
		uint32_t avail;
		if (next == inode) { /* skip the file refered by "inode" */
			next = next->next;
			if (next == NULL) break; /* ignore file is EOF */
		}
		// check available space between files
		avail = next->pos - (cur->pos + cur->len_aligned);
		if (avail >= size) break; /* yatta! */
		cur = next;
	}
	return cur;
}

GRFEXPORT grf_handle grf_new(const char *filename, bool writemode) {
	int fd;

	fd = open(filename, (writemode!=false?O_RDWR | O_CREAT:O_RDONLY) | OPEN_OPTIONS, 0744);
	return grf_new_by_fd(fd, writemode);
}

GRFEXPORT void grf_set_callback(grf_handle handler, bool (*callback)(void *, grf_handle, int, int, const char *), void *etc) {
	handler->callback = callback;
	handler->callback_etc = etc;
}

GRFEXPORT bool grf_merge(grf_handle dest, grf_handle src, uint8_t repack_type) {
	struct grf_node *cur, *rep, *prev;
	void *ptr;
	uint32_t i=0;
	if (!dest->write_mode) return false;
	// Rather simple :
	// 1. For each node in src
	cur = src->first_node;
	while(cur != NULL) {
		i++;
		if (dest->callback != NULL) if (!dest->callback(dest->callback_etc, dest, i, src->filecount, cur->filename)) break;
		dest->need_save = true;
		// 2. Seek same file in dst, if found, remove it from list. If not found, allocate a new grf_node struct
		rep = hash_lookup(dest->fast_table, cur->filename);
		prev = prv_grf_find_free_space(dest, cur->len_aligned, rep);
		if (rep != NULL) {
		// YAY! Everything made (almost) easy, but count file as replaced
			free(rep->filename);
			if (rep->next != NULL) rep->next->prev = rep->prev;
			if (rep->prev != NULL) rep->prev->next = rep->next;
			dest->wasted_space += rep->len_aligned;
			rep->filename = strdup(cur->filename);
		} else {
			// Regular add file~ (argh)
			rep = calloc(1, sizeof(struct grf_node));
			rep->filename = strdup(cur->filename);
			hash_add_element(dest->fast_table, rep->filename, rep);
			if (dest->root != NULL) prv_grf_reg_tree_node(dest->root, rep);
		}
		// filename: replace '/' with '\\' (if any)
		for(int i=0;*(rep->filename+i)!=0;i++) if (*(rep->filename+i)=='/') *(rep->filename+i)='\\';
		if (prev == NULL) {
			rep->pos = 0;
			if (rep != dest->first_node) rep->next = dest->first_node; // if we insert a new file, do some cleaning
			rep->prev = NULL;
			dest->first_node = rep;
		} else {
			// insert entry in the chained list
			rep->pos = prev->pos + prev->len_aligned;
			rep->next = prev->next;
			prev->next = rep;
			rep->prev = prev;
			if (rep->next != NULL) rep->next->prev = rep;
		}
		rep->size = cur->size;
		rep->cycle = cur->cycle;
		rep->len = cur->len;
		rep->len_aligned = cur->len_aligned;
		rep->flags = cur->flags;
		rep->parent = dest;
		// 5. Copy memory to file, and free() it
		lseek(dest->fd, rep->pos + GRF_HEADER_SIZE, SEEK_SET);
		lseek(src->fd, cur->pos + GRF_HEADER_SIZE, SEEK_SET);
#if 0
		if (repack_type == GRF_REPACK_FAST) {
//			off_t offset = cur->pos + GRF_HEADER_SIZE;
//			if (sendfile(dest->fd, src->fd, &offset, cur->len_aligned)!=cur->len_aligned) {
			if (splice(src->fd, NULL, dest->fd, NULL, cur->len_aligned, 0)) {
				perror("splice");
				hash_del_element(dest->fast_table, rep->filename);
				return false;
			}
		} else {
#endif
			ptr = calloc(1, cur->len_aligned + 1024); // in case of decrypt
			if (read(src->fd, ptr, cur->len_aligned) != cur->len_aligned) {
				free(ptr);
				hash_del_element(dest->fast_table, rep->filename);
				return false;
			}
			if (repack_type >= GRF_REPACK_DECRYPT) {
				// we have at least to decrypt the file, if encrypted.
				if (rep->cycle >= 0) decode_des_etc((unsigned char *)ptr, rep->len_aligned, (rep->cycle)==0, rep->cycle);
				// clear encryption flags...
				rep->cycle = -1;
				rep->flags = rep->flags & ~(GRF_FLAG_MIXCRYPT | GRF_FLAG_DES);
			}
			if (repack_type >= GRF_REPACK_RECOMPRESS) {
				// whoohoo recompress the file x.x
			}
			if (write(dest->fd, ptr, rep->len_aligned)!=rep->len_aligned) {
				free(ptr);
				hash_del_element(dest->fast_table, rep->filename);
				return false;
			}
			free(ptr);
#if 0
		}
#endif
		// 6. If we didn't use end of archive, but some wasted space, substract used space to handler->wasted_space
		if (rep->next != NULL) {
			dest->wasted_space -= rep->len_aligned;
		}
		cur = cur->next;
	}
	if (dest->callback != NULL) dest->callback(dest->callback_etc, dest, src->filecount, src->filecount, NULL);
	return true;
}

static void prv_grf_recount_wasted_space(struct grf_handler *handler) {
	struct stat s;
	uint32_t w;
	struct grf_node *node;

	if (fstat(handler->fd, &s)!=0) return;
	w = s.st_size - GRF_HEADER_SIZE - handler->table_size;
	node = handler->first_node;
	while(node != NULL) {
		w-=node->len_aligned;
		node=node->next;
	}
	handler->wasted_space = w;
}

GRFEXPORT bool grf_repack(grf_handle handler, uint8_t repack_type) {
	struct grf_node *node = handler->first_node;
	struct grf_node *prenode;
	uint32_t i=0;
	uint32_t save_pos = 0;
	if (!handler->write_mode) return false; /* opened in read-only mode -> repack fails */
	if (node == NULL) return true; // nothing to do on an empty file
	switch(repack_type) {
		case GRF_REPACK_FAST: break; case GRF_REPACK_DECRYPT: break; // case GRF_REPACK_RECOMPRESS: break;
		default: return false; /* bad parameter */
	}
	while(node->next != NULL) node = node->next;
	if (handler->table_offset >= (node->pos+node->len_aligned)) {
		save_pos = handler->table_offset + handler->table_size;
	} else {
		save_pos = node->pos+node->len_aligned;
	}
	node = handler->first_node;
	// ok, let's go!
	// Save header with version "0xCACA", but keep initial version. If we save again, it means it worked!
	i = handler->version;
	handler->version = (0xCACA | repack_type << 24); // save repack_type as well~
	grf_save(handler);
	handler->need_save = true;
	handler->version = i;
	// First operation: enumerate files, and find a gap
	i=0;
	// first node will never get moved, so create a "pre-first" node that'll be the one who won't get to be moved
	prenode = calloc(1, sizeof(struct grf_node));
	prenode->next = node;
	node = prenode;
	while(node != NULL) {
		struct grf_node *next = node->next;
		i++;
		if (next == NULL) break; /* can't remove void at end */
		if (node->pos+node->len_aligned < next->pos) { // found a gap !
			// save position of current file at end of GRF, in case of problem while repacking~
			void *filemem;
			int p=0;
			lseek(handler->fd, save_pos + GRF_HEADER_SIZE, SEEK_SET);
			write(handler->fd, (void *)(&next->pos), 4);
			if (handler->callback != NULL) if (!handler->callback(handler->callback_etc, handler, i, handler->filecount, next->filename)) break;
			filemem = calloc(1, next->len_aligned + 1024); // 1024 is needed in case of decryption
			lseek(handler->fd, next->pos + GRF_HEADER_SIZE, SEEK_SET);
			while (p<next->len_aligned) p+=read(handler->fd, filemem+p, next->len_aligned - p);
			// ok, we got the data :)
			if (repack_type >= GRF_REPACK_DECRYPT) {
				// we have at least to decrypt the file, if encrypted.
				if (next->cycle >= 0) decode_des_etc((unsigned char *)filemem, next->len_aligned, (next->cycle)==0, next->cycle);
				// clear encryption flags...
				next->cycle = -1;
				next->flags = next->flags & ~(GRF_FLAG_MIXCRYPT | GRF_FLAG_DES);
			}
			if (repack_type >= GRF_REPACK_RECOMPRESS) {
				// NOT SUPPORTED ! (to be implemented on next release - which will be something like 0.1.22)
//				void *filenew = malloc(next->size);
//				count = zlib_buffer_inflate(filenew, next->size, filemem, next->len);
			}
			// write the file to its new localtion !
			p=0;
			next->pos = node->pos+node->len_aligned;
			lseek(handler->fd, next->pos + GRF_HEADER_SIZE, SEEK_SET);
			while(p<next->len_aligned) p+=write(handler->fd, filemem+p, next->len_aligned - p);
		} else {
			bool need_write = false;
			void *filemem;
			int p;
			// no need to move file, but...
			if (repack_type >= GRF_REPACK_DECRYPT) {
				if (next->cycle >= 0) {
					if (handler->callback != NULL) handler->callback(handler->callback_etc, handler, i, handler->filecount, next->filename);
					filemem = calloc(1, next->len_aligned + 1024); // 1024 is needed for decryption
					p=0;
					lseek(handler->fd, next->pos + GRF_HEADER_SIZE, SEEK_SET);
					while (p<next->len_aligned) p+=read(handler->fd, filemem+p, next->len_aligned - p);
					decode_des_etc((unsigned char *)filemem, next->len_aligned, (next->cycle)==0, next->cycle);
					need_write = true;
					next->cycle = -1;
					next->flags = next->flags & ~(GRF_FLAG_MIXCRYPT | GRF_FLAG_DES);
				}
			}
			if (need_write) {
				p=0;
				lseek(handler->fd, node->pos + node->len_aligned + GRF_HEADER_SIZE, SEEK_SET);
				while(p<next->len_aligned) p+=write(handler->fd, filemem+p, next->len_aligned - p);
			}
		}
		node = next;
	}
	free(prenode);
	grf_save(handler);
	prv_grf_recount_wasted_space(handler);
	return true;
}

static inline size_t prv_grf_strnlen(const char *str, const size_t maxlen) {
	for(size_t i=0;i<maxlen;i++) if (*(str+i)==0) return i;
	return maxlen;
}

GRFEXPORT void grf_create_tree(grf_handle handler) {
	struct grf_node *cur_node;
	uint32_t i=0, j=100;
	if (handler->root != NULL) return;
	// the idea is simple : get to each file and scan them~
	// First, create the root node...
	handler->root = (struct grf_treenode *)calloc(1, sizeof(struct grf_treenode));
	memset(handler->root, 0, sizeof(struct grf_treenode));
	handler->root->is_dir = true; // root is a directory, that's common knowledge
	handler->root->name = NULL; // root does not have a name
	handler->root->subdir = hash_create_table(GRF_TREE_HASH_SIZE, prv_grf_tree_table_free_node);
	// now, list all files in the archive...
	cur_node = handler->first_node;
	while(cur_node != NULL) {
		// ... and register 'em
		prv_grf_reg_tree_node(handler->root, cur_node);
		cur_node = cur_node->next;
		i++;
		if (--j<=0) {
			j=100;
			if (handler->callback != NULL) {
				handler->callback(handler->callback_etc, handler, i, handler->filecount, cur_node->filename);
			}
		}
	}
	if (handler->callback != NULL) {
		handler->callback(handler->callback_etc, handler, handler->filecount, handler->filecount, NULL);
	}
}

GRFEXPORT grf_treenode grf_tree_get_root(grf_handle handler) {
	return handler->root;
}

GRFEXPORT grf_treenode *grf_tree_list_node(grf_treenode node) {
	if (!node->is_dir) return NULL;
	return (grf_treenode *)hash_foreach_val(node->subdir);
}

GRFEXPORT bool grf_tree_is_dir(grf_treenode node) {
	return node->is_dir;
}

GRFEXPORT uint32_t grf_tree_dir_count_files(grf_treenode node) {
	return node->subdir->count;
}

GRFEXPORT grf_node grf_tree_get_file(grf_treenode node) {
	return node->ptr;
}

GRFEXPORT const char *grf_tree_get_name(grf_treenode node) {
	return node->name;
}

GRFEXPORT grf_treenode grf_tree_get_parent(grf_treenode node) {
	return node->parent;
}

GRFEXPORT grf_treenode grf_file_get_tree(grf_node handler) {
	return handler->tree_parent;
}

GRFEXPORT void grf_update_id_list(grf_handle handler) {
	struct grf_node *cur;
	int i=0;
	if (handler->node_table != NULL) free(handler->node_table);
	handler->node_table = calloc(handler->filecount + 1, sizeof(void *));
	cur = handler->first_node;
	while(cur != NULL) {
		handler->node_table[i] = cur;
		cur->id = i++;
		cur = cur->next;
	}
}

//  quickSort
//
//  This public-domain C implementation by Darel R. Finley.
//  Edited by MagicalTux for libgrf
//  http://alienryderflex.com/quicksort/
//
//  * Returns true if sort was successful, or false if the nested
//    pivots went too deep, in which case your array will have
//    been re-ordered, but probably not sorted correctly.
//
//  * This function assumes it is called with valid parameters.
//
//  * Example calls:
//    quickSort(&myArray[0],5); // sorts elements 0, 1, 2, 3, and 4
//    quickSort(&myArray[3],5); // sorts elements 3, 4, 5, 6, and 7

#define PRV_GRF_QUICKSORT_MAX_LEVELS 1000
static bool prv_grf_quicksort(struct grf_handler *handler, uint32_t elements) {
  uint32_t beg[PRV_GRF_QUICKSORT_MAX_LEVELS], end[PRV_GRF_QUICKSORT_MAX_LEVELS];
	int i=0, L, R;
	struct grf_node *piv;
	struct grf_node **arr;
	
	arr = (struct grf_node **)hash_foreach_val(handler->fast_table);

  beg[0]=0; end[0]=elements;
  while (i>=0) {
    L=beg[i]; R=end[i]-1;
    if (L<R) {
      piv=arr[L]; if (i==PRV_GRF_QUICKSORT_MAX_LEVELS-1) return NULL;
      while (L<R) {
        while (arr[R]->pos>=piv->pos && L<R) R--; if (L<R) arr[L++]=arr[R];
        while (arr[L]->pos<=piv->pos && L<R) L++; if (L<R) arr[R--]=arr[L]; }
      arr[L]=piv; beg[i+1]=L+1; end[i+1]=end[i]; end[i++]=L; }
    else {
      i--; }}
	// restore order~
	piv = NULL;
	for(unsigned int i=0;arr[i]!=NULL;i++) {
		arr[i]->prev = piv;
		if (piv!=NULL) piv->next = arr[i];
		piv = arr[i];
	}
	piv->next = NULL;
	handler->first_node = arr[0];
	free(arr);
	grf_update_id_list(handler);
	return true;
}

static bool prv_grf_load(struct grf_handler *handler) {
	struct grf_header head;
	struct stat grfstat;
	uint32_t posinfo[2];
	uint32_t wasted_space=0;
	uint32_t brokenpos;
	int dlen, result;
	void *table, *table_comp, *pos, *pos_max;
	struct grf_node *entry, *last;
	int hcall = 100;

	// load header...
	handler->need_save = false;
	lseek(handler->fd, 0, SEEK_SET);
	result = read(handler->fd, (void *)&head, sizeof(struct grf_header));
	if (result != sizeof(struct grf_header)) return false;

	if (strncmp(head.header_magic, GRF_HEADER_MAGIC, sizeof(head.header_magic)) != 0) return false; // bad magic !
	for(int i=1;i<=sizeof(head.header_key);i++) if ((head.header_key[i-1] != i) && (head.header_key[i-1] != 0)) return false;
	switch(head.version) {
		case 0x102: case 0x103: case 0x200: case 0xCACA: break;
		default: return false; /* unknown version */
	}

	handler->table_offset = head.offset;
	handler->filecount = head.filecount - head.seed - 7;
	// version was set from grf_new()
//	handler->version = GRF_FILE_OUTPUT_VERISON; /* do not store version as we'll save to this version anyway, unless we're repacking */
	last = handler->first_node;
	if (last != NULL) {
		while(last->next != NULL) last = last->next; // seek to end of list
	}

	if (handler->filecount == 0) return true; // do not even bother reading file table, it's empty

	switch(head.version) {
		case 0x102: case 0x103: // old GRF files
//			if (handler->write_mode) return false; // disallow usage of "write mode" for those files. Repack is needed to write to those files.
			if (fstat(handler->fd, (struct stat *)&grfstat) != 0) return false;
			if ((handler->table_offset + GRF_HEADER_SIZE) > grfstat.st_size) return false;

			lseek(handler->fd, handler->table_offset + GRF_HEADER_SIZE, SEEK_SET);
			dlen = grfstat.st_size - (handler->table_offset + GRF_HEADER_SIZE);
			table = malloc(dlen);
			if (read(handler->fd, (void *)table, dlen) != dlen) { free(table); return false; }
			result = handler->filecount;
			wasted_space = handler->table_offset;
			pos = table;
			pos_max=table+dlen;
			while(pos < pos_max) {
				result--;
				size_t av_len = pos_max - pos;
				int fn_len = (*(uint32_t*)pos)-2; pos+=4+2;
				struct grf_table_entry_data tmpentry;
				char ext[3];
				if (fn_len>av_len) { free(table); return false; }
				entry = calloc(1, sizeof(struct grf_node));
				entry->filename = calloc(1, fn_len + 1);
				memcpy(entry->filename, pos, fn_len); // fn_len + 1 is already 0x00
				decode_filename((unsigned char *)entry->filename, fn_len);
				pos += fn_len;
				fn_len = strlen(entry->filename);
				memcpy((void *)&tmpentry, pos, sizeof(struct grf_table_entry_data));
				pos += sizeof(struct grf_table_entry_data);
				if ( ((tmpentry.flags & GRF_FLAG_FILE) == 0) || (tmpentry.size == 0)) {
					// do not register "directory" entries and empty(bogus) files
					free(entry->filename);
					free(entry);
					continue;
				}

				entry->flags = tmpentry.flags;
				entry->size = tmpentry.size;
				entry->len = tmpentry.len-tmpentry.size-715;
				entry->len_aligned = tmpentry.len_aligned-37579;
				entry->pos = tmpentry.pos;
				entry->parent = handler;
				entry->cycle = 0;
				wasted_space -= entry->len_aligned;
				// check file extension
				if (*((entry->filename)+(fn_len-4)) == '.') {
					char *nocrypt_list="gndgatactstr";
					bool nocrypt = false;
					memcpy(&ext, ((entry->filename)+(fn_len-3)), 3);
					for(int i=0;i<3;i++) if ((ext[i]>='A') && (ext[i]<='Z')) ext[i]+=32;
					// only files which are *NOT* one of : .gnd .gat .act .str
					// are encrypted using method GRF_FLAG_MIXCRYPT. Other files are using
					// GRF_FLAG_DES (cycle = 0).
					// we have to compare the obtained extension (in ext) against those four possibilities
					for(int j=0;j<4;j++) {
						char *p=nocrypt_list+(j*3);
						bool eq=true;
						for(int i=0;i<3;i++) if (ext[i]!=*(p+i)) eq=false;
						if (eq) {
							nocrypt=true; /* found one! */
							break;
						}
					}
					if (!nocrypt) {
						entry->flags |= GRF_FLAG_MIXCRYPT;
						entry->cycle = 1;
						for(int i=10; (entry->len)>=i; i=i*10) entry->cycle++;
					} else {
						entry->flags |= GRF_FLAG_DES;
					}
				} else {
					entry->flags |= GRF_FLAG_DES;
				}

				if (last == NULL) {
					last = entry;
					handler->first_node = entry;
				} else {
					last->next = entry;
					entry->prev = last;
					last = entry;
				}
				hash_add_element(handler->fast_table, entry->filename, entry);
				if (--hcall<=0) {
					hcall = 100;
					if (handler->callback != NULL) {
						if (!handler->callback(handler->callback_etc, handler, handler->filecount - result, handler->filecount, entry->filename)) return false;
					}
				}
			}
			free(table);
			break;
		case 0xCACA: // broken-by-repack
			// ATTEMPT TO REPAIR FILE
			// We'll find in the file, after the files table, 4 bytes containing the original position
			// of the last moved file.
			// We'll try to restore this file with 4 attempt, and consider that all previous files
			// were successfully moved. 
			// Attempt 1: Read the file in its original place, try to unpack it.
			// Attempt 2: Read the file as if it were successfully moved. Try to unpack it.
			// Attempt 3: Read 50% of the file in the new position, read 50% in the old position. Try to unpack it.
			// Attempt 4: Read whole file from new position. Read 1024 bytes from old position, try to unpack it.
			//            Read 1024 bytes before, from the old position, try again to unpack. Continue until the
			//            whole file has been read from the old position, or until unpack success.
			// If all attempts failed, we just return false. If we successfully found the original file, write the
			// info in the GRF table.
			// TODO: Add an option to just drop the file (if all attempts failed). Also add a way to know *which* file
			// failed.
			return false; // TODO TODO TODO TODO XXX FIXME TODO XXX FIXME
			if (fstat(handler->fd, (struct stat *)&grfstat) != 0) return false;
			if ((handler->table_offset + GRF_HEADER_SIZE) > grfstat.st_size) return false;

			lseek(handler->fd, handler->table_offset + GRF_HEADER_SIZE, SEEK_SET);
			if (read(handler->fd, (void *)&posinfo, sizeof(posinfo)) != sizeof(posinfo)) return false;
			// posinfo[0] = comp size
			// posinfo[1] = decomp size

			if ((handler->table_offset + GRF_HEADER_SIZE + (sizeof(uint32_t)*2) + posinfo[0] + sizeof(uint32_t)) > grfstat.st_size) return false;
			table_comp = malloc(posinfo[0]);
			table = malloc(posinfo[1]);
			if (read(handler->fd, table_comp, posinfo[0]) != posinfo[0]) { free(table); free(table_comp); return false; }
			if (read(handler->fd, (void *)&brokenpos, sizeof(uint32_t)) != sizeof(uint32_t)) { free(table); free(table_comp); return false; }
			if (zlib_buffer_inflate(table, posinfo[1], table_comp, posinfo[0]) != posinfo[1]) { free(table); free(table_comp); return false; }

			free(table_comp);

			pos = table;
			pos_max = table + posinfo[1];
			result = handler->filecount;
			wasted_space = grfstat.st_size - GRF_HEADER_SIZE - 8 - posinfo[0]; // in theory, all this space should be used for files
			while(pos < pos_max) {
				size_t av_len = pos_max - pos;
				int fn_len = prv_grf_strnlen((char *)pos, av_len);
				struct grf_table_entry_data tmpentry;
				result--;
				if (fn_len + sizeof(struct grf_table_entry_data) > av_len) { free(table); return false; }
				entry = calloc(1, sizeof(struct grf_node));
				entry->filename = calloc(1, fn_len + 1);
				memcpy(entry->filename, pos, fn_len); // fn_len + 1 is already 0x00
				pos += fn_len + 1;
				memcpy((void *)&tmpentry, pos, sizeof(struct grf_table_entry_data));
				pos += sizeof(struct grf_table_entry_data);
				if ( ((tmpentry.flags & GRF_FLAG_FILE) == 0) || (tmpentry.size == 0)) {
					// do not register "directory" entries and empty(bogus) files
					free(entry->filename);
					free(entry);
					continue;
				}
				entry->flags = tmpentry.flags;
				entry->size = tmpentry.size;
				entry->len = tmpentry.len;
				entry->len_aligned = tmpentry.len_aligned;
				entry->pos = tmpentry.pos;
				entry->parent = handler;
				entry->cycle = -1;
				if (entry->flags & GRF_FLAG_MIXCRYPT) {
					entry->cycle = 1;
					for(int i=10; entry->len>=i; i=i*10) entry->cycle++;
				}
				if (entry->flags & GRF_FLAG_DES) {
					entry->cycle = 0;
				}
				wasted_space -= tmpentry.len_aligned;

				if (last == NULL) {
					last = entry;
					handler->first_node = entry;
				} else {
					last->next = entry;
					entry->prev = last;
					last = entry;
				}
				hash_add_element(handler->fast_table, entry->filename, entry);
				if (--hcall<=0) {
					hcall = 100;
					if (handler->callback != NULL) {
						if (!handler->callback(handler->callback_etc, handler, handler->filecount - result, handler->filecount, entry->filename)) return false;
					}
				}
			}
			free(table);
			break;
		case 0x200: // new GRF files
			if (fstat(handler->fd, (struct stat *)&grfstat) != 0) return false;
			if ((handler->table_offset + GRF_HEADER_SIZE) > grfstat.st_size) return false;

			lseek(handler->fd, handler->table_offset + GRF_HEADER_SIZE, SEEK_SET);
			if (read(handler->fd, (void *)&posinfo, sizeof(posinfo)) != sizeof(posinfo)) return false;
			// posinfo[0] = comp size
			// posinfo[1] = decomp size

			if ((handler->table_offset + GRF_HEADER_SIZE + 8 + posinfo[0]) > grfstat.st_size) return false;
			table_comp = malloc(posinfo[0]);
			table = malloc(posinfo[1]);
			if (read(handler->fd, table_comp, posinfo[0]) != posinfo[0]) { free(table); free(table_comp); return false; }
			if (zlib_buffer_inflate(table, posinfo[1], table_comp, posinfo[0]) != posinfo[1]) { free(table); free(table_comp); return false; }

			free(table_comp);

			pos = table;
			pos_max = table + posinfo[1];
			result = handler->filecount;
			wasted_space = grfstat.st_size - GRF_HEADER_SIZE - 8 - posinfo[0]; // in theory, all this space should be used for files
			while(pos < pos_max) {
				size_t av_len = pos_max - pos;
				int fn_len = prv_grf_strnlen((char *)pos, av_len);
				struct grf_table_entry_data tmpentry;
				result--;
				if (fn_len + sizeof(struct grf_table_entry_data) > av_len) { free(table); return false; }
				entry = calloc(1, sizeof(struct grf_node));
				entry->filename = calloc(1, fn_len + 1);
				memcpy(entry->filename, pos, fn_len); // fn_len + 1 is already 0x00
				pos += fn_len + 1;
				memcpy((void *)&tmpentry, pos, sizeof(struct grf_table_entry_data));
				pos += sizeof(struct grf_table_entry_data);
				if ( ((tmpentry.flags & GRF_FLAG_FILE) == 0) || (tmpentry.size == 0)) {
					// do not register "directory" entries and empty(bogus) files
					free(entry->filename);
					free(entry);
					continue;
				}
				entry->flags = tmpentry.flags;
				entry->size = tmpentry.size;
				entry->len = tmpentry.len;
				entry->len_aligned = tmpentry.len_aligned;
				entry->pos = tmpentry.pos;
				entry->parent = handler;
				entry->cycle = -1;
				if (entry->flags & GRF_FLAG_MIXCRYPT) {
					entry->cycle = 1;
					for(int i=10; entry->len>=i; i=i*10) entry->cycle++;
				}
				if (entry->flags & GRF_FLAG_DES) {
					entry->cycle = 0;
				}
				wasted_space -= tmpentry.len_aligned;

				if (last == NULL) {
					last = entry;
					handler->first_node = entry;
				} else {
					last->next = entry;
					entry->prev = last;
					last = entry;
				}
				hash_add_element(handler->fast_table, entry->filename, entry);
				if (--hcall<=0) {
					hcall = 100;
					if (handler->callback != NULL) {
						if (!handler->callback(handler->callback_etc, handler, handler->filecount - result, handler->filecount, entry->filename)) return false;
					}
				}
			}
			free(table);
			break;
		default:
			return false;
	}
	if (result != 0) return false;
	handler->wasted_space = wasted_space;
	// sort entries using quicksort
	handler->filecount = handler->fast_table->count;
	entry = handler->first_node;
	if (entry == NULL) return true; // no files?
	if (!prv_grf_quicksort(handler, handler->filecount)) return false;
	// overlap check
	struct grf_node *x = handler->first_node;
	uint32_t prev=0;
	while(x != NULL) {
		if (prev > x->pos+x->len_aligned) {
			struct grf_node *x2;
#if 0
			printf("******** OVERLAP :\n");
			x=x->prev;
			printf("file %s at %u-%u: s=%u l=%u l_=%u\n", x->filename, x->pos+GRF_HEADER_SIZE, x->pos+x->len_aligned+GRF_HEADER_SIZE, x->size, x->len, x->len_aligned);
			x=x->next;
			printf("file %s at %u-%u: s=%u l=%u l_=%u\n", x->filename, x->pos+GRF_HEADER_SIZE, x->pos+x->len_aligned+GRF_HEADER_SIZE, x->size, x->len, x->len_aligned);
#endif
			// drop the second one
			x2 = x;
			x = x->next;
			handler->wasted_space += x2->len_aligned;
			hash_del_element(handler->fast_table, x2->filename);
			continue;
		}
		prev=x->pos+x->len_aligned;
		x=x->next;
	}
	// call the callback, if any~
	if (handler->callback != NULL) {
		handler->callback(handler->callback_etc, handler, handler->filecount, handler->filecount, NULL);
	}

	return true;
}

GRFEXPORT grf_handle grf_load_from_new(grf_handle handler) {
	if (handler == NULL) return NULL;
	
	if (prv_grf_load((struct grf_handler *)handler) == false) {
		grf_free(handler);
		return NULL;
	}
	return handler;
}

GRFEXPORT grf_handle grf_load(const char *filename, bool writemode) {
	void *handler;

	handler = grf_new(filename, writemode);

	return grf_load_from_new(handler);
}

GRFEXPORT bool grf_file_rename(grf_node handler, const char *newname) {
	void *rep;
	if (!handler->parent->write_mode) return false;
	handler->parent->need_save = true;
	rep = grf_get_file(handler->parent, newname);
	if (rep != NULL) grf_file_delete(rep);
	if (hash_remove_element(handler->parent->fast_table, handler->filename)!=0) return false;
	if (handler->tree_parent != NULL) hash_del_element(handler->tree_parent->parent->subdir, handler->tree_parent->name);
	free(handler->filename);
	handler->filename = strdup(newname);
	hash_add_element(handler->parent->fast_table, handler->filename, handler);
	if (handler->parent->root != NULL) prv_grf_reg_tree_node(handler->parent->root, handler);
	return true;
}

GRFEXPORT bool grf_file_delete(grf_node handler) {
	struct grf_handler *parent = handler->parent;
	uint32_t len_aligned = handler->len_aligned;
	struct grf_node *next = handler->next;
	if (!parent->write_mode) return false;
	parent->need_save = true;
	if (handler->tree_parent != NULL) hash_del_element(handler->tree_parent->parent->subdir, handler->tree_parent->name); // will free memory automatically
	if (hash_del_element(handler->parent->fast_table, handler->filename)!=0) return false;
	if (parent->first_node==handler) parent->first_node = next;
	parent->wasted_space += len_aligned; /* wasted_space accounting */
	parent->filecount--;
	return true;
}

GRFEXPORT uint32_t grf_filecount(grf_handle handler) {
	return handler->filecount;
}

GRFEXPORT uint32_t grf_wasted_space(grf_handle handler) {
	return handler->wasted_space;
}

GRFEXPORT grf_node grf_get_file(grf_handle handler, const char *filename) {
	return hash_lookup(handler->fast_table, filename);
}

GRFEXPORT const char *grf_file_get_filename(grf_node handler) {
	return handler->filename;
}

GRFEXPORT const char *grf_file_get_basename(grf_node handler) {
	char *name = handler->filename;
	char *name2 = name + strlen(name);
	for(;(*(name2-1)!='/') && (*(name2-1)!='\\') && (name2>name);name2--);
	return name2;
}

GRFEXPORT uint32_t grf_file_get_size(grf_node handler) {
	return handler->size;
}

GRFEXPORT uint32_t grf_file_get_storage_pos(grf_node handler) {
	return handler->pos;
}

GRFEXPORT uint32_t grf_file_get_storage_flags(grf_node handler) {
	return handler->flags;
}

GRFEXPORT uint32_t grf_file_get_storage_size(grf_node handler) {
	return handler->len_aligned;
}

GRFEXPORT uint32_t grf_file_get_id(grf_node node) {
	return node->id;
}

GRFEXPORT grf_node grf_get_file_by_id(grf_handle handler, uint32_t id) {
	return handler->node_table[id];
}

GRFEXPORT grf_node *grf_get_file_id_list(grf_handle handler) {
	return handler->node_table;
}

GRFEXPORT uint32_t grf_file_get_contents(grf_node fhandler, void *target) {
	void *comp;
	struct grf_handler *handler;
	uint32_t count;
	handler = fhandler->parent;
	if ((fhandler->flags & GRF_FLAG_FILE) == 0) return 0; // not a file
	comp = calloc(1, fhandler->len_aligned + 1024); // seems that we need to allocate 1024 more bytes to decrypt file safely
	lseek(handler->fd, fhandler->pos + GRF_HEADER_SIZE, SEEK_SET);
	count = read(handler->fd, (char *)comp, fhandler->len_aligned);
	if (count != fhandler->len_aligned) { free(comp); return 0; }
	// decrypt (if required)
	//static void decode_des_etc(unsigned char *buf, int len, int type, int cycle) 
	if (fhandler->cycle >= 0) decode_des_etc((unsigned char *)comp, fhandler->len_aligned, (fhandler->cycle)==0, fhandler->cycle);
	// decompress to target...
	count = zlib_buffer_inflate(target, fhandler->size, comp, fhandler->len);
	free(comp);
	return count;
}

GRFEXPORT uint32_t grf_file_put_contents_to_fd(grf_node file, int fd) {
	uint32_t size;
	void *ptr;
	uint32_t p=0;
	size = grf_file_get_size(file);
	if (size == 0) return 0;
	ptr = malloc(size);
	if (ptr == NULL) return 0;
	if (grf_file_get_contents(file, ptr) != size) {
		free(ptr);
		return 0;
	}
	while(1) {
		int i=write(fd, ptr+p, size-p);
		if (i<=0) {
			free(ptr);
			return 0;
		}
		p+=i;
		if (p==size) break;
	}
	free(ptr);
	return size;
}

static bool prv_grf_do_mkdir(char *name) {
	char *n = strdup(name);
	char *b = strdup(dirname(n));
	struct stat s;
	memset(&s, 0, sizeof(s));
	free(n);
	stat(b, &s);
	if ((s.st_mode & S_IFDIR) == S_IFDIR) { free(b); return true; } /* already good */
#ifdef __WIN32
	if (mkdir(b) == 0) 
#else
	if (mkdir(b, 0755) == 0) 
#endif
	{
		free(b);
		return true;
	}
	prv_grf_do_mkdir(b);
#ifdef __WIN32
	if (mkdir(b) != 0)
#else
	if (mkdir(b, 0755) != 0)
#endif
	{
		return false;
	}
	free(b);
	return true;
}

GRFEXPORT bool grf_put_contents_to_file(grf_node file, const char *fn) {
	int i,p=0;
	char *name;
	size_t len, size;
	void *ptr;
	FILE *f;
	name = strdup(fn);
	len = strlen(name);
	size = grf_file_get_size(file);
	if (size == 0) return false;
	for(i=0;i<len;i++) if (*(name+i)=='\\') *(name+i) = '/';
	prv_grf_do_mkdir(name);
	f = fopen(name, "wb");
	if (f==NULL) {
		free(name);
		return false;
	}
	ptr = malloc(size);
	if ((ptr==NULL) || (grf_file_get_contents(file, ptr) != size)) {
		free(ptr);
		free(name);
		fclose(f);
		return false;
	}
	while(1) {
		int i=fwrite(ptr+p, 1, size-p, f);
		if (i<=0) {
			free(ptr);
			free(name);
			fclose(f);
			return 0;
		}
		p+=i;
		if (p==size) break;
	}
	free(ptr);
	fclose(f);
	free(name);
	return true;
}


GRFEXPORT grf_node grf_file_add(grf_handle handler, const char *filename, void *ptr, size_t size) {
	// returns pointer to the newly created file structure
	void *ptr_comp;
	struct grf_node *prev, *ptr_file;
	uint32_t comp_size, comp_size_aligned;
	if (handler->write_mode == false) return NULL; // no write access
	// STEPS
	// 1. Compress file, to have its size
	ptr_comp = malloc(size+100);
	if (ptr_comp == NULL) return NULL; /* out of memory? */
	comp_size = zlib_buffer_deflate(ptr_comp, size + 100, ptr, size, handler->compression_level);
	comp_size_aligned = comp_size + (4-((comp_size-1) % 4)) - 1;
	ptr_comp = realloc(ptr_comp, comp_size_aligned);
	if (ptr_comp == NULL) return NULL; /* out of memory? */
	// 2. Check if a file already exists with the same name.
	ptr_file = hash_lookup(handler->fast_table, filename);
	// 3. Find a place to add the file, and add it
	prev = prv_grf_find_free_space(handler, comp_size_aligned, ptr_file);
	// 4. Rebuild index, replace file if needed, etc...
	if (ptr_file != NULL) {
		// YAY! Everything made (almost) easy, but count file as replaced
		free(ptr_file->filename);
		handler->wasted_space += ptr_file->len_aligned;
		if (ptr_file->next != NULL) ptr_file->next->prev = ptr_file->prev;
		if (ptr_file->prev != NULL) ptr_file->prev->next = ptr_file->next;
		ptr_file->filename = strdup(filename);
	} else {
		// Regular add file~ (argh)
		ptr_file = calloc(1, sizeof(struct grf_node));
		ptr_file->filename = strdup(filename);
		ptr_file->parent = handler;
		hash_add_element(handler->fast_table, ptr_file->filename, ptr_file);
		if (handler->root != NULL) prv_grf_reg_tree_node(handler->root, ptr_file);
	}
	// filename: replace '/' with '\\'
	for(int i=0;*(ptr_file->filename+i)!=0;i++) if (*(ptr_file->filename+i)=='/') *(ptr_file->filename+i)='\\';
	if (prev == NULL) {
		ptr_file->pos = 0;
		ptr_file->next = handler->first_node; // just in case, supposed to be null
		ptr_file->prev = NULL;
		handler->first_node = ptr_file;
	} else {
		// insert entry in the chained list
		ptr_file->pos = prev->pos + prev->len_aligned;
		ptr_file->next = prev->next;
		ptr_file->prev = prev;
		if (ptr_file->next != NULL) ptr_file->next->prev = ptr_file;
		prev->next = ptr_file;
	}
	ptr_file->size = size;
	ptr_file->len = comp_size;
	ptr_file->len_aligned = comp_size_aligned;
	ptr_file->flags = GRF_FLAG_FILE;
	// 5. Copy memory to file, and free() it
	lseek(handler->fd, ptr_file->pos + GRF_HEADER_SIZE, SEEK_SET);
	if (write(handler->fd, ptr_comp, ptr_file->len_aligned)!=ptr_file->len_aligned) {
		free(ptr_comp);
		hash_del_element(handler->fast_table, ptr_file->filename);
		return NULL;
	}
	free(ptr_comp);
	// 6. If we didn't use end of archive, but some wasted space, substract used space to handler->wasted_space
	if (ptr_file->next != NULL) {
		handler->wasted_space -= ptr_file->len_aligned;
	}
	handler->need_save = true;
	return ptr_file;
}

GRFEXPORT grf_node grf_file_add_fd(grf_handle handler, const char *filename, int fp) {
	void *ptr, *res;
	struct stat s;

	if (fp < 0) return NULL;
	if (fstat(fp, &s) != 0) return NULL;
	ptr = malloc(s.st_size);
	if (read(fp, ptr, s.st_size) != s.st_size) { free(ptr); return NULL; }
	res = grf_file_add(handler, filename, ptr, s.st_size);
	free(ptr);
	return res;
}

GRFEXPORT grf_node grf_file_add_path(grf_handle handler, const char *filename, const char *real_filename) {
	int fp;
	void *res;

	fp = open(real_filename, O_RDONLY);
	res = grf_file_add_fd(handler, filename, fp);
	close(fp);
	return res;
}

GRFEXPORT grf_node *grf_get_file_list(grf_handle handler) {
	return (grf_node *)hash_foreach_val(handler->fast_table);
}

GRFEXPORT grf_node grf_get_file_first(grf_handle handler) {
	return handler->first_node;
}

GRFEXPORT grf_node grf_get_file_next(grf_node handler) {
	return handler->next;
}

GRFEXPORT grf_node grf_get_file_prev(grf_node handler) {
	return handler->prev;
}

GRFEXPORT void grf_set_compression_level(grf_handle handler, int level) {
	handler->compression_level = level;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

static bool prv_grf_write_header(struct grf_handler *handler) {
	struct grf_header file_header;
	int result;
	
	memset(&file_header, 0, sizeof(struct grf_header));
	strncpy((char *)&file_header.header_magic, GRF_HEADER_MAGIC, sizeof(file_header.header_magic));
	for(uint8_t i=1;i<=sizeof(file_header.header_key);i++) file_header.header_key[i-1]=i;

	file_header.offset = handler->table_offset;
	file_header.filecount = handler->filecount + 7;
	file_header.version = handler->version;

	lseek(handler->fd, 0, SEEK_SET);
	result = write(handler->fd, (void *)&file_header, sizeof(struct grf_header));
	if (result != sizeof(struct grf_header)) return false;
	handler->need_save = false;
	return true;
}

static bool prv_grf_write_table(struct grf_handler *handler) {
	// Step 1 : generate a proper table
	// We need to determine the final size of the table. It's :
	// Len of Filename + 1 + sizeof(struct grf_table_entry_data)
	uint32_t table_size=0;
	struct grf_node *node = handler->first_node;
	void *table, *pos;
	struct grf_node *prev;

	while(node != NULL) {
		table_size += strlen(node->filename)+1+sizeof(struct grf_table_entry_data);
		node = node->next;
	}

	if (table_size == 0) {
		table = (void *)malloc(1);
		*(char *)(table) = 0;
	} else {
		table = (void *)malloc(table_size);
		pos=table;
		node = handler->first_node;
		while(node != NULL) {
			struct grf_table_entry_data te;
			int j=strlen(node->filename);
			memcpy(pos, node->filename, j);
			pos+=j;
			*(char *)(pos) = 0;
			pos++;
			te.len = node->len;
			te.len_aligned = node->len_aligned;
			te.size = node->size;
			te.pos = node->pos;
			te.flags = node->flags;
			memcpy(pos, (void *)&te, sizeof(struct grf_table_entry_data));
			pos+=sizeof(struct grf_table_entry_data);
			node = node->next;
		}
	}

	pos = malloc(table_size + 100);
	*(uint32_t *)(pos+4) = table_size; /* initial size */

	// Compress the table using zlib
	table_size = zlib_buffer_deflate(pos+8, table_size + 100 -8, table, table_size, handler->compression_level);
	free(table);
	if (table_size == 0) {
		free(pos);
		return false;
	}
	
	*(uint32_t *)(pos) = table_size; /* compressed size */
	table_size += 8;
	handler->table_size = table_size;
	/* compute new position for the table */
	prev = prv_grf_find_free_space(handler, table_size, NULL);
	if (prev == NULL) { // no files
		handler->table_offset = 0;
	} else {
		handler->table_offset = prev->pos + prev->len_aligned;
	}
	lseek(handler->fd, handler->table_offset + GRF_HEADER_SIZE, SEEK_SET);
	if (write(handler->fd, (char *)pos, table_size) != table_size) { free(pos); return false; }
	free(pos);
	if (prev == NULL) { // no file found in archive -> truncate at end of table
		ftruncate(handler->fd, handler->table_offset + GRF_HEADER_SIZE + table_size);
	} else if (prev->next == NULL) { // file was EOF -> truncate at end of table 
		ftruncate(handler->fd, handler->table_offset + GRF_HEADER_SIZE + table_size);
	} else { // some files are after us, truncate at end of last file
		uint32_t pos = 0;
		node = handler->first_node;
		while(node != NULL) { // TODO: Optimize this (as we know that files are sorted, optimization is possible)
			uint32_t p=node->pos+node->len_aligned; // position of EOF
			pos = MAX(pos, p);
			node = node->next;
		}
		ftruncate(handler->fd, pos+GRF_HEADER_SIZE);
	}
	return true;
}

GRFEXPORT void grf_free(grf_handle handler) {
	if (handler == NULL) return;

	if (handler->need_save) grf_save(handler);
	close(handler->fd);
	hash_free_table(handler->fast_table);
	if (handler->root != NULL) prv_grf_tree_table_free_node(handler->root);
	if (handler->node_table != NULL) free(handler->node_table);
	free(handler);
}

GRFEXPORT bool grf_save(grf_handle handler) {
	if (handler == NULL) return false;

	handler->filecount = handler->fast_table->count;
	if (prv_grf_write_table(handler) != true) { return false; }
	if (prv_grf_write_header(handler) != true) { return false; }

	return true;
}

