"""Lists or extracts files from Arma Reforger's packed data (.pak), for reading vanilla scripts, configs
and layouts as text (docs/PC_SETUP.md).

    py tools/pak_extract.py <pak folder> list <path substring>
    py tools/pak_extract.py <pak folder> extract <path prefix> <output folder>

<pak folder> is the game's addons/data folder. Paths in a pak start with "/" (/scripts/Game/...): in Git
Bash set MSYS_NO_PATHCONV=1 so the shell doesn't turn them into Windows paths. A pak is a FORM/PAC1 file:
a HEAD chunk, a DATA chunk with the files, and a FILE chunk with the tree (a type byte, 0 folder / 1 file,
a name with a length byte; a folder's child count, a file's offset, packed size, size and three more
fields, all little-endian u32). A file whose packed size differs from its size is zlib.
"""
import glob, os, struct, sys, zlib


def read_tree(path):
    f = open(path, "rb")
    f.seek(12)
    data = None
    while True:
        head = f.read(8)
        if len(head) < 8:
            break
        size = struct.unpack(">I", head[4:])[0]
        if head[:4] == b"FILE":
            data = f.read(size)
            break
        f.seek(size, 1)
    entries = []
    pos = 0

    def node(prefix):
        nonlocal pos
        kind = data[pos]
        n = data[pos + 1]
        name = data[pos + 2:pos + 2 + n].decode("utf-8", "replace")
        pos += 2 + n
        if kind == 0:
            count = struct.unpack("<I", data[pos:pos + 4])[0]
            pos += 4
            for _ in range(count):
                node(prefix + name + "/")
        else:
            off, packed, size = struct.unpack("<III", data[pos:pos + 12])
            pos += 24
            entries.append((prefix + name, off, packed, size))

    if data:
        node("")
    return f, entries


def main():
    folder, mode, pattern = sys.argv[1], sys.argv[2], sys.argv[3]
    done = 0
    for pak in sorted(glob.glob(os.path.join(folder, "*.pak"))):
        f, entries = read_tree(pak)
        for name, off, packed, size in entries:
            if mode == "list" and pattern.lower() in name.lower():
                print(os.path.basename(pak), name, size)
            elif mode == "extract" and name.startswith(pattern):
                f.seek(off)
                blob = f.read(packed)
                if packed != size:
                    blob = zlib.decompress(blob)
                dst = os.path.join(sys.argv[4], name.lstrip("/"))
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                open(dst, "wb").write(blob)
                done += 1
    if mode == "extract":
        print("extracted", done)


if __name__ == "__main__":
    main()
