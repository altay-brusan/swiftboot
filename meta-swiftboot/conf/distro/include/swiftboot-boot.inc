# swiftboot: boot and init policy. Board-agnostic.

# sysvinit rather than systemd - smaller, and it starts fewer things.
# An image may go further and bypass init entirely with init=<launcher> on the
# kernel cmdline; see meta-dashboard for a worked example.
INIT_MANAGER = "sysvinit"

# ---------------------------------------------------------------------------
# Distro features
# ---------------------------------------------------------------------------
# x11 + wayland are removed for two reasons. The obvious one is boot time and
# image size. The non-obvious one matters more: meta-qt6 chooses between the
# X11 stack and the embedded stack purely from DISTRO_FEATURES
# (qtbase_git.bb lines 55/59/94), and Qt's own configure REFUSES to enable
# gles2 while desktop GL is on:
#     Feature "opengles2": Forcing to "ON" breaks its condition
#     NOT QT_FEATURE_opengl_desktop AND GLESv2_FOUND
# So dropping x11 is not an optimisation - it is what makes eglfs possible.
#
# ptest is a poky default (POKY_DEFAULT_DISTRO_FEATURES) that makes every
# ptest-enabled recipe compile its unit-test suite. qtbase alone spent 40+
# minutes building tests that would never run on an appliance.
DISTRO_FEATURES:remove = "x11 wayland ptest systemd"

# Install only what is explicitly required.
#
# SHARP EDGE: this also drops RRECOMMENDS, which is how a surprising amount of
# Yocto wires things together. Two consequences seen in practice:
#   - init-ifupdown is only an RRECOMMENDS of packagegroup-core-boot, so
#     without naming it explicitly there is no /etc/network/interfaces and no
#     networking at all.
#   - anything dlopen'd (Qt platform plugins, QML modules, Mesa DRI drivers)
#     is invisible to the dependency scanner AND gets no help here, so it must
#     be listed in RDEPENDS by hand.
NO_RECOMMENDATIONS = "1"
