"""Prints the zombie lab's results as tables (tools/zombie_lab/run.sh calls it).

    python3 tools/zombie_lab/report.py <console.log> [<console.log of the restart boot>]

Rows are the lab's "RZL R|table|key=value|..." lines. Exit code 1 if a log has script exceptions.
"""
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
])


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
        m = re.search(r"RZL R\|([A-Za-z]+)\|(.*)$", line)
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
        if t not in TITLES:
            table(t, by[t])
    if not any("DONE" in l for p in sys.argv[1:2] for l in open(p, errors="replace")):
        print("\nThe main run did not finish (no 'RZL DONE'): it timed out or crashed.")
        bad = True
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
