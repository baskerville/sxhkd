#ifndef _GRAB_H
#define _GRAB_H

#include "sxhkd.h"

void grab(void);
void grab_chord(chord_t *);
void grab_key_button(xcb_keycode_t, xcb_button_t, uint16_t);
void grab_key_button_checked(xcb_keycode_t, xcb_button_t, uint16_t);
void ungrab(void);

#endif
