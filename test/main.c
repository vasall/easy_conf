#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EZC_DEF
#include "../ez_conf.h"

int main(void)
{
	printf("Init EZC...");
	ezc_reset();
	printf("done\n");

	printf("Read file...");
	if(ezc_parse("hello.txt") < 0) {
		printf("failed\n");
		goto err_close;
	}
	printf("done\n");

	printf("Dump...\n");
	ezc_dump();
	printf("-----\n");


	printf("Get ara\n");
	printf("ara: %s\n", ezc_get("ara\0"));


	ezc_set("ara\0", "heowoo\0");
	ezc_set("awoo\0", "1267.009\0");

	printf("Dump...\n");
	ezc_dump();
	printf("-----\n");

err_close:
	ezc_reset();
	return 0;
}
