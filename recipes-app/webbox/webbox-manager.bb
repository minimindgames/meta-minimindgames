SUMMARY = "Webbox Manager"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "\
        file://webbox-manager/ \
    "

S = "${WORKDIR}/webbox-manager"
#S = "${WORKDIR}"

DEPENDS += "vlc"
RDEPENDS_${PN} += "libvlc"

inherit cmake

EXTRA_OECMAKE = ""

do_install() {
    install -d ${D}${bindir}
    install -m 0755 WebboxManager ${D}${bindir}
    install -d ${D}${libdir}/webbox-manager
    cp -R ${S}/html ${D}${libdir}/webbox-manager
	
    ln -s /media/music ${D}${libdir}/webbox-manager/playlists
}
