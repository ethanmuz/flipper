#define __private_include__
#include <flipper/fmr.h>
#include <flipper/modules.h>
#include <platform/atmega16u2.h>

/* Define the standard modules accessible on this platform. */
const void *const fmr_modules[] PROGMEM = {
    _forward_id,
    &button,
    _forward_id,
    _forward_id,
    &fmr,
    _forward_id,
    _forward_id,
    _forward_id,
    &led,
    _forward_id,
    _forward_id,
    _forward_id,
    _forward_id,
    _forward_id,
    _forward_id,
    _forward_id,
    _forward_id,
};

const void *lf_std_module(lf_id_t module) {
    return (void *)(pgm_read_word(&fmr_modules[module]));
}
