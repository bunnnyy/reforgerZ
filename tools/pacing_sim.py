#!/usr/bin/env python3
"""ReforgerZ season pacing simulator (docs/DESIGN.md, "Season pacing").

    python3 tools/pacing_sim.py                 # 2, 3, 5, 7, 10 and 20 players with the default settings
    python3 tools/pacing_sim.py --players 1 30  # other server sizes
    python3 tools/pacing_sim.py --cure-pace 0.5 --lab-per-day 4
    python3 tools/pacing_sim.py --play-pace-hours 0   # pace by play off: Cure pace alone
    python3 tools/pacing_sim.py --legacy        # the old tuning (no lab capacity, 8/16/24 h stages)

Simulates a season hour by hour from the assumptions in DESIGN.md (kills per player-hour,
special share by stage, samples that reach the lab, research missions per player-hour) and the
rules in Cure/RZ_CureResearch.c: research counts x the pace by play (the target player-hours a day
over those played on the last 7 whole real days, x0.25 to x2.5; x1 for the first two days),
deliveries go into the lab's backlog, the lab works through it at a fixed rate per real day (also
with nobody online), the backlog is capped (samples refused, mission research beyond the cap lost),
the part of a delivery that fills the backlog up to half a day of work counts more, and the
milestones fire on processed research. With a finale registered, research stalls at 90%: the
finale takes --finale-days to reach 100%, and after a week at the gate the lab also inches up by
itself (--fallback-per-day); "never done" is a season whose finale nobody finishes. Prints the
median day each milestone and outbreak stage is reached over many runs. Nothing here is measured:
change the assumptions when play-tests say otherwise. No dependencies beyond Python 3.
"""
import argparse
import random
import statistics

# ------------------------------------------------------------------------------------- assumptions
PLAY_WINDOW_HOURS = 12          # players come on at a random time in a 12-hour stretch of the day
SESSION_HOURS = (2.0, 3.0)      # each regular plays 2 to 3 hours a day
KILLS_PER_PLAYER_HOUR = 20      # travel, looting and missions mixed
SPECIAL_SHARE = (0.011, 0.016, 0.022, 0.027)   # by stage, with night and adaptation
SAMPLES_REACH_LAB = 0.6         # deaths, the Red Hand's dead drop, pockets
SAMPLE_BONUS = 1.2              # Lab Technicians (x2) and Halcyon Champions (+25%), about +20% overall
MISSIONS_PER_PLAYER_HOUR = 0.143    # research missions finished (2.5 a day with 17.5 player-hours)
MISSION_POINTS = (1, 1.5, 2, 2, 2.5, 3, 3, 5)   # what a research mission names, 2.5 on average

# -------------------------------------------------------------------- settings (the code's defaults)
DEFAULTS = {
    "research_per_sample": 1.0,     # Cure > Research per sample
    "mission_research": 0.75,       # Cure > Mission research
    "cure_pace": 0.30,              # Cure > Cure pace
    "season_speed": 1.0,            # Season speed (top level)
    "lab_per_day": 3.0,             # Cure > Lab research per day
    "backlog_days": 4.0,            # Cure > Lab backlog days
    "empty_bonus": 0.25,            # Cure > Empty lab bonus
    "play_pace_hours": 12.5,        # Cure > Pace by play: target player-hours a day (0 = off)
    "stage_hours": (40, 80, 120),   # Director > Stages: hours played in Early Outbreak, Collapse, Overrun
    "finale_days": 5.0,             # a finale questline from 90% to 100% (planned; not built)
    "fallback_per_day": 1.0,        # Cure > Finale fallback: past 90% with a finale, per real day after a week
}
LEGACY = dict(DEFAULTS, cure_pace=1.0, lab_per_day=0.0, backlog_days=0.0, empty_bonus=0.0,
              play_pace_hours=0.0, stage_hours=(8, 16, 24))

# The code's constants (Cure/RZ_CureResearch.c)
PLAY_DAYS = 7                   # pace by play: the whole real days it averages over
PLAY_PACE = (0.25, 2.5)         # and its limits
IDLE_DAYS = 0.5                 # the empty lab bonus fills the backlog up to this many days of work
GATE_GRACE_DAYS = 7             # the finale fallback starts after this many real days at the gate

MILESTONES = (25, 50, 75, 90, 100)
GATE = 90
STAGES = ("Early Outbreak", "Collapse", "Overrun", "Aftermath")


def online_fraction(sessions, hour):
    """Share of [hour, hour+1) with at least one player on (sessions are merged intervals)."""
    covered = 0.0
    for a, b in sessions:
        lo, hi = max(a, hour), min(b, hour + 1)
        if hi > lo:
            covered += hi - lo
    return min(1.0, covered)


def merge(intervals):
    out = []
    for a, b in sorted(intervals):
        if out and a <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], b))
        else:
            out.append((a, b))
    return out


def player_hours(sessions, hour):
    """Player-hours played in [hour, hour+1) (one session per player, not merged)."""
    total = 0.0
    for a, b in sessions:
        lo, hi = max(a, hour), min(b, hour + 1)
        if hi > lo:
            total += hi - lo
    return total


def run_season(players, s, mode, rng, max_days=250):
    """One season. mode: "none" (no finale registered, no gate), "finale" (the gate, and a finale
    that takes finale_days) or "never" (the gate, and nobody finishes a finale: the fallback only).
    Returns milestone hours, stage hours and a few totals."""
    finale = mode != "none"
    capacity_per_hour = s["lab_per_day"] * s["season_speed"] / 24.0
    capped = capacity_per_hour > 0
    cap = s["lab_per_day"] * s["backlog_days"] * s["season_speed"]
    idle = s["lab_per_day"] * s["season_speed"] * IDLE_DAYS
    per_sample = s["research_per_sample"] * s["cure_pace"] * s["season_speed"]
    mission_mult = s["mission_research"] * s["cure_pace"] * s["season_speed"]
    played = []                     # player-hours on each real day so far
    paces = []
    gate_hours = 0.0

    progress = 0.0
    backlog = 0.0
    stage = 0
    stage_online = 0.0
    hit = {}
    stage_at = {0: 0.0}
    kept = [0] * players            # samples a player still carries (refused at a full lab)
    trips = 0                       # hand-ins with samples, and those the lab turned (partly) away
    turned = 0
    mission_total = 0.0             # mission research named (after the multipliers), and what was lost
    lost = 0.0
    full_hours = 0
    delivered = 0.0
    finale_start = None

    def play_pace(day):
        """The code's PlayPace: x1 until a whole day after the season's first has been counted,
        then the target over the average of the last 7 whole days."""
        if s["play_pace_hours"] <= 0:
            return 1.0
        whole = played[max(1, day - PLAY_DAYS):day]
        if not whole:
            return 1.0
        avg = sum(whole) / len(whole)
        if avg <= 0:
            return PLAY_PACE[1]
        return min(PLAY_PACE[1], max(PLAY_PACE[0], s["play_pace_hours"] / avg))

    def with_bonus(points):
        """The empty lab bonus, on the part that fills the backlog up to half a day of work."""
        if not capped:
            return points
        return points + s["empty_bonus"] * min(points, max(0.0, idle - backlog))

    def deliver(points):
        nonlocal backlog, delivered
        backlog += points
        delivered += points

    for day in range(max_days):
        # Today's sessions: start, end (hours since midnight) and the events in them
        sessions = []
        events = []     # (hour, kind, player, value)
        for p in range(players):
            length = rng.uniform(*SESSION_HOURS)
            start = rng.uniform(0, PLAY_WINDOW_HOURS - length) + (24 - PLAY_WINDOW_HOURS)
            sessions.append((start, start + length))
            kills = int(round(KILLS_PER_PLAYER_HOUR * length))
            events.append((start + length, "samples", p, kills))
            missions = rng.random() < MISSIONS_PER_PLAYER_HOUR * length - int(MISSIONS_PER_PLAYER_HOUR * length)
            for _ in range(int(MISSIONS_PER_PLAYER_HOUR * length) + int(missions)):
                events.append((rng.uniform(start, start + length), "mission", p, rng.choice(MISSION_POINTS)))
        merged = merge(sessions)
        events.sort()
        played.append(0.0)
        pace = play_pace(day)

        for h in range(24):
            t = day * 24 + h
            # 1. The outbreak's clock: only with someone online
            if stage < 3:
                stage_online += online_fraction(merged, h) * s["season_speed"]
                if stage_online >= s["stage_hours"][stage]:
                    stage_online = 0.0
                    stage += 1
                    stage_at[stage] = t + 1

            played[day] += player_hours(sessions, h)

            # 2. Deliveries this hour
            for ev in [e for e in events if h <= e[0] < h + 1]:
                _, kind, p, value = ev
                if kind == "samples":
                    share = SPECIAL_SHARE[stage]
                    samples = sum(1 for _ in range(value) if rng.random() < share * SAMPLES_REACH_LAB)
                    samples += kept[p]
                    kept[p] = 0
                    if samples == 0 or progress >= 100:
                        continue
                    worth = per_sample * SAMPLE_BONUS * pace
                    take = samples
                    if capped:
                        room = cap - backlog
                        take = 0 if room <= 0 else min(samples, -int(-room // worth))
                    kept[p] = samples - take
                    trips += 1
                    paces.append(pace)
                    if take < samples:
                        turned += 1
                    if take:
                        deliver(with_bonus(take * worth))
                else:
                    points = with_bonus(value * mission_mult * pace)
                    if progress >= 100:
                        continue
                    mission_total += points
                    if capped:
                        room = max(0.0, cap - backlog)
                        if points > room:
                            lost += points - room
                            points = room
                    if points > 0:
                        deliver(points)

            # 3. The lab works through the backlog (real time, online or not)
            limit = 100.0
            if finale:
                limit = GATE     # the gate stays shut to the backlog until the finale reaches 100%
            if capped:
                work = min(backlog, capacity_per_hour, max(0.0, limit - progress))
            else:
                work = min(backlog, max(0.0, limit - progress))
            backlog -= work
            before = progress
            progress += work
            if finale and progress >= GATE and finale_start is None:
                finale_start = t + 1
            if finale_start is not None and progress < 100:
                # After a week at the gate the lab inches on by itself (Finale fallback)
                gate_hours += 1
                if gate_hours > GATE_GRACE_DAYS * 24 / s["season_speed"]:
                    progress = min(100.0, progress + s["fallback_per_day"] * s["season_speed"] / 24.0)
                # The finale questline (planned) takes the last 10% in finale_days
                if mode == "finale":
                    progress = min(100.0, max(progress, GATE + 10.0 * (t + 1 - finale_start) / (24 * s["finale_days"])))
            if capped and backlog >= cap:
                full_hours += 1
            for m in MILESTONES:
                if m not in hit and before < m <= progress + 1e-9:
                    hit[m] = t + 1
            if progress >= 100:
                break
        if progress >= 100:
            break
    totals = {"turned": turned / max(1, trips), "lost": lost / max(0.001, mission_total),
              "full": full_hours / max(1, t + 1), "per_day": delivered / max(1.0, (t + 1) / 24.0),
              "pace": statistics.mean(paces) if paces else 1.0}
    return hit, stage_at, totals


def day_of(hour):
    return None if hour is None else int(hour // 24) + 1


def median_day(values):
    days = [day_of(v) for v in values if v is not None]
    if len(days) < len(values) // 2 + 1:
        return "-"
    return str(int(statistics.median(days)))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--players", type=int, nargs="+", default=[2, 3, 5, 7, 10, 20])
    ap.add_argument("--runs", type=int, default=60)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--legacy", action="store_true", help="the old tuning: no lab capacity, 8/16/24 h stages")
    for key, value in DEFAULTS.items():
        if isinstance(value, tuple):
            ap.add_argument("--" + key.replace("_", "-"), type=float, nargs=3)
        else:
            ap.add_argument("--" + key.replace("_", "-"), type=float)
    args = ap.parse_args()

    s = dict(LEGACY if args.legacy else DEFAULTS)
    for key in DEFAULTS:
        v = getattr(args, key)
        if v is not None:
            s[key] = tuple(v) if isinstance(DEFAULTS[key], tuple) else v

    print("Settings: cure pace %.2f, research per sample %.2f, mission research x%.2f, season speed %.2f"
          % (s["cure_pace"], s["research_per_sample"], s["mission_research"], s["season_speed"]))
    if s["lab_per_day"] > 0:
        print("Lab: %.1f%% a real day, backlog capped at %.1f days (%.1f%%), +%d%% on what fills it to half a day"
              % (s["lab_per_day"], s["backlog_days"], s["lab_per_day"] * s["backlog_days"], s["empty_bonus"] * 100))
    else:
        print("Lab: no capacity limit (legacy)")
    if s["play_pace_hours"] > 0:
        print("Pace by play: x(%.1f player-hours a day / those played on the last %d whole days), x%.2f to x%.1f"
              % (s["play_pace_hours"], PLAY_DAYS, PLAY_PACE[0], PLAY_PACE[1]))
    else:
        print("Pace by play: off")
    print("Stages: %d / %d / %d hours played; finale %.0f days from 90%% (planned); fallback %.1f%% a real day"
          " after %d days at the gate\n"
          % (s["stage_hours"][0], s["stage_hours"][1], s["stage_hours"][2], s["finale_days"],
             s["fallback_per_day"], GATE_GRACE_DAYS))

    head = ("Players", "Online h/day", "Pace", "25%", "50%", "75%", "90%", "100% finale", "never done",
            "no finale", "Collapse", "Overrun", "Aftermath", "Delivered/day", "Lab full", "Turned away", "Lost")
    rows = []
    for n in args.players:
        rng = random.Random(args.seed + n)
        with_finale = [run_season(n, s, "finale", rng) for _ in range(args.runs)]
        never = [run_season(n, s, "never", rng) for _ in range(args.runs)]
        without = [run_season(n, s, "none", rng) for _ in range(args.runs)]
        online = []
        for _ in range(200):
            sess = []
            for _p in range(n):
                length = rng.uniform(*SESSION_HOURS)
                start = rng.uniform(0, PLAY_WINDOW_HOURS - length)
                sess.append((start, start + length))
            online.append(sum(b - a for a, b in merge(sess)))
        cols = [str(n), "%.1f" % statistics.mean(online), "x%.2f" % statistics.mean(r[2]["pace"] for r in without)]
        for m in MILESTONES:
            cols.append(median_day([r[0].get(m) for r in with_finale]))
        cols.append(median_day([r[0].get(100) for r in never]))
        cols.append(median_day([r[0].get(100) for r in without]))
        for st in (1, 2, 3):
            cols.append(median_day([r[1].get(st) for r in without]))
        cols.append("%.1f" % statistics.mean(r[2]["per_day"] for r in without))
        for key in ("full", "turned", "lost"):
            cols.append("%d%%" % round(100 * statistics.mean(r[2][key] for r in without)))
        rows.append(cols)

    widths = [max(len(h), max(len(r[i]) for r in rows)) for i, h in enumerate(head)]
    print(" | ".join(h.ljust(widths[i]) for i, h in enumerate(head)))
    print("-|-".join("-" * w for w in widths))
    for r in rows:
        print(" | ".join(c.ljust(widths[i]) for i, c in enumerate(r)))
    print("\nDays: the median real day (day 1 = the season's first) over %d runs. 100%%: with a finale"
          " questline done in %.0f days (\"100%% finale\"), with one registered that nobody finishes (\"never"
          " done\": the fallback only), and with none registered (\"no finale\": no gate). The stage days count"
          " the clock only (a server that doesn't push back reaches the infection thresholds sooner). Without a"
          " finale: the average pace by play at a hand-in, research delivered a day (in percent, with the pace"
          " and the empty-lab bonus), the share of hours the backlog was at its cap, the share of sample"
          " hand-ins the lab turned (partly) away (players keep them), and the share of mission research lost"
          " beyond the cap." % (args.runs, s["finale_days"]))


if __name__ == "__main__":
    main()
