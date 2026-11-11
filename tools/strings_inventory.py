#!/usr/bin/env python3
"""ReforgerZ strings inventory (docs/WORKBENCH_SETUP.md, "Text lint").

    python3 tools/strings_inventory.py                  # every player-facing literal, as a table
    python3 tools/strings_inventory.py --summary        # counts by channel and faction
    python3 tools/strings_inventory.py --channel radio  # one channel
    python3 tools/strings_inventory.py --json           # the same, as JSON (for other tools)

Reads the game scripts (scripts/Game/ReforgerZ/**/*.c) and lists every string literal that can
reach a player: notices (NotifyPlayer, NotifyAll, NotifyNear), headlines, rumours, Changed(),
mission briefs, clue and note texts, people's labels and choices, the guidance tips, and the
voice pools (anything that reads as prose). For each it gives:

    file:line   where it is
    channel     guessed from the call, the assignment or the function (see CHANNELS)
    faction     guessed from the folder, then from the nearest `case RZ_EFaction.X:`
    register    a guess (PLAIN, WRY, JOKE, TAUNT, BLEAK, SILENCE); the author confirms it
                only for the pools being moved into the voice bank
    repeats     how many times the same literal appears in the tree
    text        the literal, unescaped

Every guess is a guess: a literal built from pieces ("Talk to " + name) is listed piece by piece.
tools/voice_lint.py reads this inventory. No dependencies beyond Python 3.
"""
import argparse
import json
import os
import re
import sys
from collections import Counter

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SCRIPTS = os.path.join("scripts", "Game", "ReforgerZ")

# Every channel the inventory can guess. The lint's limits are per channel.
CHANNELS = {
    "notice":      "a notice card's text (NotifyPlayer, NotifyAll, NotifyNear)",
    "title":       "a notice card's or a clue's title",
    "headline":    "a headline on the roads",
    "rumour":      "a rumour or road news",
    "moment":      "a remembered moment (Changed)",
    "brief":       "a mission brief",
    "label":       "an action label on a person or a thing ('|' separates several)",
    "choice":      "one choice of a person's '|'-separated choices",
    "clue":        "a clue's or a found note's text",
    "outcome":     "what happened when a mission or stage ends",
    "journal":     "a line in the journal's \"what you know so far\"",
    "radio":       "an open or secure channel line",
    "broadcast":   "the government broadcast",
    "loudspeaker": "a loudspeaker or a shout at the line",
    "leader":      "a leader's opener, closer or remark",
    "hint":        "a guidance tip (voiced or plain)",
    "speech":      "a line on the talk screen (Say)",
    "ambient":     "a line from the voice pools (people, routines, life events)",
    "setting":     "an attribute's default text",
    "text":        "prose whose channel couldn't be guessed",
}

FACTION_ENUM = {
    "SURVIVORS": "Haven",
    "MILITARY": "Bastion",
    "BANDITS": "Red Hand",
    "INSTITUTE": "Halcyon",
}

FACTION_FOLDERS = [
    ("Missions/Questlines/LongTable/", "Long Table"),
    ("World/RZ_LongTable.c", "Long Table"),
    ("/Haven/", "Haven"),
    ("/Bastion/", "Bastion"),
    ("/RedHand/", "Red Hand"),
    ("/Halcyon/", "Halcyon"),
    ("RZ_LeaderJobsHaven.c", "Haven"),
    ("RZ_LeaderJobsBastion.c", "Bastion"),
    ("RZ_LeaderJobsRedHand.c", "Red Hand"),
    ("RZ_LeaderJobsHalcyon.c", "Halcyon"),
    ("Director/RZ_Broadcast.c", "broadcast"),
    ("World/RZ_Perimeter.c", "broadcast"),
]

# Calls whose arguments never reach a player.
SKIP_CALLS = {
    "Print", "PrintFormat", "Info", "Warn", "Error", "FindAnyWidget", "FindWidget", "Load",
    "SetFlag", "Flag", "GetFlag", "AddFlag", "HasFlag", "QText", "Text", "HasCanon", "CanonText",
    "CanonPick", "CanonSettlement", "SetCanon", "Contains", "IndexOf", "StartsWith", "EndsWith",
    "Split", "GetPrefab", "SpawnPrefab", "CreateWidgets", "Find", "Remove", "Get", "Set", "Insert_Key",
    "Hash", "Seed", "ScriptCallQueue", "CallLater", "Rpc", "RplProp", "BaseContainerProps",
    "SCR_BaseContainerCustomTitleField", "Obsolete", "PosOf", "SetPlace",
}

# Wrappers: a literal inside one counts for the call around it.
WRAP_CALLS = {"Format", "Fill", "QFill", "Pick", "Insert", "ToUpper", "ToLower", "Trim", "string"}

CALL_CHANNELS = {
    "NotifyPlayer": {1: "title", 2: "notice"},
    "NotifyAll": {0: "title", 1: "notice"},
    "NotifyNear": None,          # title then text: the last two arguments
    "Headline": {"*": "headline"},
    "SpreadRumour": {"*": "rumour"},
    "AddRumour": {"*": "rumour"},
    "Rumour": {"*": "rumour"},
    "Changed": {1: "headline", 2: "rumour", 3: "moment"},
    "AddClue": {0: "title", 1: "clue"},
    "AddNote": {0: "title", 1: "clue"},
    "SpawnPerson": {4: "label", 5: "choice"},
    "SpawnMember": {5: "choice"},
    "SpawnObject": {4: "label", 5: "choice", 7: "label"},
    "SpawnClue": {4: "label"},
    "SetTalk": {2: "choice", 4: "label"},
    "StartTask": {6: "label"},
    "SetActionName": {"*": "label"},
    "Say": {3: "speech"},
    "ReplaceLeader": {2: "headline"},
    "AddHeadline": {0: "headline"},
    "NotifyParticipants": {1: "title", 2: "notice"},
    "NotifyInterested": {1: "title", 2: "notice"},
    "NotifyKnown": {1: "title", 2: "notice"},
    "NotifyPlayerAs": {1: "title", 2: "notice"},
    "WaitStage": {1: "title", 2: "journal"},
    "Note": {"*": "journal"},
    "GiveUp": {1: "journal"},
    "Reward": {"*": "outcome"},
}

ASSIGN_CHANNELS = {
    "m_sBrief": "brief",
    "m_sTitle": "title",
    "m_sName": "title",
    "m_sLabel": "label",
    "m_sChoices": "choice",
    "m_sHaven": "hint", "m_sBastion": "hint", "m_sRedHand": "hint", "m_sHalcyon": "hint",
    "m_sWanderer": "hint", "m_sRadio": "hint", "m_sPlain": "hint", "m_sShort": "hint",
}

ASSIGN_FACTIONS = {"m_sHaven": "Haven", "m_sBastion": "Bastion", "m_sRedHand": "Red Hand",
                   "m_sHalcyon": "Halcyon"}

# By function name, in the voice files (first match wins).
FUNCTION_CHANNELS = [
    (r"Broadcast\.c$", r".", "broadcast"),
    (r"Perimeter\.c$", r"Loud|Speaker|Warn|Line|Order", "loudspeaker"),
    (r"LeaderVoice\.c$", r".", "leader"),
    (r"(FactionVoice|FactionLife|GuidanceText|SecureRadio|RadioStation)\.c$",
     r"Chatter|Secure|Channel|Radio", "radio"),
    (r"(FactionVoice|FactionLife|GuidanceText|Wanderers)\.c$", r"Rumour|Gossip|News", "rumour"),
    (r"GuidanceText\.c$", r"Loudspeaker|Shout", "loudspeaker"),
    (r"(FactionVoice|FactionLife|Territory|CampLife|Safehouses|Wanderers|Checkpoints)\.c$",
     r".", "ambient"),
    (r"GuidanceTips\.c$", r".", "hint"),
    (r"(NoteText|Search|GuidanceText)\.c$", r".", "clue"),
    (r"", r"SuccessText|FailText|Fizzle|Outcome|Result", "outcome"),
    (r"", r"Rumour|Gossip|News", "rumour"),
    (r"", r"Headline", "headline"),
    (r"", r"Brief", "brief"),
    (r"", r"LeaderTalk|TalkText", "leader"),
    (r"", r"Journal", "journal"),
    (r"", r"ChoiceLabels", "choice"),
]

TOKEN_RE = re.compile(r"""
    (?P<comment>//[^\n]*|/\*.*?\*/)
  | (?P<string>"(?:[^"\\\n]|\\.)*")
  | (?P<ident>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<number>\d+(?:\.\d+)?f?)
  | (?P<op>\+=|-=|==|!=|<=|>=|&&|\|\||::|[{}()\[\];,=:.+\-*/<>!?&|%^~])
  | (?P<space>\s+)
  | (?P<other>.)
""", re.S | re.X)


def unescape(lit):
    body = lit[1:-1]
    return re.sub(r"\\(.)", lambda m: {"n": "\n", "t": "\t"}.get(m.group(1), m.group(1)), body)


def tokens(src):
    line = 1
    for m in TOKEN_RE.finditer(src):
        kind = m.lastgroup
        text = m.group()
        if kind not in ("comment", "space"):
            yield kind, text, line
        line += text.count("\n")


class Literal:
    __slots__ = ("file", "line", "text", "calls", "target", "function", "faction", "channel",
                 "register", "repeats", "returned")

    def as_dict(self):
        return {k: getattr(self, k) for k in ("file", "line", "channel", "faction", "register",
                                              "repeats", "function", "text")}


def scan_file(path, rel):
    """Every string literal in one file, with what it sits in."""
    with open(path, encoding="utf-8", errors="replace") as f:
        src = f.read()

    found = []
    parens = []        # [name, argIndex, argName, isArray]
    braces = []        # [kind, faction] kind: class, func, block
    function = None
    func_depth = -1
    statement = []     # tokens since the last ; { }
    prev = None
    prev2 = None
    last_closed_call = None
    pending_case = False

    for kind, text, line in tokens(src):
        if kind == "op" and text == "(":
            name = prev[1] if prev and prev[0] == "ident" else ""
            if name and prev2 and prev2[1] == ".":
                pass
            parens.append([name, 0, "", False])
        elif kind == "op" and text == ")":
            if parens:
                last_closed_call = parens.pop()[0]
        elif kind == "op" and text == "{":
            inline = prev is not None and prev[1] in ("=", "(", ",", "{", "return")
            if inline or (parens and parens[-1][3]):
                parens.append(["{array}", 0, "", True])
            else:
                if function is None and prev is not None and prev[1] == ")":
                    function = last_closed_call
                    func_depth = len(braces)
                    braces.append(["func", None])
                else:
                    braces.append(["block", None])
                statement = []
        elif kind == "op" and text == "}":
            if parens and parens[-1][3]:
                parens.pop()
            else:
                if braces:
                    braces.pop()
                if function is not None and len(braces) <= func_depth:
                    function = None
                    func_depth = -1
                statement = []
        elif kind == "op" and text == ";":
            if not parens:
                statement = []
        elif kind == "op" and text == ",":
            if parens:
                parens[-1][1] += 1
                parens[-1][2] = ""
        elif kind == "op" and text == ":":
            if parens and prev and prev[0] == "ident":
                parens[-1][2] = prev[1]
            if pending_case:
                pending_case = False
        elif kind == "ident" and text == "case":
            pending_case = True
            if braces:
                braces[-1][1] = None
        elif kind == "string":
            lit = Literal()
            lit.file = rel
            lit.line = line
            lit.text = unescape(text)
            lit.calls = [(p[0], p[1], p[2]) for p in reversed(parens) if not p[3]]
            lit.target = assignment_target(statement)
            lit.returned = bool(statement) and statement[0][1] == "return"
            lit.function = function or ""
            lit.faction = None
            for b in reversed(braces):
                if b[1]:
                    lit.faction = b[1]
                    break
            if not lit.faction:
                for tk in statement:
                    if tk[1] in FACTION_ENUM:
                        lit.faction = FACTION_ENUM[tk[1]]
                        break
            found.append(lit)

        if pending_case and kind == "ident" and text in FACTION_ENUM and braces:
            braces[-1][1] = FACTION_ENUM[text]

        if kind == "op" and text in ("{", "}", ";") and not (parens and parens[-1][3]):
            pass
        else:
            statement.append((kind, text))
        prev2 = prev
        prev = (kind, text)

    return found


def assignment_target(statement):
    depth = 0
    for i, (kind, text) in enumerate(statement):
        if text in ("(", "["):
            depth += 1
        elif text in (")", "]"):
            depth -= 1
        elif depth == 0 and text in ("=", "+=") and i > 0:
            for j in range(i - 1, -1, -1):
                if statement[j][0] == "ident":
                    return statement[j][1]
            return ""
    return ""


def is_prose(text):
    if len(text) < 4 or " " not in text.strip():
        return False
    if not re.search(r"[a-z]", text):
        return False
    if text.startswith("[RZ]") or text.startswith("RZ_") or text.startswith("{") and "}" in text[:40] and "/" in text:
        return False
    if re.search(r"\.(et|conf|layout|edds|imageset|json|txt|c|ent|ct)\b", text) and " " not in text.strip():
        return False
    return True


def effective_call(calls):
    """The first call around the literal that isn't a wrapper: (name, argIndex, argName)."""
    for name, idx, arg_name in calls:
        if name in WRAP_CALLS or name == "":
            continue
        return name, idx, arg_name
    return None


def guess_channel(lit):
    """The channel, or None when the literal isn't player text."""
    calls = lit.calls
    names = [c[0] for c in calls]
    if any(n in SKIP_CALLS for n in names[:1]):
        return None
    if "Attribute" in names:
        call = calls[names.index("Attribute")]
        if call[1] == 0 and not call[2] and is_prose(lit.text):
            return "setting"
        return None
    if any(n in ("Print", "PrintFormat", "Info", "Warn", "Error") for n in names):
        return None

    eff = effective_call(calls)
    if eff:
        name, idx, _ = eff
        if name in SKIP_CALLS:
            return None
        if name == "NotifyNear":
            return "notice" if is_prose(lit.text) else "title"
        rule = CALL_CHANNELS.get(name)
        if rule:
            if idx in rule:
                return rule[idx]
            if "*" in rule:
                return rule["*"]
    if lit.target in ASSIGN_CHANNELS:
        if ASSIGN_CHANNELS[lit.target] in ("title", "label") or is_prose(lit.text):
            return ASSIGN_CHANNELS[lit.target]
    if not is_prose(lit.text):
        return None
    for file_re, func_re, channel in FUNCTION_CHANNELS:
        if re.search(file_re, lit.file) and re.search(func_re, lit.function):
            return channel
    return "text"


def guess_faction(lit):
    for part, faction in FACTION_FOLDERS:
        if part in lit.file:
            return faction
    if lit.target in ASSIGN_FACTIONS:
        return ASSIGN_FACTIONS[lit.target]
    return lit.faction or "-"


BLEAK_WORDS = re.compile(r"\b(dead|died|dies|buried|bury|grave|funeral|turned|killed|body|bodies|"
                         r"nobody came|never came|lost|gone)\b", re.I)
WRY_WORDS = re.compile(r"\b(apparently|joking|mostly|anyway|of course|again|same as yesterday|"
                       r"nobody has asked|don't tell)\b", re.I)


def guess_register(lit):
    t = lit.text.strip()
    if re.fullmatch(r"[\s.\"…]*", t):
        return "SILENCE"
    if "!" in t and lit.faction == "Red Hand":
        return "TAUNT"
    if WRY_WORDS.search(t):
        return "WRY"
    if BLEAK_WORDS.search(t):
        return "BLEAK"
    return "PLAIN"


def inventory(root=ROOT):
    base = os.path.join(root, SCRIPTS)
    all_lits = []
    for dirpath, _, files in os.walk(base):
        for name in sorted(files):
            if not name.endswith(".c"):
                continue
            path = os.path.join(dirpath, name)
            rel = os.path.relpath(path, root).replace(os.sep, "/")
            all_lits.extend(scan_file(path, rel))
    all_lits.sort(key=lambda l: (l.file, l.line))

    counts = Counter(l.text for l in all_lits)
    kept = []
    for lit in all_lits:
        lit.channel = guess_channel(lit)
        if not lit.channel:
            continue
        lit.faction = guess_faction(lit)
        lit.register = guess_register(lit)
        lit.repeats = counts[lit.text]
        kept.append(lit)
    return kept, all_lits


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--channel", help="only this channel (" + ", ".join(CHANNELS) + ")")
    ap.add_argument("--faction", help="only this faction (Haven, Bastion, 'Red Hand', Halcyon, 'Long Table', -)")
    ap.add_argument("--summary", action="store_true", help="counts by channel and faction")
    ap.add_argument("--json", action="store_true", help="JSON instead of a table")
    args = ap.parse_args()

    lits, _ = inventory()
    if args.channel:
        lits = [l for l in lits if l.channel == args.channel]
    if args.faction:
        lits = [l for l in lits if l.faction.lower() == args.faction.lower()]

    if args.json:
        json.dump([l.as_dict() for l in lits], sys.stdout, indent=1, ensure_ascii=False)
        print()
        return
    if args.summary:
        by_channel = Counter(l.channel for l in lits)
        by_faction = Counter(l.faction for l in lits)
        print("%d player-facing literals in %d files" % (len(lits), len({l.file for l in lits})))
        print("\nBy channel:")
        for ch in CHANNELS:
            if by_channel[ch]:
                print("  %-12s %5d  %s" % (ch, by_channel[ch], CHANNELS[ch]))
        print("\nBy faction:")
        for fa, n in by_faction.most_common():
            print("  %-12s %5d" % (fa, n))
        repeated = sum(1 for l in lits if l.repeats > 1)
        print("\nLiterals that appear more than once: %d" % repeated)
        return

    print("file:line\tchannel\tfaction\tregister\trepeats\ttext")
    for l in lits:
        text = l.text.replace("\t", " ").replace("\n", "\\n")
        print("%s:%d\t%s\t%s\t%s\t%d\t%s" % (l.file, l.line, l.channel, l.faction, l.register, l.repeats, text))


if __name__ == "__main__":
    main()
