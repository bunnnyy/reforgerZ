#!/usr/bin/env python3
"""ReforgerZ mission rules check (docs/MISSION_RULES.md; how to run it: docs/WORKBENCH_SETUP.md,
"Mission rules check").

    python3 tools/mission_rules.py            # per-rule counts and every hit (file:line, rule #)
    python3 tools/mission_rules.py --strict   # exit 1 on any hit not in tools/mission_rules_allow.txt
    python3 tools/mission_rules.py --rule 23  # one rule's hits
    python3 tools/mission_rules.py --all      # list allowed hits too

A static check of the game scripts, the configs and our prefabs: what can be caught from source.
What can't (a thing really standing there when you arrive, a marker on a live body, a task done
without a reply) is the runtime test's (tools/runtime_test, "RZT rules" lines). Every check is a
heuristic: a hit is a place to look, and one that's right as it is goes in the allowlist with its
reason, one line each:

    <rule> <file> <text that the hit's line holds>  # why it's right

A reason that starts "pending:" names work under way that clears it: counted apart ("pending"), it
doesn't fail --strict, and it's listed every run so it isn't forgotten.

Rules checked here (their numbers are MISSION_RULES.md's):
    3  a door spawned: a prefab built on a base-game door (BuildingParts/Doors), or such a door named in code
    9  a person's marker on a stored spot: RZ_ObjPlace.Spot(...) given a giver's or a person's position
   10  a woman's name or word in a line the game shows (everyone met is a man: the stack has no female
       body); the allowlist names each woman who is only mentioned, never met (a dead wife, a voice)
   14  carrying done as a hold: a hold's words say carry, move, stack, load, haul or bring in, and its
       way isn't carried for real (no `.Carry(` before the next way)
   19  a hand-over done by walking up: a distance check to a person that completes a step or a job
   23  two actions or places in one objective line ("Find the ring, give it to the mother at ...")
   24  a kind of job the tracker never lists (so it can't be tracked when taken)
   26  a step of an opening that only sends you back
   27  an input action with no gamepad binding, or one the scripts listen for that isn't defined
    1  a building's name in a task's words in a file that never asks for that kind of building
No dependencies beyond Python 3. Reads tools/strings_inventory.py for the objective lines.
"""
import argparse
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import strings_inventory  # noqa: E402

SCRIPTS = os.path.join(ROOT, "scripts", "Game", "ReforgerZ")
ALLOW = os.path.join(ROOT, "tools", "mission_rules_allow.txt")

RULES = {
    1: "words name a building kind the file never asks for",
    2: "a work prompt whose look isn't the thing its words name",
    3: "a door spawned instead of a real one",
    9: "a person's marker on a stored spot",
    10: "a woman's name or word in a shown line",
    14: "carrying done as a hold",
    19: "a hand-over or completion by walking up to a person",
    23: "two actions or places in one objective line",
    24: "a kind of job the tracker never lists",
    26: "an opening step that only sends you back",
    27: "an input action without a gamepad binding",
}

# Task code: where missions, errands, duties, questlines, openings and events live
TASK_DIRS = ("Missions", "Guidance", "Director", "Puzzles", "Cure", "World/RZ_LongTable", "Factions/RZ_Checkpoints",
             "Factions/RZ_Supply", "Zombies/RZ_Tribe")


class Hit:
    def __init__(self, rule, path, line, text, note=""):
        self.rule = rule
        self.file = os.path.relpath(path, ROOT).replace(os.sep, "/") if os.path.isabs(path) else path
        self.line = line
        self.text = text.strip()
        self.note = note
        self.allowed = ""


def script_files():
    for dirpath, _, files in os.walk(SCRIPTS):
        if "Dialogue/Pack" in dirpath.replace(os.sep, "/"):
            continue
        for name in sorted(files):
            if name.endswith(".c"):
                yield os.path.join(dirpath, name)


def is_task_file(path):
    rel = os.path.relpath(path, SCRIPTS).replace(os.sep, "/")
    return any(rel.startswith(d) for d in TASK_DIRS)


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read().split("\n")


def strip_comment(line):
    # Good enough for this code base: no "//" inside the strings we look at in conditions
    out, q = [], False
    i = 0
    while i < len(line):
        c = line[i]
        if c == '"' and (i == 0 or line[i - 1] != "\\"):
            q = not q
        if not q and line.startswith("//", i):
            break
        out.append(c)
        i += 1
    return "".join(out)


# ---------------------------------------------------------------- 23: one objective per action and place
BACK_STARTS = ("tell ", "come back", "go back", "report", "find {giver}", "see {giver}", "hand ", "bring ",
               "carry it to", "carry them to", "it to {giver}", "them to {giver}")
JOINS = (", then ", ". Then ", " and bring ", ", and bring ")
# A second clause that goes somewhere else: "..., give it to the mother at {house}", "and bring it to
# the drop", "then answer at the lab". One place with two acts ("Find the body and take the ID") is fine.
TWO = re.compile(r"(?:,|;|\band\b|\bthen\b)\s+(?:then\s+)?(?:bring|carry|walk|take|deliver|give|sell|hand|"
                 r"answer|go|tell)\b[^,.;]*?\b(?:to|at|into)\b", re.I)


_INV = []


def objective_lits():
    if not _INV:
        kept, _ = strings_inventory.inventory(ROOT)
        _INV.extend(kept)
    return _INV


def without_back(step):
    """RZ_QuestTracker.WithoutBack: the last clause dropped when it only sends you back to the giver."""
    cut, start = -1, 0
    for j in JOINS:
        k = step.rfind(j)
        if k > cut:
            cut, start = k, k + len(j)
    if cut <= 0:
        return step
    tail = step[start:]
    if "{giver}" not in tail or not tail.lower().startswith(BACK_STARTS):
        return step
    return step[:cut].rstrip(".") + "."


def check_objectives(hits):
    for lit in objective_lits():
        if lit.channel != "objective":
            continue
        text = without_back(lit.text)
        # "(15 s there)" and "{place}" asides don't make two places
        bare = re.sub(r"\([^)]*\)", "", text)
        if TWO.search(bare):
            hits.append(Hit(23, lit.file, lit.line, lit.text))


# ---------------------------------------------------------------- 19: completing by walking up to someone
PERSON = re.compile(r"(giver|person|contact|host|asker|leader|named|npc|patient|boy|mother|runner|body|who)", re.I)
NEAR = re.compile(r"(RZ_Arrive\.AtSpot|DistXZ|vector\.Distance|DistanceSq|IsNear)\s*\(")
DONE = re.compile(r"\b(Resolve|Complete|Succeed|Succeeded|HandIn|Finish|StageDone|ObjStepDone|StepDone|EndStage|"
                  r"AdvanceStage|NextStep|Advance)\s*\(")


def check_walk_up(hits):
    for path in script_files():
        if not is_task_file(path):
            continue
        lines = read(path)
        for i, raw in enumerate(lines):
            line = strip_comment(raw)
            if "if" not in line or not NEAR.search(line):
                continue
            cond = line
            if not PERSON.search(cond):
                continue
            # The guarded statement: the next line, or the block that follows
            body = []
            j = i + 1
            depth = 0
            while j < len(lines) and j < i + 12:
                s = strip_comment(lines[j])
                body.append(s)
                depth += s.count("{") - s.count("}")
                if depth <= 0 and (s.strip().endswith(";") or s.strip() == "}"):
                    break
                j += 1
            if any(DONE.search(b) for b in body):
                hits.append(Hit(19, path, i + 1, raw))


# ---------------------------------------------------------------- 3: doors
DOOR_BASE = "BuildingParts/Doors/"


def check_doors(hits):
    pre = os.path.join(ROOT, "Prefabs", "ReforgerZ")
    for dirpath, _, files in os.walk(pre):
        for name in files:
            if not name.endswith(".et"):
                continue
            path = os.path.join(dirpath, name)
            for i, line in enumerate(read(path)):
                if DOOR_BASE in line:
                    hits.append(Hit(3, path, i + 1, line))
                    break
    for path in script_files():
        for i, line in enumerate(read(path)):
            if DOOR_BASE in line and not line.strip().startswith("//"):
                hits.append(Hit(3, path, i + 1, line))


# ---------------------------------------------------------------- 14: carrying as a hold
CARRY = re.compile(r"\b(carry|carried|move|stack|load|haul|bring in|lug|shift)\b", re.I)
HOLD = re.compile(r"(RZ_EWork\.\w+|\.Work\(|Task\()")


NEXT_WAY = re.compile(r"(\bWay\(|\bTask\(|^\s*\}\s*$)")


def carried_for_real(lines, i):
    """The way this hold belongs to is carried for real: a `.Carry(` follows it before the next way
    (Missions/Blocks/RZ_Carry.c: the things picked up and put down; the hold is only its fallback)."""
    for j in range(i, min(len(lines), i + 12)):
        s = strip_comment(lines[j])
        if j > i and NEXT_WAY.search(s):
            return False
        if ".Carry(" in s:
            return True
    return False


def check_carry(hits):
    for path in script_files():
        if not is_task_file(path):
            continue
        lines = read(path)
        for i, raw in enumerate(lines):
            line = strip_comment(raw)
            if not HOLD.search(line):
                continue
            lits = re.findall(r'"([^"]*)"', line)
            # The hold's own words: its label (the short ones), not the step's long line
            for lit in lits:
                if len(lit) <= 48 and CARRY.search(lit):
                    if not carried_for_real(lines, i):
                        hits.append(Hit(14, path, i + 1, raw))
                    break


# ---------------------------------------------------------------- 9: markers on people
SPOT_PERSON = re.compile(r"RZ_ObjPlace\.Spot\(([^;]*)", re.I)
PERSON_POS = re.compile(r"(m_vGiver|GiverPos|m_vPerson|PersonPos|m_vContact|ContactPos|m_vHost|HostPos|m_vLeader|"
                        r"LeaderPos|m_vWho|m_vAsker|HomeOf|m_vNpc)", re.I)


def check_markers(hits):
    for path in script_files():
        for i, raw in enumerate(read(path)):
            line = strip_comment(raw)
            m = SPOT_PERSON.search(line)
            if m and PERSON_POS.search(m.group(1)):
                hits.append(Hit(9, path, i + 1, raw))


# ---------------------------------------------------------------- 10: a role's sex apart from the body
# The base game's people are all men (SCR_EIdentityGender has no FEMALE): a woman's word on someone
# who stands in the world is false (docs/MISSION_RULES.md, rule 10)
SEXED = re.compile(r"^(an? |the )?[\w' ]{0,30}\b(mother|wife|widow|daughter|sister|grandmother|aunt|woman|girl|"
                   r"lady|her son)\b[\w' ]{0,20}$", re.I)


# Everyone met is a man (the author's call; no female body in the stack): a woman's first name or word
# in a line the game shows is a hit, unless the allowlist says who it is and that they're never met
# (a dead wife, a mother back home, a voice on the radio)
WOMEN_NAMES = ("Marta", "Jana", "Lucie", "Ruth", "Irena", "Elise", "Nadia", "Vera", "Sabine", "Ilse", "Dana", "Agata",
               "Ada", "Hana", "Olga", "Edith", "Agnes", "Zofia", "Greta", "Mila", "Mira", "Ines", "Mara", "Anna", "Eva",
               "Maria", "Marie", "Elena", "Helena", "Katarina", "Petra", "Monika", "Zuzana", "Ludmila", "Alena")
WOMEN = re.compile(r"\b(she|her|hers|herself|woman|women|mother|mothers|daughter|daughters|wife|wives|sister|"
                   r"sisters|girl|girls|widow|Mrs|Ms|gran|granny|grandmother|aunt|lady|ladies|niece|mum|mama|"
                   + "|".join(WOMEN_NAMES) + r")\b", re.I)
KEY = re.compile(r"[a-z_.:]+")


def women_in(text):
    text = re.sub(r"\{[a-z_]+\}", "", text)
    for m in WOMEN.finditer(text):
        word = m.group(0)
        if word.lower() == "ms" and word != "Ms":
            continue
        if word.lower() in [n.lower() for n in WOMEN_NAMES] and word not in WOMEN_NAMES:
            continue
        return True
    return False


def check_roles(hits):
    for path in script_files():
        if "/Dialogue/Pack/" in path.replace(os.sep, "/"):
            continue
        for i, raw in enumerate(read(path)):
            line = strip_comment(raw)
            for lit in re.findall(r'"((?:[^"\\]|\\.){3,})"', line):
                if KEY.fullmatch(lit):
                    continue
                if (is_task_file(path) and SEXED.match(lit)
                        and ("return" in line or "m_sWho" in line or "Add(" in line or "Spawn" in line)) or women_in(lit):
                    hits.append(Hit(10, path, i + 1, raw))
                    break
    for root, _, files in os.walk(os.path.join(ROOT, "dialogue")):
        for name in files:
            if not name.endswith(".rzd"):
                continue
            path = os.path.join(root, name)
            for i, raw in enumerate(read(path)):
                line = raw.strip()
                if not (line.startswith("say") or line.startswith(">") or line.startswith("topic")):
                    continue
                text = re.sub(r"->.*$", "", line)
                text = re.sub(r"@\w+", "", text)
                text = re.sub(r"^say\??\s*([^:]*::)?", "", text)
                if women_in(text):
                    hits.append(Hit(10, os.path.relpath(path, ROOT), i + 1, raw))


# ---------------------------------------------------------------- 24: tracked when taken
FAMILIES = {
    "missions and contracts": "GetMissions()",
    "errands and duties": "GetErrands()",
    "questlines and majors": "GetQuestlines()",
}


def check_tracker(hits):
    path = os.path.join(SCRIPTS, "UI", "RZ_Quests.c")
    text = "\n".join(read(path))
    m = re.search(r"void Jobs\(.*?\n\t\}\n", text, re.S)
    body = m.group(0) if m else ""
    for fam, call in FAMILIES.items():
        if call not in body:
            hits.append(Hit(24, path, 1, "Jobs() never lists " + fam))


# ---------------------------------------------------------------- 26: return-only steps in openings
RETURN = re.compile(r"\b(come back|go back|return to|back to)\b", re.I)


def check_openings(hits):
    for lit in objective_lits():
        if lit.channel == "objective" and "/Guidance/RZ_Opening" in lit.file and RETURN.search(lit.text):
            hits.append(Hit(26, lit.file, lit.line, lit.text))


# ---------------------------------------------------------------- 27: gamepad
def check_inputs(hits):
    conf = os.path.join(ROOT, "Configs", "System", "chimeraInputCommon.conf")
    lines = read(conf)
    defined = {}
    cur, start, pad = None, 0, False
    for i, line in enumerate(lines):
        m = re.match(r"\s*Action (\w+) \{", line)
        if m:
            if cur and cur.startswith("RZ_"):
                defined[cur] = (start, pad)
            cur, start, pad = m.group(1), i + 1, False
        elif cur and "gamepad" in line:
            pad = True
    if cur and cur.startswith("RZ_"):
        defined[cur] = (start, pad)
    for name, (ln, pad) in sorted(defined.items()):
        if not pad:
            hits.append(Hit(27, conf, ln, "Action %s has no gamepad binding" % name))
    for path in script_files():
        for i, raw in enumerate(read(path)):
            for name in re.findall(r'(?:AddActionListener|GetActionValue|GetActionTriggered|ActivateContext)\("(RZ_\w+)"', raw):
                if name not in defined and not name.endswith("Context"):
                    hits.append(Hit(27, path, i + 1, "%s is listened for but not defined" % name))


# ---------------------------------------------------------------- 1: building words without a place need
BUILDING = re.compile(r"\b(the chemist's|the pharmacy|the school|the church|the police (post|station)|the garage|"
                      r"the farmhouse|the barn|the hall|the petrol station|the clinic|the hospital|the shop|"
                      r"the doctor's house|the vet's|the radio hut|the barracks|the post office|the mill)\b", re.I)
NEEDS = re.compile(r"(m_sNeed|RZ_PlaceFind|RZ_PlaceNeeds|PlaceNeed|\.Need\(|KindOf\(|FindBuildings|BuildingAt\(|"
                   r"RZ_OpeningRoll|WordsTrue|FloorOf|Indoors\(|RZ_WorldFacts|NearestKind|BuildingOfKind|RZ_LandmarkFinder)")


def check_building_words(hits):
    texts = {}
    for lit in objective_lits():
        if lit.channel in ("objective", "brief") and BUILDING.search(lit.text):
            texts.setdefault(lit.file, []).append(lit)
    for rel, lits in texts.items():
        src = "\n".join(read(os.path.join(ROOT, rel)))
        if NEEDS.search(src):
            continue
        for lit in lits:
            hits.append(Hit(1, rel, lit.line, lit.text))


# ---------------------------------------------------------------- 2: the thing its words name
# A work prompt whose words name a thing (a pot, the wood, the line...) must have a look of that kind:
# its .Look(...) prefab (a constant in the same file, or the literal), never left to the work kind's
# default or to a box. Each kind: the words, and what its look's prefab path must contain.
THINGS = [
    ("kitchen", re.compile(r"\b(pot|stir|ladle|stove|peel|soup|supper|cook)\b", re.I),
     ("rz_work_stove", "kitchentable", "servetable", "rz_work_pot", "cookpot")),
    ("wood", re.compile(r"\b(split the wood|chop the wood|chop wood|woodpile)\b", re.I),
     ("firewood", "logchopping", "woodpile")),
    ("line", re.compile(r"\b(hang|washing line|the line)\b", re.I), ("dryingline", "clothesline", "basketlaundry")),
]
LOOK_ARG = re.compile(r"\.Look\(\s*(\w+|\"[^\"]*\")\s*\)")
CONST = re.compile(r"ResourceName\s+(\w+)\s*=\s*\"([^\"]+)\"")


def check_things(hits):
    for path in script_files():
        if not is_task_file(path):
            continue
        lines = read(path)
        consts = {}
        for raw in lines:
            m = CONST.search(raw)
            if m:
                consts[m.group(1)] = m.group(2).lower()
        for i, raw in enumerate(lines):
            line = strip_comment(raw)
            if not HOLD.search(line):
                continue
            # The task's own words: its label and its prompt (the short ones), not the step's long line
            label = " ".join(lit for lit in re.findall(r'"([^"]*)"', line) if len(lit) <= 48)
            if not label:
                continue
            for kind, words, ok in THINGS:
                if not words.search(label):
                    continue
                m = LOOK_ARG.search(line)
                look = ""
                if m:
                    arg = m.group(1)
                    look = arg.strip('"').lower() if arg.startswith('"') else consts.get(arg, "")
                if not any(o in look for o in ok):
                    hits.append(Hit(2, path, i + 1, raw, "the words name the %s, its look isn't one" % kind))
                break


# ---------------------------------------------------------------- the allowlist
def load_allow():
    rules = []
    if not os.path.exists(ALLOW):
        return rules
    for raw in read(ALLOW):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        body, _, why = line.partition("  #")
        parts = body.strip().split(" ", 2)
        if len(parts) < 3 or not why.strip():
            print("mission_rules: allowlist line without a rule, a file, a text and a reason: " + line)
            continue
        rules.append((int(parts[0]), parts[1], parts[2], why.strip()))
    return rules


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--strict", action="store_true", help="exit 1 on any hit not in the allowlist")
    ap.add_argument("--rule", type=int, help="only this rule")
    ap.add_argument("--all", action="store_true", help="also list allowed hits")
    args = ap.parse_args()

    hits = []
    for check in (check_things, check_objectives, check_walk_up, check_doors, check_carry, check_markers, check_roles,
                  check_tracker, check_openings, check_inputs, check_building_words):
        check(hits)

    allow = load_allow()
    used = set()
    for h in hits:
        for k, (rule, f, text, why) in enumerate(allow):
            if rule == h.rule and h.file.endswith(f) and text in h.text:
                h.allowed = why
                used.add(k)
                break

    if args.rule:
        hits = [h for h in hits if h.rule == args.rule]

    open_hits = [h for h in hits if not h.allowed]
    pending = [h for h in hits if h.allowed.lower().startswith("pending")]
    for h in sorted(hits, key=lambda x: (x.rule, x.file, x.line)):
        is_pending = h.allowed.lower().startswith("pending")
        if h.allowed and not is_pending and not args.all:
            continue
        mark = h.allowed
        print("%s:%d  rule %d  %s%s" % (h.file, h.line, h.rule, h.text[:150], ("   [" + mark + "]") if mark else ""))

    print()
    for rule in sorted(RULES):
        if args.rule and rule != args.rule:
            continue
        n = len([h for h in hits if h.rule == rule])
        p = len([h for h in pending if h.rule == rule])
        a = len([h for h in hits if h.rule == rule and h.allowed]) - p
        print("rule %2d  %-52s %3d hits, %3d allowed, %3d pending, %3d open" % (rule, RULES[rule], n, a, p, n - a - p))
    if not args.rule:
        for k, (rule, f, text, why) in enumerate(allow):
            if k not in used:
                print("mission_rules: allowlist line matches nothing (remove it): %d %s %s" % (rule, f, text))

    if open_hits:
        print("mission_rules: %d open, %d pending" % (len(open_hits), len(pending)))
        return 1 if args.strict else 0
    print("mission_rules: OK (%d pending)" % len(pending))
    return 0


if __name__ == "__main__":
    sys.exit(main())
