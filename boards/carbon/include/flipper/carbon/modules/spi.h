#ifndef __spi_h__
#define __spi_h__

/* Include all types and macros exposed by the Flipper Toolbox. */
#include <flipper/libflipper.h>

/* Declare the virtual interface for this modules. */
extern const struct _spi {
	int (* configure)(void);
	void (* enable)(void);
	void (* disable)(void);
	uint8_t (* ready)(void);
	void (* put)(uint8_t byte);
	uint8_t (* get)(void);
	int (* push)(void *source, uint32_t length);
	int (* pull)(void *destination, uint32_t length);
} spi;

#ifdef __private_include__

/* The fmr_module structure for this module. */
extern struct _lf_module _spi;

/* Declare the FMR overlay for this driver. */
enum { _spi_configure, _spi_enable, _spi_disable, _spi_ready, _spi_put, _spi_get, _spi_push, _spi_pull };

/* Declare each prototype for all functions within this driver. */
int spi_configure();
void spi_enable(void);
void spi_disable(void);
uint8_t spi_ready(void);
void spi_put(uint8_t byte);
uint8_t spi_get(void);
int spi_push(void *source, uint32_t length);
int spi_pull(void *destination, uint32_t length);

#endif
#endif
