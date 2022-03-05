#FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"
#FILESEXTRAPATHS:prepend := "${@ '${THISDIR}/${PN}:' if d.getVar('WPA_PSK') else ''}"

#SRC_URI:append = " file://wpa_supplicant.conf-sane"

#SYSTEMD_AUTO_ENABLE = "enable"
