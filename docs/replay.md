# Replay parity

The shared runner from th06 is available through `just replay`. th08's
format and symbols live in `scripts/replay_hook.py`; its injected tracer
is `scripts/replay_trace.cpp`. The runtime, null renderer, Wine/Xvfb
isolation, validation, comparison and evidence format remain in
`tools/replay/`.

```sh
just exe
python3 tools/replay/replay.py fetch      # requires network access
just replay                             # all downloaded manifest entries
just replay replays/example.rpy          # a particular recording
python3 tools/replay/replay.py info replays/example.rpy
python3 -m unittest scripts.test_replay
python3 -m unittest discover -s tools/replay
```

Without a downloaded corpus, extract the four demo recordings shipped in
your own game archive, into ignored build output:

```sh
python3 scripts/replay_fixtures.py
just replay build/replay/fixtures/*.rpy
```

These are single-stage demo recordings (three of them made with ZUN's
debug build, one with 1.00a), which the title plays for a fixed number of
frames rather than to their end. Parsing them is useful for format
checks; they provide no full-route coverage. Do not commit extracted
game data. The extractor reads th08.dat as `DatArchive` does (the header
and table decrypted with `DecryptData`, the entries LZSS-compressed and
then decrypted by their kind). `fetch` uses the IN world record table
and retains source URLs, players, dates and hashes in the manifest, as
the shared harness does for th06 and th07.

## What is compared

By default every recording runs in the original twice, to check
repeatability, and in the standalone `build/th08.exe`. Each uses its own
symbol table: `config/symbols.csv` for the original and the link map for
the rebuild. The tracer validates the number of hook sites it replaces:
one call of `Chain::RunCalcChain` in `GameWindow::RunFrame`, one
`MainMenu::OnUpdate` in `SetUpTitleScene`, and in
`ReplayManager::RegisterChain` one each of `PlayOnUpdate`,
`PlayOnUpdate2` and `PlayAddedCallback` (plus its one direct call, for
the stages after the first) and two of `DeletedCallback` (recording's
and playing's). Hooked DLL builds are not supported.

The tracer starts playback after 60 updates of the idle title menu, which
in th08 is `MainMenu` state 0 (`StartTitle`), substate 1, once its
loading thread is done. It sets what the replay menu (state 7,
`UpdateReplayMenu`'s substate 3) sets: the replay flag, the file,
difficulty, shot type (`character`, 0 to 11), the spell card practice
fields, the first stage and `replayMenuChoice` 0 (no fast-forward). It
wraps th08's fastcall replay callbacks and records every replay
calculation, stage boundary, natural playback deletion and return to the
title, where th08 opens the replay menu again. It counts main-loop
passes separately from dialogue fast-forwards (`PlayOnUpdate2` returning
6). It exits only after the playback deletion and another 60 updates
with the title active; crashes, limits and timeouts fail.

th08 loads every stage in a second thread, `GameManager::LoadingThread`,
which also starts the replay's stage (`RegisterChain`,
`PlayAddedCallback`) while the main loop goes on with
`GameManager::OnUpdate` returning 3. For a replay the thread waits for
0x50 of those passes, but how many more there are depends on its timing,
and the main loop must keep running for it to finish (it steps the ANM
loading). So the trace's `passes` leaves out the passes that start in the
game while `GameManager::loadingThreadState` is set (OnUpdate sets it as
a stage ends, too), unless they reach `PlayOnUpdate` because the thread
finished meanwhile; `calcs` is `App::frames` without those passes. Stage
start and end records, which the loading thread writes, repeat the counts
of the calculation before. No game state the trace records depends on how
long the loading took: both original runs agree.

A trace includes score and shown score, lives, bombs, power, graze,
point items, the point item value (`maxPointValue`), stage clear points,
deaths, bombs used, spell cards captured, continues, the youkai/human
gauge and its copy, the night clock, time orbs (collected, stage total
and needed), the gauge frame counters, the human rate frames, rank and
subrank, the power item combo, `GameManager::stateFlags`, pause and
replay-menu state, RNG state, the replay's and the keyboard's buttons,
the input consumed from the file, player position, state, focus,
youkai and last word state, the running bomb, the spell card's flags,
number, bonus and time orb bonus, entity counts and hashes of active
bullets, lasers, enemies, items and player shots. Float values are
stored as their bits. Trace scores and stage validation use the game's
units of ten points. `info` displays actual points. After returning to
the title, newly allocated GameManagerStats contains uninitialized heap
data; title records leave its fields zero.

## The format

The parser checks what `ValidateReplay` checks and more:

- "T8RP" and version 6; replays "with seeds" (`ReplayHeader::withSeeds`,
  6-byte inputs that playback reseeds the RNG from) are not supported.
  `RecordAddedCallback` clears the flag, so no saved replay has it.
- The file's first 0x68 bytes are stored as they are, but from
  `compressedSize` (0x18) to `size` they are offset by a key (byte 0x15)
  growing by 7. The checksum is 0x3f000318 plus the decoded bytes from
  0x15 to `size`. After 0x68, `compressedSize` bytes of LZSS data hold
  the rest of the 0x134-byte header, the stages and their frame rates,
  `dataSize` bytes decompressed. The "USER" text section after `size` is
  kept but not checked: the game does not read it either.
- A score-disabled config makes `ValidateReplay` refuse the replay, so
  the parser does too. The recording's version and executable identity
  are checked by the game against `th08_0100d.ver` (which lists versions
  back to 0.09); `info` shows the version. Every world record replay is
  1.00d's.
- Nine stage offsets, indexed by `GameManager::stage` from 0: stages 1,
  2, 3, 4A, 4B, 5, 6A, 6B and Extra. A stage is a 0x24-byte
  `ReplayStage` (the score at its end, the stats and the RNG seed it
  starts with) and one WORD of buttons per input; th07 kept an event word
  too. As in th07, `SaveReplay` writes all stages before all frame rate
  streams, so the last stage ends where the first stream starts.

`RecordAddedCallback` starts every stage with an empty input, which
playback plays first: recording writes a calculation's input after the
player used it (priority 0x11), playback reads it before (priority 6).
`EndStage` (from the GameManager's deleted callback) writes an empty
input after the last one, but only the final stage keeps it: for the
others, the stage's end is set after `GameManager::stage` moved on, so
the stage ends with its last recorded input. `SaveReplay` calls
`EndStage` again, so the final stage ends with two empty inputs and two
uninitialized words. The parser verifies the initial and the two marker
inputs, not the tail.

`RecordOnUpdate` records the input of the calculation in which the stage
clear starts (Gui's message script sets `stateFlags` bit 9, between the
two priorities) and of the next two; `PlayOnUpdate` stops playing as
soon as the bit is set. So for a stage of N recorded inputs, playback
should play the initial empty input and N - 3 recorded ones: N - 2
calculations. 17 of the 59 world records do, in every stage. The other
42 play one more, N - 1, again in every stage: their stage clears start
a calculation later in playback than they did when recording. Both executables and both
original runs agree on every one of them, with the recorded score at
every stage end. Which recordings do this is not known: it does not
follow the shot type, and only partly the difficulty (all 12 Easy
recordings play N - 1, 10 of 12 Extra ones N - 2). The file does not
say, so the validator accepts both endpoints for every stage (world
record runs end every stage, the last one included, with a clear), and
the hook checks what makes the endpoint the game's: the input cursor
stays where the last calculation left it until the stage ends, and the
trace's `clearFrame` (the replay frame at which `PlayOnUpdate` first saw
the stage clear) is that last calculation's.

A passing run must reach that endpoint, finish each stage with its
recorded score, play the stages in order and return naturally to the
title. An identical prefix or a forced early exit is not a pass.

`build/replay/<case>/evidence.json` contains the run checks, hashes and
complete trace comparisons. `scripts/audit_replays.py` revalidates saved
runs with the current parser, verifies the recorded hashes of the replay,
executable, instrumented executable, tracer and trace, and compares
complete traces. It writes a separate ignored audit report without
changing the raw run assessments:

```sh
python3 scripts/audit_replays.py          # original, original+, standalone
```

## Instrumentation and limits

The runner modifies a copy of each executable's import table. The
tracer installs the shared clock and null renderer, then makes the
performance counter use that same clock with a 1000 Hz frequency so
th08's main-loop pacing (`GameWindow::GetTime`) can accelerate too.
`--speed 1 --render` uses real-time pacing and drawing; `--speed 0` is
the default accelerated run. Audio is disabled. Rendering and audio
output are not compared; game draw callbacks still run.

Adding an import changes the executable checksum. Before the first
update, the tracer restores App's executable size/checksum fields to the
verified original's identity. This preserves the periodic version-file
check (every 4000 frames) while permitting instrumented/rebuilt
executables to run. The replay's embedded version and checksum are still
checked by the game. The run records the actual executable and
instrumented file hashes.

The run's config is windowed, with no music, sound or frame skip.
Writable `score.dat`, configuration and replay files are private copies;
setup removes the shared runner's symlinks for these paths.

## Validation status

The standalone executable is byte-identical to the original (SHA-1
`2044fea52667be4f61bcd78efc8f7d63fbbc244f`; `just check-exe` enforces
whole-file equality, see [executable parity](exe-parity.md)), so both
runs use the same code and differ only in their symbol tables:
`config/symbols.csv` and the link map, which put every hooked function
and global at the same address.

**All 59 recordings pass and match.** Each runs once in the standalone
executable and twice in the original; every run passes complete-route
validation, and the 59 complete trace files are byte-identical to both
original runs: 7,564,936 records and 438,766,288 fields per
original-versus-rebuild comparison, covering 7,515,092 gameplay
calculations in each executable. This includes the natural return to
the title. No recording fails in the original, unlike th07's ten. The
saved-artifact audit (`python3 scripts/audit_replays.py`, written to
`build/replay/audit.json`) revalidates all 177 runs and verifies every
replay, executable, patched executable, tracer and trace hash.

| Difficulty | Recordings passing and matching |
| --- | ---: |
| Easy | 12 / 12 |
| Normal | 12 / 12 |
| Hard | 11 / 11 |
| Lunatic | 12 / 12 |
| Extra | 12 / 12 |

Remilia Hard is the one missing record: the table's link served an
empty file on 2026-11-05.

An earlier tracer read `App::scene` in the stage records, which the
loading thread writes while the main loop moves the scene from 3 to 2:
nine of 59 recordings then differed in that field alone, between the two
original runs as often as against the rebuild. Stage records leave it
out now.

The four demo recordings in th08.dat are rejected, as expected: the
title plays them as demos for a fixed time, but run as replays they go
on past their recorded inputs (and two lose the recorded score).

The Python suites pass **32 tests**: 24 project tests and eight shared
harness tests. They cover encoded replays, checksums, multi-stage and
Extra layouts, the start and marker inputs, both stage-clear endpoints,
corrupt/incomplete traces, private writable files, actual binary hook
sites in both executables, concurrent hook loading, the archive
decryption and the bundled demos, and `fetch`'s parsing of spaced shot
names and empty downloads. The tracer is compiled with the original
MSVC toolchain.

Trace equality and full-route coverage are separate results. An identical
comparison or a parsed replay file alone does not establish route coverage.
