#define __private_include__
#include <flipper/carbon.h>
#include <flipper/posix/posix.h>
#include <dlfcn.h>

/* Define the standard modules based on platform specific usage declarations. */
const void *const fmr_modules[] = {
	&adc,
	&button,
	&dac,
	&fld,
	&gpio,
	&i2c,
	&led,
	&pwm,
	&rtc,
	&spi,
	&swd,
	&task,
	&temp,
	&timer,
	&uart0,
	&usart,
	&usb,
	&wdt
};

lf_return_t fmr_call(lf_return_t (* function)(void), uint8_t argc, uint16_t argt, void *argv) {
	/* Grab the symbol name of the function for debugging purposes. */
	Dl_info info;
	dladdr(function, &info);
	printf("Calling local function '%s', with %i arguments, arg types %i, and va_list %p.\n", info.dli_sname, argc, argt, argv);
	return function();
}

lf_return_t fmr_push(struct _fmr_push_pull_packet *packet) {
	return -1;
}

lf_return_t fmr_pull(struct _fmr_push_pull_packet *packet) {
	return -1;
}
