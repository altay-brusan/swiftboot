# swiftboot: graphics policy. Board-agnostic.
#
# The target is a single fullscreen application rendering straight to
# DRM/KMS - no X server, no Wayland compositor, no window manager.
#
# The chain, top to bottom:
#
#   QML / Qt Quick scene graph
#        -> Qt QPA plugin "eglfs"            (PACKAGECONFIG eglfs)
#        -> eglfs_kms backend                (PACKAGECONFIG kms)
#        -> OpenGL ES 2 + EGL                (PACKAGECONFIG gles2)
#        -> GBM, buffers both the GPU and the display can use
#                                            (PACKAGECONFIG gbm)
#        -> libdrm -> /dev/dri/card0
#        -> DRM kernel subsystem: KMS half (display) + GEM half (3D)
#        -> board GPU driver (vc4 on Pi, etc.)
#
# Four PACKAGECONFIG options because they are four distinct links in that
# chain; enabling "eglfs" alone gives a plugin with no way to reach the
# display. Note meta-qt6 derives all of them automatically once x11 is absent
# from DISTRO_FEATURES (see swiftboot-boot.inc), so they are not set here.

QT_QPA_DEFAULT_PLATFORM = "eglfs"
