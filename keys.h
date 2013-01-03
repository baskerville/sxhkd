#ifndef _KEYS_H
#define _KEYS_H

#include "sxhkd.h"

#define KEYSYMS_PER_KEYCODE  4

void grab(void);
void grab_key(xcb_keycode_t, uint16_t);
void grab_key_checked(xcb_keycode_t, uint16_t);
void ungrab(void);
int16_t modfield_from_keysym(xcb_keysym_t);
xcb_keycode_t *keycodes_from_keysym(xcb_keysym_t);
bool parse_keysym(char *, xcb_keysym_t *);
bool parse_modmask(char *, uint16_t *);
void get_lock_fields(void);
void generate_hotkeys(xcb_keysym_t, uint16_t, xcb_event_mask_t, char *);
hotkey_t *make_hotkey(xcb_keysym_t, uint16_t, xcb_event_mask_t, char *);
hotkey_t *find_hotkey(xcb_keysym_t, uint16_t, xcb_event_mask_t);
void add_hotkey(hotkey_t *);

#endif
