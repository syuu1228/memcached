/* 
 * MMCSTORAGE - Dynamic Database Linker
 *
 * AUTHOR
 *   Toru Maesaka <tmaesaka@users.sourceforge.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "mmcstorage.h"

MMCSTORAGE *mmcstorage_new(void) {
	MMCSTORAGE *storage; 

	storage = malloc(sizeof(*storage));
	
	return storage;
}

void mmcstorage_destruct(MMCSTORAGE *stor) {
	free(stor);
}

int mmcstorage_open(MMCSTORAGE *stor, const char *cmd) {
	return chdir(cmd);
}

int mmcstorage_close(MMCSTORAGE *stor) {
	return 0;
}

int mmcstorage_conf(MMCSTORAGE *stor, const int num, const char *opts) {
	return 0;
}

void *mmcstorage_alloc(MMCSTORAGE *stor, const void *kbuf, const int ksiz, 
					   const int vsiz)
{
	char _kbuf[ksiz + 1];
	int fd;
	char *obj;

	memcpy(_kbuf, kbuf, ksiz);
	_kbuf[ksiz] = 0;
	
	fd = open(_kbuf, O_RDWR | O_CREAT, 644);
	if (fd < 0) {
		return NULL;
	}

	if (ftruncate(fd, vsiz)) {
		perror("ftruncate");
		close(fd);
		return NULL;
	}
	
	obj = mmap(NULL, vsiz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (obj == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return NULL;
	}

	close(fd);

	return obj;
}

int mmcstorage_put(MMCSTORAGE *stor, const void *kbuf, const int ksiz,
                   const void *vbuf, const int vsiz) {
	return 0;
}

void *mmcstorage_get(MMCSTORAGE *stor, const void *kbuf, const int ksiz,
                     int *vsizp) {
	char _kbuf[ksiz + 1];
	int fd;
	char *obj;
	struct stat stat;

	memcpy(_kbuf, kbuf, ksiz);
	_kbuf[ksiz] = 0;
	
	fd = open(_kbuf, O_RDONLY);
	if (fd < 0) {
		return NULL;
	}

	if (fstat(fd, &stat)) {
		perror("fstat");
		return NULL;
	}

	*vsizp = stat.st_size;

	obj = mmap(NULL, *vsizp, PROT_READ, MAP_SHARED, fd, 0);
	if (obj == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}

	close(fd);
	
	return obj;

}

int mmcstorage_free(MMCSTORAGE *stor, const void *vbuf, const int vsiz) {
	return munmap((void *)vbuf, (size_t)vsiz);
}


int mmcstorage_del(MMCSTORAGE *stor, const void *kbuf, const int ksiz) {
	char _kbuf[ksiz + 1];

	memcpy(_kbuf, kbuf, ksiz);
	_kbuf[ksiz] = 0;
	fprintf(stderr, "_kbuf(%s)\n", _kbuf);
	
	return unlink(_kbuf);
}

int mmcstorage_flush(MMCSTORAGE *stor, const char *opts) {
	return 0;
}

