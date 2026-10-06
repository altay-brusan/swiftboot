# swiftboot board support: BeagleBone Black (beaglebone-yocto)
#
# STATUS: UNTESTED STUB. Nothing here has been run on hardware. It exists to
# show where BeagleBone settings go and to record what is already known to be
# different, so the first bring-up starts from facts rather than guesses.
#
# beaglebone-yocto ships in poky (meta-yocto-bsp), so no extra layer is needed
# for a basic build. meta-ti is required for anything GPU-related.

# ---------------------------------------------------------------------------
# Architecture - read this before building
# ---------------------------------------------------------------------------
# BeagleBone Black is AM335x: ARMv7-A Cortex-A8, 32-bit.
#   beaglebone-yocto -> DEFAULTTUNE "cortexa8hf-neon"
# The Raspberry Pi 3 build is aarch64. Switching between them rebuilds
# essentially everything - different architecture, different sysroots. Nothing
# is shared but host-native tooling.
#
# BeagleBone AI-64 and BeaglePlay are aarch64 and need meta-ti; they are
# different machines and would get their own file here.

# ---------------------------------------------------------------------------
# Display - the hard part
# ---------------------------------------------------------------------------
# The AM335x GPU is a PowerVR SGX530. There is no open-source 3D driver: GLES
# requires TI's proprietary userspace from meta-ti, which is tied to specific
# kernel versions.
#
# Consequences for a Qt eglfs image:
#   - WITHOUT meta-ti: no GLES. Qt Quick falls back to software rendering and
#     a QML dashboard will be slow. Consider QT_QPA_PLATFORM=linuxfb, which
#     needs only /dev/fb0 and no GPU at all.
#   - WITH meta-ti: SGX driver plus matching kernel; eglfs_kms may work over
#     the tilcdc DRM driver, but this combination is not verified here.
#
# Decide which of those you want before bring-up; it changes the image.
# QT_QPA_DEFAULT_PLATFORM = "linuxfb"

# ---------------------------------------------------------------------------
# Console
# ---------------------------------------------------------------------------
# beaglebone-yocto already sets SERIAL_CONSOLES for the J1 header
# (ttyS0/ttyO0 at 115200). Same reasoning as the Pi: keep a serial console,
# because a silent boot with no init system has no other diagnostic path.

# ---------------------------------------------------------------------------
# Not applicable from the Pi board file
# ---------------------------------------------------------------------------
# VC4DTBO, DISABLE_SPLASH, DISABLE_RPI_BOOT_LOGO and ENABLE_UART are
# meta-raspberrypi variables and mean nothing here. The equivalent of the
# silent-boot cmdline additions is:
#   APPEND:append = " quiet logo.nologo vt.global_cursor_default=0"
# (poky machines have no rpi-cmdline.bb, so CMDLINE_DEBUG does not exist.)
