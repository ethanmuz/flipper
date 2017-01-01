#ifndef __swd_h__
#define __swd_h__

/* Include all types and macros exposed by the Flipper Toolbox. */
#include <flipper/libflipper.h>

/* Declare the virtual interface for this module. */
extern const struct _swd {

	int (* configure)(void);

} swd;

#ifdef __private_include__

/* The fmr_module structure for this module. */
extern struct _lf_module _swd;

/* Declare the FMR overlay for this driver. */
enum { _swd_configure };

/* Declare each prototype for all functions within this driver. */
int swd_configure(void);

#endif
#endif
