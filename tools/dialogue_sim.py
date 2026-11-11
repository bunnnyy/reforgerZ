#!/usr/bin/env python3
"""ReforgerZ dialogue simulator (docs/DIALOGUE.md, "Tools").

    python3 tools/dialogue_sim.py haven.head.house --bg nurse --tier haven=trusted
    python3 tools/dialogue_sim.py redhand.post.toll --tier redhand=known --infamy redhand=20 --bg excon
    python3 tools/dialogue_sim.py occupant.house.short --house short --food 3
    python3 tools/dialogue_sim.py --all --bg doctor            # every conversation, start nodes only

Walks one conversation as one character sees it: which topic shows in the hub, every node reachable
from the start, the lines that show (variants picked), and every choice with the tag the server
would put on it, whether it's shown, and where a check would send this character (pass, short or
refused). No dice anywhere, so the result is exact for the facts you give; anything you don't give
reads false. Uses tools/dialogue_pack.py's parser. No dependencies beyond Python 3.
"""
import argparse
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dialogue_pack as dp  # noqa: E402

RANK = {"stranger": 0, "known": 1, "trusted": 2, "champion": 3, "uneasy": 0, "tolerated": 1, "wildcard": 1,
        "respected": 2, "watched": -1, "hunted": -2, "kill_on_sight": -3}
NAMES = {"haven": "Haven", "bastion": "Bastion", "redhand": "Red Hand", "halcyon": "Halcyon"}


class Char:
    def __init__(self, a):
        self.bg = a.bg
        self.tiers = dict(kv.split("=") for kv in a.tier)
        self.infamy = {k: int(v) for k, v in (kv.split("=") for kv in a.infamy)}
        self.fame = {k: int(v) for k, v in (kv.split("=") for kv in a.fame)}
        self.flags = {}
        for kv in a.flag:
            k, _, v = kv.partition("=")
            self.flags[k] = int(v or 1)
        self.knows = set(a.knows)
        self.regard = a.regard
        self.food = a.food
        self.facts = set(a.fact)
        self.house = a.house
        self.trait = a.trait
        self.attitude = a.attitude
        self.favor = {k: int(v) for k, v in (kv.split("=") for kv in a.favor)}
        self.mem = set(a.mem)

    def rank(self, f):
        return RANK.get(self.tiers.get(f, "stranger"), 0)


def cmp(a, op, b):
    return {"=": a == b, "!=": a != b, ">=": a >= b, "<=": a <= b, ">": a > b, "<": a < b}[op]


def term_true(ch, term):
    m = dp.TERM_RE.match(term.strip())
    if not m:
        return False
    neg, key, arg, op, val = m.groups()
    r = term_value(ch, key, arg, op, val)
    return (not r) if neg else r


def term_value(ch, key, arg, op, val):
    if key == "route" and arg == "none":
        return True
    if key in dp.QUIET_KEYS:
        return False
    if key == "bg":
        return ch.bg in (arg or "").split(",")
    if key == "tier":
        t = ch.tiers.get(arg, "stranger")
        if not op:
            return True
        if op == "=":
            return t == val
        return cmp(RANK.get(t, 0), op, RANK.get(val, 0))
    if key == "hunted":
        return RANK.get(ch.tiers.get(arg, "stranger"), 0) <= -2
    if key in ("infamy", "fame"):
        src = ch.infamy if key == "infamy" else ch.fame
        return cmp(src.get(arg, 0), op or ">", int(val or 0))
    if key in ("flag", "cflag"):
        v = ch.flags.get(arg, 0)
        return cmp(v, op, int(val)) if op else v != 0
    if key == "regard":
        return cmp(ch.regard, op, int(val))
    if key == "attitude":
        return cmp(ch.attitude, op, int(val))
    if key == "knows":
        return arg in ch.knows
    if key == "item":
        m = re.match(r"food\*([0-9]+)", arg or "")
        return bool(m) and ch.food >= int(m.group(1))
    if key == "house":
        return ch.house == arg
    if key == "trait":
        return ch.trait == arg
    if key in ("met", "helped", "saved", "crossed"):
        return key in ch.mem
    return ("%s:%s" % (key, arg) if arg else key) in ch.facts


def cond_true(ch, cond):
    if not cond.strip():
        return True
    return all(term_true(ch, t) for t in cond.split("&"))


def trust(ch, f, extras):
    if f == "none":
        f = "haven"
        cap = 2
    else:
        cap = 99
    if ch.rank(f) <= -2:
        return -10
    score = max(0, ch.rank(f))
    for e in extras:
        k, _, v = e.partition(":")
        if k == "bg" and ch.bg in v.split(","):
            score += 1
    if ch.regard >= 2 or "helped" in ch.mem or "saved" in ch.mem:
        score += 1
    return min(score, cap)


def fear(ch, f, extras):
    band = lambda v: 2 if v >= 40 else (1 if v >= 15 else 0)
    if f == "none":
        score = max([band(v) for v in ch.infamy.values()] + [0])
    else:
        score = band(ch.infamy.get(f, 0))
    bgs = "excon,raider,soldier"
    for e in extras:
        k, _, v = e.partition(":")
        if k == "bg":
            bgs = v
    if ch.bg in bgs.split(","):
        score += 1
    return score


def difficulty(ch, d, extras):
    for e in extras:
        k, _, v = e.partition(":")
        if k == "easier" and ch.trait == v:
            d -= 1
        if k == "harder" and ch.trait == v:
            d += 1
    return max(1, d)


def describe_choice(ch, conv, c):
    tag = ""
    shown = cond_true(ch, c.cond)
    result = c.targets[0]
    check = c.check.split()
    if check:
        kind, f, d = check[0], check[1], int(check[2])
        d = difficulty(ch, d, check[3:])
        fname = NAMES.get(f, "")
        if kind == "persuade":
            score = trust(ch, f, check[3:])
            if score < 0:
                tag = "[Persuade] (won't listen)"
                return tag, shown, "disabled"
            tag = "[Persuade - %s - %d of %d]" % (fname or "you", score, d) if fname else "[Persuade - %d of %d]" % (score, d)
        elif kind == "lie":
            score = max(0, trust(ch, f, []))
            tag = "[Lie - %d of %d]" % (score, d)
        else:
            score = fear(ch, f, check[3:])
            tag = "[Fear %d of %d - +3 Infamy]" % (score, d)
        res = "pass" if score >= d else ("short" if score == d - 1 else "refused")
        idx = {"pass": 0, "short": 1, "refused": 2}[res]
        if idx >= len(c.targets):
            idx = len(c.targets) - 1
        result = "%s -> %s" % (res, c.targets[idx])
    if c.cost:
        m = re.match(r"favor:(\w+) ([0-9]+)", c.cost)
        if m:
            tag = (tag + " " if tag else "") + "[%s %s Favor]" % (m.group(2), NAMES[m.group(1)])
            if ch.favor.get(m.group(1), 0) < int(m.group(2)):
                return tag, shown, "disabled (needs %s %s Favor)" % (m.group(2), NAMES[m.group(1)])
        m = re.match(r"item:food\*([0-9]+)", c.cost)
        if m:
            tag = (tag + " " if tag else "") + "[%s food]" % m.group(1)
            if ch.food < int(m.group(1)):
                return tag, shown, "disabled (you have %d food)" % ch.food
    for term in c.cond.split("&"):
        m = dp.TERM_RE.match(term.strip()) if term.strip() else None
        if m and m.group(2) == "bg" and not m.group(1):
            tag = "[%s]" % m.group(3).split(",")[0].capitalize() + (" " + tag if tag else "")
        if m and m.group(2) == "tier" and not m.group(1) and m.group(4) == ">=":
            tag = "[%s - %s]" % (m.group(5).capitalize(), NAMES.get(m.group(3), "")) + (" " + tag if tag else "")
    return tag, shown, result


def say_lines(ch, node):
    out = []
    pending = None
    for kind, cond, val in node.items:
        if kind != "say":
            continue
        if cond:
            if pending is None and cond_true(ch, cond):
                pending = val
            continue
        out.append(pending if pending is not None else val)
        pending = None
    if pending is not None:
        out.append(pending)
    return out


def walk(ch, conv, start_only=False):
    print("== %s  (%s; for %s)" % (conv.id, conv.label or "greet", ", ".join(conv.speakers)))
    if not cond_true(ch, conv.when):
        print("   not in the hub for this character (when: %s)" % conv.when)
        return
    seen = set()
    todo = ["start"]
    while todo:
        nid = todo.pop(0)
        if nid in seen or not conv.node(nid):
            continue
        seen.add(nid)
        node = conv.node(nid)
        jumped = None
        for kind, cond, val in node.items:
            if kind == "jump" and cond_true(ch, cond):
                jumped = val
                break
        print("  [%s]%s" % (nid, " -> jumps to " + jumped if jumped else ""))
        if jumped:
            todo.append(jumped)
            continue
        for line in say_lines(ch, node):
            print("     " + line)
        for c in node.choices:
            tag, shown, result = describe_choice(ch, conv, c)
            mark = " " if shown else "x"
            print("   %s @%-10s %s %s  => %s" % (mark, c.id, (tag + " ") if tag else "", c.label, result))
            if shown and not start_only:
                for t in c.targets:
                    if t not in dp.TARGETS_SPECIAL:
                        todo.append(t)
        print("     @leave      I'll be going.")
        if start_only:
            break


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("conv", nargs="?", help="conversation id")
    ap.add_argument("--all", action="store_true", help="every conversation, start nodes only")
    ap.add_argument("--bg", default="unemployed")
    ap.add_argument("--tier", action="append", default=[], help="faction=tier (haven=trusted)")
    ap.add_argument("--infamy", action="append", default=[], help="faction=N")
    ap.add_argument("--fame", action="append", default=[], help="faction=N")
    ap.add_argument("--favor", action="append", default=[], help="faction=N")
    ap.add_argument("--flag", action="append", default=[], help="key=value (season or character flag)")
    ap.add_argument("--knows", action="append", default=[])
    ap.add_argument("--fact", action="append", default=[], help="any other true term: night, wbitten, need:food, others")
    ap.add_argument("--mem", action="append", default=[], help="met, helped, saved, crossed")
    ap.add_argument("--regard", type=int, default=0)
    ap.add_argument("--attitude", type=int, default=0)
    ap.add_argument("--food", type=int, default=0)
    ap.add_argument("--house", default="", help="short, fed or died")
    ap.add_argument("--trait", default="")
    args = ap.parse_args()

    convs, errors = dp.load_all()
    for e in errors:
        print("error: " + e)
    ch = Char(args)
    if args.all:
        for c in convs:
            walk(ch, c, start_only=True)
        return
    if not args.conv:
        ap.error("name a conversation, or --all")
    for c in convs:
        if c.id == args.conv:
            walk(ch, c)
            return
    print("no conversation '%s' (tools/dialogue_pack.py --list)" % args.conv)
    sys.exit(1)


if __name__ == "__main__":
    main()
