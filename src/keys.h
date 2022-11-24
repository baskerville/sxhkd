#ifndef SXHKD_KEYS_H
#define SXHKD_KEYS_H

#include <xkbcommon/xkbcommon.h>

struct keysym_array {
	xkb_keysym_t const* data;
	size_t              count;
};

struct keycode_array {
	xkb_keycode_t * data;
	size_t          count;
};

void keycode_array_free(struct keycode_array*);

struct keysym_array keycode_to_keysyms(struct xkb_keymap*, xkb_keycode_t);
xkb_keysym_t        keycode_to_keysym (struct xkb_keymap*, xkb_keycode_t);

struct keycode_array keycodes_from_keysym(struct xkb_keymap*, xkb_keysym_t);

#endif

