"""Builds the camp-life pieces, their kits and the Traces compositions from pieces.py
(docs/WORKBENCH_SETUP.md, "Camp life" and "Traces").

    py tools/camp_kit/build.py [--index <resource index>]

Writes Prefabs/ReforgerZ/Dressing/<Faction>/RZ_Dress_<Name>.et and Prefabs/ReforgerZ/Traces/<Name>.et
(keeping an existing .meta, so the GUID stays; a piece no longer in pieces.py has its files deleted),
and on RZ_GameMode.et the camp-life kits (each plan entry's piece, ring, use, seats, its place and turn
in the plan, the piece's footprint, its tier, zone and rules from pieces.RULES, whether it goes indoors
and, from pieces.AT, where it stands by a building; each kit's doctrine and its layers' pieces from
pieces.DOCTRINE: docs/SITES.md, "Building in layers"), the
smart-action waypoint and the composition slots of World > Traces > Kit. Each piece's footprint
(m_sFoot, and the box round it) is its props' real boxes from footprints.json (written by the
Workbench plugin "ReforgerZ: Measure prop footprints", docs/SITES.md), up to 8, and a person round
each seat; a prop with no measured box falls back to pieces.FP and is listed. Each composition is an
empty root with an RplComponent and a Hierarchy, its props as children, and its seats as direct
children with an SCR_AISmartActionSentinelComponent. A piece with no rules, an unknown zone, a doctrine
naming a piece that isn't there, and a plan whose pieces overlap are refused.

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
from pieces import AT, C, DENSE, DOCTRINE, FP, KIT_TIER, KITS, OPENINGS, RULES, TRACES  # noqa: E402
import footprints  # noqa: E402
import pieces  # noqa: E402

PROPS = os.path.join(HERE, "props.txt")
GM = os.path.join(REPO, "Prefabs/ReforgerZ/Core/RZ_GameMode.et")
META_TEMPLATE = os.path.join(REPO, "Prefabs/ReforgerZ/Puzzles/RZ_PuzzleObject.et.meta")
WAYPOINT = "Prefabs/ReforgerZ/AI/RZ_WP_SmartAction.et"
ANIM = {"sit": "SIT", "lean_l": "LEAN_LEFT", "lean_r": "LEAN_RIGHT", "smoke": "SMOKING", "loiter": "LOITERING"}
USE = {"Sit": "SIT", "Work": "WORK", "Watch": "WATCH", "Gather": "GATHER", "Lean": "LEAN", "Sleep": "SLEEP", "Cook": "COOK", "Wash": "WASH", "Chop": "CHOP", "None": "NONE"}
ROLE = {"c": "CENTRE", "i": "INNER", "a": "APPROACH", "e": "EDGE"}
ZONES = ["yard", "wire", "gate", "kill", "court", "stores", "work", "kitchen", "sleep", "sick", "dump", "dead"]
CLOSE = {"none": "NONE", "road": "ROAD", "two": "TWO", "all": "ALL"}
HARDEN = {"none": "NONE", "doors": "DOORS", "threat": "THREAT", "all": "ALL"}
WHEN = {"supplied": "SUPPLIED", "thriving": "THRIVING", "hit": "HIT"}
# A doctrine's layer pieces: (its key, RZ_ESiteLayer)
LAYERS = (("wire", "WIRE"), ("block", "BLOCK"), ("window", "WINDOW"), ("door", "DOOR"), ("lookout", "LOOKOUT"), ("light", "LIGHT"), ("stores", "STORES"))
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
        # Always written: some prefabs (the mods' walls and barriers) carry a turn of their own, which a
        # prop placed with none would keep, standing side-on to its run
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
    """The box round the piece's footprint in its own space (min x, min z, max x, max z): its props'
    real boxes (footprints.piece_rects), and a person round each seat."""
    xs, zs = [], []
    for r in footprints.piece_rects(parts, FP):
        for x, z in footprints.corners(r):
            xs.append(x)
            zs.append(z)
    for p in parts:
        if p[0] == "s":
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


def check_rules():
    """Every piece has its rules (tier, zone, rules), every zone is known, every doctrine's pieces exist."""
    bad = ["%s: no rules in pieces.RULES" % n for n in C if n not in RULES]
    bad += ["%s: unknown zone %s" % (n, r[1]) for n, r in RULES.items() if r[1] not in ZONES]
    bad += ["%s: rules for a piece that isn't in pieces.C" % n for n in RULES if n not in C]
    for key, d in DOCTRINE.items():
        for k, _ in LAYERS:
            names = d.get(k, [])
            if isinstance(names, str):
                names = [names]
            bad += ["%s %s: no piece named %s" % (key[0], key[1], n) for n in names if n not in C]
        bad += ["%s %s: indoors %s isn't a piece" % (key[0], key[1], n) for n in d.get("indoors", []) if n not in C]
    if bad:
        sys.exit("Rules refused:" + LF + LF.join(bad))


# Props other props may stand in or under: tents, nets, a mast's guy wires, a stall, a shower, a tower
HOLLOW = ("CamoNet", "Tent", "Antenna", "MarketStand", "ShowerBeach", "GuardTower", "CanvasCover", "LifeGuardTower")


def _quad(prop, x, z, yaw):
    b = footprints.MEASURED.get(prop)
    if not b:
        return None
    a = math.radians(yaw)
    cs, sn = math.cos(a), math.sin(a)
    cx = x + b["ox"] * cs + b["oz"] * sn
    cz = z - b["ox"] * sn + b["oz"] * cs
    hw, hd = b["w"] / 2, b["d"] / 2
    return [(cx + lx * cs + lz * sn, cz - lx * sn + lz * cs) for lx, lz in ((-hw, -hd), (hw, -hd), (hw, hd), (-hw, hd))]


def _area(poly):
    return abs(sum(poly[i][0] * poly[(i + 1) % len(poly)][1] - poly[(i + 1) % len(poly)][0] * poly[i][1] for i in range(len(poly)))) / 2


def _clip(subject, clipper):
    n = len(clipper)
    if sum(clipper[i][0] * clipper[(i + 1) % n][1] - clipper[(i + 1) % n][0] * clipper[i][1] for i in range(n)) < 0:
        clipper = clipper[::-1]
    out = subject
    for i in range(n):
        a, b = clipper[i], clipper[(i + 1) % n]
        inp, out = out, []
        if not inp:
            break

        def inside(q):
            return (b[0] - a[0]) * (q[1] - a[1]) - (b[1] - a[1]) * (q[0] - a[0]) >= 0

        def cross(q, r):
            d = (a[0] - b[0]) * (q[1] - r[1]) - (a[1] - b[1]) * (q[0] - r[0])
            if abs(d) < 1e-9:
                return r
            t = ((a[0] - q[0]) * (q[1] - r[1]) - (a[1] - q[1]) * (q[0] - r[0])) / d
            return (a[0] + t * (b[0] - a[0]), a[1] + t * (b[1] - a[1]))

        prev = inp[-1]
        for cur in inp:
            if inside(cur):
                if not inside(prev):
                    out.append(cross(prev, cur))
                out.append(cur)
            elif inside(prev):
                out.append(cross(prev, cur))
            prev = cur
    return out


def check_pieces():
    """No two props of one piece stand in each other (their measured boxes, both on the ground, both a
    knee high or more): the garden's two beds once overlapped by a metre, a chopping block stood inside
    the woodpile. A prop others stand in or under (HOLLOW) isn't asked."""
    bad = []
    for name, (folder, use, role, intent, parts) in C.items():
        ground = []
        for part in parts:
            if part[0] != "p":
                continue
            _, prop, x, z, yaw, y, pitch, roll = part
            b = footprints.MEASURED.get(prop)
            if not b or y >= 0.3 or abs(pitch) > 20 or abs(roll) > 20 or b["h"] < 0.4 or any(w in prop for w in HOLLOW):
                continue
            ground.append((prop, _quad(prop, x, z, yaw)))
        for i in range(len(ground)):
            for j in range(i + 1, len(ground)):
                both = _clip(ground[i][1], ground[j][1])
                if len(both) < 3:
                    continue
                share = _area(both) / min(_area(ground[i][1]), _area(ground[j][1]))
                if share > 0.3:
                    bad.append("%s: %s stands in %s (%d%% of the smaller)" % (name, ground[i][0], ground[j][0], round(share * 100)))
    if bad:
        sys.exit("Pieces refused (props collide):" + LF + LF.join(bad))


# What people sit on, and what they sit at (docs/SITES.md, "Spacing")
SEAT_PROPS = ("ChairOld_01.et", "ChairOld_02.et", "Chair_01_white.et", "Stool_01.et", "Armchair_01.et", "BenchWooden_01.et", "DeckChair_01_ochre.et")
TABLE_PROPS = ("TableOld_01.et", "TableOld_01_white.et", "TableLong_01.et", "TableMilitary_US_01.et")


def _seg_dist(p, a, b):
    ax, az = b[0] - a[0], b[1] - a[1]
    t = max(0.0, min(1.0, ((p[0] - a[0]) * ax + (p[1] - a[1]) * az) / (ax * ax + az * az or 1)))
    return math.hypot(p[0] - a[0] - t * ax, p[1] - a[1] - t * az)


def _inside(p, quad):
    sign = 0
    for i in range(4):
        a, b = quad[i], quad[(i + 1) % 4]
        c = (b[0] - a[0]) * (p[1] - a[1]) - (b[1] - a[1]) * (p[0] - a[0])
        if c != 0:
            if sign and (c > 0) != (sign > 0):
                return False
            sign = c
    return True


def _quad_gap(qa, qb):
    """The gap between two measured boxes on the ground; 0 if they overlap."""
    if any(_inside(p, qb) for p in qa) or any(_inside(p, qa) for p in qb) or len(_clip(qa, qb)) >= 3:
        return 0.0
    return min(min(_seg_dist(p, qb[i], qb[(i + 1) % 4]) for p in qa for i in range(4)),
               min(_seg_dist(p, qa[i], qa[(i + 1) % 4]) for p in qb for i in range(4)))


def check_spacing():
    """A seat by a table is pulled up to it: its measured front within pieces.SEAT_LEFT (plus a hand's width) of the
    table's edge, not in it, and facing it. A chair a metre off its table, or turned to the wall, is refused."""
    bad = []
    for name, (folder, use, role, intent, parts) in list(C.items()) + [(k, (None, None, None, None, v[1])) for k, v in list(TRACES.items()) + list(OPENINGS.items())]:
        props = [q for q in parts if q[0] == "p" and footprints.MEASURED.get(q[1])]
        tables = [q for q in props if q[1] in TABLE_PROPS and q[5] < 0.3]
        for q in props:
            if q[1] not in SEAT_PROPS or q[5] >= 0.3 or not tables:
                continue
            sq = _quad(q[1], q[2], q[3], q[4])
            near = min(tables, key=lambda t: _quad_gap(sq, _quad(t[1], t[2], t[3], t[4])))
            gap = _quad_gap(sq, _quad(near[1], near[2], near[3], near[4]))
            if gap > 1.0:
                continue   # not at that table
            if gap <= 0:
                bad.append("%s: %s stands in %s" % (name, q[1], near[1]))
                continue
            (fx, fz), fd, across = pieces.FRONT(q[1])
            nx, nz = pieces._rot(q[4], fx, fz)
            b = footprints.MEASURED[q[1]]
            cx, cz = pieces._rot(q[4], b["ox"], b["oz"])
            tb = footprints.MEASURED[near[1]]
            tx, tz = pieces._rot(near[4], tb["ox"], tb["oz"])
            to_x, to_z = near[2] + tx - (q[2] + cx), near[3] + tz - (q[3] + cz)
            ln = math.hypot(to_x, to_z) or 1
            facing = (nx * to_x + nz * to_z) / ln
            seat_back = b["h"] > 0.7
            if gap > pieces.SEAT_LEFT + 0.1:
                bad.append("%s: %s stands %.2f m off %s (a seat in use is %.2f, one pushed back %.2f)" % (name, q[1], gap, near[1], pieces.SEAT_GAP, pieces.SEAT_LEFT))
            elif seat_back and facing < 0.6:
                bad.append("%s: %s by %s faces away from it" % (name, q[1], near[1]))
    if bad:
        sys.exit("Pieces refused (seats not at their table):" + LF + LF.join(bad))


def check_fit():
    """Every prop of every piece is placed by its measured faces (docs/SITES.md, "Spacing"; pieces.piece settles
    them): two props on the ground either touch (faces within pieces.TOUCH) or stand pieces.APART or more apart,
    never a sliver between; a seat pulled up to a table keeps its stated gap. A prop stood on another stands over
    it (no more than a quarter hanging off); one hung on a wall or a post isn't asked."""
    bad = []
    for name, (folder, use, role, intent, parts) in C.items():
        ground = [q for q in parts if pieces._ground(q)]
        for i in range(len(ground)):
            qi = pieces.QUAD(ground[i][1], ground[i][2], ground[i][3], ground[i][4])
            for j in range(i + 1, len(ground)):
                qj = pieces.QUAD(ground[j][1], ground[j][2], ground[j][3], ground[j][4])
                gap = pieces.GAP(qi, qj)
                if pieces.TOUCH < gap < pieces.APART and not pieces._seated(ground[i], ground[j], gap):
                    bad.append("%s: %s and %s %.2f m apart (touch them, or set them %.2f m or more apart)" % (name, ground[i][1], ground[j][1], gap, pieces.APART))
        for part in parts:
            if part[0] != "p" or part[5] < 0.3 or abs(part[6]) > 20 or abs(part[7]) > 20 or not footprints.MEASURED.get(part[1]):
                continue
            _, prop, x, z, yaw, y, pitch, roll = part
            under = [g for g in parts if g[0] == "p" and g is not part and g[5] < y and footprints.MEASURED.get(g[1]) and pieces._holds(g[1], g[2], g[3], g[4], x, z)]
            if not under:
                continue
            g = max(under, key=lambda q: q[5])
            gb = footprints.MEASURED[g[1]]
            if y < g[5] + pieces.TOP(g[1]) - 0.02 or (gb["w"] <= 0.2 and gb["d"] <= 0.2):
                continue   # hung on it, or set on a post's top
            qa = pieces.QUAD(prop, x, z, yaw)
            both = _clip(qa, pieces.QUAD(g[1], g[2], g[3], g[4]))
            if (len(both) < 3 or _area(both) < 0.75 * _area(qa)) and _area(qa) < _area(pieces.QUAD(g[1], g[2], g[3], g[4])):
                bad.append("%s: %s hangs off the %s it stands on" % (name, prop, g[1]))
    if bad:
        sys.exit("Pieces refused (not placed by their measured faces):" + LF + LF.join(bad))


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


def piece_lines(name, fac, place, rel, layer=None, x=0.0, z=0.0, yaw=0.0, indoors=False, plan=True, extra=False, when=""):
    """One kit entry: the piece, its ring, use, seats, where it stands by a building, its plan spot,
    footprint, tier, zone, rules, layer and whether it goes indoors."""
    folder, use, role, intent, parts = C[name]
    seats = max(1, sum(1 for p in parts if p[0] == "s"))
    x0, z0, x1, z1 = footprint(parts)
    tier, zone, rules = RULES[name]
    tier = KIT_TIER.get((fac, name), tier)
    L = ['       RZ_DressingPiece "{%s}" {' % gid(), '        m_sPrefab "%s%s"' % (guid_of(rel[name]), rel[name]), "        m_eRole %s" % ROLE[role[0].lower()],
         "        m_eUse %s" % USE[use], "        m_iSeats %d" % seats]
    if plan and name in AT and place != "ROAD_POST":
        L.append("        m_eAt %s" % AT[name].upper())
    if extra:
        L.append("        m_bExtra 1")
        if when:
            L.append("        m_eWhen %s" % WHEN[when])
    elif plan:
        L.append("        m_vPlan %s 0 %s" % (num(x), num(z)))
        if yaw:
            L.append("        m_fPlanYaw %s" % num(yaw))
    L += ["        m_vFootMin %s 0 %s" % (num(x0), num(z0)), "        m_vFootMax %s 0 %s" % (num(x1), num(z1)),
          '        m_sFoot "%s"' % footprints.foot_text(parts, FP), "        m_iTier %d" % tier]
    if zone != "yard":
        L.append("        m_eZone %s" % zone.upper())
    if rules:
        L.append('        m_sRules "%s"' % rules)
    if layer:
        L.append("        m_eLayer %s" % layer)
    if indoors:
        L.append("        m_bIndoors 1")
    L.append("       }")
    return L


def write_kits():
    seed("kits")
    s, nl = read_gm()
    rel = {name: "Prefabs/ReforgerZ/Dressing/%s/RZ_Dress_%s.et" % (C[name][0], name) for name in C}
    L = ["    m_aKits {"]
    for fac, place, _, plan in KITS:
        d = DOCTRINE.get((fac, place), {})
        indoors = set(d.get("indoors", []))
        L += ['     RZ_DressingKit "{%s}" {' % gid(), "      m_eFaction %s" % fac, "      m_ePlace %s" % place, "      m_aPieces {"]
        for name, x, z, yaw in plan:
            L += piece_lines(name, fac, place, rel, None, x, z, yaw, name in indoors)
        # A dense kit's extras: after the plan's own pieces, with no spot in the plan
        for entry in DENSE.get((fac, place), []):
            name, count, when = entry[0], entry[1], (entry[3] if len(entry) > 3 else "")
            if name not in C or (when and when not in WHEN):
                sys.exit("%s %s: no piece named %s, or no such state as '%s' (pieces.DENSE)" % (fac, place, name, when))
            for _ in range(count):
                L += piece_lines(name, fac, place, rel, None, extra=True, when=when)
        L.append("      }")
        L += ["      m_eClose %s" % CLOSE[d.get("close", "none")], "      m_eHarden %s" % HARDEN[d.get("harden", "none")]]
        if "ring" in d:
            L.append("      m_fRing %s" % num(d["ring"]))
        if "yard" in d:
            L.append("      m_fYard %s" % num(d["yard"]))
        layers = []
        for key, layer in LAYERS:
            names = d.get(key, [])
            if isinstance(names, str):
                names = [names]
            layers += [(n, layer) for n in names]
        if layers:
            L.append("      m_aLayers {")
            for n, layer in layers:
                L += piece_lines(n, fac, place, rel, layer, plan=False)
            L.append("      }")
        L.append("     }")
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
    check_rules()
    check_pieces()
    check_spacing()
    check_fit()
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
    every = [c[4] for c in C.values()] + [parts for _, parts in TRACES.values()] + [parts for _, parts in OPENINGS.values()]
    missing = footprints.unmeasured(every, FP)
    if missing:
        print("%d props have no measured footprint (pieces.FP used; run the Workbench plugin ReforgerZ: Measure prop footprints): %s" % (len(missing), ", ".join(missing)))


if __name__ == "__main__":
    main()
