# Source — Context (ICM Layer 2)

All plugin code: audio DSP (`PluginProcessor`) and the JUCE editor UI (`PluginEditor`).

## Routing table

| Task | Read | Skip | Skills / MCP |
|------|------|------|--------------|
| Add/change a DSP parameter or processing stage | `PluginProcessor.h` (member decls), `PluginProcessor.cpp` (`IDs` namespace + `createLayout()` + the relevant `process*` fn) | `PluginEditor.*` unless a new control is needed | — |
| Add/change a knob, toggle, or panel layout | `PluginEditor.h` (component members), `PluginEditor.cpp` (constructor wiring + `resized()`) | `PluginProcessor.*` unless a new parameter is needed | — |
| Change a visual display/meter (curve, meter, drag interaction) | `PluginEditor.h` (the `Component` subclass, e.g. `EqCurveDisplay`/`MultibandGRDisplay`), its `paint`/`mouseDown`/`mouseDrag`/`mouseUp` in `PluginEditor.cpp` | everything else | — |
| Build / package / install | `../CMakeLists.txt`, `../build_mac.sh` | `Source/*` entirely | — |

## Key files (only where the name doesn't say enough)

- `PluginProcessor.cpp` — the `IDs` namespace at the top is the single source of truth for
  every parameter's string ID; `createLayout()` sets ranges/defaults.
- `PluginEditor.h` — skin colours live in the `HertzColours`/`VintageColours`/`SpaceColours`
  namespaces; `HertzLookAndFeel::accent()` is the one heat-interpolated colour every UI
  element should route through (never hardcode green).

## Pipeline

EQ → Multiband Comp → Saturation (Tape→Valve) → Spectral Tame → Clip/Limiter → Out
(EQ/Comp/Sat order is user-reorderable via `chainOrder`; Tame/Clip/Limiter are fixed last.)

## Conventions & gotchas

- Parameter IDs are stable API — renaming breaks saved DAW sessions; only add new ones.
- `processBlock` reads params via `getRawParameterValue(...)->load()` only — no allocation
  or locking on the audio thread, ever.

> Upkeep: when files here are added/removed/renamed, fix the table + key files above in the
> same session, then run `python3 .icm/drift-check.py --update` from the project root.
> Content-only edits usually need no doc change. Keep this file ≲40 lines.
