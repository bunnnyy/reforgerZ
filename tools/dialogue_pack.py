#!/usr/bin/env python3
"""ReforgerZ dialogue packer and lint (docs/DIALOGUE.md, "The text format" and "Tools").

    python3 tools/dialogue_pack.py            # lint every dialogue/**/*.rzd and write the packs
    python3 tools/dialogue_pack.py --check    # lint, and fail if the packs or the lock are out of date
    python3 tools/dialogue_pack.py --lock     # add newly shipped ids to dialogue/shipped.lock
    python3 tools/dialogue_pack.py --list     # one line per conversation: id, speakers, topic

Conversations are written in .rzd files under dialogue/<area>/. This tool parses them, checks them
(structure, checks, effects, content: docs/DIALOGUE.md, "The lint") and writes them as generated
Enforce Script into scripts/Game/ReforgerZ/Dialogue/Pack/ (one file per area, functions of at most
150 Add calls), which the server loads at start. The packs are generated: never edit them by hand.
The player text in .rzd files is also checked by tools/voice_lint.py, like every other line.

dialogue/shipped.lock lists every conversation and choice id that has shipped. Removing one is an
error (memory is keyed by these ids); mark it retired instead ("retired <id>" in the lock).
No dependencies beyond Python 3.
"""
import argparse
import os
import re
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SRC = os.path.join(ROOT, "dialogue")
PACK_DIR = os.path.join(ROOT, "scripts", "Game", "ReforgerZ", "Dialogue", "Pack")
LOCK = os.path.join(SRC, "shipped.lock")
MAX_ADDS = 150

FACTIONS = {"haven": 0, "bastion": 1, "redhand": 2, "halcyon": 3, "none": -1}
FACTION_NAMES = {"haven": "Haven", "bastion": "Bastion", "redhand": "Red Hand", "halcyon": "Halcyon",
                 "table": "Long Table", "plain": "-"}
VOICES = {"haven", "bastion", "redhand", "halcyon", "table", "plain"}
PRIOS = {"quest": 0, "ask": 1, "world": 2, "reaction": 3, "flavour": 4}
TARGETS_SPECIAL = {"end", "hub", "back"}

# Speaker selectors (docs/DIALOGUE.md, "Who talks"). A `for` entry must match one of these.
SPEAKER_RE = re.compile(r"^(leader|leader:(haven|bastion|redhand|halcyon)|stronghold:(haven|bastion|redhand|halcyon)|"
                        r"seat:(haven/council_head|bastion/outpost_lead|redhand/crew_boss|halcyon/field_lead)|"
                        r"wanderer|wanderer:(trader|doctor|scavenger|deserter|preacher|courier|kid|mechanic)|"
                        r"occupant|named|named:(haven|bastion|redhand|halcyon)(/[a-z_]+)?|"
                        r"post|post:(bastion|redhand)|table|table:(host|mother|son|boy|road)|"
                        r"stranger|stranger:[a-z_]+|radio|radio:open|radio:secure|radio:(bastion|redhand|halcyon)|"
                        r"quest|quest:[a-z_]+|mission)$")

TIERS = ["kill_on_sight", "hunted", "watched", "uneasy", "stranger", "tolerated", "wildcard", "known", "respected",
         "trusted", "champion"]
BACKGROUNDS = {"unemployed", "drifter", "soldier", "police", "nurse", "doctor", "labtech", "hunter", "mechanic",
               "radioham", "elder", "excon", "raider"}
TRAITS = {"greedy", "paranoid", "merciful", "ruthless", "idealist", "pragmatist", "veteran", "talker"}

# Condition keys: name -> (takes an argument, takes an operator). The engine (RZ_DialogueCond) and
# the simulator know the same list. QUIET keys belong to designs not built yet: they read false
# (route:none reads true) until their owner is built.
COND_KEYS = {
    "bg": (True, False), "tier": (True, True), "fame": (True, True), "infamy": (True, True),
    "rep": (True, True), "hunted": (True, False), "exiled": (False, False), "kin": (False, False),
    "flag": (True, True), "cflag": (True, True), "nmem": (True, False), "regard": (False, True),
    "face": (True, False), "seen": (True, False), "claim": (True, False), "knows": (True, False),
    "item": (True, False), "armed": (False, False), "bite_shown": (False, False), "stage": (True, False),
    "night": (False, False), "dusk": (False, False), "day": (False, True), "hour": (False, True),
    "outbreak": (True, False), "cure": (False, True), "epilogue": (False, False), "trait": (True, False),
    "loyalty": (True, False), "attitude": (False, True), "house": (True, False), "town": (True, True),
    "others": (False, True), "mood": (True, False), "met": (False, False), "helped": (False, False),
    "saved": (False, False), "crossed": (False, False), "crew": (False, False), "role": (True, False),
    "need": (True, False), "wbitten": (False, False), "kidlost": (False, False), "table": (True, False),
    "picture": (True, False), "stronghold": (False, False),
    # Scenes: this conversation's scene is decided (any value, or decided:<value>); you swayed a tally
    "decided": (False, False), "swayed": (True, False),
    # The finale (docs/FINALE.md): finale:soon|pitch|open|locked|voice
    "finale": (True, False),
    # A major's sitting is open (sitting:every_name.first); an option is on its table (tabled:...pot)
    "sitting": (True, False), "tabled": (True, False),
    # Lies you're told, promises and callbacks
    "heard_lie": (True, False), "promise": (True, False), "kept": (True, False), "broke": (True, False),
    "due": (True, False),
    # Kit and the look (relative to the speaker's faction)
    "kit": (True, False), "passing": (False, False), "caught": (False, False),
    # A cover detail you could give (cover:name|dead|crew|notebook|paper); a claim line already spoken here this life
    "cover": (True, False), "claimed": (False, False),
    # The finale's lock and the season record
    "route": (True, False), "lockdays": (False, True), "title": (True, True), "seasoned": (True, False),
    "owed": (True, False), "setword": (True, False), "radio": (False, False),
    # The speaker's faction and another: a truce or a feud now (faction life)
    "truce": (True, False), "feud": (True, False),
    # The season's story (RZ_DialogueWorld): who holds the cure, a major's ending and who decided it,
    # a voter's lean, a questline's own flag
    "cure_held": (True, False), "ending": (True, True), "was": (True, False), "lean": (True, False),
    "q": (True, True),
    # The infected near the speaker; you and your bite; your group; the radio; a story's hold; a wrong made right
    "tribe": (False, False), "alpha": (False, False), "declared": (False, False), "symptoms": (False, False),
    "group": (False, False), "secure": (False, False), "power": (False, False), "distress": (False, False),
    "busy": (False, False), "made_right": (False, False),
    # A lie believed in a scene (scene_lied:<scene>); a scene's own world flag (set:<key>=N -> talk:<key>)
    "scene_lied": (True, False), "talk": (True, True),
    # This topic's bindings were picked (bound), or that one (bound:<name>)
    "bound": (False, False),
    # This life is under an hour of play
    "fresh": (False, False),
}
QUIET_KEYS = {"stranger"}
CURE_HELD = {"any", "haven", "bastion", "redhand", "halcyon", "everyone", "indep", "nobody", "slow", "none"}
MAJORS = {"every_name", "orders", "book", "proof"}
FINALE_WORDS = {"soon", "pitch", "open", "locked", "voice"}
COMMIT_ROUTES = {"haven", "bastion", "redhand", "halcyon", "indep"}
KIT_ARGS = {"own", "in_kit", "full", "other", "any", "issued", "signed_other", "dead_mans", "off_turned"}
COVER_ARGS = {"name", "dead", "crew", "notebook", "paper"}
ROUTE_ARGS = {"none", "any", "in", "out", "independent", "unsided", "won", "lost", "haven", "bastion", "redhand",
              "halcyon", "indep"}
# Keys that read the world or the player's place in it: an ask needs one (docs/DIALOGUE.md, "Asks").
WORLD_KEYS = {"house", "town", "need", "wbitten", "kidlost", "bg", "tier", "rep", "exiled", "bite_shown", "hunted",
              "outbreak", "cure", "flag", "stage", "infamy", "fame", "kin", "tribe", "q", "cure_held", "distress",
              "declared", "symptoms"}
EFFECTS = {"mark", "cset", "nset", "regard", "favor", "take", "claim", "learn", "rumour", "call", "offer", "later",
           "sway", "decide", "commit", "promise", "keep", "wire", "cover", "set", "qset", "begin", "note", "approach", "test", "watch", "unsway"}
SCENE_EFFECTS = {"sway", "decide", "set", "qset", "begin"}
# Effects that belong to another system: a stage's outcome or its physical half is the stage's own
# built choice (call:slot:N at its person); an errand is dealt by errands; Infamy and grudges come only
# from tagged checks.
WORLD_EFFECTS = {"resolve", "start", "event", "reward", "pass", "catch", "errand", "reveal", "hostile", "flee",
                 "return"}
# What a claim in someone else's kit may never lead to (docs/DIALOGUE.md, "Claims in someone else's kit")
CLAIM_BARRED = {"favor", "offer", "learn", "errand", "reveal", "begin"}
# What a lie found out later may still change (things still running), and what a lie's truth may read
FOUND_EFFECTS = {"nset", "cset", "mark", "unsway"}
KNOWABLE_KEYS = {"bg", "tier", "fame", "infamy", "rep", "hunted", "exiled", "kin", "knows", "seen", "claim",
                 "heard_lie", "item", "armed", "bite_shown", "stage", "declared", "symptoms", "kit", "passing",
                 "caught", "crew", "face", "title", "seasoned", "route", "cflag", "promise", "kept", "broke", "due",
                 "cover", "group", "secure"}
NUMBER_RE = re.compile(r"[0-9]|\{cure_percent\}|\b(one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|"
                       r"twenty|thirty|fifty|hundred|percent|half|minutes?)\b", re.I)
GENERIC_SPEAKERS = {"occupant", "named", "post", "wanderer", "leader", "stranger", "table", "quest", "mission"}
FORBIDDEN_EFFECTS = {"fame", "house", "infamy", "follow", "escort", "companion"}
REGARD_WORDS = {"thanks", "kept", "broke", "threatened", "slander", "vouched_liar", "lied"}
CALLS = re.compile(r"^(slot:[0-9]|hand_in:[0-9]+|toll|post_wave|standing|season|look|distress|net_word|net_lost|declare_bite|caught_bite)$")
CHECKS = {"persuade", "threaten", "lie", "look"}
LATER_CHECKS = {"vouch", "commit"}
SCENE_SCOPES = {"speaker", "season"}

# Tokens a .rzd line may use beyond the world tokens (RZ_DialogueText.Fill, RZ_RepVoice.FillFor).
DIALOGUE_TOKENS = {"npc", "npc_short", "player", "house", "resident", "word", "rep", "best", "worst", "kills",
                   "jobs", "picture", "name", "crew", "place", "here", "faction", "role", "toll", "seat", "boss",
                   "kit_owner", "dead_name", "cover", "pack", "pack_leader", "distress"}
# Tokens with an argument a conversation may use: a stranger's own words, the season's title with a faction
DIALOGUE_KINDS = {"their", "title", "wire", "cover", "decider", "lean", "tier", "rep", "word", "claim", "bind"}
# Tokens that would put a player on air (a radio conversation never uses them)
PERSONAL_TOKENS = {"player", "name", "crew", "word", "rep", "best", "worst", "kills", "jobs", "picture", "title", "tier",
                   "cover", "kit_owner", "dead_name"}
MARK_RE = re.compile(r"\s+~(lie|truth):([a-z0-9_.@]+)\s*$")
WORLD_TOKENS = {"day", "day_text", "days_since_command", "days_since_command_words", "cure_state", "cure_percent",
                "stage", "stage_tone", "time_of_day", "lab_town", "stolen_doses"}
WORLD_KINDS = {"town", "owner", "flag", "canon", "canon_town", "leader", "leader_short", "leader_place"}
TOKEN_RE = re.compile(r"\{([a-z_]+)(?::([^{}\s]+))?\}")

# Content rules.
TABLE_BANNED = re.compile(r"\b(meat|pork|flesh|stew|kitchen|cellar|larder|butcher\w*|bones?|cook(ed|ing)? (him|her|them)|"
                          r"long pig|eat (him|her|them|people)|what it was)\b", re.I)
TABLE_NAMES = re.compile(r"\b(Halas|Tomas|Moravec)\b")
LONG_TABLE_HINT = re.compile(r"\b(meat|pork|long pig|don'?t ask what|place set|set a place|extra place|an extra plate|"
                             r"mystery stew|who'?s for dinner)\b", re.I)
INFECTED_WORDS = re.compile(r"\b(alphas?|tribes?|tribal|crawlers?)\b", re.I)
PRONOUN_AFTER_NPC = re.compile(r"\{npc(_short)?\}[^.?!]*\b(he|she|him|her|his|hers|himself|herself)\b", re.I)

SAY_LIMIT = 200
LABEL_LIMIT = 60
TOPIC_LIMIT = 40
NODE_CHOICES = 6
FAVOR_PER_CONV = 10


class Problem(Exception):
    pass


class Choice:
    def __init__(self):
        self.id = ""
        self.label = ""
        self.targets = []
        self.check = ""
        self.cost = ""
        self.cond = ""
        self.do = ""
        self.short = ""
        self.els = ""
        self.line = 0
        self.truth = ""
        self.callout = ""
        self.dry = False
        self.tell = False
        self.found = ""


class Node:
    def __init__(self, nid, line):
        self.id = nid
        self.line = line
        self.items = []     # ("say", cond, text) | ("jump", cond, target) | ("do", effects)
        self.choices = []
        self.public = False
        self.confirm = False
        self.marks = {}     # item index -> ("lie" | "truth", key)


class Conv:
    def __init__(self, cid, path, line):
        self.id = cid
        self.path = path
        self.line = line
        self.speakers = []
        self.when = ""
        self.kind = ""      # topic | greet
        self.label = ""
        self.prio = "flavour"
        self.once = False
        self.daily = False
        self.always = False
        self.voice = "plain"
        self.nodes = []
        self.scene = ""
        self.scope = ""
        self.who = ""
        self.opens = 0
        self.binds = []     # (name, picker)
        self.uses = []      # conversation ids whose nodes are copied in at pack time

    def node(self, nid):
        for n in self.nodes:
            if n.id == nid:
                return n
        return None


def rel(path):
    return os.path.relpath(path, ROOT).replace(os.sep, "/")


def area_of(path):
    parts = rel(path).split("/")
    return parts[1] if len(parts) > 2 else "base"


TOKEN_SPLIT = re.compile(r'"[^"]*"|\S+')
CHOICE_KEYS = {"check", "cost", "if", "do", "short", "else", "label", "truth", "callout", "found", "tell"}


def parse_choice(text, lineno):
    c = Choice()
    c.line = lineno
    m = re.match(r"^>\s*@([a-z0-9_]+)\s+(.*?)\s+->\s+(.*)$", text)
    if not m:
        raise Problem("a choice is '> @id label -> target [options]'")
    c.id, c.label, rest = m.group(1), m.group(2).strip(), m.group(3)
    toks = TOKEN_SPLIT.findall(rest)
    i = 0
    c.targets.append(toks[i])
    i += 1
    while i + 1 < len(toks) and toks[i] == "|":
        c.targets.append(toks[i + 1])
        i += 2
    key = None
    vals = {}
    while i < len(toks):
        t = toks[i]
        if t == "dry":
            c.dry = True
            key = None
        elif t == "decides":
            vals.setdefault("decides", [])
            key = None
        elif t in CHOICE_KEYS:
            key = t
            vals.setdefault(key, [])
        elif key is None:
            raise Problem("unexpected '%s' after the targets" % t)
        else:
            vals[key].append(t)
        i += 1
    c.check = " ".join(vals.get("check", []))
    c.cost = " ".join(vals.get("cost", []))
    c.cond = " ".join(vals.get("if", []))
    c.do = " ".join(vals.get("do", []))
    c.short = " ".join(vals.get("short", []))
    c.els = " ".join(vals.get("else", []))
    c.truth = " ".join(vals.get("truth", []))
    c.callout = " ".join(vals.get("callout", []))
    c.found = " ".join(vals.get("found", []))
    tell = " ".join(vals.get("tell", []))
    if "tell" in vals and tell != "always":
        raise Problem("'tell' is 'tell always' (a lie that's always caught)")
    c.tell = tell == "always"
    if "decides" in vals:
        # `decides`: this pick settles the scene, as its pass target's value
        c.do = (c.do + "; " if c.do else "") + "decide:" + c.targets[0]
    if "label" in vals:
        raise Problem("'label' (the action-menu label) isn't used: the talk screen shows the whole label")
    return c


def parse_file(path):
    convs = []
    conv = None
    node = None
    with open(path, encoding="utf-8") as f:
        lines = f.read().split("\n")
    for n, raw in enumerate(lines, 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if any(ord(ch) > 126 for ch in line):
            raise Problem("%s:%d: only plain ASCII text (straight quotes, no dashes or ellipses)" % (rel(path), n))
        try:
            word, _, rest = line.partition(" ")
            rest = rest.strip()
            if word == "conv":
                conv = Conv(rest, path, n)
                convs.append(conv)
                node = None
                continue
            if conv is None:
                raise Problem("text before the first 'conv'")
            if word == "for":
                conv.speakers = [s.strip() for s in rest.split(",") if s.strip()]
            elif word == "when":
                conv.when = rest
            elif word == "topic":
                conv.kind = "topic"
                conv.label = unquote(rest)
            elif word == "greet":
                conv.kind = "greet"
            elif word == "bark":
                conv.kind = "bark"
            elif word == "prio":
                conv.prio = rest
            elif word == "once":
                conv.once = True
            elif word == "daily":
                conv.daily = True
            elif word == "always":
                conv.always = True
            elif word == "voice":
                conv.voice = rest
            elif word == "node":
                bits = rest.split()
                if not bits:
                    raise Problem("a node needs an id")
                if len(bits) > 2 or (len(bits) == 2 and bits[1] not in ("public", "confirm", "shared")):
                    raise Problem("a node is 'node <id> [public|shared|confirm]'")
                node = Node(bits[0], n)
                # shared: a scene's node the whole room sees (overheard, like public)
                node.public = len(bits) == 2 and bits[1] in ("public", "shared")
                node.confirm = len(bits) == 2 and bits[1] == "confirm"
                conv.nodes.append(node)
            elif word in ("say", "say?", "jump", "do") or line.startswith(">"):
                if node is None:
                    raise Problem("'%s' outside a node" % word)
                if line.startswith(">"):
                    node.choices.append(parse_choice(line, n))
                elif word == "say":
                    node.items.append(("say", "", strip_mark(node, role_line(rest))))
                elif word == "say?":
                    cond, sep, text = rest.partition("::")
                    if not sep:
                        raise Problem("say? is 'say? <conditions> :: <text>'")
                    node.items.append(("say", cond.strip(), strip_mark(node, role_line(text.strip()))))
                elif word == "jump":
                    cond, sep, target = rest.partition("->")
                    if not sep:
                        raise Problem("jump is 'jump <conditions> -> <node>'")
                    node.items.append(("jump", cond.strip(), target.strip()))
                else:
                    node.items.append(("do", "", rest))
            elif word == "scene":
                head, sep, who = rest.partition(" who ")
                bits = head.split()
                if len(bits) != 2 or bits[1] not in SCENE_SCOPES or (sep and not who.strip()):
                    raise Problem("a scene is 'scene <id> speaker|season [who <conditions>]'")
                conv.scene, conv.scope = bits[0], bits[1]
                conv.who = who.strip()
            elif word == "bind":
                m = re.match(r"^([a-z_]+)\s*=\s*(town:nearest|town:thick_within\([0-9]+\)|house:nearest|house:nearest_short)$", rest)
                if not m:
                    raise Problem("bind is 'bind <name> = town:nearest | town:thick_within(<m>) | house:nearest | house:nearest_short'")
                conv.binds.append((m.group(1), m.group(2)))
            elif word == "use":
                if not re.match(r"^[a-z][a-z0-9_]*(\.[a-z0-9_]+)+$", rest):
                    raise Problem("use is 'use <conversation id>'")
                conv.uses.append(rest)
            elif word == "opens":
                bits = rest.split()
                if len(bits) != 2 or bits[0] != "approach" or not bits[1].isdigit():
                    raise Problem("opens is 'opens approach <metres>'")
                conv.opens = int(bits[1])
            else:
                raise Problem("unknown keyword '%s'" % word)
        except Problem as e:
            raise Problem("%s:%d: %s" % (rel(path), n, e))
    return convs


ROLE_RE = re.compile(r"^\(([a-z][a-z ]*)\)\s+(.*)$")


def role_line(text):
    """say (<role>) <text>: someone else in the scene says it, shown as 'Role: text'."""
    m = ROLE_RE.match(text)
    if not m:
        return text
    role = m.group(1)
    return role[0].upper() + role[1:] + ": " + m.group(2)


def strip_mark(node, text):
    """A line ending in ~lie:<key> or ~truth:<key>: the mark is kept on the node, the text without it."""
    m = MARK_RE.search(text)
    if not m:
        return text
    node.marks[len(node.items)] = (m.group(1), m.group(2))
    return text[:m.start()].rstrip()


def unquote(s):
    s = s.strip()
    if len(s) >= 2 and s[0] == '"' and s[-1] == '"':
        return s[1:-1]
    return s


def load_all():
    convs = []
    errors = []
    for dirpath, _, files in os.walk(SRC):
        for name in sorted(files):
            if name.endswith(".rzd"):
                try:
                    convs.extend(parse_file(os.path.join(dirpath, name)))
                except Problem as e:
                    errors.append(str(e))
    convs.sort(key=lambda c: (area_of(c.path), c.id))
    by_id = {c.id: c for c in convs}
    for c in convs:
        for u in c.uses:
            other = by_id.get(u)
            if not other:
                errors.append("%s:%d (%s): use %s: no such conversation" % (rel(c.path), c.line, c.id, u))
                continue
            have = {n.id for n in c.nodes}
            for n in other.nodes:
                if n.id != "start" and n.id not in have:
                    c.nodes.append(n)
    return convs, errors


# ------------------------------------------------------------------------------------------ lint
TERM_RE = re.compile(r"^(!?)([a-z_]+)(?::([^<>=!]+))?(?:(>=|<=|!=|=|>|<)(.+))?$")


def lint_cond(cond, where, errs):
    keys = set()
    if not cond.strip():
        return keys
    for term in [t.strip() for t in cond.split("&")]:
        m = TERM_RE.match(term)
        if not m:
            errs.append("%s: condition '%s' isn't [!]key[:arg][op value]" % (where, term))
            continue
        neg, key, arg, op, val = m.groups()
        keys.add(key)
        if key in QUIET_KEYS:
            continue
        if key not in COND_KEYS:
            errs.append("%s: unknown condition key '%s'" % (where, key))
            continue
        takes_arg, takes_op = COND_KEYS[key]
        if takes_arg and not arg and key not in ("rep",):
            errs.append("%s: '%s' needs an argument (%s:...)" % (where, key, key))
        if op and not takes_op:
            errs.append("%s: '%s' takes no operator" % (where, key))
        if key == "bg":
            for b in (arg or "").split(","):
                if b not in BACKGROUNDS:
                    errs.append("%s: unknown background '%s'" % (where, b))
        if key in ("tier", "fame", "infamy", "rep", "hunted", "face"):
            f = (arg or "").split(",")[0]
            if f not in FACTIONS or f == "none":
                errs.append("%s: '%s' needs a faction (haven, bastion, redhand, halcyon)" % (where, key))
        if key == "tier" and val and val not in TIERS:
            errs.append("%s: unknown tier '%s'" % (where, val))
        if key == "trait" and arg not in TRAITS:
            errs.append("%s: unknown trait '%s'" % (where, arg))
        if key == "finale" and arg not in FINALE_WORDS:
            errs.append("%s: finale is finale:soon|pitch|open|locked|voice" % where)
        if key == "kit" and arg not in KIT_ARGS:
            errs.append("%s: kit is kit:own, in_kit, full, other, any, issued, signed_other, dead_mans or off_turned" % where)
        if key == "cover" and arg not in COVER_ARGS:
            errs.append("%s: cover is cover:name, dead, crew, notebook or paper" % where)
        if key == "route" and arg not in ROUTE_ARGS:
            errs.append("%s: route is route:none, any, in, out, independent, unsided, won, lost or a route's name" % where)
        if key in ("title", "seasoned", "owed", "setword", "truce", "feud") and arg not in ("haven", "bastion", "redhand", "halcyon"):
            errs.append("%s: '%s' needs a faction (haven, bastion, redhand, halcyon)" % (where, key))
        if key == "cure_held" and arg not in CURE_HELD:
            errs.append("%s: cure_held is cure_held:%s" % (where, "|".join(sorted(CURE_HELD))))
        if key in ("ending", "was") and arg not in MAJORS:
            errs.append("%s: '%s' names a major (%s)" % (where, key, ", ".join(sorted(MAJORS))))
        if key == "lean" and (arg or "").count(".") != 2:
            errs.append("%s: lean is lean:<major>.<vote>.<word>" % where)
        if key == "q" and "." not in (arg or ""):
            errs.append("%s: q is q:<questline>.<flag>[op N]" % where)
        if key == "secure" and arg and arg not in ("bastion", "redhand", "halcyon"):
            errs.append("%s: secure is secure or secure:bastion|redhand|halcyon" % where)
        if key in ("sitting", "tabled") and (arg or "").count(".") != (1 if key == "sitting" else 2):
            errs.append("%s: %s is %s:<major>.<vote>%s" % (where, key, key, "" if key == "sitting" else ".<word>"))
    return keys


def lint_effects(fx, where, conv, errs, favor_total):
    names = []
    if not fx.strip():
        return names
    for part in [p.strip() for p in fx.split(";") if p.strip()]:
        name, _, arg = part.partition(":")
        names.append(name)
        if name in FORBIDDEN_EFFECTS:
            errs.append("%s: '%s' is never a talk effect (talk never pays Fame; Infamy comes only from tagged checks)" % (where, name))
        elif name in WORLD_EFFECTS:
            errs.append("%s: '%s' needs scenes, errands or a later phase (docs/DIALOGUE.md, 'Not built yet')" % (where, name))
        elif name not in EFFECTS:
            errs.append("%s: unknown effect '%s'" % (where, name))
        elif name == "regard":
            if arg not in REGARD_WORDS:
                errs.append("%s: unknown regard word '%s'" % (where, arg))
            if arg == "lied":
                errs.append("%s: regard:lied comes only from a caught lie (phase 4)" % where)
            if conv.voice == "table":
                errs.append("%s: the Long Table keeps no regard" % where)
        elif name == "favor":
            m = re.match(r"^(haven|bastion|redhand|halcyon)\+([0-9]+)$", arg)
            if not m:
                errs.append("%s: favor is favor:<faction>+N" % where)
            else:
                favor_total[0] += int(m.group(2))
        elif name == "call" and not CALLS.match(arg):
            errs.append("%s: unknown call '%s' (slot:N, hand_in:N, toll, post_wave, standing, season, look, distress, net_word, net_lost, declare_bite)" % (where, arg))
        elif name == "call" and arg in ("season", "look") and not all(sp.startswith(("leader", "stronghold", "seat")) for sp in conv.speakers):
            errs.append("%s: call:%s is a leader's (the epilogue's carriers)" % (where, arg))
        elif name == "call" and arg in ("net_word", "net_lost") and not all(sp.startswith("radio:") and sp != "radio:open" for sp in conv.speakers):
            errs.append("%s: call:%s is on a secure net" % (where, arg))
        elif name == "call" and arg == "distress" and conv.speakers != ["radio:open"]:
            errs.append("%s: call:distress is on the open channel" % where)
        elif name == "call" and arg in ("declare_bite", "caught_bite") and conv.speakers != ["occupant"]:
            errs.append("%s: call:declare_bite is at a safehouse" % where)
        elif name == "offer" and arg != "leader_job":
            errs.append("%s: offer takes leader_job" % where)
        elif name == "offer" and not all(s.startswith(("leader", "stronghold", "seat")) for s in conv.speakers):
            errs.append("%s: offer is only for leaders" % where)
        elif name == "sway" and arg.count(".") >= 2 and not conv.scene:
            pass    # a major's sitting (every_name.first.pot): its own vote, no scene needed
        elif name == "set" and not re.match(r"^[a-z0-9_.@]+=[0-9]+$", arg):
            errs.append("%s: set is set:<key>=N" % where)
        elif name == "qset" and not re.match(r"^[a-z_]+\.[a-z0-9_.]+=[0-9]+$", arg):
            errs.append("%s: qset is qset:<questline>.<flag>=N" % where)
        elif name == "begin" and not re.match(r"^[a-z_]+$", arg):
            errs.append("%s: begin is begin:<questline>" % where)
        elif name == "note" and not re.match(r'^[a-z_]+:"[^"]+"$', arg):
            errs.append("%s: note is note:<questline>:\"text\"" % where)
        elif name == "approach" and not re.match(r'^[a-z_]+:(talk|persuade|sneak|force|bribe|betray|side|press|trade|trick|leave)(:"[^"]*")?$', arg):
            errs.append("%s: approach is approach:<questline>:<way>[:\"what\"]" % where)
        elif name in SCENE_EFFECTS and not conv.scene:
            errs.append("%s: '%s' needs a scene in this conversation ('scene <id> speaker|season')" % (where, name))
        elif name in SCENE_EFFECTS and not re.match(r"^[a-z0-9_.]+$", arg):
            errs.append("%s: %s is %s:<word>" % (where, name, name))
        elif name == "commit" and arg not in COMMIT_ROUTES:
            errs.append("%s: commit is commit:haven|bastion|redhand|halcyon|indep" % where)
        elif name == "cover" and arg not in COVER_ARGS:
            errs.append("%s: cover is cover:name, dead, crew, notebook or paper" % where)
        elif name == "take" and not re.match(r"^food\*[0-9]+$", arg):
            errs.append("%s: take is take:food*N" % where)
        elif name == "watch" and not re.match(r"^[a-z0-9_.]+@[a-z_]+:[0-9]+$", arg):
            errs.append("%s: watch is watch:<key>@<bound name>:<infection>" % where)
        elif name == "watch" and arg.split("@")[1].split(":")[0] not in [b[0] for b in conv.binds]:
            errs.append("%s: watch names a binding this conversation doesn't have ('bind <name> = ...')" % where)
        elif name == "unsway" and "found" not in where:
            errs.append("%s: unsway is only a lie's found effect" % where)
        elif name == "test" and not re.match(r"^[a-z0-9_]+:[0-9]+(\.[0-9]+)?:[0-9]+$", arg):
            errs.append("%s: test is test:<claim>:<hour>:<metres>" % where)
        elif name == "learn" and not re.match(r"^[a-z0-9_.@]+(:[0-9.]+h)?$", arg):
            errs.append("%s: learn is learn:<key>[:<hours>h]" % where)
        elif name == "later" and not re.match(r"^[0-9]+h:[a-z0-9_.@]+$", arg):
            errs.append("%s: later is later:<hours>h:<key>" % where)
        elif name == "promise" and not re.match(r"^[a-z0-9_.@]+(:[0-9.]+h)?$", arg):
            errs.append("%s: promise is promise:<key>[:<hours>h]" % where)
        elif name == "keep" and not re.match(r"^[a-z0-9_.@]+$", arg):
            errs.append("%s: keep is keep:<key>" % where)
        elif name == "wire" and (arg not in ("1", "2", "3") or not all(sp in ("post:bastion", "named:bastion") or sp.startswith("named:bastion/") for sp in conv.speakers)):
            errs.append("%s: wire is wire:1, 2 or 3, said to Bastion's people" % where)
    return names


def lint_text(text, where, conv, errs, warns):
    if len(text) > SAY_LIMIT:
        errs.append("%s: %d characters (limit %d)" % (where, len(text), SAY_LIMIT))
    for m in TOKEN_RE.finditer(text):
        name, arg = m.group(1), m.group(2)
        ok = (name in WORLD_KINDS or name in DIALOGUE_KINDS) if arg is not None else (name in WORLD_TOKENS or name in DIALOGUE_TOKENS)
        if not ok:
            errs.append("%s: unknown token %s" % (where, m.group(0)))
        if name == "their" and not any(sp.startswith("stranger") for sp in conv.speakers):
            errs.append("%s: {their:...} is a stranger's own words (for stranger:...)" % where)
        if name == "title" and arg not in ("haven", "bastion", "redhand", "halcyon"):
            errs.append("%s: {title:<faction>}" % where)
        if name == "wire" and arg not in ("1", "2", "3"):
            errs.append("%s: {wire:1}, {wire:2} or {wire:3}" % where)
        if name in ("tier", "rep", "word") and arg is not None and arg not in ("haven", "bastion", "redhand", "halcyon"):
            errs.append("%s: {%s:<faction>}" % (where, name))
        if name in ("bind",) and arg not in [b[0] for b in conv.binds]:
            errs.append("%s: {bind:%s}: no 'bind %s = ...' in this conversation" % (where, arg, arg))
        if name == "town" and arg and arg.startswith("@") and arg[1:] not in [b[0] for b in conv.binds]:
            errs.append("%s: {town:%s}: no such binding" % (where, arg))
        if name == "decider" and arg not in MAJORS:
            errs.append("%s: {decider:<major>} (%s)" % (where, ", ".join(sorted(MAJORS))))
        if name == "lean" and (arg or "").count(".") != 1:
            errs.append("%s: {lean:<major>.<vote>}" % where)
        if name in PERSONAL_TOKENS and any(sp.startswith("radio") for sp in conv.speakers):
            errs.append("%s: no name on air: %s in a radio conversation" % (where, m.group(0)))
    if INFECTED_WORDS.search(text):
        errs.append("%s: player text never says alpha, tribe or crawler" % where)
    if conv.voice == "table":
        m = TABLE_BANNED.search(text)
        if m:
            errs.append("%s: the Long Table is never described ('%s')" % (where, m.group(0)))
    else:
        if TABLE_NAMES.search(text):
            errs.append("%s: a Long Table name outside the table's files" % where)
        m = LONG_TABLE_HINT.search(text)
        if m:
            errs.append("%s: hints at the Long Table ('%s')" % (where, m.group(0)))
    if PRONOUN_AFTER_NPC.search(text):
        errs.append("%s: no he, she, him or her about a seeded speaker" % where)
    if conv.voice == "bastion" and "!" in text:
        warns.append("%s: Bastion line with an exclamation mark" % where)
    if "!" in text and conv.voice not in ("redhand", "bastion"):
        errs.append("%s: '!' only in Red Hand lines" % where)


def lint(convs):
    errs = []
    warns = []
    ids = {}
    told_lies = set()
    for c in convs:
        for n in c.nodes:
            for kind, key in n.marks.values():
                if kind == "lie":
                    told_lies.add(key)
    for c in convs:
        where_c = "%s:%d (%s)" % (rel(c.path), c.line, c.id)
        if not re.match(r"^[a-z][a-z0-9_]*(\.[a-z0-9_]+)+$", c.id):
            errs.append("%s: conversation ids are dotted lower case (haven.head.house)" % where_c)
        if c.id in ids:
            errs.append("%s: id also used at %s" % (where_c, ids[c.id]))
        ids[c.id] = where_c
        if not c.speakers:
            errs.append("%s: no 'for'" % where_c)
        for s in c.speakers:
            if not SPEAKER_RE.match(s):
                errs.append("%s: unknown speaker '%s'" % (where_c, s))
        if c.kind not in ("topic", "greet", "bark"):
            errs.append("%s: needs 'topic \"<label>\"', 'greet' or 'bark'" % where_c)
        if c.kind == "topic":
            if not c.label:
                errs.append("%s: an empty topic label" % where_c)
            if len(c.label) > TOPIC_LIMIT:
                errs.append("%s: topic label of %d characters (limit %d)" % (where_c, len(c.label), TOPIC_LIMIT))
        if c.prio not in PRIOS:
            errs.append("%s: prio is quest, ask, world, reaction or flavour" % where_c)
        if c.voice not in VOICES:
            errs.append("%s: voice is haven, bastion, redhand, halcyon, table or plain" % where_c)
        if c.voice == "table" and not all(s.startswith("table") for s in c.speakers):
            errs.append("%s: voice table is only for the Long Table's people" % where_c)
        if any(s.startswith("table") for s in c.speakers) and c.voice != "table":
            errs.append("%s: the Long Table's people speak in voice table" % where_c)
        when_keys = lint_cond(c.when, where_c + " when", errs)
        if c.who:
            lint_cond(c.who, where_c + " who", errs)
        if c.prio == "ask" and not (when_keys & WORLD_KEYS):
            errs.append("%s: an ask must read the world (house, town, need, a bite, the speaker's state...)" % where_c)
        if not c.nodes:
            errs.append("%s: no nodes" % where_c)
            continue
        if c.nodes[0].id != "start":
            errs.append("%s: the first node is 'start'" % where_c)
        if c.kind == "greet":
            if len(c.nodes) > 1 or c.nodes[0].choices:
                errs.append("%s: a greet is one node of lines, no choices (greets with choices come with phase 4)" % where_c)
        if c.kind == "bark":
            if len(c.nodes) > 1 or c.nodes[0].choices or c.nodes[0].public:
                errs.append("%s: a bark is one node of lines, no choices, never public" % where_c)
            if any(it[0] == "do" for it in c.nodes[0].items):
                errs.append("%s: a bark does nothing but say its line" % where_c)
            if any(sp.startswith("radio") for sp in c.speakers):
                errs.append("%s: no barks on the radio" % where_c)
        node_ids = {}
        choice_ids = {}
        favor_total = [0]
        for n in c.nodes:
            wn = "%s:%d (%s/%s)" % (rel(c.path), n.line, c.id, n.id)
            if n.id in node_ids:
                errs.append("%s: node id used twice" % wn)
            node_ids[n.id] = n
            says = [it for it in n.items if it[0] == "say"]
            if not says:
                errs.append("%s: a node says something" % wn)
            if len([s for s in says if not s[1]]) > 3:
                warns.append("%s: more than three lines" % wn)
            for kind, cond, val in n.items:
                if kind == "say":
                    lint_cond(cond, wn, errs)
                    lint_text(val, wn, c, errs, warns)
                elif kind == "jump":
                    lint_cond(cond, wn, errs)
                elif kind == "do":
                    fx = lint_effects(val, wn, c, errs, favor_total)
                    if "regard" in fx and "thanks" in val:
                        errs.append("%s: regard:thanks only on a choice with a cost or a deed" % wn)
            if len(n.choices) > NODE_CHOICES:
                errs.append("%s: %d choices (at most %d; Leave is added)" % (wn, len(n.choices), NODE_CHOICES))
            for ch in n.choices:
                wch = "%s:%d (%s/@%s)" % (rel(c.path), ch.line, c.id, ch.id)
                if ch.id in choice_ids:
                    errs.append("%s: choice id also used in node %s" % (wch, choice_ids[ch.id]))
                choice_ids[ch.id] = n.id
                if len(ch.label) > LABEL_LIMIT:
                    errs.append("%s: label of %d characters (limit %d)" % (wch, len(ch.label), LABEL_LIMIT))
                lint_text(ch.label, wch, c, errs, warns)
                lint_cond(ch.cond, wch, errs)
                fx = lint_effects(ch.do, wch, c, errs, favor_total)
                lint_effects(ch.short, wch, c, errs, favor_total)
                lint_effects(ch.els, wch, c, errs, favor_total)
                check = ch.check.split()
                if check:
                    if check[0] in LATER_CHECKS:
                        errs.append("%s: '%s' checks come with phase 4" % (wch, check[0]))
                    elif check[0] not in CHECKS:
                        errs.append("%s: unknown check '%s'" % (wch, check[0]))
                    elif len(check) < 3 or check[1] not in FACTIONS or not check[2].isdigit() or not 1 <= int(check[2]) <= 4:
                        errs.append("%s: a check is '%s <faction|none> <difficulty 1-4> [easier:trait] [harder:trait] [bg:list]'" % (wch, check[0]))
                    for extra in check[3:]:
                        k, _, v = extra.partition(":")
                        if k in ("easier", "harder") and v in TRAITS:
                            continue
                        if k == "bg" and all(b in BACKGROUNDS for b in v.split(",")):
                            continue
                        errs.append("%s: unknown check option '%s'" % (wch, extra))
                    if check[0] == "threaten":
                        bad = [s for s in c.speakers if s.startswith(("stronghold", "table", "occupant"))
                               or s in ("leader", "wanderer", "wanderer:kid") or s.startswith("leader:")]
                        if bad:
                            errs.append("%s: no threats at %s (stronghold leaders, the Kid, the Long Table, safehouses)" % (wch, ", ".join(bad)))
                    if len(ch.targets) < 2:
                        errs.append("%s: a check names a pass and a fail target (pass | short [| refused])" % wch)
                    for t in ch.targets[1:]:
                        if t == "end":
                            errs.append("%s: a failed check never goes to 'end' (failure is a branch)" % wch)
                elif len(ch.targets) > 1:
                    errs.append("%s: several targets but no check" % wch)
                if ch.cost:
                    if not re.match(r"^(favor:(haven|bastion|redhand|halcyon) [0-9]+|item:food\*[0-9]+)$", ch.cost):
                        errs.append("%s: cost is 'favor:<faction> N' or 'item:food*N'" % wch)
                if ch.truth:
                    tkeys = lint_cond(ch.truth, wch + " truth", errs)
                    if not check or check[0] != "lie":
                        errs.append("%s: 'truth' belongs on a [Lie] (check lie ...)" % wch)
                    unknowable = tkeys - KNOWABLE_KEYS
                    if unknowable:
                        errs.append("%s: a lie's truth reads only what the player can know (%s)" % (wch, ", ".join(sorted(unknowable))))
                if (ch.tell or ch.found) and (not check or check[0] != "lie"):
                    errs.append("%s: 'tell always' and 'found' belong on a [Lie]" % wch)
                if ch.found:
                    for part in [p.strip() for p in ch.found.split(";") if p.strip()]:
                        fname = part.partition(":")[0]
                        if fname not in FOUND_EFFECTS:
                            errs.append("%s: a found effect only touches what's still running (%s)" % (wch, ", ".join(sorted(FOUND_EFFECTS))))
                    lint_effects(ch.found, wch + " found", c, errs, [0])
                if check and check[0] == "lie" and not ch.truth and not ch.tell:
                    warns.append("%s: a [Lie] without 'truth <conditions>' or 'tell always'" % wch)
                if ch.callout and ch.callout not in told_lies:
                    errs.append("%s: callout %s: no line tells that lie (~lie:%s)" % (wch, ch.callout, ch.callout))
                if check and check[0] == "look":
                    if check[1] == "none" or check[1] == "haven":
                        errs.append("%s: a look is for Bastion, the Red Hand or Halcyon kit (no disguise with the Haven)" % wch)
                    barred = set(fx) & CLAIM_BARRED
                    passn = c.node(ch.targets[0])
                    if passn:
                        for kind, _, val in passn.items:
                            if kind == "do":
                                barred |= set(p.strip().partition(":")[0] for p in val.split(";")) & CLAIM_BARRED
                    if barred:
                        errs.append("%s: no rewards through a claim (%s)" % (wch, ", ".join(sorted(barred))))
                if check and check[0] == "threaten" and any(sp.startswith(("radio", "stranger")) for sp in c.speakers):
                    errs.append("%s: no threats on air or at a stranger through talk" % wch)
                if ch.dry:
                    twins = [o for o in n.choices if o is not ch and not o.dry and o.targets == ch.targets
                             and o.do == ch.do and o.check == ch.check and o.cost == ch.cost]
                    if not twins:
                        errs.append("%s: a dry line leads where a plain line in the same node does (same targets, check, cost and effects)" % wch)
                if "regard" in fx and "regard:thanks" in ch.do and not (ch.cost or "take:" in ch.do or "call:" in ch.do):
                    errs.append("%s: regard:thanks needs a cost or a deed on the same choice" % wch)
                for t in ch.targets:
                    if t not in TARGETS_SPECIAL and not c.node(t):
                        errs.append("%s: no node '%s'" % (wch, t))
        # A season flag set in a conversation that serves many speakers is bound to one (@here, a binding)
        many = any(sp in GENERIC_SPEAKERS or sp.startswith(("named:", "post:", "wanderer:", "stranger", "leader:", "quest", "mission")) for sp in c.speakers)
        own = c.id.split(".")[0]
        for n in c.nodes:
            fx_all = [v for k, _, v in n.items if k == "do"] + [ch.do for ch in n.choices]
            for fx in fx_all:
                for part in [p.strip() for p in fx.split(";") if p.strip()]:
                    fname, _, farg = part.partition(":")
                    if fname != "set":
                        continue
                    key = farg.split("=")[0]
                    if many and "@" not in key and c.scope != "season":
                        errs.append("%s: set:%s in a conversation for many speakers needs @here or a binding" % (where_c, key))
                    if not key.startswith(own + "."):
                        errs.append("%s: set:%s is outside this conversation's own namespace (%s.)" % (where_c, key, own))
        # Halcyon's long trees carry numbers: the lab talks in them
        if c.voice == "halcyon" and len(c.nodes) >= 4:
            if not any(NUMBER_RE.search(v) for n in c.nodes for k, _, v in n.items if k == "say"):
                errs.append("%s: a Halcyon tree of four nodes or more says a number somewhere" % where_c)
        for u in c.uses:
            if u == c.id:
                errs.append("%s: a conversation can't use itself" % where_c)
        if favor_total[0] > FAVOR_PER_CONV:
            errs.append("%s: %d Favor in one conversation (at most %d)" % (where_c, favor_total[0], FAVOR_PER_CONV))
        # Reachable nodes
        seen = set()
        todo = ["start"]
        while todo:
            nid = todo.pop()
            if nid in seen or not c.node(nid):
                continue
            seen.add(nid)
            nd = c.node(nid)
            for kind, cond, val in nd.items:
                if kind == "jump":
                    todo.append(val)
            for ch in nd.choices:
                todo.extend(ch.targets)
        for n in c.nodes:
            if n.id not in seen:
                errs.append("%s:%d (%s/%s): no path reaches this node" % (rel(c.path), n.line, c.id, n.id))
        # A failed check's node offers more than leaving
        for n in c.nodes:
            for ch in n.choices:
                if ch.check and len(ch.targets) > 1:
                    for t in ch.targets[1:]:
                        fail = c.node(t)
                        if fail and not fail.choices:
                            errs.append("%s:%d (%s/@%s): the fail node '%s' offers only Leave" % (rel(c.path), ch.line, c.id, ch.id, t))
        # The finale's commit (docs/DIALOGUE.md, "The finale's commit"): only in a confirm node
        has_confirm = False
        for n in c.nodes:
            wn = "%s:%d (%s/%s)" % (rel(c.path), n.line, c.id, n.id)
            commits = [ch for ch in n.choices if "commit:" in ch.do]
            for kind, cond, val in n.items:
                if kind == "do" and "commit:" in val:
                    errs.append("%s: commit runs only from a choice, never on entering a node" % wn)
            for ch in n.choices:
                if "commit:" in ch.short or "commit:" in ch.els:
                    errs.append("%s: commit is never a check's short or else" % wn)
            if not n.confirm:
                if commits:
                    errs.append("%s: commit only from a confirm node ('node %s confirm')" % (wn, n.id))
                continue
            has_confirm = True
            if len(commits) != 1:
                errs.append("%s: a confirm node has exactly one commit choice" % wn)
            if len(commits) == len(n.choices):
                errs.append("%s: a confirm node also offers a way back" % wn)
            for ch in commits:
                if ch.cond or ch.check or ch.cost:
                    errs.append("%s: a commit choice has no if, check or cost (the finale decides who may lock)" % wn)
        if c.opens:
            if c.kind != "greet":
                errs.append("%s: only a greet opens by itself ('opens approach <metres>')" % where_c)
            if not 1 <= c.opens <= 40:
                errs.append("%s: opens approach 1 to 40 metres" % where_c)
            if any(sp.startswith("radio") for sp in c.speakers):
                errs.append("%s: the radio never opens by itself" % where_c)
        secure = [sp for sp in c.speakers if sp.startswith("radio:") and sp != "radio:open"]
        if secure:
            if any(not (sp.startswith("radio:") and sp != "radio:open") for sp in c.speakers):
                errs.append("%s: a secure net's conversation is for the secure nets only" % where_c)
            for n in c.nodes:
                if n.public:
                    errs.append("%s: nothing on a secure net is public (node %s)" % (where_c, n.id))
        if c.always and not has_confirm:
            errs.append("%s: 'always' is only for the finale's route topics (a topic with a confirm node)" % where_c)
    return errs, warns


# ------------------------------------------------------------------------------------------ lock
def shipped_ids(convs):
    out = []
    for c in convs:
        out.append(c.id)
        for n in c.nodes:
            for ch in n.choices:
                out.append("%s/@%s" % (c.id, ch.id))
    return out


def read_lock():
    ids = set()
    retired = set()
    if not os.path.exists(LOCK):
        return ids, retired
    with open(LOCK, encoding="utf-8") as f:
        for line in f:
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            if line.startswith("retired "):
                retired.add(line[8:].strip())
            else:
                ids.add(line)
    return ids, retired


def write_lock(ids, retired):
    with open(LOCK, "w", encoding="utf-8", newline="\n") as f:
        f.write("# Shipped dialogue ids (tools/dialogue_pack.py --lock). Memory is keyed by these: never remove one.\n")
        f.write("# A conversation or choice that's gone is kept as 'retired <id>'.\n")
        for i in sorted(ids):
            f.write(i + "\n")
        for i in sorted(retired):
            f.write("retired " + i + "\n")


# ------------------------------------------------------------------------------------------ pack
def esc(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def pack_area(area, convs):
    calls = []
    for c in convs:
        flags = ((1 if c.once else 0) | (2 if c.daily else 0) | (4 if c.kind == "greet" else 0) | (8 if c.always else 0)
                 | (16 if c.kind == "bark" else 0))
        calls.append("db.Conv(%s, %s, %s, %s, %d, %d, %s);" % (esc(c.id), esc(",".join(c.speakers)), esc(c.when),
                                                             esc(c.label), PRIOS[c.prio], flags, esc(c.voice)))
        if c.opens:
            calls.append("db.Opens(%d);" % c.opens)
        for name, picker in c.binds:
            calls.append("db.Bind(%s, %s);" % (esc(name), esc(picker)))
        if c.scene and c.who:
            calls.append("db.Scene(%s, %s, %s);" % (esc(c.scene), esc(c.scope), esc(c.who)))
        elif c.scene:
            calls.append("db.Scene(%s, %s);" % (esc(c.scene), esc(c.scope)))
        for n in c.nodes:
            calls.append("db.Node(%s);" % esc(n.id))
            if n.public:
                calls.append("db.Public();")
            if n.confirm:
                calls.append("db.Confirm();")
            for idx, (kind, cond, val) in enumerate(n.items):
                if kind == "say":
                    calls.append("db.Say(%s, %s);" % (esc(cond), esc(val)))
                    if idx in n.marks:
                        calls.append("db.Mark(%s, %s);" % (esc(n.marks[idx][0]), esc(n.marks[idx][1])))
                elif kind == "jump":
                    calls.append("db.Jump(%s, %s);" % (esc(cond), esc(val)))
                else:
                    calls.append("db.Do(%s);" % esc(val))
            for ch in n.choices:
                t = ch.targets + ["", ""]
                calls.append("db.Choice(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);" % (
                    esc(ch.id), esc(ch.label), esc(t[0]), esc(t[1]), esc(t[2]), esc(ch.check), esc(ch.cost),
                    esc(ch.cond), esc(ch.do), esc(ch.short), esc(ch.els)))
                if ch.truth or ch.callout:
                    calls.append("db.ChoiceExtra(%s, %s);" % (esc(ch.truth), esc(ch.callout)))
                if ch.tell or ch.found:
                    calls.append("db.ChoiceMore(%s, %s);" % ("true" if ch.tell else "false", esc(ch.found)))
    cls = "RZ_DialoguePack_" + area
    out = ["// GENERATED by tools/dialogue_pack.py from dialogue/%s/*.rzd. Do not edit: edit the .rzd and run the tool." % area,
           "class %s" % cls, "{", "\tstatic void Add(RZ_DialogueDB db)", "\t{"]
    chunks = [calls[i:i + MAX_ADDS] for i in range(0, len(calls), MAX_ADDS)] or [[]]
    for i in range(len(chunks)):
        out.append("\t\tAdd%d(db);" % i)
    out.append("\t}")
    for i, chunk in enumerate(chunks):
        out.append("")
        out.append("\tprotected static void Add%d(RZ_DialogueDB db)" % i)
        out.append("\t{")
        for call in chunk:
            out.append("\t\t" + call)
        out.append("\t}")
    out.append("}")
    return cls, "\n".join(out) + "\n"


def packs(convs):
    by_area = {}
    for c in convs:
        by_area.setdefault(area_of(c.path), []).append(c)
    files = {}
    classes = []
    for area in sorted(by_area):
        cls, text = pack_area(area, by_area[area])
        classes.append(cls)
        files[os.path.join(PACK_DIR, cls + ".c")] = text
    index = ["// GENERATED by tools/dialogue_pack.py. Do not edit.",
             "class RZ_DialoguePacks", "{", "\tstatic void AddAll(RZ_DialogueDB db)", "\t{"]
    for cls in classes:
        index.append("\t\t%s.Add(db);" % cls)
    index += ["\t}", "", "\tstatic int Count()", "\t{", "\t\treturn %d;" % len(convs), "\t}", "}"]
    files[os.path.join(PACK_DIR, "RZ_DialoguePacks.c")] = "\n".join(index) + "\n"
    return files


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--check", action="store_true", help="fail if the packs or the lock are out of date")
    ap.add_argument("--lock", action="store_true", help="add new ids to dialogue/shipped.lock")
    ap.add_argument("--list", action="store_true", help="list the conversations")
    ap.add_argument("--warnings", action="store_true", help="also print warnings")
    args = ap.parse_args()

    convs, errors = load_all()
    if args.list:
        for c in convs:
            print("%-34s %-8s %-40s %s" % (c.id, c.prio, ",".join(c.speakers), c.label or "(greet)"))
        return
    lint_errs, warns = lint(convs)
    errors += lint_errs

    ids = shipped_ids(convs)
    locked, retired = read_lock()
    have = set(ids)
    for i in sorted(locked - have - retired):
        errors.append("dialogue/shipped.lock: '%s' has shipped and is gone (keep it, or mark it 'retired %s')" % (i, i))
    new_ids = have - locked

    for e in errors:
        print("error: " + e)
    if args.warnings:
        for w in warns:
            print("warning: " + w)
    nodes = sum(len(c.nodes) for c in convs)
    print("dialogue_pack: %d conversations, %d nodes, %d errors, %d warnings" % (len(convs), nodes, len(errors), len(warns)))
    if errors:
        print("dialogue_pack: FAILED")
        sys.exit(1)

    files = packs(convs)
    if args.check:
        stale = []
        for path, text in files.items():
            old = open(path, encoding="utf-8").read() if os.path.exists(path) else None
            if old != text:
                stale.append(rel(path))
        if os.path.isdir(PACK_DIR):
            for name in os.listdir(PACK_DIR):
                if os.path.join(PACK_DIR, name) not in files:
                    stale.append(rel(os.path.join(PACK_DIR, name)) + " (no longer generated)")
        if stale:
            print("dialogue_pack: packs out of date: " + ", ".join(stale) + " (run python3 tools/dialogue_pack.py)")
            sys.exit(1)
        if new_ids:
            print("dialogue_pack: %d ids aren't in dialogue/shipped.lock (run python3 tools/dialogue_pack.py --lock)" % len(new_ids))
            sys.exit(1)
        print("dialogue_pack: OK")
        return

    os.makedirs(PACK_DIR, exist_ok=True)
    for name in os.listdir(PACK_DIR):
        if os.path.join(PACK_DIR, name) not in files:
            os.remove(os.path.join(PACK_DIR, name))
    for path, text in files.items():
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
    if args.lock:
        write_lock(locked | have, retired)
        print("dialogue_pack: %d new ids locked" % len(new_ids))
    elif new_ids:
        print("dialogue_pack: %d ids not locked yet (python3 tools/dialogue_pack.py --lock when they ship)" % len(new_ids))
    print("dialogue_pack: wrote %d packs" % (len(files) - 1))


if __name__ == "__main__":
    main()
