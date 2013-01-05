#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <ctype.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include "helpers.h"
#include "keys.h"
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
    /* Makes key repeat events only send key press events */
    /* xcb_xkb_per_client_flags_reply_t *reply = xcb_xkb_per_client_flags_reply(dpy, xcb_xkb_per_client_flags(dpy, XCB_XKB_ID_USE_CORE_KBD, XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 0, 0, 0), NULL); */
    /* if (reply == NULL) */
    /*     warn("couldn't set detectable auto repeat\n"); */
    /* free(reply); */
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

void load_config(void)
{
    PUTS("load configuration");
    if (hotkeys != NULL) {
        cleanup();
        hotkeys = NULL;
    }

    char path[MAXLEN];
    if (config_file == NULL)
        snprintf(path, sizeof(path), "%s/%s", getenv("XDG_CONFIG_HOME"), CONFIG_PATH);
    else
        strncpy(path, config_file, sizeof(path));

    FILE *cfg = fopen(path, "r");
    if (cfg == NULL)
        err("Can't open configuration file.\n");

    char line[MAXLEN];
    xcb_keysym_t keysym = XCB_NO_SYMBOL;
    xcb_button_t button = XCB_NONE;
    uint16_t modfield = 0;
    xcb_event_mask_t event_mask = XCB_KEY_PRESS;
    char keysym_seq[MAXLEN] = {'\0'};

    while (fgets(line, sizeof(line), cfg) != NULL) {
        if (strlen(line) < 2 || line[0] == START_COMMENT) {
            continue;
        } else if (isspace(line[0])) {
            if (keysym == XCB_NO_SYMBOL && button == XCB_NONE && strlen(keysym_seq) == 0)
                continue;
            unsigned int i = strlen(line) - 1;
            while (i > 0 && isspace(line[i]))
                line[i--] = '\0';
            i = 1;
            while (i < strlen(line) && isspace(line[i]))
                i++;
            if (i < strlen(line)) {
                char *command = line + i;
                if (strlen(keysym_seq) == 0)
                    generate_hotkeys(keysym, button, modfield, event_mask, command);
                else
                    unfold_hotkeys(keysym_seq, modfield, event_mask, command);
            }
            keysym = XCB_NO_SYMBOL;
            button = XCB_NONE;
            modfield = 0;
            event_mask = XCB_KEY_PRESS;
            keysym_seq[0] = '\0';
        } else {
            char *name = strtok(line, TOK_SEP);
            if (name == NULL)
                continue;
            do {
                if (name[0] == RELEASE_PREFIX) {
                    event_mask = XCB_KEY_RELEASE;
                    name++;
                }
                if (!parse_modifier(name, &modfield) && !parse_key(name, &keysym) && !parse_button(name, &button) && !(sscanf(name, SEQ_BEGIN"%s"SEQ_END, keysym_seq) == 1)) {
                    warn("Unrecognized key name: '%s'.\n", name);
                }
            } while ((name = strtok(NULL, TOK_SEP)) != NULL);
        }
    }

    fclose(cfg);
}

void mapping_notify(xcb_generic_event_t *evt)
{
    if (!running || reload)
        return;
    xcb_mapping_notify_event_t *e = (xcb_mapping_notify_event_t *) evt;
    PRINTF("mapping notify %u %u\n", e->request, e->count);
    if (e->request == XCB_MAPPING_KEYBOARD || e->request == XCB_MAPPING_MODIFIER) {
        /* PUTS("refreshing everything"); */
        /* xcb_refresh_keyboard_mapping(symbols, e); */
        /* get_lock_fields(); */
        /* load_config(); */
        /* ungrab(); */
        /* grab(); */
    }
}

void key_button_event(xcb_generic_event_t *evt, xcb_event_mask_t event_mask)
{
    xcb_keysym_t keysym = XCB_NO_SYMBOL;
    xcb_keycode_t keycode = XCB_NONE;
    xcb_button_t button = XCB_NONE;
    uint16_t modfield = 0;
    uint16_t lockfield = num_lock | caps_lock | scroll_lock;
    if (event_mask == XCB_KEY_PRESS) {
        xcb_key_press_event_t *e = (xcb_key_press_event_t *) evt;
        keycode = e->detail;
        modfield = e->state;
        keysym = xcb_key_symbols_get_keysym(symbols, keycode, 0);
        PRINTF("key press %u %u\n", keycode, modfield);
    } else if (event_mask == XCB_KEY_RELEASE) {
        xcb_key_release_event_t *e = (xcb_key_release_event_t *) evt;
        keycode = e->detail;
        modfield = e->state;
        keysym = xcb_key_symbols_get_keysym(symbols, keycode, 0);
        PRINTF("key release %u %u\n", keycode, modfield);
    } else if (event_mask == XCB_BUTTON_PRESS) {
        xcb_button_press_event_t *e = (xcb_button_press_event_t *) evt;
        button = e->detail;
        modfield = e->state;
        PRINTF("button press %u %u\n", button, modfield);
    } else if (event_mask == XCB_BUTTON_RELEASE) {
        xcb_button_release_event_t *e = (xcb_button_release_event_t *) evt;
        button = e->detail;
        modfield = e->state;
        PRINTF("button release %u %u\n", button, modfield);
    }
    modfield &= ~lockfield & MOD_STATE_FIELD;
    if (keysym != XCB_NO_SYMBOL || button != XCB_NONE) {
        hotkey_t *hk = find_hotkey(keysym, button, modfield, event_mask);
        if (hk != NULL) {
            char *cmd[] = {SHELL, "-c", hk->command, NULL};
            spawn(cmd);
        }
    }
}

int main(int argc, char *argv[])
{
    char opt;
    config_file = NULL;

    while ((opt = getopt(argc, argv, "vhc:")) != -1) {
        switch (opt) {
            case 'v':
                printf("%s\n", VERSION);
                exit(EXIT_SUCCESS);
                break;
            case 'h':
                printf("sxhkd [-h|-v|-c CONFIG_FILE]\n");
                exit(EXIT_SUCCESS);
                break;
            case 'c':
                config_file = optarg;
                break;
        }
    }

    signal(SIGINT, hold);
    signal(SIGHUP, hold);
    signal(SIGTERM, hold);
    signal(SIGUSR1, hold);

    setup();
    get_lock_fields();
    load_config();
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
                uint8_t event_mask = XCB_EVENT_RESPONSE_TYPE(evt);
                switch (event_mask) {
                    case XCB_KEY_PRESS:
                    case XCB_KEY_RELEASE:
                    case XCB_BUTTON_PRESS:
                    case XCB_BUTTON_RELEASE:
                        key_button_event(evt, event_mask);
                        break;
                    case XCB_MAPPING_NOTIFY:
                        mapping_notify(evt);
                        break;
                    default:
                        PRINTF("unknown event %u\n", event_mask);
                        break;
                }
                free(evt);
            }
        }

        if (reload) {
            PUTS("reload configuration");
            signal(SIGUSR1, hold);
            load_config();
            ungrab();
            grab();
            reload = false;
        }
        
        if (xcb_connection_has_error(dpy)) {
            warn("One of the previous requests failed.\n");
            running = false;
        }
    }

    ungrab();
    cleanup();
    xcb_key_symbols_free(symbols);
    xcb_disconnect(dpy);
    return EXIT_SUCCESS;
}
