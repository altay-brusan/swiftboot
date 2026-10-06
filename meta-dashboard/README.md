# meta-dashboard

An **example** of building a product on [swiftboot](../README.md): a Qt 6 QML
smart-home dashboard for the Raspberry Pi 3 (64-bit).

This layer is not part of the OS. It exists to show what an application layer
looks like — copy its shape for your own product and leave `meta-swiftboot`
untouched.

Verified on a Raspberry Pi 3 Model B.

## What's here

```
recipes-core/images/dashboard-image.bb          the image
recipes-dashboard/smarthome-dashboard/
├── smarthome-dashboard_git.bb                  the Qt app
└── files/
    ├── smarthome-dashboard.init                sysvinit script (S00)
    └── dashboard-launcher                      optional PID 1 replacement
```

The application itself is third-party:
[allankoechke/SmartHomeDashboardNew](https://github.com/allankoechke/SmartHomeDashboardNew),
pinned to a specific commit.

## Things worth copying

**`DEPENDS` needs the `-native` half for QML.**

```bitbake
DEPENDS += "qtbase qtdeclarative qtdeclarative-native"
```

`qtbase`/`qtdeclarative` give the aarch64 libraries you link against;
`qtdeclarative-native` gives the x86-64 host tools that *run* during the build
(`qmltyperegistrar`, `qmlcachegen`, `qmlimportscanner`). `find_package(Qt6
COMPONENTS Quick)` needs both — without the native half CMake finds `Qt6Quick`
but not `Qt6QuickTools` and `do_configure` fails.

**Name dlopen'd runtime dependencies explicitly.**

```bitbake
RDEPENDS:${PN} += "qtbase-plugins qtdeclarative-qmlplugins mesa-megadriver"
```

Qt loads its platform plugin, QML modules and the Mesa DRI driver at runtime,
so nothing links against them and the dependency scanner cannot see them.
swiftboot sets `NO_RECOMMENDATIONS = "1"`, so nothing arrives implicitly
either.

**Pin `SRCREV`, never `AUTOREV`.**

`AUTOREV` re-polls the remote on every parse, defeats sstate reuse, and means
two builds a day apart ship different code with no record of the difference.

**Start early.**

```bitbake
INITSCRIPT_PARAMS = "start 00 2 3 4 5 . stop 20 0 1 6 ."
```

Priority `00` puts the UI ahead of `S01networking`, whose DHCP wait was the
single largest userspace cost. The dashboard needs no network to draw its
first frame; networking still comes up, just behind the UI.

## The PID 1 launcher

`files/dashboard-launcher` replaces the init system entirely when selected on
the kernel cmdline:

```
init=/usr/bin/dashboard-launcher
```

It mounts `/proc`, `/sys`, `/run`, `/tmp` and `/var/volatile`, sets the Qt
eglfs environment, waits for `/dev/dri/card0`, then supervises the app.

`/dev` needs nothing: the kernel's `devtmpfs` already created the device nodes
before init runs, which is exactly why udev is not required on this path.

It does **not** `exec` the application. PID 1 exiting panics the kernel, so a
bare `exec` would turn any application crash into a kernel panic. Instead it
restarts the app, and after three consecutive failures drops to a shell on
`/dev/console` so the board stays debuggable.

With no SSH and no getty on this path, the serial console is the only way in —
keep `ENABLE_UART = "1"` and a USB-serial adapter handy.

## Image notes

**`read-only-rootfs` is deliberate, and it is not a speed feature.** It costs a
little at boot — `/var` is rebuilt in tmpfs every time — and exists so that
yanking the power cannot corrupt the SD card. One consequence: dropbear
regenerates its host key on every boot (~2 s), because it cannot persist one.
For a product, remove dropbear rather than making the rootfs writable.

**`IMAGE_FEATURES` uses `+=`, not `=`.** A hard assignment discards
`debug-tweaks` and leaves root with a locked password — an image that builds
cleanly and that you cannot log into, over SSH or serial.

**`init-ifupdown` is listed explicitly.** It is only an `RRECOMMENDS` of
`packagegroup-core-boot`, and swiftboot's `NO_RECOMMENDATIONS = "1"` drops it.
Without it there is no `/etc/network/interfaces` and no networking at all.
