% Sxhkd User's Guide
% Bastien Dejean
% June 23, 2013

# Synopsis

**sxhkd** [*OPTIONS*] [*EXTRA_CONFIG* …]

# Description

sxhkd is a simple X hotkey daemon with a powerful and compact configuration syntax.

# Options

`-h`
:    Print the synopsis to standard output and exit.

`-v`
:    Print the version information to standard output and exit.

`-t` *TIMEOUT*
:    Timeout in seconds for the recording of chord chains.

`-c` *CONFIG_FILE*
:    Read the main configuration from the given file.

`-r` *REDIR_FILE*
:    Redirect the commands output to the given file.

# Configuration

Each line of the configuration file is interpreted as so:

- If it is empty or starts with `#`, it is ignored.

- If it starts with a space, it is read as a command.

- Otherwise, it is read as a hotkey.

General syntax:

    HOTKEY
        COMMAND

    HOTKEY := CHORD_1 ; CHORD_2 ; … ; CHORD_n
    CHORD_i := [MODIFIER_i][@|!|:]KEYSYM_i
    MODIFIER_i := MODIFIER_i1 + MODIFIER_i2 + … + MODIFIER_ik

The valid modifier names are: *super*, *hyper*, *meta*, *alt*, *control*, *ctrl*, *shift*, *mode_switch*, *lock*, *mod1*, *mod2*, *mod3*, *mod4* and *mod5*.

The keysym names are given by the output of **xev**.

Hotkeys and commands can be spread across multiple lines by ending each partial line with a backslash character.

When multiple chords are separated by semicolons, the hotkey is a chord chain: the command will only be executed after receiving each chord of the chain in consecutive order.

If **@** is added at the beginning of the keysym, the command will be run on key release events, otherwise on key press events.

If **!** is added at the beginning of the keysym, the command will be run on motion notify events and must contain two integer conversion specifications which will be replaced by the *x* and *y* coordinates of the pointer relative to the root window referential (the only valid button keysyms for this type of hotkeys are: *button1*, …, *button5*).

If **:** is added at the beginning of the keysym, the captured event will be replayed for the other clients.

Mouse hotkeys can be defined by using one of the following special keysym names: *button1*, *button2*, *button3*, …, *button24*.

The hotkey and the command may contain sequences of the form **{*STRING\_1*,*…*,*STRING\_N*}**.

In addition, the sequences can contain ranges of the form *A*-*Z* where *A* and *Z* are alphanumeric characters.

The underscore character represents an empty sequence element.

What is actually executed is **SHELL** *-c* **COMMAND**, which means you can use environment variables in **COMMAND**.

**SHELL** will be the content of the first defined environment variable in the following list: **SXHKD_SHELL**, **SHELL**.

If **sxhkd** receives a *SIGUSR1* signal, it will reload its configuration file.

If no configuration file is specified via the *-c* option, the following is used: *$XDG_CONFIG_HOME/sxhkd/sxhkdrc*.
