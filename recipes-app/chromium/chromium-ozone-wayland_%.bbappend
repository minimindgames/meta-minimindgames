# See https://github.com/OSSystems/meta-browser/tree/master/meta-chromium
FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
PACKAGECONFIG = "use-egl"
# Use "use-egl kiosk-mode" for fullscreen no bars etc.
