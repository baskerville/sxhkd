## Description

sxhkd is a simple X hotkey daemon.

## Usage

### Synopsis

    sxhkd [OPTIONS] [EXTRA_CONFIG ...]

### Options

* `-h`: Print the synopsis to standard output and exit.
* `-v`: Print the version information to standard output and exit.
* `-c CONFIG_FILE`: Read the main configuration from the given file.

## Configuration

Each line of the configuration file is interpreted as so:
- If it starts with `#`, it is ignored.
- If it starts with one or more white space characters, it is read as a command.
- Otherwise, it is parsed as a hotkey: each key name is separated by spaces and/or `+` characters.

General syntax:

    [MODIFIER + ]*[@]KEYSYM
        COMMAND

Where `MODIFIER` is one of the following names: `super`, `hyper`, `meta`, `alt`, `control`, `ctrl`, `shift`, `mode_switch`, `lock`, `mod1`, `mod2`, `mod3`, `mod4`, `mod5`.

If `@` is added at the beginning of the keysym, the command will be run on key release events, otherwise on key press events.

The keysym names are those your will get from `xev` (minus the prefix if any).

Mouse hotkeys can be defined by using one of the following special keysym names: `button1`, `button2`, `button3`, ..., `button24`.

`KEYSYM` can contain a sequence of the form `{STRING_1,…,STRING_N}`, in which case, `COMMAND` must also contain a sequence with *N* elements: the pairing of the two sequences generates *N* hotkeys.

In addition, the sequences can contain ranges of the form `A-Z` where *A* and *Z* are alphanumeric characters.

What is actually executed is `SHELL -c COMMAND`, which means you can use environment variables in `COMMAND`.

`SHELL` will be the content of the first defined environment variable in the following list: `SXHKD_SHELL`, `SHELL`.

If *sxhkd* receives a `SIGUSR1` signal, it will reload its configuration file.

If no configuration file is specified through the `-c` option, the following is used: `$XDG_CONFIG_HOME/sxhkd/sxhkdrc`.

## Installation

    make
    make install

## Dependencies

- libxcb
- xcb-util-keysyms
