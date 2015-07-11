/*
 * cfg2
 * a simplistic configuration parser for INI like syntax in ANSI C
 *
 * author: lubomir i. ivanov (neolit123 at gmail)
 * this code is released in the public domain without warranty of any kind.
 * providing credit to the original author is recommended but not mandatory.
 *
 * cfg2.c:
 *	this file holds the library definitions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cfg2.h"

/* This helper function added by Anders "Ichimusai" Sikvall although I
   do not claim authorship to this code as it is likely to be very
   similar to other such code snippets since donkey years ago...

   I wanted to deal with trimming of white space at the beginning and
   end of strings to be able to cope with configuration files and make
   the following two lines to behave as equivalent:

   key=value
   key = value

   Just as well as other varieties, such as indentation and other
   things in the key/value pair that I think is sanity to get rid of.

   PARAMS

         char *str 
                 String to trim whitespace at beginning and end from.

   RETURNS

         The start of the string which should always be the same
         address as the string inserted to trim. This is done in this
         way because I don't want to screw up any malloc()/free()
         pairs for the strings.
*/

char *trimwhitespace(char *str)
{
	size_t len = 0;
	char *frontp = str;
	char *endp = NULL;
	
	if(str == NULL) 
		return NULL;
	
	if(str[0] == '\0') 
		return str;
	
	len = strlen(str);
	endp = str + len;
	
	// Move the front and back pointers to address the first
	// non-whitespace characters from each end.
	while( isspace(*frontp) ) { ++frontp; }
	if(endp != frontp) {
		while( isspace(*(--endp)) && endp != frontp ) {}
	}
	
	if( str + len - 1 != endp )
		*(endp + 1) = '\0';
	else if( frontp != str &&  endp == frontp )
		*str = '\0';
	// Shift the string so that it starts at str so that if it's
        // dynamically allocated, we can still free it on the returned pointer.
        // Note the reuse of endp to mean the front of the string
        // buffer now.
	endp = str;
	if(frontp != str) {
		while( *frontp ) { *endp++ = *frontp++; }
		*endp = '\0';
	}

	// Return the pointer
	return str;
}


void cfg_cache_clear(cfg_t *st)
{
	// Clearing the cache buffers technically sets them to the first index (0)
	if (st->cache_size && st->init == CFG_TRUE) {
		memset((void *)st->cache_keys_hash, 0, st->cache_size * sizeof(cfg_uint32));
		memset((void *)st->cache_keys_index, 0, st->cache_size * sizeof(cfg_int));
	}
}

cfg_error_t cfg_cache_size_set(cfg_t *st, cfg_int size)
{
	int diff;
	if (st->init != CFG_TRUE)
		return CFG_ERROR_INIT;
	if (size < 0)
		return CFG_ERROR_CRITICAL;
	// Check if we are setting the buffers to zero length
	if (size == 0) {
		if (st->cache_keys_hash)
			free(st->cache_keys_hash);
		st->cache_keys_hash = NULL;
		if (st->cache_keys_index)
			free(st->cache_keys_index);
		st->cache_keys_index = NULL;
	} else {
		st->cache_keys_hash = (cfg_uint32 *)realloc(st->cache_keys_hash, size * sizeof(cfg_uint32));
		st->cache_keys_index = (cfg_int *)realloc(st->cache_keys_index, size * sizeof(cfg_int));
		if (!st->cache_keys_index || !st->cache_keys_hash)
			return CFG_ERROR_ALLOC;
		// if the new buffers are larger, lets fill the extra indexes with zeroes
		if (size > st->cache_size) {
			diff = size - st->cache_size;
			memset((void *)&st->cache_keys_hash[st->cache_size], 0, diff * sizeof(cfg_uint32));
			memset((void *)&st->cache_keys_index[st->cache_size], 0, diff * sizeof(cfg_int));
		}
	}
	st->cache_size = size;
	return CFG_ERROR_OK;
}

cfg_error_t cfg_init(cfg_t *st, cfg_int cache_size)
{
	st->entry = NULL;
	st->buf = NULL;
	st->file = NULL;
	st->nkeys = 0;
	st->buf_size = 0;
	st->cache_keys_index = NULL;
	st->cache_keys_hash = NULL;
	st->init = CFG_TRUE;
	if (cache_size < 0)
		st->cache_size = CFG_CACHE_SIZE;
	else
		st->cache_size = cache_size;
	return CFG_ERROR_OK;
}

// fast fnv-32 hash
static cfg_uint32 cfg_hash_get(cfg_char *str)
{
	cfg_uint32 hash = 0x811c9dc5;
	if (str)
		while(*str) {
			// or multiple with addition and bit shifting:
			// hash += (hash << 1) + (hash << 4) + (hash
			// << 7) + (hash << 8) + (hash << 24); 
			hash *= hash;
			hash ^= *str++;
		}
	return hash;
}

#define cfg_escape_char_trim(c, p, end)		\
	*p = c;					\
	memmove(p + 1, p + 2, end - (p + 1));	\
	*((end)--) = '\0'

#define cfg_multi_line_trim(p, end)	  \
	memmove(p, p + 2, end - (p + 2)); \
	end -= 2;			  \
	*(end) = '\0'

// escape all special characters (like \n) in a string
static cfg_char *cfg_value_escape(cfg_char *str) {
	cfg_char *p = str, *end = str + strlen(str);
	cfg_char next;

	while (*p) {
		if (*p == '\\') {
			next = *(p + 1);
			switch (next) {
			case '\n': // multi-line value: remove the \\ and \n characters
				cfg_multi_line_trim(p, end);
				break;
			case 'n':
				cfg_escape_char_trim('\n', p, end);
				break;
			case 't':
				cfg_escape_char_trim('\t', p, end);
				break;
			case 'r':
				cfg_escape_char_trim('\r', p, end);
				break;
			case 'v':
				cfg_escape_char_trim('\v', p, end);
				break;
			case 'f':
				cfg_escape_char_trim('\f', p, end);
				break;
			case 'b':
				cfg_escape_char_trim('\b', p, end);
				break;
			}
		}
		p++;
	}
	return str;
}

cfg_entry_t *cfg_entry_nth(cfg_t *st, cfg_int n)
{
	if (n < 0 || n > st->nkeys - 1 || !st->nkeys)
		return NULL;
	return &(st->entry[n]);
}

cfg_char* cfg_key_nth(cfg_t *st, cfg_int n)
{
	if (n < 0 || n > st->nkeys - 1 || !st->nkeys)
		return NULL;
	return st->entry[n].key;
}

cfg_char* cfg_value_nth(cfg_t *st, cfg_int n)
{
	if (n < 0 || n > st->nkeys - 1 || !st->nkeys)
		return NULL;
	return st->entry[n].value;
}

cfg_int cfg_key_get_index(cfg_t *st, cfg_char *key)
{
	cfg_int i = 0;
	cfg_uint32 hash;

	if (!key || !st->nkeys)
		return -1;
	hash = cfg_hash_get(key);
	while (i < st->nkeys) {
		if (hash == st->entry[i].key_hash)
			return i;
		i++;
	}
	return -1;
}

cfg_char* cfg_key_get(cfg_t *st, cfg_char *value)
{
	cfg_int i = 0;
	cfg_uint32 hash;

	if (!value || !st->nkeys)
		return NULL;
	hash = cfg_hash_get(value);
	while (i < st->nkeys) {
		if (hash == st->entry[i].value_hash)
			return st->entry[i].key;
		i++;
	}
	return NULL;
}

cfg_char* cfg_value_get(cfg_t *st, cfg_char *key)
{
	cfg_int i;
	cfg_uint32 hash;

	if (!key || !st->nkeys)
		return NULL;
	hash = cfg_hash_get(key);

	// check for value in cache first
	if (st->cache_size > 0) {
		i = 0;
		while (i < st->cache_size) {
			if (hash == st->cache_keys_hash[i])
				return st->entry[st->cache_keys_index[i]].value;
			i++;
		}
	}

	// check for value in main list
	i = 0;
	while (i < st->nkeys) {
		if (hash == st->entry[i].key_hash) {
			if (st->cache_size > 0) {
				// lets add to the cache
				if (st->cache_size > 1) {
					memmove((void *)(st->cache_keys_hash + 1), (void *)st->cache_keys_hash,
					        (st->cache_size - 1) * sizeof(cfg_uint32));
					memmove((void *)(st->cache_keys_index + 1), (void *)st->cache_keys_index,
					        (st->cache_size - 1) * sizeof(cfg_int));
				}
				st->cache_keys_hash[0] = hash;
				st->cache_keys_index[0] = i;
			}
			return st->entry[i].value;
		}
		i++;
	}
	return NULL;
}

cfg_long cfg_value_get_long(cfg_t *st, cfg_char *key, cfg_int base)
{
	cfg_char *str = cfg_value_get(st, key);
	if (str)
		return strtol(str, NULL, base);
	return 0;
}

cfg_ulong cfg_value_get_ulong(cfg_t *st, cfg_char *key, cfg_int base)
{
	cfg_char *str = cfg_value_get(st, key);
	if (str)
		return strtoul(str, NULL, base);
	return 0;
}

// msvcr does not support strtof so we only use strtod for doubles
cfg_double cfg_value_get_double(cfg_t *st, cfg_char *key)
{
	cfg_char *str = cfg_value_get(st, key);
	if (str)
		return strtod(str, NULL);
	return 0.0;
}

cfg_error_t cfg_value_set(cfg_t *st, cfg_char *key, cfg_char *value)
{
	cfg_int i = 0;
	cfg_int len = strlen(value);
	cfg_uint32 hash_key;

	if (!value || !key || st->nkeys == 0)
		return CFG_ERROR_CRITICAL;
	hash_key = cfg_hash_get(key);
	value = cfg_value_escape(value);
	while (i < st->nkeys) {
		if (hash_key == st->entry[i].key_hash) {
			st->entry[i].value_hash = cfg_hash_get(value);
			if (st->entry[i].value)
				free(st->entry[i].value);
			st->entry[i].value = (cfg_char *)malloc((len + 1) * sizeof(cfg_char));
			if (!st->entry[i].value)
				return CFG_ERROR_ALLOC;
			strncpy(st->entry[i].value, value, len);
			st->entry[i].value[len] = '\0';
			return CFG_ERROR_OK;
		}
		i++;
	}
	return CFG_ERROR_KEY_NOT_FOUND;
}

static cfg_error_t cfg_free_memory(cfg_t *st)
{
	cfg_int i = 0;

	if (!st->entry && st->nkeys)
		return CFG_ERROR_CRITICAL;
	if (!st->nkeys) // nothing to do here
		return CFG_ERROR_OK;

	while (i < st->nkeys) {
		if (!st->entry[i].key) {
			return CFG_ERROR_NULL_KEY;
		} else {
			free(st->entry[i].key);
			st->entry[i].key = NULL;
		}
		if (st->entry[i].value) {
			free(st->entry[i].value);
			st->entry[i].value = NULL;
		}
		i++;
	}
	free(st->entry);
	st->entry = NULL;
	if (st->cache_keys_index) {
		free(st->cache_keys_index);
		st->cache_keys_index = NULL;
	}
	if (st->cache_keys_hash) {
		free(st->cache_keys_hash);
		st->cache_keys_hash = NULL;
	}
	st->nkeys = 0;
	return CFG_ERROR_OK;
}

cfg_error_t cfg_free(cfg_t *st)
{
	cfg_int ret;

	if (st->init != CFG_TRUE)
		return CFG_ERROR_INIT;
	if (st->nkeys) {
		ret = cfg_free_memory(st);
		if (ret > 0)
			return ret;

		st->buf = NULL;
		st->buf_size = 0;

		if (st->file)
			fclose(st->file);
		st->file = NULL;
	}
	memset((void *)st, 0, sizeof(st));
	st->init = CFG_FALSE; // not needed if CFG_FALSE is zero
	return CFG_ERROR_OK;
}

// a couple of helper macros
#define cfg_check_endline_multiline(bp, buf) \
	*bp == '\n' && (bp > buf && *(bp - 1) != '\\')

#define cfg_check_line_equalsign(bp, ec, ls) \
	*bp = '\0'; \
	ec = strchr(ls, '='); \
	*bp = '\n'; \
	if (bp - ls == 0 || *ls == '#' || !ec) { \
		ls = bp + 1; \
		bp++; \
		continue; \
	}

cfg_error_t cfg_parse_buffer(cfg_t *st, cfg_char *buf, cfg_int sz)
{
	cfg_int n, ret, nkeys;
	cfg_char *bp, *bend, *ls, *ec;

	if (st->init != CFG_TRUE)
		return CFG_ERROR_INIT;

	// set buffer
	st->buf = buf;
	st->buf_size = sz;

	// clear old keys
	ret = cfg_free_memory(st);
	if (ret > 0)
		return ret;
	ret = cfg_cache_size_set(st, st->cache_size);
	if (ret > 0)
		return ret;
	cfg_cache_clear(st);
	if (ret > 0)
		return ret;

	// find the number of keys by going trough the entire buffer
        // initially. we use this method instead of a growing list,
        // since realloc can end up being slow. even if our estimate
        // is off a little realloc will have to copy all pointer
        // values for the buffer to grow

	ls = bp = st->buf;
	bend = bp + sz;
	n = nkeys = 0;
	while (bp < bend) {
		// check if this is a multi-line definition
		if (cfg_check_endline_multiline(bp, st->buf)) {
			cfg_check_line_equalsign(bp, ec, ls);
			nkeys++;
			ls = bp + 1;
		}
		bp++;
	}

	// allocate memory for the list
	st->entry = (cfg_entry_t *)malloc(nkeys * sizeof(cfg_entry_t));
	if (!st->entry)
		return CFG_ERROR_ALLOC;

	// fill keys and values
	ls = bp = st->buf;
	bend = bp + sz;
	n = 0;
	while (bp < bend) {
		// check if this is a multi-line definition
		if (cfg_check_endline_multiline(bp, st->buf)) {
			cfg_check_line_equalsign(bp, ec, ls);
			/* fill key */
			sz = ec - ls + 1;
			st->entry[n].key = (cfg_char *)malloc(sz * sizeof(cfg_char));
			if (!st->entry[n].key)
				return CFG_ERROR_ALLOC;
			ls = trimwhitespace(ls);
			strncpy(st->entry[n].key, ls, sz);
			st->entry[n].key[sz - 1] = '\0';
			st->entry[n].key_hash = cfg_hash_get(st->entry[n].key);

			// fill key value
			sz = bp - ec;
			if (sz - 1 == 0) { // empty value
				st->entry[n].value = NULL;
				st->entry[n].value_hash = cfg_hash_get(NULL);
				ls = bp + 1;
				bp++;
				n++;
				continue;
			}
			st->entry[n].value = (cfg_char *)malloc(sz * sizeof(cfg_char));
			if (!st->entry[n].value)
				return CFG_ERROR_ALLOC;
			strncpy(st->entry[n].value, ec + 1, sz);
			st->entry[n].value[sz - 1] = '\0';
			st->entry[n].value_hash = cfg_hash_get(cfg_value_escape(st->entry[n].value));
			ls = bp + 1;

			// Additional trim of whitespace on current key/value pair
			st->entry[n].value = trimwhitespace(st->entry[n].value);
			st->entry[n].key   = trimwhitespace(st->entry[n].key);

			n++;
		}
		bp++;
	}
	st->nkeys = nkeys;
	return CFG_ERROR_OK;
}

cfg_error_t cfg_parse_file(cfg_t *st, cfg_char *filename)
{
	FILE *f;
	cfg_char *buf;
	cfg_int sz = 0, ret;

	if (st->init != CFG_TRUE)
		return CFG_ERROR_INIT;

	// read file
	f = fopen(filename, "r");
	if (!f)
		return CFG_ERROR_FOPEN;
	st->file = f;

	// get file size
	while (fgetc(f) != EOF)
		sz++;
	rewind(f);

	buf = (cfg_char *)malloc(sz * sizeof(cfg_char));
	if (!buf) {
		fclose(f);
		st->file = NULL;
		return CFG_ERROR_ALLOC;
	}
	if (fread(buf, sizeof(cfg_char), sz, f) != (cfg_uint32)sz) {
		fclose(f);
		return CFG_ERROR_FREAD;
	}
	fclose(f);
	st->file = NULL;

	ret = cfg_parse_buffer(st, buf, sz);
	free(st->buf);
	st->buf = NULL;
	return ret;
}
