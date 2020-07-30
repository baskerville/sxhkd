## Description

*sxhkd* is an X daemon that reacts to input events by executing commands.

Its configuration file is a series of bindings that define the associations between the input events and the commands.

The format of the configuration file supports a simple notation for mapping multiple shortcuts to multiple commands in parallel.

## Example Bindings

	XF86Audio{Prev,Next}
		mpc -q {prev,next}

	@XF86LaunchA
		scrot -s -e 'image_viewer $f'

	super + shift + equal
		sxiv -rt "$HOME/image"

	XF86LaunchB
		xdotool selectwindow | xsel -bi

	super + {h,j,k,l}
		bspc node -f {west,south,north,east}

	super + alt + {0-9}
		mpc -q seek {0-9}0%

	super + {alt,ctrl,alt + ctrl} + XF86Eject
		sudo systemctl {suspend,reboot,poweroff}

	super + {_,shift + }{h,j,k,l}
		bspc node -{f,s} {west,south,north,east}

	{_,shift + ,super + }XF86MonBrightness{Down,Up}
		bright {-1,-10,min,+1,+10,max}

	super + o ; {e,w,m}
		{gvim,firefox,thunderbird}

	super + alt + control + {h,j,k,l} ; {0-9}
		bspc node @{west,south,north,east} -r 0.{0-9}

	super + alt + p
		bspc config focus_follows_pointer {true,false}

## Macros

The configuration file _may_ contain a set of macros to ease bindings
definitions. A macro is defined using the keyword `DEF` as follows:

    # A general prefix key for launching applications
    DEF APP_KEY super + x

    # Rofi shortned.
    DEF ROFI rofi -matching fuzzy -modi 'window,windowcd,run,drun'
    DEF MAKE_TIMESTAMP  $(date +%Y-%m-%d_%H%M%S)

    # Icons
    DEF ICONS_PATH /usr/share/icons/Papirus-Dark/48x48
    DEF SCROT_ICON {{ICONS_PATH}}/apps/screengrab.svg

In the examples above, `ROFI` is a macro, and its replacement text is `rofi
-matching fuzzy -modi 'window,windowcd,run,drun'`. Now, we can use `{{ROFI}}`
anywhere in the configuration file and it will be replaced with the replacement
text. 

Note also how we have defined a common icons path using the macro `ICONS_PATH`
and how we have used it to define `SCROT_ICON`. In fact, macros may be defined
using previously defined macros, and so on.

Using these macros, we can define a binding to take screen shots (using scrot)
as follows:

    {{APP_KEY}} ; p
        stamp={{MAKE_TIMESTAMP}};\
        filename="/tmp/screen_$stamp.png";\
        scrot $filename;\
        notify-send -a 'Scrot' "Screenshot taken in $filename" -i {{SCROT_ICON}};

Macros may essentially be used for modularity (See the icons macros above), to
avoid typing (eg. ROFI), and to define global variables.

## Editor Plugins

### Vim

- [vim-sxhkdrc](https://github.com/baskerville/vim-sxhkdrc).
- [sxhkd-vim](https://github.com/kovetskiy/sxhkd-vim).

----

For further information, check the `man` pages.
