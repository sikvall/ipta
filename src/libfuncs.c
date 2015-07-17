#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Trim white space from the beginning and the end of a string by
 * moving pointer and null termination. String may be shorter but the
 * stard address should remain the same so free() should not barf. */
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


/* Convert a given string to upper case and then return a pointer to
 * the same string. Original string will be overwritten and lost. */
char *strupr(char *s)
{
	char *p = s;
	while(*s) {
		*s = toupper(*s);
		s++;
	}
	return p;
}

/* Convert a given string to lower case and then return a pointer to
 * the same string. Original string will be overwritten and lost. */
char *strlwr(char *s)
{
	char *p = s;
	while(*s) {
		*s = tolower(*s);
		s++;
	}
	return p;
}
