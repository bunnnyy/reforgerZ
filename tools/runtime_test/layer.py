"""Helpers for the runtime test's behaviour layer test (tools/runtime_test/run.sh layer; docs/INFECTED.md,
"What the server test shows").

    python3 layer.py prepare <addon dir> <world .ent>   # tribe slots, Bacon's faction, the navmesh, the fake player in the lookups
    python3 layer.py summary <console.log>              # the results table (exit 1 if a check failed)
    python3 layer.py navmesh <addon dir> <world .ent>   # only the navmesh and the perception manager (run.sh patrols)

Nothing here is part of the mod: prepare edits only the throwaway addon copy.
"""
import os, re, sys

BACON_FACTION = '''SCR_FactionManager RZSitesFactions : "{4A188E44289B9A50}Prefabs/MP/Managers/Factions/FactionManager_Editor.et" {
 coords 1000 50 1010
 Factions {
  SCR_Faction "{622120A5448725E3}" : "{297E7D166247105C}622120A5448725E3/Configs/BaconZ_Faction.conf" {
  }
 }
}'''

# The site audit's world places the map's AI world for its roads only. Bodies need the rest: the map's
# soldier navmesh (without it every AI move fails and Bacon's bodies walk off toward the world's
# origin) and a perception manager (without it no AI ever sees anyone). As in the zombie lab's world
# and the game's own GM_Arland.
AI_WORLD = '''SCR_AIWorld RZSitesAIWorld : "{01DC74137CFDDB6A}Prefabs/AI/SCR_AIWorld_Arland.et" {
 components {
  NavmeshWorldComponent "{5584F30E67F617AD}" {
   NavmeshSettings NavmeshWorld "{50FC63BEBE3973C5}" {
    NavmeshFilesConfig BaseNavmeshFilesConfig "{6A1E5C3B2D4F7081}" : "{418D8BF6A342C0BD}Configs/Navmesh/Navmesh_GM_Arland_Soldier.conf" {
    }
   }
  }
  NavmeshWorldComponent "{5584F30EEFEE1223}" {
   NavmeshSettings ChimeraNavmeshWorld "{50FC63BEBE3973C5}" {
    NavmeshFilesConfig BaseNavmeshFilesConfig "{6A1E5C3B2D4F7082}" {
     NavmeshFile "{A0AEEB1E7EF474FA}worlds/GameMaster/Navmeshes/GM_Arland_vehicles.nmn"
    }
   }
  }
 }
 coords 0 0 0
}
PerceptionManager RZSitesPerception : "{028DAEAD63E056BE}Prefabs/World/Game/PerceptionManager.et" {
 coords 1000 50 1030
}'''

# The fake player (id 901, a spawned civilian) is no engine player, so the player manager knows
# neither its entity nor its id. The mod's lookups under Zombies/ and in the game mode go through
# RZB_Who in the copy, which answers for 901 and asks the player manager for everyone else.
LOOKUPS = (
    ("!GetGame().GetPlayerManager().GetPlayerController(hunt.m_iTarget)", "!RZB_Who.Online(hunt.m_iTarget)"),
    ("GetGame().GetPlayerManager().GetPlayerControlledEntity(", "RZB_Who.Ent("),
    ("GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(", "RZB_Who.Id("),
)


def prepare(addon, world):
    path = os.path.join(addon, world)
    text = open(path, encoding="utf-8").read()
    # Arland's wild land is under the floor for a tribe: two slots, as the full test's world has
    text, n = re.subn(r'(\n *m_TribeSettings RZ_TribeSettings "\{[0-9A-F]+\}" \{\n)', lambda m: m.group(1) + "    m_iSlots 2\n", text)
    assert n == 1, "no tribe settings in the world's game mode"
    # Bacon's own faction, as in the zombie lab's world: its infected are enemies of everyone else
    text, n = re.subn(r'SCR_FactionManager RZSitesFactions : "[^"]+" \{\n coords [^\n]+\n\}', lambda m: BACON_FACTION, text)
    assert n == 1, "no faction manager in the world"
    text, n = re.subn(r'SCR_AIWorld RZSitesAIWorld : "[^"]+" \{\n coords [^\n]+\n\}', lambda m: AI_WORLD, text)
    assert n == 1, "no AI world in the world"
    open(path, "w", encoding="utf-8").write(text)

    base = os.path.join(addon, "scripts", "Game", "ReforgerZ")
    files = [os.path.join(base, "Core", "RZ_GameModeComponent.c")]
    for root, _, names in os.walk(os.path.join(base, "Zombies")):
        files += [os.path.join(root, f) for f in names if f.endswith(".c")]
    counts = [0, 0, 0]
    for f in files:
        src = open(f, encoding="utf-8").read()
        new = src
        for i, (a, b) in enumerate(LOOKUPS):
            counts[i] += new.count(a)
            new = new.replace(a, b)
        if new != src:
            open(f, "w", encoding="utf-8").write(new)
    assert counts[0] == 1 and counts[1] >= 8 and counts[2] >= 2, "the mod's player lookups moved: %s" % counts


# ---------------------------------------------------------------------------------------- the summary
STAGES = {1: "SETUP", 2: "HUNT", 3: "ENGAGE", 4: "AWAY", 5: "HOME", 6: "HEAR", 7: "SHOTSET", 8: "SHOT", 9: "AWAY2",
          10: "CONTACTSET", 11: "CONTACT", 12: "PUTAWAY"}
REACH = {"YOUNG": 35, "HUNTER": 25, "SCOUT": 15, "GUARD": 30, "ALPHA": 40}     # docs/INFECTED.md, "The kinds"
SPRINT = 4.4                                                                    # a cap is a fraction of this (B7)
NUM = ("t", "st", "u", "i", "d", "v", "sd", "tg", "ss", "pf", "hp", "mel", "tb", "cap", "n", "drv", "hunt", "arr", "slp",
       "deaf", "dl", "df", "ds", "seen", "tgt", "alive", "lost", "others", "undriven", "hour", "phase", "from", "to", "pass",
       "home", "hold", "torest", "body")


def kv(text):
    d = {}
    for part in text.split():
        k, sep, v = part.partition("=")
        if not sep:
            continue
        if k in NUM:
            try:
                v = float(v)
            except ValueError:
                pass
        d[k] = v
    return d


def xz(p):
    x, _, z = p.partition(",")
    return float(x), float(z)


def dist(a, b):
    return ((a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2) ** 0.5


def med(values):
    s = sorted(values)
    if not s:
        return 0.0
    return s[len(s) // 2]


def pct(values, q):
    s = sorted(values)
    if not s:
        return 0.0
    return s[min(len(s) - 1, int(len(s) * q))]


def span(values, fmt="%d"):
    if not values:
        return "-"
    return (fmt + "-" + fmt) % (min(values), max(values))


def load(log):
    data = {"b": [], "g": [], "f": [], "dmg": [], "part": [], "ev": [], "stage": [], "status": [], "exc": [], "calls": [], "mod": [],
            "errors": [], "done": False}
    lines = open(log, encoding="utf-8", errors="replace").read().split("\n")
    now = 0
    for n, line in enumerate(lines):
        i = line.find("RZB ")
        if i >= 0:
            tag, _, rest = line[i + 4:].partition(" ")
            if tag in ("b", "g", "f", "dmg", "part"):
                d = kv(rest)
                data[tag].append(d)
                if tag == "f":
                    now = d["t"]
            elif tag in ("ev", "stage", "status"):
                head, _, text = rest.partition(" | ")
                d = kv(head)
                d["text"] = text
                data[tag].append(d)
            elif tag == "done:":
                data["done"] = True
        elif "Virtual Machine Exception" in line:
            block = lines[n + 1:n + 16]
            reason = next((b.split("Reason:", 1)[1].strip() for b in block if "Reason:" in b), "?")
            where = next((b.strip() for b in block if b.strip().startswith("scripts/")), "?")
            ours = any("ReforgerZ" in b or "/RZ" in b for b in block if b.strip().startswith("scripts/"))
            data["exc"].append((reason.split(" from <")[0], where.split(" Function ")[-1], ours))
        elif "[ReforgerZ] Tribes: " in line:
            text = line.split("[ReforgerZ] Tribes: ", 1)[1]
            data["mod"].append((now, text))
            if "a scout of" in text and "called on player" in text:
                data["calls"].append(now)
        elif "SCRIPT    (E): " in line and "@\"" not in line and now > 0:
            data["errors"].append(line.split("SCRIPT    (E): ", 1)[1].strip())
    return data


def summary(log):
    data = load(log)
    out = []
    fails = []

    def row(item, what, ok):
        verdict = "pass"
        if ok is None:
            verdict = "not shown"
        elif ok == "info":
            verdict = "measured"
        elif not ok:
            verdict = "FAIL"
            fails.append(item)
        out.append((item, what, verdict))

    starts = [s["t"] for s in data["stage"] if s["to"] == 1]
    passes = len(starts)

    def pass_of(t):
        p = -1
        for k, s in enumerate(starts):
            if t >= s:
                p = k
        return p

    hours = {}
    for e in data["ev"]:
        m = re.search(r"pass (\d+) at hour (\d+)", e["text"])
        if m:
            hours[int(m.group(1))] = int(m.group(2))

    bodies = {}
    for r in data["b"]:
        bodies.setdefault(r["i"], []).append(r)
    groups = {}
    for r in data["g"]:
        groups[(r["u"], r["t"])] = r
    shots = [(e["t"], e["text"]) for e in data["ev"] if e["text"].startswith("shot ")]
    hurt = {}
    for r in data["dmg"]:
        if r["by"] != "self":
            hurt.setdefault(r["i"], []).append(r["t"])
    selfhurt = [r for r in data["dmg"] if r["by"] == "self"]
    friendly = [r for r in data["dmg"] if r["by"].startswith("Bacon")]

    # Mode changes, and each row's seconds in its mode
    changes = []
    for i, rows in bodies.items():
        since = rows[0]["t"]
        for k, r in enumerate(rows):
            if k and (rows[k - 1]["m"] != r["m"] or r["t"] - rows[k - 1]["t"] > 3):
                since = r["t"]
                if rows[k - 1]["m"] != r["m"] and r["t"] - rows[k - 1]["t"] <= 3:
                    changes.append((rows[k - 1], r))
            r["age"] = r["t"] - since
            r["g"] = groups.get((r["u"], r["t"]), {})
    releases = [(a, b) for a, b in changes if b["m"] == "ENGAGE"]
    takebacks = [(a, b) for a, b in changes if a["m"] == "ENGAGE" and b["m"] == "RECOVER"]
    resteers = [(a, b) for a, b in changes if a["m"] == "RECOVER" and b["m"] == "STEER"]

    # ---- put out and steered: the hunt's bodies from their first second to their first release
    walked = []
    for i, rows in bodies.items():
        for p in range(passes):
            mine = [r for r in rows if pass_of(r["t"]) == p and r["st"] in (2, 3)]
            if not mine or not any(r["g"].get("hunt") == 1 for r in mine):
                continue
            steer = []
            for r in mine:
                if r["m"] != "STEER":
                    break
                steer.append(r)
            if len(steer) < 3:
                continue
            still = longest = 0
            for r in steer:
                if r["v"] < 0.2 and r["sd"] > 8 and r["slot"] != r["p"]:
                    still += 1
                    longest = max(longest, still)
                else:
                    still = 0
            walked.append({"i": i, "k": steer[0]["k"], "p": p, "d0": steer[0]["d"], "dmin": min(r["d"] for r in steer),
                           "dend": steer[-1]["d"], "secs": len(steer), "still": longest, "released": len(steer) < len(mine)})
    if walked:
        closed = [w for w in walked if w["released"] or w["dmin"] <= max(REACH.get(w["k"], 25) + 15, 0.6 * w["d0"])]
        rest = [w for w in walked if w not in closed]
        frozen = [w for w in walked if w["still"] >= 40]
        ranoff = [w for w in walked if w["dend"] > w["d0"] + 50]
        text = "%d bodies of the hunt's groups in %d passes, put out %s m from the player (median %d); %d were released or came at least 40%% nearer; " \
               "%d had not when the fight stage ended (nearest %s m); stood 40 s or more with a slot to walk to: %d; " \
               "ended 50 m further off than they began: %d" % (len(walked), passes, span([w["d0"] for w in walked]), med([w["d0"] for w in walked]),
                                                                len(closed), len(rest), span([w["dmin"] for w in rest]), len(frozen), len(ranoff))
        row("Put out and steered to the player", text, len(closed) >= 0.5 * len(walked) and not frozen and not ranoff)
    else:
        row("Put out and steered to the player", "no body of a hunting group was sampled", False)

    # ---- how fast a hunting group closes, by what the plan told it to do
    pace = {}
    for i, rows in bodies.items():
        prev = None
        for r in rows:
            if r["m"] != "STEER" or r["g"].get("hunt") != 1 or r["st"] not in (2, 3) or r["d"] < 0:
                prev = None
                continue
            key = "approach"
            if r["g"].get("hs") == "STRIKE":
                key = "strike"
            if prev is not None and prev["t"] == r["t"] - 1 and r["slot"] != r["p"]:
                e = pace.setdefault(key, {"secs": 0, "closed": 0.0, "moving": 0, "path": 0.0})
                e["secs"] += 1
                e["closed"] += prev["d"] - r["d"]
                e["path"] += r["v"]
                if r["v"] > 0.5:
                    e["moving"] += 1
            prev = r
    parts = []
    for key in ("approach", "strike"):
        e = pace.get(key)
        if e and e["secs"] >= 20:
            parts.append("%s: %d body-seconds, on the move %d%% of them, %.1f m/s over the ground on average, %.1f m/s nearer the player" % (
                key, e["secs"], 100 * e["moving"] / e["secs"], e["path"] / e["secs"], e["closed"] / e["secs"]))
    pauses = []
    for i, rows in bodies.items():
        n = 0
        for r in rows:
            if r["m"] == "STEER" and r["v"] < 0.2 and 8 < r["sd"] < 40 and r["slot"] != r["p"] and r["g"].get("slp") != 1 and r["g"].get("hunt") == 1:
                n += 1
            else:
                if n:
                    pauses.append(n)
                n = 0
    row("How fast a steered group closes", "; ".join(parts) + ". They move in hops: %d pauses of a hunting body with a slot 8-40 m off, median %d s, longest %d s" % (
        len(pauses), med(pauses), max(pauses or [0])), "info")

    # ---- nobody stuck: a steered body that stands a minute with a slot to walk to
    stuck = {}
    for i, rows in bodies.items():
        n = 0
        for r in rows:
            if r["m"] == "STEER" and r["v"] < 0.2 and r["sd"] > 8 and r["slot"] != r["p"] and r["g"].get("slp") != 1:
                n += 1
                if n >= 60:
                    stuck[i] = max(stuck.get(i, 0), n)
            else:
                n = 0
    row("Nobody stuck", "%d of %d bodies stood a minute or more with a slot more than 8 m off (longest %d s)" % (
        len(stuck), len(bodies), max(list(stuck.values()) or [0])), not stuck)

    # ---- speed while steered, against the cap the layer set
    speed = {}
    over = 0
    for r in data["b"]:
        if r["m"] != "STEER" or r["v"] < 0.5 or r["age"] < 2:
            continue
        gait = "uncapped"
        limit = SPRINT + 0.4
        if r["k"] == "ALPHA":
            limit = 5.3
        if r["cap"] < 0.99:
            gait = "capped"
            limit = r["cap"] * SPRINT * 1.15 + 0.3
        speed.setdefault((r["k"], gait), []).append(r["v"])
        if r["v"] > limit:
            over += 1
    parts = []
    for (k, gait), vs in sorted(speed.items()):
        if len(vs) >= 10:
            parts.append("%s %s: median %.1f, 95%% under %.1f m/s (%d s)" % (k.lower(), gait, med(vs), pct(vs, 0.95), len(vs)))
    moving = sum(len(v) for v in speed.values())
    row("Speed while steered", "; ".join(parts) + "; seconds over its cap's speed: %d of %d" % (over, moving), moving > 0 and over <= 0.02 * moving)

    # ---- blind while steered: Bacon never has the player as an enemy
    steer = [r for r in data["b"] if r["m"] == "STEER" and r["age"] >= 3 and r["d"] >= 0]
    enemy = [r for r in steer if r["tg"] == 1 and 0 <= r["ss"] < 2]
    band = [r for r in steer if 30 <= r["d"] <= 100 and r["tb"] == 0]
    near = [r for r in steer if r["d"] < 30 and r["tb"] == 0]
    shape = [r for r in band + near if 0 <= r["ss"] < 2]
    factor = [r for r in data["b"] if r["m"] in ("STEER", "RECOVER") and r["age"] >= 2 and r["pf"] > 0.001]
    fight = [r for r in data["b"] if r["m"] == "ENGAGE" and r["age"] >= 3 and 0 <= r["d"] <= 30 and r["tb"] == 0]
    saw = [r for r in fight if r["tg"] == 1 and 0 <= r["ss"] < 2]
    text = "%d steered body-seconds: Bacon held the player as an enemy in view in %d; with the terrain's sight line open at 30-100 m: %d of %d; under 30 m: %d of %d; " \
           "stored factor above 0: %d. Its perception still noticed him there (seen in the last 2 s, never as an enemy) in %d of those %d. " \
           "The same read on engaged bodies within 30 m: an enemy in view in %d of %d" % (
               len(steer), len(enemy), len([r for r in band if r in enemy]), len(band), len([r for r in near if r in enemy]), len(near),
               len(factor), len(shape), len(band) + len(near), len(saw), len(fight))
    row("Blind while steered", text, len(band) >= 30 and not enemy and not factor and len(saw) > 0.5 * max(1, len(fight)))

    # ---- releases, each put to a cause
    marked = []
    for a, b in releases:
        dmin = min(a["d"], b["d"])
        g = b["g"] or a["g"]
        why = ""
        if any(b["t"] - 2 <= t <= b["t"] for t in hurt.get(b["i"], [])):
            why = "hit"
        elif any(b["t"] - 2 <= t <= b["t"] and text.startswith("shot release at") for t, text in shots) and dmin <= 33:
            why = "shot"
        elif dmin <= 9.5:
            why = "contact"
        elif g.get("hunt") == 1 and g.get("hs") == "STRIKE" and dmin <= REACH.get(b["k"], 25) + 3:
            why = "strike"
        elif b["k"] == "SCOUT" and any(b["t"] - 3 <= t <= b["t"] + 1 for t in data["calls"]):
            why = "scout's call"
        marked.append([a, b, why, dmin])
    for _ in range(3):
        for m in marked:
            if m[2]:
                continue
            for o in marked:
                if o is not m and o[2] and abs(o[1]["t"] - m[1]["t"]) <= 2 and dist(xz(o[1]["p"]), xz(m[1]["p"])) <= 20:
                    m[2] = "shared"
                    break
    causes = {}
    for a, b, why, dmin in marked:
        causes.setdefault(why or "unexplained", []).append(dmin)
    parts = []
    for why in ("strike", "contact", "shot", "hit", "scout's call", "shared", "unexplained"):
        if why in causes:
            ds = causes[why]
            parts.append("%s %d (at %s m, median %d)" % (why, len(ds), span(ds), med(ds)))
    for m in marked:
        if not m[2]:
            parts.append("unexplained: t=%d %s at %.0f m" % (m[1]["t"], m[1]["k"].lower(), m[3]))
    row("Released to Bacon, by cause", "%d releases: %s. A body's own falls and trips: %d (about 2 of 100 each), none released it" % (
        len(releases), "; ".join(parts), len(selfhurt)), "unexplained" not in causes and len(releases) > 0)
    strikes = causes.get("strike", [])
    row("Release on a strike within reach", "%d (at %s m; reach 25 m a hunter, 35 a young, 30 a guard, 40 an alpha)" % (len(strikes), span(strikes)), len(strikes) > 0)
    contacts = [m for m in marked if m[2] == "contact" and m[1]["st"] == 11]
    lag = [m[1]["t"] - max([s["t"] for s in data["stage"] if s["to"] == 11 and s["t"] <= m[1]["t"]] or [m[1]["t"]]) for m in contacts]
    row("Release on contact (player 5-7 m off, in sight)", "%d bodies in %d passes, %s s after he stood there (with those sharing it: %d)" % (
        len(contacts), len(set(pass_of(m[1]["t"]) for m in contacts)), span(lag), len([m for m in marked if m[1]["st"] == 11])), len(contacts) > 0)

    # ---- the shot: nobody released in the seconds before it, everyone within 30 m at once
    shot_rows = 0
    for t, what in shots:
        if not what.startswith("shot release at"):
            continue
        m = re.search(r"steer (\d+) > (\d+), engage (\d+) > (\d+)", what)
        before = [r for r in data["b"] if r["t"] == t and 0 <= r["d"] <= 30]
        steered = [r for r in before if r["m"] != "ENGAGE"]
        early = [b for a, b in releases if t - 4 <= b["t"] < t and b["st"] == 7]
        after = [r for r in data["b"] if r["t"] == t + 1 and r["i"] in [x["i"] for x in steered]]
        gone = [r for r in after if r["m"] == "ENGAGE"]
        text = "pass %d: the player stood %s m from %d steered bodies for 3 s and %d were released; at the shot the layer's own count went steer %s > %s, " \
               "engage %s > %s inside the call; a second later %d of the %d were engaged" % (
                   pass_of(t), span([r["d"] for r in steered]), len(steered), len(early), m.group(1), m.group(2), m.group(3), m.group(4), len(gone), len(steered))
        row("Release on a shot within 30 m", text, bool(steered) and not early and len(gone) == len(steered) and int(m.group(4)) > int(m.group(3)))
        shot_rows += 1
    if not shot_rows:
        row("Release on a shot within 30 m", "the stage found no steered body to stand 21-26 m from", None)

    # ---- the fight
    reached = []
    for a, b in releases:
        if b["st"] not in (2, 3, 7, 8, 10, 11) or min(a["d"], b["d"]) > 45:
            continue
        rows = [r for r in bodies[b["i"]] if b["t"] <= r["t"] <= b["t"] + 40]
        at = next((r["t"] - b["t"] for r in rows if r["m"] == "ENGAGE" and 0 <= r["d"] <= 2.5), None)
        reached.append(at)
    stamps = len([r for r in data["b"] if 0 <= r["mel"] < 1.2 and r["st"] in (3, 8, 11)])
    lost = [f for f in data["f"] if f["lost"] > 0 and f["st"] in (3, 8, 11)]
    got = [x for x in reached if x is not None]
    chase = [r["v"] for r in data["b"] if r["m"] == "ENGAGE" and r["v"] > 0.5 and 3 <= r["d"] <= 45 and r["tg"] == 1]
    deaths = len([e for e in data["ev"] if "fake player died" in e["text"]])
    text = "%d bodies released within 45 m in the fight stages: %d reached the player (2.5 m) in a median %d s, chasing at a median %.1f m/s (95%% under %.1f); " \
           "Bacon's melee test passed in %d body-seconds; the player lost health in %d seconds, %.1f times his whole health in all " \
           "(made whole twenty times a second; he still died %d times). Bodies hurt by another infected's swing: %d times" % (
               len(reached), len(got), med(got), med(chase), pct(chase, 0.95), stamps, len(lost), sum(f["lost"] for f in lost), deaths, len(friendly))
    row("Engage: chase, reach, the blow lands", text, len(got) > 0 and stamps > 0 and len(lost) > 0)

    # ---- taken back, recovered, steered again
    never = []
    waits = []
    backs = []
    for e in data["ev"]:
        if "fake moved away" not in e["text"] or e["st"] not in (4, 9):
            continue
        t0 = e["t"]
        for i, rows in bodies.items():
            now = next((r for r in rows if r["t"] == t0), None)
            if not now or now["m"] != "ENGAGE":
                continue
            later = [r for r in rows if r["t"] > t0]
            rec = next((r["t"] - t0 for r in later if r["m"] == "RECOVER"), None)
            ste = next((r["t"] - t0 for r in later if r["m"] == "STEER"), None)
            if rec is None:
                if len(later) > 60:
                    never.append(i)
                continue
            waits.append(rec)
            if ste is not None:
                backs.append(ste)
    moved = 0
    for a, b in resteers:
        after = [r for r in bodies[b["i"]] if b["t"] <= r["t"] <= b["t"] + 30 and r["m"] == "STEER"]
        if any(r["v"] > 0.5 for r in after) or any(r["sd"] < 8 for r in after):
            moved += 1
    feet = [r for r in data["b"] if r["m"] == "RECOVER" and r["age"] >= 3 and 0 <= r["d"] <= 3 and 0 <= r["mel"] < 1.5]
    text = "%d bodies were engaged when the player was moved 420 m or more off: taken back after %s s (median %d), steered again after %s s (median %d); " \
           "not taken back within a minute: %d. In all %d take-backs and %d steered again, %d of which then walked or stood on their slot. " \
           "Taken back while still swinging at the player's feet (a wounded hunter's break-off): %d body-seconds" % (
               len(waits) + len(never), span(waits), med(waits), span(backs), med(backs), len(never), len(takebacks), len(resteers), moved, len(feet))
    row("Take-back and recover", text, len(waits) > 0 and not never and max(waits) <= 45 and (not backs or max(backs) <= 75) and moved >= 0.9 * len(resteers))

    # ---- the hunt lost, the walk home
    lostlines = [t for t, text in data["mod"] if "lost player" in text and "goes home" in text]
    broke = [t for t, text in data["mod"] if "breaks off" in text]
    homes = [s for s in data["stage"] if s["from"] == 5]
    walking = [s for s in homes if s["text"].startswith("walking home")]
    holds = [p["hold"] for p in data["part"] if p["st"] == 5 and p.get("home") == 1 and p["t"] in [s["t"] for s in data["stage"] if s["to"] == 5]]
    text = "the hunt ended by itself in %d of %d passes (%d lost him for 60 s with every group 300 m off, %d broke off); the parts sent home hold where they are first " \
           "(%s s left at that moment, by design up to 600); %s" % (
               len(lostlines) + len(broke), passes, len(lostlines), len(broke), span(holds), "; ".join("pass %d: %s" % (pass_of(s["t"] - 1), s["text"]) for s in homes))
    ok = None
    if walking:
        ok = True
    row("Home", text, ok if len(lostlines) + len(broke) >= 1 else False)

    # ---- put away
    away = [s for s in data["stage"] if s["from"] == 12]
    put = [s for s in away if "back to data" in s["text"]]
    how = [e["text"] for e in data["ev"] if e["text"].startswith("put away:")]
    row("Put away", "; ".join("pass %d: %s; %s" % (pass_of(s["t"] - 1), h, s["text"]) for s, h in zip(away, how)), len(put) == len(away) and len(away) > 0)

    # ---- hearing: a shot gives a steered group a lead
    heard = []
    for t, what in shots:
        if not what.startswith("shot hearing at"):
            continue
        best = None
        for u in set(r["u"] for r in data["g"] if r["t"] == t):
            rows = [r for r in data["g"] if r["u"] == u and t <= r["t"] <= t + 75 and r["ds"] >= 0]
            if not rows or rows[0]["hunt"] == 1 or rows[0]["deaf"] == 1 or rows[0]["slp"] == 1 or not 95 <= rows[0]["ds"] <= 135:
                continue
            gain = rows[0]["ds"] - min(r["ds"] for r in rows)
            if not best or gain > best[2]:
                best = (rows[0]["part"], rows[0]["ds"], gain, any(r["hunt"] == 1 for r in rows), pass_of(t), rows[0]["ord"])
        if best:
            heard.append(best)
    if heard:
        text = "; ".join("pass %d: a %s group (order %s) %d m from the shot came %d m nearer the spot in 75 s%s" % (
            h[4], h[0].lower(), h[5], h[1], h[2], " (it joined a hunt meanwhile)" if h[3] else ", with no hunt and the player gone 420 m off a second after it") for h in heard)
        row("Hearing: a shot leads a steered group", text, all(h[2] >= 25 for h in heard))
    else:
        row("Hearing: a shot leads a steered group", "no steered group that takes leads was out to fire by", None)

    # ---- the layer's own counters against what was sampled
    last = data["status"][-1]["text"] if data["status"] else ""
    m = re.search(r"releases (\d+), take-backs (\d+), unsticks (\d+)", last)
    w = re.search(r"by waypoint (\d+)", last)
    undriven = max([f["undriven"] for f in data["f"]] or [0])
    agree = 0
    checked = 0
    for s in data["status"]:
        c = re.search(r"steer (\d+), engage (\d+), recover (\d+)", s["text"])
        if not c or s["st"] < 1:
            continue
        rows = [r for r in data["b"] if r["t"] == s["t"]]
        mine = tuple(len([r for r in rows if r["m"] == mode]) for mode in ("STEER", "ENGAGE", "RECOVER"))
        checked += 1
        if mine == tuple(int(x) for x in c.groups()):
            agree += 1
    if m:
        secs = data["f"][-1]["t"] - data["f"][0]["t"] if data["f"] else 0
        text = "Status says releases %s, take-backs %s, by waypoint %s; sampled: %d releases, %d take-backs, %d bodies not driven; its steer, engage and recover counts matched " \
               "the sampled bodies in %d of %d status lines. Unsticks %s in %d minutes" % (
                   m.group(1), m.group(2), w.group(1), len(releases), len(takebacks), int(undriven), agree, checked, m.group(3), secs / 60)
        row("Status | behaviour: counters", text, abs(int(m.group(1)) - len(releases)) <= 2 and abs(int(m.group(2)) - len(takebacks)) <= 2 and undriven == 0 and agree >= 0.9 * checked)
    else:
        row("Status | behaviour: counters", "no status line", False)

    # ---- script exceptions and errors
    ours = [e for e in data["exc"] if e[2]]
    theirs = {}
    for e in data["exc"]:
        if not e[2]:
            theirs[(e[0], e[1])] = theirs.get((e[0], e[1]), 0) + 1
    text = "%d from ReforgerZ's code" % len(ours)
    if theirs:
        text += "; from the game's own code: " + "; ".join("%d x %s in %s" % (n, k[0], k[1]) for k, n in sorted(theirs.items(), key=lambda x: -x[1]))
    errs = {}
    for e in data["errors"]:
        if "Virtual Machine Exception" not in e:
            errs[e[:70]] = errs.get(e[:70], 0) + 1
    if errs:
        text += "; other script errors: " + "; ".join("%d x %s" % (n, k) for k, n in sorted(errs.items(), key=lambda x: -x[1])[:4])
    row("Script exceptions", text, not ours)
    if not data["done"]:
        row("The test ran to its end", "no 'RZB done' line", False)

    print("Behaviour layer test: %d passes (%s)" % (passes, ", ".join("hour %d" % hours.get(p, -1) for p in range(passes))))
    for item, what, verdict in out:
        print("  [%s] %s: %s" % (verdict, item, what))
    for s in data["stage"]:
        print("    t=%d %s > %s: %s" % (s["t"], STAGES.get(s["from"], "-"), STAGES.get(s["to"], "end"), s["text"]))
    if fails:
        print("FAILED: " + ", ".join(fails))
        return 1
    return 0


def navmesh(addon, world):
    """Only the map's navmesh and a perception manager, so AI groups can walk (the road patrols check)."""
    path = os.path.join(addon, world)
    text = open(path, encoding="utf-8").read()
    text, n = re.subn(r'SCR_AIWorld RZSitesAIWorld : "[^"]+" \{\n coords [^\n]+\n\}', lambda m: AI_WORLD, text)
    assert n == 1, "no AI world in the world"
    open(path, "w", encoding="utf-8").write(text)


if __name__ == "__main__":
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "prepare":
        prepare(sys.argv[2], sys.argv[3])
    elif cmd == "navmesh":
        navmesh(sys.argv[2], sys.argv[3])
    elif cmd == "summary":
        sys.exit(summary(sys.argv[2]))
    else:
        sys.exit(__doc__)
