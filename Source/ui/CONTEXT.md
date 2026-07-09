# ui — Context (ICM Layer 2)

JUCE editor. `PluginEditor.h` declares the editor plus every widget class and the three
skin colour namespaces; the editor lives in `PluginEditor.cpp`, and each widget group has
its own `.cpp`. Never hardcode a colour — route through `HertzLookAndFeel::accent()`.

## Routing table

| Task | Read | Skip | Skills / MCP |
|------|------|------|--------------|
| Add/change a knob, toggle, or panel layout | `PluginEditor.h` (component members), `PluginEditor.cpp` (ctor wiring + `resized`/`layoutModules`) | widget `.cpp`, `dsp/` unless a new parameter is needed | — |
| Change a display/meter's drawing or drag interaction | the matching widget `.cpp` (see key files) + its class in `PluginEditor.h` | `PluginEditor.cpp` | — |
| Change knob / slider / toggle / label look | `LookAndFeel.cpp` | everything else | — |
| Add or restyle a skin | `PluginEditor.h` (`*Colours` namespaces), `PluginEditor.cpp` (`paint*Background`, `applySkinToAll`) | widget `.cpp` | — |
| Change the spectrum analyser | `PluginEditor.cpp` (`updateAnalyzer`) + `EqCurveDisplay.cpp` (`paint`) | `dsp/` | — |
| Build / package / install | `../../CMakeLists.txt`, `../../build_mac.sh` | `Source/*` entirely | — |

## Key files (widget → file; only where the name doesn't say enough)

- `EqCurveDisplay.cpp` — EQ curve, analyser overlay, genre reference curves.
- `MultibandGRDisplay.cpp` — comp GR bars + draggable crossover dividers.
- `SaturationDisplays.cpp` — `TapeDisplay`, `ValveDisplay`, `DriveMeter`.
- `Meters.cpp` — `IdealInputMeter`, `MasteringMeter`, `LevelMeter`.
- `SpectralTameDisplay.cpp` — draggable/toggleable spectral-tame bands.
- Skin colours live in `HertzColours`/`VintageColours`/`SpaceColours` in `PluginEditor.h`.

## Conventions & gotchas

- UI components follow `<Feature>Display` / `<Feature>Meter`.
- The editor polls the processor's atomics on a `Timer` — displays read, never write DSP.

> Upkeep: when files here are added/removed/renamed, fix the table + key files above in the
> same session, then run `python3 .icm/drift-check.py --update` from the project root. Keep
> this file ≲40 lines.
