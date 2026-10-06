# swiftboot board support: Raspberry Pi 3, 64-bit (raspberrypi3-64)
#
# Requires meta-raspberrypi in bblayers.conf.
# Verified on real hardware: Raspberry Pi 3 Model B.

# ---------------------------------------------------------------------------
# Display
# ---------------------------------------------------------------------------
# Full KMS, NOT meta-raspberrypi's vc4-fkms-v3d default.
#
# With "fake" KMS the VideoCore firmware still owns the display controller and
# every mode-set is forwarded to it over a mailbox; Linux only presents a
# KMS-shaped interface. eglfs_kms wants Linux to own the hardware directly.
# Full KMS is confirmed working on this board.
VC4DTBO = "vc4-kms-v3d"

# ---------------------------------------------------------------------------
# Console
# ---------------------------------------------------------------------------
# Serial on GPIO 14/15 at 115200 8N1. Keep this enabled: with a silent boot
# and (optionally) no init system, there is no getty and no SSH, so a
# USB-serial adapter is the ONLY way to diagnose a board that does not come up.
ENABLE_UART = "1"

# ---------------------------------------------------------------------------
# Silent boot - power-on straight to the application
# ---------------------------------------------------------------------------
# Three different pieces of software draw something during boot, at three
# different times, and each needs its own switch:
#
#   rainbow test pattern  VideoCore firmware, BEFORE Linux exists
#                         -> only the firmware can suppress it (config.txt)
#   raspberry logos       kernel framebuffer logo, one per CPU core
#   "Booting ... please wait"  kernel console messages
#
# vt.global_cursor_default=0 removes the blinking text cursor that would
# otherwise sit on a black screen until Qt takes over the display.
DISABLE_SPLASH = "1"
DISABLE_RPI_BOOT_LOGO = "1"
CMDLINE_DEBUG = "quiet vt.global_cursor_default=0"

# ---------------------------------------------------------------------------
# Radios
# ---------------------------------------------------------------------------
# The Pi 3 onboard WiFi/BT firmware is proprietary Cypress/Synaptics, gated
# behind LICENSE_FLAGS = "synaptics-killswitch". Omitted here: this product
# uses ethernet and USB.
#
# To enable WiFi/BT instead, delete both lines below and add:
#   LICENSE_FLAGS_ACCEPTED += "synaptics-killswitch"
MACHINE_FEATURES:remove = "wifi bluetooth"
# raspberrypi3-64.conf adds the firmware unconditionally through
# MACHINE_EXTRA_RRECOMMENDS, independently of MACHINE_FEATURES, so removing
# the features above is not sufficient on its own - the build still fails with
# "Nothing RPROVIDES linux-firmware-rpidistro-bcm43430".
MACHINE_EXTRA_RRECOMMENDS:remove = "linux-firmware-rpidistro-bcm43430 linux-firmware-rpidistro-bcm43455 bluez-firmware-rpidistro-bcm43430a1-hcd bluez-firmware-rpidistro-bcm4345c0-hcd"
