#include <grf.h>
#include <stdlib.h>
#include <zlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
	fd = open(filename, (writemode!=false?O_RDWR:O_RDONLY) | O_CREAT | O_LARGEFILE, 0744);
#else
	fd = open(filename, (writemode!=false?O_RDWR:O_RDONLY) | O_CREAT, 0744);
#endif
	if (fd < 0) return NULL;

	handler = (struct grf_handler *)malloc(sizeof(struct grf_handler));
	if (handler == NULL) { close(fd); return NULL; }
	memset(handler, 0, sizeof(struct grf_handler));
	handler->fast_table = hash_create_table(GRF_HASH_TABLE_SIZE, prv_grf_free_node);
	handler->fd = fd;
	handler->need_save = writemode; // file should be new (flag will be unset by prv_grf_load)
	return handler;
}

inline size_t prv_grf_strnlen(const char *str, const size_t maxlen) {
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

bool prv_grf_load(struct grf_handler *handler) {
	struct grf_header head;
	struct stat grfstat;
	uint32_t posinfo[2];
	int32_t wasted_space=0;
	int result;
	void *table, *table_comp, *pos, *pos_max;

	// load header...
	handler->need_save = false;
	lseek(handler->fd, 0, SEEK_SET);
	result = read(handler->fd, (void *)&head, sizeof(struct grf_header));
	if (result != sizeof(struct grf_header)) return false;

	if (strncmp(head.header_magic, GRF_HEADER_MAGIC, sizeof(head.header_magic)) != 0) return false; // bad magic !
	for(int i=1;i<=sizeof(head.header_key);i++) if (head.header_key[i-1] != i) return false;
	if (head.version != 0x200) return false;

	handler->table_offset = head.offset;
	handler->filecount = head.filecount - head.seed - 7;

	if (handler->filecount == 0) return true; // do not even bother reading file table, it's empty

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
	wasted_space = handler->table_offset;
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
		entry->type = tmpentry.type;
		entry->size = tmpentry.size;
		entry->len = tmpentry.len;
		entry->len_aligned = tmpentry.len_aligned;
		entry->pos = tmpentry.pos;
		entry->parent = handler;
		wasted_space -= tmpentry.len_aligned;

		entry->next = handler->first_node;
		entry->prev = NULL;
		if (handler->first_node != NULL) handler->first_node->prev = entry;
		handler->first_node = entry;
		hash_add_element(handler->fast_table, entry->filename, entry);
	}
	if (result != 0) return false;
	if (wasted_space < 0) {
		wasted_space = -1; // got more files data than can fit before the files table ?
	}
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
	comp = malloc(fhandler->len_aligned);
	lseek(handler->fd, fhandler->pos + GRF_HEADER_SIZE, SEEK_SET);
	count = read(handler->fd, (char *)comp, fhandler->len_aligned);
	if (count != fhandler->len_aligned) { free(comp); return 0; }
	// decompress to target...
	switch(fhandler->type) {
		case 1: break;
		default: return 0;
	}
	count = zlib_buffer_inflate(target, fhandler->size, comp, fhandler->len);
	free(comp);
	return count;
}

GRFEXPORT bool grf_file_add(void *tmphandler, const char *filename, const void *ptr, int size) {
//	void *comp;
	struct grf_handler *handler;
	handler = (struct grf_handler *)tmphandler;
	
	handler->need_save = true;
	return false;
}

bool prv_grf_write_header(struct grf_handler *handler) {
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

bool prv_grf_write_table(struct grf_handler *handler) {
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
			*(char *)(pos+1) = 0;
			pos++;
			te.len = ((struct grf_node *)(files_list[i])->pointer)->len;
			te.len_aligned = ((struct grf_node *)(files_list[i])->pointer)->len_aligned;
			te.size = ((struct grf_node *)(files_list[i])->pointer)->size;
			te.pos = ((struct grf_node *)(files_list[i])->pointer)->pos;
			te.type = ((struct grf_node *)(files_list[i])->pointer)->type;
			memcpy(pos, (void *)&te, sizeof(struct grf_table_entry_data));
			pos+=sizeof(struct grf_table_entry_data);
		}
	}
	free(files_list);

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
	struct grf_node *fn, *fn2;
	handler = (struct grf_handler *)tmphandler;
	if (handler == NULL) return;

	if (handler->need_save) {
		prv_grf_write_header(handler);
		prv_grf_write_table(handler);
	}
	close(handler->fd);
	fn = handler->first_node;
	while(fn != NULL) {
		free(fn->filename);
		fn2=fn->next;
		free(fn);
		fn=fn2;
	}
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

