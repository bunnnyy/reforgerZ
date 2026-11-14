#!/usr/bin/env python3
"""ReforgerZ text lint (docs/WRITING.md, "Checking text"; how to run it: docs/WORKBENCH_SETUP.md, "Text lint").

    python3 tools/voice_lint.py                     # check; exit 1 on any finding not in the baseline
    python3 tools/voice_lint.py --all               # also list the findings the baseline holds
    python3 tools/voice_lint.py --warnings          # also list warnings (long briefs), which never fail
    python3 tools/voice_lint.py --update-baseline   # write today's findings to the baseline
    python3 tools/voice_lint.py --self-test         # plant bad lines and check that each check fails

Checks every player-facing literal that tools/strings_inventory.py finds against the rules in
docs/WRITING.md:

    length       radio, hints, leader lines, rumours, headlines, the broadcast, loudspeakers and
                 Traces' lines (trace, door): 200 characters or fewer. Labels and choices: the action part
                 (before a bracket) 40 or fewer, the whole label 64 or fewer. Briefs: a warning
                 over 500.
    quote        a spoken line (talk, radio, loudspeakers, the broadcast, the voice pools, replies,
                 a conversation's says and choices) has no double quotation marks: it's speech
                 already (a quote inside it takes single quotes)
    speaker      a talk line doesn't start with a speaker's label ("The guard: ...", "Sgt Hale: ")
    reply        a reply is what you say, not a menu label: not a bare label word ("Trade",
                 "Leave.", "Something else."), not an action ("Ask for work", "Tell them..."),
                 and it ends like speech (. ? !). Check tags in brackets are fine; a built
                 reply with an entry in RZ_TalkWords is said in its words
    split        a talk line (a code line, or a conversation node's says together) is 420
                 characters or fewer: longer, split it into nodes ("Go on.")
    ellipsis     no line that's only an ellipsis
    bang         '!' only in Red Hand lines, notice titles and loudspeakers (alarms)
    format       %1-%9 only (no %0, %10, %s, %d)
    token        every {token} is a world token (RZ_WorldFacts.Fill) or a questline text key
                 (some run.SetText("key", ...) in the scripts)
    never        the never list: sexual violence, slurs, real countries and pandemics, internet talk
    long-table   no ambient pool hints at the Long Table (meat, pork, "don't ask what it was"...)
    infected     no "alpha", "tribe" or "crawler" in player text; "strays" never for the infected
    pronoun      no he/she/him/her/his after a seeded person's token ({keeper}, {leader:id}...):
                 the seed stores no gender
    info-token   information lines (truck times, shifts, tolls, the lab's count) keep a token
    clue-echo    an ambient line never repeats a clue's text
    seed-name    seeded name lists never use a name LORE.md gives a person
    punchline    a short last sentence ("Habit.") is used by one line only
    objective    an objective or step line (the tracker, the Quests tab, the map) stands on its own:
                 no he/she/his/her/him/they/them/their without a person named in the same line (a
                 capitalised name, a person's {token} or a %1), and three words or more;
                 a stage title is never only a pronoun and a noun ("His number"); every mission
                 that sets a brief names its objective (m_sObjective) in the same class

The baseline (tools/voice_lint_baseline.txt) holds today's findings as check, file and a hash of
the literal, so the lint fails only on new or changed text. Fixing a line drops it from the
baseline at the next --update-baseline; never add to the baseline by hand to get a new line through.
No dependencies beyond Python 3.
"""
import argparse
import hashlib
import os
import re
import sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import strings_inventory as si  # noqa: E402

BASELINE = os.path.join(si.ROOT, "tools", "voice_lint_baseline.txt")
LORE = os.path.join(si.ROOT, "docs", "LORE.md")

SHORT_CHANNELS = {"radio", "hint", "leader", "rumour", "headline", "broadcast", "loudspeaker", "trace", "door"}
SHORT_LIMIT = 200
SPLIT_LIMIT = 420

# Spoken channels: no quotation marks around or in them (docs/WRITING.md, "Talk")
SPOKEN_CHANNELS = {"speech", "radio", "loudspeaker", "broadcast", "leader", "ambient", "reply"}
QUOTE_RE = re.compile("[\"\u201c\u201d]")
# "The guard: ", "Sgt Hale: ", "Ines: " at the start of a talk line (a role line in a scene is the engine's)
# A voice's own heading words (Bastion's "Report: ...", Halcyon's "Subject: ...") are speech, not a label
VOICE_HEADINGS = {"Report", "Entry", "Subject", "Status", "Note", "Update", "Item", "Correction"}
SPEAKER_RE = re.compile(r"^((The|A|An) [a-z]+( [a-z]+)?|(Sgt|Cpl|Pte|Dr|Mr|Mrs|Ms|Old|Corporal|Sergeant|Private|Captain|Lieutenant) [A-Z][a-z]+|[A-Z][a-z]+( [A-Z][a-z]+)?)\s?: ")
ELLIPSIS_RE = re.compile(r"^[\s.\u2026()]*$")
# Replies (docs/WRITING.md, "Replies"): a menu label's words
LABEL_WORDS = {"Ask", "Tell", "Check", "Trade", "Leave", "Talk", "Show", "Request", "Buy", "Sell", "Hand",
               "Listen", "Read", "Rob", "Share", "Offer", "Accept", "Decline", "Refuse", "Report", "Inquire",
               "Greet", "Persuade", "Threaten", "Bribe", "Give", "Pay", "Inspect", "Examine", "Investigate",
               "Mention", "Discuss", "Explain", "Request", "Continue", "Goodbye", "Bye", "Back", "Exit",
               "Cancel", "Return", "Something", "Done", "Next", "Agree", "Leave", "Volunteer", "Deliver"}
# ...unless the next word makes it speech to the listener ("Show me.", "Tell me about Rudi.", "Leave it.")
SPEECH_SECOND = {"me", "us", "you", "it", "that", "this", "up", "over", "him", "her", "them", "anything", "nothing"}
SHORT_LABELS = {"Leave", "Goodbye", "Bye", "Back", "Trade", "Talk", "Something", "Continue", "Next", "Done",
                "Accept", "Decline", "Refuse", "Agree", "Exit", "Cancel", "Return", "Ask", "Tell", "Check"}
TAGS_HEAD_RE = re.compile(r"^(\[[^\]]*\]\s*)+")
TAGS_TAIL_RE = re.compile(r"(\s*(\[[^\]]*\]|\([^)]*\)))+$")
SPEECH_END_RE = re.compile("[.?!\u2026]['\u2019]?$")
TALKWORDS = os.path.join(si.ROOT, "scripts", "Game", "ReforgerZ", "Dialogue", "RZ_TalkWords.c")
# Built labels RZ_TalkWords says in other words by their start ("Rob <name>")
TALKWORDS_PREFIXES = ("Rob ", "Ask about ", "Send them to ")
BRIEF_WARN = 500
LABEL_ACTION_LIMIT = 40
LABEL_LIMIT = 64

# Ambient pools: lines anyone can hear anywhere, not a mission's or questline's own text.
AMBIENT_CHANNELS = {"radio", "rumour", "headline", "broadcast", "loudspeaker", "leader", "hint",
                    "ambient", "moment", "trace", "door"}

# Pools where a short last line is a punchline, and the formulae that aren't one.
PUNCHLINE_CHANNELS = {"radio", "rumour", "leader", "hint", "ambient", "speech", "loudspeaker", "trace", "door"}
FORMULA_RE = re.compile(r"(\w+ out|over|out|thank you|thanks|please|go on|good)[.!]?", re.I)

# World tokens RZ_WorldFacts.Fill knows, and the kinds that take an argument ({town:N}).
WORLD_TOKENS = {"day", "day_text", "days_since_command", "days_since_command_words", "cure_state",
                "cure_percent", "stage", "stage_tone", "time_of_day", "lab_town", "stolen_doses"}
WORLD_KINDS = {"town", "owner", "flag", "canon", "canon_town", "leader", "leader_short", "leader_place",
               "tribe", "alpha", "young"}
# Tokens a conversation (.rzd) line may use (RZ_DialogueText.Fill, RZ_RepVoice.FillFor), and the voice
# bank's context token {place} (RZ_VoiceBank.PickFor).
DIALOGUE_TOKENS = {"npc", "npc_short", "player", "house", "resident", "word", "rep", "best", "worst", "kills",
                   "jobs", "picture", "name", "crew", "place", "here", "faction", "role", "toll", "seat", "boss",
                   "kit_owner", "dead_name", "cover", "pack", "pack_leader", "distress"}
VOICE_TOKENS = {"place"}
# The voice bank's openers are filled for the listener too (RZ_RepVoice.Opening: reputation's FillFor)
VOICE_LISTENER_TOKENS = {"word", "best", "worst", "crew", "rep", "kills", "jobs", "name"}
# A conversation's tokens with an argument: a stranger's own words, the season's title with a faction
DIALOGUE_KINDS = {"their", "title", "wire", "cover", "decider", "lean", "tier", "rep", "word", "claim", "bind"}

NEVER = [
    # Sexual violence is never mentioned
    r"rape[ds]?", r"raping", r"rapist", r"molest\w*", r"sexual(ly)? assault\w*",
    # Slurs and ableist insults (the author extends this list)
    r"retard(ed|s)?", r"spaz", r"spastic", r"cripple[sd]?", r"midget", r"tranny", r"gypp?ed",
    # Real countries, governments and pandemics
    r"covid(-19)?", r"coronavirus", r"ebola", r"spanish flu", r"nato", r"ussr", r"soviets?",
    r"americans?", r"russians?", r"ukrain\w*", r"chinese", r"united states", r"united nations",
    r"european union", r"democrats?", r"republicans?", r"kremlin", r"white house",
    # Internet talk, memes, fourth wall
    r"lol", r"lmao", r"rofl", r"omg", r"wtf", r"memes?", r"noobs?", r"n00bs?", r"pwn(ed)?", r"yolo",
    r"bruh", r"git gud", r"epic fail", r"cringe", r"vibes?", r"selfies?", r"hashtags?", r"emojis?",
    r"kill yourself", r"kys",
]
NEVER_RE = re.compile(r"\b(" + "|".join(NEVER) + r")\b", re.I)

LONG_TABLE = [r"meat", r"pork", r"long pig", r"don'?t ask what", r"place set", r"set a place",
              r"extra place", r"an extra plate", r"mystery stew", r"who'?s for dinner"]
LONG_TABLE_RE = re.compile(r"\b(" + "|".join(LONG_TABLE) + r")\b", re.I)

INFECTED_RE = re.compile(r"\b(alphas?|tribes?|tribal|crawlers?)\b", re.I)
STRAYS_RE = re.compile(r"\bstrays\b", re.I)
INFECTED_CONTEXT_RE = re.compile(r"\b(infected|horde|bite|bitten|the dead|zombies?|shamblers?|groan\w*)\b", re.I)

PRONOUN_RE = re.compile(r"\b(he|she|him|her|his|hers|himself|herself|he's|she's|he'd|she'd|he'll|she'll)\b", re.I)
# Questline tokens that are places, things or gendered roles, not seeded people.
NOT_PERSON_TOKEN = re.compile(r"(place|town|house|grid|village|post|farm|barn|store|lab|clinic|pit|drop|"
                              r"refuge|stronghold|keys|why|lost|count|number|from|far|to|day|fizzle|"
                              r"relay|site|dir|wife|husband|brother|sister|mother|father|son|daughter|"
                              r"boy|girl|man|woman|crew$|family|camp|road|bridge|time|what|item|thing)", re.I)

# Functions whose lines carry information: their information clause must keep a token.
INFO_FUNCTIONS = re.compile(r"^(ChatterTruck|ChatterTruckOnRoad|ChatterShift|ChatterToll|ChatterSamples|"
                            r"SecureTruck|SecureDeparture|SecurePatrol|SecureSamples|SecureShort)$")

# Objective lines (docs/WRITING.md, "Objective lines"): a pronoun needs a name before it in the same line
OBJ_PRONOUN_RE = re.compile(r"\b(he|she|him|her|his|hers|himself|herself|they|them|their|theirs|themselves|"
                            r"he's|she's|he'd|she'd|he'll|she'll|they're|they've|they'll|they'd)\b", re.I)
# What a pronoun can point back to: a {token}, a %1, or a capitalised word that doesn't start a sentence
OBJ_ANTECEDENT_RE = re.compile(r"\{[a-z_]+(?::[^{}\s]+)?\}|%[1-9]|(?<=[^.!?]\s)[A-Z][a-z]+")
# Capitalised words that are never a person
OBJ_NOT_NAMES = {"Haven", "Bastion", "Red", "Hand", "Halcyon", "Long", "Table", "Zone", "Day", "Command",
                 "Volunteer", "Institute", "Optional", "Failed", "Search", "Reached", "Go", "Get", "Wait",
                 "The", "A", "An", "Grid", "North", "South", "East", "West", "Morse", "Final", "Twice"}
OBJ_MIN_WORDS = 3
STAGE_CALLS = ("WaitStage", "MissionStage")
# A stage title made of a pronoun and a noun or two ("His number", "Find him")
STAGE_PRONOUN_RE = re.compile(r"\b(he|she|him|her|his|they|them|their)\b", re.I)
# A brief assigned to a mission (self-modifying ones, `x = f(x)`, decorate a brief and are skipped)
OBJ_PLACE_TOKEN = re.compile(r"(place|town|house|grid|village|post|farm|barn|store|lab|clinic|pit|drop|"
                             r"stronghold|site|crossroads|road|camp|where|lies|here|from|to|far)$")
BRIEF_ASSIGN_RE = re.compile(r"^\s*(?:(\w+)\.)?m_sBrief\s*=\s*(.*)$")
CLASS_RE = re.compile(r"^class\s+(\w+)")

TOKEN_RE = re.compile(r"\{([a-z_]+)(?::([^{}\s]+))?\}")
SENTENCE_RE = re.compile(r"[^.?!…]+[.?!…]+[\"']?")


def h(text):
    return hashlib.sha1(text.encode("utf-8")).hexdigest()[:12]


def excerpt(text, n=70):
    t = text.replace("\n", " ").replace("\t", " ")
    return t if len(t) <= n else t[:n - 3] + "..."


class Finding:
    def __init__(self, check, lit, message, warning=False):
        self.check = check
        self.lit = lit
        self.message = message
        self.warning = warning

    def key(self):
        return "%s\t%s\t%s" % (self.check, self.lit.file, h(self.lit.text))

    def __str__(self):
        return "%s:%d: [%s] %s\n    %s" % (self.lit.file, self.lit.line, self.check, self.message, excerpt(self.lit.text, 110))


def questline_keys(all_lits):
    """Questline text keys: run.SetText("key", ...) anywhere, plus every bare key a questline file
    passes to a helper that sets it (KeepLeader(run, "head", ...) sets {head}, {head_short}...)."""
    keys = set()
    suffixes = {""}
    for lit in all_lits:
        if lit.calls and lit.calls[0][0] in ("SetText", "Text") and lit.calls[0][1] == 0:
            keys.add(lit.text)
            if lit.text.startswith("_"):
                suffixes.add(lit.text)
    for lit in all_lits:
        if "/Questlines/" in lit.file and re.fullmatch(r"[a-z][a-z0-9_]*", lit.text):
            for suffix in suffixes:
                keys.add(lit.text + suffix)
    return keys


def seeded_people(all_lits):
    """Single-word literals in name lists: names = {"Marta", ...}."""
    seeds = []
    for lit in all_lits:
        if re.fullmatch(r"[A-Z][a-z]+", lit.text) and re.search(r"name|nick|first|surname", lit.target or "", re.I):
            seeds.append(lit)
    return seeds


def lore_names():
    """Capitalised words in LORE.md that never appear in lower case there: its people and places."""
    try:
        with open(LORE, encoding="utf-8") as f:
            text = f.read()
    except OSError:
        return set()
    caps = set(re.findall(r"\b[A-Z][a-z]+\b", text))
    lower = set(w.lower() for w in re.findall(r"\b[a-z]+\b", text))
    return {w for w in caps if w.lower() not in lower}


def talkwords_labels():
    """The built labels RZ_TalkWords turns into speech (its case labels)."""
    try:
        with open(TALKWORDS, encoding="utf-8") as f:
            return set(re.findall(r'case "([^"]+)":', f.read()))
    except OSError:
        return set()


def reply_problem(text, mapped):
    """Why a reply reads as a menu label, or None."""
    t = text.strip()
    if t.startswith("[Admin]") or not t:
        return None
    t = TAGS_HEAD_RE.sub("", QUOTE_RE.sub("", t)).strip()
    t = TAGS_TAIL_RE.sub("", t).strip()
    if not t or t in mapped or t.startswith(TALKWORDS_PREFIXES):
        return None
    words = re.findall(r"[A-Za-z']+", TOKEN_RE.sub("x", t))
    if not words:
        return None
    first = words[0]
    if len(words) <= 2 and first in SHORT_LABELS:
        return "'%s' is a menu label, not something you'd say" % excerpt(t, 40)
    if first in LABEL_WORDS and (len(words) < 2 or words[1].lower() not in SPEECH_SECOND):
        return "starts like a menu label ('%s ...'): say it as the words you'd use" % first
    if not SPEECH_END_RE.search(t.rstrip("\"")) and not text.endswith(" "):
        return "doesn't end like speech (. ? !)"
    return None


def node_lengths(lits):
    """A conversation node's says together, as the screen shows them: a say? variant stands in for
    the next plain say, so the longest of them counts. {(file, node): (first lit, characters)}"""
    nodes = {}
    pending = {}
    for lit in lits:
        node = getattr(lit, "node", "")
        if lit.channel != "speech" or not node:
            continue
        key = (lit.file, node)
        first, total = nodes.get(key, (lit, 0))
        n = len(lit.text)
        if lit.cond:
            pending[key] = max(pending.get(key, 0), n)
            nodes[key] = (first, total)
            continue
        total += max(n, pending.pop(key, 0))
        nodes[key] = (first, total)
    for key, n in pending.items():
        first, total = nodes[key]
        nodes[key] = (first, total + n)
    return nodes


def label_parts(text):
    return [p.strip() for p in text.split("|") if p.strip()]


def last_sentence(text):
    t = text.strip().strip("\"'").strip()
    sentences = SENTENCE_RE.findall(t)
    if not sentences:
        return ""
    return sentences[-1].strip().strip("\"'").strip()


def obj_names(text):
    """What a pronoun in an objective line can point back to: person-like {tokens}, %1s and
    capitalised names (not the line's first word, not a faction or a place word)."""
    names = []
    for m in OBJ_ANTECEDENT_RE.finditer(text):
        a = m.group(0)
        if a.startswith("{"):
            name = TOKEN_RE.match(a).group(1)
            if name in WORLD_TOKENS or OBJ_PLACE_TOKEN.search(name) or name in ("leader_place", "lab_town"):
                continue
        elif a in OBJ_NOT_NAMES:
            continue
        names.append(a)
    return names


def objective_problem(text):
    """Why an objective line doesn't stand on its own (docs/WRITING.md, "Objective lines"), or None."""
    t = text
    if not t.strip():
        return None
    m = OBJ_PRONOUN_RE.search(t)
    if m and not obj_names(t):
        return "'%s' with no named person in the line: say who" % m.group(0)
    # A piece of a built line ("Get to " + place) isn't counted
    if t == t.strip():
        words = re.findall(r"\{[^}]*\}|%[1-9]|[A-Za-z0-9'’-]+", t)
        if len(words) < OBJ_MIN_WORDS:
            return "objective of %d word(s): say who, what and where" % len(words)
    return None


def stage_title_problem(lit):
    """A stage title that leans on a pronoun ("His number", "Find him"): it shows as a done objective."""
    eff = si.effective_call(lit.calls) if lit.calls else None
    if not eff or eff[0] not in STAGE_CALLS or eff[1] != 1:
        return None
    m = STAGE_PRONOUN_RE.search(lit.text)
    if m and not obj_names(lit.text[:m.start()]):
        return "stage title '%s' leans on '%s': name the person" % (lit.text, m.group(0))
    return None


def missing_objectives(root=si.ROOT):
    """Every class that sets a mission's brief (m_sBrief = ..., or scene.m_sBrief = ...) also names its
    objective (the same receiver's m_sObjective) in that class. A brief that decorates another
    (`m_sBrief = x + m_sBrief`) is skipped."""
    findings = []
    base = os.path.join(root, si.SCRIPTS)
    for dirpath, _, files in os.walk(base):
        if os.path.relpath(dirpath, root) == si.PACK_DIR:
            continue
        for name in sorted(files):
            if not name.endswith(".c"):
                continue
            path = os.path.join(dirpath, name)
            rel = os.path.relpath(path, root).replace(os.sep, "/")
            with open(path, encoding="utf-8", errors="replace") as f:
                lines = f.read().split("\n")
            cls, cls_line = "", 0
            briefs = {}      # receiver -> first line
            objectives = set()

            def flush():
                for recv, line in briefs.items():
                    if recv not in objectives:
                        lit = si.Literal()
                        lit.file, lit.line, lit.text = rel, line, cls + ":" + (recv or "this")
                        lit.calls, lit.target, lit.returned, lit.function = [], "", False, ""
                        lit.faction, lit.channel = "-", "objective"
                        who = (recv + ".") if recv else ""
                        findings.append(Finding("objective", lit, "%s sets %sm_sBrief but no %sm_sObjective: name the "
                                                "objective in one line (who, what, where)" % (cls, who, who)))

            for i, line in enumerate(lines):
                m = CLASS_RE.match(line)
                if m:
                    flush()
                    cls, cls_line = m.group(1), i + 1
                    briefs, objectives = {}, set()
                    continue
                if line.lstrip().startswith("//"):
                    continue
                m = BRIEF_ASSIGN_RE.match(line)
                if m:
                    recv = m.group(1) or ""
                    rhs = m.group(2)
                    if "m_sBrief" not in rhs:
                        briefs.setdefault(recv, i + 1)
                    continue
                m = re.match(r"^\s*(?:(\w+)\.)?m_sObjective\s*=", line)
                if m:
                    objectives.add(m.group(1) or "")
                # ObjThen(first, then): the objective in two lines (RZ_Mission.ObjThen)
                elif re.match(r"^\s*ObjThen\(", line):
                    objectives.add("")
            flush()
    return findings


def check(lits, all_lits):
    findings = []
    known_keys = questline_keys(all_lits)

    clue_texts = {}
    for lit in lits:
        if lit.channel == "clue" and len(lit.text) >= 30:
            clue_texts.setdefault(lit.text.strip().lower(), lit)

    punchlines = defaultdict(set)
    mapped = talkwords_labels()

    for lit in lits:
        t = lit.text
        ch = lit.channel
        rzd = lit.file.endswith(".rzd")

        # quote: spoken lines carry no quotation marks
        if (ch in SPOKEN_CHANNELS or (rzd and ch == "choice")) and QUOTE_RE.search(t):
            findings.append(Finding("quote", lit, "quotation marks in a spoken line (it's speech already; a quote inside takes single quotes)"))

        # speaker: no speaker's label in front of a talk line
        if ch == "speech" and not getattr(lit, "role", False) and SPEAKER_RE.match(t) and not t.startswith("[") \
                and t.split(":")[0] not in VOICE_HEADINGS:
            findings.append(Finding("speaker", lit, "a speaker's label in front of the line (the name shows on screen)"))

        # reply: what you say, not a menu label
        if ch == "reply" or (rzd and ch == "choice"):
            why = reply_problem(t, mapped)
            if why:
                findings.append(Finding("reply", lit, why))

        # split: a talk line over 420 (a conversation node: below, all its says together)
        if ch == "speech" and not rzd and len(t) > SPLIT_LIMIT:
            findings.append(Finding("split", lit, "talk line of %d characters (over %d): split it" % (len(t), SPLIT_LIMIT)))

        # ellipsis: never a line that's only "..."
        if ch in SPOKEN_CHANNELS and t.strip() and ELLIPSIS_RE.match(QUOTE_RE.sub("", t)):
            findings.append(Finding("ellipsis", lit, "a line that's only an ellipsis"))

        # length
        if ch in SHORT_CHANNELS and len(t) > SHORT_LIMIT:
            findings.append(Finding("length", lit, "%s line of %d characters (limit %d)" % (ch, len(t), SHORT_LIMIT)))
        if ch == "brief" and len(t) > BRIEF_WARN:
            findings.append(Finding("length", lit, "brief of %d characters (over %d)" % (len(t), BRIEF_WARN), warning=True))
        if ch in ("label", "choice"):
            for part in label_parts(t):
                action = re.split(r"\s[\(\[]", part, maxsplit=1)[0]
                if len(action) > LABEL_ACTION_LIMIT or len(part) > LABEL_LIMIT:
                    findings.append(Finding("length", lit, "label '%s' (action %d, whole %d; limits %d and %d)"
                                            % (excerpt(part, 50), len(action), len(part), LABEL_ACTION_LIMIT, LABEL_LIMIT)))
                    break

        # bang
        if "!" in t and lit.faction != "Red Hand" and ch not in ("title", "loudspeaker"):
            findings.append(Finding("bang", lit, "'!' outside a Red Hand line or an alarm"))

        # format
        bad = re.findall(r"%(?:0|\d{2,}|[sdifxu]\b)", t)
        if bad:
            findings.append(Finding("format", lit, "format argument %s (only %%1-%%9)" % ", ".join(sorted(set(bad)))))

        # token
        for m in TOKEN_RE.finditer(t):
            name, arg = m.group(1), m.group(2)
            if arg is not None:
                ok = name in WORLD_KINDS or (lit.file.endswith(".rzd") and name in DIALOGUE_KINDS)
            else:
                ok = name in WORLD_TOKENS or name in known_keys
                if lit.file.endswith(".rzd"):
                    ok = ok or name in DIALOGUE_TOKENS
                elif lit.file.endswith("RZ_VoiceDefaults.c"):
                    ok = ok or name in VOICE_TOKENS or name in VOICE_LISTENER_TOKENS
            if not ok:
                findings.append(Finding("token", lit, "unknown token {%s}" % m.group(0)[1:-1]))
                break

        # never
        m = NEVER_RE.search(t)
        if m:
            findings.append(Finding("never", lit, "never-list word '%s'" % m.group(0)))

        # long-table
        if ch in AMBIENT_CHANNELS and lit.faction != "Long Table" and "/LongTable/" not in lit.file:
            m = LONG_TABLE_RE.search(t)
            if m:
                findings.append(Finding("long-table", lit, "ambient line hints at the Long Table ('%s')" % m.group(0)))

        # infected
        m = INFECTED_RE.search(TOKEN_RE.sub("", t))  # {tribe:N} and {alpha:N} are tokens, not words
        if m:
            findings.append(Finding("infected", lit, "'%s' in player text" % m.group(0)))
        elif STRAYS_RE.search(t) and INFECTED_CONTEXT_RE.search(t):
            findings.append(Finding("infected", lit, "'strays' for the infected"))

        # pronoun
        for sentence in re.split(r"(?<=[.?!])\s+", t):
            person = None
            for m in TOKEN_RE.finditer(sentence):
                name = m.group(1)
                if name in ("leader", "leader_short", "npc", "npc_short", "alpha", "young") or (m.group(2) is None and name not in WORLD_TOKENS
                                                           and name not in DIALOGUE_TOKENS
                                                           and not NOT_PERSON_TOKEN.search(name)):
                    person = m
                    break
            if person and PRONOUN_RE.search(sentence[person.end():]):
                findings.append(Finding("pronoun", lit, "a pronoun after {%s}" % person.group(0)[1:-1]))
                break

        # objective: a tracker line stands on its own; a stage title never leans on a pronoun
        if ch == "objective":
            why = objective_problem(t)
            if why:
                findings.append(Finding("objective", lit, why))
        else:
            why = stage_title_problem(lit)
            if why:
                findings.append(Finding("objective", lit, why))

        # info-token
        if INFO_FUNCTIONS.match(lit.function) and si.is_prose(t):
            if not re.search(r"%[1-9]", t) and not TOKEN_RE.search(t):
                findings.append(Finding("info-token", lit, "information line in %s() without a token" % lit.function))

        # clue-echo
        if ch in AMBIENT_CHANNELS:
            echo = clue_texts.get(t.strip().lower())
            if echo:
                findings.append(Finding("clue-echo", lit, "repeats the clue at %s:%d" % (echo.file, echo.line)))

        # punchline (collected; checked below)
        if ch in PUNCHLINE_CHANNELS:
            last = last_sentence(t)
            words = last.split()
            if (1 <= len(words) <= 3 and len(SENTENCE_RE.findall(t.strip().strip("\"'"))) >= 2
                    and not re.search(r"[%{0-9]", last) and not FORMULA_RE.fullmatch(last)):
                punchlines[last.lower()].add(t)

    # split: a conversation node's says together
    for (path, node), (first, total) in node_lengths(lits).items():
        if total > SPLIT_LIMIT:
            findings.append(Finding("split", first, "node %s says %d characters (over %d): split it into nodes" % (node, total, SPLIT_LIMIT)))

    # seed-name
    names = lore_names()
    for lit in seeded_people(all_lits):
        if lit.text in names:
            findings.append(Finding("seed-name", lit, "seeded name '%s' is a person in LORE.md" % lit.text))

    # punchline
    owners = defaultdict(list)
    for lit in lits:
        last = last_sentence(lit.text).lower()
        if last in punchlines and len(punchlines[last]) > 1 and lit.text in punchlines[last]:
            owners[last].append(lit)
    for last, group in owners.items():
        for lit in group:
            findings.append(Finding("punchline", lit, "punchline '%s' also ends %d other line(s)"
                                    % (last, len(punchlines[last]) - 1)))

    return findings


def read_baseline():
    keys = set()
    if not os.path.exists(BASELINE):
        return keys
    with open(BASELINE, encoding="utf-8") as f:
        for line in f:
            line = line.split("#", 1)[0].rstrip()
            if line:
                keys.add(line)
    return keys


def write_baseline(findings):
    rows = {}
    for f in findings:
        if not f.warning:
            rows.setdefault(f.key(), excerpt(f.lit.text, 60))
    with open(BASELINE, "w", encoding="utf-8", newline="\n") as out:
        out.write("# tools/voice_lint.py baseline: today's findings (check, file, hash of the literal).\n")
        out.write("# The lint fails only on findings not listed here. Each writing phase shrinks it;\n")
        out.write("# regenerate with: python3 tools/voice_lint.py --update-baseline\n")
        for key in sorted(rows):
            out.write("%s  # %s\n" % (key, rows[key].replace("#", "")))
    return len(rows)


SELF_TEST = [
    ("length", "radio", "-", "ChatterShort", "\"" + "Shift change at the post. " * 9 + "\""),
    ("bang", "rumour", "Haven", "Rumour", "I heard Bastion left Levie in a hurry!"),
    ("format", "notice", "-", "", "You have %10 minutes."),
    ("token", "rumour", "-", "Rumour", "Someone saw {ghost_of_nobody} at the well."),
    ("never", "rumour", "-", "Rumour", "Bastion went full noob at the bridge, lol."),
    ("long-table", "ambient", "Haven", "PersonLine", "\"Supper's stew tonight. Don't ask what it was.\""),
    ("infected", "rumour", "-", "Rumour", "A tribe of them came down from the quarry at dusk."),
    ("pronoun", "journal", "Red Hand", "", "{keeper} said he would pay by Sunday."),
    ("info-token", "radio", "Bastion", "ChatterTruck", "\"Logistics, Command. The truck leaves soon.\""),
    ("label", "label", "-", "", "Persuade the quartermaster to open the stores for everyone in town"),
    ("quote", "speech", "Haven", "", "\"We're managing. Don't tell anyone I said that.\""),
    ("speaker", "speech", "Bastion", "", "The guard: Papers. Now."),
    ("reply", "reply", "-", "", "Request a lift to the lab"),
    ("reply", "reply", "-", "", "Something else."),
    ("split", "speech", "-", "", "We buried him by the wall. " * 16),
    ("ellipsis", "speech", "-", "", "..."),
    ("objective", "objective", "Halcyon", "", "Find out his number."),
    ("objective", "objective", "Haven", "", "Go back to her."),
    ("objective", "objective", "-", "", "Burn it."),
]


def self_test(all_lits):
    failures = 0
    for i, (want, channel, faction, function, text) in enumerate(SELF_TEST):
        lit = si.Literal()
        lit.file, lit.line, lit.text = "self-test", i + 1, text
        lit.calls, lit.target, lit.returned = [], "", False
        lit.node, lit.cond, lit.role = "", "", False
        lit.function, lit.faction, lit.channel = function, faction, channel
        lit.register, lit.repeats = "PLAIN", 1
        got = {f.check for f in check([lit], all_lits) if not f.warning}
        want_check = "length" if want == "label" else want
        status = "ok" if want_check in got else "FAILED"
        if status != "ok":
            failures += 1
        print("%-6s %-11s %s" % (status, want, excerpt(text, 60)))
    print("self-test: %d of %d planted lines caught" % (len(SELF_TEST) - failures, len(SELF_TEST)))
    return 1 if failures else 0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--all", action="store_true", help="also list the findings the baseline holds")
    ap.add_argument("--warnings", action="store_true", help="also list warnings")
    ap.add_argument("--update-baseline", action="store_true", help="write today's findings to the baseline")
    ap.add_argument("--self-test", action="store_true", help="check that planted bad lines fail")
    args = ap.parse_args()

    lits, all_lits = si.inventory()
    if args.self_test:
        sys.exit(self_test(all_lits))

    findings = check(lits, all_lits) + missing_objectives()
    if args.update_baseline:
        n = write_baseline(findings)
        print("voice_lint: wrote %d findings to %s" % (n, os.path.relpath(BASELINE, si.ROOT)))
        return

    baseline = read_baseline()
    errors = [f for f in findings if not f.warning]
    warnings = [f for f in findings if f.warning]
    new = [f for f in errors if f.key() not in baseline]
    seen = {f.key() for f in errors}
    stale = [k for k in baseline if k not in seen]

    shown = errors if args.all else new
    for f in sorted(shown, key=lambda f: (f.lit.file, f.lit.line)):
        print(("" if f.key() not in baseline else "(baseline) ") + str(f))
    if args.warnings:
        for f in warnings:
            print("warning: " + str(f))

    by_check = defaultdict(int)
    for f in new:
        by_check[f.check] += 1
    print("voice_lint: %d literals, %d findings (%d in the baseline, %d new), %d warnings"
          % (len(lits), len(errors), len(errors) - len(new), len(new), len(warnings)))
    if stale:
        print("voice_lint: %d baseline entries are fixed or gone; run --update-baseline to drop them" % len(stale))
    if new:
        print("voice_lint: FAILED (" + ", ".join("%s %d" % kv for kv in sorted(by_check.items())) + ")")
        sys.exit(1)
    print("voice_lint: OK")


if __name__ == "__main__":
    main()
