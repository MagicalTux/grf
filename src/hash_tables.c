/* hash_tables.c : Hash tables manager
 * $Id: hash_tables.c 2 2005-07-24 23:28:07Z magicaltux $
 *
 */

/* System includes */
#include <stdlib.h>
#include <string.h>

/* Local includes */
#include <hash_tables.h>
#include <grf.h>

/* Our hash function
 * It is based on the published UNIX ELF hash algorythm used to find object files
 * This version is less efficient but fits our needs
 */
unsigned long hash_calc(char *name, unsigned long limit) {
	unsigned long h = 0, g;
	char *name2;
	name2=name;
	while(*name) {
		h = (h << 4) + *(name++);
		/* gcc3 will display a warning if we don't put the additional parentheses */
		if ( (g = (h & 0xF0000000)) ) h ^= (g >> 24);
		h &= ~g;
	}
	h = h % limit;
	return h;
}

/* a little function to get lowercased string
 */
char *strduptolower(const char *str) {
	char *res,*tmp;
	res=(char *)strdup(str);
	tmp=res-1;
	while(*(++tmp)) {
		if ((*tmp>='A') && (*tmp<='Z')) *tmp+=32;
		if (*tmp=='\\') *tmp='/';
	}
	return res;
}

hash_table *hash_create_table(unsigned long size, void *func) {
	hash_table *new_table;

	if (size<1) return NULL; /* illegal table size */
	
	/* Attempt to malloc some memory */
	if ((new_table = calloc(1, sizeof(hash_table))) == NULL) {
		return NULL;
	}
	new_table->free_func = func;

	/* get some memory for our dynamic array */
	if ((new_table->table = calloc(1, sizeof(list_element)*size)) == NULL) {
		free(new_table);
		return NULL;
	}

	new_table->size=size;

	return new_table;
}

list_element *hash_lookup_raw(hash_table *table, const char *string) {
	list_element *element;
	unsigned long hash_val;
	char *str;

	str=strduptolower(string);
	hash_val=hash_calc(str, table->size);
	for(element=table->table[hash_val]; element != NULL; element=element->next) {
		if (strcmp(str, element->string)==0) {
			free(str);
			return element;
		}
	}
	free(str);

	return NULL;
}

void *hash_lookup(hash_table *table, const char *string) {
	list_element *element;
	element=hash_lookup_raw(table,string);

	if (element == NULL) return NULL;

	return element->pointer;
}

int hash_set_element(hash_table *table, char *string, void *pointer, int delold) {
	list_element *current_element;

	if (table == NULL) return 1;

	current_element = hash_lookup_raw(table, string);
	if (current_element == NULL) return 2;

	if ( (delold != 0 ) && (table->free_func != NULL)) (table->free_func)(current_element->pointer);
	current_element->pointer = pointer;
	return 0;
}

int hash_add_element(hash_table *table, char *string, void *pointer) {
	list_element *new_element;
	list_element *current_element;
	unsigned long hashval;
	char *str;

	str=strduptolower(string);
	hashval=hash_calc(str, table->size);
	current_element=hash_lookup_raw(table, string);
	if (current_element != NULL) {
		free(str);
		return 2; /* already present in hash table */
	}

	if ((new_element = malloc(sizeof(list_element))) == NULL) {
		free(str);
		return 1;
	}

	new_element->string = str;
	new_element->next = table->table[hashval];
	new_element->pointer = pointer;
	table->table[hashval]=new_element;
	table->count += 1;

	return 0;
}

int hash_del_element(hash_table *table, char *string) {
	list_element *old_element;
	list_element *current_element, *prev;
	unsigned long hashval;
	char *str;

	str=strduptolower(string);
	hashval=hash_calc(str, table->size);
	old_element=hash_lookup_raw(table, str);
	free(str);
	if (old_element == NULL) return 1; /* not found in table */
	if (table->free_func != NULL) (table->free_func)(old_element->pointer);

	prev=NULL;
	for(current_element=table->table[hashval]; current_element != NULL; current_element=current_element->next) {
		if (current_element==old_element) {
			if (prev==NULL) {
				table->table[hashval] = current_element->next;
				table->count -= 1;
				free(current_element->string);
				free(current_element);
				return 0;
			}
			prev->next=current_element->next;
			table->count -= 1;
			free(current_element->string);
			free(current_element);
			return 0;
		}
		prev=current_element;
	}
	return 2; /* not found ! */
}

int hash_remove_element(hash_table *table, char *string) {
	/* same as previous, but do not free the element */
	list_element *old_element;
	list_element *current_element, *prev;
	unsigned long hashval;
	char *str;

	str=strduptolower(string);
	hashval=hash_calc(str, table->size);
	old_element=hash_lookup_raw(table, str);
	free(str);
	if (old_element == NULL) return 1; /* not found in table */

	prev=NULL;
	for(current_element=table->table[hashval]; current_element != NULL; current_element=current_element->next) {
		if (current_element==old_element) {
			if (prev==NULL) {
				table->table[hashval] = current_element->next;
				table->count -= 1;
				free(current_element->string);
				free(current_element);
				return 0;
			}
			prev->next=current_element->next;
			table->count -= 1;
			free(current_element->string);
			free(current_element);
			return 0;
		}
		prev=current_element;
	}
	return 2; /* not found ! */
}

void hash_free_table(hash_table *table) {
	int i;
	list_element *cur,*prev;

	if (table == NULL) return;

	for(i=0;i<table->size; i++) {
		cur=table->table[i];
		while(cur!=NULL) {
			if (table->free_func != NULL) (table->free_func)(cur->pointer);
			prev=cur;
			cur=cur->next;
			free(prev->string);
			free(prev);
		}
	}

	free(table->table);
	free(table);
}

list_element **hash_foreach(hash_table *table) {
	/* will return an array of pointers with every entry of the hash table list */
	list_element **result, *cur;
	int num_entries=0,num_alloc=0, i;
	result = NULL;

	if (table == NULL) return NULL;

	for(i=0;i<table->size; i++) {
		cur=table->table[i];
		while(cur != NULL) {
			num_entries++;
			if (num_entries > num_alloc) {
				num_alloc+=512;
				if (result == NULL) {
					result = calloc(num_alloc, sizeof(void *));
				} else {
					result = realloc(result, sizeof(void *) * num_alloc);
				}
			}
			result[num_entries-1]=cur;
			cur=cur->next;
		}
	}
	if (result != NULL) {
		result = realloc(result, sizeof(void *) * (num_entries+1));
		result[num_entries]=NULL;
	}
	return result;
}

void **hash_foreach_val(hash_table *table) {
	/* will return an array of pointers with every pointer in the list */
	void **result;
	list_element *cur;
	int num_entries=0,num_alloc=0, i;
	result = NULL;

	if (table == NULL) return NULL;

	for(i=0;i<table->size; i++) {
		cur=table->table[i];
		while(cur != NULL) {
			num_entries++;
			if (num_entries > num_alloc) {
				num_alloc+=512;
				if (result == NULL) {
					result = calloc(num_alloc, sizeof(void *));
				} else {
					result = realloc(result, sizeof(void *) * num_alloc);
				}
			}
			result[num_entries-1]=cur->pointer;
			cur=cur->next;
		}
	}
	if (result != NULL) {
		result = realloc(result, sizeof(void *) * (num_entries+1));
		result[num_entries]=NULL;
	}
	return result;
}

