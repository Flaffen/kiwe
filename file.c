#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "file.h"

struct file_data *load_file(char *path)
{
	char *buffer, *p;
	struct stat buf;
	
	stat(path, &buf);

	FILE *fp = fopen(path, "rb");

	if (fp == NULL)
		return NULL;

	buffer = malloc(buf.st_size);

	fread(buffer, 1, buf.st_size, fp);

	struct file_data *fdata = malloc(sizeof(*fdata));
	fdata->data = buffer;
	fdata->len = buf.st_size;

	fclose(fp);

	return fdata;
}
