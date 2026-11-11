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
                        r"post|post:(bastion|redhand)|table|table:(host|mother|son|boy|road))$")

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
}
QUIET_KEYS = {"tribe", "alpha", "lean", "sitting", "finale", "route", "cure_held", "ending",
              "was", "kit", "claimed", "passing", "caught", "stranger", "promise", "heard_lie"}
# Keys that read the world or the player's place in it: an ask needs one (docs/DIALOGUE.md, "Asks").
WORLD_KEYS = {"house", "town", "need", "wbitten", "kidlost", "bg", "tier", "rep", "exiled", "bite_shown", "hunted",
              "outbreak", "cure", "flag", "stage", "infamy", "fame", "kin"}
EFFECTS = {"mark", "cset", "nset", "regard", "favor", "take", "claim", "learn", "rumour", "call", "offer", "later",
           "sway", "decide"}
SCENE_EFFECTS = {"sway", "decide"}
WORLD_EFFECTS = {"set", "qset", "resolve", "start", "event", "commit", "reward", "pass", "catch", "errand",
                 "promise", "approach", "reveal", "begin", "hostile", "flee", "note", "return"}
FORBIDDEN_EFFECTS = {"fame", "house", "infamy", "follow", "escort", "companion"}
REGARD_WORDS = {"thanks", "kept", "broke", "threatened", "slander", "vouched_liar", "lied"}
CALLS = re.compile(r"^(slot:[0-9]|hand_in:[0-9]+|toll|post_wave|standing)$")
CHECKS = {"persuade", "threaten", "lie"}
LATER_CHECKS = {"look", "vouch", "commit"}
SCENE_SCOPES = {"speaker", "season"}

# Tokens a .rzd line may use beyond the world tokens (RZ_DialogueText.Fill, RZ_RepVoice.FillFor).
DIALOGUE_TOKENS = {"npc", "npc_short", "player", "house", "resident", "word", "rep", "best", "worst", "kills",
                   "jobs", "picture", "name", "crew", "place", "here", "faction", "role", "toll", "seat", "boss"}
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


class Node:
    def __init__(self, nid, line):
        self.id = nid
        self.line = line
        self.items = []     # ("say", cond, text) | ("jump", cond, target) | ("do", effects)
        self.choices = []
        self.public = False


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
        self.voice = "plain"
        self.nodes = []
        self.scene = ""
        self.scope = ""
        self.who = ""

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
CHOICE_KEYS = {"check", "cost", "if", "do", "short", "else", "label"}


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
        if t in CHOICE_KEYS:
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
            elif word == "prio":
                conv.prio = rest
            elif word == "once":
                conv.once = True
            elif word == "daily":
                conv.daily = True
            elif word == "voice":
                conv.voice = rest
            elif word == "node":
                bits = rest.split()
                if not bits:
                    raise Problem("a node needs an id")
                if len(bits) > 2 or (len(bits) == 2 and bits[1] != "public"):
                    raise Problem("a node is 'node <id> [public]'")
                node = Node(bits[0], n)
                node.public = len(bits) == 2
                conv.nodes.append(node)
            elif word in ("say", "say?", "jump", "do") or line.startswith(">"):
                if node is None:
                    raise Problem("'%s' outside a node" % word)
                if line.startswith(">"):
                    node.choices.append(parse_choice(line, n))
                elif word == "say":
                    node.items.append(("say", "", rest))
                elif word == "say?":
                    cond, sep, text = rest.partition("::")
                    if not sep:
                        raise Problem("say? is 'say? <conditions> :: <text>'")
                    node.items.append(("say", cond.strip(), text.strip()))
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
            elif word in ("bind", "use", "opens", "always", "callout"):
                raise Problem("'%s' comes with a later phase (docs/DIALOGUE.md, 'Not built yet')" % word)
            else:
                raise Problem("unknown keyword '%s'" % word)
        except Problem as e:
            raise Problem("%s:%d: %s" % (rel(path), n, e))
    return convs


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
            errs.append("%s: unknown call '%s' (slot:N, hand_in:N, toll, post_wave, standing)" % (where, arg))
        elif name == "offer" and arg != "leader_job":
            errs.append("%s: offer takes leader_job" % where)
        elif name == "offer" and not all(s.startswith(("leader", "stronghold", "seat")) for s in conv.speakers):
            errs.append("%s: offer is only for leaders" % where)
        elif name in SCENE_EFFECTS and not conv.scene:
            errs.append("%s: '%s' needs a scene in this conversation ('scene <id> speaker|season')" % (where, name))
        elif name in SCENE_EFFECTS and not re.match(r"^[a-z0-9_.]+$", arg):
            errs.append("%s: %s is %s:<word>" % (where, name, name))
        elif name == "take" and not re.match(r"^food\*[0-9]+$", arg):
            errs.append("%s: take is take:food*N" % where)
        elif name == "later" and not re.match(r"^[0-9]+h:[a-z0-9_.]+$", arg):
            errs.append("%s: later is later:<hours>h:<key>" % where)
    return names


def lint_text(text, where, conv, errs, warns):
    if len(text) > SAY_LIMIT:
        errs.append("%s: %d characters (limit %d)" % (where, len(text), SAY_LIMIT))
    for m in TOKEN_RE.finditer(text):
        name, arg = m.group(1), m.group(2)
        ok = (name in WORLD_KINDS) if arg is not None else (name in WORLD_TOKENS or name in DIALOGUE_TOKENS)
        if not ok:
            errs.append("%s: unknown token %s" % (where, m.group(0)))
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
        if c.kind not in ("topic", "greet"):
            errs.append("%s: needs 'topic \"<label>\"' or 'greet'" % where_c)
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
                if "regard" in fx and "regard:thanks" in ch.do and not (ch.cost or "take:" in ch.do or "call:" in ch.do):
                    errs.append("%s: regard:thanks needs a cost or a deed on the same choice" % wch)
                for t in ch.targets:
                    if t not in TARGETS_SPECIAL and not c.node(t):
                        errs.append("%s: no node '%s'" % (wch, t))
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
        flags = (1 if c.once else 0) | (2 if c.daily else 0) | (4 if c.kind == "greet" else 0)
        calls.append("db.Conv(%s, %s, %s, %s, %d, %d, %s);" % (esc(c.id), esc(",".join(c.speakers)), esc(c.when),
                                                             esc(c.label), PRIOS[c.prio], flags, esc(c.voice)))
        if c.scene and c.who:
            calls.append("db.Scene(%s, %s, %s);" % (esc(c.scene), esc(c.scope), esc(c.who)))
        elif c.scene:
            calls.append("db.Scene(%s, %s);" % (esc(c.scene), esc(c.scope)))
        for n in c.nodes:
            calls.append("db.Node(%s);" % esc(n.id))
            if n.public:
                calls.append("db.Public();")
            for kind, cond, val in n.items:
                if kind == "say":
                    calls.append("db.Say(%s, %s);" % (esc(cond), esc(val)))
                elif kind == "jump":
                    calls.append("db.Jump(%s, %s);" % (esc(cond), esc(val)))
                else:
                    calls.append("db.Do(%s);" % esc(val))
            for ch in n.choices:
                t = ch.targets + ["", ""]
                calls.append("db.Choice(%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);" % (
                    esc(ch.id), esc(ch.label), esc(t[0]), esc(t[1]), esc(t[2]), esc(ch.check), esc(ch.cost),
                    esc(ch.cond), esc(ch.do), esc(ch.short), esc(ch.els)))
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
