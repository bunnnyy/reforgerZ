"""Real prop footprints (docs/SITES.md, "Real footprints"), shared by build.py and render.py.

footprints.json is written by the Workbench plugin "ReforgerZ: Measure prop footprints" (or by the
runtime test's site audit, tools/runtime_test/run.sh sites): for each prop file name, its measured
box: width (x), depth (z), height, the box's middle offset from the prop's origin (ox, oz) and
whether its origin is at its feet. A prop not in it falls back to the hand-typed pieces.FP, and
build.py says which.
"""
import json, math, os

HERE = os.path.dirname(os.path.abspath(__file__))
JSON = os.path.join(HERE, "footprints.json")
MAX_RECTS = 8        # a piece's footprint: at most this many prop boxes (the smallest are merged)
ON_TOP = 0.3         # a prop this high up stands on another (a lamp on a table): the one below covers it
SEAT = 0.6           # a seat's person: a 0.6 m square
OVERHEAD = ("CamoNet",)  # props people stand under (a net on poles): not part of the ground footprint


def load():
    if not os.path.exists(JSON):
        return {}
    with open(JSON, encoding="utf-8") as f:
        data = json.load(f)
    return data.get("props", data)


MEASURED = load()


try:
    from pieces import FOOT_FORCED as FORCED
except ImportError:
    FORCED = {}


def rot(x, z, yaw):
    a = math.radians(yaw)
    return x * math.cos(a) + z * math.sin(a), -x * math.sin(a) + z * math.cos(a)


def prop_box(name, fp):
    """(w, d, ox, oz, measured) of a prop: its measured box, else the hand-typed FP (0.5 m if neither)."""
    if name in FORCED:  # what stands on the ground, where the measured box says more (pieces.FOOT_FORCED)
        return FORCED[name][0], FORCED[name][1], 0.0, 0.0, True
    m = MEASURED.get(name)
    if m and m.get("w", 0) > 0.01 and m.get("d", 0) > 0.01:
        return m["w"], m["d"], m.get("ox", 0.0), m.get("oz", 0.0), True
    w, d = fp.get(name, (0.5, 0.5))[:2]
    return w, d, 0.0, 0.0, False


def unmeasured(parts_lists, fp):
    """Prop names in the given parts lists that have no measured footprint."""
    out = set()
    for parts in parts_lists:
        for p in parts:
            if p[0] == "p" and not prop_box(p[1], fp)[4]:
                out.add(p[1])
    return sorted(out)


def corners(r):
    x, z, w, d, yaw = r
    out = []
    for cx, cz in ((-w / 2, -d / 2), (w / 2, -d / 2), (w / 2, d / 2), (-w / 2, d / 2)):
        rx, rz = rot(cx, cz, yaw)
        out.append((x + rx, z + rz))
    return out


def aabb(rects):
    xs, zs = [], []
    for r in rects:
        for x, z in corners(r):
            xs.append(x)
            zs.append(z)
    return min(xs), min(zs), max(xs), max(zs)


def merged(a, b):
    """The upright box round two rectangles."""
    x0, z0, x1, z1 = aabb([a, b])
    return ((x0 + x1) / 2, (z0 + z1) / 2, x1 - x0, z1 - z0, 0.0)


def piece_rects(parts, fp):
    """A piece's props' boxes in its own space, (x, z, w, d, yaw) each, at most MAX_RECTS: props
    standing on others are left out, then the pair whose merged box adds least is merged."""
    rects = []
    for p in parts:
        if p[0] != "p" or p[5] >= ON_TOP or p[1].startswith(OVERHEAD):
            continue
        w, d, ox, oz, _ = prop_box(p[1], fp)
        dx, dz = rot(ox, oz, p[4])
        rects.append((p[2] + dx, p[3] + dz, w, d, float(p[4])))
    if not rects:
        for p in parts:
            if p[0] == "p":
                w, d, ox, oz, _ = prop_box(p[1], fp)
                dx, dz = rot(ox, oz, p[4])
                rects.append((p[2] + dx, p[3] + dz, w, d, float(p[4])))
    while len(rects) > MAX_RECTS:
        best = None
        for i in range(len(rects)):
            for j in range(i + 1, len(rects)):
                m = merged(rects[i], rects[j])
                cost = m[2] * m[3] - rects[i][2] * rects[i][3] - rects[j][2] * rects[j][3]
                if best is None or cost < best[0]:
                    best = (cost, i, j, m)
        _, i, j, m = best
        rects = [r for k, r in enumerate(rects) if k not in (i, j)] + [m]
    return rects


def seats(parts):
    return [(p[2], p[3]) for p in parts if p[0] == "s"]


def foot_text(parts, fp):
    """RZ_DressingPiece.m_sFoot: "x z w d yaw; ...; s x z"."""
    def n(v):
        v = round(v, 2)
        return str(int(v)) if v == int(v) else ("%.2f" % v).rstrip("0")
    items = ["%s %s %s %s %s" % (n(x), n(z), n(w), n(d), n(yaw)) for x, z, w, d, yaw in piece_rects(parts, fp)]
    items += ["s %s %s" % (n(x), n(z)) for x, z in seats(parts)]
    return "; ".join(items)
