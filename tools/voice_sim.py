#!/usr/bin/env python3
"""ReforgerZ voice bank simulator (docs/WRITING.md, "The voice bank"; docs/WORKBENCH_SETUP.md, "Text lint").

    python3 tools/voice_sim.py                 # pool sizes and joke shares per kind, faction and mood
    python3 tools/voice_sim.py --visits 10     # a veteran's day: 10 visits a seat, per mood
    python3 tools/voice_sim.py --list          # every line in the bank
    python3 tools/voice_sim.py --season speed1 # a scripted season: joke share by mood and repeats per window

Reads the code defaults in scripts/Game/ReforgerZ/Factions/RZ_VoiceDefaults.c (one Line() per line)
and replays the picker's rules offline, in the manner of tools/pacing_sim.py: for each mood (Early,
Grim, Bleak, Hope, Epilogue) and each pool (kind x faction x manner), how many lines can be said,
how many of them are jokes, the share of picks that are jokes (one in 3 / 5 / 10 by mood), and
whether the pool is big enough for its window: a pool's jokes should number at least
1.5 x visits x share (docs/WRITING.md, "Repeats"), or a veteran hears plain lines only. Grief and
Voice > Lines overrides aren't simulated. No dependencies beyond Python 3.
"""
import argparse
import os
import random
import re
from collections import defaultdict

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
DEFAULTS = os.path.join(ROOT, "scripts", "Game", "ReforgerZ", "Factions", "RZ_VoiceDefaults.c")
MOODS = [("early", 1, 3), ("grim", 2, 5), ("bleak", 4, 10), ("hope", 8, 3), ("epilogue", 16, 3)]
FACTIONS = {"HAVEN": "haven", "BASTION": "bastion", "REDHAND": "redhand", "HALCYON": "halcyon", "-1": "any"}
JOKES = {"W", "J", "T"}
LINE_RE = re.compile(r'Line\(lines, "([^"]+)", (\w+|-1), "([^"]*)", "([^"]*)", (\w), (\d+), "([^"]*)", "((?:[^"\\]|\\.)*)"\);')


def load():
    src = open(DEFAULTS, encoding="utf-8").read()
    lines = []
    for m in LINE_RE.finditer(src):
        lid, faction, kind, manner, reg, moods, when, text = m.groups()
        lines.append({"id": lid, "faction": FACTIONS.get(faction, faction), "kind": kind, "manner": manner,
                      "joke": reg in JOKES, "moods": int(moods), "when": when, "text": text.replace('\\"', '"')})
    return lines


# Scripted seasons (docs/WRITING.md, "Checking text"): the moods a season runs through, in played hours.
# Speed 1 is about 45 days; speed 2 halves every stretch; slow is a gate that runs to the slow finish
# (the long stall in Bleak). Each visit is one leader opener and one radio tail.
SEASONS = {
    "speed1": [("early", 40), ("grim", 60), ("bleak", 15), ("hope", 2), ("grim", 30), ("epilogue", 24)],
    "speed2": [("early", 20), ("grim", 30), ("bleak", 8), ("hope", 2), ("grim", 15), ("epilogue", 24)],
    "slow": [("early", 40), ("grim", 50), ("bleak", 40), ("epilogue", 24)],
}


def season(lines, which, visits_per_hour, rng):
    """Walk a scripted season: at each visit, a face-to-face opener (24 real hours of joke memory per
    player) and a radio tail (6 hours per channel, the lower of 1 in 4 and the mood's share, the Red
    Hand never under 1 in 5). Reports the joke share by mood and any joke heard twice inside its window."""
    one_in = {name: n for name, _, n in MOODS}
    bits = {name: bit for name, bit, _ in MOODS}
    face = [l for l in lines if l["kind"] == "opener"]
    radio = [l for l in lines if l["kind"] == "radio_tail"]
    heard_face = {}
    heard_radio = {}
    clock = 0.0
    totals = defaultdict(lambda: [0, 0, 0])   # mood -> picks, jokes, repeats
    for mood, hours in SEASONS[which]:
        steps = int(hours * visits_per_hour)
        for _ in range(steps):
            clock += 1.0 / visits_per_hour
            for pool, memory, window, cap in ((face, heard_face, 24, 1), (radio, heard_radio, 6, 4)):
                faction = rng.choice(["haven", "bastion", "redhand", "halcyon"])
                fit = [l for l in pool if l["faction"] in (faction, "any") and (l["moods"] == 0 or l["moods"] & bits[mood])]
                if not fit:
                    continue
                share = max(cap, one_in[mood])
                if pool is radio and faction == "redhand":
                    share = min(share, 5)
                jokes = [l for l in fit if l["joke"] and clock - memory.get(l["id"], -999) >= window]
                plain = [l for l in fit if not l["joke"]]
                totals[mood][0] += 1
                if jokes and rng.random() < 1.0 / share:
                    pick = rng.choice(jokes)
                    if clock - memory.get(pick["id"], -999) < window:
                        totals[mood][2] += 1
                    memory[pick["id"]] = clock
                    totals[mood][1] += 1
                elif plain:
                    rng.choice(plain)
    print("%-9s %6s %6s %7s %8s" % ("mood", "picks", "jokes", "share", "repeats"))
    for name, _, n in MOODS:
        if name not in totals:
            continue
        picks, jokes, repeats = totals[name]
        print("%-9s %6d %6d %6.0f%% %8d   (target about 1 in %d)" % (name, picks, jokes, 100.0 * jokes / max(1, picks), repeats, n))
    print("voice_sim: season %s at %d visits an hour" % (which, visits_per_hour))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--visits", type=int, default=10, help="visits to one seat in a real day (the joke window)")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--season", choices=sorted(SEASONS), help="replay a scripted season (speed1, speed2, slow)")
    ap.add_argument("--per-hour", type=int, default=4, help="with --season: visits an hour (the fastest cadence)")
    args = ap.parse_args()
    lines = load()
    if args.season:
        season(lines, args.season, args.per_hour, random.Random(args.seed))
        return
    if args.list:
        for l in lines:
            print("%-28s %-8s %-11s %-8s %-5s %s" % (l["id"], l["faction"], l["kind"], l["manner"] or "-",
                                                    "joke" if l["joke"] else "plain", l["text"]))
        print("%d lines" % len(lines))
        return

    pools = defaultdict(list)
    for l in lines:
        pools[(l["kind"], l["faction"], l["manner"])].append(l)
    rng = random.Random(args.seed)
    short = 0
    print("%-34s %-9s %5s %5s %6s %6s  %s" % ("pool (kind faction manner)", "mood", "lines", "jokes", "share", "heard", "enough?"))
    for key in sorted(pools):
        for name, bit, one_in in MOODS:
            fit = [l for l in pools[key] if l["moods"] == 0 or l["moods"] & bit]
            # a manner-less mood line also joins every manner's pool of its kind and faction
            if key[2]:
                fit += [l for l in pools[(key[0], key[1], "")] if l["moods"] & bit]
            if not fit:
                continue
            jokes = [l for l in fit if l["joke"]]
            plain = [l for l in fit if not l["joke"]]
            heard = set()
            joke_picks = 0
            for _ in range(args.visits):
                fresh = [j for j in jokes if j["id"] not in heard]
                if fresh and rng.random() < 1.0 / one_in:
                    pick = rng.choice(fresh)
                    heard.add(pick["id"])
                    joke_picks += 1
                elif plain:
                    rng.choice(plain)
            need = 1.5 * args.visits / one_in
            ok = "yes" if not jokes or len(jokes) >= need else "no (%.0f jokes wanted)" % need
            if not ok.startswith("yes"):
                short += 1
            print("%-34s %-9s %5d %5d %6s %6d  %s" % (" ".join(k or "-" for k in key), name, len(fit), len(jokes),
                                                     "1/%d" % one_in, joke_picks, ok))
    print("voice_sim: %d lines, %d pools, %d pool-moods short of jokes (they fall back to plain lines)"
          % (len(lines), len(pools), short))


if __name__ == "__main__":
    main()
