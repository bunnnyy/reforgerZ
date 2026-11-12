#!/usr/bin/env python3
"""ReforgerZ prefab manifest tool.

    python3 tools/prefab_manifest.py check    # compare the manifest with the scripts
    python3 tools/prefab_manifest.py sync     # copy attribute defaults from the scripts into the manifest
    python3 tools/prefab_manifest.py format   # rewrite the manifest in its compact layout
    python3 tools/prefab_manifest.py docs     # regenerate the manifest tables in docs/WORKBENCH_SETUP.md

The manifest (Configs/ReforgerZ/PrefabManifest.json) is the one source of truth for what the
user must create in Workbench. The Workbench plugin reads it; the doc tables are generated from it.
Run `check` and `docs` after changing a prefab attribute, component or action in the scripts.
No dependencies beyond Python 3.
"""
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SCRIPTS = os.path.join(ROOT, "scripts", "Game", "ReforgerZ")
MANIFEST = os.path.join(ROOT, "Configs", "ReforgerZ", "PrefabManifest.json")
DOC = os.path.join(ROOT, "docs", "WORKBENCH_SETUP.md")
PLUGIN = os.path.join(ROOT, "scripts", "WorkbenchGame", "ReforgerZ", "RZ_CreatePrefabsPlugin.c")

# Classes that exist but never go on a prefab themselves.
NOT_ON_PREFABS = {
    "RZ_SampleDeliveryActionBase",  # abstract base of the lab and dead-drop actions
    "RZ_GroupedAction",  # base of every action that can be a row of the object menu
}
# Vanilla component bases (outside our scripts) that make a class a component.
VANILLA_COMPONENT_BASES = {"SCR_GadgetComponent", "ScriptGameComponent", "SCR_UniversalInventoryStorageComponent"}
# The engine's own properties of a user action (not script attributes) a manifest action may set:
# Duration makes it a timed hold with vanilla's progress bar (RZ_WorkSiteAction).
NATIVE_ACTION_KEYS = {"Duration"}
# Consumable effects are set on an item's consumable component by hand (see the entry notes).
CONSUMABLE_EFFECTS = {"RZ_ConsumableAntidote", "RZ_ConsumableMed"}


# ------------------------------------------------------------------------------------------ code scan
def strip_comments_and_strings(line):
    line = re.sub(r'"(\\.|[^"\\])*"', '""', line)
    return line.split("//")[0]


def scan_code():
    """Returns {class: {"base", "file", "attrs": [{"type", "name", "attr"}]}} and {enum: [names]}."""
    classes = {}
    enums = {}
    for dirpath, _, files in os.walk(SCRIPTS):
        for f in sorted(files):
            if not f.endswith(".c"):
                continue
            path = os.path.join(dirpath, f)
            lines = open(path, encoding="utf-8").read().split("\n")
            depth = 0
            cls = None
            enum = None
            pending = None
            for line in lines:
                s = line.strip()
                if depth == 0:
                    m = re.match(r"(?:modded )?class (\w+)\s*(?::\s*(\w+))?", s)
                    if m:
                        cls = m.group(1)
                        enum = None
                        classes.setdefault(cls, {"base": m.group(2), "file": os.path.relpath(path, ROOT), "attrs": []})
                    m = re.match(r"enum (\w+)", s)
                    if m:
                        enum = m.group(1)
                        cls = None
                        enums[enum] = []
                elif depth == 1 and enum:
                    m = re.match(r"(\w+)\s*(=\s*[^,]*)?,?\s*(//.*)?$", s)
                    if m and s not in ("{", "}"):
                        enums[enum].append(m.group(1))
                elif depth == 1 and cls:
                    if s.startswith("[Attribute"):
                        pending = s
                    elif pending and s and not s.startswith("//"):
                        m = re.match(r"(?:protected |private )?(?:ref )?([\w<> ]+?)\s+(m_\w+)", s)
                        if m:
                            classes[cls]["attrs"].append({"type": m.group(1), "name": m.group(2), "attr": pending})
                        pending = None
                t = strip_comments_and_strings(line)
                depth += t.count("{") - t.count("}")
                if depth == 0 and "}" in t:
                    enum = None
    return classes, enums


def base_chain(classes, cls):
    chain = []
    while cls in classes and classes[cls]["base"]:
        cls = classes[cls]["base"]
        chain.append(cls)
    return chain


def attr_default(attr):
    m = re.match(r'\[Attribute\(\s*"((?:\\.|[^"\\])*)"', attr)
    return m.group(1) if m else None


def find_attr(classes, cls, name):
    while cls in classes:
        for a in classes[cls]["attrs"]:
            if a["name"] == name:
                return a
        cls = classes[cls]["base"]
    return None


# ------------------------------------------------------------------------------------------ manifest
def load():
    return json.load(open(MANIFEST, encoding="utf-8"))


def dump_compact(obj, indent=0, width=120):
    """JSON with small objects and arrays kept on one line."""
    one = json.dumps(obj, ensure_ascii=False)
    pad = " " * indent
    if len(one) + indent <= width or not isinstance(obj, (dict, list)) or not obj:
        return one
    inner = " " * (indent + 1)
    if isinstance(obj, dict):
        parts = ["%s%s: %s" % (inner, json.dumps(k, ensure_ascii=False), dump_compact(v, indent + 1, width).lstrip())
                 for k, v in obj.items()]
        return "{\n" + ",\n".join(parts) + "\n" + pad + "}"
    parts = [inner + dump_compact(v, indent + 1, width).lstrip() for v in obj]
    return "[\n" + ",\n".join(parts) + "\n" + pad + "]"


def save(m):
    open(MANIFEST, "w", encoding="utf-8").write(dump_compact(m) + "\n")


def all_parts(m):
    """(where, part, kind) for every component and action the manifest asks for."""
    gm = m["gameMode"]
    for p in gm["components"]:
        yield ("gameMode", p, "component")
    for e in m["prefabs"]:
        for p in e["components"]:
            yield (e["id"], p, "component")
        for p in e["actions"]:
            yield (e["id"], p, "action")
    for l in m["loadouts"]:
        for p in l["actions"]:
            yield ("loadout " + l["name"], p, "action")
    for a in m["additions"]:
        for p in a["add"]:
            kind = "component" if p["cls"].endswith("Component") else "action"
            yield ("addition: " + a["target"], p, kind)


def slot_type(classes, gm_field_types, slot):
    """Type of a slot path like m_ZombieSettings.m_sMoveWaypointPrefab, or None if it doesn't exist."""
    bits = slot.split(".")
    cls = "RZ_GameModeComponent"
    if bits[0].startswith("RZ_"):  # an action's field, e.g. RZ_FactionShopAction.m_sItemPrefab
        cls = bits[0]
        bits = bits[1:]
    for i, b in enumerate(bits):
        name = b.replace("[]", "")
        a = find_attr(classes, cls, name)
        if not a:
            return None
        t = a["type"]
        if i == len(bits) - 1:
            return t
        m = re.match(r"(?:array<)?(?:ref )?(\w+)>?", t)
        cls = m.group(1)
    return None


# ------------------------------------------------------------------------------------------ check
def prefab_fields(classes):
    """Every ResourceName attribute the user can set: slot paths from RZ_GameModeComponent,
    plus the shop and requisition actions' item fields."""
    wanted = set()

    def walk(cls, prefix, seen):
        for a in classes.get(cls, {"attrs": []})["attrs"]:
            t = a["type"]
            if "ResourceName" in t:
                wanted.add(prefix + a["name"])
                continue
            mm = re.match(r"(array<)?(?:ref )?(RZ_\w+)>?", t)
            if mm and mm.group(2) in classes and mm.group(2) not in seen:
                sub = prefix + a["name"] + ("[]" if mm.group(1) else "") + "."
                walk(mm.group(2), sub, seen | {mm.group(2)})

    walk("RZ_GameModeComponent", "", set())
    for cls in ("RZ_FactionShopAction", "RZ_RequisitionAction"):
        for a in classes[cls]["attrs"]:
            if "ResourceName" in a["type"]:
                wanted.add(cls + "." + a["name"])
    return wanted


def check(m, classes, enums):
    errors = []
    warns = []

    # Every ResourceName attribute reachable from the game mode component is in some slot.
    wanted = prefab_fields(classes)

    have = {}
    for e in m["prefabs"]:
        for s in e["slots"]:
            if s in have:
                errors.append("slot %s is in both %s and %s" % (s, have[s], e["id"]))
            have[s] = e["id"]
    for s in sorted(wanted - set(have)):
        errors.append("ResourceName field %s has no manifest entry" % s)
    for s in sorted(set(have) - wanted):
        errors.append("manifest slot %s (%s) is not a ResourceName field in the code" % (s, have[s]))
    for e in m["prefabs"]:
        for s in e["slots"]:
            t = slot_type(classes, None, s)
            if t and e.get("listSlot") != t.startswith("array"):
                errors.append("%s: listSlot is %s but %s is %s" % (e["id"], e.get("listSlot"), s, t))

    # Components and actions exist, have the right base, and their attribute names exist.
    used = set()
    for where, p, kind in all_parts(m):
        cls = p["cls"]
        used.add(cls)
        if not cls.startswith("RZ_"):
            continue
        if cls not in classes:
            errors.append("%s: class %s doesn't exist" % (where, cls))
            continue
        chain = base_chain(classes, cls)
        want = "ScriptedUserAction" if kind == "action" else "ScriptComponent"
        if kind == "component" and cls == "RZ_GameModeComponent":
            want = "SCR_BaseGameModeComponent"
        if want not in chain and not (kind == "component" and VANILLA_COMPONENT_BASES & set(chain)):
            errors.append("%s: %s is listed as a %s but derives from %s" % (where, cls, kind, " > ".join(chain)))
        for kv in p["values"]:
            k, _, v = kv.partition("=")
            if kind == "action" and k in NATIVE_ACTION_KEYS:
                continue
            a = find_attr(classes, cls, k)
            if not a:
                errors.append("%s: %s has no attribute %s" % (where, cls, k))
            elif v.startswith("@"):
                if not any(x["id"] == v[1:] for x in m["prefabs"]):
                    errors.append("%s: %s=%s refers to no manifest entry" % (where, k, v))

    # Every component and action class in the code is used somewhere.
    for cls, info in sorted(classes.items()):
        chain = base_chain(classes, cls)
        if ("ScriptComponent" in chain or "ScriptedUserAction" in chain or VANILLA_COMPONENT_BASES & set(chain)) and cls.startswith("RZ_") and not cls.endswith("Class"):
            if cls not in used and cls not in NOT_ON_PREFABS:
                errors.append("%s (%s) is on no prefab in the manifest" % (cls, info["file"]))

    # @refs point to an earlier created prefab (the plugin works in file order).
    ids = [e["id"] for e in m["prefabs"]]
    if len(ids) != len(set(ids)):
        errors.append("duplicate ids")
    paths = [e["path"] for e in m["prefabs"] if e["path"]]
    if len(paths) != len(set(paths)):
        errors.append("duplicate paths")
    for i, e in enumerate(m["prefabs"]):
        if e["kind"] == "create" and not e["path"]:
            errors.append("%s: create without a path" % e["id"])
        if e["kind"] != "create" and e["path"]:
            errors.append("%s: %s with a path" % (e["id"], e["kind"]))
        for p in e["components"] + e["actions"]:
            for kv in p["values"]:
                v = kv.partition("=")[2]
                if v.startswith("@"):
                    j = ids.index(v[1:]) if v[1:] in ids else -1
                    if j >= i:
                        errors.append("%s refers to %s, which comes later" % (e["id"], v))

    # Settings objects: every one on the game mode component, with current defaults.
    gm_settings = []
    for a in classes["RZ_GameModeComponent"]["attrs"]:
        t = a["type"]
        if re.match(r"RZ_\w+Settings$", t):
            gm_settings.append((a["name"], t))
    listed = {(s["field"], s["cls"]) for s in m["settings"]}
    for fs in gm_settings:
        if fs not in listed:
            errors.append("settings object %s (%s) is missing from the manifest" % fs)
    for s in m["settings"]:
        if (s["field"], s["cls"]) not in gm_settings:
            errors.append("manifest settings %s isn't on RZ_GameModeComponent" % s["field"])
        for k, v in code_defaults(classes, s["cls"]).items():
            if s["defaults"].get(k) != v:
                warns.append("%s.%s default is %r in the code, %r in the manifest (run sync)" % (s["field"], k, v, s["defaults"].get(k)))
        for k in s["defaults"]:
            if not find_attr(classes, s["cls"], k):
                warns.append("%s.%s no longer exists (run sync)" % (s["field"], k))
    fields = {r.get("field") for r in m["settingsTable"]}
    for s in m["settings"]:
        if s["field"] not in fields:
            errors.append("settings %s has no row in settingsTable" % s["field"])

    # Loadouts match the built-in backgrounds.
    names = re.findall(r'Make(?:Locked)?\("\w+", "([^"]+)"', open(os.path.join(SCRIPTS, "Backgrounds", "RZ_Background.c"), encoding="utf-8").read())
    if sorted(names) != sorted(l["name"] for l in m["loadouts"]):
        errors.append("loadouts %s don't match the built-in backgrounds %s" % ([l["name"] for l in m["loadouts"]], names))

    errors.extend(check_plugin_keys(m))

    for w in warns:
        print("warning: " + w)
    for e in errors:
        print("error: " + e)
    print("%d errors, %d warnings. %d prefab fields in the code, %d manifest entries." % (len(errors), len(warns), len(wanted), len(m["prefabs"])))
    return not errors


def plugin_members():
    """{class: [member names]} of the plugin's RZ_Mf* data classes and RZ_Manifest."""
    src = open(PLUGIN, encoding="utf-8").read()
    out = {}
    for m in re.finditer(r"class (RZ_Mf\w+|RZ_Manifest)\s*\{(.*?)\n\}", src, re.S):
        names = re.findall(r"^\s*(?:ref )?[\w<> ]+?\s+(\w+)(?:\s*=.*)?;", m.group(2), re.M)
        out[m.group(1)] = names
    return out


def check_plugin_keys(m):
    """The JSON keys the plugin reads must match its data classes' member names."""
    members = plugin_members()
    errors = []

    def same(where, obj, cls, ignore=()):
        want = set(members.get(cls, []))
        have = set(obj.keys()) - set(ignore)
        for k in sorted(want - have):
            errors.append("%s: no key %r (member of %s in the plugin)" % (where, k, cls))
        for k in sorted(have - want):
            errors.append("%s: key %r isn't read by the plugin (%s has no such member)" % (where, k, cls))

    same("manifest", m, "RZ_Manifest", ignore=("about", "settingsTable", "settings", "topLevelDefaults"))
    same("gameMode", m["gameMode"], "RZ_MfGameMode")
    for e in m["prefabs"]:
        same("prefab " + e["id"], e, "RZ_MfPrefab")
    for where, p, kind in all_parts(m):
        same(where, p, "RZ_MfPart")
    for l in m["loadouts"]:
        same("loadout " + l["name"], l, "RZ_MfLoadout")
    for a in m["additions"]:
        same("addition", a, "RZ_MfAddition")
    same("factions", m["factions"], "RZ_MfFactions")
    for f in m["factions"]["main"]:
        same("faction " + f["key"], f, "RZ_MfBaseFaction")
    same("outlaw", m["factions"]["outlaw"], "RZ_MfOutlaw")
    for w in m["world"]:
        same("world step", w, "RZ_MfWorldStep")
    return errors


def code_defaults(classes, cls):
    d = {}
    for a in classes[cls]["attrs"]:
        if "ResourceName" in a["type"] or a["type"].startswith("array"):
            continue
        v = attr_default(a["attr"])
        if v is not None:
            d[a["name"]] = v
    return d


def sync(m, classes):
    for s in m["settings"]:
        s["defaults"] = code_defaults(classes, s["cls"])
    top = {}
    for a in classes["RZ_GameModeComponent"]["attrs"]:
        t = a["type"]
        if "ResourceName" in t or t.startswith("array") or t.startswith("RZ_"):
            continue
        v = attr_default(a["attr"])
        if v is not None:
            top[a["name"]] = v
    m["topLevelDefaults"] = top


# ------------------------------------------------------------------------------------------ docs
GENERIC = None  # filled from the manifest: the generic puzzle object's parts
CLUE = None


def part_key(p):
    return (p["cls"], tuple(p["values"]))


def nice_key(k):
    """m_eRequiredTier -> Required tier."""
    k = re.sub(r"^m_[a-z]", "", k)
    words = re.findall(r"[A-Z][a-z0-9]*|[a-z0-9]+", k)
    text = " ".join(words).capitalize()
    return text


def render_value(m, classes, enums, cls, k, v):
    a = find_attr(classes, cls, k)
    if a and a["type"] in enums and v.isdigit():
        names = enums[a["type"]]
        i = int(v)
        if i < len(names):
            return names[i]
    if v.startswith("@"):
        for e in m["prefabs"]:
            if e["id"] == v[1:]:
                return "`%s`" % e["path"].split("/")[-1]
    if a and a["type"] == "bool":
        return {"1": "on", "0": "off"}.get(v, v)
    return v


def render_one(m, classes, enums, p):
    if not p["values"]:
        return "`%s`" % p["cls"]
    kv = ", ".join("%s %s" % (nice_key(x.split("=")[0]), render_value(m, classes, enums, p["cls"], *x.split("=", 1))) for x in p["values"])
    return "`%s` (%s)" % (p["cls"], kv)


def render_parts(m, classes, enums, parts):
    """'RZ_A, 4 × RZ_B (Value 0-3)' with runs of the same class folded."""
    out = []
    i = 0
    while i < len(parts):
        p = parts[i]
        j = i
        while j + 1 < len(parts) and parts[j + 1]["cls"] == p["cls"]:
            j += 1
        run = parts[i:j + 1]
        keys = {kv.split("=")[0] for q in run for kv in q["values"]}
        if len(run) == 1:
            out.append(render_one(m, classes, enums, p))
        elif len(keys) == 1 and all(len(q["values"]) == 1 for q in run):
            k = next(iter(keys))
            vals = [render_value(m, classes, enums, p["cls"], *q["values"][0].split("=", 1)) for q in run]
            if all(v.isdigit() for v in vals) and [int(v) for v in vals] == list(range(int(vals[0]), int(vals[0]) + len(vals))):
                rng = "%s-%s" % (vals[0], vals[-1])
            else:
                rng = ", ".join(vals)
            out.append("%d × `%s` (%s %s)" % (len(run), p["cls"], nice_key(k), rng))
        else:
            out.append("; ".join(render_one(m, classes, enums, q) for q in run))
        i = j + 1
    return ", ".join(out)


def render_add(m, classes, enums, e):
    comps = [part_key(p) for p in e["components"]]
    acts = [part_key(p) for p in e["actions"]]
    if comps == GENERIC[0] and acts == GENERIC[1]:
        return "the generic set"
    if comps == CLUE[0] and acts == CLUE[1]:
        return "the clue set"
    bits = []
    if e["components"]:
        bits.append(render_parts(m, classes, enums, e["components"]))
    if e["actions"]:
        bits.append(render_parts(m, classes, enums, e["actions"]))
    return "; ".join(bits) if bits else "—"


def need(e):
    if e["minimum"]:
        return "**minimum**"
    if e["required"]:
        return "required"
    return "optional"


def prefab_name(e):
    if e["path"]:
        return "`%s`" % e["path"].split("/")[-1]
    return "(existing prefab)"


def gen_tables(m, classes, enums):
    global GENERIC, CLUE
    po = next(e for e in m["prefabs"] if e["id"] == "PuzzleObject")
    note = next(e for e in m["prefabs"] if e["id"] == "Note")
    GENERIC = ([part_key(p) for p in po["components"]], [part_key(p) for p in po["actions"]])
    CLUE = ([part_key(p) for p in note["components"]], [part_key(p) for p in note["actions"]])
    blocks = {}

    # Minimum playable subset.
    rows = ["| Prefab | Suggested base | Add | Slot | Gets you |", "|---|---|---|---|---|"]
    gm = m["gameMode"]
    rows.append("| `%s` | %s | %s | (placed in the world) | the mode |" % (gm["path"].split("/")[-1], gm["baseHint"], render_parts(m, classes, enums, gm["components"])))
    for e in m["prefabs"]:
        if e["minimum"]:
            rows.append("| %s | %s | %s | %s | %s |" % (prefab_name(e), e["baseHint"], render_add(m, classes, enums, e), e["slotLabel"], e["use"]))
    blocks["minimum"] = "\n".join(rows)

    # Full table, by system.
    out = []
    systems = []
    for e in m["prefabs"]:
        if e["system"] not in systems:
            systems.append(e["system"])
    for s in systems:
        out.append("#### " + s)
        out.append("")
        out.append("| Prefab | Suggested base | Add | Slot | Need | Used for |")
        out.append("|---|---|---|---|---|---|")
        for e in m["prefabs"]:
            if e["system"] != s:
                continue
            use = e["use"]
            if e.get("notes"):
                use += ". " + e["notes"].rstrip(".")
            out.append("| %s | %s | %s | %s | %s | %s |" % (prefab_name(e), e["baseHint"], render_add(m, classes, enums, e), e["slotLabel"], need(e), use))
        out.append("")
    blocks["prefabs"] = "\n".join(out).rstrip()

    # Additions to existing prefabs.
    rows = ["| Prefab | Add | Why | Need |", "|---|---|---|---|"]
    for a in m["additions"]:
        rows.append("| %s | %s | %s | %s |" % (a["target"], render_parts(m, classes, enums, a["add"]), a["why"], "required" if a["required"] else "optional"))
    blocks["additions"] = "\n".join(rows)

    # Loadouts.
    rows = ["| Loadout name | Prefab | Unlock | Suggested gear |", "|---|---|---|---|"]
    for l in m["loadouts"]:
        rows.append("| `%s` | `%s` | %s | %s |" % (l["name"], l["path"].split("/")[-1], "Legacy" if l["unlockable"] else "—", l["gear"]))
    blocks["loadouts"] = "\n".join(rows)

    # World.
    rows = ["| Step | Plugin | Minimum |", "|---|---|---|"]
    for w in m["world"]:
        auto = {"yes": "does it", "no": "manual", "check": "checks it"}[w["plugin"]]
        rows.append("| %s | %s | %s |" % (w["step"], auto, "yes" if w["minimum"] else "—"))
    blocks["world"] = "\n".join(rows)

    # Factions and relations.
    f = m["factions"]
    keys = [x["key"] for x in f["main"]]
    rel = {}
    for pair in f["friendly"]:
        a, b = pair.split()
        rel[(a, b)] = rel[(b, a)] = "friendly"
    for pair in f["hostile"]:
        a, b = pair.split()
        rel[(a, b)] = rel[(b, a)] = "hostile"
    rows = ["| | " + " | ".join(keys) + " |", "|---" * (len(keys) + 1) + "|"]
    for a in keys:
        cells = []
        for b in keys:
            if a == b:
                cells.append("friendly" if a == f["playerKey"] else "")
            else:
                cells.append(rel.get((a, b), ""))
        rows.append("| %s | %s |" % (a, " | ".join(cells)))
    blocks["relations"] = "\n".join(rows)
    rows = ["| Key | In game | Used for |", "|---|---|---|"]
    for x in f["main"]:
        rows.append("| `%s` | %s | %s |" % (x["key"], x["name"], x["use"]))
    rows.append("| (the zombie mod's) | The infected | %s |" % f["zombieFaction"])
    blocks["factions"] = "\n".join(rows)

    # Settings table.
    rows = ["| Category | Object | Field | Covers |", "|---|---|---|---|"]
    for r in m["settingsTable"]:
        rows.append("| %s | %s | %s | %s |" % (r["category"], r["object"], ("`%s`" % r["field"]) if r.get("field") else "", r["covers"]))
    blocks["settings"] = "\n".join(rows)

    # Counts.
    create = [e for e in m["prefabs"] if e["kind"] == "create"]
    counts = {
        "entries": len(m["prefabs"]),
        "create": len(create),
        "external": sum(1 for e in m["prefabs"] if e["kind"] == "external"),
        "manual": sum(1 for e in m["prefabs"] if e["kind"] == "manual"),
        "minimum": sum(1 for e in m["prefabs"] if e["minimum"]) + 1,
        "required": sum(1 for e in m["prefabs"] if e["required"]) + 1,
        "settings": len(m["settings"]),
        "loadouts": len(m["loadouts"]),
        "additions": len(m["additions"]),
    }
    fields = prefab_fields(classes)
    counts["fields"] = len(fields)
    counts["top"] = sum(1 for f in fields if "." not in f)
    counts["inSettings"] = sum(1 for f in fields if "." in f and "[]" not in f and not f.startswith("RZ_"))
    counts["inLists"] = counts["fields"] - counts["top"] - counts["inSettings"]
    blocks["counts"] = ("The code has **%(fields)d prefab fields**: %(top)d on `RZ_GameModeComponent`, %(inSettings)d in its "
                        "settings objects, and %(inLists)d in list entries and actions (shop item, requisition item, recipe "
                        "output, the loot table lists). The manifest covers them in **%(entries)d entries**: %(create)d "
                        "prefabs the plugin creates, %(external)d slots filled with prefabs you already have (zombie mods, "
                        "vanilla cars, loot) and %(manual)d list entries set by hand; plus the game mode prefab, "
                        "%(loadouts)d loadout characters, %(additions)d additions to existing prefabs and %(settings)d "
                        "settings objects. %(minimum)d of them, counting the game mode, make up the minimum playable subset." % counts)
    return blocks


def docs(m, classes, enums):
    text = open(DOC, encoding="utf-8").read()
    blocks = gen_tables(m, classes, enums)
    for name, body in blocks.items():
        pat = re.compile(r"(<!-- manifest:%s -->\n)(?:.*?\n)?(<!-- /manifest:%s -->)" % (name, name), re.S)
        if not pat.search(text):
            print("warning: no <!-- manifest:%s --> block in the doc" % name)
            continue
        text = pat.sub(lambda mm: mm.group(1) + body + "\n" + mm.group(2), text)
    open(DOC, "w", encoding="utf-8").write(text)
    print("Wrote %d blocks into %s" % (len(blocks), os.path.relpath(DOC, ROOT)))


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "check"
    classes, enums = scan_code()
    m = load()
    if cmd == "check":
        sys.exit(0 if check(m, classes, enums) else 1)
    elif cmd == "sync":
        sync(m, classes)
        save(m)
    elif cmd == "format":
        save(m)
    elif cmd == "docs":
        docs(m, classes, enums)
    else:
        print(__doc__)
        sys.exit(2)


if __name__ == "__main__":
    main()
