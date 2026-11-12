"""Builds the camp-life pieces, their kits and the Traces compositions from pieces.py
(docs/WORKBENCH_SETUP.md, "Camp life" and "Traces").

    py tools/camp_kit/build.py [--index <resource index>]

Writes Prefabs/ReforgerZ/Dressing/<Faction>/RZ_Dress_<Name>.et and Prefabs/ReforgerZ/Traces/<Name>.et
(keeping an existing .meta, so the GUID stays; a piece no longer in pieces.py has its files deleted),
and on RZ_GameMode.et the camp-life kits (each plan entry's piece, ring, use, seats, its place and turn
in the plan, the piece's footprint and, from pieces.AT, where it stands by a building), the
smart-action waypoint and the composition slots of World > Traces > Kit. Each composition is an
empty root with an RplComponent and a Hierarchy, its props as children, and its seats as direct
children with an SCR_AISmartActionSentinelComponent. A plan whose pieces overlap is refused.

props.txt holds each prop's entity class, its ResourceName and "hier" when the prefab already has a
Hierarchy component ("Class {GUID}path [hier]"). A prop that isn't in it is looked up in the resource
index (the Workbench plugin "ReforgerZ: Write resource index" writes $profile:RZ_ResourceIndex.txt)
and added as GenericEntity: check it with "ReforgerZ: Dump prefabs and classes" and correct
props.txt (docs/PC_SETUP.md).
"""
import glob, hashlib, math, os, re, sys, random

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
from pieces import AT, C, FP, KITS, OPENINGS, TRACES  # noqa: E402

PROPS = os.path.join(HERE, "props.txt")
GM = os.path.join(REPO, "Prefabs/ReforgerZ/Core/RZ_GameMode.et")
META_TEMPLATE = os.path.join(REPO, "Prefabs/ReforgerZ/Puzzles/RZ_PuzzleObject.et.meta")
WAYPOINT = "Prefabs/ReforgerZ/AI/RZ_WP_SmartAction.et"
ANIM = {"sit": "SIT", "lean_l": "LEAN_LEFT", "lean_r": "LEAN_RIGHT", "smoke": "SMOKING", "loiter": "LOITERING"}
USE = {"Sit": "SIT", "Work": "WORK", "Watch": "WATCH", "Gather": "GATHER", "Lean": "LEAN", "Sleep": "SLEEP", "Cook": "COOK", "Wash": "WASH", "Chop": "CHOP", "None": "NONE"}
ROLE = {"c": "CENTRE", "i": "INNER", "a": "APPROACH", "e": "EDGE"}
CR, LF = chr(13), chr(10)
SEAT_PAD = 0.45      # a seated or standing person round a seat's spot, inside the footprint (a lean's 0.6)
PLAN_GAP = 0.3       # pieces in a plan stand at least this far apart


RNG = random.Random()


def gid():
    """IDs come from RNG, seeded per file (seed()), so a rebuild of an unchanged piece is unchanged."""
    return "%016X" % RNG.getrandbits(64)


def seed(key):
    RNG.seed(int(hashlib.sha1(key.encode("utf-8")).hexdigest()[:16], 16))


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
    seed("meta " + rel)
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


def num(v):
    v = round(v, 2)
    if v == int(v):
        return str(int(v))
    return ("%.2f" % v).rstrip("0")


def rot(x, z, yaw):
    """Piece space to its parent's, turned by yaw (degrees, clockwise from +Z, as the game's yaw)."""
    a = math.radians(yaw)
    return x * math.cos(a) + z * math.sin(a), -x * math.sin(a) + z * math.cos(a)


def footprint(parts):
    """The piece's footprint in its own space (min x, min z, max x, max z): every prop's box turned
    by its yaw, and a person round each seat."""
    xs, zs = [], []
    for p in parts:
        if p[0] == "p":
            w, d = FP.get(p[1], (0.5, 0.5))[:2]
            for cx, cz in ((-w / 2, -d / 2), (w / 2, -d / 2), (w / 2, d / 2), (-w / 2, d / 2)):
                rx, rz = rot(cx, cz, p[4])
                xs.append(p[2] + rx)
                zs.append(p[3] + rz)
        else:
            pad = 0.6 if p[4].startswith("lean") else SEAT_PAD
            xs += [p[2] - pad, p[2] + pad]
            zs += [p[3] - pad, p[3] + pad]
    return min(xs), min(zs), max(xs), max(zs)


def plan_quad(name, x, z, yaw, grow=0.0):
    """A plan entry's footprint corners in the plan."""
    x0, z0, x1, z1 = footprint(C[name][4])
    q = []
    for cx, cz in ((x0 - grow, z0 - grow), (x1 + grow, z0 - grow), (x1 + grow, z1 + grow), (x0 - grow, z1 + grow)):
        rx, rz = rot(cx, cz, yaw)
        q.append((x + rx, z + rz))
    return q


def separated(a, b):
    """One of rectangle a's sides separates it from rectangle b."""
    for i in range(2):
        (ax, az), (bx, bz) = a[i], a[i + 1]
        nx, nz = az - bz, bx - ax
        pa = [p[0] * nx + p[1] * nz for p in a]
        pb = [p[0] * nx + p[1] * nz for p in b]
        if max(pa) < min(pb) or max(pb) < min(pa):
            return True
    return False


def check_plans():
    """Every kit's plan: known pieces, and no two footprints closer than PLAN_GAP."""
    bad = []
    for fac, place, _, plan in KITS:
        quads = []
        for name, x, z, yaw in plan:
            if name not in C:
                bad.append("%s %s: no piece named %s" % (fac, place, name))
                continue
            q = plan_quad(name, x, z, yaw, PLAN_GAP / 2)
            for label, oq in quads:
                if not separated(q, oq) and not separated(oq, q):
                    bad.append("%s %s: %s at (%s, %s) overlaps %s" % (fac, place, name, num(x), num(z), label))
            quads.append(("%s at (%s, %s)" % (name, num(x), num(z)), q))
    if bad:
        sys.exit("Plans refused:" + LF + LF.join(bad))


def remove_stale():
    """Pieces no longer in pieces.py: their prefab and .meta go (only the kits pointed at them)."""
    keep = {os.path.normpath(os.path.join(REPO, "Prefabs/ReforgerZ/Dressing/%s/RZ_Dress_%s.et" % (C[n][0], n))) for n in C}
    for path in glob.glob(os.path.join(REPO, "Prefabs/ReforgerZ/Dressing/*/RZ_Dress_*.et")):
        if os.path.normpath(path) not in keep:
            os.remove(path)
            if os.path.exists(path + ".meta"):
                os.remove(path + ".meta")
            print("removed", os.path.relpath(path, REPO))


def read_gm():
    s = open(GM, encoding="utf-8", newline="").read()
    return s, (CR + LF if CR + LF in s else LF)


def write_gm(s):
    open(GM, "w", encoding="utf-8", newline="").write(s)


def write_kits():
    seed("kits")
    s, nl = read_gm()
    rel = {name: "Prefabs/ReforgerZ/Dressing/%s/RZ_Dress_%s.et" % (C[name][0], name) for name in C}
    L = ["    m_aKits {"]
    for fac, place, _, plan in KITS:
        L += ['     RZ_DressingKit "{%s}" {' % gid(), "      m_eFaction %s" % fac, "      m_ePlace %s" % place, "      m_aPieces {"]
        for name, x, z, yaw in plan:
            folder, use, role, intent, parts = C[name]
            seats = max(1, sum(1 for p in parts if p[0] == "s"))
            x0, z0, x1, z1 = footprint(parts)
            L += ['       RZ_DressingPiece "{%s}" {' % gid(), '        m_sPrefab "%s%s"' % (guid_of(rel[name]), rel[name]), "        m_eRole %s" % ROLE[role[0].lower()],
                  "        m_eUse %s" % USE[use], "        m_iSeats %d" % seats]
            if name in AT and place != "ROAD_POST":
                L.append("        m_eAt %s" % AT[name].upper())
            L.append("        m_vPlan %s 0 %s" % (num(x), num(z)))
            if yaw:
                L.append("        m_fPlanYaw %s" % num(yaw))
            L += ["        m_vFootMin %s 0 %s" % (num(x0), num(z0)), "        m_vFootMax %s 0 %s" % (num(x1), num(z1)), "       }"]
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


def write_opening_scenes(slots):
    """Guidance > Opening scenes (m_aOpeningScenes on m_GuidanceSettings): one slot per key, in pieces.OPENINGS' order."""
    seed("opening_scenes")
    s, nl = read_gm()
    L = ["    m_aOpeningScenes {"]
    for key, res in slots:
        L += ['     RZ_OpeningSceneSlot "{%s}" {' % gid(), '      m_sKey "%s"' % key, '      m_sPrefab "%s"' % res, "     }"]
    L.append("    }")
    block = nl.join(L)
    head = s.index("   m_GuidanceSettings RZ_GuidanceSettings ")
    close = s.index(nl + "   }", head)
    if "    m_aOpeningScenes {" in s[head:close]:
        start = s.index("    m_aOpeningScenes {", head)
        end = s.index(nl + "    }", start) + len(nl + "    }")
        s = s[:start] + block + s[end:]
    else:
        s = s[:close] + nl + block + s[close:]
    write_gm(s)


def main():
    check_plans()
    index = []
    if "--index" in sys.argv:
        path = sys.argv[sys.argv.index("--index") + 1]
        index = [l.strip() for l in open(path, encoding="utf-8") if l.strip().endswith(".et")]
    props = load_props()
    for name, (folder, use, role, intent, parts) in C.items():
        rel = "Prefabs/ReforgerZ/Dressing/%s/RZ_Dress_%s.et" % (folder, name)
        seed(rel)
        write_text(rel, piece_text(parts, props, index))
        write_meta(rel)
    remove_stale()
    write_kits()
    slots = {}
    for slot, (name, parts) in TRACES.items():
        rel = "Prefabs/ReforgerZ/Traces/%s.et" % name
        seed(rel)
        write_text(rel, piece_text(parts, props, index))
        write_meta(rel)
        slots[slot] = guid_of(rel) + rel
    write_trace_kit(slots)
    scenes = []
    for key, (name, parts) in OPENINGS.items():
        rel = "Prefabs/ReforgerZ/Guidance/Openings/%s.et" % name
        seed(rel)
        write_text(rel, piece_text(parts, props, index))
        write_meta(rel)
        scenes.append((key, guid_of(rel) + rel))
    write_opening_scenes(scenes)
    lines =["# Entity class, ResourceName and 'hier' when the prefab already has a Hierarchy component (build.py adds new props)."]
    lines += ["%s %s%s" % (v[0], v[1], " hier" if v[2] else "") for _, v in sorted(props.items())]
    write_text(os.path.relpath(PROPS, REPO), LF.join(lines) + LF)
    print("built %d pieces, %d kits, %d trace compositions and %d opening scenes" % (len(C), len(KITS), len(TRACES), len(OPENINGS)))


if __name__ == "__main__":
    main()
