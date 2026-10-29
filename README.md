# th08

A from-scratch matching decompilation of 東方永夜抄 ~ Imperishable
Night (Touhou 8), built and verified on Linux through Wine.

The goal is C++ source that compiles, with the original toolchain, into a
binary that is byte-for-byte identical to the original executable.

This repository contains no game code or data. You need your own copy of
the game, see [`orig/README.md`](orig/README.md).

## Clean room

This is an independent, clean-room implementation. It is written from
the binary alone: no code, names, struct layouts or function lists are
taken from other decompilations or ports of Touhou games. Names come from
our own analysis or, for library code, from the libraries themselves.

## Status

The toolchain and workflow are up, taken from the th07 project, and the
first function matches.

Ghidra finds 2225 functions. As in th06 and th07, the game code is mostly
compiled without optimization, so its functions start with `push ebp;
mov ebp, esp`, with a few objects optimized for size; the statically
linked D3DX8 and CRT code is optimized. That puts the game code at
`0x402000`–`0x475298`: 1116 functions, about 460 KB. D3DX8's tables
follow it, then the DirectX import thunks, D3DX8 and the CRT.

```sh
just analyze   # writes build/functions_100d.csv
```

![Progress](docs/progress.svg)

`just progress` renders it: every game function, by address and as a
treemap by size, coloured by whether it is decompiled and matching.
`just hooks` makes every commit redraw it and include it.

## Target

Retail 1.00d, released 2004-09-19, the last official update:

- 32-bit PE, image base `0x400000`, entry point `0x4a619e`, `.text` is
  0xb1b78 bytes and starts at `0x402000`, a page later than th06 and
  th07
- `.data` is 0x1413840 bytes in memory but only 0x6e00 on disk, so most
  of it is zero-initialized globals. A `.data1` section of 0x8e0 bytes
  follows it.
- Imports `d3d8`, `DSOUND`, `DINPUT8`, `WINMM`, `GDI32`, `USER32`,
  `KERNEL32`, `ADVAPI32` (registry reads) and `ole32`. Only
  `Direct3DCreate8` is imported from D3D, so D3DX8 is statically linked.

## Toolchain

Read from the Rich header (`tools/peinfo.py`):

| Tool | Build | Objects | Probably |
| --- | --- | --- | --- |
| C++ compiler 13.00 | 9466 | 63 | Game code |
| C compiler 13.00 | 9466 | 130 | MSVC CRT |
| MASM 7.00 | 9466 | 48 | MSVC CRT |
| C++ compiler 13.00 | 9177 | 69 | Static libraries, probably D3DX8 |
| C compiler 13.00 | 9177, 9178 | 3, 3 | Static libraries |
| Import library 7.00 | 9210 | 12 | DirectX SDK import libraries |
| Import library | 9111 | 7 | |
| Linker 7.00 | 9466 | | |

This is the th06 and th07 toolchain: Visual C++ .NET 2002 (MSVC 7.0,
`cl` 13.00.9466) with the DirectX 8 SDK. Unlike th07, no object was
compiled with `/GL`. The static libraries come from a different build
(9177) than th07's (8830). The compiler flags start as th07's and change
as functions show otherwise.

## Workflow

Everything runs headless, so it works over SSH. `just` lists the recipes.
`tools/` is a submodule: clone with `--recursive`, or run
`git submodule update --init tools` after cloning.

1. Analyze the binary with Ghidra in headless mode (`just analyze`).
   New functions are added to `config/symbols.csv` with placeholder names
   (`Func43be50`); library functions keep the names Ghidra's function ID
   gives them.
2. Write C++ for a function in `src/`, in the style of 2004-era code.
   `just decompile [ADDR...]` writes Ghidra's pseudocode to
   `build/decomp/<address>.c` to read next to the disassembly.
3. Record the function and every original function and global it
   uses in [`config/symbols.csv`](config/symbols.csv): address, size, our
   name, and the mangled name, which the link error for an unresolved
   external gives.
4. Compare (`just compare`): builds with `cl.exe` under Wine and
   checks each function against the original byte for byte. Calls and
   global addresses are resolved through `symbols.csv` and compared too;
   only references to unnamed data such as string literals are masked.
   `just compare SYMBOL` shows a disassembly diff.
5. Play (`just run`): runs the original game with the new code hooked
   in, headless under Xvfb, to check that the code behaves correctly as
   well as matching. Steps drive it and take screenshots, e.g.
   `just run wait:12 key:Escape key:z wait:2 shot:menu` writes
   `build/shots/menu.png`.
6. Standalone (`just exe`): links the same objects, without the hook
   DLL, into `build/th08.exe`, a game that needs nothing from the
   original executable. It needs the game's data and global objects in
   `src/exe/`. `just exe STEPS` runs it like `just run`.

### Hooking

The decompiled code runs inside the original game. `tools/patch_exe.py` adds
one import, `th08.dll`, to a copy of the executable. The DLL is
built from the same objects that `just compare` checks. On load it
overwrites each decompiled function in the original with a jump to the
new code.

Calls from new code to functions that are not decompiled yet go through
generated thunks (`push address; ret`) to the original. Globals are
absolute symbols at their original addresses. Both are generated from
`symbols.csv`. The DLL has no C runtime of its own: CRT functions belong
in `symbols.csv` too, so the game and the new code share one heap.

## Layout

```
orig/     your game files (not tracked) and their checksums
src/      decompiled source; src/dll/ is the hook DLL's own code,
          src/exe/ the game's data and globals for the standalone exe
config/   symbols.csv: address, size, name and mangled symbol
tools/    build, compare, exe patcher, symbol seeding, Ghidra scripts:
          the decomp-tools repository shared with th06 and th07, as a
          git submodule (just tools-init, tools-pull, tools-push)
project.toml  th08's settings for tools/
build/    outputs; build/game/ is a runnable copy of the patched game
```

## Requirements

- Wine (32-bit), Python 3, [just](https://github.com/casey/just),
  binutils (`objdump`), 7-Zip
- For `just run`: Xvfb, xdotool, ImageMagick, 32-bit Mesa OpenGL
  (`libgl1-mesa-dri:i386`, `libglx-mesa0:i386`) and the `ja_JP.UTF-8`
  locale
- The original toolchain in the Wine prefix at `.wine/` (bring your own
  copies)
- Ghidra (for analysis, optional)

## Legal

Touhou Project and 東方永夜抄 are by ZUN / Team Shanghai Alice. This is an
unofficial fan project. It contains no assets or code from the original
game, and it is not affiliated with or endorsed by ZUN or Team Shanghai
Alice.

## License

The source in this repository is licensed under the GNU General Public
License, version 3 only (`GPL-3.0-only`), see [`LICENSE`](LICENSE). The
license covers our code, not the original game or its data.
