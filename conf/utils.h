/*
 * UTILS.h
 *
 *  Created on: 2012-9-12
 *      Author: liuzc
 */

#ifndef __UTILS_H_
#define __UTILS_H_

#include <stddef.h>

/*c hearders*/
#ifdef __cplusplus
extern "C" {
#endif

size_t strlcpy(char *dst, const char *src, size_t siz);
size_t strlcat(char *dst, const char *src,  size_t siz);
//directory mode, tobe 07xx
#define DIR_MODE 0755

struct str_split
{
	int count;
	char *str;
	char **str_array;
};

int recursive_make_dir(char *path, mode_t mode);
int is_file(char *filepath);
int is_dir(char *filepath);
int file_exists_state(char *filename);
int get_file_size(const char* filename);
int create_file( char *filepath);
int delete_file( char *path);
int move_file(char* origFile, char* destFile);
int str_split_func(struct str_split *split, char * src, char delimiter);
int str_split_free(struct str_split *split);


#ifdef __cplusplus
}
#endif

#endif /* __UTILS_H_ */
