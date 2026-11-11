"""Prints the zombie lab's results as tables (tools/zombie_lab/run.sh calls it).

    python3 tools/zombie_lab/report.py <console.log> [<console.log of the restart boot>]

Rows are the lab's "RZL R|table|key=value|..." lines. Exit code 1 if a log has script exceptions.
With SVG_DIR set, the behaviour run's B20 tracks (a fan and a pincer, top-down) are drawn there as SVG.
"""
import os
import re
import sys
from collections import OrderedDict

TITLES = OrderedDict([
    ("world", "AI world"),
    ("factions", "Factions (Bacon's faction against each)"),
    ("site", "Sites found"),
    ("clock", "7. Game clock"),
    ("groups", "1. Runtime groups"),
    ("groupsAdd", "1. AddAIEntityToGroup, per character (prefab 0-1 civilian, 2 military, 3 heavy)"),
    ("groupsKill", "1. Killing the members one by one"),
    ("leak", "2. The waypoint leak (RZ_Util)"),
    ("waypoint", "2. One group on waypoints for 30 minutes"),
    ("held", "2. The held waypoint (one entity, moved and re-added)"),
    ("light", "3. Light blocks"),
    ("perc", "3. Perception trials (dist in m; times in s after the target appeared; -1 = never)"),
    ("share", "3. Target sharing (listeners behind a crest; cameAfter in s after the seer saw the target, -1 = never)"),
    ("memory", "3. Memory after losing sight (s and m after the target vanished)"),
    ("noise", "3. Gunfire (blind listeners; cameAfter = s until 30 m closer, -1 = never)"),
    ("speeds", "4. Speeds (m/s)"),
    ("perfInfo", "5. AI limit"),
    ("perf", "5. Server frame time"),
    ("health", "6. Health"),
    ("healthRegen", "6. Health 15 s after setting it to half"),
    ("kill", "6. Removing an infected"),
    ("walls", "B. Wall prefabs for the courses (height above the pivot, m)"),
    ("water", "B. Water sites"),
    ("house", "B. Houses"),
    ("b1", "B1. Blind spawn: steered from the spawn frame, target already in reach (times in s after spawn, -1 = never)"),
    ("b6", "B6. Release: factor back, with and without our injected share event (s after the release)"),
    ("b4", "B4. A new waypoint 150 m away during a fight, then the kill"),
    ("b5", "B5. Take-back: what keeps a chase alive (s after the target vanished)"),
    ("b5t", "B5. Every 2 s (ages in s since each hook last fired)"),
    ("b3", "B3. Per-member slots (L2): a group of 4, two anchors 40 m apart, 50 s each"),
    ("b20", "B20. A fan and a pincer (tracks: SVG_DIR)"),
    ("b7cap", "B7. Speed caps (L4) and SPRINT wanted (L5), m/s"),
    ("b7walk", "B7. Walk-stop (Roam without a waypoint) and dynamic stance (L6)"),
    ("b8", "B8. The climb gate (L11)"),
    ("b17", "B17. The heavy's contact"),
    ("b9", "B9. Stuck and the unstick ladder"),
    ("b9snap", "B9. Navmesh snap cost (L18)"),
    ("b16", "B16. Water"),
    ("b19", "B19. Seeing through walls (Bacon's own sight, factor 1)"),
    ("b10", "B10. Shut, unlocked doors"),
    ("b11", "B11. Stairs"),
    ("b13", "B13. Sight-check cost"),
    ("b14", "B14. Attack pieces"),
    ("b18", "B18. Hearing with the intake gate (L8)"),
    ("b15", "B15. Server frame time with the behaviour layer"),
    ("b12", "B12. LOD, the think-rate hook and sleep"),
    ("b2", "B2. Hook calls (cumulative, logged each minute)"),
])

SKIP_TABLE = {"b20t"}


def b1_summary(rows):
    """B1 per cell: trials, and how many noticed the target while blind."""
    cells = OrderedDict()
    for r in rows:
        if r.get("note"):
            continue
        c = cells.setdefault(r.get("cell"), [0, 0, 0, 0, []])
        c[0] += 1
        if num(r.get("enemyAfter"), -1) >= 0 or num(r.get("baconTargetAfter"), -1) >= 0:
            c[1] += 1
        if num(r.get("chargedAfter"), -1) >= 0:
            c[2] += 1
        if num(r.get("seenAfter"), -1) >= 0:
            c[3] += 1
        if num(r.get("releasedEngagedAfter"), -1) >= 0:
            c[4].append(num(r.get("releasedEngagedAfter")))
    out = []
    for cell, (n, eng, ch, seen, rel) in cells.items():
        out.append(OrderedDict([("cell", cell), ("trials", str(n)), ("seen while blind", str(seen)),
                                ("engaged while blind", str(eng)), ("charged while blind", str(ch)),
                                ("engaged after the release (s)", " ".join("%.1f" % x for x in rel) or "-")]))
    return out


def svg_tracks(rows, outdir):
    """One SVG per B20 case: each body's path (target at the centre), slots as faint dots."""
    os.makedirs(outdir, exist_ok=True)
    cases = OrderedDict()
    for r in rows:
        cases.setdefault(r.get("case"), []).append(r)
    colours = ["#d33", "#36c", "#393", "#c80", "#93c", "#555"]
    written = []
    for case, rs in cases.items():
        tgt = [r for r in rs if r.get("i") == "target"]
        flank = [r for r in rs if r.get("i") == "flank"]
        body = [r for r in rs if r.get("i") not in ("target", "flank")]
        if not tgt or not body:
            continue
        tx, tz = num(tgt[0]["x"]), num(tgt[0]["z"])
        pts = [(num(r["x"]) - tx, num(r["z"]) - tz) for r in body]
        span = max(max(abs(x) for x, z in pts), max(abs(z) for x, z in pts), 50) * 1.1
        size = 600
        sc = size / (2 * span)

        def xy(x, z):
            return size / 2 + x * sc, size / 2 - z * sc
        parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d">' % (size, size + 40, size, size + 40),
                 '<rect width="100%" height="100%" fill="#fff"/>']
        for ring in (35, 50, 100, 150, 200):
            if ring < span:
                cx, cy = xy(0, 0)
                parts.append('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="none" stroke="#ddd"/>' % (cx, cy, ring * sc))
                parts.append('<text x="%.1f" y="%.1f" font-size="10" fill="#aaa">%d m</text>' % (cx + 2, cy - ring * sc - 2, ring))
        for fr in flank:
            for kx, kz in (("x", "z"), ("slotX", "slotZ")):
                fx, fy = xy(num(fr[kx]) - tx, num(fr[kz]) - tz)
                parts.append('<rect x="%.1f" y="%.1f" width="8" height="8" fill="none" stroke="#000"/>' % (fx - 4, fy - 4))
        ids = sorted({r["i"] for r in body}, key=lambda v: int(v))
        for k, i in enumerate(ids):
            mine = [r for r in body if r["i"] == i]
            mine.sort(key=lambda r: num(r["t"]))
            col = colours[k % len(colours)]
            path = " ".join("%.1f,%.1f" % xy(num(r["x"]) - tx, num(r["z"]) - tz) for r in mine)
            parts.append('<polyline points="%s" fill="none" stroke="%s" stroke-width="1.5"/>' % (path, col))
            for r in mine[::10]:
                if r.get("slotX"):
                    sx, sy = xy(num(r["slotX"]) - tx, num(r["slotZ"]) - tz)
                    parts.append('<circle cx="%.1f" cy="%.1f" r="1.5" fill="%s" opacity="0.4"/>' % (sx, sy, col))
            ex, ey = xy(num(mine[-1]["x"]) - tx, num(mine[-1]["z"]) - tz)
            parts.append('<circle cx="%.1f" cy="%.1f" r="3" fill="%s"/>' % (ex, ey, col))
        cx, cy = xy(0, 0)
        parts.append('<circle cx="%.1f" cy="%.1f" r="5" fill="#000"/>' % (cx, cy))
        parts.append('<text x="8" y="%d" font-size="13">B20 %s: each line is one body (dot = where it ended), faint dots = its slots every 10 s, black = target, squares = flank points</text>' % (size + 25, case))
        parts.append("</svg>")
        f = os.path.join(outdir, "b20_%s.svg" % case)
        open(f, "w").write("\n".join(parts))
        written.append(f)
    return written



def rows_of(path):
    rows = []
    exceptions = []
    try:
        text = open(path, encoding="utf-8", errors="replace").read()
    except OSError:
        return rows, ["could not read " + path]
    lines = text.splitlines()
    for i, line in enumerate(lines):
        if "Virtual Machine Exception" in line:
            ctx = " / ".join(l.strip() for l in lines[i:i + 7] if "Reason" in l or "Function" in l or "scripts/" in l)
            exceptions.append(ctx)
        m = re.search(r"RZL R\|([A-Za-z0-9]+)\|(.*)$", line)
        if not m:
            continue
        kv = OrderedDict()
        for part in m.group(2).split("|"):
            if "=" in part:
                k, v = part.split("=", 1)
                kv[k] = v
            elif part:
                kv[part] = ""
        rows.append((m.group(1), kv))
    return rows, exceptions


def table(title, rows):
    cols = []
    for r in rows:
        for k in r:
            if k not in cols:
                cols.append(k)
    print("\n### " + title + "\n")
    print("| " + " | ".join(cols) + " |")
    print("|" + "---|" * len(cols))
    for r in rows:
        print("| " + " | ".join(r.get(c, "") for c in cols) + " |")


def num(v, default=None):
    try:
        return float(v)
    except (TypeError, ValueError):
        return default


def perception_summary(rows):
    """The ladder: one row per condition, one column per distance. A cell is what the infected did
    within 30 s of the target appearing: C = charged, E = counted it an enemy, S = only saw it,
    - = nothing; with the seconds it took."""
    rungs = []
    groups = OrderedDict()
    for r in rows:
        if r.get("note"):
            continue
        d = num(r.get("dist"))
        if d is None:
            continue
        stance = r.get("stance")
        key = (r.get("light"), r.get("terrain"), stance, r.get("factor"))
        rung = "%.0f" % d
        if "hidden" in (stance or ""):
            rung = "hidden"
        elif d > 200:
            rung = "line"
        if rung not in rungs:
            rungs.append(rung)
        cell = "-"
        if num(r.get("chargedAfter"), -1) >= 0:
            cell = "C %.0fs" % num(r.get("chargedAfter"))
        elif num(r.get("enemyAfter"), -1) >= 0:
            cell = "E %.0fs" % num(r.get("enemyAfter"))
        elif num(r.get("seenAfter"), -1) >= 0:
            cell = "S %.0fs" % num(r.get("seenAfter"))
        if rung == "line":
            cell += " (%.0f m)" % d
        groups.setdefault(key, OrderedDict()).setdefault(rung, []).append(cell)

    def order(x):
        if x == "line":
            return 10000
        if x == "hidden":
            return 20000
        return float(x)
    rungs.sort(key=order)
    out = []
    for (light, terrain, stance, factor), cells in groups.items():
        row = OrderedDict([("light", light), ("terrain", terrain), ("stance", stance), ("factor", factor)])
        for rung in rungs:
            title = rung + " m" if rung not in ("line", "hidden") else rung
            row[title] = " / ".join(cells.get(rung, []))
        out.append(row)
    return out


def perception_rates(rows, keyf, title_key):
    """Share of trials (within 30 s) in which the infected saw / engaged (enemy or charge) the target,
    and the median seconds to engage, grouped by keyf(row)."""
    groups = OrderedDict()
    for r in rows:
        if r.get("note") or num(r.get("dist")) is None:
            continue
        k = keyf(r)
        if k is None:
            continue
        groups.setdefault(k, []).append(r)
    def sort_key(k):
        parts = []
        for part in str(k).replace(",", " ").split():
            try:
                parts.append((1, float(part)))
            except ValueError:
                parts.append((0, part))
        return parts
    out = []
    for k in sorted(groups, key=sort_key):
        rs = groups[k]
        n = len(rs)
        seen = sum(1 for r in rs if num(r.get("seenAfter"), -1) >= 0)
        times = sorted(min(t for t in (num(r.get("enemyAfter"), -1), num(r.get("chargedAfter"), -1)) if t >= 0)
                       for r in rs if num(r.get("enemyAfter"), -1) >= 0 or num(r.get("chargedAfter"), -1) >= 0)
        charged = sum(1 for r in rs if num(r.get("chargedAfter"), -1) >= 0)
        engaged_seen = sum(1 for r in rs if num(r.get("seenAfter"), -1) >= 0 and
                           (num(r.get("enemyAfter"), -1) >= 0 or num(r.get("chargedAfter"), -1) >= 0))
        med = "%.0f" % times[len(times) // 2] if times else "-"
        when_seen = "%d%%" % (100 * engaged_seen // seen) if seen else "-"
        out.append(OrderedDict([(title_key, k), ("trials", str(n)), ("seen", "%d%%" % (100 * seen // n)),
                                ("engaged (enemy or charge)", "%d%%" % (100 * len(times) // n)),
                                ("engaged when seen", when_seen),
                                ("charged", "%d%%" % (100 * charged // n)), ("median s to engage", med)]))
    return out


def rung_of(r):
    d = num(r.get("dist"))
    if "hidden" in (r.get("stance") or ""):
        return None
    if d > 200:
        return "line (300-450 m)"
    return "%.0f m" % d


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    all_rows = []
    bad = False
    for n, path in enumerate(sys.argv[1:]):
        rows, exc = rows_of(path)
        print("Log %d: %s (%d rows, %d script exceptions)" % (n + 1, path, len(rows), len(exc)))
        for e in sorted(set(exc)):
            print("  exception: " + e)
        bad = bad or bool(exc)
        all_rows += rows
    by = OrderedDict()
    for t, kv in all_rows:
        by.setdefault(t, []).append(kv)
    for t, title in TITLES.items():
        if t in by:
            table(title, by[t])
            if t == "b1":
                table("B1. Summary per cell", b1_summary(by[t]))
            if t == "perc":
                table("3. Perception ladder: within 30 s, C = charged, E = enemy, S = only seen, - = nothing (seconds after the target appeared)", perception_summary(by[t]))
                f1 = [r for r in by[t] if r.get("factor") == "1.00"]
                table("3. Perception by light and distance (factor 1, all stances and terrain)",
                      perception_rates(f1, lambda r: "%s, %s" % (r.get("light"), rung_of(r)) if rung_of(r) else None, "light, distance"))
                table("3. Perception by stance (factor 1, 25-200 m, all light and terrain)",
                      perception_rates(f1, lambda r: r.get("stance") if num(r.get("dist")) <= 200 else None, "stance"))
                table("3. Perception by terrain (factor 1, 25-200 m)",
                      perception_rates(f1, lambda r: "%s, %s" % (r.get("light"), r.get("terrain")) if num(r.get("dist")) <= 200 and "hidden" not in r.get("stance", "") else None, "light, terrain"))
                table("3. Perception by factor (open, standing and prone, 25-200 m)",
                      perception_rates([r for r in by[t] if r.get("terrain") == "open" and r.get("stance") in ("standing", "prone")],
                                       lambda r: "%s, %s, factor %s" % (r.get("light"), r.get("stance"), r.get("factor")) if num(r.get("dist")) <= 200 else None, "light, stance, factor"))
    for t in by:
        if t not in TITLES and t not in SKIP_TABLE:
            table(t, by[t])
    if "b20t" in by:
        print("\n### B20 tracks: %d rows" % len(by["b20t"]))
        if os.environ.get("SVG_DIR"):
            for f in svg_tracks(by["b20t"], os.environ["SVG_DIR"]):
                print("  drawn: " + f)
    if not any("DONE" in l for p in sys.argv[1:2] for l in open(p, errors="replace")):
        print("\nThe main run did not finish (no 'RZL DONE'): it timed out or crashed.")
        bad = True
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
