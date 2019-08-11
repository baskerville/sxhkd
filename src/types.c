/* Copyright (c) 2013, Bastien Dejean
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "parse.h"
#include "grab.h"

hotkey_t *find_hotkey(xcb_keysym_t keysym, xcb_button_t button, uint16_t modfield, uint8_t event_type, bool *replay_event)
{
	int num_active = 0;
	int num_locked = 0;
	hotkey_t *result = NULL;

	for (hotkey_t *hk = hotkeys_head; hk != NULL; hk = hk->next) {
		chain_t *c = hk->chain;
		if ((chained && c->state == c->head) || (locked && c->state != c->tail))
			continue;
		if (match_chord(c->state, event_type, keysym, button, modfield)) {
			if (status_fifo != NULL && num_active == 0) {
				if (!chained) {
					snprintf(progress, sizeof(progress), "%s", c->state->repr);
				} else {
					strncat(progress, ";", sizeof(progress) - strlen(progress) - 1);
					strncat(progress, c->state->repr, sizeof(progress) - strlen(progress) - 1);
				}
				put_status(HOTKEY_PREFIX, progress);
			}
			if (replay_event != NULL && c->state->replay_event)
				*replay_event = true;
			if (c->state->lock_chain) {
				num_locked += 1;
				if (timeout > 0)
					alarm(0);
			}
			if (c->state == c->tail) {
				if (hk->cycle != NULL) {
					unsigned char delay = hk->cycle->delay;
					hk->cycle->delay = (delay == 0 ? hk->cycle->period - 1 : delay - 1);
					if (delay == 0)
						result = hk;
					continue;
				}
				if (chained && !locked)
					abort_chain();
				return hk;
			} else {
				c->state = c->state->next;
				num_active++;
				grab_chord(c->state);
			}
		} else if (chained) {
			if (!locked && c->state->event_type == event_type)
				c->state = c->head;
			else
				num_active++;
		}
	}

	if (result != NULL)
		return result;

	if (num_locked > 0) {
		locked = true;
	}

	if (!chained) {
		if (num_active > 0) {
			chained = true;
			put_status(BEGIN_CHAIN_PREFIX, "Begin chain");
			grab_chord(abort_chord);
		}
	} else if (num_active == 0 || match_chord(abort_chord, event_type, keysym, button, modfield)) {
		abort_chain();
		return find_hotkey(keysym, button, modfield, event_type, replay_event);
	}
	if (chained && !locked && timeout > 0)
		alarm(timeout);
	PRINTF("num active %i\n", num_active);

	return NULL;
}

bool match_chord(chord_t *chord, uint8_t event_type, xcb_keysym_t keysym, xcb_button_t button, uint16_t modfield)
{
	for (chord_t *c = chord; c != NULL; c = c->more)
		if (c->event_type == event_type && c->keysym == keysym && c->button == button && (c->modfield == XCB_MOD_MASK_ANY || c->modfield == modfield))
			return true;
	return false;
}

chord_t *make_chord(xcb_keysym_t keysym, xcb_button_t button, uint16_t modfield, uint8_t event_type, bool replay_event, bool lock_chain)
{
	chord_t *chord;
	if (button == XCB_NONE) {
		chord_t *prev = NULL;
		chord_t *orig = NULL;
		xcb_keycode_t *keycodes = keycodes_from_keysym(keysym);
		if (keycodes != NULL) {
			for (xcb_keycode_t *kc = keycodes; *kc != XCB_NO_SYMBOL; kc++) {
				xcb_keysym_t natural_keysym = xcb_key_symbols_get_keysym(symbols, *kc, 0);
				for (unsigned char col = 0; col < KEYSYMS_PER_KEYCODE; col++) {
					xcb_keysym_t ks = xcb_key_symbols_get_keysym(symbols, *kc, col);
					if (ks == keysym) {
						uint16_t implicit_modfield = (col & 1 ? XCB_MOD_MASK_SHIFT : 0) | (col & 2 ? modfield_from_keysym(Mode_switch) : 0);
						uint16_t explicit_modfield = modfield | implicit_modfield;
						chord = malloc(sizeof(chord_t));
						bool unique = true;
						for (chord_t *c = orig; unique && c != NULL; c = c->more)
							if (c->modfield == explicit_modfield && c->keysym == natural_keysym)
								unique = false;
						if (!unique) {
							free(chord);
							break;
						}
						chord->keysym = natural_keysym;
						chord->button = button;
						chord->modfield = explicit_modfield;
						chord->next = chord->more = NULL;
						chord->event_type = event_type;
						chord->replay_event = replay_event;
						chord->lock_chain = lock_chain;
						if (prev != NULL)
							prev->more = chord;
						else
							orig = chord;
						prev = chord;
						PRINTF("key chord %u %u\n", natural_keysym, explicit_modfield);
						break;
					}
				}
			}
		} else {
			warn("No keycodes found for keysym %u.\n", keysym);
		}
		free(keycodes);
		chord = orig;
	} else {
		chord = malloc(sizeof(chord_t));
		chord->keysym = keysym;
		chord->button = button;
		chord->modfield = modfield;
		chord->event_type = event_type;
		chord->replay_event = replay_event;
		chord->lock_chain = lock_chain;
		chord->next = chord->more = NULL;
		PRINTF("button chord %u %u\n", button, modfield);
	}
	return chord;
}

void add_chord(chain_t *chain, chord_t *chord)
{
	if (chain->head == NULL) {
		chain->head = chain->tail = chain->state = chord;
	} else {
		chain->tail->next = chord;
		chain->tail = chord;
	}
}

chain_t *make_chain(void)
{
	chain_t *chain = malloc(sizeof(chain_t));
	chain->head = chain->tail = chain->state = NULL;
	return chain;
}

cycle_t *make_cycle(int delay, int period)
{
	cycle_t *cycle = malloc(sizeof(cycle_t));
	cycle->delay = delay;
	cycle->period = period;
	return cycle;
}

hotkey_t *make_hotkey(chain_t *chain, char *command)
{
	hotkey_t *hk = malloc(sizeof(hotkey_t));
	hk->chain = chain;
	hk->sync = false;
	if (command[0] == SYNCHRONOUS_CHAR) {
		command = lgraph(command+1);
		hk->sync = true;
	}
	snprintf(hk->command, sizeof(hk->command), "%s", command);
	hk->cycle = NULL;
	hk->next = hk->prev = NULL;
	return hk;
}

void add_hotkey(hotkey_t *hk)
{
	if (hotkeys_head == NULL) {
		hotkeys_head = hotkeys_tail = hk;
	} else {
		hotkeys_tail->next = hk;
		hk->prev = hotkeys_tail;
		hotkeys_tail = hk;
	}
}

void abort_chain(void)
{
	PUTS("abort chain");
	put_status(END_CHAIN_PREFIX, "End chain");
	for (hotkey_t *hk = hotkeys_head; hk != NULL; hk = hk->next)
		hk->chain->state = hk->chain->head;
	chained = false;
	locked = false;
	if (timeout > 0)
		alarm(0);
	ungrab();
	grab();
}

void destroy_chain(chain_t *chain)
{
	chord_t *c = chain->head;
	while (c != NULL) {
		chord_t *n = c->next;
		destroy_chord(c);
		c = n;
	}
	free(chain);
}

void destroy_chord(chord_t *chord)
{
	chord_t *c = chord->more;
	while (c != NULL) {
		chord_t *n = c->more;
		free(c);
		c = n;
	}
	free(chord);
}
