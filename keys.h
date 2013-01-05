#ifndef _KEYS_H
#define _KEYS_H

#include "sxhkd.h"

#define KEYSYMS_PER_KEYCODE  4
#define MOD_STATE_FIELD      255
#define NUM_MOD              8
#define SEQ_MIN_LEN          3
#define RELEASE_PREFIX       '@'
#define START_COMMENT        '#'
#define TOK_SEP              "+ \n"
#define SEQ_SEP              ","
#define SEQ_BEGIN            '{'
#define SEQ_END              '}'

void grab(void);
void grab_key_button(xcb_keycode_t, xcb_button_t, uint16_t);
void grab_key_button_checked(xcb_keycode_t, xcb_button_t, uint16_t);
void ungrab(void);
int16_t modfield_from_keysym(xcb_keysym_t);
xcb_keycode_t *keycodes_from_keysym(xcb_keysym_t);
bool parse_key(char *, xcb_keysym_t *);
bool parse_button(char *, xcb_button_t *);
bool parse_modifier(char *, uint16_t *);
bool parse_fold(char *, char *);
xcb_event_mask_t key_to_mouse(xcb_event_mask_t);
void get_lock_fields(void);
void unfold_hotkeys(char *, uint16_t, xcb_event_mask_t, char *);
void generate_hotkeys(xcb_keysym_t, xcb_button_t, uint16_t, xcb_event_mask_t, char *);
hotkey_t *make_hotkey(xcb_keysym_t, xcb_button_t, uint16_t, xcb_event_mask_t, char *);
hotkey_t *find_hotkey(xcb_keysym_t, xcb_button_t, uint16_t, xcb_event_mask_t);
void add_hotkey(hotkey_t *);

#endif
