# Strata kernel

Strata uses `gentoo-sources` with a performance-oriented Kconfig fragment.

```
emerge sys-kernel/gentoo-sources
cd /usr/src/linux
make defconfig
/path/to/strata/config/kernel/apply.sh /usr/src/linux
make -j"$(nproc)"
make modules_install
make install
```

`strata.fragment` enables:

- 1000 Hz high-resolution timers with dynamic preemption.
- NVMe, AHCI, USB, EFI stub, and ext4.
- Common Ethernet, Wi-Fi, and virtio drivers.
- amdgpu, i915, and nouveau as modules.
- ZRAM, ZSWAP, and transparent huge pages.
- Debug info off and a `-strata` localversion.

The fragment is a baseline, not a universal config. Hardware-specific options such as NVIDIA, exotic Wi-Fi, or laptop power management belong in a local fragment. `apply.sh` merges the fragment into an existing `.config` with the kernel's own `merge_config.sh`, then runs `olddefconfig`.

A prebuilt `strata-kernel` binpkg is planned once the binhost is live.
