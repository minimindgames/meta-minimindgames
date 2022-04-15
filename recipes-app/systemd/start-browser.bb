SUMMARY = "Start browser"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# see https://docs.yoctoproject.org/ref-manual/classes.html
inherit systemd features_check
SYSTEMD_SERVICE:${PN} = "start-browser.service"

SRC_URI = " \
    file://start-browser.service \
    file://start-browser.sh \
"

# bindir and paths come from meta/conf/bitbake.conf
FILES:${PN} += "${systemd_system_unitdir}/start-browser.service"

do_install() {
    install -d ${D}${bindir} 
    install -m 0755 ${WORKDIR}/start-browser.sh ${D}${bindir}
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/start-browser.service ${D}${systemd_system_unitdir}
}

# required by meta/classes/systemd.bbclass
REQUIRED_DISTRO_FEATURES= "systemd"
