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
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
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

int PathIsAbsolute(const char *path) {
    if (strlen(path) > 1 && path[0] == '/') {
        return 1;
    }

#if defined(__WIN32) || defined(_WIN32)
    if (strlen(path) > 2) {
        if (isalpha(path[0]) && path[1] == ':') {
            return 1;
        }
    }
#endif

    return 0;
}

/**
 *  \brief Check if a path is relative
 *
 *  \param path string with the path
 *
 *  \retval 1 relative
 *  \retval 0 not relative
 */
int PathIsRelative(const char *path) {
    return PathIsAbsolute(path) ? 0 : 1;
}

static int one_dir_make(char *path, mode_t mode)
{
    if (mkdir(path, mode) != 0) {
        return -1;
    }

    chmod(path, mode);

    return 0;
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

int recursive_make_dir(char *path, mode_t mode)
{
    if (path == NULL || strlen(path) == 0) {
        return -1;
    } 

    struct stat info;
    if (stat(path, &info) == 0) {
        //directory exist
        if (info.st_mode & S_IFDIR) {
            return 0;
        }
        //same name file exist
        return -1;
    }

    //try create directory
    if (one_dir_make(path, mode) == 0) {
        return 0;
    }

    //get pre path
    char predir[2048];
    if (strlen(path) > sizeof(predir)-1) {
        return -1;
    }
    strncpy(predir, path, sizeof(predir));
    get_pre_path(predir);
    //create pre path
    if (recursive_make_dir(predir, mode) != 0) {
        return -1;
    } 

    //create current path again
    int ret = 0;
    ret = one_dir_make(path, mode);

    return ret;
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

int move_file(char* origFile, char* destFile)
{
    int sd = open(origFile, O_RDONLY);
    if(sd == -1)
    {
        return -1;
    }

    struct stat st;
    if(fstat(sd, &st) != 0)
    {
        close(sd);
        return -1;
    }

    const long blocksize = st.st_blksize;
    //int dd = open(destFile, O_CREAT|O_TRUNC|O_WRONLY, st.st_mode & S_IRWXU);
    int dd = open(destFile, O_CREAT|O_TRUNC|O_WRONLY, st.st_mode);
    if(dd == -1)
    {
        close(sd);
        return -1;
    }

    char buffer[blocksize];
    memset(buffer, 0, sizeof(buffer));
    int n;
    while((n = read(sd, buffer, blocksize)) > 0)
    {
        if(write(dd, buffer, n) != n)
        {
            close(sd);
            close(dd);
            return -1;
        }
    }

    close(sd);
    close(dd);

    if(n < 0)
    {
        return -1;
    }

    fsync(dd);
    remove(origFile);
    
    return 0;
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

