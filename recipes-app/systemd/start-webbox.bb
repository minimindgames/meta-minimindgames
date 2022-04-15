SUMMARY = "Start webbox-manager"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# see https://docs.yoctoproject.org/ref-manual/classes.html
inherit systemd features_check
SYSTEMD_SERVICE:${PN} = "start-webbox.service"

SRC_URI = " \
    file://start-webbox.service \
    file://start-webbox.sh \
"

# bindir and paths come from meta/conf/bitbake.conf
FILES:${PN} += "${systemd_system_unitdir}/start-webbox.service"

do_install() {
    install -d ${D}${bindir} 
    install -m 0755 ${WORKDIR}/start-webbox.sh ${D}${bindir}
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/start-webbox.service ${D}${systemd_system_unitdir}
}

# required by meta/classes/systemd.bbclass
REQUIRED_DISTRO_FEATURES= "systemd"
