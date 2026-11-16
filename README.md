# Strata GNU/Linux

**Gentoo, refined.** An ebuild-native, source-based GNU/Linux distribution built on Gentoo's package tree, with a faster package manager called **Step**, a guided installer, and a tuned kernel.

> Status: **pre-alpha**. The architecture, installer flow, Step foundation, overlay profile, and test harness exist. Strata is not installable yet. Step's resolver, merge engine, and binary package layer are the next milestone.

## What Strata is

Strata keeps what makes Gentoo good — the ebuild tree, USE flags, profiles, and source-based control — and replaces the rough edges:

- **Step** — a C++ package manager with emerge-compatible syntax, a faster resolver and metadata cache, and easier unmasking.
- **Guided install** — `strata-guide` walks through disk setup, mounting, base extraction, chroot, and configuration with whiptail.
- **Binhost first** — GPKG-compatible binary packages so users get Gentoo's binary cache and Strata can publish its own.
- **OpenRC** — predictable, dependency-free init.
- **Tuned kernel** — `gentoo-sources` plus a performance-oriented Kconfig fragment.
- **Real documentation** — the website is the handbook.

Strata does not fork Gentoo's repository. It consumes it unchanged.

## Repositories

- **strata** — this repository: Step, the installer, configuration defaults, the kernel fragment, and the Strata overlay.
- **strata-www** — the documentation site: [github.com/Xioriya/strata-www](https://github.com/Xioriya/strata-www).

## Repository layout

```
strata/
├── step/                 C++ package manager
├── installer/            whiptail installer suite
├── config/portage/       make.conf and repos.conf defaults
├── config/kernel/        Kconfig fragment and apply script
├── overlay/              Strata profile and package overlay
├── tests/                Step test harness and fixtures
└── .github/workflows/    CI build and test
```

## Build and test Step

```
make
./step/step --version
make test
```

Install into `/usr/local/bin`:

```
sudo make install
```

Step currently implements `--help`, `--version`, `--info`, `--sync`, `--search`, `which`, `unmask`, `mask`, and `use`. Install, remove, update, and depclean report that the resolver is the next milestone.

## Install Strata

```
sudo make -C installer install
sudo strata-guide
```

The pre-alpha installer bootstraps from a Gentoo OpenRC stage3 and finishes with `strata-setup` inside the chroot. See the website install tab for the full flow.

## Documentation

Live at [xioriya.github.io/strata-www](https://xioriya.github.io/strata-www/). The site lives in the separate `strata-www` repository. Preview locally:

```
git clone https://github.com/Xioriya/strata-www
python3 -m http.server 8000 -d strata-www
```

## License

GPL-2.0. Step reimplements the behaviour of Portage, which is GPL-2.0. Contributions are accepted under the same license.

## Name

Strata is the distribution. Step is the package manager. Geology is the theme: layers, strata, bedrock.
