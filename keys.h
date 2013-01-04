#ifndef _KEYS_H
#define _KEYS_H

#include "sxhkd.h"

#define KEYSYMS_PER_KEYCODE  4

void grab(void);
void grab_key_button(xcb_keycode_t, xcb_button_t, uint16_t);
void grab_key_button_checked(xcb_keycode_t, xcb_button_t, uint16_t);
void ungrab(void);
int16_t modfield_from_keysym(xcb_keysym_t);
xcb_keycode_t *keycodes_from_keysym(xcb_keysym_t);
bool parse_key(char *, xcb_keysym_t *);
bool parse_button(char *, xcb_button_t *);
bool parse_modifier(char *, uint16_t *);
xcb_event_mask_t key_to_mouse(xcb_event_mask_t);
void get_lock_fields(void);
void generate_hotkeys(xcb_keysym_t, xcb_button_t, uint16_t, xcb_event_mask_t, char *);
hotkey_t *make_hotkey(xcb_keysym_t, xcb_button_t, uint16_t, xcb_event_mask_t, char *);
hotkey_t *find_hotkey(xcb_keysym_t, xcb_button_t, uint16_t, xcb_event_mask_t);
void add_hotkey(hotkey_t *);

#endif
