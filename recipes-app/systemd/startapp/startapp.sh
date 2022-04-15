#!/bin/sh
if test -z "$XDG_RUNTIME_DIR"; then
    export XDG_RUNTIME_DIR=/run/user/`id -u`
    if ! test -d "$XDG_RUNTIME_DIR"; then
        mkdir --parents $XDG_RUNTIME_DIR
        chmod 0700 $XDG_RUNTIME_DIR
    fi
fi

# wait for weston
while [ ! -e  $XDG_RUNTIME_DIR/wayland-0 ] ; do sleep 0.1; done
sleep 1

export DISPLAY=:0.0

# Webbox looks playlist folders at /media/music
# Assume that music is on the last USB stick and mount it, e.g. Webbox may have been booted from USB sdb
USB="/dev/sd"
DIR="/media/music"
for x in e d c b
do
	DEV="$USB$x"
	if [ -e $DEV ]; then
		mkdir -p $DIR
		if [ ! -e "/usr/lib/webbox-manager/playlists" ]; then
			ln -s $DIR "/usr/lib/webbox-manager/playlists"
		fi
		mount $DEV $DIR
		break
	fi
done

/usr/bin/WebboxManager &
/usr/bin/chromium & # maybe with "--start-fullscreen --start-maximized"
