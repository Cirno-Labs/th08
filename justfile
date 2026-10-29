export WINEPREFIX := justfile_directory() / ".wine"
export WINEDEBUG := env("WINEDEBUG", "-all")

exe := "build/th08_100d.exe"

# List recipes
default:
    @just --list

# Compile src/ and link build/th08.dll
build:
    python3 tools/build.py

# Compare compiled functions with the original; show a diff for each SYMBOL
[positional-arguments]
compare *symbols: build _orig
    python3 tools/compare.py "$@"

# Set up build/game: game data, patched th08.exe and th08.dll
game: build _orig
    mkdir -p build/game
    cp -r orig/th08_100d/. build/game/
    rm build/game/th08.exe
    python3 tools/patch_exe.py {{ exe }} build/game/th08.exe
    cp build/th08.dll build/game/

# Run the patched game headless with STEPS (wait:S key:K hold:K:S shot:NAME)
[positional-arguments]
run *steps: game
    python3 tools/play.py "$@"

# Link the standalone build/th08.exe (no DLL); with STEPS, run it like `run`
[positional-arguments]
exe *steps: game
    python3 tools/build.py exe
    cp build/th08.exe build/game/th08_standalone.exe
    if [ $# -gt 0 ]; then python3 tools/play.py --exe th08_standalone.exe "$@"; fi

# Analyze the original with headless Ghidra; add new functions to symbols.csv
analyze: _orig
    tools/ghidra/analyze.sh
    python3 tools/seed_symbols.py

# Decompile ADDRs (default: every function) with Ghidra into build/decomp/
decompile *addrs:
    tools/ghidra/decompile.sh {{ addrs }}

# Render docs/progress.svg from symbols.csv and the build
progress: build _orig
    python3 tools/render_progress.py

# Use tools/hooks/: each commit redraws and includes docs/progress.svg,
# and its message must be 50/72
hooks:
    git config core.hooksPath tools/hooks

# Remove build outputs (keeps the Ghidra project)
clean:
    rm -rf build/obj build/src build/game build/th08.* build/th08.*

# Verify the original and copy it into build/
_orig:
    cd orig && sha1sum --quiet -c th08_100d.sha1
    mkdir -p build && cp orig/th08_100d/th08.exe {{ exe }}
