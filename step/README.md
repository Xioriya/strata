# Step

Step is Strata's package manager. It is a C++ reimplementation of Portage's orchestration layer with an emerge-compatible command line.

## Design

Step consumes Gentoo's ebuild tree unchanged. Ebuild phases remain bash scripts executed in a sandbox; the C++ code owns the parts where speed and clarity matter:

- argument parsing and user interface
- configuration loading (`make.conf`, `package.*`, `repos.conf`)
- metadata cache
- dependency resolution
- task scheduling and parallel fetch/build
- merge planning and VDB writes
- GPKG binary package handling

Rewriting ebuild execution in C++ would not make it faster. Ebuilds are bash. Step is honest about that and optimizes the surrounding work instead.

## Implemented

```
step --help
step --version
step --info
step --sync
step --search <query>
step -s <query>
step which <atom>
step unmask <atom>...
step mask <atom>...
step use <atom> <flag>...
```

`--sync` clones or fast-forwards the Gentoo tree. `which` prints the highest matching ebuild path, or an exact version when one is given. Its version comparison covers numeric components, letters, `_alpha` through `_p` suffixes, and `-rN` revisions; full PMS version semantics arrive with the resolver. `unmask`, `mask`, and `use` write to `/etc/portage/package.*/strata.conf`, deduplicating entries. `--info` prints configuration, repositories, installed package count, and repository status.

## Planned

```
step -av <atom>          resolve and install
step -uD @world          update the world set
step -C <atom>           uninstall
step --depclean          remove unused packages
```

The resolver, merge engine, sandboxed ebuild execution, and GPKG layer are in progress.

## Compatibility

Step reads Portage's configuration formats and writes VDB-compatible records. GPKG is the binary package format. A step installation can coexist with Portage during the transition.

## Build

```
make
./step --info
```

Requires a C++20 compiler. The current code has no third-party dependencies.

## Test

```
make test
```

The harness builds Step and exercises argument parsing, configuration loading, search, ebuild selection, unmasking, masking, USE flags, and sync planning against fixture data.

## Reference

`~/Projects/portage-reference` is a read-only clone of Gentoo Portage used to study behaviour. Step is a clean-room implementation and does not copy Portage source.

## License

GPL-2.0.
