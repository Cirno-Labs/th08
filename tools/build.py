"""Compiles src/ with the original compiler and links the hook DLL.

    build.py        the hook DLL, build/th08.dll
    build.py exe    the standalone game, build/th08.exe

Every function in config/symbols.csv that src/ defines is hooked: the DLL
overwrites the original function with a jump to the new one. Every other
symbol becomes an absolute symbol at its original address, so new code
calls and reads the original game.

The standalone exe links src/ without src/dll/ but with src/exe/ (the
game's data and global objects, which the DLL takes from the original)
against the real libraries, plus the original's resources.
"""
import csv, glob, os, struct, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

import coff

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OBJ = "build/obj"
SJIS = "build/src"  # src/ converted to Shift-JIS
DLL = "build/th08.dll"
EXE = "build/th08.exe"
ORIGINAL = "build/th08_100d.exe"
CFLAGS = ["/Od", "/Gr", "/GX", "/Oi"]
GLUE_PREFIX = "dll_"  # objects from src/dll/, the DLL's own code
DLL_ONLY, EXE_ONLY = "dll", "exe"  # subdirectories of src/ only one target builds
TIMEOUT = 60  # cl spins forever after a fatal error
# Compiles run in parallel on half the cores by default, leaving the rest
# free; JOBS overrides it.
JOBS = int(os.environ.get("JOBS") or max(1, (os.cpu_count() or 2) // 2))

ENV = dict(
    os.environ,
    WINEPREFIX=os.environ.get("WINEPREFIX", os.path.join(ROOT, ".wine")),
    WINEDEBUG=os.environ.get("WINEDEBUG", "-all"),
    INCLUDE=r"C:\dx8\include;C:\msvc7\include;C:\psdk\Include",
    LIB=r"C:\dx8\lib;C:\msvc7\lib;C:\psdk\Lib",
)


def win(path):
    return path.replace("/", "\\")


def run_tool(name, *args):
    """Runs an MSVC tool; returns its exit code and its output, without the
    lines that only name the file."""
    cmd = ["wine", rf"C:\msvc7\bin\{name}.exe", "/nologo", *args]
    try:
        r = subprocess.run(cmd, env=ENV, timeout=TIMEOUT, capture_output=True, text=True, errors="replace")
    except subprocess.TimeoutExpired:
        return 1, f"{name} timed out (fatal error?): {' '.join(args)}"
    out = "\n".join(l for l in r.stdout.splitlines() if l.strip() and not l.endswith(".cpp"))
    return r.returncode, out


def tool(name, *args):
    code, out = run_tool(name, *args)
    if out:
        print(out)
    if code:
        sys.exit(code)


def read_symbols():
    with open("config/symbols.csv") as f:
        return {s: int(r["address"], 16) for r in csv.DictReader(f) for s in r["symbol"].split()}


def to_sjis(text):
    """Encodes `text` as Shift-JIS for cl.

    cl reads source bytes as single-byte characters, so a double-byte
    character whose second byte is 0x5c ("能", "表", "ソ") would end in an
    escape inside a string literal: that byte is doubled.
    """
    out = bytearray()
    for ch in text:
        b = ch.encode("cp932")
        out += b
        if len(b) == 2 and b[1] == 0x5C:
            out += b"\\"
    return bytes(out)


def compile_all(target=DLL):
    """Compiles src/ from a Shift-JIS copy in build/src/.

    Sources are UTF-8, but the game's strings are Shift-JIS and MSVC 7
    copies string bytes as they are, so each file is converted first.
    """
    os.makedirs(OBJ, exist_ok=True)
    for src in glob.glob("src/**/*.*", recursive=True):
        copy = os.path.join(SJIS, os.path.relpath(src, "src"))
        if not os.path.exists(copy) or os.path.getmtime(copy) < os.path.getmtime(src):
            os.makedirs(os.path.dirname(copy), exist_ok=True)
            with open(src, encoding="utf-8") as f:
                text = f.read()
            with open(copy, "wb") as f:
                f.write(to_sjis(text))
    headers = max((os.path.getmtime(h) for h in glob.glob("src/**/*.h", recursive=True)), default=0)
    objs, stale = [], []
    for src, obj in sources(target):
        objs.append(obj)
        if not (os.path.exists(obj) and os.path.getmtime(obj) > max(os.path.getmtime(src), headers)):
            stale.append((src, obj))

    def compile_one(job):
        src, obj = job
        return run_tool("cl", "/c", *file_flags(src), "/I" + win(SJIS), "/Fo" + win(obj),
                        win(os.path.join(SJIS, os.path.relpath(src, "src"))))

    failed = 0
    with ThreadPoolExecutor(JOBS) as pool:
        # Results come back in source order, so the output reads as before.
        for (src, obj), (code, out) in zip(stale, pool.map(compile_one, stale)):
            print("cl", src)
            if out:
                print(out)
            failed = failed or code
    if failed:
        sys.exit(failed)
    return objs


def sources(target=DLL):
    """Yields (source, object) for each .cpp in src/ that `target` builds."""
    skip = EXE_ONLY if target == DLL else DLL_ONLY
    for src in sorted(glob.glob(os.path.join(ROOT, "src/**/*.cpp"), recursive=True)):
        rel = os.path.relpath(src, os.path.join(ROOT, "src"))
        if rel.split(os.sep)[0] == skip:
            continue
        yield os.path.relpath(src, ROOT), os.path.join(OBJ, rel.replace("/", "_")[:-4] + ".obj")


def file_flags(src):
    """The flags for one file: CFLAGS, changed by a `// cflags: ...` first
    line. `/X` adds a flag, `-/X` removes a default one."""
    with open(src) as f:
        first = f.readline()
    changes = first.split(":", 1)[1].split() if first.startswith("// cflags:") else []
    flags = [f for f in CFLAGS if "-" + f not in changes]
    return flags + [c for c in changes if not c.startswith("-")]


def write_hooks(objs, symbols):
    """Writes the hook table and the symbols that point into the original.

    The linker rejects calls to absolute symbols, so each original function
    that new code calls gets a thunk instead: push <address>; ret. It
    leaves every register intact, whatever the calling convention.
    """
    defined, code, undefined, called, glue = set(), set(), set(), set(), set()
    for path in objs:
        obj = coff.Object(path)
        for s in obj.externals():
            if s.section <= 0:
                undefined.add(s.name)
                if os.path.basename(path).startswith(GLUE_PREFIX):
                    glue.add(s.name)
                continue
            defined.add(s.name)
            if obj.sections[s.section - 1].flags & coff.CODE:
                code.add(s.name)
        for sec in obj.sections:
            called.update(obj.symbols[i].name for _, i, t in sec.relocs if t == coff.REL32)
    # Data that src/ defines, such as vtables, is never hooked. Functions
    # the linker folded into one share an address; one hook covers them.
    hooks = sorted({symbols[n]: n for n in sorted(code & symbols.keys())}.items())
    # Imports the hook installer itself uses stay real imports: it runs
    # from DllMain and should not rely on the game's import table.
    external = {n: a for n, a in symbols.items()
                if n in undefined - defined and not (n.startswith("__imp_") and n in glue)}

    table, table_relocs = bytearray(), []
    for addr, name in hooks:
        table_relocs.append((len(table) + 4, name, coff.DIR32))
        table += struct.pack("<II", addr, 0)
    table += bytes(8)
    thunks, entry = bytearray(), {"_g_Hooks": (1, 0)}
    for name, addr in external.items():
        if name in called:
            entry[name] = (2, len(thunks))
            thunks += struct.pack("<BIB", 0x68, addr, 0xC3).ljust(8, b"\xcc")
    absolute = {n: a for n, a in external.items() if n not in entry}
    # Functions with exception handling address the handler chain as
    # fs:[__except_list]; the CRT defines it as the absolute value 0.
    if "__except_list" in undefined:
        absolute["__except_list"] = 0

    path = os.path.join(OBJ, "hooks.obj")
    coff.write_object(path, [(".rdata", coff.RDATA, bytes(table), table_relocs),
                             (".text", coff.TEXT, bytes(thunks), [])],
                      entry, [n for _, n in hooks], absolute)
    print(f"{len(hooks)} hooked, {len(external)} used from the original")
    return path


def link_exe(objs):
    """Links the standalone game with the libraries the original was built
    with: the static multithreaded CRT, D3DX8 and the import libraries."""
    import rsrc
    res = os.path.join(OBJ, "th08.res")
    rsrc.write_res(res, rsrc.resources(ORIGINAL))
    tool("link", "/SUBSYSTEM:WINDOWS", "/FIXED", "/NODEFAULTLIB:libc.lib",
         "/MAP:" + win(EXE[:-4] + ".map"), "/OUT:" + win(EXE), *map(win, objs), win(res),
         "libcmt.lib", "d3dx8.lib", "d3d8.lib", "dsound.lib", "dinput8.lib", "dxguid.lib",
         "winmm.lib", "kernel32.lib", "user32.lib", "gdi32.lib")
    copy_header_fields(ORIGINAL, EXE)
    print("linked", EXE)


def copy_header_fields(src, dst):
    """Gives dst the original's link timestamp, the one header field the
    linker has no option for."""
    with open(src, "rb") as f:
        orig = f.read(0x400)
    pe = struct.unpack_from("<I", orig, 0x3C)[0]
    with open(dst, "r+b") as f:
        f.seek(0x3C)
        dpe = struct.unpack("<I", f.read(4))[0]
        f.seek(dpe + 8)
        f.write(orig[pe + 8:pe + 12])


def main():
    os.chdir(ROOT)
    if sys.argv[1:] == ["exe"]:
        link_exe(compile_all(EXE))
        return
    objs = compile_all()
    hooks = write_hooks(objs, read_symbols())
    tool("link", "/DLL", "/NODEFAULTLIB", "/ENTRY:DllMain", "/BASE:0x30000000",
         "/MAP:" + win(DLL[:-4] + ".map"), "/OUT:" + win(DLL),
         *map(win, objs), win(hooks), "kernel32.lib", "user32.lib", "winmm.lib")
    print("linked", DLL)


if __name__ == "__main__":
    main()
