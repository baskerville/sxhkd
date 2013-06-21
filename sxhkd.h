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
#define NUM_MOD          8
#define TIMEOUT          3

typedef struct chord_t chord_t;
struct chord_t {
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

xcb_connection_t *dpy;
xcb_window_t root;
xcb_key_symbols_t *symbols;

char *shell;
char config_file[MAXLEN];
char *config_path;
char **extra_confs;
int num_extra_confs;
int redir_fd;
int timeout;

hotkey_t *hotkeys, *hotkeys_tail;
bool running, reload, bell, chained;

uint16_t num_lock;
uint16_t caps_lock;
uint16_t scroll_lock;

void hold(int);
void setup(void);
void load_config(char *);
void cleanup(void);
void destroy_chain(chain_t *);
void reload_cmd(void);
void parse_event(xcb_generic_event_t *, uint8_t, xcb_keysym_t *, xcb_button_t *, uint16_t *);
void key_button_event(xcb_generic_event_t *, uint8_t);
void motion_notify(xcb_generic_event_t *, uint8_t);

#endif
