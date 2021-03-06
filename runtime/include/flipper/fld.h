#ifndef __fld_h__
#define __fld_h__

/* Include all types and macros exposed by the Flipper Toolbox. */
#include <flipper/libflipper.h>

#ifdef __private_include__

/* Declare the virtual interface for this module. */
extern const struct _fld {
	int (* configure)(void);
	int (* index)(lf_crc_t identifier);
} fld;

/* Declare the FMR overlay for this module. */
enum { _fld_configure, _fld_index };

/* Declare the _lf_module structure for this module. */
extern struct _lf_module _fld;

/* Declare the prototypes for all of the functions within this module. */
int fld_configure(void);
/* Returns the index of a loaded module. */
int fld_index(lf_crc_t identifier);

#endif
#endif
