/*
 * wur@guardian
 * date:09.10. 2019
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "files.h"

static int fd = -1;
static char *fname = NULL;
static int Version = 1;


int
file_open(char *name)
{
	if (fd == -1) close(fd);
	fd = open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) return -1;
	return 0;
}


int
file_open_tr(char *name)
{
	if (fd == -1) close(fd);
	fd = open(name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) return -1;
	return 0;
}


struct var_sheet *
file_read(char *name)
{
	if (file_open(name) < 0) return NULL;
	fname = name;
	int magic = 0x0e11;
	struct var_sheet *s;
	size_t rows;
	int version;
	if (fd == -1) return NULL;
	if (read(fd, &magic, sizeof(int)) != sizeof(int)) return NULL;
	if (magic != 0x0e11) return NULL;
	if (read(fd, &version, sizeof(int)) != sizeof(int)) return NULL;
	if (version != Version) return NULL;
	if ((s = malloc(sizeof(struct var_sheet))) == NULL) return NULL;
	s->rows = 0;
	if (read(fd, &(s->cols), sizeof(size_t)) != sizeof(size_t)) return NULL;
	if (read(fd, &(rows), sizeof(size_t)) != sizeof(size_t)) return NULL;
	if ((s = vsheet_add_rows(s, rows)) == NULL) return NULL;
	if (read(fd, &(s->vals), sizeof(box_sz) * s->cols * s->rows) !=
		                 sizeof(box_sz) * s->cols * s->rows) return NULL;
	return s;
}


int
file_write(struct var_sheet *s)
{
	fd = -1;
	int magic = 0x0e11;
	if (fname != NULL) {
		if (file_open_tr(fname) < 0) return -1;
	} else {
		if (file_open_tr("out.isc") < 0) return -1;
	};
	if (fd == -1) return -1;
	if (write(fd, &magic, sizeof(int)) != sizeof(int)) return -1;
	if (write(fd, &Version, sizeof(int)) != sizeof(int)) return -1;
	if (write(fd, &(s->cols), sizeof(size_t)) != sizeof(size_t)) return -1;
	if (write(fd, &(s->rows), sizeof(size_t)) != sizeof(size_t)) return -1;
	if (write(fd, &(s->vals), sizeof(box_sz) * s->cols * s->rows) !=
		                  sizeof(box_sz) * s->cols * s->rows) return -1;
	return 0;
}
