/*
 * util-strex.c
 *
 *  Created on: 2012-7-22
 *      Author: colin
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
//#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
//#include <signal.h>
#include "utils.h"

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz)
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0 && --n != 0) {
        do {
            if ((*d++ = *s++) == 0)
                break;
        } while (--n != 0);
    }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0) {
        if (siz != 0)
            *d = '\0'; /* NUL-terminate dst */
        while (*s++)
            ;
    }

    return(s - src - 1); /* count does not include NUL */
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(initial dst) + strlen(src); if retval >= siz,
 * truncation occurred.
 */
size_t strlcat(char *dst, const char *src,  size_t siz)
{
    register char *d = dst;
    register const char *s = src;
    register size_t n = siz;
    size_t dlen;

    /* Find the end of dst and adjust bytes left but don't go past end */
    while (n-- != 0 && *d != '\0')
        d++;
    dlen = d - dst;
    n = siz - dlen;

    if (n == 0)
        return(dlen + strlen(s));
    while (*s != '\0') {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return(dlen + (s - src)); /* count does not include NUL */
}



static int get_pre_path(char *path)
{
    int len = strlen(path);

    while (len > 0 && path[len-1] == '/') {
        path[len-1] = '\0'; 
        len--;
    }

    while (len > 0 && path[len-1] != '/') {
        path[len-1] = '\0';
        len--;
    }    

    while (len > 0 && path[len-1] == '/') {
        path[len-1] = '\0'; 
        len--;
    }

    return 0;
}


//is it a file 
//return 0:yes  -1:no
int is_file(char *filepath) 
{
    struct stat st;
    if (filepath != NULL && stat(filepath, &st) == 0)
    {
        if (S_ISREG(st.st_mode))
        {
            return 0;
        }
    }

    return -1; 
}

//is it a directroy
//return 0:yes -1:no 
int is_dir(char *filepath) 
{
    struct stat st;
    if (stat(filepath, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            return 0;
        }
    }

    return -1; 
}
int file_exists_state(char *filename)
{
	//exists 0: is not exits -1
	return access(filename, 0);
}
int get_file_size(const char* filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
    {
        return st.st_size;
    }

    return 0;
}

//create sign file for mark file need to upload
//example: xxxxxxxx.ctr.group1-srv1
int create_file( char *filepath)
{
    FILE *fd = fopen(filepath, "w");
    if (fd == NULL)
    {
    	return -1;
    }
    fclose(fd);

    return 0;
}

int delete_file( char *path)
{
	remove(path);
    return 0;
}

unsigned str_hash(const char * key)
{
    int         len ;
    unsigned    hash ;
    int         i ;

    len = strlen(key);
    for (hash=0, i=0 ; i<len ; i++) {
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
    return hash ;
}

int str_split_func(struct str_split *split, char * src, char delimiter)
{
	int count = 0;
	char *pchar, **ptr;

	if ( NULL != split ) {
		memset(split, 0, sizeof(struct str_split));
	}
    
    if(NULL == split || NULL == src || src[0] == '\0')
    {
        return 0;
    }
    
    split->str = strdup(src);
    if(NULL == split->str)
    {
        return 0;
    }
    count = 1;
    pchar = src;
    while('\0' != *pchar)
    {
        if (delimiter == *pchar)
        {
            count++;
        }
        pchar++;
    }
    split->str_array = (char **)malloc(count*sizeof(char*));
    if(NULL == split->str_array)
    {
        return 0;
    }
    split->count = count;
    
    ptr = split->str_array;
    *ptr = split->str;
    pchar = split->str;
    while('\0' != *pchar && count > 1)
    {
        if (delimiter == *pchar)
        {
            ptr++;
            *ptr = pchar+1;
            *pchar = '\0';
            count--;
        }
        pchar++;
    }
	return 0;
}
int str_split_free(struct str_split *split)
{
	if(split == NULL)
	{
		return 0;
	}
	if(split->str!=NULL)
	{
		free(split->str);
		split->str=NULL;
	}
	if(split->str_array != NULL)
	{
		free(split->str_array);
		split->str_array=NULL;
	}
	return 0;
}

