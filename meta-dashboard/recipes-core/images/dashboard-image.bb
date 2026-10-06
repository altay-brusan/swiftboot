SUMMARY = "Fast-Boot Qt6 Dashboard Image"
DESCRIPTION = "Minimal Linux image optimized for fast boot with Qt6 EGLFS and networking support."
LICENSE = "MIT"

inherit core-image

# Must be += and not a hard assignment: "IMAGE_FEATURES = ..." discards the
# debug-tweaks that local.conf sets, which leaves root with a locked password
# and no way in over either dropbear or the serial console.
# Drop debug-tweaks once you are past bring-up and provision a real credential.
IMAGE_FEATURES += " \
    ssh-server-dropbear \
    debug-tweaks \
    read-only-rootfs \
"

# core-image.bbclass already seeds IMAGE_INSTALL with packagegroup-core-boot,
# so append rather than listing it again.
IMAGE_INSTALL:append = " \
    smarthome-dashboard \
    qtbase-plugins \
    qtdeclarative \
    qtdeclarative-qmlplugins \
    mesa-megadriver \
    init-ifupdown \
    iproute2 \
"

# init-ifupdown is only an RRECOMMENDS of packagegroup-core-boot, and
# NO_RECOMMENDATIONS = "1" in local.conf drops it. Without it there is no
# /etc/network/interfaces, so eth0 never comes up and you get no networking at
# all - hence the explicit entry above.
#
# Dropped from the previous revision:
#   iw, wpa-supplicant - Wi-Fi tooling. QEMU gives you virtio-net and the
#                        BeagleBone Black has no Wi-Fi, so these were dead
#                        weight on an image that is meant to boot fast.
#   dhcpcd             - wants a writable /var/db/dhcpcd, which fights
#                        read-only-rootfs. init-ifupdown drives busybox udhcpc
#                        instead and writes only to tmpfs.
