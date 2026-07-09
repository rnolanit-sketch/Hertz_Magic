# dsp — Context (ICM Layer 2)

Audio DSP. `HertzMagicAudioProcessor` is **one class** (declared in `PluginProcessor.h`)
whose implementation is split by module across the `.cpp` files below; all share
`ParamIDs.h`. No allocation or locking on the audio thread — ever.

## Routing table

| Task | Read | Skip | Skills / MCP |
|------|------|------|--------------|
| Add/change a parameter (range, default, ID) | `ParamIDs.h` (string ID), `Parameters.cpp` (`createLayout` range/default), + the module `.cpp` that reads it | `ui/` unless a new control is needed | — |
| Change EQ / notch / low-cut response | `Eq.cpp` (`updateEqCoefficients`), `PluginProcessor.h` (filter members) | other module `.cpp` | — |
| Change multiband compressor or crossovers | `MultibandComp.cpp` | everything else | — |
| Change tape / valve saturation | `Saturation.cpp` | everything else | — |
| Change spectral tame (resonance suppression) | `SpectralTame.cpp` | everything else | — |
| Change clipper / limiter / loudness / gain-match | `Master.cpp` (`processFinal`) | everything else | — |
| Change chain order or block flow | `PluginProcessor.cpp` (`processBlock`) | module `.cpp` | — |

## Key files (only where the name doesn't say enough)

- `ParamIDs.h` — the `IDs` namespace (single source of truth for parameter string IDs) plus
  the fixed frequency tables shared by `createLayout` and the EQ.
- `PluginProcessor.cpp` — core only: ctor, `prepareToPlay`, `processBlock` (chain routing
  via `chainOrder`), state save/load, analyser scope ring.

## Pipeline

EQ → Multiband Comp → Saturation (Tape→Valve) → Spectral Tame → Clip/Limiter → Out
(EQ/Comp/Sat order is user-reorderable via `chainOrder`; Tame/Clip/Limiter are fixed last.)

## Conventions & gotchas

- Parameter IDs are stable API — renaming breaks saved DAW sessions; only add new ones.
- `processBlock` reads params via `getRawParameterValue(...)->load()` only.

> Upkeep: when files here are added/removed/renamed, fix the table + key files above in the
> same session, then run `python3 .icm/drift-check.py --update` from the project root. Keep
> this file ≲40 lines.
