"""Draws pieces.py and bases.py as a page you can open in a browser: the faction base plans, a camp
concept per faction and a top-down plan of every piece.  Usage: py tools/camp_kit/render.py [out.html]
(default designs/camp_plans.html)."""
import math, os, sys, html
sys.path.insert(0, os.path.dirname(__file__))
from pieces import FP, C, CAMPS

OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(__file__), '..', '..', 'designs', 'camp_plans.html')


def short(name):
    n = name[:-3]
    for a, b in [("_01", ""), ("_02", " 2"), ("_03", " 3"), ("_04", " 4"), ("_US", " US"), ("LitterInterior_USSR_", ""), ("Decorative", ""), ("_burlap", ""), ("_plain", ""),
                 ("_1x1x1", " (big)"), ("_empty_brown", ""), ("ToolsSurgical_autoclavedrum", "sample drum"), ("_begonia_white", ""), ("Assembled", "")]:
        n = n.replace(a, b)
    return n.replace("_", " ").strip()


def rot(x, z, yaw):
    a = math.radians(yaw)
    return x * math.cos(a) + z * math.sin(a), -x * math.sin(a) + z * math.cos(a)


def shape_svg(name, x, z, yaw, scale, ox, oz, extra=""):
    w, d, kind, fill = FP.get(name, (0.5, 0.5, "r", "misc"))
    cx, cy = ox + x * scale, oz - z * scale
    t = "<title>%s</title>" % html.escape(short(name))
    if kind == "c":
        r = max(w * scale / 2, 2.2)
        return '<circle class="k-%s%s" cx="%.1f" cy="%.1f" r="%.1f">%s</circle>' % (fill, extra, cx, cy, r, t)
    ww, dd = max(w * scale, 2.5), max(d * scale, 2.5)
    return '<rect class="k-%s%s" x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="1" transform="rotate(%.1f %.1f %.1f)">%s</rect>' % (
        fill, extra, cx - ww / 2, cy - dd / 2, ww, dd, yaw, cx, cy, t)


def bounds(parts):
    xs, zs = [], []
    for p in parts:
        if p[0] == "p":
            w, d = FP.get(p[1], (0.5, 0.5))[:2]
            r = max(w, d) / 2
            xs += [p[2] - r, p[2] + r]; zs += [p[3] - r, p[3] + r]
        else:
            xs.append(p[2]); zs.append(p[3])
    return min(xs), max(xs), min(zs), max(zs)


def piece_svg(parts, size=230):
    x0, x1, z0, z1 = bounds(parts)
    span = max(x1 - x0, z1 - z0, 4.0) + 1.6
    scale = size / span
    ox = size / 2 - (x0 + x1) / 2 * scale
    oz = size / 2 + (z0 + z1) / 2 * scale
    g = ['<svg viewBox="0 0 %d %d" class="plan" role="img">' % (size, size)]
    # 1 m grid
    for i in range(int(span) + 2):
        gx = ox % scale + i * scale
        gz = oz % scale + i * scale
        g.append('<line class="grid" x1="%.1f" y1="0" x2="%.1f" y2="%d"/><line class="grid" x1="0" y1="%.1f" x2="%d" y2="%.1f"/>' % (gx, gx, size, gz, size, gz))
    stacked = sorted([p for p in parts if p[0] == "p"], key=lambda p: p[5])
    for p in stacked:
        g.append(shape_svg(p[1], p[2], p[3], p[4], scale, ox, oz, " up" if p[5] > 0.3 else ""))
    for s in parts:
        if s[0] != "s":
            continue
        _, tag, x, z, anim, lx, lz = s
        if lx == 0 and lz == 0:
            lx, lz = -x, -z
        cx, cy = ox + x * scale, oz - z * scale
        L = math.hypot(lx, lz) or 1
        tx, ty = cx + lx / L * 11, cy - lz / L * 11
        g.append('<g class="seat"><title>%s, %s</title><line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/><circle cx="%.1f" cy="%.1f" r="4.2"/></g>' % (tag, anim, cx, cy, tx, ty, cx, cy))
    g.append('<g class="front"><path d="M %d 14 l -5 8 h 10 z"/><text x="%d" y="32">front</text></g>' % (size - 18, size - 18))
    g.append('<text class="scale" x="8" y="%d">1 m grid</text>' % (size - 8))
    g.append("</svg>")
    return "".join(g)


def camp_svg(camp, size=520, span=62.0):
    scale = size / span
    o = size / 2
    g = ['<svg viewBox="0 0 %d %d" class="camp" role="img">' % (size, size)]
    for r in (8, 14, 22, 28, 34):
        g.append('<circle class="ring" cx="%d" cy="%d" r="%.1f"/>' % (o, o, r * scale))
    g.append('<path class="road" d="M %.1f 0 C %.1f %.1f, %.1f %.1f, %.1f %.1f"/>' % (o - 10, o + 6, o * 0.5, o - 4, o * 0.75, o + 2, o - 24 * scale))
    for name, cx, cz, yaw in camp[2]:
        parts = C[name][4]
        for p in parts:
            if p[0] != "p":
                continue
            x, z = rot(p[2], p[3], yaw)
            g.append(shape_svg(p[1], cx + x, cz + z, p[4] + yaw, scale, o, o))
        g.append('<text class="lbl" x="%.1f" y="%.1f">%s</text>' % (o + cx * scale, o - cz * scale - 9, html.escape(name.replace("_", " "))))
    g.append('<text class="scale" x="10" y="%d">rings: 8 / 14 / 22 / 28 / 34 m from the centre &#183; road comes in from the top</text>' % (size - 10))
    g.append("</svg>")
    return "".join(g)


FOLDERS = [("Bastion", "Task Force Bastion"), ("RedHand", "The Red Hand"), ("Haven", "The Haven"), ("Halcyon", "Halcyon"), ("Common", "Shared pieces")]
cards = {}
for name, (folder, use, role, intent, parts) in C.items():
    props = [p for p in parts if p[0] == "p"]
    seats = [p for p in parts if p[0] == "s"]
    counts = {}
    for p in props:
        counts[short(p[1])] = counts.get(short(p[1]), 0) + 1
    lst = ", ".join(("%d %s" % (n, k)) if n > 1 else k for k, n in counts.items())
    cards.setdefault(folder, []).append(
        '<article class="card"><header><h3>%s</h3><span class="chip use">%s</span><span class="chip role">%s</span></header>%s<p class="intent">%s</p><p class="props"><b>%d props, %d seats:</b> %s</p></article>'
        % (html.escape(name.replace("_", " ")), use, role, piece_svg(parts), html.escape(intent), len(props), len(seats), html.escape(lst)))

total = len(C)
body = []
body.append('<section class="camps"><h2>How a camp hangs together</h2><p class="lede">Concept plans for a stronghold of each faction, drawn from the same pieces. Today the script places pieces on rings by role; these show the looser layout to aim for, with pieces turned to each other and to the road rather than to the centre.</p><div class="campgrid">')
for camp in CAMPS:
    body.append('<figure class="campfig"><figcaption><h3>%s</h3><p>%s</p></figcaption>%s</figure>' % (camp[0], html.escape(camp[1]), camp_svg(camp)))
body.append("</div></section>")
for folder, title in FOLDERS:
    body.append('<section><h2>%s <span class="count">%d pieces</span></h2><div class="cards">%s</div></section>' % (title, len(cards.get(folder, [])), "".join(cards.get(folder, []))))


from bases import BASES, MODS
import re as _re

def site_svg(plan, size=300, span=130.0):
    sc = size / span
    o = size / 2
    def X(x): return o + x * sc
    def Y(z): return o - z * sc
    g = ['<svg viewBox="0 0 %d %d" class="site" role="img">' % (size, size)]
    for it in plan:
        if it[0] == "road":
            toks = _re.findall(r"[ML]|-?[0-9.]+", it[1]); d = []; i = 0
            while i < len(toks):
                if toks[i] in "ML":
                    d.append("%s %.1f %.1f" % (toks[i], X(float(toks[i+1])), Y(float(toks[i+2])))); i += 3
                else: i += 1
            g.append('<path class="sroad" d="%s"/>' % " ".join(d))
    for it in plan:
        k = it[0]
        if k in ("bld", "yard", "def"):
            x, z, w, d, label = it[1:6]
            cls = "s" + k + (" outline" if len(it) > 6 else "")
            g.append('<rect class="%s" x="%.1f" y="%.1f" width="%.1f" height="%.1f"><title>%s</title></rect>' % (cls, X(x - w / 2), Y(z + d / 2), w * sc, d * sc, html.escape(label)))
            ty = Y(z + d / 2) - 4 if len(it) > 6 else Y(z) + 3
            g.append('<text class="slbl %s" x="%.1f" y="%.1f">%s</text>' % (k, X(x), ty, html.escape(label)))
        elif k == "post":
            x, z, label = it[1:4]
            g.append('<g class="spost"><circle cx="%.1f" cy="%.1f" r="5"/><path d="M %.1f %.1f l 0 -9"/><title>%s</title></g>' % (X(x), Y(z), X(x), Y(z) - 5, html.escape(label)))
    g.append('<text class="scale" x="8" y="%d">schematic &#183; about %d m across</text>' % (size - 8, span))
    g.append("</svg>")
    return "".join(g)

def bases_html():
    out = ['<section class="bases"><h2>How each faction builds</h2><p class="lede">Holdings sit in real towns, so a faction moves into the buildings that suit what it does and fortifies around them. These plans are schematic: the script would pick the right building in each town (largest stone building for Bastion, the garage or petrol station for the Red Hand, the church or a big house for the Haven, the lab building for Halcyon) and dress its yard and road entries with the camp pieces.</p>']
    for b in BASES:
        out.append('<div class="faction f-%s"><h3 class="fname">%s</h3><p class="idea">%s</p><div class="sites">' % (b["key"], b["faction"], html.escape(b["idea"])))
        for st in b["sites"]:
            rows = "".join('<li><b>%s.</b> %s</li>' % (html.escape(a), html.escape(c)) for a, c in st["buildings"])
            out.append('<article class="sitecard"><h4>%s</h4>%s<dl><dt>People</dt><dd>%s</dd><dt>Where</dt><dd>%s</dd><dt>For</dt><dd>%s</dd><dt>Defence</dt><dd>%s</dd></dl><ul class="blds">%s</ul></article>'
                       % (html.escape(st["title"]), site_svg(st["plan"]), html.escape(st["people"]), html.escape(st["where"]), html.escape(st["purpose"]), html.escape(st["defence"]), rows))
        out.append('</div></div>')
    out.append('</section>')
    return "\n".join(out)

body.insert(0, bases_html())

NL2 = chr(10)


def bases_markdown():
    """designs/BASES.md, written from bases.py (edit bases.py, not the .md)."""
    L = ["# Faction bases", "",
         "**Status: designed, not built.** Written by `tools/camp_kit/render.py` from `tools/camp_kit/bases.py`:",
         "edit that file, not this one. The drawings are in [camp_plans.html](camp_plans.html).", "",
         "Today camp life lays out a holding's pieces on rings round its centre "
         "([DESIGN.md](../docs/DESIGN.md#camp-life)). This design has each faction move into the building in "
         "the town that suits what it does, dress its yard and close its road entries, sized to how many "
         "people live there. Building it means camp life picking that building (by size and kind) and "
         "placing pieces relative to it and to the roads; the pieces themselves are built "
         "(`tools/camp_kit/pieces.py`).", ""]
    for b in BASES:
        L += ["## " + b["faction"], "", b["idea"], ""]
        for st in b["sites"]:
            L += ["### " + st["title"], "", "| | |", "|---|---|", "| **People** | %s |" % st["people"], "| **Where** | %s |" % st["where"],
                  "| **For** | %s |" % st["purpose"], "| **Defence** | %s |" % st["defence"], "", "| Building | What it's for |", "|---|---|"]
            L += ["| %s | %s |" % (a, c) for a, c in st["buildings"]]
            L.append("")
    L += ["## Workshop mods to try for the look", "",
          "Researched on 2026-11-12, none added. Adding one is a [MOD_STACK.md](../docs/MOD_STACK.md) decision, "
          "after the compile check and a load test. Sizes are the Workshop's.", "",
          "| Mod | Workshop ID | MB | For | Note |", "|---|---|---|---|---|"]
    L += ["| %s | `%s` | %s | %s | %s |" % m for m in MODS]
    return NL2.join(L) + NL2



NL = chr(10)
page = """<title>Camp Piece Plans</title>
<meta name="claude:viewport-fit-sides" content="cover">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Barlow+Condensed:wght@500;700&family=IBM+Plex+Sans:wght@400;600&family=IBM+Plex+Mono:wght@400&display=swap">
<style>
/* Layout: a surveyor's sheet. Faction sections of plan cards, camp concept plans first. */
:root{--bg:#eef0ea;--paper:#f8f9f4;--ink:#22261f;--muted:#5d6457;--line:#c9cec0;--accent:#b8541c;--seat:#b8541c;
--wood:#a87c4f;--cloth:#7f8f6a;--metal:#8a9196;--sand:#c4ad7c;--tent:#6f8455;--fire:#d0662a;--board:#5a4a3a;--paper2:#e9e6d8;--soil:#6b5639;--net:#7d8a5c;--rubber:#3c3f3a;--glass:#7aa3a8;--light:#e0b23a;--plant:#5d8a4a;--plastic:#dcdcd2;--misc:#9a9a8a;
--display:"Barlow Condensed",Impact,sans-serif;--body:"IBM Plex Sans",system-ui,sans-serif;--mono:"IBM Plex Mono",ui-monospace,monospace;color-scheme:light}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]){--bg:#171a16;--paper:#20241e;--ink:#e6e8df;--muted:#a3aa9a;--line:#3a4036;--accent:#e2814a;--seat:#ef8a4f;--paper2:#cfcab5;--rubber:#5a5e57;--plastic:#bdbdb2;color-scheme:dark}}
:root[data-theme="dark"]{--bg:#171a16;--paper:#20241e;--ink:#e6e8df;--muted:#a3aa9a;--line:#3a4036;--accent:#e2814a;--seat:#ef8a4f;--paper2:#cfcab5;--rubber:#5a5e57;--plastic:#bdbdb2;color-scheme:dark}
body{background:var(--bg);color:var(--ink);font-family:var(--body);font-size:15px;line-height:1.5}
main{max-width:1240px;margin:0 auto;padding-block:28px 60px;padding-left:max(16px,env(safe-area-inset-left,0px));padding-right:max(16px,env(safe-area-inset-right,0px));display:grid;gap:44px}
h1,h2,h3{font-family:var(--display);font-weight:700;letter-spacing:.02em;margin:0;text-wrap:balance}
h1{font-size:2.6rem;line-height:1}h2{font-size:1.7rem;margin-bottom:14px}h3{font-size:1.25rem}
.intro{display:grid;gap:10px;max-width:70ch}.intro p{margin:0;color:var(--muted)}
.key{display:flex;flex-wrap:wrap;gap:10px 18px;font-size:.82rem;color:var(--muted)}.key span{display:inline-flex;align-items:center;gap:6px}
.key i{width:14px;height:10px;border-radius:2px;display:inline-block}
.count{font-family:var(--mono);font-size:.85rem;font-weight:400;color:var(--muted);letter-spacing:0;margin-left:8px}
.lede{margin:0 0 16px;color:var(--muted);max-width:75ch}
.campgrid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,500px),1fr));gap:22px}
.campfig{margin:0;background:var(--paper);border:1px solid var(--line);border-radius:6px;padding:14px;display:grid;gap:10px;min-width:0}
.campfig p{margin:4px 0 0;color:var(--muted);font-size:.9rem}
.cards{display:grid;grid-template-columns:repeat(auto-fill,minmax(min(100%,262px),1fr));gap:16px}
.card{background:var(--paper);border:1px solid var(--line);border-radius:6px;padding:12px;display:grid;gap:8px;align-content:start;min-width:0}
.card header{display:flex;flex-wrap:wrap;align-items:baseline;gap:6px}.card h3{flex:1 1 auto}
.chip{font-family:var(--mono);font-size:.7rem;text-transform:uppercase;letter-spacing:.06em;padding:2px 6px;border-radius:3px;border:1px solid var(--line);color:var(--muted)}
.chip.use{color:var(--accent);border-color:var(--accent)}
.intent{margin:0;font-size:.88rem}.props{margin:0;font-size:.78rem;color:var(--muted)}
svg{display:block;width:100%;height:auto;background:var(--paper2);border-radius:4px}
.camp{background:var(--paper2)}
.grid{stroke:var(--line);stroke-width:.5;opacity:.6}
.ring{fill:none;stroke:var(--line);stroke-dasharray:3 4}
.road{fill:none;stroke:var(--muted);stroke-width:14;opacity:.25;stroke-linecap:round}
.lbl{font-family:var(--mono);font-size:8.5px;fill:var(--ink);text-anchor:middle;paint-order:stroke;stroke:var(--paper2);stroke-width:3px}
.scale{font-family:var(--mono);font-size:9px;fill:var(--muted)}
.front path{fill:var(--muted)}.front text{font-family:var(--mono);font-size:8px;fill:var(--muted);text-anchor:middle}
.seat circle{fill:var(--seat);stroke:var(--paper2);stroke-width:1.2}.seat line{stroke:var(--seat);stroke-width:1.6}
[class^="k-"],[class*=" k-"]{stroke:rgba(0,0,0,.35);stroke-width:.6}
.up{stroke-width:1.2;stroke:var(--ink)}
.k-wood{fill:var(--wood)}.k-cloth{fill:var(--cloth)}.k-metal{fill:var(--metal)}.k-sand{fill:var(--sand)}.k-tent{fill:var(--tent);opacity:.85}.k-fire{fill:var(--fire)}.k-board{fill:var(--board)}
.k-paper{fill:#fff}.k-soil{fill:var(--soil)}.k-net{fill:var(--net);opacity:.45}.k-rubber{fill:var(--rubber)}.k-glass{fill:var(--glass)}.k-light{fill:var(--light)}.k-plant{fill:var(--plant)}.k-plastic{fill:var(--plastic)}.k-misc{fill:var(--misc)}
.bases{display:grid;gap:26px}.faction{display:grid;gap:12px}.fname{font-size:1.5rem}.idea{margin:0;max-width:80ch}
.sites{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,520px),1fr));gap:18px}
.sitecard{background:var(--paper);border:1px solid var(--line);border-radius:6px;padding:14px;display:grid;grid-template-columns:minmax(0,300px) minmax(0,1fr);gap:8px 16px;align-content:start;min-width:0}
.sitecard h4{grid-column:1/-1;margin:0;font-family:var(--display);font-size:1.2rem}
.sitecard dl{margin:0;display:grid;grid-template-columns:auto 1fr;gap:4px 10px;font-size:.86rem;align-content:start}.sitecard dt{font-family:var(--mono);font-size:.72rem;text-transform:uppercase;color:var(--muted);padding-top:2px}.sitecard dd{margin:0}
.blds{grid-column:1/-1;margin:0;padding-left:18px;font-size:.86rem;display:grid;gap:3px}
@media (max-width:640px){.sitecard{grid-template-columns:1fr}}
.site{background:var(--paper2)}.sroad{fill:none;stroke:var(--muted);stroke-width:9;opacity:.3;stroke-linecap:round;stroke-linejoin:round}
.sbld{fill:var(--board);opacity:.8}.syard{fill:var(--sand);opacity:.55}.sdef{fill:var(--accent)}.sdef.outline{fill:none;stroke:var(--accent);stroke-width:1.5;stroke-dasharray:4 3}
.slbl{font-family:var(--mono);font-size:7.5px;text-anchor:middle;fill:var(--ink);paint-order:stroke;stroke:var(--paper2);stroke-width:2.5px}
.spost circle{fill:none;stroke:var(--seat);stroke-width:1.6}.spost path{stroke:var(--seat);stroke-width:1.6}
.mods table{border-collapse:collapse;width:100%;font-size:.86rem}.mods th,.mods td{border-bottom:1px solid var(--line);padding:6px 8px;text-align:left;vertical-align:top}.mods th{font-family:var(--mono);font-size:.72rem;text-transform:uppercase;color:var(--muted)}
.tablewrap{overflow-x:auto;background:var(--paper);border:1px solid var(--line);border-radius:6px}.num{font-variant-numeric:tabular-nums;white-space:nowrap}
@media (prefers-reduced-motion:reduce){*{transition:none!important}}
</style>
<main>
<div class="intro"><h1>Camp piece plans</h1>
<p>Top-down plans for the @@TOTAL@@ camp-life pieces, richer and less regular: props turned at odd angles, stacked, leaned and left where people put them. Each plan is to scale on a 1 m grid; the arrow marks the piece's front. Orange dots are where a person stands or sits, with the way they face. Hover a shape for its prop.</p>
<p>Drawn from <code>tools/camp_kit/pieces.py</code>, the file the prefabs are built from (<code>build.py</code>), so the plans and the game match. Decals (the red hand, the bear, the plague doctor) are still planned, so their boards are plain for now.</p>
<div class="key"><span><i style="background:var(--wood)"></i>wood</span><span><i style="background:var(--cloth)"></i>cloth, sacks</span><span><i style="background:var(--metal)"></i>metal</span><span><i style="background:var(--sand)"></i>sandbags</span><span><i style="background:var(--tent)"></i>tent</span><span><i style="background:var(--fire)"></i>fire</span><span><i style="background:var(--rubber)"></i>tyres</span><span><i style="background:var(--seat);border-radius:50%"></i>seat or work spot</span><span><i style="border:1.5px solid var(--ink)"></i>raised (on a table or stacked)</span></div></div>
@@BODY@@
</main>""".replace("@@TOTAL@@", str(total)).replace("@@BODY@@", NL.join(body))
open(OUT, "w", encoding="utf-8").write(page)
print("written", OUT, total, "pieces")
MD = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "designs", "BASES.md")
open(MD, "w", encoding="utf-8", newline=chr(10)).write(bases_markdown())
print("written", MD)
