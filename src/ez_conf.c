#include "ez_conf.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct ezc_conf_hdl g_ezc_hdl;


EZC_API void ezc_reset(void)
{
	s32 i;
	struct ezc_conf_ent *ptr;
	struct ezc_conf_ent *next;


	/*
	 * Free all rows in the hashtbl.
	 */
	for(i = 0; i < EZC_ROWS; i++) {
		ptr = g_ezc_hdl.tbl[i];
		while(ptr) {
			next = ptr->next;
			sfree(ptr);
			ptr = next;
		}

		g_ezc_hdl.tbl[i] = NULL;
	}

	g_ezc_hdl.num = 0;
}


EZC_INTERN u64 ezc_hash(s8 *str)
{
	u64 hash = 5381;
	s32 c;

	while((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}


EZC_INTERN u8 ezc_check(s8 c)
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


EZC_API s8 ezc_parse(s8 *pth)
{
	FILE *fd;
	s8 line[EZC_LINE_LIM];
	s32 line_c = 0;

	s32 low_lim_key;
	s32 high_lim_key;
	s32 low_lim_val;
	s32 high_lim_val;
	s32 i;
	s32 ret;
	s32 phs;
	s32 flg;

	s8 key[EZC_KEY_LIM];
	s8 val[EZC_VAL_LIM];

	s32 len;

	if(!pth) {
		ALARM(ALARM_WARN, "pth undefined");
		return -1;
	}

	/*
	 * Open the file.
	 */
	if(!(fd = fopen((char *)pth, "r"))) {
		ALARM(ALARM_ERR, "Failed to open the file");
		goto err_return;
	}


	while(fgets((char *)line, EZC_LINE_LIM, fd)) {
		/*
		 * Adjust null-terminator.
		 */
		if((len = tozero(line)) < 0) {
			goto err_return;
		}

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


err_return:
	ALARM(ALARM_ERR, "Failed to parse config file");
	return -1;
}


EZC_API s8 ezc_set(s8 *key, s8 *val)
{
	u64 hash;
	u8 row;
	struct ezc_conf_ent *ptr;
	struct ezc_conf_ent *ent;

	hash = ezc_hash(key);
	row = hash % EZC_ROWS;

	if(g_ezc_hdl.tbl[row] == NULL) {	
		if(!(ent = smalloc(sizeof(struct ezc_conf_ent))))
			return -1;

		cpytozero(ent->key, key);
		cpytozero(ent->val, val);
		ent->next = NULL;
		
		g_ezc_hdl.tbl[row] = ent;
	}
	else {
		ptr = g_ezc_hdl.tbl[row];
		while(ptr->next) {
			/* Overwrite value */
			if(cmptozero(ptr->key, key) == 0) {
				cpytozero(ptr->val, val);
				return 0;
			}

			ptr = ptr->next;
		}

		if(!(ent = smalloc(sizeof(struct ezc_conf_ent))))
			return -1;

		cpytozero(ent->key, key);
		cpytozero(ent->val, val);
		ent->next = NULL;

		ptr->next = ent;
	}

	g_ezc_hdl.num++;

	return 0;
}


EZC_API s8 *ezc_get(s8 *key)
{
	u64 hash;
	u8 row;
	struct ezc_conf_ent *ptr;

	hash = ezc_hash(key);
	row = hash % EZC_ROWS;

	ptr = g_ezc_hdl.tbl[row];
	while(ptr) {
		if(cmptozero(ptr->key, key) == 0) {
			return ptr->val;
		}

		ptr = ptr->next;
	}

	return NULL;
}



EZC_API void ezc_dump(void)
{
	s32 i;
	struct ezc_conf_ent *ptr;

	for(i = 0; i < EZC_ROWS; i++) {
		printf(">> %d:\n", i);

		ptr = g_ezc_hdl.tbl[i];
		while(ptr) {
			printf("%s: %s\n", (char *)ptr->key, (char *)ptr->val);
			ptr = ptr->next;
		}

	}
}
