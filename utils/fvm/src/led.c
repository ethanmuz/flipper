#include <flipper/libflipper.h>

#ifdef __use_led__
#define __private_include__
#include <flipper/led.h>

int led_configure(void) {
	printf("Configuring the led.\n");
	return lf_success;
}

void led_rgb(uint8_t r, uint8_t g, uint8_t b) {
	printf("Setting the led color to R: %i, G: %i, B: %i.\n", r, g, b);
}

#endif
