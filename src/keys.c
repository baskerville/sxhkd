#include "keys.h"
#include <stdlib.h>
#include <stddef.h>

struct keysym_array keycode_to_keysyms(struct xkb_keymap* keymap, xkb_keycode_t keycode)
{
	xkb_keysym_t const* data  = NULL;
	int                 count = 0;
	count = xkb_keymap_key_get_syms_by_level(keymap, keycode, 0, 0, &data);

	if(count <= 0) {
		return (struct keysym_array){ .data = NULL, .count = 0 };
	}

	return (struct keysym_array){ .data = data, .count = count };
}

xkb_keysym_t keycode_to_keysym (struct xkb_keymap* keymap, xkb_keycode_t keycode)
{
	struct keysym_array keysyms = keycode_to_keysyms(keymap, keycode);

	if(keysyms.data)
		return *keysyms.data;
	else 
		return XKB_KEY_NoSymbol;
}

void keycode_array_free(struct keycode_array* keycodes)
{
	if(keycodes->data) {
		free(keycodes->data);
		keycodes->data  = NULL;
		keycodes->count = 0;
	}
}

struct keycodes_from_keysym_iterator
{
	xkb_keysym_t    keysym;
	xkb_keycode_t * data;
	size_t          count;
};

static void keycodes_from_keysym_counter(struct xkb_keymap* keymap, xkb_keycode_t keycode, void* data)
{
	struct keycodes_from_keysym_iterator * state = data;
	struct keysym_array keysyms = keycode_to_keysyms(keymap, keycode);

	xkb_keysym_t const* first = keysyms.data;
	xkb_keysym_t const* last  = keysyms.data + keysyms.count;

	for(; first != last; ++first) {
		if(state->keysym == *first) {
			state->count++;
			break;
		}
	}
}

static void keycodes_from_keysym_collector(struct xkb_keymap* keymap, xkb_keycode_t keycode, void* data)
{
	struct keycodes_from_keysym_iterator * state   = data;
	struct keysym_array                    keysyms = keycode_to_keysyms(keymap, keycode);

	xkb_keysym_t const* first = keysyms.data;
	xkb_keysym_t const* last  = keysyms.data + keysyms.count;

	for(; first != last; ++first) {
		if(state->keysym == *first) {
			state->data[state->count++] = keycode;
			break;
		}
	}
}

struct keycode_array keycodes_from_keysym(struct xkb_keymap* keymap, xkb_keysym_t keysym)
{
	struct keycodes_from_keysym_iterator counter = {
		.keysym = keysym,
		.data   = NULL,
		.count  = 0
	};

	xkb_keymap_key_for_each(keymap, keycodes_from_keysym_counter, &counter);

	struct keycodes_from_keysym_iterator collector = {
		.keysym = keysym,
		.data   = malloc(counter.count * sizeof(xkb_keycode_t)),
		.count  = 0
	};

	xkb_keymap_key_for_each(keymap, keycodes_from_keysym_collector, &collector);

	return (struct keycode_array){ .data = collector.data, .count = collector.count };
}
