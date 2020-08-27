#include <stdio.h>
#include "utils.h"

void hexdump(void *data, size_t len)
{
	FILE *fp = fopen("hexdump.bin", "w");
	fwrite(data, 1, len, fp);
	fclose(fp);
}
