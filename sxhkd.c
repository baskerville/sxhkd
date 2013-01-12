#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <fcntl.h>
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include "helpers.h"
#include "hotkeys.h"
#include "sxhkd.h"

void hold(int sig)
{
    if (sig == SIGHUP || sig == SIGINT || sig == SIGTERM)
        running = false;
    else if (sig == SIGUSR1)
        reload = true;
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
    hotkeys = NULL;
}

void cleanup(void)
{
    hotkey_t *hk = hotkeys;
    while (hk != NULL) {
        hotkey_t *tmp = hk->next;
        free(hk);
        hk = tmp;
    }
}

void reload_cmd(void)
{
    PUTS("reload");
    cleanup();
    hotkeys = NULL;
    load_config(config_file);
    for (int i = 0; i < num_extra_confs; i++)
        load_config(extra_confs[i]);
    ungrab();
    grab();
}

void load_config(char *config_file)
{
    PRINTF("load configuration '%s'\n", config_file);


    FILE *cfg = fopen(config_file, "r");
    if (cfg == NULL)
        err("Can't open configuration file: '%s'.\n", config_file);

    char line[MAXLEN];
    xcb_keysym_t keysym = XCB_NO_SYMBOL;
    xcb_button_t button = XCB_NONE;
    uint16_t modfield = 0;
    uint8_t event_type = XCB_KEY_PRESS;
    char folded_hotkey[MAXLEN] = {'\0'};

    while (fgets(line, sizeof(line), cfg) != NULL) {
        if (strlen(line) < 2 || line[0] == START_COMMENT) {
            continue;
        } else if (isspace(line[0])) {
            if (keysym == XCB_NO_SYMBOL && button == XCB_NONE && strlen(folded_hotkey) == 0)
                continue;
            unsigned int i = strlen(line) - 1;
            while (i > 0 && isspace(line[i]))
                line[i--] = '\0';
            i = 1;
            while (i < strlen(line) && isspace(line[i]))
                i++;
            if (i < strlen(line)) {
                char *command = line + i;
                if (strlen(folded_hotkey) == 0)
                    generate_hotkeys(keysym, button, modfield, event_type, command);
                else
                    unfold_hotkeys(folded_hotkey, command);
            }
            keysym = XCB_NO_SYMBOL;
            button = XCB_NONE;
            modfield = 0;
            event_type = XCB_KEY_PRESS;
            folded_hotkey[0] = '\0';
        } else {
            unsigned int i = strlen(line) - 1;
            while (i > 0 && isspace(line[i]))
                line[i--] = '\0';
            if (!parse_fold(line, folded_hotkey))
                parse_hotkey(line, &keysym, &button, &modfield, &event_type);
        }
    }

    fclose(cfg);
}

void key_button_event(xcb_generic_event_t *evt, uint8_t event_type)
{
    xcb_keysym_t keysym = XCB_NO_SYMBOL;
    xcb_keycode_t keycode = XCB_NONE;
    xcb_button_t button = XCB_NONE;
    uint16_t modfield = 0;
    uint16_t lockfield = num_lock | caps_lock | scroll_lock;
    if (event_type == XCB_KEY_PRESS) {
        xcb_key_press_event_t *e = (xcb_key_press_event_t *) evt;
        keycode = e->detail;
        modfield = e->state;
        keysym = xcb_key_symbols_get_keysym(symbols, keycode, 0);
        PRINTF("key press %u %u\n", keycode, modfield);
    } else if (event_type == XCB_KEY_RELEASE) {
        xcb_key_release_event_t *e = (xcb_key_release_event_t *) evt;
        keycode = e->detail;
        modfield = e->state;
        keysym = xcb_key_symbols_get_keysym(symbols, keycode, 0);
        PRINTF("key release %u %u\n", keycode, modfield);
    } else if (event_type == XCB_BUTTON_PRESS) {
        xcb_button_press_event_t *e = (xcb_button_press_event_t *) evt;
        button = e->detail;
        modfield = e->state;
        PRINTF("button press %u %u\n", button, modfield);
    } else if (event_type == XCB_BUTTON_RELEASE) {
        xcb_button_release_event_t *e = (xcb_button_release_event_t *) evt;
        button = e->detail;
        modfield = e->state;
        PRINTF("button release %u %u\n", button, modfield);
    }
    modfield &= ~lockfield & MOD_STATE_FIELD;
    if (keysym != XCB_NO_SYMBOL || button != XCB_NONE) {
        hotkey_t *hk = find_hotkey(keysym, button, modfield, event_type);
        if (hk != NULL)
            run(hk->command);
    }
}

void motion_notify(xcb_generic_event_t *evt, uint8_t event_type)
{
    xcb_motion_notify_event_t *e = (xcb_motion_notify_event_t *) evt;
    /* PRINTF("motion notify %X %X %u\n", e->child, e->detail, e->state); */
    uint16_t lockfield = num_lock | caps_lock | scroll_lock;
    uint16_t buttonfield = e->state >> 8;
    uint16_t modfield = e->state & ~lockfield & MOD_STATE_FIELD;
    xcb_button_t button = 1;
    while (~buttonfield & 1 && button < 5) {
        buttonfield = buttonfield >> 1;
        button++;
    }
    hotkey_t *hk = find_hotkey(XCB_NO_SYMBOL, button, modfield, event_type);
    if (hk != NULL) {
        char command[MAXLEN];
        snprintf(command, sizeof(command), hk->command, e->root_x, e->root_y);
        run(command);
    }
}

int main(int argc, char *argv[])
{
    char opt;
    config_path = NULL;

    while ((opt = getopt(argc, argv, "vhc:r:")) != -1) {
        switch (opt) {
            case 'v':
                printf("%s\n", VERSION);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                printf("sxhkd [-h|-v|-c CONFIG_FILE|-r REDIR_FILE] [EXTRA_CONFIG ...]\n");
                exit(EXIT_SUCCESS);
                break;
            case 'r':
                redir_fd = open(optarg, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (redir_fd == -1)
                    warn("Failed to open the command redirection file.\n");
                break;
            case 'c':
                config_path = optarg;
                break;
        }
    }

    num_extra_confs = argc - optind;
    extra_confs = argv + optind;

    if (config_path == NULL) {
        char *config_home = getenv(CONFIG_HOME_ENV);
        if (config_home == NULL)
            err("The following environment variable is not defined: '%s'.\n", CONFIG_HOME_ENV); 
        else
            snprintf(config_file, sizeof(config_file), "%s/%s", config_home, CONFIG_PATH);
    } else {
        strncpy(config_file, config_path, sizeof(config_file));
    }

    signal(SIGINT, hold);
    signal(SIGHUP, hold);
    signal(SIGTERM, hold);
    signal(SIGUSR1, hold);

    setup();
    get_standard_keysyms();
    get_lock_fields();
    load_config(config_file);
    for (int i = 0; i < num_extra_confs; i++)
        load_config(extra_confs[i]);
    grab();

    xcb_generic_event_t *evt;
    int fd = xcb_get_file_descriptor(dpy);

    fd_set descriptors;

    reload = false;
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
                        motion_notify(evt, event_type);
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
        
        if (xcb_connection_has_error(dpy)) {
            warn("The server has closed the connection.\n");
            running = false;
        }
    }

    if (redir_fd != -1)
        close(redir_fd);
    ungrab();
    cleanup();
    xcb_key_symbols_free(symbols);
    xcb_disconnect(dpy);
    return EXIT_SUCCESS;
}
