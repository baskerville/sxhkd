#ifndef _HOTKEYS_H
#define _HOTKEYS_H

#include "sxhkd.h"

#define KEYSYMS_PER_KEYCODE  4
#define MOD_STATE_FIELD      255
#define NUM_MOD              8
#define SEQ_MIN_LEN          3
#define RELEASE_PREFIX       '@'
#define MOTION_PREFIX        '!'
#define REPLAY_PREFIX        ':'
#define START_COMMENT        '#'
#define MAGIC_INHIBIT        '\\'
#define PARTIAL_LINE         '\\'
#define LNK_SEP              ";"
#define SYM_SEP              "+ "
#define SEQ_BEGIN            '{'
#define SEQ_END              '}'
#define SEQ_SEP              ','

typedef struct {
    char *name;
    xcb_keysym_t keysym;
} keysym_dict_t;

xcb_keysym_t Alt_L, Alt_R, Super_L, Super_R, Hyper_L, Hyper_R,
             Meta_L, Meta_R, Mode_switch, Num_Lock, Scroll_Lock;

void grab(void);
void grab_chord(chord_t *);
void grab_key_button(xcb_keycode_t, xcb_button_t, uint16_t);
void grab_key_button_checked(xcb_keycode_t, xcb_button_t, uint16_t);
void ungrab(void);
int16_t modfield_from_keysym(xcb_keysym_t);
xcb_keycode_t *keycodes_from_keysym(xcb_keysym_t);
chord_t *make_chord(xcb_keysym_t, xcb_button_t, uint16_t, uint8_t, bool);
chain_t *make_chain(void);
void add_chord(chain_t *, chord_t *);
void process_hotkey(char *, char *);
char *gettok(char *, char *, char);
bool extract_sequence(char *, char *, char *, char *);
bool parse_chain(char *, chain_t *);
bool parse_keysym(char *, xcb_keysym_t *);
bool parse_button(char *, xcb_button_t *);
bool parse_modifier(char *, uint16_t *);
bool parse_fold(char *, char *);
uint8_t key_to_button(uint8_t);
void get_standard_keysyms(void);
void get_lock_fields(void);
bool match_chord(chord_t *, uint8_t, xcb_keysym_t, xcb_button_t, uint16_t);
hotkey_t *find_hotkey(xcb_keysym_t, xcb_button_t, uint16_t, uint8_t, bool *);
hotkey_t *make_hotkey(chain_t *, char *);
void abort_chain(void);
void add_hotkey(hotkey_t *);

#endif
