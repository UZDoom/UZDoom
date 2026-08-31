# Nuked-OPL3-fast

Bit-exact, performance-optimized fork of
[Nuked-OPL3](https://github.com/nukeykt/Nuked-OPL3), Nuke.YKT's
cycle-accurate Yamaha YMF262 (OPL3) emulator. Pinned to upstream commit
`cfedb09` (Nuked-OPL3 v1.8).

Audio output is identical to upstream for the same register stream.

## Performance

Measured on x86-64 Linux, GCC `-O2`, best of repeated runs:

| Workload                                | Upstream | This fork | Speedup |
|-----------------------------------------|----------|-----------|---------|
| Synthetic chip-core (TSC ticks/sample)  | ~900     | ~400      | ~2.2x   |
| Full-track VGM render (3.5 min)         | 4.8 s    | 3.0 s     | ~1.6x   |

Mileage may vary depending on content being rendered. There are several
shortcuts that are only hit for channels that are silent, uninitialized, etc.
Tests are based on a light early-Apogee-style IMF tune and a recent demoscene
production that is doing stuff in basically all the slots all the time.

Across a 40-track random sample from
[The OPL Archive](https://opl.wafflenet.com/), this fork rendered every file
1.44x to 2.10x faster than upstream (median 1.85x), with output identical to
upstream Nuked-OPL3 on all 40.

## API

Drop-in source-level replacement. All public functions and the public
`opl3_chip` struct are unchanged.

Internal structs (`opl3_slot`, `opl3_channel`) gained some cached fields.
If your code touches these directly, you'll need to change it.

## Building

Add to your build:

- `opl3.c`
- `opl3.h`
- `wf_rom.h`

`gen_logsin.py` is the generator for `wf_rom.h`. The generated file is
committed; you only need Python if you want to regenerate it.

## Bit-exactness

Every kept change produces identical 16-bit PCM samples for the same
register stream as an unmodified upstream build. Candidate optimizations
that affected output were discarded.

A 40-track random sample from
[The OPL Archive](https://opl.wafflenet.com/) produced render checksums
identical to upstream Nuked-OPL3 on every file.

The test suite will be made available in the near future.

## Changes

The full annotated list is at the top of `opl3.c`. At a high level:

- Waveform math replaced by a precomputed 8x1024 logsin lookup table
  (`wf_rom.h`).
- Several write-time caches on `opl3_slot` (TL+KSL sum, envelope key-scale
  shift, non-vibrato phase increment, envelope rate resolution).
- Fast paths in `OPL3_ProcessSlot` for the common attenuated-and-key-off,
  permanently-dead, and sustain-with-rate-zero slot states.
- Silent-regime variant of `OPL3_SlotGenerate` used by the attenuated
  fast path.
- Both 18-channel mix loops in `OPL3_Generate4Ch` unrolled to skip dummy
  reads for muted and 2-op voices via a per-channel `out_cnt`.
- Rhythm-mode dispatch in `OPL3_PhaseGenerate` fused into a single
  `slot_num` switch for jump-table dispatch.
- Hot fields hoisted into the first cache line of `opl3_slot`; struct
  size dropped from 96 to 88 bytes.

## License

LGPL-2.1-or-later. See `LICENSE`.

## Credits

See comments in `opl3.c`.

This fork is maintained by Tony Gies.
