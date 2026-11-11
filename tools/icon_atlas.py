#!/usr/bin/env python3
"""Packs the mono icons into one texture atlas and writes the RZ_Icons imageset for it.

    py tools/icon_atlas.py              # 1. writes UI/Textures/ReforgerZ/RZ_Icons.png
    (import that PNG in Workbench's Resource Browser: it becomes RZ_Icons.edds)
    py tools/icon_atlas.py imageset     # 2. writes UI/Imagesets/ReforgerZ/RZ_Icons.imageset (+ .meta)

Every icon in UI/Icons/png/128/ becomes an image named like its file (faction_haven,
status_fever...), which is what the scripts ask the imageset for. Needs Pillow.
"""
import os
import secrets
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "UI", "Icons", "png", "128")
CELL = 128
COLUMNS = 8
ATLAS = "UI/Textures/ReforgerZ/RZ_Icons.png"
IMAGESET = "UI/Imagesets/ReforgerZ/RZ_Icons.imageset"
PLATFORMS = ["XBOX_ONE", "XBOX_SERIES", "PS4", "PS5", "HEADLESS"]


def icons():
    return sorted(f[:-4] for f in os.listdir(SRC) if f.endswith(".png"))


def size(count):
    rows = (count + COLUMNS - 1) // COLUMNS
    return COLUMNS * CELL, rows * CELL


def write_atlas():
    from PIL import Image
    names = icons()
    atlas = Image.new("RGBA", size(len(names)), (0, 0, 0, 0))
    for i, name in enumerate(names):
        icon = Image.open(os.path.join(SRC, name + ".png")).convert("RGBA")
        if icon.size != (CELL, CELL):
            icon = icon.resize((CELL, CELL), Image.LANCZOS)
        atlas.paste(icon, ((i % COLUMNS) * CELL, (i // COLUMNS) * CELL))
    path = os.path.join(ROOT, ATLAS)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    atlas.save(path)
    print("%d icons -> %s (%dx%d). Import it in Workbench, then run: py tools/icon_atlas.py imageset"
          % (len(names), ATLAS, atlas.size[0], atlas.size[1]))


def texture_name():
    """The imported texture's ResourceName, from its .meta."""
    meta = os.path.join(ROOT, ATLAS[:-4] + ".edds.meta")
    if not os.path.exists(meta):
        sys.exit("No %s: import %s in Workbench first." % (meta, ATLAS))
    for line in open(meta, encoding="utf-8"):
        line = line.strip()
        if line.startswith("Name "):
            return line.split('"')[1]
    sys.exit("No Name line in " + meta)


def write_imageset():
    names = icons()
    width, height = size(len(names))
    lines = ["ImageSetClass {", " RefSize %d %d" % (width, height), " Textures {",
             "  ImageSetTextureClass {", "   mpix 1", '   path "%s"' % texture_name(), "  }", " }",
             " Images {"]
    for i, name in enumerate(names):
        lines += ["  ImageSetDefClass %s {" % name, '   Name "%s"' % name,
                  "   Pos %d %d" % ((i % COLUMNS) * CELL, (i // COLUMNS) * CELL),
                  "   Size %d %d" % (CELL, CELL), "   Flags 0", "  }"]
    lines += [" }", "}"]
    path = os.path.join(ROOT, IMAGESET)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, "w", encoding="utf-8", newline="\n").write("\n".join(lines) + "\n")
    meta = path + ".meta"
    if not os.path.exists(meta):  # keep the GUID once Workbench knows it
        guid = secrets.token_hex(8).upper()
        out = ["MetaFileClass {", ' Name "{%s}%s"' % (guid, IMAGESET), " Configurations {",
               "  ImageSetResourceClass PC {", "  }"]
        out += ["  ImageSetResourceClass %s : PC {\n  }" % p for p in PLATFORMS]
        out += [" }", "}"]
        open(meta, "w", encoding="utf-8", newline="\n").write("\n".join(out))
    print("%d images -> %s" % (len(names), IMAGESET))


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "imageset":
        write_imageset()
    else:
        write_atlas()
