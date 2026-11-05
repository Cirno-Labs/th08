"""Parser and harness integration checks, without Wine or network access."""
from __future__ import annotations

from pathlib import Path
from concurrent.futures import ThreadPoolExecutor
from threading import Event
from types import ModuleType, SimpleNamespace
import hashlib
import json
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

from scripts import replay_hook as hook
from scripts.replay_fixtures import ROOT, decrypt, extract

sys.path.insert(0, str(ROOT / "tools/replay"))
import replay


def literals(data: bytes) -> bytes:
    """Encode literals and the end marker independently of the decoder."""
    bits = "".join("1" + f"{value:08b}" for value in data) + "0" * 14
    bits += "0" * (-len(bits) % 8)
    return int(bits, 2).to_bytes(len(bits) // 8, "big")


def encode(data: bytearray, user: bytes = b"USER\x0c\0\0\0\0\0\0\0") -> bytes:
    """SaveReplay's encoding of a decoded replay."""
    packed = literals(data[0x68:])
    file = bytearray(data[:0x68]) + packed
    struct.pack_into("<i", file, 0xc, len(file))
    struct.pack_into("<ii", file, 0x18, len(packed), len(data) - 0x68)
    file[0x15] = 0x35
    struct.pack_into("<I", file, 0x10, (0x3f000318 + sum(file[0x15:])) & 0xffffffff)
    key = file[0x15]
    for i in range(0x18, len(file)):
        file[i] = (file[i] + key) & 255
        key = (key + 7) & 255
    return bytes(file) + user


# Five recorded inputs: playback plays the initial empty one and two.
INPUTS = [0, 1, 2, 3, 0x10, 0x11]
FINAL_TAIL = [0, 0, 0xcdcd, 0xcdcd]


def stage_block(score: int, inputs: list[int]) -> bytes:
    block = bytearray(0x24)
    struct.pack_into("<I", block, 0, score)
    struct.pack_into("<HBBBB", block, 0x1a, 0xabcd, 128, 2, 3, 16)
    return bytes(block) + struct.pack(f"<{len(inputs)}H", *inputs)


def decoded(difficulty: int = 1, stages: list[tuple[int, int, list[int]]] | None = None) -> bytearray:
    """A decoded replay: its stages (index, score, inputs), then a frame
    rate stream of two bytes for each."""
    if stages is None:
        stages = [(8 if difficulty == 4 else 0, 123, INPUTS + FINAL_TAIL)]
    data = bytearray(0x134)
    data[:4] = b"T8RP"
    struct.pack_into("<H", data, 4, 6)
    data[0x6a:0x6c] = bytes([4, difficulty])
    data[0x6c:0x72] = b"11/05\0"
    data[0x72:0x76] = b"TEST"
    struct.pack_into("<h", data, 0x7c, -1)
    struct.pack_into("<I", data, 0xb0, 123)
    data[0x12c:0x131] = b"0100d"
    for idx, score, inputs in stages:
        struct.pack_into("<I", data, 0x20 + 4 * idx, len(data))
        data += stage_block(score, inputs)
    for idx, _, _ in stages:
        struct.pack_into("<I", data, 0x44 + 4 * idx, len(data))
        data += bytes([60, 60])
    return data


FIELDS = [*replay.REQUIRED_FIELDS, "entry"]


def trace(score: int = 123, frames: int = 3, end_entry: int | None = None,
          title: bool = True, stage: int = 0) -> replay.Trace:
    records: list[tuple[int, ...]] = []

    def add(kind: int, **values: int) -> None:
        base = dict(kind=kind, seq=len(records), passes=len(records) + 10,
                    calcs=len(records) + 10, stage=stage)
        base.update(values)
        records.append(tuple(base.get(field, 0) for field in FIELDS))

    add(replay.STAGE_START)
    for frame in range(1, frames + 1):
        add(replay.CALC, replayFrame=frame, entry=frame, flags=1)
    # The stage clear's calculations play no inputs.
    add(replay.CALC, replayFrame=frames, entry=frames, flags=0x400)
    add(replay.REPLAY_END, replayFrame=frames, entry=frames if end_entry is None else end_entry, score=score)
    if title:
        add(replay.TITLE)
    return replay.Trace(FIELDS, "test", records)


class ReplayTest(unittest.TestCase):
    def setUp(self) -> None:
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.path = Path(directory.name, "test.rpy")
        self.path.write_bytes(encode(decoded()))

    def test_header_and_inputs(self) -> None:
        record = hook.Replay(self.path)
        self.assertEqual(record.label, "Reimu Normal")
        self.assertEqual(record.score, 1230)
        self.assertEqual(record.name, "TEST")
        self.assertEqual(record.version, "0100d")
        self.assertEqual(record.user[:4], b"USER")
        self.assertEqual(record.start(), "0,1,4,-1")
        self.assertEqual(record.stages[0], dict(score=123, frames=3, frame_counts=[3], entries=10,
                                               seed=0xabcd, lives=2, bombs=3, power=128, rank=16))

    def test_downloaded_corpus(self) -> None:
        manifest = ROOT / "replays/manifest.json"
        if not manifest.exists():
            self.skipTest("downloaded replay corpus is optional")
        data = json.loads(manifest.read_text())
        entries = data["replays"]
        self.assertEqual(len(entries), 59)
        self.assertEqual([(s["shot"], s["difficulty"]) for s in data["skipped"]], [("Remilia", "Hard")])
        self.assertEqual(len({(e["shot"], e["difficulty"]) for e in entries} |
                             {(s["shot"], s["difficulty"]) for s in data["skipped"]}), 60)
        for entry in entries:
            path = ROOT / "replays" / entry["file"]
            with self.subTest(replay=path.name):
                self.assertEqual(hashlib.sha1(path.read_bytes()).hexdigest(), entry["sha1"])
                record = hook.Replay(path)
                self.assertEqual(record.label, f"{entry['shot']} {entry['difficulty']}")
                self.assertEqual(record.score, entry["score"])
                self.assertEqual(list(record.stages), [8] if entry["difficulty"] == "Extra" else
                                 [0, 1, 2, 3 if 3 in record.stages else 4, 5, 6 if 6 in record.stages else 7])
                self.assertTrue(all(stage["frames"] > 0 for stage in record.stages.values()))

    def test_multiple_stages(self) -> None:
        # Only the last stage gets the EndStage markers.
        data = decoded(stages=[(0, 123, INPUTS), (1, 456, INPUTS + [4] + FINAL_TAIL)])
        self.path.write_bytes(encode(data))
        record = hook.Replay(self.path)
        self.assertEqual(list(record.stages), [0, 1])
        self.assertEqual([stage["score"] for stage in record.stages.values()], [123, 456])
        self.assertEqual([stage["frames"] for stage in record.stages.values()], [3, 4])

    def test_extra_is_stage_eight(self) -> None:
        self.path.write_bytes(encode(decoded(4)))
        record = hook.Replay(self.path)
        self.assertEqual(record.label, "Reimu Extra")
        self.assertEqual(list(record.stages), [8])
        self.assertEqual(record.start(), "8,4,4,-1")

    def test_bad_checksum(self) -> None:
        data = bytearray(self.path.read_bytes())
        data[0x80] ^= 1
        self.path.write_bytes(data)
        with self.assertRaisesRegex(ValueError, "checksum"):
            hook.Replay(self.path)

    def test_user_section_is_not_checked(self) -> None:
        self.path.write_bytes(encode(decoded(), user=b"anything"))
        self.assertEqual(hook.Replay(self.path).user, b"anything")

    def test_truncated_file(self) -> None:
        for length in (0, 4, 0x67, 0x68, 100):
            self.path.write_bytes(encode(decoded())[:length])
            with self.subTest(length=length), self.assertRaises(ValueError):
                hook.Replay(self.path)

    def test_invalid_structure(self) -> None:
        # Shot, difficulty, score disabled, seeds, version, stage and
        # frame rate offsets.
        for offset, value in [(0x6a, 12), (0x6b, 6), (0xb4 + 0x25, 1), (0x6, 1), (0x4, 7), (0x20, 0x10),
                              (0x44, 0)]:
            data = decoded()
            data[offset] = value
            self.path.write_bytes(encode(data))
            with self.subTest(offset=offset), self.assertRaises(ValueError):
                hook.Replay(self.path)

    def test_hook_is_published_only_after_loading_finishes(self) -> None:
        entered, release = Event(), Event()
        loaded = ModuleType("replay_hook")

        def load(module: ModuleType) -> None:
            entered.set()
            if not release.wait(5):
                raise TimeoutError("test hook was not released")
            setattr(module, "ready", True)

        spec = SimpleNamespace(loader=SimpleNamespace(exec_module=load))
        with patch.object(replay, "_hook", None), \
                patch.object(replay.importlib.util, "spec_from_file_location", return_value=spec), \
                patch.object(replay.importlib.util, "module_from_spec", return_value=loaded):
            with ThreadPoolExecutor(1) as pool:
                future = pool.submit(replay.hook)
                try:
                    self.assertTrue(entered.wait(5))
                    self.assertIsNone(replay._hook)
                finally:
                    release.set()
                self.assertIs(future.result(5), loaded)
                self.assertTrue(getattr(loaded, "ready"))

    def test_config(self) -> None:
        cfg = hook.config_bytes()
        self.assertEqual(len(cfg), 0x3c)
        self.assertEqual(struct.unpack_from("<I", cfg, 0x14)[0], 0x80001)
        self.assertEqual(cfg[0x1f:0x21], bytes(2))  # no music or sound
        self.assertEqual(cfg[0x22:0x24], b"\x01\x00")  # windowed, no skip
        self.assertEqual(cfg[0x25], 0)
        # LoadConfig's bounds.
        self.assertTrue(cfg[0x1c] < 7 and cfg[0x1d] < 4 and cfg[0x24] < 3 and cfg[0x26] < 2)

    def test_setup_preserves_users_files(self) -> None:
        root = self.path.parent
        original = root / "build/th08_100d.exe"
        original.parent.mkdir()
        original.write_bytes(struct.pack("<3I", 1, 2, 3))
        source = root / "user"
        source.mkdir()
        (source / "harness.rpy").write_bytes(b"user replay")
        (source / "score.dat").write_bytes(b"user scores")
        (source / "th08.cfg").write_bytes(b"user config")
        game = root / "game"
        game.mkdir()
        (game / "replay").symlink_to(source, target_is_directory=True)
        for name in ("score.dat", "th08.cfg"):
            (game / name).symlink_to(source / name)
        with patch.object(hook, "ROOT", root):
            env = hook.setup(str(game), hook.Replay(self.path))
        self.assertEqual((source / "harness.rpy").read_bytes(), b"user replay")
        self.assertEqual((source / "th08.cfg").read_bytes(), b"user config")
        (game / "score.dat").write_bytes(b"run scores")
        self.assertEqual((source / "score.dat").read_bytes(), b"user scores")
        self.assertEqual((game / "replay/harness.rpy").read_bytes(), self.path.read_bytes())
        self.assertEqual(env["TRACE_EXE_CHECKSUM"], "3")
        self.assertEqual(env["TRACE_EXE_SIZE"], "12")
        self.assertEqual(env["TRACE_START"], "0,1,4,-1")

    def test_complete_run(self) -> None:
        with patch.object(replay, "hook", return_value=hook):
            result = replay.validate(hook.Replay(self.path), trace())
        self.assertTrue(result["passed"], result["failures"])

    def test_inputs_recorded_after_the_stage_clear_are_not_played(self) -> None:
        record = hook.Replay(self.path)
        for frames in (2, 4, 6):
            with self.subTest(frames=frames), patch.object(replay, "hook", return_value=hook):
                result = replay.validate(record, trace(frames=frames))
            self.assertFalse(result["passed"], "only the endpoint passes")

    def test_terminal_markers_are_checked_but_their_tail_is_not(self) -> None:
        for marker in (-4, -3):
            tail = FINAL_TAIL.copy()
            tail[marker] = 1
            self.path.write_bytes(encode(decoded(stages=[(0, 123, INPUTS + tail)])))
            with self.subTest(marker=marker), self.assertRaisesRegex(ValueError, "empty input"):
                hook.Replay(self.path)
        self.path.write_bytes(encode(decoded(stages=[(0, 123, [1] + INPUTS[1:] + FINAL_TAIL)])))
        with self.assertRaisesRegex(ValueError, "initial empty input"):
            hook.Replay(self.path)
        self.path.write_bytes(encode(decoded(stages=[(0, 123, INPUTS + [0, 0, 0x1234, 0x5678])])))
        self.assertEqual(hook.Replay(self.path).stages[0]["frame_counts"], [3])

    def test_incomplete_runs_fail(self) -> None:
        for bad in (trace(score=122), trace(frames=2), trace(end_entry=4), trace(title=False), trace(stage=1)):
            with patch.object(replay, "hook", return_value=hook):
                result = replay.validate(hook.Replay(self.path), bad)
            self.assertFalse(result["passed"], result)

    def test_original_and_standalone_symbols_resolve(self) -> None:
        original = replay.resolve(replay.symbols_csv())
        self.assertIn("PlayOnUpdate=0x452550", original)
        self.assertIn("SpellCard=0x4ea670", original)
        mapfile = ROOT / "build/th08_exe.map"
        if mapfile.exists():
            self.assertIn("PlayOnUpdate=", replay.resolve(replay.map_symbols(mapfile)))

    def test_binary_hook_sites(self) -> None:
        # Check the actual instructions each runtime patch expects, in both
        # executables. No assumptions about rebuilt addresses.
        import compare_exe

        for spec in ("original", "standalone"):
            _, path, symbols = replay.executable(spec)
            if not Path(path).exists():
                continue
            image = compare_exe.PE(path)
            functions = {name: symbols[symbol] for name, symbol in hook.FUNCTIONS.items()}

            def code(name: str) -> tuple[int, bytes]:
                address, size = functions[name]
                return address, image.read(address, size)

            def immediates(container: str, target: str) -> int:
                _, data = code(container)
                value = struct.pack("<I", functions[target][0])
                return sum(data[i:i + 4] == value for i in range(len(data) - 3))

            def calls(container: str, target: str) -> int:
                address, data = code(container)
                return sum(data[i] in (0xe8, 0xe9) and
                           (address + i + 5 + struct.unpack_from("<i", data, i + 1)[0]) & 0xffffffff
                           == functions[target][0] for i in range(len(data) - 4))

            with self.subTest(executable=spec):
                self.assertEqual(calls("RunFrame", "RunCalcChain"), 1)
                self.assertEqual(immediates("SetUpTitleScene", "MenuOnUpdate"), 1)
                for callback in ("PlayOnUpdate", "PlayOnUpdate2", "PlayAddedCallback"):
                    self.assertEqual(immediates("RegisterChain", callback), 1)
                self.assertEqual(calls("RegisterChain", "PlayAddedCallback"), 1)
                self.assertEqual(immediates("RegisterChain", "DeletedCallback"), 2)

    def test_compare_checks_every_record(self) -> None:
        self.assertTrue(replay.compare(trace(), trace())["identical"])
        self.assertFalse(replay.compare(trace(), trace(score=122))["identical"])
        self.assertFalse(replay.compare(trace(), trace(title=False))["identical"])


class LzssTest(unittest.TestCase):
    def test_literals(self) -> None:
        data = bytes(range(256))
        self.assertEqual(hook.decompress(literals(data), len(data)), data)

    def test_overlapping_copy(self) -> None:
        # literal A at ring position 1, followed by a 6-byte copy from 1.
        bits = "1" + f"{65:08b}" + "0" + f"{1:013b}" + f"{3:04b}" + "0" * 14
        bits += "0" * (-len(bits) % 8)
        packed = int(bits, 2).to_bytes(len(bits) // 8, "big")
        self.assertEqual(hook.decompress(packed, 7), b"AAAAAAA")

    def test_zero_tail_omitted_by_compressor(self) -> None:
        self.assertEqual(hook.decompress(literals(b"ABC")[:-1], 3), b"ABC")

    def test_bounds_and_truncation(self) -> None:
        for packed, size in [(b"", 1), (literals(b"ABC"), 2), (literals(b"ABC"), 4), (b"", -1)]:
            with self.subTest(size=size), self.assertRaises(ValueError):
                hook.decompress(packed, size)


class ArchiveTest(unittest.TestCase):
    def test_decrypt_interleaves_blocks_backwards(self) -> None:
        # One 4-byte block: the first half lands on bytes 3 and 1, the
        # second on 2 and 0; the key grows by the step. Then a tail of 1
        # (under a quarter block) is copied.
        data = bytes([0x10, 0x11, 0x12, 0x13, 0x99])
        key, step = 0x40, 3
        keys = [key + step * i for i in range(4)]
        expected = bytes([0x13 ^ keys[3], 0x11 ^ keys[1], 0x12 ^ keys[2], 0x10 ^ keys[0], 0x99])
        self.assertEqual(decrypt(data, key, step, 4, 100), expected)
        # Past the limit, the bytes are copied.
        self.assertEqual(decrypt(data[:4] * 2, key, step, 4, 4)[4:], data[:4])

    def test_bundled_replays(self) -> None:
        archive = ROOT / "orig/th08_100d/th08.dat"
        if not archive.exists():
            self.skipTest("requires the user's game archive")
        with tempfile.TemporaryDirectory() as directory:
            paths = extract(archive, Path(directory))
            self.assertEqual([path.name for path in paths], [f"demorpy{i}.rpy" for i in range(4)])
            records = [hook.Replay(path) for path in paths]
        self.assertEqual([list(r.stages) for r in records], [[5], [3], [2], [1]])
        self.assertEqual([r.label for r in records], ["Border Team Lunatic", "Magic Team Lunatic",
                                                      "Ghost Team Lunatic", "Scarlet Team Lunatic"])
        self.assertEqual([next(iter(r.stages.values()))["frames"] for r in records], [11077, 7734, 9266, 9257])
        self.assertEqual([r.score for r in records], [70652270, 56566480, 68585530, 74330640])


if __name__ == "__main__":
    unittest.main()
