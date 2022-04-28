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
		if grep -qs $DIR /proc/mounts; then
			echo "Webbox playlists mounted at ${DIR}"
			mount $DEV $DIR
		fi
		break
	fi
done

mkdir -m 777 -p $HOME/.webbox
/usr/bin/WebboxManager &
