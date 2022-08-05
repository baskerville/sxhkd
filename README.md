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

	# Smart resize, will grow or shrink depending on location.
	# Will always grow for floating nodes.
	super + ctrl + alt + {Left,Down,Up,Right}
	  n=10; \
	  { d1=left;   d2=right;  dx=-$n; dy=0;   \
	  , d1=bottom; d2=top;    dx=0;   dy=$n;  \
	  , d1=top;    d2=bottom; dx=0;   dy=-$n; \
	  , d1=right;  d2=left;   dx=$n;  dy=0;   \
	  } \
	  bspc node --resize $d1 $dx $dy || bspc node --resize $d2 $dx $dy

## Editor Plugins

### Vim
- [vim-sxhkdrc](https://github.com/baskerville/vim-sxhkdrc).
- [sxhkd-vim](https://github.com/kovetskiy/sxhkd-vim).

### VS Code
- [sxhkdrc-syntax](https://github.com/mosbasik/sxhkdrc-syntax).

### Emacs
- [sxhkd-mode](https://github.com/xFA25E/sxhkd-mode)

----

For further information, check the `man` pages.
