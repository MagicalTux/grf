#include <stdlib.h>
#include <zlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grf.h>

/* BEGIN: INCLUDE FROM GRFIO.C */

#ifdef __WIN32
#include <windows.h>
#else
/* Since GRF is a windows-type file, we're using windows types "BYTE", "WORD" and "DWORD".
 * However, when we compile on __WIN32 machine, those types are already defined in windef.h
 * included from windows.h, so we don't need to redefine them ! */
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
#endif

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

static void NibbleSwap(BYTE *Src, int len) {
	for( ; 0 < len; len--, Src++) {
		*Src = (*Src >> 4) | (*Src << 4);
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

static void prv_grf_free_node(struct grf_node *node) {
	free(node->filename);
	if (node->next != NULL) node->next->prev = node->prev;
	if (node->prev != NULL) node->prev->next = node->next;
	free(node);
}

GRFEXPORT void *grf_new(const char *filename, bool writemode) {
	struct grf_handler *handler;
	int fd;

#ifdef O_LARGEFILE
	fd = open(filename, (writemode!=false?O_RDWR | O_CREAT:O_RDONLY) | O_LARGEFILE, 0744);
#else
	fd = open(filename, (writemode!=false?O_RDWR | O_CREAT:O_RDONLY), 0744);
#endif
	if (fd < 0) return NULL;

	handler = (struct grf_handler *)malloc(sizeof(struct grf_handler));
	if (handler == NULL) { close(fd); return NULL; }
	memset(handler, 0, sizeof(struct grf_handler));
	handler->fast_table = hash_create_table(GRF_HASH_TABLE_SIZE, prv_grf_free_node);
	handler->fd = fd;
	handler->need_save = writemode; // file should be new (flag will be unset by prv_grf_load)
	handler->write_mode = writemode;
	return handler;
}

static inline size_t prv_grf_strnlen(const char *str, const size_t maxlen) {
	for(size_t i=0;i<maxlen;i++) if (*(str+i)==0) return i;
	return maxlen;
}

static void prv_grf_tree_table_free_node(struct grf_treenode *node) {
	if (node->is_dir) {
		hash_free_table(node->subdir);
	}
	free(node);
}

GRFEXPORT void grf_create_tree(void *tmphandler) {
	struct grf_node *cur_node;
	struct grf_handler *handler = tmphandler;
	if (handler->root != NULL) return;
	// the idea is simple : get to each file and scan them~
	// First, create the root node...
	handler->root = (struct grf_treenode *)malloc(sizeof(struct grf_treenode));
	memset(handler->root, 0, sizeof(struct grf_treenode));
	handler->root->is_dir = true; // root is a directory, that's common knowledge
	handler->root->subdir = hash_create_table(GRF_TREE_HASH_SIZE, prv_grf_tree_table_free_node);
	// now, list all files in the archive
	cur_node = handler->first_node;
	while(cur_node != NULL) {
		char *fn = cur_node->filename;
		char dirname[128];
		struct grf_treenode *parent = handler->root;
		struct grf_treenode *new;
		while(1) {
			size_t pos;
			// locate either / or \ in filename...
			for(pos=0;(*(fn+pos)!=0) && (*(fn+pos)!='/') && (*(fn+pos)!='\\');pos++);
			memcpy(&dirname, fn, pos);
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
			new = (struct grf_treenode *)malloc(sizeof(struct grf_treenode));
			memset(new, 0, sizeof(struct grf_treenode));
			new->is_dir = true;
			new->subdir = hash_create_table(GRF_TREE_HASH_SIZE, prv_grf_tree_table_free_node);
			hash_add_element(parent->subdir, (char *)&dirname, new);
			parent = new;
		}
		// record file
		new = (struct grf_treenode *)malloc(sizeof(struct grf_treenode));
		memset(new, 0, sizeof(struct grf_treenode));
		new->is_dir = false;
		new->ptr = cur_node;
		hash_add_element(parent->subdir, (char *)&dirname, new);
		cur_node = cur_node->next;
	}
}

static bool prv_grf_load(struct grf_handler *handler) {
	struct grf_header head;
	struct stat grfstat;
	uint32_t posinfo[2];
	int32_t wasted_space=0;
	int dlen, result;
	void *table, *table_comp, *pos, *pos_max;

	// load header...
	handler->need_save = false;
	lseek(handler->fd, 0, SEEK_SET);
	result = read(handler->fd, (void *)&head, sizeof(struct grf_header));
	if (result != sizeof(struct grf_header)) return false;

	if (strncmp(head.header_magic, GRF_HEADER_MAGIC, sizeof(head.header_magic)) != 0) return false; // bad magic !
	for(int i=1;i<=sizeof(head.header_key);i++) if ((head.header_key[i-1] != i) && (head.header_key[i-1] != 0)) return false;
	switch(head.version) {
		case 0x102: case 0x103: case 0x200: break;
		default: return false; /* unknown version */
	}

	handler->table_offset = head.offset;
	handler->filecount = head.filecount - head.seed - 7;

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
				struct grf_node *entry;
				char ext[3];
				if (fn_len>av_len) { free(table); return false; }
				entry = malloc(sizeof(struct grf_node));
				entry->filename = calloc(1, fn_len + 1);
				memcpy(entry->filename, pos, fn_len); // fn_len + 1 is already 0x00
				decode_filename((unsigned char *)entry->filename, fn_len);
				pos += fn_len;
				fn_len = strlen(entry->filename);
				memcpy((void *)&tmpentry, pos, sizeof(struct grf_table_entry_data));
				pos += sizeof(struct grf_table_entry_data);
				if ((tmpentry.flags & GRF_FLAG_FILE) == 0) {
					// do not register "directory" entries
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

				entry->next = handler->first_node;
				entry->prev = NULL;
				if (handler->first_node != NULL) handler->first_node->prev = entry;
				handler->first_node = entry;
				hash_add_element(handler->fast_table, entry->filename, entry);
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
			wasted_space = handler->table_offset + 8;
			while(pos < pos_max) {
				size_t av_len = pos_max - pos;
				int fn_len = prv_grf_strnlen((char *)pos, av_len);
				struct grf_table_entry_data tmpentry;
				struct grf_node *entry;
				result--;
				if (fn_len + sizeof(struct grf_table_entry_data) > av_len) { free(table); return false; }
				entry = malloc(sizeof(struct grf_node));
				entry->filename = calloc(1, fn_len + 1);
				memcpy(entry->filename, pos, fn_len); // fn_len + 1 is already 0x00
				pos += fn_len + 1;
				memcpy((void *)&tmpentry, pos, sizeof(struct grf_table_entry_data));
				pos += sizeof(struct grf_table_entry_data);
				if ((tmpentry.flags & GRF_FLAG_FILE) == 0) {
					// do not register "directory" entries
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

				entry->next = handler->first_node;
				entry->prev = NULL;
				if (handler->first_node != NULL) handler->first_node->prev = entry;
				handler->first_node = entry;
				hash_add_element(handler->fast_table, entry->filename, entry);
			}
			free(table);
			break;
		default:
			return false;
	}
	if (result != 0) return false;
	handler->wasted_space = wasted_space;

	return true;
}

GRFEXPORT void *grf_load(const char *filename, bool writemode) {
	void *handler;

	handler = grf_new(filename, writemode);
	if (handler == NULL) return NULL;

	if (prv_grf_load((struct grf_handler *)handler) == false) {
		grf_free(handler);
		return NULL;
	}
	return handler;
}

GRFEXPORT bool grf_file_delete(void *tmphandler) {
	struct grf_node *handler;
	handler = (struct grf_node *)tmphandler;
	handler->parent->need_save = true;
	if (hash_del_element(handler->parent->fast_table, handler->filename)==0) return true;
	return false;
}

GRFEXPORT uint32_t grf_filecount(void *tmphandler) {
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	return handler->filecount;
}

GRFEXPORT uint32_t grf_wasted_space(void *tmphandler) {
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	return handler->wasted_space;
}

GRFEXPORT void *grf_get_file(void *tmphandler, const char *filename) {
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	return hash_lookup(handler->fast_table, filename);
}

GRFEXPORT const char *grf_file_get_filename(void *tmphandler) {
	struct grf_node *handler;
	handler = (struct grf_node *)tmphandler;
	return handler->filename;
}

GRFEXPORT uint32_t grf_file_get_size(void *tmphandler) {
	struct grf_node *handler;
	handler = (struct grf_node *)tmphandler;
	return handler->size;
}

GRFEXPORT uint32_t grf_file_get_contents(void *tmphandler, void *target) {
	void *comp;
	struct grf_node *fhandler;
	struct grf_handler *handler;
	uint32_t count;
	fhandler = (struct grf_node *)tmphandler;
	handler = fhandler->parent;
	if ((fhandler->flags & GRF_FLAG_FILE) == 0) return 0; // not a file
	comp = malloc(fhandler->len_aligned);
	lseek(handler->fd, fhandler->pos + GRF_HEADER_SIZE, SEEK_SET);
	count = read(handler->fd, (char *)comp, fhandler->len_aligned);
	if (count != fhandler->len_aligned) { free(comp); return 0; }
	// decrypt (if required)
	//static void decode_des_etc(unsigned char *buf, int len, int type, int cycle) {
	if (fhandler->cycle >= 0) decode_des_etc((unsigned char *)comp, fhandler->len_aligned, (fhandler->cycle)==0, fhandler->cycle);
	// decompress to target...
	count = zlib_buffer_inflate(target, fhandler->size, comp, fhandler->len);
	free(comp);
	return count;
}

GRFEXPORT void *grf_file_add(void *tmphandler, const char *filename, const void *ptr, int size) {
//	void *comp;
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	
	handler->need_save = true;
	return NULL;
}

static bool prv_grf_write_header(struct grf_handler *handler) {
	struct grf_header file_header;
	int result;
	
	memset(&file_header, 0, sizeof(struct grf_header));
	strncpy((char *)&file_header.header_magic, GRF_HEADER_MAGIC, sizeof(file_header.header_magic));
	for(uint8_t i=1;i<=sizeof(file_header.header_key);i++) file_header.header_key[i-1]=i;

	file_header.offset = handler->table_offset;
	file_header.filecount = handler->filecount + 7;
	file_header.version = GRF_FILE_OUTPUT_VERISON;

	lseek(handler->fd, 0, SEEK_SET);
	result = write(handler->fd, (void *)&file_header, sizeof(struct grf_header));
	if (result != sizeof(struct grf_header)) return false;
	return true;
}

GRFEXPORT void **grf_get_file_list(void *tmphandler) {
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	return hash_foreach_val(handler->fast_table);
}

static bool prv_grf_write_table(struct grf_handler *handler) {
	// Step 1 : generate a proper table
	// We need to determine the final size of the table. It's :
	// Len of Filename + 1 + sizeof(struct grf_table_entry_data)
	uint32_t table_size=0;
	list_element **files_list;
	files_list = hash_foreach(handler->fast_table);
	void *table, *pos;

	if (files_list != NULL) {
		for(int i=0;files_list[i]!=NULL;i++) {
			table_size += strlen(((struct grf_node *)(files_list[i])->pointer)->filename) + 1 + sizeof(struct grf_table_entry_data);
		}
	}

	if (table_size == 0) {
		table = (void *)malloc(1);
		*(char *)(table) = 0;
	} else {
		table = (void *)malloc(table_size);
		pos=table;
		for(int i=0;files_list[i]!=NULL;i++) {
			struct grf_table_entry_data te;
			int j=strlen(((struct grf_node *)(files_list[i])->pointer)->filename);
			memcpy(pos, ((struct grf_node *)(files_list[i])->pointer)->filename, j);
			pos+=j;
			*(char *)(pos) = 0;
			pos++;
			te.len = ((struct grf_node *)(files_list[i])->pointer)->len;
			te.len_aligned = ((struct grf_node *)(files_list[i])->pointer)->len_aligned;
			te.size = ((struct grf_node *)(files_list[i])->pointer)->size;
			te.pos = ((struct grf_node *)(files_list[i])->pointer)->pos;
			te.flags = ((struct grf_node *)(files_list[i])->pointer)->flags;
			memcpy(pos, (void *)&te, sizeof(struct grf_table_entry_data));
			pos+=sizeof(struct grf_table_entry_data);
		}
		free(files_list);
	}

	pos = malloc(table_size + 100);
	*(uint32_t *)(pos+4) = table_size; /* initial size */

	// Compress the table using zlib
	table_size = zlib_buffer_deflate(pos+8, table_size + 100 -8, table, table_size);
	free(table);
	if (table_size == 0) {
		free(pos);
		return false;
	}
	
	*(uint32_t *)(pos) = table_size; /* compressed size */
	table_size += 8;
	lseek(handler->fd, handler->table_offset + GRF_HEADER_SIZE, SEEK_SET);
	if (write(handler->fd, (char *)pos, table_size) != table_size) { free(pos); return false; }
	free(pos);
	handler->need_save = false;

	return true;
}

GRFEXPORT void grf_free(void *tmphandler) {
	struct grf_handler *handler;
//	struct grf_node *fn, *fn2;
	handler = (struct grf_handler *)tmphandler;
	if (handler == NULL) return;

	if (handler->need_save) {
		prv_grf_write_header(handler);
		prv_grf_write_table(handler);
	}
	close(handler->fd);
	/*
	fn = handler->first_node;
	while(fn != NULL) {
		free(fn->filename);
		fn2=fn->next;
		free(fn);
		fn=fn2;
	}*/
	hash_free_table(handler->fast_table);
	if (handler->root != NULL) prv_grf_tree_table_free_node(handler->root);
	free(handler);
}

GRFEXPORT bool grf_save(void *tmphandler) {
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	if (handler == NULL) return false;

	if (prv_grf_write_header(handler) != true) { return false; }
	if (prv_grf_write_table(handler) != true) { return false; }

	return true;
}

