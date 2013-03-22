## Description

sxhkd is a simple X hotkey daemon.

## Usage

### Synopsis

    sxhkd [OPTIONS] [EXTRA_CONFIG ...]

### Options

* `-h`: Print the synopsis to standard output and exit.
* `-v`: Print the version information to standard output and exit.
* `-c CONFIG_FILE`: Read the main configuration from the given file.
* `-r REDIR_FILE`: Redirect the commands output to the given file.

## Configuration

Each line of the configuration file is interpreted as so:
- If it starts with `#`, it is ignored.
- If it starts with one or more white space characters, it is read as a command.
- Otherwise, it is parsed as a hotkey: each key name is separated by spaces and/or `+` characters.

General syntax:

    [MODIFIER + ]*[@|!|:]KEYSYM
        COMMAND

Where `MODIFIER` is one of the following names: `super`, `hyper`, `meta`, `alt`, `control`, `ctrl`, `shift`, `mode_switch`, `lock`, `mod1`, `mod2`, `mod3`, `mod4`, `mod5`.

If `@` is added at the beginning of the keysym, the command will be run on key release events, otherwise on key press events.

If `!` is added at the beginning of the keysym, the command will be run on motion notify events and must contain two integer conversion specifications which will be replaced by the *x* and *y* coordinates of the pointer relative to the root window referential (the only valid button keysyms for this type of hotkeys are: `button1`, ..., `button5`).

If `:` is added at the beginning of the keysym, the captured event will be replayed for the other clients.

The keysym names are those your will get from `xev`.

Mouse hotkeys can be defined by using one of the following special keysym names: `button1`, `button2`, `button3`, ..., `button24`.

The hotkey can contain a sequence of the form `{STRING_1,…,STRING_N}`, in which case, the command must also contain a sequence with *N* elements: the pairing of the two sequences generates *N* hotkeys.

In addition, the sequences can contain ranges of the form `A-Z` where *A* and *Z* are alphanumeric characters.

What is actually executed is `SHELL -c COMMAND`, which means you can use environment variables in `COMMAND`.

`SHELL` will be the content of the first defined environment variable in the following list: `SXHKD_SHELL`, `SHELL`.

If *sxhkd* receives a `SIGUSR1` signal, it will reload its configuration file.

If no configuration file is specified through the `-c` option, the following is used: `$XDG_CONFIG_HOME/sxhkd/sxhkdrc`.

## Example Configuration

    XF86Audio{Prev,Next}
        mpc -q {prev,next}

    @XF86LaunchA
        scrot -s -e 'image_viewer $f'

    super + shift + equal
        sxiv -rt "$HOME/image"

    XF86LaunchB
        xdotool selectwindow | xsel -bi

    super + {h,j,k,l}
        bspc focus {left,down,up,right}

    super + alt + {0-9}
        mpc -q seek {0-9}0%

    super + {alt,ctrl,alt + ctrl} + XF86Eject
        sudo systemctl {suspend,reboot,poweroff}

    :button1
        bspc grab_pointer focus

    super + button{1,2,3}
        bspc grab_pointer {move,resize_side,resize_corner}

    super + !button{1,2,3}
        bspc {track_pointer,track_pointer,track_pointer} %i %i

    super + @button{1,2,3}
        bspc {ungrab_pointer,ungrab_pointer,ungrab_pointer}


## Installation

    make
    make install

## Dependencies

- libxcb
- xcb-util-keysyms
