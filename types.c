#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "types.h"
#include "parse.h"
#include "grab.h"

hotkey_t *find_hotkey(xcb_keysym_t keysym, xcb_button_t button, uint16_t modfield, uint8_t event_type, bool *replay_event)
{
    int num_active = 0;

    for (hotkey_t *hk = hotkeys; hk != NULL; hk = hk->next) {
        chain_t *c = hk->chain;
        if (chained && c->state == c->head)
            continue;
        if (match_chord(c->state, event_type, keysym, button, modfield)) {
            if (status_fifo != NULL && num_active == 0) {
                if (!chained) {
                    strncpy(progress, c->state->repr, sizeof(progress));
                } else {
                    strncat(progress, LNK_SEP, sizeof(progress) - strlen(progress) - 1);
                    strncat(progress, c->state->repr, sizeof(progress) - strlen(progress) - 1);
                }
                put_status(HOTKEY_PREFIX, progress);
            }
            if (replay_event != NULL && c->state->replay_event)
                *replay_event = true;
            if (!locked && c->state->lock_chain) {
                locked = true;
                grab_chord(escape_chord);
            }
            if (c->state == c->tail) {
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

    if (!chained) {
        if (num_active > 0)
            chained = true;
    } else if (num_active == 0 || (locked && match_chord(escape_chord, event_type, keysym, button, modfield))) {
        abort_chain();
        return find_hotkey(keysym, button, modfield, event_type, replay_event);
    }
    if (chained && timeout > 0)
        alarm(timeout);
    PRINTF("num active %i\n", num_active);

    return NULL;
}

bool match_chord(chord_t *chord, uint8_t event_type, xcb_keysym_t keysym, xcb_button_t button, uint16_t modfield) {
    for (chord_t *c = chord; c != NULL; c = c->more)
        if (c->event_type == event_type && c->keysym == keysym && c->button == button && c->modfield == modfield)
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
        if (keycodes != NULL)
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

hotkey_t *make_hotkey(chain_t *chain, char *command)
{
    hotkey_t *hk = malloc(sizeof(hotkey_t));
    hk->chain = chain;
    strncpy(hk->command, command, sizeof(hk->command));
    hk->next = NULL;
    return hk;
}

void add_hotkey(hotkey_t *hk)
{
    if (hotkeys == NULL) {
        hotkeys = hotkeys_tail = hk;
    } else {
        hotkeys_tail->next = hk;
        hotkeys_tail = hk;
    }
}

void abort_chain(void)
{
    PUTS("abort chain");
    for (hotkey_t *hk = hotkeys; hk != NULL; hk = hk->next)
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
