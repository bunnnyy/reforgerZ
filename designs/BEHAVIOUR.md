# The infected behaviour layer: what's left

**Status: designed, not built.** The layer itself is built ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)
owns it): Bacon's hooks, the four modes, the kinds' profiles, sight, the shot as a lead, memory,
hand-offs, formations and flanking, every hunt strategy, navigation and the climb gate, LOD, the
hunter's lunge, the young's leap and the alpha's charge as the lab allows them, and the crawlers' daily
life ([crawler life](../docs/INFECTED.md#crawler-life)). The zombie lab's B1–B20 results are in
[docs/WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#zombie-lab-bacons-real-ai-measured). This file
keeps only what's still to build. Approved by the author on 2026-11-11.

**The rule that still holds.** Our code drives the infected except at attack range. Inside attack
range Bacon's own attack is used, and each kind may add one special move on top. Effects are status
(a hold, a stumble, a knockdown, a little bleeding), never extra damage.

---

## Contents

1. [Levers still in play](#1-levers-still-in-play)
2. [Small rules left in the built parts](#2-small-rules-left-in-the-built-parts)
3. [Hearing beyond shots](#3-hearing-beyond-shots)
4. [Blood](#4-blood)
5. [Shrieks](#5-shrieks)
6. [Doors](#6-doors)
7. [Reactions](#7-reactions)
8. [The ankle grab and the client effects](#8-the-ankle-grab-and-the-client-effects)
9. [Settings](#9-settings)
10. [Build steps](#10-build-steps)
11. [Risks](#11-risks)
12. [Decisions for the author](#12-decisions-for-the-author)

---

## 1. Levers still in play

The levers the unbuilt parts need, with the lab's verdicts (B tests, 2026-11-11).

| # | Lever | How | Status |
|---|---|---|---|
| L4 | **Speed cap per body** | `SCR_ChimeraCharacter.SetSpeedLimit(source, fraction, instant)` | **Works with limits** (B7): a fraction of the sprint speed; the animation stays in its run state, so probably slow motion (needs a client to see) |
| L6 | **Stance** | `ForceStance` for lying up | A prone `SetStanceChange` didn't hold (B12); `ForceStance` untested |
| L10 | **Release injection** | Our own `BaconZombies_TargetSharing` event at a body, naming the target | Not needed at a release (B6). Useful to name a target a body has never perceived (B10, B11) |
| L14 | **Sleep** | `AIAgent.DeactivateAI()` / `ActivateAI()` | **Works with limits** (B12): built for crawlers; the resting ring isn't asleep yet |
| L16 | **Our danger events** | A scripted `AIDangerEventWeaponFire` pulls engaged bodies the way a shot does | Feasible |
| L18 | **Navmesh snap** | `AIPathfindingComponent.GetClosestPositionOnNavmesh` | **Doesn't work as written** (B9: not on Bacon's bodies). A vanilla group's or the AI world's navmesh is untested |
| L21 | **Player effects, client side** | `RZ_PlayerEffectsComponent` on the player controller, owner-only RPCs: `SetSpeedLimit` on the player's own character, `ForceStance`, `SetDisableMovementControls` for a moment. A player's movement is predicted on their own machine, so these must run there | Spike (needs a real client) |
| L23 | **Pathing bias** | Mod `SCR_CreateAIExtraMoveParams` for Bacon bodies: straight-path cost per kind (×1 young, ×3 hunters), so a chase curves | Spike: the hook loads and fires (B2); its effect isn't tested |

---

## 2. Small rules left in the built parts

| Part | Still designed |
|---|---|
| Control | The release's reaction delay of 0.2–1.2 s per body |
| Kinds | The crawler's profile in use (crawlers aren't steered into fights yet); blood scent radii; the young's erratic slot jumps and stares at houses; the hunter's freeze when you look its way outside a stalk; the alpha's roar before a charge |
| Sight | Calibrate **Notice rate** to phase 0's ladder in a play-test and re-run `tools/tribe_sim.py` with it; a 2 s line-of-sight cache per check |
| Formations | A re-plan only when the target moves 25 m (the code re-plans each second); an ETA so the slower element starts first |
| Navigation | **Forest edge points** in the land cache (the code uses the square's tree class and its open neighbours); low ground preferred on a leg; the navmesh snap (L18) |
| Strategies | A **regroup** 60–100 m back in cover once per target; at most 3 elements and 2 strategies near one player cluster |
| Crawlers | A forced prone pose (L6); the crawler steered into a fight within 12 m; the ankle grab (section 8) |
| Performance | The resting ring asleep with its AI off (L14) |

---

## 3. Hearing beyond shots

A sound gives a group a **lead**: a position with an error of 15% of the distance. A lead starts an
investigation (walk-stop toward it), never a fix on you. The shots are built; the rest of the table:

| Sound | Base radius | Source |
|---|---|---|
| Sprinting / running / walking / crouched | 30 / 20 / 8 / 3 m | Stealth's speed per player |
| A vehicle with its engine running | 200 m | In a vehicle (heat already counts it) |
| A door opening, a forced lock, a search | 40 m | Door and search code |
| Remembered noises (8+ heat at once: bells, noisemakers, sirens) | heat × 10 m, at most 400 m | `RZ_HeatMap`'s last 16 |
| Banging at a door | 60 m | The door blockers |

Each kind multiplies the radius. Crawlers are the good listeners.

## 4. Blood

A bleeding player (the damage manager's bleeding state) leaves a scent: their last 6 positions, one
every 10 s, kept 60 s after the bleeding stops. A group within its kind's blood radius of the newest
point gets a lead at that point, with ±20 m of error, every 10 s. Blood never gives a fix on its own:
it pulls them along your trail. *"It's following the blood."*

## 5. Shrieks

An aware young (half the time) or a scout shrieks: a sound, plus *"A shriek from the ridge, and an
answer from the wood."* Groups of the same tribe within 200 m by day or 300 m at night get a lead
(±20 m). A group passes a shriek on at most once every 30 s, so it never chains across the map. The
built Aftermath screamers keep their heat; they're the town version.

---

## 6. Doors

Built: doorway blockers and navmesh rebuilds at locked places, banging at a locked door (heat), and
Bacon's own opening of shut, unlocked doors (B10).

| Behaviour | How | Status |
|---|---|---|
| **Find another way in** | After 30 s at a shut door, the group's slots move to another outside door of the same building within 20 m (`RZ_DoorLocks` knows them) that is open or unlocked. If none, they wait outside until the siege cap (10 minutes, approved), then leave | Feasible |
| **Break a weak door** | Only an **alpha**, or a bold tribe's night hunt with 3 or more bodies, banging for 45 s with loud tells. Only doors whose prefab name matches **Weak door words** (default `Shed,Barn,Garage,Wood`). Never a holding's, safehouse's, outpost's, quartermaster's or the lab's door. The door goes to the built **Forced** state (open and broken until the reset) | Feasible; the word list needs a play-test |

Hiding indoors is a real escape only behind a **locked** door. Behind a lock, only the alpha or a bold
hunt at night can break a weak door (decision 1).

---

## 7. Reactions

| Event | Crawler | Young | Hunter | Alpha (by temperament) |
|---|---|---|---|---|
| **Gunfire heard** | Lead: drift toward it. Its town is stirred | Rush the lead at RUN | Freeze 3 s, then flank toward the shooter (a Pincer from 2 sides) rather than straight in | Bold: comes, with a roar. Cautious: holds or withdraws. Others: like hunters |
| **Fire** (burning buildings, molotovs) | Slots never within 30 m of a burning building. An engaged body within 6 m of fire is taken back and moved 20 m away | Same | Same | Same |
| **Losses: group down by half** | Keeps coming | Keeps coming | The regroup, once; then give up | Bold: presses. Cautious: withdraws |
| **Wounded body** (health under 40%) | Keeps coming | Keeps coming | Breaks off to 50–80 m and holds; comes back in the next approach | Under 50%: goes home with the guard (approved) |
| **Player in a vehicle** | Ignore beyond 20 m | Chase up to 60 m, then give up | Ignore beyond 30 m | Charge if within 30 m and on foot speed (under 6 m/s); else ignore |

---

## 8. The ankle grab and the client effects

A move runs as the built ones do: the gate on an ENGAGE body, the wind-up behind the melee gate, Bacon's
melee, the bite detector's attribution, then the effect. Limits for every move stay as built: one special
move landing on a player every 10 s from all infected together, none in the gentle start, and **Own
attacks** off gives plain Bacon melee.

| Move | Kind | Trigger | Effect on a hit | Counterplay | Cooldown and caps |
|---|---|---|---|---|---|
| **Ankle grab** | Crawler | Target within 2 m, and the crawler is low (stance 0.6) or the target is prone or crouched; Bacon's crouch melee | **Held:** the player's speed capped at 0.25 for 1.5 s. A second hit within 4 s already counts as "grabbed" (×1.5 bite chance, built). *"Something has your ankle."* | Kill it, step back before it's in reach, don't go prone among crawlers | 20 s per crawler; one hold on a player at a time; never two in a row from different crawlers within 5 s |
| **Stagger** (on the built leap) | Young | A hit after the leap's sprint burst | Speed capped at 0.6 for 0.8 s | Sidestep: the leap goes straight | At most one stagger per player per 6 s |
| **Knockdown** (on the built charge) | Alpha | The charge reaches within 1.6 m | The player forced prone, movement off for 1.2 s, then Bacon's swing | Sidestep at the last moment, or get an obstacle between you | **Never** while 3 or more other infected are within 10 m of the player; at most once per player per 30 s |

**The client component.** `RZ_PlayerEffectsComponent` goes on the player controller prefab, next to
`RZ_PlayerUIComponent`, with owner-only RPCs:
- `Hold(seconds, speedFraction)`, through `SetSpeedLimit` on the player's own character;
- `Knockdown(seconds)`, through `ForceStance` prone and `SetDisableMovementControls`.

Every effect ends on a timer on the client and is cleared on death or respawn. Without the component,
hold, stagger and knockdown stay off, and the line and the built grab chance stay. It becomes a
WORKBENCH_SETUP item when built. Dragging a player is dropped ([ROADMAP](../docs/ROADMAP.md)).

**The play-test after the client test** (through the pc-setup skill): a crawler street's ankle grab, an
alpha charge and knockdown, and a young's stagger, each checking that the tell is heard before the
strike.

---

## 9. Settings

Still to add under **Zombies > Behaviour**: **Weak door words**; Order gap and Orders per 5 s (now fixed
at 5 s a group).

## 10. Build steps

Each ends with `tools/compile_check.sh` (exit 0, no ReforgerZ warnings) and docs in the same commit.

| Step | What | Lines (rough) |
|---|---|---|
| **A. Senses' rest** | The hearing table (section 3), blood (4), shrieks (5), the 2 s sight cache | 300 |
| **B. Doors** | Another way in and weak doors (6), **Weak door words** | 200 |
| **C. Reactions and the regroup** | The reactions table (7), the regroup and the element caps (2) | 300 |
| **D. Client effects** | `RZ_PlayerEffectsComponent`, the ankle grab, stagger and knockdown (8), after the client test | 300 |
| **E. The small rules** | Section 2's rest: the release delay, the kinds' tells, forest edge points, the resting ring asleep | 300 |

---

## 11. Risks

1. **Speed caps look like slow motion.** B7: under a cap the animation stays "run". The hold and the
   stagger need a client check before they ship.
2. **A knockdown in a crowd is a death sentence.** Hence the "3 or more others within 10 m" rule and
   one special per player per 10 s. Watch deaths by cause.
3. **Weak doors take away the last safe room.** Protected buildings and the word list keep it rare;
   play-test the word list on each map.
4. **Hearing everything makes them omniscient.** Every sound gives a lead with error, never a fix;
   tune the radii in play.

## 12. Decisions for the author

| # | Decision | Recommended |
|---|---|---|
| 1 | Weak doors break, but only for an alpha or a bold night hunt | **Yes**, with the word list and protected buildings. Or no, and hiding behind a *locked* door stays absolute |
| 2 | The alpha's knockdown | **Yes**, with its caps, after the client test (its charge can't break fences: B17) |
