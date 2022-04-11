#ifndef _EZC_CONF_H
#define _EZC_CONF_H

#define EZC_API                 extern
#define EZC_INTERN              static

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define EZC_ROWS 12

#define EZC_KEY_LIM 32
#define EZC_VAL_LIM 128

#define EZC_LINE_LIM 256


struct ezc_conf_ent {
	char                   key[EZC_KEY_LIM];
	char          	       val[EZC_VAL_LIM];

	struct ezc_conf_ent    *next;
};

struct ezc_conf_hdl {
	struct ezc_conf_ent *tbl[EZC_ROWS];
	int                 num;
};


/*
 * Reset the global config table and free the allocated memory.
 */
EZC_API void ezc_reset(void);


/*
 * Read a file and parse the config table into the hashtbl.
 *
 * @pth: A nullterminated buffer containing the path to the config file
 *
 * Returns: 0 on success or -1 if an error occurred
 */
EZC_API int ezc_parse(char *pth);


/*
 * Update the value of a key in the hashtbl.
 *
 * @key: The keyword to search for
 * @val: The new value
 *
 * Returns: 0 on success or -1 if an error occurred
 */
EZC_API int ezc_set(char *key, char *val); 


/*
 * Get the value of a keyword.
 *
 * @key: The nullterminated buffer containing the key
 *
 * Returns: A nullterminated string containing the value
 */
EZC_API char *ezc_get(char *key);


/*
 * Dump the whole easy config table in the console.
 */
EZC_API void ezc_dump(void);


#endif /* _EZC_CONF_H */
#ifdef EZC_DEF

struct ezc_conf_hdl g_ezc_hdl;


EZC_API void ezc_reset(void)
{
	int i;
	struct ezc_conf_ent *ptr;
	struct ezc_conf_ent *next;


	/*
	 * Free all rows in the hashtbl.
	 */
	for(i = 0; i < EZC_ROWS; i++) {
		ptr = g_ezc_hdl.tbl[i];
		while(ptr) {
			next = ptr->next;
			free(ptr);
			ptr = next;
		}

		g_ezc_hdl.tbl[i] = NULL;
	}

	g_ezc_hdl.num = 0;
}


EZC_INTERN unsigned long ezc_hash(char *str)
{
	unsigned long hash = 5381;
	int c;

	while((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}


EZC_INTERN int ezc_check(char c)
{
	if(isspace(c))
		return 1;

	if(isdigit(c))
		return 2;

	if(isupper(c))
		return 3;

	if(islower(c))
		return 4;

	if(c == '=')
		return 5;

	if(c == '#')
		return 6;

	if(ispunct(c))
		return 7;

	return 0;
}


EZC_API int ezc_parse(char *pth)
{
	FILE *fd;
	char line[EZC_LINE_LIM];
	int line_c = 0;

	int low_lim_key;
	int high_lim_key;
	int low_lim_val;
	int high_lim_val;
	int i;
	int ret;
	int phs;
	int flg;

	char key[EZC_KEY_LIM];
	char val[EZC_VAL_LIM];

	if(!pth)
		return -1;

	/*
	 * Open the file.
	 */
	if(!(fd = fopen(pth, "r"))) {
		fprintf(stderr, "Failed to open config %s\n", pth);
		return -1;
	}


	while(fgets(line, EZC_LINE_LIM, fd)) {
		/*
		 * Adjust null-terminator.
		 */
		int len = strlen(line);

		line_c++;

		low_lim_key = -1;
		high_lim_key = -1;
		low_lim_val = -1;
		high_lim_val = -1;

		phs = 0;
		flg = 0;

		for(i = 0; i < len; i++) {
			/*
			 * Read the digit and check its type.
			 * Quit if the digit is invalid.
			 */
			if((ret = ezc_check(line[i])) == 0) {
				break;
			}

			/* Break if the digit is a hashtag */
			if(ret == 6 || flg == 2)
				break;


			switch(phs) {
				case 0:
					/*
					 * Keyword not found
					 *
					 * Look for a letter.
					 */
				

					if(ret == 3 || ret == 4) {
						low_lim_key = i;
						phs = 1;
					}

					break;

				case 1:
					/*
					 * Read keyword
					 *
					 * Wait for a space or an equal sign.
					 */

					if(ret == 1) {
						high_lim_key = i - 1;
						phs = 2;
					}
					else if(ret == 5) {
						high_lim_key = i - 1;
						phs = 3;
					}

					break;

				case 2:
					/*
					 * Equal sign not found
					 *
					 * Loook for an equal sign.
					 */
		
					if(ret == 5) {
						phs = 3;
					}

					break;

				case 3:
					/*
					 * Value not found
					 *
					 * Look for anything valid.
					 */

					if(ret != 1) {
						low_lim_val = i;
						phs = 4;
						flg = 1;
					}

					break;

				case 4:
					/*
					 * Read value
					 */
				

					if(ret == 1) {
						high_lim_val = i - 1;
						flg = 2;
					}
					break;
			}
		}

		if(flg == 1) {
			high_lim_val = i - 1;
		}

		if(low_lim_key < 0 || high_lim_key < 0)
			continue;

		if(low_lim_val < 0 || high_lim_val < 0)
			continue;


		len = (high_lim_key-low_lim_key) + 1;
		memcpy(key, line + low_lim_key, len);
		key[len] = '\0';

		len = (high_lim_val-low_lim_val) + 1;
		memcpy(val, line + low_lim_val, len);
		val[len] = '\0';

		if(ezc_set(key, val) < 0)
			goto err_close;
	}

	/*
	 * Close the file.
	 */
	fclose(fd);

	return 0;

err_close:
	fclose(fd);

	return -1;
}


EZC_API int ezc_set(char *key, char *val)
{
	unsigned long hash;
	unsigned char row;
	struct ezc_conf_ent *ptr;
	struct ezc_conf_ent *ent;

	hash = ezc_hash(key);
	row = hash % EZC_ROWS;

	if(g_ezc_hdl.tbl[row] == NULL) {	
		if(!(ent = malloc(sizeof(struct ezc_conf_ent))))
			return -1;

		strcpy(ent->key, key);
		strcpy(ent->val, val);
		ent->next = NULL;
		
		g_ezc_hdl.tbl[row] = ent;
	}
	else {
		ptr = g_ezc_hdl.tbl[row];
		while(ptr->next) {
			/* Overwrite value */
			if(strcmp(ptr->key, key) == 0) {
				strcpy(ptr->val, val);
				return 0;
			}

			ptr = ptr->next;
		}

		if(!(ent = malloc(sizeof(struct ezc_conf_ent))))
			return -1;

		strcpy(ent->key, key);
		strcpy(ent->val, val);
		ent->next = NULL;

		ptr->next = ent;
	}

	g_ezc_hdl.num++;

	return 0;
}


EZC_API char *ezc_get(char *key)
{
	unsigned long hash;
	unsigned char row;
	struct ezc_conf_ent *ptr;

	hash = ezc_hash(key);
	row = hash % EZC_ROWS;

	ptr = g_ezc_hdl.tbl[row];
	while(ptr) {
		if(strcmp(ptr->key, key) == 0) {
			return ptr->val;
		}

		ptr = ptr->next;
	}

	return NULL;
}



EZC_API void ezc_dump(void)
{
	int i;
	struct ezc_conf_ent *ptr;

	for(i = 0; i < EZC_ROWS; i++) {
		printf(">> %d:\n", i);

		ptr = g_ezc_hdl.tbl[i];
		while(ptr) {
			printf("%s: %s\n", ptr->key, ptr->val);
			ptr = ptr->next;
		}

	}
}

#endif
