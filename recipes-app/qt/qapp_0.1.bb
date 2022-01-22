SUMMARY = "Qapp recipe"
DESCRIPTION = "GUI app based on Qt"

# Check appropriate licenses from Qt
LICENSE = "CLOSED"

python do_display_banner() {
    bb.plain("*****************************************");
    bb.plain("*  Building meta-minimindgames/qapp     *");
    bb.plain("*****************************************");
}

addtask display_banner before do_build

SRC_URI = "file://qapp.pro \
           file://qapp.cpp"

DEPENDS += "qtbase" 

RDEPENDS_${PN} += "qtwayland"

S = "${WORKDIR}"

inherit qt6-qmake

do_install() {
    install -d ${D}${bindir}
    install -m 0755 qapp ${D}${bindir}
}
