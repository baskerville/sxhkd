#ifndef _SXHKD_H
#define _SXHKD_H

#include <xcb/xcb_keysyms.h>
#include <stdio.h>
#include <stdbool.h>
#include "helpers.h"

#define CONFIG_HOME_ENV  "XDG_CONFIG_HOME"
#define SXHKD_SHELL_ENV  "SXHKD_SHELL"
#define SHELL_ENV        "SHELL"
#define CONFIG_PATH      "sxhkd/sxhkdrc"
#define TOK_SEP          "+ \n"
#define NUM_MOD          8

typedef struct hotkey_t hotkey_t;
struct hotkey_t {
    xcb_keysym_t keysym;
    xcb_button_t button;
    uint16_t modfield;
    uint8_t event_type;
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
char *shell;
hotkey_t *hotkeys;

char config_file[MAXLEN];
char *config_path;
char **extra_confs;
int num_extra_confs;

bool running, reload;

uint16_t num_lock;
uint16_t caps_lock;
uint16_t scroll_lock;

void hold(int);
void setup(void);
void cleanup(void);
void reload_all(void);
void load_config(char *config_file);
void mapping_notify(xcb_generic_event_t *);
void key_event(xcb_generic_event_t *, uint8_t);
void motion_notify(xcb_generic_event_t *, uint8_t);

#endif
