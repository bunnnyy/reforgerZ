"""Draws the sites the audit actually laid out (docs/SITES.md, "Seeing it"):

    py tools/camp_kit/render.py --placed <console.log> [out.html]

reads the "Sites draw" lines of a site audit's log (tools/runtime_test/run.sh sites) and writes
tools/camp_kit/sites_placed.html: each site top-down, its ground read on the grid (water, buildings,
walls and trees, bushes, roads, steep ground), its building, road entries, gate and door, every
piece placed (its real footprint) and every piece dropped (dashed, with why), the posts and what
they see, the patrol's loop, any overlap, and the threat round it, with its score. It also writes
tools/camp_kit/sites_list.txt for the Workbench plugin "ReforgerZ: Fly to the next site".
"""
import base64, html, math, os, re, struct, sys, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
COLOURS = {".": (233, 230, 216), "~": (120, 160, 196), "b": (90, 74, 58), "#": (120, 112, 100), "*": (150, 178, 120),
           "=": (178, 176, 168), "^": (205, 180, 140), "k": (160, 130, 180), "p": (233, 210, 170)}
FACTIONS = {"0": "Haven", "1": "Bastion", "2": "Red Hand", "3": "Halcyon"}
KINDS = {"1": "stronghold", "2": "outpost", "3": "safehouse", "4": "road post"}
SECTORS = ["N", "NE", "E", "SE", "S", "SW", "W", "NW"]


def png(rows):
    """A tiny PNG of the grid letters, row 0 at the bottom (south)."""
    h = len(rows)
    w = max(len(r) for r in rows) if rows else 1
    raw = b""
    for r in reversed(rows):
        line = bytearray([0])
        for x in range(w):
            ch = r[x] if x < len(r) else "."
            line += bytes(COLOURS.get(ch, COLOURS["."]))
        raw += bytes(line)

    def chunk(kind, data):
        c = struct.pack(">I", len(data)) + kind + data
        return c + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)

    out = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
    out += chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b"")
    return "data:image/png;base64," + base64.b64encode(out).decode("ascii")


def parse(log):
    sites = {}
    lines = []
    summary = ""
    for line in open(log, encoding="utf-8", errors="replace"):
        m = re.search(r"Sites draw (\d+) (\w+) ?(.*)$", line.rstrip())
        if m:
            sid, kind, rest = m.group(1), m.group(2), m.group(3)
            s = sites.setdefault(sid, {"rows": {}, "pieces": [], "drops": [], "posts": [], "entries": [], "overlaps": [], "route": []})
            bits = rest.split(" ")
            if kind == "site":
                s.update(faction=FACTIONS.get(bits[0], bits[0]), kind=KINDS.get(bits[1], bits[1]), grid=bits[2], cx=float(bits[3]), cz=float(bits[4]), half=float(bits[5]),
                         score=int(bits[6]), key=bits[7], title=" ".join(bits[8:]))
            elif kind == "grid":
                s.update(x0=float(bits[0]), z0=float(bits[1]), n=int(bits[2]))
            elif kind == "row":
                s["rows"][int(bits[0])] = bits[1] if len(bits) > 1 else ""
            elif kind == "building":
                what, coords = rest.split("|")
                v = [float(t) for t in coords.split()]
                s["building"] = (what.strip(), list(zip(v[0::2], v[1::2])))
            elif kind == "entry":
                s["entries"].append((float(bits[0]), float(bits[1]), bits[2]))
            elif kind in ("gate", "door"):
                s[kind] = (float(bits[0]), float(bits[1]))
            elif kind == "around":
                s["around"] = rest
            elif kind in ("piece", "drop"):
                why = ""
                if "|" in rest:
                    rest, why = rest.split("|", 1)
                    bits = rest.split()
                tier, x, z, yaw, name, props = int(bits[0]), float(bits[1]), float(bits[2]), float(bits[3]), bits[4], int(bits[5])
                v = [float(t) for t in bits[6:]]
                rects = [tuple(v[i:i + 5]) for i in range(0, len(v) - 4, 5)]
                item = {"tier": tier, "x": x, "z": z, "yaw": yaw, "name": name, "props": props, "rects": rects, "why": why.strip()}
                s["pieces" if kind == "piece" else "drops"].append(item)
            elif kind == "post":
                s["posts"].append((bits[0], float(bits[1]), float(bits[2]), float(bits[3]), bits[4] == "1"))
            elif kind == "route":
                v = bits
                s["route"] = [(float(v[i]), float(v[i + 1]), v[i + 2] == "1") for i in range(0, len(v) - 2, 3)]
            elif kind == "overlap":
                s["overlaps"].append((float(bits[0]), float(bits[1]), " ".join(bits[2:])))
            elif kind == "goto":
                s["goto"] = tuple(float(t) for t in bits[:4])
            continue
        m = re.search(r"\[ReforgerZ\] (Sites: .*)$", line.rstrip())
        if m:
            lines.append(m.group(1))
        m = re.search(r"\[ReforgerZ\] (Sites audit: \d+ sites,.*)$", line.rstrip())
        if m:
            summary = m.group(1)
    for s in sites.values():
        s["line"] = next((l for l in lines if (" %s at %s:" % (s.get("kind", "?"), s.get("grid", "?"))) in l), "")
    return [sites[k] for k in sorted(sites, key=int) if "grid" in sites[k] and "x0" in sites[k]], summary


def frame(x, z, yaw, lx, lz):
    a = math.radians(yaw)
    return x + lx * math.cos(a) + lz * math.sin(a), z - lx * math.sin(a) + lz * math.cos(a)


def rect_points(px, pz, pyaw, r):
    rx, rz, w, d, ryaw = r
    cx, cz = frame(px, pz, pyaw, rx, rz)
    return [frame(cx, cz, pyaw + ryaw, sx * w / 2, sz * d / 2) for sx, sz in ((-1, -1), (1, -1), (1, 1), (-1, 1))]


def site_svg(s, size=560):
    n = s["n"]
    sc = size / n
    x0, z0 = s["x0"], s["z0"]

    def X(x):
        return (x - x0) * sc

    def Y(z):
        return size - (z - z0) * sc

    def pts(points):
        return " ".join("%.1f,%.1f" % (X(x), Y(z)) for x, z in points)

    rows = [s["rows"].get(i, "") for i in range(n)]
    g = ['<svg viewBox="0 0 %d %d" class="site" role="img"><title>%s</title>' % (size, size, html.escape(s.get("title", "")))]
    g.append('<image href="%s" x="0" y="0" width="%d" height="%d" preserveAspectRatio="none" style="image-rendering:pixelated"/>' % (png(rows), size, size))
    # Threat round the site: eight wedges at the edge, darker for more threat
    around = s.get("around", "")
    m = re.search(r"threat (\S+) front (\d+)", around)
    if m:
        threat = [float(t) for t in m.group(1).split(",")]
        front = int(m.group(2))
        cx, cy = X(s["cx"]), Y(s["cz"])
        for i, t in enumerate(threat):
            if t <= 0:
                continue
            a0 = math.radians(i * 45 - 22.5)
            a1 = math.radians(i * 45 + 22.5)
            r0, r1 = size * 0.47, size * 0.5
            p = [(cx + r0 * math.sin(a0), cy - r0 * math.cos(a0)), (cx + r1 * math.sin(a0), cy - r1 * math.cos(a0)),
                 (cx + r1 * math.sin(a1), cy - r1 * math.cos(a1)), (cx + r0 * math.sin(a1), cy - r0 * math.cos(a1))]
            g.append('<polygon class="threat%s" style="opacity:%.2f" points="%s"><title>%s threat %g</title></polygon>' % (
                " front" if i == front else "", min(0.9, 0.15 + t * 0.12), " ".join("%.1f,%.1f" % q for q in p), SECTORS[i], t))
    if "building" in s:
        g.append('<polygon class="bld" points="%s"><title>%s</title></polygon>' % (pts(s["building"][1]), html.escape(s["building"][0])))
    for d in s["drops"]:
        for r in d["rects"][:d["props"]]:
            g.append('<polygon class="drop" points="%s"><title>%s dropped: %s</title></polygon>' % (pts(rect_points(d["x"], d["z"], d["yaw"], r)), html.escape(d["name"]), html.escape(d["why"])))
    for p in s["pieces"]:
        for i, r in enumerate(p["rects"]):
            cls = "seat" if i >= p["props"] else "prop t%d" % p["tier"]
            g.append('<polygon class="%s" points="%s"><title>%s (tier %d)</title></polygon>' % (cls, pts(rect_points(p["x"], p["z"], p["yaw"], r)), html.escape(p["name"]), p["tier"]))
        g.append('<text class="lbl" x="%.1f" y="%.1f">%s</text>' % (X(p["x"]), Y(p["z"]) - 6, html.escape(p["name"].replace("_", " "))))
    route = s.get("route", [])
    if route:
        g.append('<polygon class="route" points="%s"/>' % pts([(x, z) for x, z, _ in route]))
        for x, z, ok in route:
            g.append('<circle class="rpt%s" cx="%.1f" cy="%.1f" r="4"/>' % ("" if ok else " bad", X(x), Y(z)))
    for x, z, kind in s["entries"]:
        g.append('<circle class="entry %s" cx="%.1f" cy="%.1f" r="6"><title>road entry (%s)</title></circle>' % (kind, X(x), Y(z), kind))
    if "gate" in s:
        gx, gz = s["gate"]
        for kind, px, pz, face, clear in s["posts"]:
            g.append('<line class="sight%s" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % ("" if clear else " blocked", X(px), Y(pz), X(gx), Y(gz)))
        g.append('<path class="gate" d="M %.1f %.1f l -6 10 h 12 z"><title>gate</title></path>' % (X(gx), Y(gz) - 5))
    for kind, px, pz, face, clear in s["posts"]:
        tx, tz = frame(px, pz, face, 0, 6)
        g.append('<g class="post"><circle cx="%.1f" cy="%.1f" r="5"/><line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/><title>%s</title></g>' % (X(px), Y(pz), X(px), Y(pz), X(tx), Y(tz), kind))
    if "door" in s:
        g.append('<rect class="door" x="%.1f" y="%.1f" width="8" height="8"><title>door</title></rect>' % (X(s["door"][0]) - 4, Y(s["door"][1]) - 4))
    for x, z, what in s["overlaps"]:
        g.append('<circle class="overlap" cx="%.1f" cy="%.1f" r="9"><title>%s</title></circle>' % (X(x), Y(z), html.escape(what)))
    g.append('<text class="scale" x="8" y="%d">%d m across &#183; north up</text>' % (size - 8, n))
    g.append("</svg>")
    return "".join(g)


def write(log, out=None):
    sites, summary = parse(log)
    out = out or os.path.join(HERE, "sites_placed.html")
    cards = []
    for s in sites:
        tone = "ok" if s["score"] >= 70 else "low"
        cards.append('<article class="card"><header><h3>%s %s at %s</h3><span class="score %s">%d</span></header><p class="title">%s &#183; %s</p>%s<p class="line">%s</p></article>' % (
            html.escape(s["faction"]), html.escape(s["kind"]), s["grid"], tone, s["score"], html.escape(s.get("title", "")),
            html.escape(s.get("building", ("on open ground", []))[0]), site_svg(s), html.escape(s["line"])))
    page = PAGE.replace("@@SUMMARY@@", html.escape(summary or "%d sites" % len(sites))).replace("@@CARDS@@", "\n".join(cards)).replace("@@LOG@@", html.escape(os.path.basename(os.path.dirname(log))))
    open(out, "w", encoding="utf-8").write(page)
    listing = ["# name x z gate_x gate_z: written by render.py --placed for the Workbench plugin ReforgerZ: Fly to the next site"]
    for s in sites:
        if "goto" in s:
            cx, cz, gx, gz = s["goto"]
            listing.append("%s_%s_%s %.1f %.1f %.1f %.1f" % (s["faction"].replace(" ", ""), s["kind"].replace(" ", ""), s["grid"], cx, cz, gx, gz))
    open(os.path.join(HERE, "sites_list.txt"), "w", encoding="utf-8").write("\n".join(listing) + "\n")
    print("written", out, len(sites), "sites; and tools/camp_kit/sites_list.txt")


PAGE = """<title>Sites As Placed</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
:root{--bg:#eef0ea;--paper:#f8f9f4;--ink:#22261f;--muted:#5d6457;--line:#c9cec0;--accent:#b8541c;--ok:#3f7a3a;--low:#b8541c;--bad:#c0302a;color-scheme:light}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]){--bg:#171a16;--paper:#20241e;--ink:#e6e8df;--muted:#a3aa9a;--line:#3a4036;--accent:#e2814a;--ok:#7cc06f;--low:#e2814a;--bad:#ff6a5e;color-scheme:dark}}
:root[data-theme="dark"]{--bg:#171a16;--paper:#20241e;--ink:#e6e8df;--muted:#a3aa9a;--line:#3a4036;--accent:#e2814a;--ok:#7cc06f;--low:#e2814a;--bad:#ff6a5e;color-scheme:dark}
body{background:var(--bg);color:var(--ink);font-family:system-ui,sans-serif;font-size:15px;line-height:1.45;margin:0}
main{max-width:1240px;margin:0 auto;padding:24px 16px 60px;display:grid;gap:22px}
h1{margin:0;font-size:2rem}h3{margin:0;font-size:1.1rem;flex:1}
.lede{margin:0;color:var(--muted);max-width:80ch}
.cards{display:grid;grid-template-columns:repeat(auto-fill,minmax(min(100%,520px),1fr));gap:18px}
.card{background:var(--paper);border:1px solid var(--line);border-radius:6px;padding:12px;display:grid;gap:8px;min-width:0}
.card header{display:flex;align-items:baseline;gap:8px}
.score{font-weight:700;font-variant-numeric:tabular-nums;padding:1px 8px;border-radius:4px;color:#fff}.score.ok{background:var(--ok)}.score.low{background:var(--low)}
.title,.line{margin:0;font-size:.82rem;color:var(--muted);overflow-wrap:anywhere}
svg{display:block;width:100%;height:auto;border-radius:4px}
.bld{fill:none;stroke:#2b2118;stroke-width:2}
.prop{stroke:rgba(0,0,0,.55);stroke-width:.8}.t1{fill:#c0302a}.t2{fill:#d08a2a}.t3{fill:#6f8455}.t4{fill:#7d6ab0}.t5{fill:#9a9a8a}
.seat{fill:rgba(184,84,28,.35);stroke:#b8541c;stroke-width:.6}
.drop{fill:none;stroke:#c0302a;stroke-width:1.2;stroke-dasharray:3 2}
.lbl{font-size:8px;text-anchor:middle;fill:#22261f;paint-order:stroke;stroke:#f8f9f4;stroke-width:2.5px}
.route{fill:none;stroke:#3a5f9a;stroke-width:1.2;stroke-dasharray:5 4}.rpt{fill:#3a5f9a}.rpt.bad{fill:#c0302a}
.entry{fill:none;stroke:#22261f;stroke-width:1.5}.entry.main{stroke:#c0302a;stroke-width:2.5}
.gate{fill:#c0302a}.door{fill:#22261f}
.post circle{fill:#fff;stroke:#3a5f9a;stroke-width:2}.post line{stroke:#3a5f9a;stroke-width:2}
.sight{stroke:#3a5f9a;stroke-width:.8;stroke-dasharray:2 3}.sight.blocked{stroke:#c0302a}
.overlap{fill:none;stroke:#c0302a;stroke-width:2.5}
.threat{fill:#c0302a}.threat.front{stroke:#22261f;stroke-width:1.5}
.scale{font-size:9px;fill:#22261f}
.key{display:flex;flex-wrap:wrap;gap:6px 16px;font-size:.8rem;color:var(--muted)}.key i{display:inline-block;width:12px;height:10px;margin-right:5px;vertical-align:middle}
</style>
<main>
<h1>Sites as placed</h1>
<p class="lede">@@SUMMARY@@</p>
<p class="lede">Drawn from the site audit's log (@@LOG@@) by <code>tools/camp_kit/render.py --placed</code>: the layouts the game actually laid out, as data, on the ground it read. Each piece is drawn as its real footprint; dashed red outlines are pieces dropped (hover for why). The ring's wedges are the threat each side carries (the outlined one is the front).</p>
<div class="key"><span><i style="background:#e9e6d8"></i>open</span><span><i style="background:#78a0c4"></i>water</span><span><i style="background:#5a4a3a"></i>building</span><span><i style="background:#787064"></i>wall, tree, rock, car</span><span><i style="background:#96b278"></i>bush</span><span><i style="background:#b2b0a8"></i>road</span><span><i style="background:#cdb48c"></i>steep</span><span><i style="background:#a082b4"></i>other scenery</span><span><i style="background:#c0302a"></i>tier 1</span><span><i style="background:#d08a2a"></i>tier 2</span><span><i style="background:#6f8455"></i>tier 3</span><span><i style="background:#7d6ab0"></i>tier 4</span><span><i style="background:#9a9a8a"></i>tier 5</span></div>
<div class="cards">
@@CARDS@@
</div>
</main>
"""

if __name__ == "__main__":
    write(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)
