#ifndef _EZC_CONF_H
#define _EZC_CONF_H

#include "define.h"
#include "imports.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EZC_ROWS 12

#define EZC_KEY_LIM 32
#define EZC_VAL_LIM 128

#define EZC_LINE_LIM 256


struct ezc_conf_ent;
struct ezc_conf_ent {
	s8                   key[EZC_KEY_LIM];
	s8          	       val[EZC_VAL_LIM];

	struct ezc_conf_ent    *next;
};

struct ezc_conf_hdl {
	struct ezc_conf_ent *tbl[EZC_ROWS];
	s32                 num;
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
EZC_API s8 ezc_parse(s8 *pth);


/*
 * Update the value of a key in the hashtbl.
 *
 * @key: The keyword to search for
 * @val: The new value
 *
 * Returns: 0 on success or -1 if an error occurred
 */
EZC_API s8 ezc_set(s8 *key, s8 *val); 


/*
 * Get the value of a keyword.
 *
 * @key: The nullterminated buffer containing the key
 *
 * Returns: A nullterminated string containing the value
 */
EZC_API s8 *ezc_get(s8 *key);


/*
 * Dump the whole easy config table in the console.
 */
EZC_API void ezc_dump(void);



#endif /* _EZ_CONF_H */
