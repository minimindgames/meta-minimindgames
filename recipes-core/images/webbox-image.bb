SUMMARY = "Webbox image"

LICENSE = "MIT"

inherit core-image

IMAGE_FEATURES += "splash package-management ssh-server-dropbear hwcodecs weston"

# Chromium browser
CORE_IMAGE_EXTRA_INSTALL += "chromium-ozone-wayland"

# Let systemd start WebbboxManager as service
CORE_IMAGE_EXTRA_INSTALL += "start-webbox"
CORE_IMAGE_EXTRA_INSTALL += "start-browser"

# Audio over HDMI
CORE_IMAGE_EXTRA_INSTALL += "pulseaudio pulseaudio-server"

# ethtool, for WakeOnLAN etc.
CORE_IMAGE_EXTRA_INSTALL += "ethtool"

# vlc video/music player
CORE_IMAGE_EXTRA_INSTALL += "vlc"

# systemd-firewalld
CORE_IMAGE_EXTRA_INSTALL += "firewalld"

CORE_IMAGE_EXTRA_INSTALL += "webbox-manager"
