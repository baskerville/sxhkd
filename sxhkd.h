#ifndef _SXHKD_H
#define _SXHKD_H

#include <xcb/xcb_keysyms.h>
#include <stdio.h>
#include <stdbool.h>
#include "helpers.h"

#define CONFIG_PATH  "sxhkd/sxhkdrc"
#define TOK_SEP      "+ \n"
#define NUM_MOD      8

typedef struct hotkey_t hotkey_t;
struct hotkey_t {
    xcb_keysym_t keysym;
    xcb_button_t button;
    uint16_t modfield;
    xcb_event_mask_t event_mask;
    char command[MAXLEN];
    hotkey_t *next;
};

typedef struct {
    char *name;
    xcb_keysym_t keysym;
} keysym_dict_t;

xcb_connection_t *dpy;
xcb_window_t root;
xcb_key_symbols_t *symbols;
hotkey_t *hotkeys;
char *config_file;
bool running, reload;

uint16_t num_lock;
uint16_t caps_lock;
uint16_t scroll_lock;

void hold(int);
void setup(void);
void cleanup(void);
void load_config(void);
void mapping_notify(xcb_generic_event_t *);
void key_event(xcb_generic_event_t *, xcb_event_mask_t);

#endif
