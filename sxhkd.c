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

#include <xcb/xcb_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include "parse.h"
#include "grab.h"
#include "sxhkd.h"

int main(int argc, char *argv[])
{
	char opt;
	char *fifo_path = NULL;
	char *socket_path = NULL;
	status_fifo = NULL;
	config_path = NULL;
	mapping_count = 0;
	timeout = TIMEOUT;
	sock_address.sun_family = AF_UNIX;
	sock_address.sun_path[0] = 0;
	snprintf(motion_msg_tpl, sizeof(motion_msg_tpl), "%s", MOTION_MSG_TPL);
	unsigned int max_freq = 0;
	motion_interval = 0;
	last_motion_time = 0;
	redir_fd = -1;

	while ((opt = getopt(argc, argv, "vhm:t:c:r:s:f:o:g:")) != (char)-1) {
		switch (opt) {
			case 'v':
				printf("%s\n", VERSION);
				exit(EXIT_SUCCESS);
				break;
			case 'h':
				printf("sxhkd [-h|-v|-m COUNT|-t TIMEOUT|-c CONFIG_FILE|-r REDIR_FILE|-s STATUS_FIFO|-o MOTION_SOCKET|-g MOTION_MSG_TPL] [EXTRA_CONFIG ...]\n");
				exit(EXIT_SUCCESS);
				break;
			case 'm':
				if (sscanf(optarg, "%i", &mapping_count) != 1)
					warn("Can't parse mapping count.\n");
				break;
			case 't':
				timeout = atoi(optarg);
				break;
			case 'c':
				config_path = optarg;
				break;
			case 'r':
				redir_fd = open(optarg, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (redir_fd == -1)
					warn("Failed to open the command redirection file.\n");
				break;
			case 's':
				fifo_path = optarg;
				break;
			case 'o':
				socket_path = optarg;
				break;
			case 'g':
				snprintf(motion_msg_tpl, sizeof(motion_msg_tpl), "%s", optarg);
				break;
			case 'f':
				if (sscanf(optarg, "%u", &max_freq) != 1)
					warn("Can't parse maximum pointer frequency.\n");
				break;
		}
	}

	num_extra_confs = argc - optind;
	extra_confs = argv + optind;

	if (config_path == NULL) {
		char *config_home = getenv(CONFIG_HOME_ENV);
		if (config_home != NULL)
			snprintf(config_file, sizeof(config_file), "%s/%s", config_home, CONFIG_PATH);
		else
			snprintf(config_file, sizeof(config_file), "%s/%s/%s", getenv("HOME"), ".config", CONFIG_PATH);
	} else {
		snprintf(config_file, sizeof(config_file), "%s", config_path);
	}

	if (socket_path == NULL) {
		socket_path = getenv(SOCKET_ENV);
	}

	if (socket_path == NULL) {
		char *host = NULL;
		int dn = 0, sn = 0;
		if (xcb_parse_display(NULL, &host, &dn, &sn) != 0) {
			snprintf(sock_address.sun_path, sizeof(sock_address.sun_path), SOCKET_PATH_TPL, host, dn, sn);
		} else {
			warn("Failed to set motion socket address.");
		}
		free(host);
	} else {
		snprintf(sock_address.sun_path, sizeof(sock_address.sun_path), "%s", socket_path);
	}

	if (fifo_path != NULL) {
		int fifo_fd = open(fifo_path, O_RDWR | O_NONBLOCK);
		if (fifo_fd != -1)
			status_fifo = fdopen(fifo_fd, "w");
		else
			warn("Couldn't open status fifo.\n");
	}

	if (max_freq != 0)
		motion_interval = 1000.0 / max_freq;

	signal(SIGINT, hold);
	signal(SIGHUP, hold);
	signal(SIGTERM, hold);
	signal(SIGUSR1, hold);
	signal(SIGALRM, hold);

	setup();
	get_standard_keysyms();
	get_lock_fields();
	escape_chord = make_chord(ESCAPE_KEYSYM, XCB_NONE, 0, XCB_KEY_PRESS, false, false);
	load_config(config_file);
	for (int i = 0; i < num_extra_confs; i++)
		load_config(extra_confs[i]);
	grab();

	xcb_generic_event_t *evt;
	int fd = xcb_get_file_descriptor(dpy);

	fd_set descriptors;

	reload = bell = chained = locked = false;
	running = true;

	xcb_flush(dpy);

	while (running) {
		FD_ZERO(&descriptors);
		FD_SET(fd, &descriptors);

		if (select(fd + 1, &descriptors, NULL, NULL, NULL) > 0) {
			while ((evt = xcb_poll_for_event(dpy)) != NULL) {
				uint8_t event_type = XCB_EVENT_RESPONSE_TYPE(evt);
				switch (event_type) {
					case XCB_KEY_PRESS:
					case XCB_KEY_RELEASE:
					case XCB_BUTTON_PRESS:
					case XCB_BUTTON_RELEASE:
						key_button_event(evt, event_type);
						break;
					case XCB_MOTION_NOTIFY:
						motion_notify(evt);
						break;
					case XCB_MAPPING_NOTIFY:
						mapping_notify(evt);
						break;
					default:
						PRINTF("received event %u\n", event_type);
						break;
				}
				free(evt);
			}
		}

		if (reload) {
			signal(SIGUSR1, hold);
			reload_cmd();
			reload = false;
		}

		if (bell) {
			signal(SIGALRM, hold);
			abort_chain();
			if (status_fifo != NULL)
				put_status(TIMEOUT_PREFIX, "Timeout reached");
			bell = false;
		}

		if (xcb_connection_has_error(dpy)) {
			warn("The server closed the connection.\n");
			running = false;
		}
	}

	if (redir_fd != -1)
		close(redir_fd);
	if (status_fifo != NULL)
		fclose(status_fifo);
	ungrab();
	cleanup();
	destroy_chord(escape_chord);
	xcb_key_symbols_free(symbols);
	xcb_disconnect(dpy);
	return EXIT_SUCCESS;
}

void key_button_event(xcb_generic_event_t *evt, uint8_t event_type)
{
	xcb_keysym_t keysym = XCB_NO_SYMBOL;
	xcb_button_t button = XCB_NONE;
	bool replay_event = false;
	uint16_t modfield = 0;
	uint16_t lockfield = num_lock | caps_lock | scroll_lock;
	parse_event(evt, event_type, &keysym, &button, &modfield);
	modfield &= ~lockfield & MOD_STATE_FIELD;
	if (keysym != XCB_NO_SYMBOL || button != XCB_NONE) {
		hotkey_t *hk = find_hotkey(keysym, button, modfield, event_type, &replay_event);
		if (hk != NULL) {
			run(hk->command);
			if (status_fifo != NULL)
				put_status(COMMAND_PREFIX, hk->command);
		}
	}
	switch (event_type) {
		case XCB_BUTTON_PRESS:
		case XCB_BUTTON_RELEASE:
			if (replay_event)
				xcb_allow_events(dpy, XCB_ALLOW_REPLAY_POINTER, XCB_CURRENT_TIME);
			else
				xcb_allow_events(dpy, XCB_ALLOW_SYNC_POINTER, XCB_CURRENT_TIME);
			break;
		case XCB_KEY_PRESS:
		case XCB_KEY_RELEASE:
			if (replay_event)
				xcb_allow_events(dpy, XCB_ALLOW_REPLAY_KEYBOARD, XCB_CURRENT_TIME);
			else
				xcb_allow_events(dpy, XCB_ALLOW_SYNC_KEYBOARD, XCB_CURRENT_TIME);
			break;
	}
	xcb_flush(dpy);
}

void motion_notify(xcb_generic_event_t *evt)
{
	xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *) evt;
	PRINTF("motion notify %X %X %u %i %i\n", e->child, e->detail, e->state, e->root_x, e->root_y);
	if (motion_interval > 0 && (e->time - last_motion_time) < motion_interval)
		return;
	last_motion_time = e->time;
	char msg[MAXLEN];
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		return;
	}
	if (connect(fd, (struct sockaddr *) &sock_address, sizeof(sock_address)) == -1) {
		close(fd);
		return;
	}
	int len = sizeof(msg), i = 0;
	for (int j = 0, c = motion_msg_tpl[j]; c && i < len; j++, c = motion_msg_tpl[j]) {
		if (c == ' ') {
			msg[i++] = 0;
		} else if (c == 'X') {
			i += snprintf(msg+i, len-i, "%i", e->root_x);
		} else if (c == 'Y') {
			i += snprintf(msg+i, len-i, "%i", e->root_y);
		} else {
			msg[i++] = c;
		}
	}
	if (i >= len) {
		i--;
	}
	msg[i] = 0;
	send(fd, msg, i+1, 0);
	close(fd);
}

void mapping_notify(xcb_generic_event_t *evt)
{
	if (!mapping_count)
		return;
	xcb_mapping_notify_event_t *e = (xcb_mapping_notify_event_t *) evt;
	PRINTF("mapping notify %u %u\n", e->request, e->count);
	if (e->request == XCB_MAPPING_POINTER)
		return;
	if (xcb_refresh_keyboard_mapping(symbols, e) == 1) {
		destroy_chord(escape_chord);
		get_lock_fields();
		reload_cmd();
		escape_chord = make_chord(ESCAPE_KEYSYM, XCB_NONE, 0, XCB_KEY_PRESS, false, false);
		if (mapping_count > 0)
			mapping_count--;
	}
}

void setup(void)
{
	dpy = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(dpy))
		err("Can't open display.\n");
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
	if (screen == NULL)
		err("Can't acquire screen.\n");
	root = screen->root;
	if ((shell = getenv(SXHKD_SHELL_ENV)) == NULL && (shell = getenv(SHELL_ENV)) == NULL)
		err("The '%s' environment variable is not defined.\n", SHELL_ENV);
	symbols = xcb_key_symbols_alloc(dpy);
	hotkeys_head = hotkeys_tail = NULL;
	progress[0] = '\0';
}

void cleanup(void)
{
	PUTS("cleanup");
	hotkey_t *hk = hotkeys_head;
	while (hk != NULL) {
		hotkey_t *next = hk->next;
		destroy_chain(hk->chain);
		free(hk->cycle);
		free(hk);
		hk = next;
	}
	hotkeys_head = hotkeys_tail = NULL;
}

void reload_cmd(void)
{
	PUTS("reload");
	cleanup();
	load_config(config_file);
	for (int i = 0; i < num_extra_confs; i++)
		load_config(extra_confs[i]);
	ungrab();
	grab();
}

void hold(int sig)
{
	if (sig == SIGHUP || sig == SIGINT || sig == SIGTERM)
		running = false;
	else if (sig == SIGUSR1)
		reload = true;
	else if (sig == SIGALRM)
		bell = true;
}

void put_status(char c, const char *s)
{
	fprintf(status_fifo, "%c%s\n", c, s);
	fflush(status_fifo);
}
