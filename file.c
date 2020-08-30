#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "file.h"

struct file_data *load_file(char *path)
{
	char *buffer;

	FILE *fp = fopen(path, "r");

	if (fp == NULL) {
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	size_t bufsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buffer = malloc(bufsize);

	fread(buffer, 1, bufsize, fp);

	struct file_data *fdata = malloc(sizeof(struct file_data));
	fdata->data = buffer;
	fdata->len = bufsize;

	return fdata;
}

struct file_data *load_file2(char *path)
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
