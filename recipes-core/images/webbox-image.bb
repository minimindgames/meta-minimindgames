SUMMARY = "Webbox image"

LICENSE = "MIT"

inherit core-image

IMAGE_FEATURES += "splash package-management ssh-server-dropbear hwcodecs weston"

# Chromium browser
CORE_IMAGE_EXTRA_INSTALL += "chromium-ozone-wayland"

# Start-up app by systemd
CORE_IMAGE_EXTRA_INSTALL += "startapp"

# Audio over HDMI
CORE_IMAGE_EXTRA_INSTALL += "pulseaudio pulseaudio-server"

# ethtool, for WakeOnLAN etc.
CORE_IMAGE_EXTRA_INSTALL += "ethtool"
