# HERTZ MAGIC

**Hertzteq EQ + 3-band multiband compressor + tape/valve saturation — Ric Hertz Mastering**

A mastering-grade Audio Unit for Logic Pro. Signal path:

```
IN · HERTZTEQ EQ · MULTIBAND COMP · TAPE→VALVE · SPECTRAL TAME · CLIP/LIMITER · OUT
```

EQ / Comp / Saturation are drag-reorderable; Spectral Tame is glued to the
saturation output and the clipper/limiter is always last.

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
Drag the nodes on the curve display, or use the knobs. Knobs are grouped
**LOW · NOTCH · HIGH** left-to-right, under their part of the frequency axis.
Low: boost/atten, 20/30/60/100 Hz. High: bell boost with bandwidth,
3–16 kHz, plus shelf atten at 5/10/20 kHz. **Four fully parametric notches**
(freq / gain / Q each, two rows of knobs) — each is a draggable square on the
curve, so you can park one cut in the 2–4 kHz vocal-edge zone and another at
5–8 kHz sibilance while keeping two spare.

- **LOW CUT** — a steep 24 dB/oct high-pass for pulling out rumble and sub,
  sweepable **10–50 Hz**. Toggle with **LC**, or grab its own **roll-off node**
  on the curve (the little high-pass symbol, distinct from the shelf circles
  and notch squares) and drag it — grabbing it also switches the cut in.
- **Spectrum analyser** — a live FFT of the processed output sits behind the
  curve so you see how the EQ (and the saturation harmonics) shape the mix.
  **SPEC** toggles it on/off; **DET·L/M/H** selects the resolution (low =
  smooth and broad, high = fine and snappy).

### MULTIBAND COMP
Three bands (Low / Mid / High, Drawmer-style) split by 4th-order
Linkwitz-Riley crossovers — XO1 (60–800 Hz) and XO2 (0.8–12 kHz).
Per band: Threshold, Ratio, Attack, Release, Makeup, Solo (S) and
Bypass (B), with colour-coded GR meters (cyan/green/amber).
Mastering starting point: 1–2 dB GR per band, ratios around 2:1.

### SATURATION — Tape ⇄ Valve
Two independent stages in series, both inside one 4x-oversampled block
(linear-phase filters, latency reported to Logic). **SWAP** reverses the
processing order to Valve → Tape — the tape stage's HF softening then rounds
off the valve's upper harmonics instead of feeding them, a genuinely different
top-end character worth an A/B when a master reads harsh. The stage captions
read **TAPE 1 / VALVE 2** (or 2/1) so the live order is always visible.

- **TAPE** (Phoenix-inspired): soft compression of peaks, gentle even-harmonic
  warmth, and HF softening that deepens with drive. Character selects the
  tonal family — Luminous (open), Gold (classic), Dark (thick).
- **VALVE** (ECC83-style): asymmetric tube shaping, 2nd-harmonic biased. In
  stereo mode a **VALVE LP** control adds a gentle low-pass on the valve output
  (2–20 kHz; 20 kHz = off) to round off the top. In M/S mode this is replaced
  by the per-side SIDE LP.

Each has its own drive and IN switch, and each shows an **extremity meter** —
how hard that stage is bending the signal (the added harmonic content), so you
can see when you're pushing into obvious distortion. In **M/S** mode the
CHARACTER control stays available alongside the per-Mid/Side drives. Both
stages are unity-gain at low level — they only touch the peaks, so density you
hear is saturation, not loudness. The whole UI shifts green → orange as drive
and gain reduction build.

### SPECTRAL TAME (post-saturation)
Soothe-style dynamic top-end smoothing — six bands from 1.8 to 15 kHz.
Each band is compared against the overall spectral tilt; anything poking
above it (sizzle, harsh tops, digital glare) gets pulled down dynamically
and released in ~120 ms. **DEPTH** sets how hard offending bands are pulled,
**SENS** sets how easily they trigger. **Click any band lane** in the display
to disable/enable just that frequency (all on by default) — handy for leaving
the air band alone while taming the 4–10 kHz glare. Zero latency; the display
shows which bands are working. Start around DEPTH 4 / SENS 5.

**FLAG** paints the same live detection onto the main EQ analyser as a **red
"problem area" glow**: any tame band actively cutting glows at its frequency
on the curve display, brighter the harder it's working — so you can see where
the harshness sits and counter it with a notch or the saturation controls, not
just let the tame absorb it. Purely visual; toggle it off any time.

### INPUT STAGE (left rail)
God Particle-style ideal-level metering: the meter shows post-trim **RMS**
with a sweet-spot zone at **−18 dBFS RMS (±2 dB)**. Turn the INPUT knob until
the bar sits in the zone (it lights up in the accent colour when you're
there; orange means too hot). The white tick is the peak. Hitting the zone
puts every stage after it in its designed operating range.

### CLIP · LIMITER (fixed, always last)
Transient **POKE** → soft **CLIP** → lookahead **LIMITER** (Transparent /
Punch / Dynamic), with **GAIN**, **CEILING** and:

- **OVERSMP** — clipper/limiter oversampling, selectable **4× / 8× / 16×**.
  Higher = cleaner peak control and less aliasing on hot masters (more CPU).
  Switching rebuilds the stage off the audio thread and re-reports latency.
- **TP** — true-peak limiting. On, the limiter clamps inter-sample peaks
  (measured from the oversampled signal) so the output stays under the
  ceiling as a true-peak value, not just per-sample.
- Three meters read at a glance: **GR** (limiter reduction), **PK** (transient
  poke activity) and **CL** (clipper drive).

### MASTER
Mix / Output faders (Input lives in the left rail). The **OUTPUT** fader has a
soft **detent at 0 dB** (it catches there as you drag; double-click also snaps
to 0) with a tick marking unity. The dry path for Mix is latency-compensated,
so parallel settings stay phase-coherent. The **LOUDNESS** cluster reads
**RMS** and **short-term LUFS** as a true sliding average, with a
**3 / 5 / 10 s** selector (3 s is the EBU R128 short-term window; 5 and 10 s
are slower for steadier monitoring).

### OUTPUT METER (right rail)
A mastering meter in the unit that matters for delivery: **LUFS**. The bar is
short-term loudness with the scale marked, a dashed **−14 LUFS** line (the
de-facto streaming target — Spotify/Apple/YouTube), a **−1 dBTP** ceiling line,
and a **true-peak indicator** tick. Aim the bar around the −14 line and keep
the peak tick under −1 for a safe, streaming-ready master.

### SKINS
Cycle the button top-right through three looks (it shows the **active** skin):
**DIGITAL** (phosphor green), **VINTAGE** (brushed-aluminium hardware), and
**SPACE** — a Star Trek **LCARS** console: black void, starfield with warp
streaks, rounded LCARS colour rails, and an ion-blue → amber heat shift.
Purely cosmetic — the processing is identical in every skin.

---

## Technical notes

- Formats: **AU (.component)**, VST3, Standalone — universal binary
  (Apple Silicon + Intel)
- Latency: reported to the host (linear-phase oversampling filters, ~1 ms)
- Framework: JUCE 8 (GPLv3 / commercial dual licence — fine for personal
  use; a JUCE licence would be needed only if you sell it closed-source)
- State: full preset/session recall via Logic

Built for Ric Hertz · richertz.com · signal present
