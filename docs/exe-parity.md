# Executable parity

The standalone build achieves **100% whole-file byte identity**: **all
840,704 bytes match** retail th08 1.00d, with zero differing bytes. Both
`build/th08.exe` and `build/th08_100d.exe` have SHA-1
`2044fea52667be4f61bcd78efc8f7d63fbbc244f`, as in `orig/th08_100d.sha1`.

```sh
just check-exe            # builds the exe, then requires exact equality
just compare              # ordinary-object function comparison
just compare-exe          # detailed headers/sections/symbol report
python3 scripts/check_exe_layout.py   # after just exe: every recorded address
```

`just check-exe` (decomp-tools' `project.py check-exe`) compares every
byte and the file lengths. It exits nonzero on any mismatch, including
headers, padding, tables and metadata; it does not mask addresses.
Ordinary comparison passes **1130/1130 functions, 460,204 bytes**
(counting separately emitted inline copies and folded symbols). All
**1116 game functions** sit at their original addresses and match
**raw**, as do the C runtime's 199 named functions. Every section, the
import tables, the PE headers and the Rich header match exactly. Replay
validation is recorded in [replay parity](replay.md).

`scripts/check_exe_layout.py` checks the layout by name rather than by
byte: all 1116 named game functions, all 136 named writable globals
(including function statics absent from the public link map, which it
resolves through the linked COFF relocations) and all 1143 readonly
COMDAT symbols of `config/exe_constants.csv` are at their original
addresses.

This is executable identity, **not a claim to have recovered the
original source**. The build uses the original compiler and linker
and the documented layout aids below. No instruction or data bytes are
copied from the game executable; the build takes only the original's
resources and its link timestamp, the one header field the linker has
no option for. Feature builds (`FEAT=orig_checksum`) are not the
identical one.

## What the final build retains

- **VS.NET 2002's prerelease DirectX libraries.** The game code is
  compiled with DirectX 8.0's headers (its `D3DXIMAGE_INFO` has 8.0's
  20 bytes, not 8.1's 28), but it links the DirectX libraries Visual
  Studio .NET 2002 itself ships in `Vc7\PlatformSDK\lib\prerelease`
  (the `vs_setup.msi` merge module `zDxF_pre_b`). Their d3dx8.lib
  objects are build 9177 and their d3d8, dinput8 and dsound import
  libraries Implib700 9111, as the original's Rich header records; the
  DirectX 8.1 SDK's say 9178 and 9210. Their Intel-compiled D3DX8
  objects also bring the compiler banners that fill the original's
  0x1e00 bytes of headers, which an earlier build faked with a padded
  DOS stub. The Platform SDK import libraries are VS.NET 2002's own
  too (`Vc7\PlatformSDK\Lib`, build 9210), as in th06 and th07.
- **`/ORDER` placement.** `scripts/build_hook.py` generates
  `build/exe_order.txt` from `config/symbols.csv`,
  `config/exe_constants.csv` and the compiled objects, and the link
  passes it with `/ORDER`. It places every game function and the
  compiler's readonly constant and string COMDATs at their original
  addresses; folded aliases are all listed, since the linker orders the
  ICF representative. This is a layout reconstruction aid and does not
  establish that the original build used `/ORDER`.
- **`config/exe_constants.csv`.** 1143 compiler constant/string COMDAT
  addresses, recovered by `scripts/recover_exe_constants.py` from the
  original's instruction operands and initialized tables, following our
  objects' COFF relocations. Unreferenced string literals need a unique
  byte match in the original's `.rdata`. After a vanilla build the
  script regenerates the file; no executable bytes go into the build
  inputs.
- **Data layout.** Each object's `.bss` is laid out by the front end's
  hash of the names (`tools/locals.py`), so some globals are named for
  their slot, as in th06. Identifier, `selectany` and function-static
  choices reproduce MSVC's data layout; the game's constant tables are
  `const` in `.rdata`; `/GF` pools string literals and `/Gy` makes every
  function a COMDAT, as the original's 16-byte aligned, int3-padded
  functions show.
- **Import load and IAT order.** The import name table lists imports in
  the order the linker loads their import objects, and libcmt's members
  lie in the order they load; both follow the order objects first
  mention a function, including in code `/OPT:REF` dropped. Exe-only
  `Unused*` functions mention what the original's dropped code did:
  GetVersionExA after WriteToFile (as in th07); sqrt, tan and malloc
  ahead of stage.cpp's MemoryManager copies; a rethrow in ending.cpp
  that loads throw.obj, and with it RaiseException and unhandld.obj,
  early (as th06 found); CreateFontA, SetBkMode and SetTextColor in the
  text object; midiOutOpen before MidiDevice::Close. A function's
  references come out in reverse, so each mention goes where it must
  fall, and they sit in `.text$u`, outside the `/OPT:ICF` sort. The
  import address table also depends on import objects the original
  loaded and then dropped: link sorts every loaded import object by DLL
  name with msvcr70's quicksort. The Rich header counted 178 import
  objects to our 172; more `Unused*` mentions (midiOutGetNumDevs and
  SetThreadPriority at the end of midi.cpp, DrawTextA and ReleaseDC in
  the text object as th06's UnusedDrawText, and six resource functions
  at the end of archive.cpp) restore all 178, the IAT and all 154
  import names.
- **C runtime order.** The C runtime's objects load in the original's
  order. The two lookups call the old names `strcmpi` and `stricmp`,
  which OLDNAMES.lib resolves after libcmt's first pass, so
  stricmp.obj, tolower.obj and _stricmp.obj load where the original's
  do and the Rich header counts the original's three alias objects
  (39/9162); the code is the same, since both name `_stricmp`. The
  unused archives are defined after File's constructor, so their
  vector constructor/destructor iterators no longer load ehvecctr.obj
  and ehvecdtr.obj before purevirt.obj. An exe-only `UnusedMoveBits`
  mentions memmove in the text object, as in th06, so memmove.obj loads
  right after strlen.obj; with it the C runtime and the entry point lie
  at their original addresses. An exe-only mention of
  `CLSID_ShellLink` before ResolveShortcut loads uuid.lib's
  shguids.obj before objidl_i.obj, which puts the shell link GUIDs in
  front of `IID_IPersistFile` in `.rdata`.
- **`/OPT:ICF` fold steering.** ICF sorts every code COMDAT, those
  `/OPT:REF` drops later included, with msvcr70's quicksort by
  relocation count and size, and keeps the first copy of each identical
  class. Links traced under winedbg with th07's `icftrace.py`, whose
  emulator predicts the classes, showed which copies differed: the
  empty functions (ours kept fpinit's `__fpclear`, the original
  D3DX8's cd3dxcodec copy), `zcfree` (jmemnobs' against zutil's),
  d3dxmath.obj's init stub, the SSE against the SSE2 copies that
  d3dxmathsse.obj and d3dxmathsse2.obj share, and the codec folds. The
  original's choices come from exe-only dead functions found with the
  emulator, by annealing and by a native search over the ICF input: a
  24-byte function with two relocations after
  `AsciiText::DeletedCallback`, `UnusedMixClock` in gui.cpp, two
  five-byte naked jumps tied with the codec destructors
  (`UnusedJumpToUpdateShooting`, `UnusedJumpToTextSurface`) and a
  152-byte function with three relocations in resultscreen.cpp, plus a
  few functions defined in a different order in their files
  (`StartNameOnly`, `StartResurrection`, `App::OnDraw`,
  `LzssTreeInitNode`, `EffectManager::Reset`, `MidiPlayer::FadeOut`),
  whose addresses `/ORDER` keeps.
- **The 63-object split.** The Rich header counts the original's 63
  C++ objects (Utc13_CPP 9466), and the source is split into 63
  objects. Each new file's first lines say what its seam rests on:
  - *Shown by the original's bytes.* The bomb code ends an object with
    a run of the inline copies only its own block uses four times,
    after the dissolve bomb and after Alice's, Marisa's and Sakuya's
    code, with the blocks in alphabetical order of the character:
    bomb_alice.cpp, bomb_marisa.cpp, bomb_remilia.cpp and
    bomb_yukari.cpp start there.
  - *A pattern from those.* bomb_sakuya.cpp, bomb_youmu.cpp and
    bomb_yuyuko.cpp follow the same per-character, alphabetical
    pattern, at plausible seams nothing in the binary shows.
  - *th06's or th07's objects for the same code and data.* log.cpp,
    rng.cpp and utils.cpp (as th06 splits its system.cpp),
    mididevice.cpp and mmtimer.cpp, textsurface.cpp and
    enemyscriptpattern.cpp follow th06; filemodes.cpp and the data unit
    unsorted18d7f68.cpp, which holds the unused word ending the game's
    `.bss`, follow th07.
  - *Neither.* app.cpp, spritedraw.cpp and practicebgm.cpp have no
    precedent in th06 or th07; their only basis in th08's bytes is
    where an out-of-line copy (App::EnterCriticalSection,
    ModulateChannel) or a run of inline copies sits.

  The splits are byte-neutral apart from the Rich header's object count,
  which reached the original's 63 with the last one; with it the exe
  became the original byte for byte. The original's precompiled-header
  layout remains unknown.

One functional error was fixed along the way, in the key configuration:
the original's jump table sends cursor 3 to the skip button's body and
cursors 4 to 8 to the menu, up, down, left and right bodies, since the
menu lists Skip before Pause. Ours, in numeric order, set the wrong
button for each of those cursors. Correcting the case labels changes
only the jump table, which function comparison masks.

## Earlier checkpoints

The measurements below are historical: `just compare-exe`'s identical
share of the whole file after each step, as the commit messages record
it. They explain how the matching build was reached.

| Step | Whole file |
| --- | ---: |
| Link from our objects with the game's libraries (`408fe7e`) | 2.90% |
| DirectX 8.1 SDK libraries (`68ea959`) | 4.66% |
| `/OPT:NOWIN98`: the original's 0x200 file alignment (`1aa5f19`) | 4.09% |
| `/OPT:REF` and `/OPT:ICF` (`a915c5c`) | 4.06% |
| A DOS stub padding the headers to 0x1e00 bytes (`088c4e9`) | 5.00% |
| `/Gy` (`61c9a63`) | 15.36% |
| `/GF` (`e2ca88f`) | 16.46% |
| Constant tables in `.rdata` (`28686e4`) | 16.66% |
| Globals renamed for their `.bss` slots (`d7ef615`) | 16.70% |
| `/ORDER` placement, then the prerelease DirectX libraries (`f422017`) | 60.5% → 83.5% |
| Imports mentioned in the original's load order (`b80036b`) | 87.9% |
| The import objects the original dropped (`0a1c033`) | 88.7% |
| The original's copies of two folded functions (`8e666ba`) | 93.5% |
| memmove mentioned in the text object (`61ff5cd`) | 98.6% |
| D3DX8's math folds (`0de6488`) | 98.9% |
| D3DX8's SSE folds and private BSS placement (`2489f3d`) | 99.73% |
| D3DX8's codec folds (`fdd6ff2`) | 99.98% |
| The 63rd object (`0ed185b`) | **100%** |

`f422017`'s message gives 60.5% → 83.5% for its library change, after
which every section was at the original's address and size; it gives
no figure for the `/ORDER` placement alone. After `fdd6ff2` every
section was identical and the 132 differing bytes were the Rich
header's, which the third alias object (`ff46fff`) and the object
splits then made the original's.
