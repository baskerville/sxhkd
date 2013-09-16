#include <stdlib.h>
#include "types.h"
#include "parse.h"
#include "grab.h"

void grab(void)
{
    for (hotkey_t *hk = hotkeys; hk != NULL; hk = hk->next)
        grab_chord(hk->chain->head);
}

void grab_chord(chord_t *chord)
{
    for (chord_t *c = chord; c != NULL; c = c->more) {
        if (c->button == XCB_NONE) {
            xcb_keycode_t *keycodes = keycodes_from_keysym(c->keysym);
            if (keycodes != NULL)
                for (xcb_keycode_t *kc = keycodes; *kc != XCB_NO_SYMBOL; kc++)
                    if (c->keysym == xcb_key_symbols_get_keysym(symbols, *kc, 0))
                        grab_key_button(*kc, c->button, c->modfield);
            free(keycodes);
        } else {
            grab_key_button(XCB_NONE, c->button, c->modfield);
        }
    }
}

void grab_key_button(xcb_keycode_t keycode, xcb_button_t button, uint16_t modfield)
{
    grab_key_button_checked(keycode, button, modfield);
    if (num_lock != 0)
        grab_key_button_checked(keycode, button, modfield | num_lock);
    if (caps_lock != 0)
        grab_key_button_checked(keycode, button, modfield | caps_lock);
    if (scroll_lock != 0)
        grab_key_button_checked(keycode, button, modfield | scroll_lock);
    if (num_lock != 0 && caps_lock != 0)
        grab_key_button_checked(keycode, button, modfield | num_lock | caps_lock);
    if (caps_lock != 0 && scroll_lock != 0)
        grab_key_button_checked(keycode, button, modfield | caps_lock | scroll_lock);
    if (num_lock != 0 && scroll_lock != 0)
        grab_key_button_checked(keycode, button, modfield | num_lock | scroll_lock);
    if (num_lock != 0 && caps_lock != 0 && scroll_lock != 0)
        grab_key_button_checked(keycode, button, modfield | num_lock | caps_lock | scroll_lock);
}

void grab_key_button_checked(xcb_keycode_t keycode, xcb_button_t button, uint16_t modfield)
{
    xcb_generic_error_t *err;
    if (button == XCB_NONE)
        err = xcb_request_check(dpy, xcb_grab_key_checked(dpy, true, root, modfield, keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_SYNC));
    else
        err = xcb_request_check(dpy, xcb_grab_button_checked(dpy, true, root, XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, button, modfield));
    unsigned int value = (button == XCB_NONE ? keycode : button);
    char *type = (button == XCB_NONE ? "key" : "button");
    if (err != NULL) {
        warn("Could not grab %s %u with modfield %u: ", type, value, modfield);
        if (err->error_code == XCB_ACCESS)
            warn("the combination is already grabbed.\n");
        else
            warn("error %u encountered.\n", err->error_code);
        free(err);
    } else {
        PRINTF("grab %s %u %u\n", type, value, modfield);
    }
}

void ungrab(void)
{
    PUTS("ungrab");
    xcb_ungrab_key(dpy, XCB_GRAB_ANY, root, XCB_BUTTON_MASK_ANY);
    xcb_ungrab_button(dpy, XCB_BUTTON_INDEX_ANY, root, XCB_MOD_MASK_ANY);
}
