# HERTZ MAGIC

**Hertzteq EQ + 3-band multiband compressor + tape/valve saturation — Ric Hertz Mastering**

A mastering-grade Audio Unit for Logic Pro. Signal path:

```
IN · HERTZTEQ EQ · MULTIBAND COMP · TAPE→VALVE · OUT   (drag to reorder)
```

The interface runs phosphor green when the signal is clean and shifts to
orange as you push it — the heat follows both the valve drive and the
amount of gain reduction.

---

## Building on your Mac (one-time, ~5 minutes)

1. **Install the tools** (skip anything you already have):
   ```bash
   xcode-select --install        # Apple's compiler
   brew install cmake            # build system (https://brew.sh if no Homebrew)
   ```

2. **Build + install + make the .pkg:**
   ```bash
   cd HertzMagic
   chmod +x build_mac.sh
   ./build_mac.sh
   ```
   The first run downloads JUCE automatically and takes a few minutes.
   When it finishes you'll have:
   - `Hertz Magic.component` installed to `~/Library/Audio/Plug-Ins/Components/`
   - `Hertz Magic Installer.pkg` in the project folder (for installing on
     any other Mac, or distributing)

3. **Open Logic Pro.** The plugin appears under
   **Audio Units → Ric Hertz → Hertz Magic**. If Logic doesn't see it
   immediately, restart Logic once (it rescans the AU cache on launch).

> **Gatekeeper note:** the script ad-hoc signs the plugin, which is fine for
> your own machines. If you ever distribute the .pkg publicly you'd want an
> Apple Developer ID certificate ($99/yr) and notarization.

---

## The controls

Three modules, **drag any module header to reorder the chain** (default: EQ → Comp → Saturation). The order saves with your Logic session. Master strip is always last.

### HERTZTEQ EQ
Passive-style curve with the classic trick: boost and attenuate the same low
frequency simultaneously to carve a low-mid dip while the sub stays full.
Drag the nodes on the curve display, or use the knobs.
Low: boost/atten, 20/30/60/100 Hz. High: bell boost with bandwidth,
3–16 kHz, plus shelf atten at 5/10/20 kHz.

### MULTIBAND COMP
Three bands (Low / Mid / High, Drawmer-style) split by 4th-order
Linkwitz-Riley crossovers — XO1 (60–800 Hz) and XO2 (0.8–12 kHz).
Per band: Threshold, Ratio, Attack, Release, Makeup, Solo (S) and
Bypass (B), with colour-coded GR meters (cyan/green/amber).
Mastering starting point: 1–2 dB GR per band, ratios around 2:1.

### SATURATION — Tape → Valve
Two independent stages in series, both inside one 4x-oversampled block
(linear-phase filters, latency reported to Logic):

- **TAPE** (Phoenix-inspired): soft compression of peaks, gentle even-harmonic
  warmth, and HF softening that deepens with drive. Character selects the
  tonal family — Luminous (open), Gold (classic), Dark (thick).
- **VALVE** (ECC83-style): asymmetric tube shaping, 2nd-harmonic biased.

Each has its own drive and IN switch. Both are unity-gain at low level —
they only touch the peaks, so density you hear is saturation, not loudness.
The whole UI shifts green → orange as drive and gain reduction build.

### MASTER
Input / Mix / Output faders. The dry path for Mix is latency-compensated,
so parallel settings stay phase-coherent. IN/OUT meters flank the plugin.

---

## Technical notes

- Formats: **AU (.component)**, VST3, Standalone — universal binary
  (Apple Silicon + Intel)
- Latency: reported to the host (linear-phase oversampling filters, ~1 ms)
- Framework: JUCE 8 (GPLv3 / commercial dual licence — fine for personal
  use; a JUCE licence would be needed only if you sell it closed-source)
- State: full preset/session recall via Logic

Built for Ric Hertz · richertz.com · signal present
