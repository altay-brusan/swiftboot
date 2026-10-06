# swiftboot

A minimal Yocto Linux distribution that boots straight to a single fullscreen
application, as fast as the hardware allows.

No X server. No Wayland compositor. No desktop. The application renders
directly to DRM/KMS through Qt's `eglfs` platform plugin, and can optionally
replace the init system entirely.

**Status:** working and verified on a Raspberry Pi 3 Model B (64-bit).

---

## Tested configuration

Everything below was actually built and booted, not assumed. If you are
returning to this after a while, these are the versions that are known to
work together.

| Component | Version |
|---|---|
| Yocto release | **scarthgap (5.0)** |
| poky | `yocto-5.0.20-106-gcbd62bb2a9` — commit `cbd62bb2a9f2ab3466a0f72f4289bc86ca20a019` |
| bitbake | 2.8.1 |
| meta-openembedded (`meta-oe`, `meta-python`) | `scarthgap` — `0f00f8b9a21950640da8c5707343e5540133f86e` |
| meta-qt6 | branch `6.7` — `416a83c4c4cedde4503239fff0079a66d8aacc16` (Qt **6.7.3**) |
| meta-raspberrypi | `scarthgap` — `6ca1f75017cc5d5acdb8bb05634c4bc01fa049fd` |

| Target | |
|---|---|
| Machine | `raspberrypi3-64` (aarch64, cortexa53) |
| Distro | `swiftboot` |
| Kernel | linux-raspberrypi 6.6.63 |
| Hardware | Raspberry Pi 3 Model B, booted from SD |

**Build host used:** Ubuntu 26.04 under WSL2 — note that scarthgap is only
*validated* up to Ubuntu 24.04. It does build on newer hosts, but expect
breakage from the host toolchain and Python running ahead of what the release
supports. `meta-swiftboot/recipes-qt/qt6/qtshadertools_git.bbappend` exists
because of exactly that. If you hit more such failures, use
`poky/scripts/install-buildtools` for a known-good self-contained toolchain
rather than patching them one by one.

---

## Repository layout

| Directory | What it is |
|---|---|
| `meta-swiftboot/` | **The OS.** Board-agnostic distro policy, plus one file per supported board. |
| `meta-dashboard/` | **An example.** A Qt 6 QML smart-home dashboard built on swiftboot — a worked case study, not part of the OS. |

Two Yocto layers, one repository. They are genuinely independent layers added
separately to `bblayers.conf`; keeping them in one repo just means a single
commit describes the whole system, with no submodule or manifest needed to
pair versions.

To build a different product on swiftboot, add your own layer next to
`meta-dashboard` and leave `meta-swiftboot` alone.

---

## Quick start (Raspberry Pi 3, 64-bit)

Clone the dependencies alongside this repo:

```bash
git clone -b scarthgap https://git.yoctoproject.org/poky
git clone -b scarthgap https://github.com/openembedded/meta-openembedded
git clone -b 6.7        https://code.qt.io/yocto/meta-qt6
git clone -b scarthgap https://github.com/agherzan/meta-raspberrypi
```

To reproduce the exact tested build, check each out at the commit in the table
above rather than taking branch tips.

```bash
source poky/oe-init-build-env build

bitbake-layers add-layer ../meta-openembedded/meta-oe
bitbake-layers add-layer ../meta-openembedded/meta-python
bitbake-layers add-layer ../meta-qt6
bitbake-layers add-layer ../meta-raspberrypi
bitbake-layers add-layer ../swiftboot/meta-swiftboot
bitbake-layers add-layer ../swiftboot/meta-dashboard
```

`conf/local.conf` needs only this — everything that defines the product lives
in the layers:

```bitbake
MACHINE ?= "raspberrypi3-64"
DISTRO  ?= "swiftboot"

# your build host, not the product
BB_NUMBER_THREADS = "4"
PARALLEL_MAKE = "-j 4"
```

```bash
bitbake dashboard-image
```

Output:
`tmp/deploy/images/raspberrypi3-64/dashboard-image-raspberrypi3-64.rootfs.wic.bz2`

Flash with Raspberry Pi Imager, balenaEtcher, or
`bmaptool copy <image.wic.bz2> /dev/sdX`.

### Build host resources

Qt 6 plus a kernel is a heavy build. On a memory-constrained machine, lower
concurrency rather than anything else — `BB_NUMBER_THREADS x PARALLEL_MAKE` is
the number of compilers running at once, and that is what drives peak usage.
Both variables are excluded from task hashes, so changing them costs no sstate.
Budget roughly 100 GB of disk.

---

## The configuration rule

**Deleting `conf/local.conf` must not change the resulting image.**

If it would, the setting is in the wrong place. `local.conf` is per-developer
and not version controlled; it holds `MACHINE`, `DISTRO`, and how many cores
*your* machine has. Everything else belongs in a layer:

| Question | Goes in |
|---|---|
| A fact about the **hardware**? | `meta-swiftboot/conf/distro/include/board/<MACHINE>.inc` |
| **Product policy** — init, features, graphics? | `meta-swiftboot/conf/distro/` |
| What's **in this image**? | the image recipe |
| How **one package** builds? | a recipe or `.bbappend` |
| About **my PC** — cores, paths, mirrors? | `local.conf` |

---

## Two boot paths, one image

Every image ships both, so the fast path can be tried without giving up a
known-good system.

**Conventional (default)** — sysvinit, networking, SSH, login prompt. The
application starts at priority `S00`, ahead of networking, so the UI is up
before DHCP is even attempted.

**Appliance** — append to the single line in `cmdline.txt` on the FAT
partition:

```
init=/usr/bin/dashboard-launcher
```

This replaces sysvinit entirely: no runlevels, no `rcS.d`/`rc5.d` (~26 shell
scripts), no getty, no login. Delete the line to go back.

The launcher runs as **PID 1**, which the kernel treats specially — if it
exits, the kernel panics. So it never execs the app directly; it supervises
it, restarts it, and drops to a rescue shell on `/dev/console` after three
consecutive failures rather than leaving a bricked board.

---

## Measured results

From the QEMU bring-up, where before/after could be compared directly:

| Change | Time to UI |
|---|---|
| Baseline | 26.5 s |
| App moved from `S99` to `S00` (ahead of DHCP) | 23.0 s |
| PID 1 launcher, no init system | ~13 s |

Those numbers are from `qemuarm64` under **TCG** — software emulation of ARM
on x86, roughly 10–20x slower than native. They are useful for comparing
changes to each other and useless as absolutes. On real hardware the same
image is far quicker.

Where the remaining time goes, measured on the booted system:

```
kernel        ~12.9s   (~100% of what is left)
userspace       ~0s    (eliminated)
```

Userspace tuning is essentially done. Further gains have to come from the
kernel — see the roadmap.

---

## Adding a board

One file:

```
meta-swiftboot/conf/distro/include/board/<MACHINE>.inc
```

`swiftboot.conf` pulls it in with `include` (not `require`), so an unknown
`MACHINE` is not an error — it simply gets the board-agnostic defaults.
Nothing else changes.

`board/raspberrypi3-64.inc` is verified on hardware.
`board/beaglebone-yocto.inc` is an explicitly untested stub that records what
is already known to differ (ARMv7 vs aarch64; the SGX530 GPU has no
open-source GLES driver, so it needs either `meta-ti` or `linuxfb` with
software rendering).

---

## Roadmap

- **Kernel trimming.** The kernel is now ~100% of boot time. A config fragment
  dropping drivers for absent hardware is the next real win.
- **`kas` manifest** pinning every layer's URL and commit, so one command
  reproduces an exact build.
- **BeagleBone Black bring-up**, starting from the stub board file.

---

## License

MIT. See `COPYING.MIT` in each layer.
