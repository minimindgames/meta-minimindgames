#!/bin/sh

# Webbox looks playlist folders at /media/music
# Assume that music is on the last USB stick and mount it, e.g. Webbox may have been booted from USB sdb
USB="/dev/sd"
DIR="/media/music"
for x in e d c b
do
	DEV="${USB}${x}1"
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
