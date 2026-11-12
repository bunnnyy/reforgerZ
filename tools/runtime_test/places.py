"""Helpers for the runtime test's task place audit (tools/runtime_test/run.sh tasks; docs/PLACES.md, "The audit").

    python3 places.py prepare <addon dir> <world .ent> ...   # the mod's own game mode settings into the test worlds
    python3 places.py summary <console.log>                  # the audit's lines, for run.sh to print
    python3 places.py draw <console.log> [out.html]          # where each kind of task went, on a map (red: failed)

Nothing here is part of the mod: prepare edits only the throwaway addon copy.
"""
import html, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
GM = os.path.join(REPO, "Prefabs/ReforgerZ/Core/RZ_GameMode.et")


def block(text, start):
    """The text from `start` to the brace that closes the block opened on its line."""
    i = text.index("{", text.index('" {', start) + 1)
    depth = 0
    while True:
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
        i += 1


# The spawn tests make the things a task's words name from the mod's own slots (the server logs
# "Wrong GUID/name" for the mod's prefabs in a loose addon folder, but finds them by their path). The
# specials come from Special Zombies, which is left out on Linux, so A Sample's remains get a vanilla
# civilian as a stand-in special.
CIVILIAN = "{8C7093AF368F496A}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_1.et"


def standins(text):
    # A special that doesn't burst, for A Sample's remains (RZ_OpeningTasks.RemainsPrefab)
    return re.sub(r'(\n *m_aSpecialPrefabs \{\n)[^\n]*\n', lambda m: m.group(1) + '     "%s"\n' % CIVILIAN, text)


def prepare(addon, worlds):
    gm = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    real = standins(block(gm, gm.index("  RZ_GameModeComponent ")))
    for world in worlds:
        path = os.path.join(addon, world)
        text = open(path, encoding="utf-8").read()
        m = re.search(r'  RZ_GameModeComponent "\{[0-9A-F]+\}" \{\n', text)
        old = block(text, m.start())
        text = text.replace(old, real, 1)
        open(path, "w", encoding="utf-8").write(text)


KEEP = ("[ReforgerZ] Places:", "[ReforgerZ] Places audit", "[ReforgerZ] Places fail", "[ReforgerZ] Places need", "[ReforgerZ] Place check", "RZK ")


def summary(log):
    for line in open(log, encoding="utf-8", errors="replace"):
        if any(k in line for k in KEEP):
            i = line.find("Place")
            j = line.find("RZK")
            at = min(x for x in (i, j) if x >= 0)
            print("  " + line[at:].rstrip())


PALETTE = ["#2f6db5", "#7a4fb0", "#1c8a6e", "#b5772f", "#5b7d1f", "#a8416f", "#2d8fa8", "#8a6d1c", "#4c5bb3", "#3c8a3c",
           "#9c5a2e", "#6b6b6b", "#1f6f8b", "#875a9e", "#5e8f2f", "#a36a00", "#3a6ea5", "#7f4f4f", "#2e7d7d", "#6d5bd0"]


def draw(log, out):
    towns, points, lines = [], [], []
    for line in open(log, encoding="utf-8", errors="replace"):
        m = re.search(r"Places town: (-?\d+) (-?\d+) (\d+) (.*)$", line)
        if m:
            towns.append((int(m.group(1)), int(m.group(2)), int(m.group(3)), m.group(4).strip()))
            continue
        m = re.search(r"Places draw: (\S+(?: things)?) ([01]) (-?\d+) (-?\d+) (-?\d+) (-?\d+) (\S+) \| (.*)$", line)
        if m:
            points.append({"key": m.group(1), "ok": m.group(2) == "1", "x": int(m.group(3)), "z": int(m.group(4)),
                           "fx": int(m.group(5)), "fz": int(m.group(6)), "grid": m.group(7), "text": m.group(8).strip()})
            continue
        if "Places:" in line or "Places audit" in line:
            lines.append(line[line.find("Places"):].strip())
    if not points:
        return 0
    xs = [p["x"] for p in points] + [t[0] for t in towns]
    zs = [p["z"] for p in points] + [t[1] for t in towns]
    x0, x1, z0, z1 = min(xs) - 300, max(xs) + 300, min(zs) - 300, max(zs) + 300
    size = 1000.0
    scale = size / max(x1 - x0, z1 - z0)
    keys = sorted({p["key"] for p in points})
    colour = {k: PALETTE[i % len(PALETTE)] for i, k in enumerate(keys)}

    def X(x):
        return round((x - x0) * scale, 1)

    def Y(z):
        return round(size - (z - z0) * scale, 1)

    svg = ['<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 %d %d" class="map">' % (size, size)]
    for x, z, r, name in towns:
        svg.append('<circle cx="%s" cy="%s" r="%s" class="town"/><text x="%s" y="%s" class="tname">%s</text>'
                   % (X(x), Y(z), max(2, round(r * scale, 1)), X(x), Y(z), html.escape(name)))
    for p in points:
        if (p["fx"], p["fz"]) != (p["x"], p["z"]):
            svg.append('<line x1="%s" y1="%s" x2="%s" y2="%s" class="from k-%s"/>' % (X(p["fx"]), Y(p["fz"]), X(p["x"]), Y(p["z"]), keys.index(p["key"])))
    for p in sorted(points, key=lambda q: q["ok"], reverse=True):
        tip = html.escape("%s at %s%s: %s" % (p["key"], p["grid"], "" if p["ok"] else " FAILED", p["text"]))
        svg.append('<circle cx="%s" cy="%s" r="%s" fill="%s" class="pt k-%s%s"><title>%s</title></circle>'
                   % (X(p["x"]), Y(p["z"]), 3.2 if p["ok"] else 5, "#c62828" if not p["ok"] else colour[p["key"]],
                      keys.index(p["key"]), "" if p["ok"] else " bad", tip))
    svg.append("</svg>")
    legend = "".join('<label><input type="checkbox" checked data-k="%d"> <span style="background:%s"></span>%s (%d, %d failed)</label>'
                     % (i, colour[k], html.escape(k), sum(1 for p in points if p["key"] == k), sum(1 for p in points if p["key"] == k and not p["ok"]))
                     for i, k in enumerate(keys))
    fails = "".join("<li>%s</li>" % html.escape("%s at %s: %s" % (p["key"], p["grid"], p["text"])) for p in points if not p["ok"])
    page = """<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Task places</title><style>
:root{--bg:#fbfaf7;--fg:#1d1d1b;--muted:#6b6a65;--town:#d9d4c7}
@media (prefers-color-scheme: dark){:root{--bg:#1b1b1a;--fg:#ecebe6;--muted:#a3a19a;--town:#3a3833}}
body{background:var(--bg);color:var(--fg);font:14px/1.45 system-ui,sans-serif;margin:0 auto;max-width:1100px;padding:16px}
.map{width:100%%;height:auto;border:1px solid var(--town);border-radius:6px}
.town{fill:var(--town);opacity:.55}.tname{font-size:9px;fill:var(--muted);text-anchor:middle}
.from{stroke:var(--muted);stroke-width:.4;opacity:.35}.pt{stroke:var(--bg);stroke-width:.6}.bad{stroke:#fff;stroke-width:1}
.legend{display:flex;flex-wrap:wrap;gap:4px 14px;margin:10px 0}.legend span{display:inline-block;width:10px;height:10px;border-radius:5px;margin-right:2px}
pre{white-space:pre-wrap;font-size:12px;color:var(--muted)}li{font-size:13px}
</style></head><body><h1>Task places</h1><p>%s. Each dot is a place a picker gave; a red, bigger dot failed its need (hover for why). Grey lines join it to where it was asked from.</p>
<div class="legend">%s</div>%s<h2>Summary</h2><pre>%s</pre><h2>Failed places</h2><ul>%s</ul>
<script>document.querySelectorAll('.legend input').forEach(function(c){c.addEventListener('change',function(){document.querySelectorAll('.k-'+c.dataset.k).forEach(function(e){e.style.display=c.checked?'':'none'})})})</script>
</body></html>""" % (html.escape(os.path.basename(os.path.dirname(log)) or log), legend, "".join(svg), html.escape("\n".join(lines)), fails or "<li>none</li>")
    open(out, "w", encoding="utf-8").write(page)
    return len(points)


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "prepare":
        prepare(sys.argv[2], sys.argv[3:])
    elif cmd == "summary":
        summary(sys.argv[2])
    elif cmd == "draw":
        log = sys.argv[2]
        out = sys.argv[3] if len(sys.argv) > 3 else os.path.join(os.path.dirname(os.path.abspath(log)), "task_places.html")
        n = draw(log, out)
        print("%d places drawn: %s" % (n, out))
    else:
        sys.exit(__doc__)
