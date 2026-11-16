# Strata overlay

The Strata profile is a thin layer on top of Gentoo's `default/linux/amd64/17.1` profile. It sets the OpenRC-friendly defaults, keywords, and USE flags that define a Strata system without forking the Gentoo tree.

## Layout

```
overlay/
├── metadata/layout.conf
└── profiles/
    ├── repo_name
    ├── profiles.desc
    └── strata/
        ├── make.defaults
        ├── packages
        └── parent
```

## Using it

Add the overlay to `repos.conf`:

```
[strata]
location = /var/db/repos/strata
```

Select the profile:

```
eselect profile set strata:strata
```

Or with Step:

```
step profile set strata:strata
```

`step profile` lands with the resolver. Until then, use `eselect`.

## Adding packages

Strata packages belong in `overlay/<category>/<package>/` as normal ebuilds. Keep changes minimal and send fixes upstream to Gentoo when they are not Strata-specific.

## License

GPL-2.0.
