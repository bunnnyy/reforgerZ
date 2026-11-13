#!/usr/bin/env python3
"""Ships hand-built sites (docs/SITES.md, "Hand-built sites").

The site builder saves each site the author builds in Game Master to
  $profile:ReforgerZ/hand_built/<world>/<site id>.txt
on the machine it ran on. This tool reads those saves, checks them, and writes them into the mod as
  scripts/Game/ReforgerZ/World/HandBuilt/RZ_HandBuiltPack.c
(generated script, never edited by hand), so they ship with it. Sites already in the pack stay unless
a save replaces them (same id) or a save marks them deleted.

  py tools/hand_built.py                     # read the usual profile folders (Windows), write the pack
  py tools/hand_built.py --from <folder>     # read saves from <folder>/hand_built (or <folder> itself)
  py tools/hand_built.py --check             # check the saves and the pack; write nothing

Exit status 1 if anything is wrong (a bad line, a prefab without its GUID, an object outside its site).
"""
import argparse
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PACK = os.path.join(REPO, "scripts", "Game", "ReforgerZ", "World", "HandBuilt", "RZ_HandBuiltPack.c")
KINDS = {"hold", "out", "safe", "post"}
PREFAB = re.compile(r"^\{[0-9A-F]{16}\}[^\s\"]+\.et$")
ID = re.compile(r"^(hold|out|safe|post)_-?\d+_-?\d+$")
SLACK = 1.05  # an object may stand this much past the radius (the builder measures on the ground plane)


def default_roots():
    """Where the saves usually are on the author's PC: Workbench's profile, then the game's."""
    docs = os.path.join(os.path.expanduser("~"), "Documents", "My Games")
    roots = []
    for name in ("ArmaReforgerWorkbench", "ArmaReforger"):
        roots.append(os.path.join(docs, name, "profile", "ReforgerZ", "hand_built"))
    return roots


def find_dir(path):
    """A folder given as the profile, its ReforgerZ folder or hand_built itself."""
    for cand in (os.path.join(path, "ReforgerZ", "hand_built"), os.path.join(path, "hand_built"), path):
        if not os.path.isdir(cand):
            continue
        for d in os.listdir(cand):
            sub = os.path.join(cand, d)
            if os.path.isdir(sub) and any(n.endswith(".txt") for n in os.listdir(sub)):
                return cand
    return None


class Site:
    def __init__(self, world):
        self.world = world
        self.id = ""
        self.title = ""
        self.kind = ""
        self.centre = (0.0, 0.0, 0.0)
        self.radius = 0.0
        self.use = True
        self.saved = ""
        self.deleted = False
        self.objs = []  # (prefab, x, y, z, yaw, pitch, roll, scale)
        self.source = ""


def read_save(path, world, errors):
    s = Site(world)
    s.source = path
    with open(path, encoding="utf-8") as f:
        for n, raw in enumerate(f, 1):
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            word, _, rest = line.partition(" ")
            rest = rest.strip()
            parts = rest.split()
            try:
                if word == "site":
                    s.id = rest
                elif word == "title":
                    s.title = rest
                elif word == "kind":
                    s.kind = rest
                elif word == "saved":
                    s.saved = rest
                elif word == "use":
                    s.use = rest != "0"
                elif word == "deleted":
                    s.deleted = rest == "1"
                elif word == "radius":
                    s.radius = float(rest)
                elif word == "centre":
                    s.centre = tuple(float(v) for v in parts[:3])
                elif word == "obj":
                    if len(parts) < 8:
                        errors.append(f"{path}:{n}: an obj line needs a prefab and 7 numbers")
                        continue
                    nums = [float(v) for v in parts[1:8]]
                    s.objs.append((parts[0], *nums))
                # any other kind of line is a later version's: kept out of the pack, not an error
            except ValueError:
                errors.append(f"{path}:{n}: not a number in '{line}'")
    return s


def check(s, errors, warnings):
    where = s.source or f"the pack's {s.id}"
    if not ID.match(s.id):
        errors.append(f"{where}: site id '{s.id}' isn't <kind>_<x>_<z>")
    if s.kind not in KINDS:
        errors.append(f"{where}: kind '{s.kind}' isn't one of {', '.join(sorted(KINDS))}")
    if s.deleted:
        return
    if s.radius <= 0:
        errors.append(f"{where}: no radius")
    cx, _, cz = s.centre
    for o in s.objs:
        prefab = o[0]
        if not PREFAB.match(prefab):
            errors.append(f"{where}: '{prefab}' isn't a prefab with its GUID")
        d = ((o[1] - cx) ** 2 + (o[3] - cz) ** 2) ** 0.5
        if s.radius > 0 and d > s.radius * SLACK:
            errors.append(f"{where}: {prefab.split('/')[-1]} stands {d:.0f} m from the centre, outside the site's {s.radius:.0f} m")
        if o[7] <= 0 or o[7] > 20:
            warnings.append(f"{where}: {prefab.split('/')[-1]} has scale {o[7]}")
    if not s.objs:
        warnings.append(f"{where}: {s.title or s.id} has no objects (an empty site: camp life lays nothing there)")


def read_pack(errors):
    """The sites already shipped, read back from the generated script."""
    sites = []
    if not os.path.exists(PACK):
        return sites
    text = open(PACK, encoding="utf-8").read()
    cur = None
    for m in re.finditer(r'hb\.(Site|Obj)\((.*?)\);', text):
        args = split_args(m.group(2))
        if m.group(1) == "Site":
            cur = Site(args[0])
            cur.id, cur.title, cur.kind = args[1], args[2], args[3]
            cur.centre = (float(args[4]), float(args[5]), float(args[6]))
            cur.radius = float(args[7])
            cur.use = args[8] == "true"
            cur.saved = args[9]
            sites.append(cur)
        elif cur is not None:
            cur.objs.append((args[0], *[float(v) for v in args[1:8]]))
    return sites


def split_args(s):
    out, cur, q, esc = [], "", False, False
    for ch in s:
        if esc:
            cur += ch
            esc = False
        elif ch == "\\" and q:
            esc = True
        elif ch == '"':
            q = not q
        elif ch == "," and not q:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    out.append(cur.strip())
    return out


def lit(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def num(v):
    t = f"{v:.3f}".rstrip("0").rstrip(".")
    return t if t not in ("", "-0") else "0"


def write_pack(sites):
    lines = [
        "// GENERATED by tools/hand_built.py from the site builder's saves. Do not edit: build the site in",
        "// Game Master, save it with the site builder, then run the tool (docs/SITES.md, \"Hand-built sites\").",
        "class RZ_HandBuiltPack",
        "{",
        "\tstatic void Add(RZ_HandBuilt hb)",
        "\t{",
    ]
    for i, _ in enumerate(sites):
        lines.append(f"\t\tSite{i}(hb);")
    lines.append("\t}")
    for i, s in enumerate(sites):
        use = "true" if s.use else "false"
        lines += ["", f"\t// {s.world}: {s.title} ({len(s.objs)} objects, saved {s.saved})",
                  f"\tprotected static void Site{i}(RZ_HandBuilt hb)", "\t{",
                  f"\t\thb.Site({lit(s.world)}, {lit(s.id)}, {lit(s.title)}, {lit(s.kind)}, {num(s.centre[0])}, {num(s.centre[1])}, {num(s.centre[2])}, {num(s.radius)}, {use}, {lit(s.saved)});"]
        for o in s.objs:
            lines.append(f"\t\thb.Obj({lit(o[0])}, " + ", ".join(num(v) for v in o[1:8]) + ");")
        lines.append("\t}")
    lines.append("}")
    with open(PACK, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--from", dest="src", action="append", help="a profile folder, its ReforgerZ folder or hand_built (repeatable)")
    ap.add_argument("--check", action="store_true", help="check the saves and the pack, write nothing")
    a = ap.parse_args()

    errors, warnings = [], []
    roots = []
    for cand in (a.src or default_roots()):
        d = find_dir(cand)
        if d:
            roots.append(d)
        elif a.src:
            errors.append(f"no hand_built saves under {cand}")

    saves = {}
    for root in roots:
        for world in sorted(os.listdir(root)):
            wdir = os.path.join(root, world)
            if not os.path.isdir(wdir):
                continue
            for name in sorted(os.listdir(wdir)):
                if not name.endswith(".txt") or ".bak" in name:
                    continue
                s = read_save(os.path.join(wdir, name), world, errors)
                if not s.id:
                    errors.append(f"{os.path.join(wdir, name)}: no 'site' line")
                    continue
                key = (world, s.id)
                if key in saves:
                    warnings.append(f"{s.id} on {world} is saved in two profiles: {saves[key].source} wins over {s.source}")
                    continue
                saves[key] = s

    shipped = {(s.world, s.id): s for s in read_pack(errors)}
    merged = dict(shipped)
    replaced = added = dropped = 0
    for key, s in saves.items():
        if s.deleted:
            if key in merged:
                del merged[key]
                dropped += 1
            continue
        if key in merged:
            replaced += 1
        else:
            added += 1
        merged[key] = s

    sites = [merged[k] for k in sorted(merged)]
    for s in sites:
        check(s, errors, warnings)
    for s in saves.values():
        if s.deleted:
            check(s, errors, warnings)

    for w in warnings:
        print("warning:", w)
    for e in errors:
        print("error:", e)

    worlds = sorted({s.world for s in sites})
    objs = sum(len(s.objs) for s in sites)
    in_use = sum(1 for s in sites if s.use)
    print(f"hand_built: {len(saves)} saves read from {len(roots)} folder(s); the pack: {len(sites)} sites ({in_use} in use), "
          f"{objs} objects on {', '.join(worlds) or 'no map'}; {added} new, {replaced} replaced, {dropped} deleted.")
    if errors:
        print(f"hand_built: {len(errors)} errors; nothing written.")
        return 1
    if a.check:
        print("hand_built: OK (checked, nothing written)")
        return 0
    write_pack(sites)
    print(f"hand_built: OK, wrote {os.path.relpath(PACK, REPO)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
