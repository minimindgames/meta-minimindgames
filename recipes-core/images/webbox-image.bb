SUMMARY = "A web game image with Chromium browser and Qt"

IMAGE_FEATURES += "splash package-management ssh-server-dropbear hwcodecs weston"

LICENSE = "MIT"

inherit core-image

# Qemu boot memory
QB_MEM = "-m 1024"

CORE_IMAGE_EXTRA_INSTALL += "chromium-ozone-wayland startapp"
