#ifndef _TYPES_H
#define _TYPES_H

#include <xcb/xcb_keysyms.h>
#include <stdbool.h>
#include "helpers.h"

#define KEYSYMS_PER_KEYCODE  4
#define MOD_STATE_FIELD      255

typedef struct chord_t chord_t;
struct chord_t {
    char repr[MAXLEN];
    xcb_keysym_t keysym;
    xcb_button_t button;
    uint16_t modfield;
    uint8_t event_type;
    bool replay_event;
    chord_t *next;
    chord_t *more;
};

typedef struct {
    chord_t *head;
    chord_t *tail;
    chord_t *state;
} chain_t;

typedef struct hotkey_t hotkey_t;
struct hotkey_t {
    chain_t *chain;
    char command[MAXLEN];
    hotkey_t *next;
};

typedef struct {
    char *name;
    xcb_keysym_t keysym;
} keysym_dict_t;

xcb_keysym_t Alt_L, Alt_R, Super_L, Super_R, Hyper_L, Hyper_R,
             Meta_L, Meta_R, Mode_switch, Num_Lock, Scroll_Lock;

chord_t *make_chord(xcb_keysym_t, xcb_button_t, uint16_t, uint8_t, bool);
chain_t *make_chain(void);
void add_chord(chain_t *, chord_t *);
bool match_chord(chord_t *, uint8_t, xcb_keysym_t, xcb_button_t, uint16_t);
hotkey_t *find_hotkey(xcb_keysym_t, xcb_button_t, uint16_t, uint8_t, bool *);
hotkey_t *make_hotkey(chain_t *, char *);
void add_hotkey(hotkey_t *);
void abort_chain(void);
void destroy_chain(chain_t *);

#endif
