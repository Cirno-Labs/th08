"""th08's T8RP format and setup for the shared replay parity harness."""
from __future__ import annotations

import os
from pathlib import Path
import struct
from collections.abc import Callable, Iterator
from typing import TypedDict

ROOT = Path(__file__).resolve().parents[1]
TRACER = ["scripts/replay_trace.cpp"]
WR_TABLE = "IN_table"
FUNCTIONS = {
    "RunFrame": "?RunFrame@GameWindow@@QAEHXZ",
    "RunCalcChain": "?RunCalcChain@Chain@@QAEHXZ",
    "SetUpTitleScene": "?SetUpTitleScene@@YIHH@Z",
    "MenuOnUpdate": "?OnUpdate@MainMenu@@SIHPAV1@@Z",
    "RegisterChain": "?RegisterChain@ReplayManager@@SIHHPAD@Z",
    "PlayOnUpdate": "?PlayOnUpdate@ReplayManager@@SIHPAV1@@Z",
    "PlayOnUpdate2": "?PlayOnUpdate2@ReplayManager@@SIHPAV1@@Z",
    "PlayAddedCallback": "?PlayAddedCallback@ReplayManager@@SIHPAV1@@Z",
    "DeletedCallback": "?DeletedCallback@ReplayManager@@SIHPAV1@@Z",
}
SIZED = ["RunFrame", "SetUpTitleScene", "RegisterChain"]
GLOBALS = {
    "GameManager": "?g_GameManager@@3VGameManager@@A",
    "App": "?g_App@@3VApp@@A",
    "Random": "?g_Random@@3VRng@@A",
    "Buttons": "?g_Buttons@@3GA",
    "PlayButtons": "?g_PlayButtons@@3GA",
    "Player": "?g_Player@@3VPlayer@@A",
    "BulletManager": "?g_BulletManager@@3VBulletManager@@A",
    "EnemyManager": "?g_EnemyManager@@3VEnemyManager@@A",
    "ItemManager": "?g_ItemManager@@3VItemManager@@A",
    "SpellCard": "?g_SpellCard@@3USpellCard@@A",
}
# GameManager::character, as the world record table names the shot types.
SHOTS = ["Border Team", "Magic Team", "Scarlet Team", "Ghost Team", "Reimu", "Yukari",
         "Marisa", "Alice", "Sakuya", "Remilia", "Youmu", "Yuyuko"]
DIFFICULTIES = ["Easy", "Normal", "Hard", "Lunatic", "Extra", "Last Word"]
# GameManager::stage, from 0 (g_StageNames).
STAGES = ["1", "2", "3", "4A", "4B", "5", "6A", "6B", "Extra"]
CONFIG = "th08.cfg"
REPLAY_PATH = "replay/harness.rpy"
HEADER = 0x68  # stored as is; the rest is compressed
DECODED = 0x134  # sizeof(ReplayHeader)
STAGE_HEADER = 0x24  # ReplayStage up to its frames
# RecordOnUpdate records the input of the calculation the stage clear
# starts in (Gui sets stateFlags bit 9, after PlayOnUpdate's priority
# but before RecordOnUpdate's) and of the next two; PlayOnUpdate plays
# none of them.
CLEAR_FRAMES = 3


class Stage(TypedDict):
    score: int  # native game units: ten points
    frames: int
    frame_counts: list[int]
    entries: int
    seed: int
    lives: int
    bombs: int
    power: int
    rank: int


def decompress(data: bytes | bytearray, size: int) -> bytes:
    """The game's MSB-first LZSS (src/lzss.cpp), with bounded output."""
    if not 0 <= size <= 32 * 1024 * 1024:
        raise ValueError("invalid LZSS output size")
    # The compressor omits its partially filled last byte. The game
    # supplies zero bytes at EOF, completing the zero end marker.
    data = bytes(data) + bytes(2)
    bit = 0

    def read(count: int) -> int:
        nonlocal bit
        if bit + count > len(data) * 8:
            raise ValueError("truncated LZSS stream")
        value = 0
        for _ in range(count):
            value = (value << 1) | ((data[bit // 8] >> (7 - bit % 8)) & 1)
            bit += 1
        return value

    window = bytearray(0x2000)
    pos = 1
    out = bytearray()
    while True:
        if read(1):
            chunk = [read(8)]
        else:
            source = read(13)
            if source == 0:
                break
            count = read(4) + 3
            # Copies may overlap their source in the ring buffer.
            chunk = []
            for i in range(count):
                value = window[(source + i) & 0x1fff]
                chunk.append(value)
                window[(pos + i) & 0x1fff] = value
        if len(out) + len(chunk) > size:
            raise ValueError("LZSS output exceeds declared size")
        for value in chunk:
            out.append(value)
            window[pos] = value
            pos = (pos + 1) & 0x1fff
    if len(out) != size:
        raise ValueError("LZSS output does not match declared size")
    return bytes(out)


def decode(file: bytes | bytearray) -> tuple[bytes, bytes]:
    """ValidateReplay's checks and decoding: the decoded replay (header
    and data) and the user section after it."""
    data = bytearray(file)
    if not data:
        raise ValueError("empty file")
    if len(data) < HEADER or data[:4] != b"T8RP":
        raise ValueError("not a th08 replay")
    version, with_seeds = struct.unpack_from("<HB", data, 4)
    if version != 6:
        raise ValueError(f"unsupported version {version:#x}")
    if with_seeds:
        # Saved replays never are: RecordAddedCallback clears it.
        raise ValueError("unsupported replay with seeds")
    size, checksum = struct.unpack_from("<iI", data, 0xc)
    if not HEADER <= size <= len(data):
        raise ValueError("inconsistent replay size")
    key = data[0x15]
    for i in range(0x18, size):
        data[i] = (data[i] - key) & 255
        key = (key + 7) & 255
    if (0x3f000318 + sum(data[0x15:size])) & 0xffffffff != checksum:
        raise ValueError("bad checksum")
    packed, unpacked = struct.unpack_from("<ii", data, 0x18)
    if packed != size - HEADER or unpacked < DECODED - HEADER:
        raise ValueError("inconsistent replay sizes")
    return bytes(data[:HEADER]) + decompress(data[HEADER:size], unpacked), bytes(data[size:])


class Replay:
    def __init__(self, path: str | os.PathLike[str]) -> None:
        self.path = Path(path)
        try:
            data, self.user = decode(self.path.read_bytes())
        except ValueError as exc:
            raise ValueError(f"{path}: {exc}") from None
        self.shot, self.difficulty = data[0x6a:0x6c]
        if self.shot >= len(SHOTS) or self.difficulty >= len(DIFFICULTIES):
            raise ValueError(f"{path}: invalid shot or difficulty")
        self.date = data[0x6c:0x72].split(b"\0")[0].decode("ascii", "replace")
        self.name = data[0x72:0x7b].split(b"\0")[0].decode("shift_jis", "replace").strip()
        self.practice = data[0x7b]
        self.spell_card = struct.unpack_from("<h", data, 0x7c)[0]
        self.score = struct.unpack_from("<I", data, 0xb0)[0] * 10
        self.config = bytes(data[0xb4:0xf0])
        self.slowdown = struct.unpack_from("<f", data, 0x118)[0]
        self.version = data[0x12c:0x132].split(b"\0")[0].decode("ascii", "replace")
        if self.config[0x25]:
            raise ValueError(f"{path}: score-disabled replay cannot play inputs")
        offsets = struct.unpack_from("<9I", data, 0x20)
        extras = struct.unpack_from("<9I", data, 0x44)
        boundaries = sorted({*offsets, *extras, len(data)} - {0})
        if any(offsets[i] and not extras[i] for i in range(9)):
            raise ValueError(f"{path}: a stage without frame rates")
        self.stages: dict[int, Stage] = {}
        for idx, off in enumerate(offsets):
            if not off:
                continue
            # SaveReplay writes ALL stages, then ALL frame rate streams,
            # as th07's.
            end = next((value for value in boundaries if value > off), len(data))
            extra_end = next((value for value in boundaries if value > extras[idx]), len(data))
            if not DECODED <= off < off + STAGE_HEADER + 2 <= end <= extras[idx] < extra_end <= len(data):
                raise ValueError(f"{path}: invalid stage {STAGES[idx]} offsets")
            if (end - off - STAGE_HEADER) % 2:
                raise ValueError(f"{path}: misaligned stage inputs")
            entries = (end - off - STAGE_HEADER) // 2
            inputs = struct.unpack_from(f"<{entries}H", data, off + STAGE_HEADER)
            # RecordAddedCallback starts the stage with an empty input,
            # which playback plays first. The GameManager's deleted
            # callback (EndStage) ends a stage with another, but kept
            # only for the last one: earlier stages' ends are set after
            # GameManager::stage moved on. SaveReplay's EndStage adds
            # one more, and keeps two uninitialized words after it.
            final = off == max(offsets)
            padding = 5 if final else 1
            if entries <= padding + CLEAR_FRAMES or inputs[0] != 0:
                raise ValueError(f"{path}: missing initial empty input or gameplay frames")
            if final and inputs[-4:-2] != (0, 0):
                raise ValueError(f"{path}: missing terminal empty input")
            recorded = entries - padding
            # Recording writes an input after the player used it,
            # playback reads it before: playback plays the initial empty
            # input and the recorded ones but the last CLEAR_FRAMES.
            frames = recorded + 1 - CLEAR_FRAMES
            seed, power, lives, bombs, rank = struct.unpack_from("<HBBBB", data, off + 0x1a)
            self.stages[idx] = Stage(score=struct.unpack_from("<I", data, off)[0],
                                     frames=frames, frame_counts=[frames],
                                     entries=entries, seed=seed,
                                     lives=lives, bombs=bombs, power=power, rank=rank)
        if not self.stages:
            raise ValueError(f"{path}: no stages")

    @property
    def label(self) -> str:
        return f"{SHOTS[self.shot]} {DIFFICULTIES[self.difficulty]}"

    def describe(self) -> Iterator[str]:
        # ValidateReplay checks the recording executable against
        # th08_0100d.ver, which lists versions back to 0.09.
        yield (f"{self.name} on {self.date}, score {self.score}, slowdown {self.slowdown:.2f}%, "
               f"version {self.version}")
        for stage, record in self.stages.items():
            counts = " or ".join(map(str, record["frame_counts"]))
            yield (f"stage {STAGES[stage]} ({stage}): ends with {record['score'] * 10} after {counts} "
                   "replay calculations")

    def start(self) -> str:
        return f"{min(self.stages)},{self.difficulty},{self.shot},{self.spell_card}"


def config_bytes() -> bytes:
    """A th08.cfg: the default keys and counts, windowed, no music or
    sound, no frame skip."""
    cfg = bytearray(0x3c)
    struct.pack_into("<9h", cfg, 0, 0, 1, 2, 4, -1, -1, -1, -1, 3)
    struct.pack_into("<I2h", cfg, 0x14, 0x80001, 600, 600)
    # lives, bombs, 16-bit colour, music, sound, difficulty, windowed,
    # frame skip, effects quality, options cursor, shoot focuses, volumes
    cfg[0x1c:0x29] = bytes([2, 3, 0, 0, 0, 1, 1, 0, 2, 0, 0, 100, 80])
    struct.pack_into("<I", cfg, 0x38, 1)
    return bytes(cfg)


def setup(game_dir: str, replay: Replay) -> dict[str, str]:
    # The shared runner links DAT files and directories. score.dat is
    # writable, and replay/ may contain the user's own replays.
    score = Path(game_dir, "score.dat")
    if score.is_symlink():
        saved = score.read_bytes()
        score.unlink()
        score.write_bytes(saved)
    directory = Path(game_dir, "replay")
    if directory.is_symlink():
        directory.unlink()
    directory.mkdir(exist_ok=True)
    for name, data in ((CONFIG, config_bytes()), (REPLAY_PATH, replay.path.read_bytes())):
        path = Path(game_dir, name)
        path.unlink(missing_ok=True)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
    # The imported tracer changes the executable's checksum. Keep the
    # original identity for th08's periodic version-file check.
    original = ROOT / "build/th08_100d.exe"
    data = original.read_bytes()
    checksum = sum(struct.unpack_from(f"<{len(data) // 4 - 1}I", data)) & 0xffffffff
    return dict(TRACE_REPLAY=REPLAY_PATH, TRACE_START=replay.start(),
                TRACE_EXE_SIZE=str(len(data)), TRACE_EXE_CHECKSUM=str(checksum))


def stage_failures(stage: int, recorded: Stage, last: tuple[int, ...], end: tuple[int, ...],
                   get: Callable[[tuple[int, ...], str], int]) -> list[str]:
    failures = []
    consumed = get(last, "entry")
    if consumed != get(last, "replayFrame") or consumed not in recorded["frame_counts"]:
        failures.append("last calculation did not consume the recorded inputs")
    if get(end, "entry") != consumed:
        failures.append("stage played inputs after its last calculation")
    return failures
