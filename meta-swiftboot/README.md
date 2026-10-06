# meta-swiftboot

The OS layer: distro policy for a Linux system that boots to one fullscreen
application as fast as the hardware allows.

Board-agnostic. Contains no application, and nothing specific to any product.

## Layout

```
conf/
├── layer.conf
└── distro/
    ├── swiftboot.conf                  the distro
    └── include/
        ├── swiftboot-boot.inc          init system, DISTRO_FEATURES
        ├── swiftboot-graphics.inc      eglfs / KMS policy
        └── board/
            ├── raspberrypi3-64.inc     verified on hardware
            └── beaglebone-yocto.inc    untested stub
recipes-qt/qt6/
└── qtshadertools_git.bbappend          host-toolchain build fix
```

## How board support works

The last line of `swiftboot.conf`:

```bitbake
include conf/distro/include/board/${MACHINE}.inc
```

`include`, not `require` — a missing file is silent, so an unsupported
`MACHINE` still builds with the board-agnostic defaults. Adding a board means
adding one file; this layer never needs to know the list in advance.

## The decisions, and why

**`DISTRO_FEATURES:remove = "x11 wayland ptest systemd"`**

Dropping `x11` is not an optimisation — it is what makes `eglfs` possible at
all. meta-qt6 chooses between the X11 stack and the embedded stack purely from
`DISTRO_FEATURES` (`qtbase_git.bb` lines 55/59/94), and Qt's own configure
refuses to enable GLES while desktop GL is on:

```
Feature "opengles2": Forcing to "ON" breaks its condition:
    NOT QT_FEATURE_opengl_desktop AND GLESv2_FOUND
```

Appending `eglfs gles2 kms gbm` to `PACKAGECONFIG` while `x11` is present
fails `do_configure`. Remove `x11` and meta-qt6 derives all four by itself.

`ptest` is a poky default that makes every ptest-enabled recipe compile its
unit-test suite — qtbase alone spent 40+ minutes on tests that would never run
on an appliance.

**`NO_RECOMMENDATIONS = "1"` — sharp edge**

This also drops `RRECOMMENDS`, which is how a lot of Yocto quietly wires
itself together. Two consequences seen in practice:

- `init-ifupdown` is only an `RRECOMMENDS` of `packagegroup-core-boot`, so
  without naming it explicitly there is no `/etc/network/interfaces` and **no
  networking at all**.
- Anything `dlopen`'d — Qt platform plugins, QML modules, Mesa DRI drivers —
  is invisible to the dependency scanner *and* gets no help here, so it must
  be listed in `RDEPENDS` by hand.

If an application starts and immediately dies, a missing plugin package is the
first thing to check.

**`TARGET_VENDOR` is deliberately not set**

Overriding it (e.g. to `-swiftboot`) rewrites the toolchain triplet from
`aarch64-poky-linux` to `aarch64-swiftboot-linux`, which invalidates **every**
sstate artifact and rebuilds `gcc-cross` and all of userspace. It buys nothing
but cosmetics.

## `qtshadertools_git.bbappend`

Qt 6.7.3 bundles a glslang snapshot whose headers use `uint32_t` without
including `<cstdint>`. libstdc++ 13 removed the transitive include that used
to make this compile, so on a modern host `qtshadertools-native` fails with:

```
SpvBuilder.h:238:30: error: 'uint32_t' has not been declared
```

`-native` recipes build with the **host** compiler, so this is a property of
your build machine, not the target. 15 of the bundled `SPIRV/*.h` headers have
the same hole, so the fix forces the include rather than patching each file.

It lives here rather than in an application layer because it applies to any Qt
image.

## Host requirements

Yocto scarthgap is validated up to Ubuntu 24.04. On newer hosts expect
breakage from the toolchain and Python running ahead of what the release
supports. If more failures like the glslang one appear, use
`poky/scripts/install-buildtools` to get a known-good self-contained toolchain
rather than patching them one at a time.
