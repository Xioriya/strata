# Strata installer

Whiptail-driven installation flow. Run each tool as root.

```
strata-guide
```

## Flow

1. `setup-disks` selects a disk and creates a GPT layout: 1 GiB EFI, 4 GiB swap, rest root. Manual `cfdisk` partitioning is also available.
2. `setup-mounts` finds the `STRATA-ROOT`, `STRATA-EFI`, and `STRATA-SWAP` labels, mounts them under `/mnt/strata`, and enables swap.
3. `setup-tar` downloads, verifies, and extracts a Gentoo OpenRC stage3 as the bootstrap base system.
4. `strata-chroot` bind-mounts the virtual filesystems and enters the target.
5. `strata-setup` runs inside the chroot and configures the hostname, users, timezone, locale, kernel, fstab, GRUB, and networking.

## Status

Pre-alpha. The installer bootstraps from Gentoo's stage3 and uses Portage inside the chroot. Strata stage3 images and Step as the default package manager land after the Step resolver and merge engine are complete.

## Environment

- `TARGET_MNT` defaults to `/mnt/strata`.
- `STRATA_STAGE3_BASE` overrides the stage3 download directory.

## Install the scripts

```
make install
```
