/*-
 * This file is in the public domain.
 * Do with it as you will.
 */
//  gcc minitar.c -larchive
//  ./a.out tar test.c test.tar
#include <sys/types.h>
#include <sys/stat.h>

#include "../Archive/archive.h"
#include "../Archive/archive_entry.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>





static int
do_compress_archive(int compress, char *buff, int buff_size,const char *dfilename, const char *sfilename);

#define NEWIUP_OK 0
#define NEWIUP_FAIL -1


enum compress_type_e
{
	UPLOAD_COMPRESS_GZ,
	UPLOAD_COMPRESS_BZ,
	UPLOAD_COMPRESS_BZ2,
	UPLOAD_COMPRESS_Z,
	UPLOAD_COMPRESS_ZIP,
	UPLOAD_COMPRESS_7Z,
	UPLOAD_COMPRESS_TAR,
	UPLOAD_COMPRESS_TAR_GZ,
	UPLOAD_COMPRESS_TAR_BZ,
	UPLOAD_COMPRESS_TAR_BZ2,
	UPLOAD_COMPRESS_TAR_Z,
	UPLOAD_COMPRESS_MAX
};

static char *g_gzip_type[UPLOAD_COMPRESS_MAX]={
	"gz",
	"bz",
	"bz2",
	"Z",
	"zip",
	"7z",
	"tar",
	"tar.gz",
	"tar.bz",
	"tar.bz2",
	"tar.Z",
};

static int get_compress_type_id(char *str)
{
    if (str == NULL)
    {
        return -1;
    }
	int i;
	for(i=0; i<UPLOAD_COMPRESS_MAX; i++){
		printf("  %s %d %s\n", g_gzip_type[i],strlen(str), str);
	    if (0==strcmp(str, g_gzip_type[i]))
	    {
	        return i; 
	    }
	}

    return -1;
}
static char buff[16384];

int
main(int argc, const char **argv)
{
	int compress;

	if(argc != 4){
		printf("usage: type src dest\n");
		return 0;
	}
	compress = get_compress_type_id(argv[1]);
	printf("%d %s %s\n", compress, g_gzip_type[compress], argv[1]);
	if(compress == -1){
		printf("type error\n");
		return 0;
	}

	do_compress_archive(compress, buff, sizeof(buff),argv[argc-1], argv[argc-2]);

	return (0);
}


static int
do_compress_archive(int compress, char *buff, int buff_size,const char *dfilename, const char *sfilename)
{
	struct archive *a;
	struct archive *disk;
	struct archive_entry *entry;
	ssize_t len;
	int fd;

	a = archive_write_new();
	switch (compress) {
	case UPLOAD_COMPRESS_GZ:
	case UPLOAD_COMPRESS_TAR_GZ:
		archive_write_add_filter_gzip(a);
		archive_write_set_format_ustar(a);
		break;
	case UPLOAD_COMPRESS_BZ:
	case UPLOAD_COMPRESS_TAR_BZ:
	case UPLOAD_COMPRESS_BZ2:
	case UPLOAD_COMPRESS_TAR_BZ2:
		archive_write_add_filter_bzip2(a);
		archive_write_set_format_ustar(a);
		break;
	case UPLOAD_COMPRESS_Z:
	case UPLOAD_COMPRESS_TAR_Z:
		archive_write_add_filter_compress(a);
		archive_write_set_format_ustar(a);
		break;
	case UPLOAD_COMPRESS_ZIP:
		archive_write_add_filter_lzip(a);
		archive_write_set_format_zip(a);
		break;
	case UPLOAD_COMPRESS_7Z:
		archive_write_add_filter_lzip(a);
		archive_write_set_format_7zip(a);
		break;
	case UPLOAD_COMPRESS_TAR:
		archive_write_set_format_ustar(a);
		break;
	default:
		archive_write_add_filter_none(a);
		break;
	}

	archive_write_open_filename(a, dfilename);

		struct archive *disk2 = archive_read_disk_new();
		int r;

		r = archive_read_disk_open(disk2, sfilename);
		if (r != ARCHIVE_OK) {
			printf("do1 %s\n",archive_error_string(disk2));
			return NEWIUP_FAIL;
		}

		for (;;) {

			entry = archive_entry_new();
			r = archive_read_next_header2(disk2, entry);
			if (r == ARCHIVE_EOF){
				printf("read eof\n");
				break;
			}
			if (r != ARCHIVE_OK) {
				printf("do2 %s\n",archive_error_string(disk2));
				return NEWIUP_FAIL;
			}
			archive_read_disk_descend(disk2);

			r = archive_write_header(a, entry);
			if (r < ARCHIVE_OK) {
				printf("%s\n",archive_error_string(disk2));
			}
			if (r == ARCHIVE_FATAL){
				printf("%s\n",archive_error_string(disk2));
				return NEWIUP_FAIL;
			}
			if (r > ARCHIVE_FAILED) {
				/* For now, we use a simpler loop to copy data
				 * into the target archive. */
				fd = open(archive_entry_sourcepath(entry), O_RDONLY);
				len = read(fd, buff, buff_size);
				while (len > 0) {
					archive_write_data(a, buff, len);
					len = read(fd, buff, buff_size);
				}
				close(fd);
			}
			archive_entry_free(entry);
		}
		archive_read_close(disk2);
		archive_read_free(disk2);

	archive_write_close(a);
	archive_write_free(a);
	return NEWIUP_OK;
}
