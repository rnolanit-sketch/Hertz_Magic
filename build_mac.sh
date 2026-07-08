#!/bin/bash
# ==============================================================
#  Hertz Magic — macOS build + installer script
#  Run this on your Mac from inside the HertzMagic folder:
#      chmod +x build_mac.sh && ./build_mac.sh
#  Requires: Xcode Command Line Tools (xcode-select --install)
#            CMake (brew install cmake)
# ==============================================================
set -e

echo "==> Configuring (JUCE downloads automatically on first run)..."
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

echo "==> Building Audio Unit + VST3 (this takes a few minutes first time)..."
cmake --build build --config Release -j"$(sysctl -n hw.ncpu)"

AU_PATH="build/HertzMagic_artefacts/Release/AU/Hertz Magic.component"

if [ ! -d "$AU_PATH" ]; then
    echo "ERROR: AU component not found at $AU_PATH"
    exit 1
fi

echo "==> Ad-hoc code signing (required on Apple Silicon)..."
codesign --force --deep -s - "$AU_PATH"

# The build already copies the AU to ~/Library/Audio/Plug-Ins/Components.
# Now also wrap it into a distributable .pkg installer:
echo "==> Building installer package..."
PKG_ROOT="build/pkgroot"
rm -rf "$PKG_ROOT"
mkdir -p "$PKG_ROOT/Library/Audio/Plug-Ins/Components"
cp -R "$AU_PATH" "$PKG_ROOT/Library/Audio/Plug-Ins/Components/"

pkgbuild --root "$PKG_ROOT" \
         --identifier com.richertz.hertzmagic \
         --version 1.0.0 \
         --install-location / \
         "Hertz Magic Installer.pkg"

echo ""
echo "==> Done."
echo "    Installer:  $(pwd)/Hertz Magic Installer.pkg"
echo "    Component:  ~/Library/Audio/Plug-Ins/Components/Hertz Magic.component"
echo ""
echo "==> Refreshing Logic's Audio Unit cache..."
killall -9 AudioComponentRegistrar 2>/dev/null || true
echo "    Open Logic Pro — Hertz Magic will appear under:"
echo "    Audio Units > Ric Hertz > Hertz Magic"
