# Strata Portage configuration

Step reads Portage's configuration formats directly. These files are the Strata defaults.

- `make.conf` sets conservative x86-64-v2 optimization, parallel builds, and a Wayland-friendly USE baseline.
- `repos.conf/gentoo.conf` points at the official Gentoo ebuild tree.

Custom repositories use the same `repos.conf` syntax:

```
[my-overlay]
location = /var/db/repos/my-overlay
sync-type = git
sync-uri = https://example.com/my-overlay.git
auto-sync = yes
```

Place additional `.conf` files in `/etc/portage/repos.conf/`. Step lists every `location` it finds when you run `step --info`.

Copy these defaults into place with:

```
install -Dm644 make.conf /etc/portage/make.conf
install -Dm644 repos.conf/gentoo.conf /etc/portage/repos.conf/gentoo.conf
```
