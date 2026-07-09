# Hertz Magic — Router (ICM Layer 1)

> Map, not manual: read this, open exactly ONE workspace's CONTEXT.md, follow its routing
> table. Don't pre-read anything else. (Methodology: METHODOLOGY.md.)
> Docs reflect the repo at last sync; a session-start `[icm]` warning means code moved
> since then — trust structure, re-verify contents for load-bearing work.

## What this project is

A JUCE-based Audio Unit / VST3 mastering plugin for Logic Pro — Hertzteq EQ, 3-band
multiband compressor, tape/valve saturation, spectral tame, and a clipper/limiter — built
under the "Ric Hertz Mastering" brand (richertz.com).

## Workspaces

| Workspace | Path | Purpose | Open when |
|-----------|------|---------|-----------|
| Source | `Source/` | All plugin code: DSP (PluginProcessor) + JUCE editor UI (PluginEditor) | Any parameter, DSP, or UI change |

## Folder map (top level only — each workspace maps its own depth)

```
Hertz_Magic/
├── CMakeLists.txt       # build config — targets: AU, VST3, Standalone
├── build_mac.sh         # one-shot build + install + .pkg script
├── README.md            # user-facing build/install instructions
└── Source/              # all plugin code — see Source/CONTEXT.md
```

## Naming conventions

- Parameter string IDs live in the `IDs` namespace at the top of `PluginProcessor.cpp` —
  stable API, only add, never rename (breaks saved sessions).
- UI components follow `<Feature>Display` / `<Feature>Meter` (e.g. `EqCurveDisplay`,
  `MasteringMeter`).

## Standing rules (in force until the user says otherwise)

- Load only what a routing table points to; stay inside the chosen workspace.
- Keep names identical across router, contexts, and folders — routing depends on it.
- **Upkeep:** if you add, remove, or rename files, update that workspace's CONTEXT.md (and
  this router if the top-level layout changed) in the same session — the context is already
  loaded, so this is nearly free. Creating or retiring a whole workspace? Also update the
  Workspaces table above and `.icm/manifest.json`. Then run (from the project root):
  `python3 .icm/drift-check.py --update`
- Keep this file ≲60 lines and each CONTEXT.md ≲40. Overflow means push detail down a layer.
