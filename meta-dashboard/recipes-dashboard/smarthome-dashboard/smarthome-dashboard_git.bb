SUMMARY = "Smart Home Dashboard in Qt 6 QML"
DESCRIPTION = "A simple smart home dashboard built with Qt 6 QML"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "git://github.com/allankoechke/SmartHomeDashboardNew.git;protocol=https;branch=master \
           file://smarthome-dashboard.init \
           file://dashboard-launcher \
"

# Pinned deliberately. AUTOREV re-polls GitHub on every parse, defeats sstate
# reuse and makes the image unreproducible - a different dashboard each build.
# Bump this when you actually want the new upstream commit.
SRCREV = "1274e0045705207ce7e8b5fec9a8a28a4e32a968"

S = "${WORKDIR}/git"

DEPENDS += "qtbase qtdeclarative qtdeclarative-native"

inherit qt6-cmake update-rc.d

# Start first (00): ahead of S01networking, whose DHCP wait was the single
# biggest userspace cost. The dashboard needs no network to draw its first
# frame; networking still comes up, just behind the UI instead of in front.
# Measured: DRM ready ~8.8s, init starts ~12.8s, so the GPU always exists by
# the time rc scripts run - the init script's /dev/dri/card0 poll is now just
# insurance rather than a necessity.
#
# This is the CONVENTIONAL boot path. The image also ships
# ${bindir}/dashboard-launcher, which replaces sysvinit entirely when selected
# with init=/usr/bin/dashboard-launcher on the kernel cmdline. Both live in the
# same image on purpose, so the fast path can be tested without giving up a
# known-good bootable system.
INITSCRIPT_NAME = "smarthome-dashboard"
INITSCRIPT_PARAMS = "start 00 2 3 4 5 . stop 20 0 1 6 ."

do_install:append() {
    install -d ${D}${bindir} ${D}${sysconfdir}/init.d

    install -m 0755 ${WORKDIR}/dashboard-launcher \
        ${D}${bindir}/dashboard-launcher

    install -m 0755 ${WORKDIR}/smarthome-dashboard.init \
        ${D}${sysconfdir}/init.d/${INITSCRIPT_NAME}
}

FILES:${PN} += "${sysconfdir}/init.d/${INITSCRIPT_NAME}"

# The QML runtime is loaded at runtime via dlopen, so it is invisible to the
# shared-library dependency scanner and has to be named explicitly. Doubly so
# here because local.conf sets NO_RECOMMENDATIONS = "1", which means nothing
# arrives via RRECOMMENDS.
RDEPENDS:${PN} += " \
    qtbase-plugins \
    qtdeclarative-qmlplugins \
    mesa-megadriver \
"
