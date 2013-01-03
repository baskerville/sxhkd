## Description

sxhkd is a simple X hotkey daemon.

## Usage

### Synopsis

    sxhkd [OPTIONS]

### Options

* `-h`: Print the synopsis to standard output and exit.
* `-v`: Print the version information to standard output and exit.
* `-c CONFIG_FILE`: Read the configuration from the given file.

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

What is actually executed is `/bin/sh -c COMMAND`, which means you can use environment variables in `COMMAND`.

If *sxhkd* receives a `SIGUSR1` signal, it will reload its configuration file.

If no configuration file is specified through the `-c` option, the following is used: `$XDG_CONFIG_HOME/sxhkd/sxhkdrc`.

## Installation

    make
    make install

## Dependencies

- libxcb
- xcb-util-keysyms
