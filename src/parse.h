/* Copyright (c) 2013, Bastien Dejean
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SXHKD_PARSE_H
#define SXHKD_PARSE_H

#include "sxhkd.h"

#define RELEASE_PREFIX       '@'
#define REPLAY_PREFIX        '~'
#define START_COMMENT        '#'
#define MAGIC_INHIBIT        '\\'
#define PARTIAL_LINE         '\\'
#define GRP_SEP              ":"
#define LNK_SEP              ";" GRP_SEP
#define SYM_SEP              "+ "
#define SEQ_BEGIN            '{'
#define SEQ_END              '}'
#define SEQ_SEP              ","
#define SEQ_NONE             '_'

typedef struct chunk_t chunk_t;
struct chunk_t {
	char text[2 * MAXLEN];
	char item[2 * MAXLEN];
	char *advance;
	bool sequence;
	char range_cur;
	char range_max;
	chunk_t *next;
};

xcb_keysym_t Alt_L, Alt_R, Super_L, Super_R, Hyper_L, Hyper_R,
             Meta_L, Meta_R, Mode_switch, Num_Lock, Scroll_Lock;

void load_config(const char *config_file);
void parse_event(xcb_generic_event_t *evt, uint8_t event_type, xcb_keysym_t *keysym, xcb_button_t *button, uint16_t *modfield);
void process_hotkey(char *hotkey_string, char *command_string);
char *get_token(char *dst, char *ign, char *src, char *sep);
void render_next(chunk_t *chunks, char *dest);
chunk_t *extract_chunks(char *s);
chunk_t *make_chunk(void);
void destroy_chunks(chunk_t *chunk);
bool parse_chain(char *string, chain_t *chain);
bool parse_keysym(char *name, xcb_keysym_t *keysym);
bool parse_button(char *name, xcb_button_t *butidx);
bool parse_modifier(char *name, uint16_t *modfield);
bool parse_fold(char *string, char *folded_string);
uint8_t key_to_button(uint8_t event_type);
void get_standard_keysyms(void);
void get_lock_fields(void);
int16_t modfield_from_keysym(xcb_keysym_t keysym);
int16_t modfield_from_keycode(xcb_keycode_t keycode);
xcb_keycode_t *keycodes_from_keysym(xcb_keysym_t keysym);

#endif
