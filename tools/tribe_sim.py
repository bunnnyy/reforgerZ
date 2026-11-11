#!/usr/bin/env python3
"""ReforgerZ tribe ecology simulator (the infected tribes, on the rules the code runs).

    python3 tools/tribe_sim.py                    # Arland, Everon, Cain, Chernarus-size; 2, 5 and 10 players
    python3 tools/tribe_sim.py --maps arland everon chernarus --slots 2   # the three map sizes (Arland needs Slots)
    python3 tools/tribe_sim.py --proposed         # with phase 0's proposed retunes (waiting for the author)
    python3 tools/tribe_sim.py --maps arland --slots 1   # Arland with the Slots override (no tribe without it)
    python3 tools/tribe_sim.py --legacy           # the same on the old 10-day pacing
    python3 tools/tribe_sim.py --maps everon --players 5 --runs 100
    python3 tools/tribe_sim.py --wild everon=31   # another wild land (km2; the defaults are the land survey's)
    python3 tools/tribe_sim.py --design           # the design's numbers, before this tool's tuning
    python3 tools/tribe_sim.py --set split_at=22 ran_points=5   # try any number (--list shows them)
    python3 tools/tribe_sim.py --trace            # one Everon season, a line a real day

Simulates the infected tribes as data, the way the design's model rules describe them, on the play
model and clocks of tools/pacing_sim.py (its sessions, its stage clock, and one of its seasons for
when the 75% serum, the 90% gate and the cure come). A season runs phase by phase: a game day of
4 real hours (x6), so 6 days, dusks, nights and dawns a real day. On each phase:
  - towns rise by the Director's spread as coded (creep, and spread from towns at 60+, both only
    with someone online) and fall 1.5 a kill; each tribe banks its towns' rises (the ones who ran);
  - players spend their minutes in towns, the wild and safe places; the wild minutes that fall
    inside a range meet that tribe's bodies at a rate worked out from what is out (the day party,
    scouts, the lair ring, or the night's hunting parts), the range's area and the sight distance;
  - each meeting is a fight: players kill bodies, bodies sometimes kill players (who are taken);
  - a counted night (20 real minutes online) adds hunger, data takings (wanderers, trucks) and
    border fights; the night's commitment caps what a hunt can lose;
  - each season day (8 hours played; 1.6 on the old pacing) runs ageing, challenges, deaths of age,
    successions with their splinters, lone ones, new tribes, splits and clans, culls and lair moves.

The day party is the author's change: by day each tribe keeps hunters and young out (at most a
quarter of it, tuned to a fifth; twice that when hungry; in small groups, 6 bodies at most at one
player cluster), scouts stay on the
ridges, alpha and guard rest at the lair. A scout's call by day brings at most 4 of the day party;
day noise draws only the day party; nobody is out the day after a feed; towns by day stay the
crawlers'. Fog counts as dusk: the hunt can come by day in fog.

The output starts with every number this tool tuned against the design, and why (TUNED_WHY below);
--design runs the design's numbers. One rule is offered but not tuned in: with
--set ran_highs=1 ran_points=4 a town banks only its new highs, so a town that players pushed back
sends nobody new until it passes its old level (the pitch's "players who push a town back starve the
tribe"; as written, rule 5 banks every tick's rise and the background creep keeps every tribe near
its cap). It makes clans depend on the server's size, and starves Arland's one tribe of young.

Measured (phase 0): the wild land, land, holdings and lair candidates of Arland, Everon and Cain
(tools/land_survey/), and the infected's sight by day and night (tools/zombie_lab/: the chance of
being engaged in 30 s at 25-200 m, turned into the width of ground a walker passing one infected is
seen across). Arland's wild land is under the 1.5 km2 floor, so it has no tribe unless --slots says
otherwise. Every other number is the design's or an assumption printed at the top of the output.
Change them with --set when play-tests say otherwise. Simplified: the
map is a square of towns with the wild between them; ranges are discs; rivers, valleys, habits, fear
places, marks, the Red Hand's moat, the Long Table, tags and seasonal pull are left out; the season
ends at the cure. No dependencies beyond Python 3.
"""
import argparse
import math
import os
import random
import statistics
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pacing_sim as ps  # noqa: E402  (its play model, its stage clock and its research timeline)

# ----------------------------------------------------------------------------- the design's numbers
# The tribes design's model rules, as written. TUNED below holds what this tool changed and why.
DESIGN = {
    "slot_km2": 6.0,            # rule 1: slots = wild km2 / this, rounded, 0-15
    "slot_floor_km2": 1.5,      # at least 1 slot from this much wild land
    "seeds_div": 3,             # rule 2: seeds = slots / this, rounded up, at most 4
    "seed_open": 0,             # 1: seeds = every slot not kept for a split, at most 4
    "seed_size": (10, 14),      # members of a seed tribe
    "seed_lead": (2, 5),        # season days its Rising alpha has already led
    "dark_infection": 70,       # a town gone dark: this infection or more...
    "dark_km": 3.0,             # ...with no range within this...
    "dark_days": 5,             # ...for this many season days sends out a tribe of...
    "dark_size": 6,             # ...this many
    "taken_found": 5,           # people taken outside every range...
    "taken_found_vacuum": 3,    # (in a vacuum)
    "taken_found_km": 1.5,      # ...within this of each other...
    "taken_found_days": 3,      # ...in this many season days become a tribe
    "lone_gather": 6,           # splinter members a lone one gathers into a tribe
    "room_big": 18,             # while a kept slot is free, no new tribe within room_km of a tribe this big
    "room_km": 3.0,
    "young_days": 4,            # rule 3: the young grow up after this many season days
    "max_members": 30,
    "split_at": 25,             # rule 11
    "players_young_max": 3,     # rule 4: a tribe's players' young at a time
    "ran_points": 4.0,          # rule 5: banked infection points per young
    "ran_per_day": 2,           # young from the ones who ran, per tribe per season day
    "ran_bank": 8.0,            # a tribe's bank holds this many points at most
    "ran_km": 1.5,              # a town banks to the nearest tribe whose range edge is this close
    "ran_highs": 0,             # 1: a town banks only rises above the highest it has been (0: every tick's rise)
    "hunger_start": 30,         # rule 6
    "hunger_night": 5,          # per counted night
    "hunger_decline": 7,        # per counted night with a Declining alpha
    "hunger_restless": 2,       # more per counted night while restless
    "hunger_taken": 25,         # off for each person taken
    "starve_share": 0.2,        # starving: this share (at least 2) walks into the nearest town
    "starve_reset": 70,
    "starve_town_cap": 10,      # infection per town a season day from walkers
    "wanderer_take": 0.10,      # rule 7: per crossing of a hunting trail at night
    "wanderer_every": 3,        # at most one per tribe every this many counted nights
    "truck_take": 0.15,         # per truck within 200 m of a hunting part at night
    "truck_every": 3,           # at most one per server every this many counted nights
    "break_share": 1 / 3,       # rule 8: a hunt breaks off after losing this share of who set out
    "break_cautious": 1 / 4,
    "break_bold": 1 / 2,
    "waves": 2,                 # ...or after this many waves at one cluster
    "weak_under": 8,            # under this, a tribe hunts as data only
    "breakup_under": 4,         # under this (never in its first 3 season days) it breaks up
    "never_break_days": 3,
    "first_wave": 8,            # bodies in a hunt's first wave (x the cluster share / 28)
    "commit_cap": 0.0,          # a hunt never sends in what could cost it more than this share of who set out (0: no cap)
    "weak_quiet": 0,            # 1: a weakened tribe posts no scouts (0: its scouts call, with no pack)
    "cull_min": 0,              # Bastion's cull contracts only name a tribe of this many or more
    "presence": 14,             # one tribe at one cluster
    "share_cap": 28,            # a cluster's share of the 100-agent budget
    "disturb_decay": 10,        # rule 10: per season day
    "disturb_guard": 15,        # a guard killed at the lair
    "disturb_cull": 50,         # a cull (shelling or a Fire Mission on it)
    "displace_cap": 2,          # rule 17: displacements a season per causing faction
    "displace_km": 2.0,         # the new lair at least this far from that faction's holdings
    "moves_per_day": 2,         # lair moves on the server a season day
    "split_share": 0.4,         # rule 11: the daughter takes this share
    "split_room": (1.5, 3.0),   # a free lair candidate this far away
    "clan_hold": 8,             # both halves hold this many...
    "clan_days": 3,             # ...for this many season days
    "clan_km": 4.0,
    "rising_days": 5,           # rule 14: Rising 0-5, Prime 6-20, Decline 21+
    "prime_days": 20,
    "challenge": 0.15,          # a season day in Decline (x2 while restless)
    "challenge_win": 0.5,       # the challenger's chance...
    "challenge_win_day": 0.05,  # ...plus this a season day of Decline...
    "challenge_win_max": 0.85,  # ...up to this
    "age_death": 15,            # season days into Decline, never challenged
    "splinter_share": (0.3, 0.4),   # rule 15
    "lone_leave": 0.25,         # the other guard leaves as a lone one
    "lone_max": 2,              # rule 16
    "lone_days": 6,
    "weak_days": 2,             # weak season days after a succession
    "vacuum_days": 3,           # rule 18
    "border_km": 0.15,          # rule 12
    "border_old_days": 5,
    "frenzy_space": 3,          # season days between frenzy nights; none in the first 2
    # The author's change: wild infected by day
    "day_share": 0.25,          # the day party: at most this share of the tribe (hunters and young)
    "day_hungry": 2.0,          # x this when Hungry or worse (to town edges, never into no man's land)
    "day_group": (2, 4),        # in groups of
    "day_cluster_cap": 6,       # day bodies at one player cluster
    "day_scout_pack": 4,        # a scout's call by day brings at most this many (never the alpha)
    "day_packs": 2,             # day packs per tribe per game day
}

# What this tool's runs changed against the design, and why (python3 tools/tribe_sim.py --design shows
# the design's numbers). Each value is (the tuned number, the reason).
TUNED_WHY = {
    "commit_cap": (0.5, "rule 8 breaks a hunt off at a third lost, but a first wave of 8 alone overshot it: tribes "
                        "of 12-20 lost 40-60% a hunt, a bold one with its core 75-90% on its first night. Now no wave "
                        "(and no core) goes in that could take the night's losses past half of who set out"),
    "weak_quiet": (1, "a weakened tribe's scouts still charged and died one by one; it now posts none"),
    "cull_min": (12, "Bastion's cull contracts kept naming tribes already under 8; they name a tribe of 12+"),
    "slot_km2": (5.0, "the land survey measured Everon's wild land at 24-26 km2 (the design estimated 24-36): at 1 "
                      "per 6 km2 that is 4 slots with one kept for a split, so 3 tribes, and Everon was in 4-6 in only "
                      "2-22% of seasons; 1 per 5 gives 5 slots and 4 tribes (Cain 4 slots, 3 tribes)"),
    "seed_open": (1, "slots / 3 gave Everon 2 seeds, and busy servers keep towns from going dark, so it sat at 2-3 "
                     "tribes; every slot not kept for a split is seeded (Everon 4, Cain 3, Chernarus-size 4)"),
    "hunger_night": (3, "+5 a counted night against about 0.06 takings a tribe a night: 70% of tribe-nights "
                        "Hungry and 3-4 starvations a tribe a season; +3 gives about half Hungry"),
    "hunger_decline": (5, "kept 2 above the normal rise"),
    "dark_days": (3, "with 5 season days and 3 km Chernarus-size stopped at 11 tribes of 15 slots"),
    "dark_km": (2.0, "most towns lie within 3 km of some range once a few tribes exist"),
    "ran_points": (6.0, "with 4, every tribe sat at the 2-a-day cap whatever players did (27-31 points banked a "
                        "tribe a season day on Everon), and Everon had a clan in 85-95% of seasons"),
    "split_at": (26, "with 6 points and the measured wild land, a clan in about half of Everon seasons at 5 "
                     "players (25: about two in three; 27: about two in five)"),
    "day_group": ((2, 3), "the author's 2-4 kept to the small end, so a quarter of a tribe of 10-15 makes "
                          "more, smaller groups"),
    "day_share": (0.2, "with the measured sight (210 m, not 120) a quarter of the tribe out by day made a day "
                       "crossing as busy as a night's (1.3-1.5 fights against 1.1-1.6 hunts); a fifth keeps it "
                       "under the night's in every row, inside the author's 'at most a quarter'"),
}
TUNED = {k: v[0] for k, v in TUNED_WHY.items()}

# What the game's code runs (scripts/Game/ReforgerZ/Zombies: RZ_Tribes, RZ_TribeLife, RZ_TribeBorders),
# where it differs from the design: the default. Of phase 0's proposals the code took the slot divisor
# and the day party's share and groups; the rest wait for the author (--proposed runs them).
BUILT_WHY = {
    "slot_km2": (5.0, "Zombies > Tribes > Wild km2 per slot (phase 0's land survey)"),
    "day_share": (0.2, "Zombies > Tribes > Day party share"),
    "day_group": ((2, 3), "Zombies > Tribes > Day party group"),
    "hunger_night": (3, "Zombies > Tribes > Hunger a counted night (phase 3's run: at 5, Everon's tribes were hungry "
                        "60-70% of the time and starved 8-13 times a season; 3 gives 36-38%, about 3)"),
    "hunger_decline": (5, "2 above Hunger a counted night"),
    "commit_cap": (0.5, "the night's commitment as coded: no wave (and no core) goes in that could take the night's "
                        "losses past half of who set out (without it the worst first hunt lost 64-75%, and 1-2 tribes a "
                        "season were wiped)"),
    "dark_days": (3, "a town gone dark founds after 3 season days (phase 3's run: at 5, Everon sat at 3 tribes)"),
    "dark_km": (2.0, "...with no range within 2 km (at 3 km most towns are near some range once a few tribes exist)"),
    "split_at": (24, "Zombies > Tribes > Split at (phase 3's run: a clan in about half of Everon seasons at 2-5 players; 26: a quarter at 5)"),
}
BUILT = {k: v[0] for k, v in BUILT_WHY.items()}

# ----------------------------------------------------------------------------------- assumptions
# Not design numbers: how players and Bacon's infected behave. P and trail_r are measured (the zombie
# lab, Bacon Zombies 1.8.0 on clear lines: about 2 in 3 engaged within 30 s at 25 m, a flat 25% by day
# and 17% by night from 50 to 200 m, none beyond about 250 m; behind a hill, never); the rest are not.
ASSUME = {
    "time_accel": 6.0,          # a game day of 4 real hours
    "counted_minutes": 20,      # a night counts with someone online this long
    "towns_day": 0.45, "wild_day": 0.35,        # share of a player's minutes by day (rest: safe places)
    "towns_dusk": 0.35, "wild_dusk": 0.35,
    "towns_night": 0.30, "wild_night": 0.20,
    "wild_ref": 0.5,            # the wild shares above are for a map half wild; x (wild / land) / this, 0.5-1.5
    "foot": 0.75,               # share of wild minutes on foot (the rest in a car meets nothing)
    "walk_kmh": 4.0,            # on foot in the wild, with stops
    "groups": (0.45, 0.35, 0.20),   # a player plays solo, in a pair, or in a three
    "town_kill_share": 0.8,     # of the kills not made in the wild, the share in towns (1.5 infection each)
    "P": 0.21,                  # km: Bacon's sight by day: the width a walker passing one is seen across (measured; design 0.12)
    "trail_r": 0.19,            # km: a night hunt's: the same at night (measured, near-full moon; design 0.15)
    "scout_day": 0.12, "scout_dusk": 0.09, "scout_night": 0.06,   # km: scouts' sight
    "overlap": 1.5,             # players walk the same tracks, field edges and woods the tribe does
    "scout_overlap": 2.0,       # scouts sit over the trails and roads
    "lair_r": 0.12,             # km: walking this close to a lair by day meets its ring (P: it is lying up)
    "lair_avoid": 0.5,          # players who know where a lair is go round it
    "rel_speed": 1.27,          # moving groups meet a walker this much more often than standing ones
    "loud_per_km": 0.1,         # night: you make 30 heat inside a range this often
    "fog": 0.10, "rain": 0.15, "storm": 0.03,   # shares of days (fog) and nights (rain, storm)
    "silence": (0.3, 0.45, 0.55),   # a scout is silenced within 5 s (by group size)
    "pk_day": (0.90, 0.95, 0.97),   # each body in a fight is killed (by group size)
    "pk_night": (0.75, 0.85, 0.90),
    "pd_day": 0.015,            # each player dies, per body per player, by day
    "pd_night": 0.04,           # ...per wave, at night (at most 0.5)
    "alpha_core": (0.25, 0.35, 0.45),   # the alpha dies when the core is fought at night
    "alpha_hunts": 0.005,       # deliberate lair raids on a Prime-or-older alpha (a tribe of 8+), per player-hour
    "alpha_raid_win": 0.4,
    "lone_hunt": 0.1,           # a lone one is hunted down (Alpha Sighting), per season day
    "culls": 0.012,             # Bastion cull contracts done per player-hour (Collapse and Overrun)
    "cull_kills": (4, 8),
    "open_deaths": 0.15,        # people dying to the infected in the open, per real day (wanderers, crews, patrols)
    "player_open": 0.03,        # players dying to the infected (or turning) in the wild, per player-hour
    "walkouts": 0.08,           # Haven walk-outs a real day
    "wanderers": 6,             # the roster: 6, plus one per 6 settlements above 8, at most 10
    "wanderer_km": 4.0,         # km a wanderer walks in a night (crossings: Buffon, over the land)
    "trucks_day": 24,           # supply trucks a real day, all factions
    "truck_near": 0.1,          # share of night trucks that pass within 200 m of some hunting part
    "town_hunt": 0.6,           # night: a hunt finds you in one of its hunting towns, per hour there
    "town_edge": 0.3,           # day: a hungry party meets you at one of its towns' edges, per hour, per group
    "town_full": 40,            # below this infection a town has fewer crawlers: kills x infection / this
    "frenzy": 0.06,             # a counted night is a frenzy night (Every Ridge Calls)
    "cand_share": 0.15,         # share of far-enough wild 250 m cells that are lair candidates
    "life_ok": 0.9,             # a player death that counts (a life of 10 minutes or more)
}

# Wild land, holdings and lair candidates as the land survey measured them (wild land: the middle of
# what four to six starts gave; Arland 0.4-1.1, Everon 23.8-25.8, Cain 19.7-21.3 km2). The town count
# is the synthetic map's, chosen so its land comes out near the real one (Arland 8 km2 against 6.3,
# Everon 52 against 51, Cain 44 against 39); cands: the share of far-enough wild cells that are lair
# candidates, chosen to land near the survey's count (Everon 34-35; Cain 77-81, more than the
# synthetic map has room for). Chernarus-size is not measured.
MAPS = {
    "arland": {"label": "Arland", "wild": 0.75, "towns": 14, "holdings": 8, "safehouses": 2, "cands": 1.0},
    "everon": {"label": "Everon", "wild": 24.5, "towns": 45, "holdings": 14, "safehouses": 3, "cands": 0.55},
    "cain": {"label": "Cain", "wild": 20.5, "towns": 30, "holdings": 14, "safehouses": 3, "cands": 1.0},
    "chernarus": {"label": "Chernarus-size", "wild": 120.0, "towns": 90, "holdings": 12, "safehouses": 4},
}
TEMPERS = (("bold", 25), ("cautious", 20), ("cunning", 20), ("territorial", 20), ("nomadic", 15))
FACTIONS = ("haven", "bastion", "redhand", "halcyon")
STAGE_SPREAD = (0.01, 0.016, 0.02, 0.006)       # Director: spread per minute by stage
STAGE_PRESENCE = (1.0, 0.6, 0.25, 0.1)          # Bastion presence by stage
SPREAD_FROM = 60                                # Director: towns at this infection spread...
SPREAD_KM = 2.5                                 # ...to every town this close
KILL_INFECTION = 1.5
BORDER_BAND = 0.3                               # km of no wild along the line (warning zone + 200 m)
MAX_DAYS = 70
MAP_VARIANTS = 6


def poisson(rng, lam):
    if lam <= 0:
        return 0
    if lam > 30:
        return max(0, int(round(rng.gauss(lam, math.sqrt(lam)))))
    limit, k, p = math.exp(-lam), 0, 1.0
    while True:
        p *= rng.random()
        if p <= limit:
            return k
        k += 1


def binom(rng, n, p):
    return sum(1 for _ in range(n) if rng.random() < p)


def dist(a, b):
    return math.hypot(a[0] - b[0], a[1] - b[1])


# ----------------------------------------------------------------------------------------- the map
class Map:
    """A square of land: towns (some lived in), the wild between them, lair candidates in the wild.
    The side is solved so that the wild land comes out at the wild km2 asked for."""

    def __init__(self, key, md, wild, rng):
        self.key, self.label, self.W = key, md["label"], wild
        n = md["towns"]
        unit = []
        spacing = 0.6 / math.sqrt(n)
        tries = 0
        while len(unit) < n and tries < 50000:
            tries += 1
            x, y = rng.random(), rng.random()
            if all((x - a) ** 2 + (y - b) ** 2 >= spacing ** 2 for a, b in unit):
                unit.append((x, y))
        sizes = [rng.choices((1, 2, 3, 4), weights=(50, 30, 15, 5))[0] for _ in unit]
        order = sorted(range(len(unit)), key=lambda i: -sizes[i] - rng.random())
        holdings = order[:md["holdings"]]
        rest = [i for i in range(len(unit)) if i not in holdings]
        safe = rng.sample(rest, min(md["safehouses"], len(rest)))
        self.faction = {}
        for j, i in enumerate(holdings):
            self.faction[i] = FACTIONS[j % 4]
        for i in safe:
            self.faction[i] = "haven"
        self.sizes = sizes
        self.radius = [0.12 + 0.06 * sz for sz in sizes]
        self.lived = set(self.faction)
        self.side = self._solve(unit, wild)
        s = self.side
        self.towns = [(x * s, y * s) for x, y in unit]
        self.small = s * s < 25
        self.land = s * s
        # 250 m cells: wild ones (where the dead in the open lie) and lair candidates
        self.wild_cells, self.cands = [], []
        near_c, near_l = (0.4, 0.6) if self.small else (0.6, 0.8)
        g = int(s / 0.25)
        for i in range(g):
            for j in range(g):
                p = ((i + 0.5) * 0.25, (j + 0.5) * 0.25)
                if not self._wild(p[0], p[1], s, self.towns):
                    continue
                self.wild_cells.append(p)
                ok = True
                for k, t in enumerate(self.towns):
                    d = dist(p, t)
                    if d < near_c or (k in self.lived and d < near_l + self.radius[k]):
                        ok = False
                        break
                if ok and rng.random() < md.get("cands", ASSUME["cand_share"]):
                    self.cands.append(p)
        self.neigh = [[j for j, u in enumerate(self.towns) if j != i and dist(t, u) <= SPREAD_KM]
                      for i, t in enumerate(self.towns)]
        self.bastion = [self.towns[i] for i in self.lived if self.faction[i] == "bastion"]
        self.lived_pos = [self.towns[i] for i in self.lived]

    def _wild(self, px, py, s, towns):
        if px < BORDER_BAND or py < BORDER_BAND or px > s - BORDER_BAND or py > s - BORDER_BAND:
            return False
        for k, (x, y) in enumerate(towns):
            lim = self.radius[k] + (0.3 if k in self.lived else 0.1)
            dx = px - x
            if dx > lim or dx < -lim:
                continue
            dy = py - y
            if dx * dx + dy * dy < lim * lim:
                return False
        return True

    def _wild_area(self, unit, s, grid=48):
        towns = [(x * s, y * s) for x, y in unit]
        step = s / grid
        hit = 0
        for i in range(grid):
            for j in range(grid):
                if self._wild((i + 0.5) * step, (j + 0.5) * step, s, towns):
                    hit += 1
        return hit / (grid * grid) * s * s

    def _solve(self, unit, wild):
        lo, hi = 1.0, 80.0
        for _ in range(22):
            mid = (lo + hi) / 2
            if self._wild_area(unit, mid) < wild:
                lo = mid
            else:
                hi = mid
        return (lo + hi) / 2

    def range_radius(self, rng):
        return rng.uniform(0.35, 0.7) if self.small else rng.uniform(0.5, 1.0)


# -------------------------------------------------------------------------------------- the tribes
class Alpha:
    __slots__ = ("start", "health", "taken", "challenged", "great")

    def __init__(self, start):
        self.start, self.health, self.taken, self.challenged, self.great = start, 1.0, 0, False, False


class Tribe:
    _next = 0

    def __init__(self, sim, pos, grown, lead, how, mother=None, kept=False, r=None):
        Tribe._next += 1
        self.id = Tribe._next
        self.pos = pos
        self.r = r if r is not None else sim.map.range_radius(sim.rng)
        self.grown = grown
        self.young = []             # [season day joined, was a player]
        self.pending = []           # taken, back as young at the next dusk: was a player
        self.alpha = Alpha(sim.sd - lead)
        self.hunger = sim.P["hunger_start"]
        self.disturb, self.disturb_by = 0.0, {}
        names = [t for t, _ in TEMPERS]
        self.temper = sim.rng.choices(names, weights=[w for _, w in TEMPERS])[0]
        self.founded = sim.sd
        self.how = how
        self.mother, self.kept = mother, kept
        self.clan, self.clan_hold = None, 0
        self.bank, self.ran_today = 0.0, 0
        self.post_feed = False      # sleeps through this day (took someone last counted night)
        self.took = False           # took someone tonight
        self.starve = 0
        self.weak_until = -1.0
        self.restless = False
        self.vigil = False          # no hunt tonight: the alpha fell
        self.succession = False     # the guard fight is at the next dawn
        self.night = None
        self.first_night = None     # (members at dusk, lost) on its first counted night with bodies
        self.moved = -1
        self.party = 0              # day party bodies out today
        self.packs = 0              # day packs today
        self.reopened = 0           # counted nights its borders stay reopened
        self.last_wanderer = -99    # counted night index of its last wanderer taken
        self.alive = True
        self.peak = 0
        self.hunting = []           # hunting towns (not lived in) within its range + 500 m
        self.first_hunt = None      # share of its members lost on its first night with a hunt at players

    def members(self):
        return self.grown + len(self.young) + (1 if self.alpha else 0)


class Season:
    def __init__(self, mp, players, P, A, rng, legacy, tl, trace=False):
        self.map, self.n, self.P, self.A, self.rng, self.legacy = mp, players, P, A, rng, legacy
        pace = ps.LEGACY if legacy else ps.DEFAULTS
        self.speed = pace["season_speed"]
        self.stage_hours = pace["stage_hours"]
        self.sd_hours = sum(self.stage_hours) / 30.0
        # The old tuning moved infection as much per stage as today's: spread per minute x5
        self.spread_mult = sum(ps.DEFAULTS["stage_hours"]) / sum(self.stage_hours)
        self.tl = tl
        self.end = min(tl[100] * 60 if tl[100] else MAX_DAYS * 1440, MAX_DAYS * 1440)
        self.trace = trace
        self.played = 0.0           # hours played (x season speed): the stage and season-day clock
        self.sd = 0.0               # season days
        self.sd_int = 0
        self.stage = 0
        self.cured = False
        self.counted = 0            # counted nights so far
        self.towns = [min(100.0, max(0.0, 10 + 12 * sz + rng.uniform(-10, 15))) for sz in mp.sizes]
        self.dark = [0] * len(self.towns)
        self.town_mark = list(self.towns)
        self.walkers_today = [0.0] * len(self.towns)
        self.tribes = []
        self.lone = []              # [season day alone, health]
        self.splinters = []         # [count, origin tribe id, resolve season day, pos]
        self.vacuums = []           # [pos, until season day]
        self.open_dead = []         # [pos, season day]: taken outside every range
        self.borders = {}           # (id, id) -> [last fight season day, crossing]
        self.moves_today = 0
        self.new_today = False
        self.split_today = False
        self.last_frenzy = -99.0
        self.frenzy = False
        self.storm = False
        self.rain = False
        self.last_truck = -99
        self.wanderers_lost = []    # real minute each wanderer lost comes back
        self.displaced = {f: 0 for f in FACTIONS}
        self.gate_done = False
        self.share = P["share_cap"]
        self.bank_map = None
        self.banked = 0.0
        self.stats = {
            "tribes_by_day": {}, "clan": False, "clan_day": None, "great": False,
            "splits": 0, "successions": 0, "challenges": 0, "age_deaths": 0, "alpha_killed": 0,
            "lone": 0, "breakups": 0, "absorbed": 0, "new": {"seed": 0, "dark": 0, "taken": 0, "lone": 0, "split": 0},
            "starved": 0, "walkers": 0, "culls": 0, "displaced": 0, "moves": 0,
            "first": [], "first_wiped": 0, "first_half": 0, "first_hunt": [], "first_hunt_wiped": 0,
            "rate_active": [0.0, 0.0], "rate_fed": [0.0, 0.0], "rate_hungry": [0.0, 0.0], "clan_streak": {},
            "ran": 0, "taken_player": 0, "taken_data": 0, "taken_other": 0,
            "km_day": 0.0, "km_night": 0.0, "enc_day": 0, "enc_night": 0, "bodies_day": 0, "bodies_night": 0,
            "kills_day": 0, "kills_night": 0, "kills_other": 0, "ph": 0.0, "ph_day": 0.0, "ph_night": 0.0,
            "deaths_day": 0, "deaths_night": 0,
            "rate_day": [0.0, 0.0], "rate_night": [0.0, 0.0], "party": [0.0, 0.0], "r": [],
            "members": [], "hungry": [0, 0], "restless_days": 0, "border_fights": 0,
            "counted_per_sd": None, "max_night_loss": 0.0, "tribe_sd": 0, "town_inf": [],
        }
        self.player_taken_day = [-1] * players
        self.sessions = self._sessions()

    # ---------------------------------------------------------------------------------- the play model
    def _sessions(self):
        """pacing_sim's play model: each regular plays 2-3 hours a day at a random time in a 12-hour
        stretch. Each session also has a group size, a home (a lived-in place) and two towns."""
        rng, mp = self.rng, self.map
        lived = sorted(mp.lived)
        out = []
        for day in range(MAX_DAYS + 1):
            today = []
            for p in range(self.n):
                length = rng.uniform(*ps.SESSION_HOURS)
                start = rng.uniform(0, ps.PLAY_WINDOW_HOURS - length) + (24 - ps.PLAY_WINDOW_HOURS)
                k = rng.choices((1, 2, 3), weights=self.A["groups"])[0]
                k = min(k, self.n)
                home = mp.towns[rng.choice(lived)] if lived else (mp.side / 2, mp.side / 2)
                w = [mp.sizes[i] * math.exp(-dist(home, t) / 2.5) for i, t in enumerate(mp.towns)]
                towns = rng.choices(range(len(mp.towns)), weights=w, k=2)
                a = day * 1440 + start * 60
                today.append((a, a + length * 60, p, k, towns))
            out.append(today)
        return out

    def _online(self, a, b):
        sess = []
        for d in range(int(a // 1440), int((b - 1) // 1440) + 1):
            if d < len(self.sessions):
                sess.extend(self.sessions[d])
        per = []
        spans = []
        for s in sess:
            lo, hi = max(a, s[0]), min(b, s[1])
            if hi > lo:
                per.append((s, hi - lo))
                spans.append((lo, hi))
        spans.sort()
        total, end = 0.0, -1.0
        for lo, hi in spans:
            if hi <= end:
                continue
            total += hi - max(lo, end)
            end = hi
        return total, per

    # --------------------------------------------------------------------------------------- clocks
    def stage_of(self, t):
        if t.alpha is None:
            return 1
        d = self.sd - t.alpha.start
        if d < self.P["rising_days"] + 1:
            return 0
        if d < self.P["prime_days"] + 1:
            return 1
        return 2

    def advance(self, online):
        self.played += online / 60.0 * self.speed
        acc = 0.0
        for i, h in enumerate(self.stage_hours):
            acc += h
            if self.played < acc:
                self.stage = i
                break
        else:
            self.stage = 3
        self.sd = self.played / self.sd_hours
        while int(self.sd) > self.sd_int:
            self.sd_int += 1
            self.season_day()

    # ---------------------------------------------------------------------------------------- roles
    def roles(self, t):
        if t.alpha is None:
            gt = 3
        elif t.alpha.great:
            gt = 4
        else:
            gt = 2 if self.stage_of(t) == 0 else 3
        g = min(gt, t.grown)
        sc = min(2 if t.members() >= 15 else 1, t.grown - g)
        return g, sc, t.grown - g - sc, len(t.young)

    def lose(self, t, grown=0, young=0):
        young = min(young, len(t.young))
        for _ in range(young):
            t.young.pop(self.rng.randrange(len(t.young)))
        t.grown = max(0, t.grown - grown)

    def lose_mixed(self, t, n):
        """n members killed among hunters and young, in proportion."""
        _, _, h, y = self.roles(t)
        n = min(n, h + y)
        ky = binom(self.rng, n, y / (h + y)) if h + y else 0
        ky = min(ky, y)
        kh = min(n - ky, h)
        ky = min(n - kh, y)
        self.lose(t, kh, ky)
        return kh + ky

    def weak(self, t):
        return t.members() < self.P["weak_under"]

    # ---------------------------------------------------------------------------------------- slots
    def slots(self):
        P = self.P
        if self.slot_override is not None:
            n = self.slot_override
        else:
            n = int(round(self.map.W / P["slot_km2"]))
            n = max(0, min(15, n))
            if n == 0 and self.map.W >= P["slot_floor_km2"] and self.map.cands:
                n = 1
        kept = 0
        if n >= 13:
            kept = 3
        elif n >= 9:
            kept = 2
        elif n >= 3:
            kept = 1
        return n, kept

    def free_slot(self, split):
        n, kept = self.slots()
        used_kept = sum(1 for t in self.tribes if t.kept)
        used_open = len(self.tribes) - used_kept
        if split and used_kept < kept:
            return "kept"
        if used_open < n - kept:
            return "open"
        return None

    def kept_free(self):
        n, kept = self.slots()
        return sum(1 for t in self.tribes if t.kept) < kept

    def outside_ranges(self, p, margin=0.35, skip=None):
        return all(dist(p, t.pos) > t.r + margin for t in self.tribes if t is not skip)

    def room_ok(self, p):
        """Room kept for a split: no new tribe within room_km of a big tribe while a kept slot is free."""
        if not self.kept_free():
            return True
        return all(dist(p, t.pos) > self.P["room_km"] for t in self.tribes if t.members() >= self.P["room_big"])

    def free_cands(self, near=None, within=None):
        out = []
        for c in self.map.cands:
            if not self.outside_ranges(c):
                continue
            if near is not None and within is not None:
                d = dist(c, near)
                if d < within[0] or d > within[1]:
                    continue
            out.append(c)
        return out

    # -------------------------------------------------------------------------------------- founding
    def found(self, pos, grown, lead, how, kept=False, mother=None, r=None):
        t = Tribe(self, pos, grown, lead, how, mother=mother, kept=kept, r=r)
        self.tribes.append(t)
        self.stats["new"][how] += 1
        self.bank_map = None
        self.vacuums = [v for v in self.vacuums if dist(v[0], pos) > 1.0]
        return t

    def seed(self):
        n, kept = self.slots()
        if n <= 0 or not self.map.cands:
            return
        if self.P["seed_open"]:
            seeds = min(4, max(1, n - kept))
        else:
            seeds = min(4, -(-n // self.P["seeds_div"]), n - kept if n - kept > 0 else n)
        lab = self.map.lived_pos[0] if self.map.lived_pos else (0, 0)
        cands = list(self.map.cands)
        for i in range(seeds):
            free = [c for c in cands if self.outside_ranges(c, margin=0.8)]
            if not free:
                break
            if not self.tribes:
                # the column, near the line, and away from the lab
                side = self.map.side
                pick = max(free, key=lambda c: dist(c, lab) - 0.5 * min(c[0], c[1], side - c[0], side - c[1])
                           + self.rng.random())
            else:
                pick = max(free, key=lambda c: min(dist(c, t.pos) for t in self.tribes) + 0.3 * self.rng.random())
            size = self.rng.randint(*self.P["seed_size"])
            self.found(pick, size - 1, self.rng.uniform(*self.P["seed_lead"]), "seed")

    def try_new_tribe(self):
        """Rule 2, at most one a season day, into a free slot that isn't kept."""
        if self.cured or self.new_today or self.free_slot(False) != "open":
            return
        P = self.P
        # a town gone dark
        for i, inf in enumerate(self.towns):
            if self.dark[i] >= P["dark_days"]:
                pos = self.map.towns[i]
                cands = [c for c in self.free_cands() if self.room_ok(c)]
                if not cands:
                    continue
                c = min(cands, key=lambda c: dist(c, pos))
                if dist(c, pos) > 4.0:
                    continue
                t = self.found(c, P["dark_size"] - 1, 0, "dark")
                t.young = []
                self.dark[i] = 0
                self.new_today = True
                return
        # from the taken
        recent = [d for d in self.open_dead if self.sd - d[1] <= P["taken_found_days"]]
        self.open_dead = recent
        for d in recent:
            near = [e for e in recent if dist(e[0], d[0]) <= P["taken_found_km"]]
            need = P["taken_found"]
            if any(dist(v[0], d[0]) < 1.5 for v in self.vacuums):
                need = P["taken_found_vacuum"]
            if len(near) >= need:
                cands = [c for c in self.free_cands() if self.room_ok(c)]
                if not cands:
                    return
                c = min(cands, key=lambda c: dist(c, d[0]))
                self.found(c, len(near) - 1, 0, "taken")
                self.open_dead = [e for e in recent if e not in near]
                self.new_today = True
                return
        # around a lone one
        if self.lone and sum(s[0] for s in self.splinters) >= P["lone_gather"] and self.rng.random() < 0.3:
            cands = [c for c in self.free_cands() if self.room_ok(c)]
            if cands:
                c = self.rng.choice(cands)
                gathered = 0
                while self.splinters and gathered < P["lone_gather"]:
                    gathered += self.splinters.pop()[0]
                self.lone.pop(0)
                self.found(c, gathered, 0, "lone")
                self.new_today = True

    # -------------------------------------------------------------------------------------- leaving
    def remove(self, t, why):
        t.alive = False
        if t in self.tribes:
            self.tribes.remove(t)
        self.vacuums.append([t.pos, self.sd + self.P["vacuum_days"]])
        self.bank_map = None
        if why == "breakup":
            self.stats["breakups"] += 1

    def to_splinters(self, t, n):
        """n members leave as 1-2 loose packs of 3-6 (4 splinters on the server)."""
        while n >= 3 and len(self.splinters) < 4:
            k = min(n, self.rng.randint(3, 6))
            self.splinters.append([k, t.id, self.sd + self.rng.uniform(1, 2), t.pos])
            n -= k
        if n > 0:
            self.to_crawlers(t.pos, n)

    def to_crawlers(self, pos, n):
        """Loose ones and starving walkers end up with the nearest town's crawlers (+2 each, capped)."""
        if self.cured or not self.towns:
            return
        i = min(range(len(self.towns)), key=lambda k: dist(pos, self.map.towns[k]))
        add = min(2 * n, self.P["starve_town_cap"] - self.walkers_today[i])
        if add > 0:
            self.towns[i] = min(100.0, self.towns[i] + add)
            self.walkers_today[i] += add
        self.stats["walkers"] += n

    def breakup(self, t):
        if t.alpha and len(self.lone) < self.P["lone_max"]:
            self.lone.append([self.sd, t.alpha.health])
            self.stats["lone"] += 1
        self.to_splinters(t, t.grown + len(t.young))
        self.remove(t, "breakup")

    def check_breakup(self, t):
        if t.alive and t.members() < self.P["breakup_under"] and self.sd - t.founded >= self.P["never_break_days"]:
            self.breakup(t)

    def kill_alpha(self, t, why):
        if t.alpha is None:
            return
        t.alpha = None
        t.vigil = True
        t.succession = True
        if why == "players":
            self.stats["alpha_killed"] += 1

    def guard_fight(self, t):
        """Rule 15 at dawn: the strongest guard leads; one other guard dies or leaves; splinters."""
        P, rng = self.P, self.rng
        t.succession = False
        self.stats["successions"] += 1
        g, _, _, _ = self.roles(t)
        if t.grown <= 0 or g <= 0:
            self.to_splinters(t, t.grown + len(t.young))
            self.remove(t, "breakup")
            return
        t.grown -= 1
        t.alpha = Alpha(self.sd)
        if t.grown > 0:
            t.grown -= 1
            if rng.random() < P["lone_leave"] and len(self.lone) < P["lone_max"]:
                self.lone.append([self.sd, 0.8])
                self.stats["lone"] += 1
        n = int(round(rng.uniform(*P["splinter_share"]) * (t.grown + len(t.young))))
        n = min(n, 12)
        if n < 3:
            n = 0               # splinters leave as packs of 3-6: too few to make one stay
        kp = self.lose_mixed(t, n)
        if kp < n:
            extra = min(n - kp, max(0, t.grown - 2))
            self.lose(t, extra, 0)
            kp += extra
        self.to_splinters(t, kp)
        t.weak_until = self.sd + P["weak_days"]
        t.reopened = 2
        self.check_breakup(t)

    # ---------------------------------------------------------------------------------- season days
    def season_day(self):
        P, rng = self.P, self.rng
        self.new_today = False
        self.split_today = False
        self.moves_today = 0
        self.walkers_today = [0.0] * len(self.towns)
        sd = self.sd_int
        for t in list(self.tribes):
            t.ran_today = 0
            t.disturb = max(0.0, t.disturb - P["disturb_decay"])
            if t.disturb <= 0:
                t.disturb_by = {}
            # the young grow up
            keep = []
            for y in t.young:
                if sd - y[0] >= P["young_days"]:
                    t.grown += 1
                else:
                    keep.append(y)
            t.young = keep
            # ageing, challenges and death of age
            if t.alpha and self.stage_of(t) == 2:
                into = self.sd - t.alpha.start - P["prime_days"] - 1
                chance = P["challenge"] * (2 if t.restless else 1)
                g, _, _, _ = self.roles(t)
                if g > 0 and rng.random() < chance:
                    self.stats["challenges"] += 1
                    t.alpha.challenged = True
                    win = min(P["challenge_win_max"], P["challenge_win"] + P["challenge_win_day"] * max(0, into))
                    t.grown -= 1
                    if rng.random() < win:
                        if len(self.lone) < P["lone_max"] and t.alpha.health >= 0.3:
                            self.lone.append([self.sd, t.alpha.health])
                            self.stats["lone"] += 1
                        t.alpha = Alpha(self.sd)
                        t.reopened = 2
                    elif len(self.lone) < P["lone_max"]:
                        self.lone.append([self.sd, 0.8])
                        self.stats["lone"] += 1
                elif into >= P["age_death"] and not t.alpha.challenged and not t.alpha.great:
                    self.stats["age_deaths"] += 1
                    self.kill_alpha(t, "age")
            t.peak = max(t.peak, t.members())
        # lone ones: hunted down, or die after their days alone
        keep = []
        for lo in self.lone:
            if sd - lo[0] >= P["lone_days"] or rng.random() < self.A["lone_hunt"] * min(1.0, self.n / 5):
                continue
            keep.append(lo)
        self.lone = keep
        # splinters: rejoin, join the nearest tribe, or end with the crawlers
        keep = []
        for sp in self.splinters:
            if sp[2] > self.sd:
                keep.append(sp)
                continue
            roll = rng.random()
            home = [t for t in self.tribes if t.id == sp[1]]
            if roll < 0.5 and home:
                self.add_grown(home[0], sp[0])
            elif roll < 0.75 and self.tribes:
                near = min(self.tribes, key=lambda t: dist(t.pos, sp[3]))
                self.add_grown(near, sp[0])
            else:
                self.to_crawlers(sp[3], sp[0])
        self.splinters = keep
        self.vacuums = [v for v in self.vacuums if v[1] > self.sd]
        self.try_new_tribe()
        self.try_splits()
        self.clans()
        self.record()

    def add_grown(self, t, n):
        if self.cured:
            return
        room = self.P["max_members"] - t.members()
        t.grown += max(0, min(n, room))

    def try_splits(self):
        """Rule 11: one split a season day on the server, into a free slot (a kept one first) with room."""
        P, rng = self.P, self.rng
        for t in sorted(self.tribes, key=lambda x: -x.members()):
            if t.members() < P["split_at"]:
                t.restless = False
                continue
            if self.cured:
                continue
            slot = self.free_slot(True)
            room = None
            weak_host = None
            if slot and not self.split_today:
                cands = [c for c in self.free_cands(near=t.pos, within=P["split_room"])]
                if cands:
                    room = rng.choice(cands)
                else:
                    for o in self.tribes:
                        if o is t or o.members() >= 10 or not self.bordering(t, o):
                            continue
                        inside = [c for c in self.map.cands
                                  if dist(c, o.pos) <= o.r and dist(c, t.pos) >= P["split_room"][0]]
                        if inside:
                            room, weak_host = rng.choice(inside), o
                            break
            if room is None:
                t.restless = True
                continue
            if weak_host is not None:
                self.displace(weak_host, "tribes", forced=True)
            g, sc, h, y = self.roles(t)
            share = P["split_share"]
            dh = int(round(h * share))
            dy = int(round(y * share))
            take_sc = 1 if sc > 1 else 0
            self.lose(t, dh + take_sc + 1, 0)
            young = []
            for _ in range(dy):
                young.append(t.young.pop(rng.randrange(len(t.young))))
            d = self.found(room, dh + take_sc + (1 if g > 1 else 0), 0, "split", kept=(slot == "kept"), mother=t.id)
            if g > 1:
                t.grown -= 1
            d.young = young
            d.hunger = t.hunger
            d.temper = t.temper
            t.restless = False
            t.reopened = 2
            self.split_today = True
            self.stats["splits"] += 1

    def bordering(self, a, b):
        return dist(a.pos, b.pos) - a.r - b.r <= self.P["border_km"]

    def clans(self):
        """A daughter is clan once both halves have held clan_hold+ for clan_days season days."""
        P = self.P
        by_id = {t.id: t for t in self.tribes}
        for t in self.tribes:
            m = by_id.get(t.mother)
            if m is None or dist(m.pos, t.pos) > P["clan_km"]:
                t.clan_hold = 0
                t.clan = None
                continue
            if t.members() >= P["clan_hold"] and m.members() >= P["clan_hold"]:
                t.clan_hold += 1
            else:
                t.clan_hold = 0
            if t.clan_hold >= P["clan_days"]:
                t.clan = m.clan or m.id
                m.clan = t.clan
                if not self.stats["clan"]:
                    self.stats["clan"] = True
                    self.stats["clan_day"] = self.sd
        # the great alpha: a clan of 3+ that holds together 8 season days, from Overrun
        clans = {}
        for t in self.tribes:
            if t.clan:
                clans.setdefault(t.clan, []).append(t)
        streak = self.stats["clan_streak"]
        for cid in list(streak):
            if cid not in clans or len(clans[cid]) < 3:
                del streak[cid]
        for cid, members in clans.items():
            if len(members) >= 3:
                streak[cid] = streak.get(cid, 0) + 1
                if streak[cid] >= 8 and self.stage >= 2 and not any(t.alpha and t.alpha.great for t in self.tribes):
                    old = [t for t in members if t.alpha and self.stage_of(t) >= 1]
                    if old:
                        max(old, key=lambda t: self.sd - t.alpha.start).alpha.great = True
                        self.stats["great"] = True

    def record(self):
        day = int(self.now // 1440) + 1
        self.stats["tribes_by_day"][day] = len(self.tribes)
        self.stats["tribe_sd"] += len(self.tribes)
        self.stats["town_inf"].append(sum(self.towns) / max(1, len(self.towns)))
        for t in self.tribes:
            self.stats["members"].append(t.members())
            if t.restless:
                self.stats["restless_days"] += 1

    # ------------------------------------------------------------------------------ disturbance, moves
    def disturb(self, t, amount, who):
        if amount <= 0:
            return
        t.disturb += amount
        t.disturb_by[who] = t.disturb_by.get(who, 0) + amount
        if t.disturb >= 100 and t.moved < self.sd_int and self.moves_today < self.P["moves_per_day"]:
            cause = max(t.disturb_by, key=t.disturb_by.get)
            self.displace(t, cause)

    def displace(self, t, cause, forced=False):
        """Rule 10 and 17: the lair moves; displaced by a faction, 2 km or more from its holdings."""
        P = self.P
        t.moved = self.sd_int
        t.disturb, t.disturb_by = 0.0, {}
        self.moves_today += 1
        self.stats["moves"] += 1
        if cause in self.displaced and self.displaced[cause] >= P["displace_cap"] and not forced:
            return          # it just moves within its range
        hold = self.map.bastion if cause == "bastion" else []
        cands = []
        for c in self.map.cands:
            d = dist(c, t.pos)
            if d < 1.5 or d > 4.0:
                continue
            if not self.outside_ranges(c, skip=t):
                continue
            if any(dist(c, h) < P["displace_km"] for h in hold):
                continue
            if any(dist(c, l) < 1.2 for l in self.map.lived_pos):
                continue
            cands.append(c)
        if not cands:
            return
        self.vacuums.append([t.pos, self.sd + P["vacuum_days"]])
        t.pos = self.rng.choice(cands)
        self.bank_map = None
        if cause in self.displaced:
            self.displaced[cause] += 1
        self.stats["displaced"] += 1

    # ------------------------------------------------------------------------------------- the taken
    def taken(self, t, player=None, kind="data"):
        """Rule 4: back as a young at the next dusk; hunger -25. Halved from the 75% serum; none after the cure."""
        if self.cured:
            return
        if player is not None:
            if self.rng.random() > self.A["life_ok"] or self.player_taken_day[player] == self.sd_int:
                return
            if sum(1 for y in t.young if y[1]) + sum(1 for p in t.pending if p) >= self.P["players_young_max"]:
                return
            self.player_taken_day[player] = self.sd_int
        t.hunger = max(0, t.hunger - self.P["hunger_taken"])
        t.took = True
        if t.alpha:
            t.alpha.taken += 1
        if self.serum and self.rng.random() < 0.5:
            return
        t.pending.append(player is not None)
        key = "taken_player" if player is not None else ("taken_data" if kind == "data" else "taken_other")
        self.stats[key] += 1

    def open_death(self, pos):
        """Someone dies to the infected in the open: the nearest tribe within 1.5 x its radius takes them."""
        near = [t for t in self.tribes if dist(t.pos, pos) <= 1.5 * t.r]
        if near:
            self.taken(min(near, key=lambda t: dist(t.pos, pos)), kind="other")
        elif not self.cured:
            self.open_dead.append([pos, self.sd])

    # ------------------------------------------------------------------------------------------ towns
    def town_tick(self, online, per, kills_town):
        """The Director's spread as coded, the kill infection, and the ones who ran."""
        mp = self.map
        mult = 1.0
        if self.cured:
            mult = 0.2
        elif self.tl[75] is not None and self.now >= self.tl[75] * 60:
            mult = 0.55
        elif self.tl[25] is not None and self.now >= self.tl[25] * 60:
            mult = 0.85
        spread = STAGE_SPREAD[self.stage] * online * mult * self.speed * self.spread_mult
        gross = [spread * 0.25] * len(self.towns)
        for i, inf in enumerate(self.towns):
            if inf >= SPREAD_FROM:
                for j in mp.neigh[i]:
                    gross[j] += spread * inf / 100.0
        # kills: each session's town minutes, split over its two towns
        killed = [0.0] * len(self.towns)
        tot_min = sum(m for _, m in per) or 1.0
        full = self.A["town_full"]
        for (s, m) in per:
            share = kills_town * m / tot_min
            for i in s[4]:
                killed[i] += share / 2 * min(1.0, self.towns[i] / full)
        self.ensure_maps()
        ran_mult = 0.0 if self.cured else (0.5 if self.serum else 1.0)
        for i in range(len(self.towns)):
            rise = min(100.0, self.towns[i] + gross[i]) - self.towns[i]
            self.towns[i] = max(0.0, min(100.0, self.towns[i] + gross[i] - killed[i] * KILL_INFECTION))
            t = self.bank_map[i]
            if self.P["ran_highs"]:
                # a town pushed back that fills up again sends nobody new: only new highs bank
                if self.towns[i] > self.town_mark[i]:
                    gain = self.towns[i] - self.town_mark[i]
                    self.town_mark[i] = self.towns[i]
                    if t is not None and t.alive and ran_mult > 0:
                        t.bank = min(self.P["ran_bank"], t.bank + gain * ran_mult)
                        self.banked += gain * ran_mult
                continue
            if t is not None and t.alive and ran_mult > 0 and rise > 0:
                # rises bank, falls don't: the minutes with a kill in the town make a fall, not a rise
                quiet = max(0.0, 1.0 - killed[i] / max(1.0, online))
                t.bank = min(self.P["ran_bank"], t.bank + rise * quiet * ran_mult)
                self.banked += rise * quiet * ran_mult

    def ensure_maps(self):
        """Which tribe each town banks to (the ones who ran), and each tribe's hunting towns."""
        if self.bank_map is not None:
            return
        mp = self.map
        self.bank_map = []
        for p in mp.towns:
            best, bd = None, 1e9
            for t in self.tribes:
                d = dist(p, t.pos) - t.r
                if d <= self.P["ran_km"] and d < bd:
                    best, bd = t, d
            self.bank_map.append(best)
        for t in self.tribes:
            t.hunting = [i for i, p in enumerate(mp.towns) if i not in mp.lived and dist(p, t.pos) <= t.r + 0.5]

    def dark_towns(self):
        """Once a season day: count the days each town has been dark with no range near."""
        for i, inf in enumerate(self.towns):
            p = self.map.towns[i]
            if inf >= self.P["dark_infection"] and all(dist(p, t.pos) - t.r > self.P["dark_km"] for t in self.tribes):
                self.dark[i] += 1
            else:
                self.dark[i] = 0

    # ------------------------------------------------------------------------------------- encounters
    def pk(self, k, night):
        return self.A["pk_night" if night else "pk_day"][min(k, 3) - 1]

    def player_deaths(self, t, k, risk, players):
        died = 0
        for p in players:
            if self.rng.random() < min(0.5, risk):
                died += 1
                self.taken(t, player=p)
        return died

    def cluster(self, per):
        """A player cluster in the wild: its size and who's in it."""
        if not per:
            return 1, [0]
        weights = [m * self.wild_share for (_, m) in per]
        s, _ = self.rng.choices(per, weights=weights)[0]
        k = s[3]
        others = [x[0][2] for x in per if x[0][2] != s[2]]
        self.rng.shuffle(others)
        return k, [s[2]] + others[:k - 1]

    def day_party(self, t):
        """The author's change: hunters and young out by day."""
        P = self.P
        if t.post_feed or self.weak(t) or t.alpha is None and t.succession:
            return 0
        _, _, h, y = self.roles(t)
        share = P["day_share"] * (P["day_hungry"] if t.hunger >= 60 else 1.0)
        return min(h + y, int(share * t.members()))

    def day_rate(self, t):
        """Meetings per km walked inside the range by day: the day party's groups, the scouts, the ring."""
        A = self.A
        area = math.pi * t.r * t.r * (1.25 if t.hunger >= 60 else 1.0)
        groups = 0.0
        if t.party > 0:
            groups = t.party / ((self.P["day_group"][0] + self.P["day_group"][1]) / 2.0)
            groups = max(1.0, groups)
        party = groups * 2 * A["P"] * A["rel_speed"] * A["overlap"] / area
        _, sc, _, _ = self.roles(t)
        if self.weak(t) and self.P["weak_quiet"]:
            sc = 0
        scouts = 0.0 if t.post_feed else sc * 2 * A["scout_day"] * (0.6 if self.rain else 1.0) * A["scout_overlap"] / area
        lair = 2 * A["lair_r"] * A["lair_avoid"] / (math.pi * t.r * t.r)
        return party, scouts, lair

    def night_rate(self, t, scouts_only=False):
        A, P = self.A, self.P
        mult = 1.0
        if self.frenzy:
            mult = 2.0
        elif t.hunger >= 85:
            mult = 1.5
        elif t.hunger >= 60:
            mult = 1.25
        area = math.pi * t.r * t.r * mult
        g, sc, h, y = self.roles(t)
        scouts = sc * 2 * A["scout_night"] * A["scout_overlap"] / area
        if scouts_only:
            return 0.0, scouts
        groups = math.ceil(h / 4.0) + (1 if y else 0) + (1 if self.core_goes(t) else 0)
        trails = 3 if self.frenzy else (2 if t.hunger >= 60 else 1)
        if self.sd < t.weak_until:
            trails = 1
        hunt = groups * 2 * A["trail_r"] * A["rel_speed"] * A["overlap"] / area * min(1.0, 0.6 + 0.2 * trails)
        hunt += A["loud_per_km"]
        if self.stage_of(t) == 2:
            hunt *= 0.65        # home by 02:00
        if self.rain:
            hunt *= 0.75        # home by 03:00
        return hunt, scouts

    def core_goes(self, t):
        if t.alpha is None or t.alpha.health < 0.5:
            return False
        cautious = t.temper == "cautious" or self.stage_of(t) == 0
        return not cautious or t.hunger >= 60

    def bodies_out(self, t):
        return not (t.vigil or self.storm or self.weak(t))

    def encounters(self, phase, online, per, seg_len):
        """Players' wild minutes inside ranges meet the tribes' bodies. Returns kills in the wild."""
        A = self.A
        if online <= 0 or not self.tribes:
            return 0
        key = {"day": "wild_day", "dawn": "wild_day", "dusk": "wild_dusk", "night": "wild_night"}[phase]
        # more of the map wild, more of the travel through it (Everon, half wild, is x1)
        self.wild_share = A[key] * min(1.5, max(0.5, self.map.W / self.map.land / A["wild_ref"]))
        km = 0.0
        for (s, m) in per:
            km += m * self.wild_share * A["foot"] * A["walk_kmh"] / 60.0 / s[3]
        total_area = sum(math.pi * t.r * t.r for t in self.tribes)
        denom = max(self.map.W, total_area)
        kills = 0
        mean_group = sum(x * w for x, w in zip((1, 2, 3), A["groups"]))
        nclusters = max(1, int(round(sum(m for _, m in per) / seg_len / mean_group)))
        self.share = min(self.P["share_cap"], 100.0 / nclusters)
        self.ensure_maps()
        town_min = {}
        tkey = {"day": "towns_day", "dawn": "towns_day", "dusk": "towns_dusk", "night": "towns_night"}[phase]
        for (s, m) in per:
            for i in s[4]:
                town_min[i] = town_min.get(i, 0.0) + m * A[tkey] / 2 / s[3]
        for t in list(self.tribes):
            if not t.alive:
                continue
            hours = sum(town_min.get(i, 0.0) for i in t.hunting) / 60.0
            if hours > 0:
                kills += self.town_meet(t, phase, hours, per)
            km_t = km * math.pi * t.r * t.r / denom
            if phase == "day":
                fog = A["fog"]
                kills += self.day_meet(t, km_t * (1 - fog), per)
                kills += self.night_meet(t, km_t * fog, per, daylight=True)
            elif phase == "dusk":
                kills += self.dusk_meet(t, km_t * 0.5, per)
                kills += self.night_meet(t, km_t * 0.5, per, daylight=False)
            elif phase == "dawn":
                kills += self.night_meet(t, km_t * 0.5, per, daylight=True)
            else:
                kills += self.night_meet(t, km_t, per, daylight=False)
        return kills

    def day_meet(self, t, km, per):
        party, scouts, lair = self.day_rate(t)
        st = self.stats
        st["km_day"] += km
        kills = 0
        n_party = poisson(self.rng, party * km)
        n_scout = poisson(self.rng, scouts * km)
        n_lair = poisson(self.rng, lair * km)
        for _ in range(n_party):
            if t.party <= 0:
                break
            k, who = self.cluster(per)
            w = min(t.party, self.rng.randint(*self.P["day_group"]), self.P["day_cluster_cap"])
            kills += self.fight(t, w, k, who, night=False, from_party=True)
        for _ in range(n_scout):
            k, who = self.cluster(per)
            kills += self.scout_call(t, k, who)
        for _ in range(n_lair):
            k, who = self.cluster(per)
            kills += self.lair_fight(t, k, who, raid=False)
        return kills

    def town_meet(self, t, phase, hours, per):
        """Players in a tribe's hunting towns: its hunt comes through at night; by day only a hungry
        party comes to the town's edge (towns by day stay the crawlers')."""
        A = self.A
        kills = 0
        if phase in ("night", "dusk"):
            if not self.bodies_out(t):
                return 0
            if t.night is None:
                self.set_out(t)
            trails = 3 if self.frenzy else (2 if t.hunger >= 60 else 1)
            lam = A["town_hunt"] * hours * min(1.0, 0.6 + 0.2 * trails) * (0.5 if phase == "dusk" else 1.0)
            for _ in range(poisson(self.rng, lam)):
                if t.night["broken"]:
                    break
                k, who = self.cluster(per)
                kills += self.hunt(t, k, who, daylight=False)
            return kills
        if phase == "day" and t.hunger >= 60 and t.party > 0:
            groups = max(1.0, t.party / ((self.P["day_group"][0] + self.P["day_group"][1]) / 2.0))
            for _ in range(poisson(self.rng, A["town_edge"] * groups * hours)):
                if t.party <= 0:
                    break
                k, who = self.cluster(per)
                w = min(t.party, self.rng.randint(*self.P["day_group"]), self.P["day_cluster_cap"])
                kills += self.fight(t, w, k, who, night=False, from_party=True)
        return kills

    def dusk_meet(self, t, km, per):
        """Gathering: the party is home; scouts at their posts; a call brings a hunter group."""
        if not self.bodies_out(t) and not self.weak(t):
            return 0
        if self.weak(t) and self.P["weak_quiet"]:
            return 0
        _, scouts = self.night_rate(t, scouts_only=True)
        scouts *= self.A["scout_dusk"] / self.A["scout_night"]
        self.stats["km_night"] += km
        kills = 0
        for _ in range(poisson(self.rng, scouts * km)):
            k, who = self.cluster(per)
            if self.weak(t):
                kills += self.fight(t, 1, k, who, night=False, scout=True)
            elif self.rng.random() < self.A["silence"][min(k, 3) - 1]:
                kills += self.fight(t, 1, k, who, night=False, scout=True, silenced=True)
            else:
                kills += self.hunt(t, k, who, daylight=True)
        return kills

    def scout_call(self, t, k, who):
        """By day: a scout sees you; silenced or not; a call brings at most 4 of the day party."""
        P = self.P
        if self.rng.random() < self.A["silence"][min(k, 3) - 1]:
            return self.fight(t, 1, k, who, night=False, scout=True, silenced=True)
        kills = self.fight(t, 1, k, who, night=False, scout=True)
        if self.weak(t) or t.packs >= P["day_packs"] or t.party <= 0:
            return kills
        t.packs += 1
        w = min(P["day_scout_pack"], t.party, P["day_cluster_cap"] - 1)
        return kills + self.fight(t, w, k, who, night=False, from_party=True, count=False)

    def fight(self, t, w, k, who, night, from_party=False, scout=False, silenced=False, count=True):
        """w bodies against a cluster of k: the players kill some, and may die."""
        if w <= 0:
            return 0
        st = self.stats
        if count:
            st["enc_night" if night else "enc_day"] += 1
        st["bodies_night" if night else "bodies_day"] += w
        if silenced:
            kills = 1
        else:
            kills = binom(self.rng, w, self.pk(k, night))
        if scout:
            _, sc, _, _ = self.roles(t)
            ks = min(kills, sc)
            self.lose(t, ks, 0)
            kills = ks + self.lose_mixed(t, kills - ks) if kills > ks else ks
        else:
            kills = self.lose_mixed(t, kills)
        if from_party:
            t.party = max(0, t.party - w if not night else t.party)
            t.party = max(0, t.party)
        if not silenced:
            risk = (self.A["pd_night"] if night else self.A["pd_day"]) * w / k
            st["deaths_night" if night else "deaths_day"] += self.player_deaths(t, k, risk, who)
        st["kills_night" if night else "kills_day"] += kills
        return kills

    def lair_fight(self, t, k, who, raid):
        """The ring round the lair by day. A raid also fights the guard and the alpha."""
        g, _, h, y = self.roles(t)
        ring = max(0, h + y - t.party)
        w = min(ring, 4 if not raid else 6)
        kills = self.fight(t, w, k, who, night=False) if w else 0
        self.disturb(t, 10, "players")
        if raid and t.alpha:
            kg = binom(self.rng, g, self.pk(k, False) * 0.8)
            self.lose(t, kg, 0)
            self.stats["kills_day"] += kg
            kills += kg
            self.player_deaths(t, k, self.A["pd_day"] * 2 * (g + 1) / k, who)
            self.disturb(t, self.P["disturb_guard"] * kg, "players")
            if t.alive and self.rng.random() < self.A["alpha_raid_win"]:
                self.kill_alpha(t, "players")
                kills += 1
                self.stats["kills_day"] += 1
        self.check_breakup(t)
        return kills

    def night_meet(self, t, km, per, daylight):
        if km <= 0:
            return 0
        self.stats["km_night"] += km
        if t.vigil or self.storm:
            return 0
        kills = 0
        if self.weak(t):
            if self.P["weak_quiet"]:
                return 0
            _, scouts = self.night_rate(t, scouts_only=True)
            for _ in range(poisson(self.rng, scouts * km)):
                k, who = self.cluster(per)
                kills += self.fight(t, 1, k, who, night=not daylight, scout=True)
            return kills
        if t.night is None:
            self.set_out(t)
        hunt, scouts = self.night_rate(t)
        for _ in range(poisson(self.rng, (hunt + scouts) * km)):
            if t.night["broken"] or not t.alive:
                break
            k, who = self.cluster(per)
            if t.night["hunts"] >= max(1, len(per)):     # one hunt per player cluster a night
                break
            kills += self.hunt(t, k, who, daylight)
        return kills

    def set_out(self, t):
        g, sc, h, y = self.roles(t)
        core = (g + 1) if self.core_goes(t) else 0
        t.night = {"set_out": h + y + core, "lost": 0, "broken": False, "hunts": 0, "dusk": t.members()}

    def hunt(self, t, k, who, daylight):
        """A hunt at one player cluster: waves, the core held back, and the night's commitment."""
        P, rng = self.P, self.rng
        if t.night is None:
            self.set_out(t)
        tn = t.night
        if tn["broken"]:
            return 0
        tn["hunts"] += 1
        self.stats["enc_night"] += 1
        scale = self.share / P["share_cap"]
        first = max(3, int(round(P["first_wave"] * scale)))
        presence = max(first, int(round(P["presence"] * scale)))
        stage = self.stage_of(t)
        bold = t.temper == "bold" and stage != 0
        cautious = t.temper == "cautious" or stage == 0
        frac = P["break_cautious"] if cautious else (P["break_bold"] if bold else P["break_share"])
        waves = 1 if cautious else P["waves"]
        night = not daylight
        brought = 0
        kills = 0
        core_in = False
        allow = frac * tn["set_out"]                # it breaks off once it has lost this many
        cap_total = P["commit_cap"] * tn["set_out"] if P["commit_cap"] else None
        for _ in range(waves):
            g, sc, h, y = self.roles(t)
            cap = first
            core_now = bold and not core_in and self.core_goes(t)
            if cap_total is not None:
                # no wave (and no core) goes in that could take the night's losses past the cap
                room = int(math.floor(cap_total - tn["lost"] + 1e-9))
                if core_now and room - (g + 1) >= 2:
                    room -= g + 1
                else:
                    core_now = False
                cap = min(first, room)
            hg = min(h, rng.randint(3, 5), cap)
            yg = min(y, 3, max(0, cap - hg))
            w = min(hg + yg, presence - brought)
            if w <= 0:
                break
            brought += w
            killed = self.fight(t, w, k, who, night=night, count=False)
            kills += killed
            tn["lost"] += killed
            if cap_total is not None:
                late = (not bold and w - killed <= 2 and not core_in and self.core_goes(t)
                        and tn["lost"] < allow and tn["lost"] + g + 1 <= cap_total)
                core_go = core_now or late
            else:
                core_go = (bold or w - killed <= 2) and not core_in and self.core_goes(t) and tn["lost"] < allow
            if core_go:
                core_in = True
                kills += self.core_fight(t, k, who, night)
            if tn["lost"] >= allow or not t.alive or t.alpha is None:
                tn["broken"] = True
                break
        self.stats["max_night_loss"] = max(self.stats["max_night_loss"], tn["lost"] / max(1, tn["dusk"]))
        return kills

    def core_fight(self, t, k, who, night):
        g, _, _, _ = self.roles(t)
        kg = binom(self.rng, g, self.pk(k, night) * 0.8)
        self.lose(t, kg, 0)
        t.night["lost"] += kg
        self.stats["kills_night" if night else "kills_day"] += kg
        self.stats["bodies_night" if night else "bodies_day"] += g + 1
        self.player_deaths(t, k, (self.A["pd_night"] if night else self.A["pd_day"]) * 1.5 * (g + 1) / k, who)
        if t.alpha and self.rng.random() < self.A["alpha_core"][min(k, 3) - 1]:
            self.kill_alpha(t, "players")
            self.stats["kills_night" if night else "kills_day"] += 1
            return kg + 1
        return kg

    # ------------------------------------------------------------------------------------ the phases
    def start_day(self):
        for t in self.tribes:
            t.party = self.day_party(t)
            t.packs = 0
            self.stats["party"][0] += t.party / max(1, t.members())
            self.stats["party"][1] += 1

    def dusk(self):
        """The young arrive (the taken and the ones who ran); tonight's weather and frenzy."""
        P = self.P
        for t in self.tribes:
            for was_player in t.pending:
                if t.members() < P["max_members"]:
                    t.young.append([self.sd, was_player])
            t.pending = []
            if not self.cured:
                n = int(t.bank // P["ran_points"])
                n = min(n, P["ran_per_day"] - t.ran_today)
                for _ in range(max(0, n)):
                    t.bank -= P["ran_points"]
                    t.ran_today += 1
                    self.stats["ran"] += 1
                    if t.members() < P["max_members"]:
                        t.young.append([self.sd, False])
            t.night = None
            t.took = False
            t.party = 0
        self.rain = self.rng.random() < self.A["rain"]
        self.storm = self.rng.random() < self.A["storm"]
        self.frenzy = False

    def night_end(self, counted):
        """05:00. A counted night: hunger, data takings, border fights, starvation, post-feed sleep."""
        P, A, rng = self.P, self.A, self.rng
        if counted:
            self.counted += 1
            if (self.sd >= 2 and self.sd - self.last_frenzy >= P["frenzy_space"] and rng.random() < A["frenzy"]):
                self.frenzy = True
                self.last_frenzy = self.sd
            self.data_takings()
            self.border_fights()
        for t in list(self.tribes):
            if t.night is not None and t.night["hunts"] > 0 and t.first_hunt is None:
                t.first_hunt = t.night["lost"] / max(1, t.night["dusk"])
                self.stats["first_hunt"].append(t.first_hunt)
                if t.members() < P["breakup_under"] or not t.alive:
                    self.stats["first_hunt_wiped"] += 1
            if counted:
                if t.first_night is None and not self.weak(t) and not t.vigil and t.night is not None:
                    t.first_night = (t.night["dusk"], t.night["lost"])
                    self.stats["first"].append(t.night["lost"] / max(1, t.night["dusk"]))
                    if t.members() < P["breakup_under"] or not t.alive:
                        self.stats["first_wiped"] += 1
                    if t.night["lost"] * 2 > t.night["dusk"]:
                        self.stats["first_half"] += 1
                rise = P["hunger_decline"] if self.stage_of(t) == 2 else P["hunger_night"]
                if t.restless:
                    rise += P["hunger_restless"]
                t.hunger = min(100, t.hunger + rise)
                self.stats["hungry"][0] += 1 if t.hunger >= 60 else 0
                self.stats["hungry"][1] += 1
                if t.hunger >= 100:
                    t.starve += 1
                    if t.starve >= 2:
                        n = max(2, int(P["starve_share"] * t.members()))
                        n = self.lose_mixed(t, n)
                        self.stats["starved"] += 1
                        if not self.cured and self.towns:
                            self.to_crawlers(t.pos, n)
                        t.hunger = P["starve_reset"]
                        t.starve = 0
                else:
                    t.starve = 0
                t.post_feed = t.took
                if t.reopened > 0:
                    t.reopened -= 1
            else:
                t.post_feed = False
            t.vigil = False if not t.succession else t.vigil
            self.check_breakup(t)

    def data_takings(self):
        """Rule 7, only with no player within 1 km of the hunting part."""
        P, A, rng = self.P, self.A, self.rng
        if self.cured or self.storm:
            return
        roster = min(10, A["wanderers"] + max(0, len(self.towns) - 8) // 6)
        wanderers = max(0, roster - len([w for w in self.wanderers_lost if w > self.now]))
        self.wanderers_lost = [w for w in self.wanderers_lost if w > self.now]
        truck_ok = self.counted - self.last_truck >= P["truck_every"]
        for t in list(self.tribes):
            if t.vigil or t.members() < 2:
                continue
            trails = 3 if self.frenzy else (2 if t.hunger >= 60 else 1)
            if self.counted - t.last_wanderer >= P["wanderer_every"] and wanderers > 0:
                cross = trails * wanderers * 2 * A["wanderer_km"] * 1.5 * t.r / (math.pi * self.map.land)
                if rng.random() < 1 - (1 - P["wanderer_take"]) ** cross:
                    t.last_wanderer = self.counted
                    self.wanderers_lost.append(self.now + 1440)
                    wanderers -= 1
                    self.taken(t)
            if truck_ok:
                trucks = A["trucks_day"] * self.speed * (85 / 1440.0) * A["truck_near"] / max(1, len(self.tribes)) * trails
                if rng.random() < 1 - (1 - P["truck_take"]) ** trucks:
                    truck_ok = False
                    self.last_truck = self.counted
                    for _ in range(rng.randint(1, 2)):
                        self.taken(t)

    def border_fights(self):
        P, rng = self.P, self.rng
        ts = list(self.tribes)
        for i in range(len(ts)):
            for j in range(i + 1, len(ts)):
                a, b = ts[i], ts[j]
                if not (a.alive and b.alive) or not self.bordering(a, b):
                    continue
                if a.clan and a.clan == b.clan:
                    continue
                key = (min(a.id, b.id), max(a.id, b.id))
                rec = self.borders.setdefault(key, [self.sd, rng.random() < 0.15])
                cond = a.hunger >= 60 or b.hunger >= 60 or rec[1] or a.reopened or b.reopened or self.frenzy
                if not cond or a.vigil or b.vigil:
                    continue
                p = 0.2
                if self.sd - rec[0] >= P["border_old_days"]:
                    p *= 0.5
                if a.reopened or b.reopened:
                    p *= 2
                if a.restless or b.restless:
                    p *= 2
                if rng.random() >= min(0.8, p):
                    continue
                rec[0] = self.sd
                self.stats["border_fights"] += 1
                sa, sb = self.strength(a), self.strength(b)
                win, lose = (a, b) if sa * rng.uniform(0.8, 1.2) >= sb * rng.uniform(0.8, 1.2) else (b, a)
                self.lose_mixed(lose, rng.randint(2, 4))
                win.hunger = max(0, win.hunger - 10)
                if lose.members() < P["weak_under"] and (lose.alpha is None or self.stage_of(lose) == 2):
                    n = lose.grown + len(lose.young)
                    spl = int(round(0.3 * n))
                    self.add_grown(win, n - spl)
                    self.to_splinters(lose, spl)
                    if lose.alpha and len(self.lone) < P["lone_max"]:
                        self.lone.append([self.sd, lose.alpha.health])
                        self.stats["lone"] += 1
                    self.remove(lose, "absorbed")
                    self.stats["absorbed"] += 1
                else:
                    self.check_breakup(lose)

    def strength(self, t):
        g, _, h, _ = self.roles(t)
        mult = (0.8, 1.2, 0.9)[self.stage_of(t)] if t.alpha else 0.5
        if t.temper == "territorial":
            mult *= 1.3
        return max(0.1, (h + g) * mult)

    def dawn(self):
        for t in list(self.tribes):
            if t.succession:
                t.vigil = False
                self.guard_fight(t)
            t.night = None

    def day_extras(self, per, online, seg_len):
        """By day: Bastion's culls, deliberate raids on old alphas, and the dead in the open."""
        A, P, rng = self.A, self.P, self.rng
        ph = sum(m for _, m in per) / 60.0
        if ph > 0 and self.tribes:
            presence = STAGE_PRESENCE[self.stage]
            if self.stage >= 1 and presence >= 0.2:
                for _ in range(poisson(rng, A["culls"] * ph)):
                    named = [x for x in self.tribes if x.members() >= P["cull_min"]]
                    if not named:
                        break
                    t = min(named, key=lambda x: min([dist(x.pos, h) for h in self.map.bastion] or [0]))
                    n = self.lose_mixed(t, rng.randint(*A["cull_kills"]))
                    self.stats["kills_day"] += n
                    self.stats["culls"] += 1
                    self.disturb(t, P["disturb_cull"], "bastion")
                    self.check_breakup(t)
            # raids find a lair by its calls, scouts and tracks: a weakened tribe is quiet
            old = [t for t in self.tribes if t.alpha and self.stage_of(t) >= 1 and not t.post_feed and not self.weak(t)]
            if old:
                for _ in range(poisson(rng, A["alpha_hunts"] * ph)):
                    t = rng.choice(old)
                    if t.alive:
                        k = min(self.n, 3)
                        who = rng.sample(range(self.n), k)
                        self.lair_fight(t, k, who, raid=True)
        # players dying to the infected in the wild, outside the tribes' fights
        if not self.cured and self.map.wild_cells:
            for _ in range(poisson(rng, A["player_open"] * ph)):
                pos = rng.choice(self.map.wild_cells)
                near = [t for t in self.tribes if dist(t.pos, pos) <= 1.5 * t.r]
                if near:
                    self.taken(min(near, key=lambda t: dist(t.pos, pos)), player=rng.randrange(self.n))
                else:
                    self.open_dead.append([pos, self.sd])
        # people dying in the open (wanderers, crews, patrols) and walk-outs: real time, online or not
        lam = (A["open_deaths"] + 0) * self.speed * seg_len / 1440.0
        if not self.cured:
            for _ in range(poisson(rng, lam)):
                if self.map.wild_cells:
                    self.open_death(rng.choice(self.map.wild_cells))
            for _ in range(poisson(rng, A["walkouts"] * self.speed * seg_len / 1440.0)):
                safe = [self.map.towns[i] for i in self.map.lived if self.map.faction[i] == "haven"]
                if safe:
                    h = rng.choice(safe)
                    near = [t for t in self.tribes if dist(t.pos, h) <= 2.0]
                    if near:
                        self.taken(min(near, key=lambda t: dist(t.pos, h)), kind="other")

    def timeline(self):
        self.serum = self.tl[75] is not None and self.now >= self.tl[75] * 60
        if self.tl[90] is not None and self.now >= self.tl[90] * 60 and not self.gate_done:
            self.gate_done = True
            old = [t for t in self.tribes if t.alpha and self.stage_of(t) >= 1]
            if old and self.rng.random() < 0.8:
                self.kill_alpha(self.rng.choice(old), "capture")
        if self.tl[100] is not None and self.now >= self.tl[100] * 60:
            self.cured = True

    # ------------------------------------------------------------------------------------- the season
    def run(self):
        A = self.A
        game_day = 1440.0 / A["time_accel"]
        phases = (("day", 12.5), ("dusk", 2.0), ("night", 8.5), ("dawn", 1.0))
        self.now = 0.0
        self.serum = False
        self.wild_share = A["wild_day"]
        self.seed()
        while self.now < self.end:
            for phase, hours in phases:
                seg = hours * game_day / 24.0
                a, b = self.now, self.now + seg
                self.timeline()
                if phase == "day":
                    self.start_day()
                elif phase == "dusk":
                    self.dusk()
                online, per = self._online(a, b)
                if online > 0:
                    sd_before = self.sd_int
                    self.advance(online)
                    if self.sd_int > sd_before:
                        self.dark_towns()
                ph = sum(m for _, m in per) / 60.0
                self.stats["ph"] += ph
                if phase == "night":
                    self.stats["ph_night"] += ph
                else:
                    self.stats["ph_day"] += ph
                kills = self.encounters(phase, online, per, seg)
                if phase == "day":
                    self.day_extras(per, online, seg)
                    self.time_avg()
                if phase == "night":
                    self.time_avg(night=True)
                if online > 0:
                    other = max(0.0, ps.KILLS_PER_PLAYER_HOUR * ph - kills)
                    self.stats["kills_other"] += other
                    self.town_tick(online, per, other * A["town_kill_share"])
                if phase == "night":
                    self.night_end(online >= A["counted_minutes"])
                elif phase == "dawn":
                    self.dawn()
                self.now = b
                if self.now >= self.end:
                    break
            if self.trace and int(self.now // 1440) != int((self.now - game_day) // 1440):
                self.trace_line()
        if self.sd > 0:
            self.stats["counted_per_sd"] = self.counted / self.sd
        self.stats["sd"] = self.sd
        self.stats["days"] = self.now / 1440.0
        self.stats["final"] = len(self.tribes)
        self.stats["banked"] = self.banked
        return self.stats

    def time_avg(self, night=False):
        """Time-averaged meeting rates per km inside a range, over every tribe."""
        st = self.stats
        for t in self.tribes:
            if night:
                if self.bodies_out(t):
                    hunt, scouts = self.night_rate(t)
                else:
                    hunt, scouts = 0.0, (self.night_rate(t, scouts_only=True)[1] if not t.vigil and not self.storm else 0.0)
                st["rate_night"][0] += hunt + scouts
                st["rate_night"][1] += 1
            else:
                party, scouts, lair = self.day_rate(t)
                st["rate_day"][0] += party + scouts
                st["rate_day"][1] += 1
                st["r"].append(t.r)
                if t.party > 0:
                    st["rate_active"][0] += party + scouts
                    st["rate_active"][1] += 1
                    key = "rate_hungry" if t.hunger >= 60 else "rate_fed"
                    st[key][0] += party + scouts
                    st[key][1] += 1

    def trace_line(self):
        day = int(self.now // 1440)
        parts = []
        for t in self.tribes:
            g, sc, h, y = self.roles(t)
            stage = "RPD"[self.stage_of(t)] if t.alpha else "-"
            parts.append("#%d %s%d(%s g%d s%d h%d y%d) hu%d%s%s" % (
                t.id, {"seed": "S", "dark": "D", "taken": "T", "lone": "L", "split": "s"}[t.how], t.members(),
                stage, g, sc, h, y, t.hunger,
                " restless" if t.restless else "", " clan" if t.clan else ""))
        avg = sum(self.towns) / max(1, len(self.towns))
        print("day %2d sd %5.1f %s inf %2.0f lone %d spl %d | %s" % (
            day, self.sd, ("EO", "Co", "Ov", "Af")[self.stage], avg, len(self.lone), len(self.splinters),
            "; ".join(parts)))


# ------------------------------------------------------------------------------------------ running
def timeline(players, legacy, rng):
    s = dict(ps.LEGACY if legacy else ps.DEFAULTS)
    hit, _, _ = ps.run_season(players, s, "none" if legacy else "finale", rng)
    return {m: hit.get(m) for m in (25, 75, 90, 100)}


def run_config(mkey, maps, players, P, A, runs, seed, legacy, slots):
    out = []
    for r in range(runs):
        rng = random.Random(seed * 1000003 + r * 7919 + players * 31 + len(mkey))
        tl = timeline(players, legacy, random.Random(seed + r * 13 + players))
        sim = Season(maps[r % len(maps)], players, P, A, rng, legacy, tl)
        sim.slot_override = slots
        out.append(sim.run())
    return out


def pct(x):
    return "%d%%" % round(100 * x)


def median_or(values, fmt="%.0f", none="-"):
    values = [v for v in values if v is not None]
    if not values:
        return none
    return fmt % statistics.median(values)


def summarise(label, res):
    days = {}
    for s in res:
        for d, n in s["tribes_by_day"].items():
            days.setdefault(d, []).append(n)

    def at(day):
        vals = [n for d, ns in days.items() if d == day for n in ns]
        return median_or(vals)

    mids = []
    for s in res:
        # each season's median count from a quarter of the way through to the cure
        vals = [n for d, n in s["tribes_by_day"].items() if d >= max(2, s["days"] / 4)]
        if vals:
            mids.append(statistics.median(vals))
    km_d = sum(s["km_day"] for s in res)
    km_n = sum(s["km_night"] for s in res)
    rate_d = sum(s["rate_day"][0] for s in res) / max(1, sum(s["rate_day"][1] for s in res))
    rate_a = sum(s["rate_active"][0] for s in res) / max(1, sum(s["rate_active"][1] for s in res))
    rate_f = sum(s["rate_fed"][0] for s in res) / max(1, sum(s["rate_fed"][1] for s in res))
    rate_h = sum(s["rate_hungry"][0] for s in res) / max(1, sum(s["rate_hungry"][1] for s in res))
    fh = [x for s in res for x in s["first_hunt"]]
    rate_n = sum(s["rate_night"][0] for s in res) / max(1, sum(s["rate_night"][1] for s in res))
    radii = [r for s in res for r in s["r"]]
    rbar = statistics.mean(radii) if radii else 0.75
    ph = sum(s["ph"] for s in res)
    kd = sum(s["kills_day"] for s in res)
    kn = sum(s["kills_night"] for s in res)
    ko = sum(s["kills_other"] for s in res)
    firsts = [x for s in res for x in s["first"]]
    sds = [s["sd"] for s in res]
    members = [m for s in res for m in s["members"]]
    hungry = sum(s["hungry"][0] for s in res) / max(1, sum(s["hungry"][1] for s in res))
    party = sum(s["party"][0] for s in res) / max(1, sum(s["party"][1] for s in res))
    row = {
        "label": label,
        "days": statistics.median(s["days"] for s in res),
        "sd": statistics.median(sds),
        "cn": median_or([s["counted_per_sd"] for s in res], "%.1f"),
        "d10": at(10), "d20": at(20), "d30": at(30),
        "final": median_or([s["final"] for s in res]),
        "mid": median_or(mids, "%.1f"),
        "in46": pct(sum(1 for m in mids if 4 <= m <= 6) / max(1, len(mids))),
        "range": "%s-%s" % (min(mids) if mids else "-", max(mids) if mids else "-"),
        "clan": pct(sum(1 for s in res if s["clan"]) / len(res)),
        "great": pct(sum(1 for s in res if s["great"]) / len(res)),
        "first": "%s / %s" % (pct(max(firsts)) if firsts else "-", pct(statistics.mean(firsts)) if firsts else "-"),
        "wiped": str(sum(s["first_wiped"] for s in res)),
        "half": str(sum(s["first_half"] for s in res)),
        "members": median_or(members),
        "hungry": pct(hungry),
        "party": pct(party),
        "day_km": "%.2f" % rate_d,
        "night_km": "%.2f" % rate_n,
        "day_cross": "%.1f" % (rate_d * 2 * rbar),
        "active_cross": "%.1f" % (rate_a * 2 * rbar),
        "fed_cross": "%.1f / %.1f" % (rate_f * 2 * rbar, rate_h * 2 * rbar),
        "first_hunt": "%s / %s" % (pct(max(fh)) if fh else "-", pct(statistics.mean(fh)) if fh else "-"),
        "first_worst": max(firsts + fh) if firsts + fh else 0.0,
        "wiped_n": sum(s["first_wiped"] + s["first_hunt_wiped"] for s in res),
        "mids": mids,
        "clan_x": sum(1 for s in res if s["clan"]) / len(res),
        "fed_x": rate_f * 2 * rbar, "hungry_x": rate_h * 2 * rbar, "night_x": rate_n * 2 * rbar,
        "day_x": rate_d * 2 * rbar,
        "fh_wiped": str(sum(s["first_hunt_wiped"] for s in res)),
        "night_cross": "%.1f" % (rate_n * 2 * rbar),
        "meas_day": "%.2f" % (sum(s["enc_day"] for s in res) / km_d) if km_d > 0 else "-",
        "meas_night": "%.2f" % (sum(s["enc_night"] for s in res) / km_n) if km_n > 0 else "-",
        "body_day": "%.1f" % (sum(s["bodies_day"] for s in res) / max(1, sum(s["enc_day"] for s in res))),
        "body_night": "%.1f" % (sum(s["bodies_night"] for s in res) / max(1, sum(s["enc_night"] for s in res))),
        "kills_ph": "%.1f" % ((kd + kn) / max(1e-9, ph)),
        "wild_share": "%s / %s" % (pct(kd / max(1e-9, kd + kn + ko)), pct(kn / max(1e-9, kd + kn + ko))),
        "deaths": "%.2f / %.2f" % (sum(s["deaths_day"] for s in res) / max(1e-9, sum(s["ph_day"] for s in res)),
                                   sum(s["deaths_night"] for s in res) / max(1e-9, sum(s["ph_night"] for s in res))),
        "ran": "%.2f" % (sum(s["ran"] for s in res) / max(1, sum(s["tribe_sd"] for s in res))),
        "banked": "%.1f" % (sum(s["banked"] for s in res) / max(1, sum(s["tribe_sd"] for s in res))),
        "inf": "%.0f" % statistics.mean(x for s in res for x in s["town_inf"]),
        "taken": "%.1f / %.1f / %.1f" % (statistics.mean(s["taken_player"] for s in res),
                                        statistics.mean(s["taken_data"] for s in res),
                                        statistics.mean(s["taken_other"] for s in res)),
        "succ": "%.1f" % statistics.mean(s["successions"] for s in res),
        "chal": "%.1f" % statistics.mean(s["challenges"] for s in res),
        "age": "%.1f" % statistics.mean(s["age_deaths"] for s in res),
        "akill": "%.1f" % statistics.mean(s["alpha_killed"] for s in res),
        "lone": "%.1f" % statistics.mean(s["lone"] for s in res),
        "splits": "%.1f" % statistics.mean(s["splits"] for s in res),
        "breakups": "%.1f" % statistics.mean(s["breakups"] + s["absorbed"] for s in res),
        "starved": "%.1f" % statistics.mean(s["starved"] for s in res),
        "culls": "%.1f" % statistics.mean(s["culls"] for s in res),
        "displaced": "%.1f" % statistics.mean(s["displaced"] for s in res),
        "new": "/".join("%.1f" % statistics.mean(s["new"][k] for s in res) for k in ("seed", "dark", "taken", "lone", "split")),
        "rbar": rbar,
    }
    return row


def table(head, rows):
    widths = [max(len(h), max(len(str(r[i])) for r in rows)) for i, h in enumerate(head)]
    print(" | ".join(h.ljust(widths[i]) for i, h in enumerate(head)))
    print("-|-".join("-" * w for w in widths))
    for r in rows:
        print(" | ".join(str(c).ljust(widths[i]) for i, c in enumerate(r)))


def parse_value(old, text):
    if isinstance(old, tuple):
        parts = [float(x) for x in text.split(",")]
        return tuple(int(p) if isinstance(old[0], int) else p for p in parts)
    if isinstance(old, int):
        return int(float(text))
    return float(text)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--maps", nargs="+", default=["arland", "everon", "cain", "chernarus"], choices=sorted(MAPS))
    ap.add_argument("--players", type=int, nargs="+", default=[2, 5, 10])
    ap.add_argument("--runs", type=int, default=40)
    ap.add_argument("--seed", type=int, default=11)
    ap.add_argument("--legacy", action="store_true", help="the old 10-day pacing (pacing_sim --legacy)")
    ap.add_argument("--design", action="store_true", help="the design's numbers, as written")
    ap.add_argument("--proposed", action="store_true", help="phase 0's proposed retunes on top of the code's numbers")
    ap.add_argument("--wild", nargs="+", default=[], metavar="MAP=KM2", help="wild land per map, e.g. everon=31")
    ap.add_argument("--slots", type=int, default=None, help="the Slots override (0 = from the land)")
    ap.add_argument("--set", nargs="+", default=[], metavar="NAME=VALUE", help="change a design number or an assumption")
    ap.add_argument("--list", action="store_true", help="list every number --set can change")
    ap.add_argument("--trace", action="store_true", help="print one season of the first map and player count")
    args = ap.parse_args()

    P = dict(DESIGN)
    base = dict(DESIGN)
    if not args.design:
        P.update(BUILT)
        base.update(BUILT)
    if args.proposed:
        P.update(TUNED)
        base.update(TUNED)
    A = dict(ASSUME)
    if args.list:
        for name, value in sorted(P.items()):
            print("%-20s %-14s design %s" % (name, value, DESIGN[name]))
        for name, value in sorted(A.items()):
            print("%-20s %-14s (assumption)" % (name, value))
        return
    for item in args.set:
        name, _, text = item.partition("=")
        if name in P:
            P[name] = parse_value(P[name], text)
        elif name in A:
            A[name] = parse_value(A[name], text)
        else:
            ap.error("unknown number: " + name)
    wild = {k: v["wild"] for k, v in MAPS.items()}
    for item in args.wild:
        name, _, text = item.partition("=")
        if name not in MAPS:
            ap.error("unknown map: " + name)
        wild[name] = float(text)
    slots = args.slots if args.slots else None

    pace = "the old 10-day pacing" if args.legacy else "the 45-day pacing"
    print("Tribe ecology on %s, %d runs a row (seed %d)." % (pace, args.runs, args.seed))
    if args.design:
        print("Numbers: the design's, as written.")
    else:
        print("Numbers: the code's (--design for the design's own):")
        for name, (value, why) in BUILT_WHY.items():
            print("  %-15s %-7s (design %s): %s" % (name, value, DESIGN[name], why))
    if args.proposed:
        print("With phase 0's proposed retunes (waiting for the author):")
        for name, (value, why) in TUNED_WHY.items():
            print("  %-15s %-7s (design %s): %s" % (name, value, DESIGN[name], why))
    changed = [n for n in P if P[n] != base[n]]
    changed += [n for n in A if A[n] != ASSUME[n]]
    if changed:
        print("Changed with --set: " + ", ".join("%s=%s" % (n, P.get(n, A.get(n))) for n in changed))
    print("Day parties (the author's change): %d%% of the tribe (x%.0f hungry) in groups of %d-%d, %d bodies at a"
          " cluster; a scout's call by day brings %d; fog counts as dusk."
          % (P["day_share"] * 100, P["day_hungry"], P["day_group"][0], P["day_group"][1], P["day_cluster_cap"],
             P["day_scout_pack"]))
    print("Assumptions: sight %d m by day, a hunt's trail %d m, players and tribes on the same tracks x%.1f,"
          " %.0f km/h on foot; %d%% / %d%% of a player's minutes in the wild by day / night; fog on %d%% of days.\n"
          % (A["P"] * 1000, A["trail_r"] * 1000, A["overlap"], A["walk_kmh"], A["wild_day"] * 100,
             A["wild_night"] * 100, A["fog"] * 100))

    maps = {}
    for key in args.maps:
        rng = random.Random(args.seed * 31 + len(key))
        maps[key] = [Map(key, MAPS[key], wild[key], rng) for _ in range(MAP_VARIANTS)]
        m0 = maps[key]
        n_sl = Season.__new__(Season)
        n_sl.map, n_sl.P, n_sl.slot_override, n_sl.tribes = m0[0], P, slots, []
        sl, kept = Season.slots(n_sl)
        print("%s: %.0f km2 of wild land in %.0f km2 of land, %d towns; %d slots (%d kept for splits); lair"
              " candidates %d-%d; range radius %s m." % (
                  MAPS[key]["label"], wild[key], statistics.mean(m.land for m in m0), MAPS[key]["towns"], sl, kept,
                  min(len(m.cands) for m in m0), max(len(m.cands) for m in m0),
                  "350-700" if m0[0].small else "500-1,000"))
    print()

    if args.trace:
        key = args.maps[0] if len(args.maps) == 1 else ("everon" if "everon" in args.maps else args.maps[0])
        n = args.players[0] if len(args.players) == 1 else (5 if 5 in args.players else args.players[0])
        rng = random.Random(args.seed)
        tl = timeline(n, args.legacy, random.Random(args.seed + 1))
        sim = Season(maps[key][0], n, P, A, rng, args.legacy, tl, trace=True)
        sim.slot_override = slots
        st = sim.run()
        print("clan: %s (season day %s), splits %d, successions %d, culls %d" % (
            st["clan"], st["clan_day"] and "%.0f" % st["clan_day"], st["splits"], st["successions"], st["culls"]))
        return

    rows = []
    for key in args.maps:
        for n in args.players:
            res = run_config(key, maps[key], n, P, A, args.runs, args.seed, args.legacy, slots)
            rows.append(summarise("%s, %d" % (MAPS[key]["label"], n), res))

    print("Tribes (medians over the runs; 'Over the season': each season's median count from a quarter of the way"
          " through to the cure):")
    table(("Map, players", "Cure day", "Season days", "Counted nights/sd", "Day 10", "Day 20", "Day 30", "At cure",
           "Over the season", "In 4-6", "Clan", "Great alpha", "Members", "Hungry"),
          [(r["label"], "%.0f" % r["days"], "%.0f" % r["sd"], r["cn"], r["d10"], r["d20"], r["d30"], r["final"],
            r["mid"], r["in46"], r["clan"], r["great"], r["members"], r["hungry"]) for r in rows])
    print("\nMeeting them, inside a range (a meeting is a fight; a crossing is a solo walk through the middle, the"
          " lair's ring not counted):")
    table(("Map, players", "By day /km", "Day crossing", "With a party out: fed / hungry", "Bodies a day fight",
           "By night /km", "Night crossing", "Bodies a hunt", "Met /km day / night", "Deaths/ph day / night"),
          [(r["label"], r["day_km"], r["day_cross"], r["fed_cross"], r["body_day"], r["night_km"], r["night_cross"],
            r["body_night"], "%s / %s" % (r["meas_day"], r["meas_night"]), r["deaths"]) for r in rows])
    print("\nLife and death (means a season):")
    table(("Map, players", "First night: worst / mean lost", "First hunt: worst / mean", "Wiped", "Tribe kills/ph",
           "Kills in the wild day / night", "Town infection", "Banked/tribe/sd", "Ran/tribe/sd",
           "Taken: players / data / other", "New: seed/dark/taken/lone/split", "Successions", "Alphas killed",
           "Challenges", "Age deaths", "Lone ones", "Splits", "Broken up", "Starved", "Culls", "Displaced"),
          [(r["label"], r["first"], r["first_hunt"], str(r["wiped_n"]), r["kills_ph"], r["wild_share"], r["inf"],
            r["banked"], r["ran"], r["taken"], r["new"], r["succ"], r["akill"], r["chal"], r["age"], r["lone"],
            r["splits"], r["breakups"], r["starved"], r["culls"], r["displaced"]) for r in rows])
    print("\nBy day /km and By night /km: fights expected per km walked inside a range, averaged over every tribe and"
          " every day or night of the season (the day after a feed, weakened tribes and vigils included). Crossing:"
          " that x the range's diameter; 'with a party out' leaves out the days nobody is out, split by a fed and a"
          " hungry tribe. Met /km: what the players in the runs met per km they walked there. A day fight is a day"
          " party group, or a scout and the pack its call brings (at most 4); a hunt is up to two waves, with the"
          " core held back. First night: the share of its members a tribe lost on its first counted night with"
          " bodies out; first hunt: on its first night with a hunt at players. Wiped: tribes either left under 4."
          " Tribe kills/ph: tribe bodies killed per player-hour (of the %d players kill in all). Banked: infection"
          " points a tribe banked a season day; Ran: young from them." % ps.KILLS_PER_PLAYER_HOUR)

    print("\nTargets (from the design and the author's change):")
    for r in rows:
        key = r["label"].split(",")[0]
        checks = []
        if key == "Everon":
            checks.append("tribes %s (4-6 in %s of seasons)" % (r["mid"], r["in46"]))
            checks.append("a clan in %s of seasons (about half)" % r["clan"])
        checks.append("first night worst %s, %d wiped (none)" % (pct(r["first_worst"]), r["wiped_n"]))
        checks.append("solo day crossing %.1f fights (%.1f fed / %.1f hungry, of %s bodies) against %.1f hunts by night"
                      % (r["day_x"], r["fed_x"], r["hungry_x"], r["body_day"], r["night_x"]))
        print("  %-20s %s" % (r["label"] + ":", "; ".join(checks)))


if __name__ == "__main__":
    main()
