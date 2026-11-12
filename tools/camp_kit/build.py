"""Builds the camp-life pieces, their kits and the Traces compositions from pieces.py
(docs/WORKBENCH_SETUP.md, "Camp life" and "Traces").

    py tools/camp_kit/build.py [--index <resource index>]

Writes Prefabs/ReforgerZ/Dressing/<Faction>/RZ_Dress_<Name>.et and Prefabs/ReforgerZ/Traces/<Name>.et
(keeping an existing .meta, so the GUID stays), and on RZ_GameMode.et the camp-life kits (each piece's
ring, use, count, seats and, from pieces.AT, where it stands by a building), the
smart-action waypoint and the composition slots of World > Traces > Kit. Each composition is an
empty root with an RplComponent and a Hierarchy, its props as children, and its seats as direct
children with an SCR_AISmartActionSentinelComponent.

props.txt holds each prop's entity class, its ResourceName and "hier" when the prefab already has a
Hierarchy component ("Class {GUID}path [hier]"). A prop that isn't in it is looked up in the resource
index (the Workbench plugin "ReforgerZ: Write resource index" writes $profile:RZ_ResourceIndex.txt)
and added as GenericEntity: check it with "ReforgerZ: Dump prefabs and classes" and correct
props.txt (docs/PC_SETUP.md).
"""
import os, re, sys, random

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
from pieces import AT, C, KITS, TRACES  # noqa: E402

PROPS = os.path.join(HERE, "props.txt")
GM = os.path.join(REPO, "Prefabs/ReforgerZ/Core/RZ_GameMode.et")
META_TEMPLATE = os.path.join(REPO, "Prefabs/ReforgerZ/Puzzles/RZ_PuzzleObject.et.meta")
WAYPOINT = "Prefabs/ReforgerZ/AI/RZ_WP_SmartAction.et"
ANIM = {"sit": "SIT", "lean_l": "LEAN_LEFT", "lean_r": "LEAN_RIGHT", "smoke": "SMOKING", "loiter": "LOITERING"}
USE = {"Sit": "SIT", "Work": "WORK", "Watch": "WATCH", "Gather": "GATHER", "Lean": "LEAN", "Sleep": "SLEEP", "Cook": "COOK", "Wash": "WASH", "Chop": "CHOP", "None": "NONE"}
ROLE = {"c": "CENTRE", "i": "INNER", "a": "APPROACH", "e": "EDGE"}
CR, LF = chr(13), chr(10)


def gid():
    return "%016X" % random.getrandbits(64)


def load_props():
    props = {}
    if os.path.exists(PROPS):
        for line in open(PROPS, encoding="utf-8"):
            line = line.strip()
            if line and not line.startswith("#"):
                bits = line.split(" ")
                name = bits[1].split("}", 1)[1].rsplit("/", 1)[-1]
                props[name] = (bits[0], bits[1], len(bits) > 2 and bits[2] == "hier")
    return props


def resolve(name, props, index):
    if name in props:
        return props[name]
    hits = [r for r in index if r.endswith("/" + name)]
    hits.sort(key=lambda r: (not r.split("}", 1)[1].startswith("Prefabs/"), len(r)))
    if not hits:
        sys.exit("No prefab named %s: check the name, or pass --index with a fresh resource index" % name)
    props[name] = ("GenericEntity", hits[0], False)
    print("added to props.txt as GenericEntity (check its class and Hierarchy):", hits[0])
    return props[name]


def guid_of(rel):
    t = open(os.path.join(REPO, rel + ".meta"), encoding="utf-8").read()
    return re.search(r'Name "(\{[0-9A-F]{16}\})', t).group(1)


def write_text(rel, text):
    path = os.path.join(REPO, rel)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, "w", encoding="utf-8", newline=LF).write(text)


def write_meta(rel):
    if os.path.exists(os.path.join(REPO, rel + ".meta")):
        return
    t = open(META_TEMPLATE, encoding="utf-8").read()
    old = t.split('Name "')[1].split('"')[0]
    write_text(rel + ".meta", t.replace(old, "{%s}%s" % (gid(), rel)))


def piece_text(parts, props, index):
    L = ["GenericEntity {", ' ID "%s"' % gid(), " components {", '  RplComponent "{%s}" {' % gid(), "  }", '  Hierarchy "{%s}" {' % gid(), "  }", " }", " coords 0 0 0", " {"]
    for p in parts:
        if p[0] != "p":
            continue
        _, name, x, z, yaw, y, pitch, roll = p
        cls, rn, hier = resolve(name, props, index)
        L += ['  %s : "%s" {' % (cls, rn), '   ID "%s"' % gid()]
        if not hier:  # a prefab with its own Hierarchy can't take a second one
            L += ["   components {", '    Hierarchy "{%s}" {' % gid(), "    }", "   }"]
        L.append("   coords %s %s %s" % (x, y, z))
        if yaw or pitch or roll:
            L.append("   angles %s %s %s" % (pitch, yaw, roll))
        L.append("  }")
    for s in parts:
        if s[0] != "s":
            continue
        _, tag, x, z, anim, lx, lz = s
        if lx == 0 and lz == 0:
            lx, lz = -x, -z
        L += ["  GenericEntity {", '   ID "%s"' % gid(), "   components {", '    SCR_AISmartActionSentinelComponent "{%s}" {' % gid(), "     Tags {", '      "%s"' % tag, "     }",
              "     m_vLookPosition %s 1.6 %s" % (round(lx, 2), round(lz, 2))]
        if tag == "RZ_Watch":
            L.append("     m_fLookDirectionRange 120")
        L += ["     m_eLoiterAnimation %s" % ANIM[anim], "    }", '    Hierarchy "{%s}" {' % gid(), "    }", "   }", "   coords %s 0 %s" % (x, z), "  }"]
    L += [" }", "}"]
    return LF.join(L) + LF


def read_gm():
    s = open(GM, encoding="utf-8", newline="").read()
    return s, (CR + LF if CR + LF in s else LF)


def write_gm(s):
    open(GM, "w", encoding="utf-8", newline="").write(s)


def write_kits():
    s, nl = read_gm()
    rel = {name: "Prefabs/ReforgerZ/Dressing/%s/RZ_Dress_%s.et" % (C[name][0], name) for name in C}
    L = ["    m_aKits {"]
    for fac, place, pieces in KITS:
        L += ['     RZ_DressingKit "{%s}" {' % gid(), "      m_eFaction %s" % fac, "      m_ePlace %s" % place, "      m_aPieces {"]
        for entry in pieces.split(", "):
            name, count, ring = entry.split()
            seats = max(1, sum(1 for p in C[name][4] if p[0] == "s"))
            L += ['       RZ_DressingPiece "{%s}" {' % gid(), '        m_sPrefab "%s%s"' % (guid_of(rel[name]), rel[name]), "        m_eRole %s" % ROLE[ring],
                  "        m_eUse %s" % USE[C[name][1]], "        m_iCount %s" % count, "        m_iSeats %d" % seats]
            if name in AT and place != "ROAD_POST":
                L.append("        m_eAt %s" % AT[name].upper())
            L.append("       }")
        L += ["      }", "     }"]
    L.append("    }")
    block = nl.join(L)
    if "    m_aKits {" in s:
        start = s.index("    m_aKits {")
        end = s.index(nl + "    }", start) + len(nl + "    }")
        s = s[:start] + block + s[end:]
    else:
        anchor = '    m_sGroupPrefab "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et"'
        s = s.replace(anchor, anchor + nl + block)
    if "m_sSmartActionWaypoint" not in s:
        s = s.replace("    m_aKits {", '    m_sSmartActionWaypoint "%s%s"' % (guid_of(WAYPOINT), WAYPOINT) + nl + "    m_aKits {")
    write_gm(s)


def write_trace_kit(slots):
    """The composition slots of World > Traces > Kit; its other fields (sounds, decals, boards) are kept."""
    s, nl = read_gm()
    if "   m_TraceSettings RZ_TraceSettings" not in s:
        block = ['   m_TraceSettings RZ_TraceSettings "{%s}" {' % gid(), '    m_Kit RZ_TraceKit "{%s}" {' % gid(), "    }", "   }"]
        i = s.index("   m_UISettings RZ_UISettings ")
        s = s[:i] + nl.join(block) + nl + s[i:]
    start = s.index("    m_Kit RZ_TraceKit ")
    end = s.index(nl + "    }", start)
    lines = s[start:end].split(nl)
    rest = [l for l in lines[1:] if l.strip().split(" ")[0] not in slots]
    rest += ['     %s "%s"' % (k, v) for k, v in slots.items()]
    s = s[:start] + nl.join([lines[0]] + rest) + s[end:]
    write_gm(s)


def main():
    index = []
    if "--index" in sys.argv:
        path = sys.argv[sys.argv.index("--index") + 1]
        index = [l.strip() for l in open(path, encoding="utf-8") if l.strip().endswith(".et")]
    props = load_props()
    for name, (folder, use, role, intent, parts) in C.items():
        rel = "Prefabs/ReforgerZ/Dressing/%s/RZ_Dress_%s.et" % (folder, name)
        write_text(rel, piece_text(parts, props, index))
        write_meta(rel)
    write_kits()
    slots = {}
    for slot, (name, parts) in TRACES.items():
        rel = "Prefabs/ReforgerZ/Traces/%s.et" % name
        write_text(rel, piece_text(parts, props, index))
        write_meta(rel)
        slots[slot] = guid_of(rel) + rel
    write_trace_kit(slots)
    lines = ["# Entity class, ResourceName and 'hier' when the prefab already has a Hierarchy component (build.py adds new props)."]
    lines += ["%s %s%s" % (v[0], v[1], " hier" if v[2] else "") for _, v in sorted(props.items())]
    write_text(os.path.relpath(PROPS, REPO), LF.join(lines) + LF)
    print("built %d pieces, %d kits and %d trace compositions" % (len(C), len(KITS), len(TRACES)))


if __name__ == "__main__":
    main()
