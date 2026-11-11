# The infected behaviour layer

**Status: designed; most of it is built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): the hooks, modes, the five kinds' profiles, sight, formations, every hunt strategy (rush, stalk, pincer, encircle, flush, bait, herd, ambush, give up and the hand-off), navigation (water, stairs, the climb gate, the ladder), LOD, the hunter's lunge, the young's leap and the alpha's charge as the lab allows them, and the crawlers' daily life. What's left here is designed. Approved by the author on 2026-11-11 (the behaviour ideas). Lab tests B1–B20 have run; their verdicts are in each lever below and in the zombie lab section of [docs/WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md).


An addendum to the approved tribes design ([INFECTED.md](INFECTED.md)). It uses phase 0's measurements
(the zombie lab and the land survey) and a reading of Bacon Zombies 1.8.0's scripts and behaviour trees,
extracted from its Workshop pak.

**Lab results are in.** The zombie lab ran B1-B20 (section 15) on 2026-11-11. The feasibility calls below are
updated to match.

**What the author asked for.** Our code drives the infected everywhere except attack range. Inside
attack range Bacon's own attack is used, and each kind may get its own attack on top. That covers
speed, sight, how they move and where they go, navigation, and hunting: spread out rather than single
file, stalking and flanking in the open and in bush, vaulting low fences but not tall ones.

**The short answer.** Most of it can be done, through a lever phase 0 didn't use: Bacon's AI is
built from small scripted nodes, and Enforce Script lets us mod them. Bacon's main decision points
(where to walk, when to climb, whom to hear, when to swing) are such nodes, or methods on
`Bacon_622120A5448725E3_InfectedCharacter`. We change those few points and leave the trees and the
engine alone. The big step is **blind steering**: while our code drives a body, its engine
perception factor is 0 and Bacon's target intake is closed, so it can't charge on its own. Our own
senses decide when it has noticed you, and our code walks it into place. At attack range we hand it
to Bacon, which fights as it does today.

---

## Contents

1. [What we know](#1-what-we-know)
2. [The levers](#2-the-levers)
3. [Who controls when](#3-who-controls-when)
4. [The kinds](#4-the-kinds)
5. [Senses](#5-senses)
6. [Formations and flanking](#6-formations-and-flanking)
7. [Navigation and getting unstuck](#7-navigation-and-getting-unstuck)
8. [Hunt strategies](#8-hunt-strategies)
9. [Reactions](#9-reactions)
10. [Attacks](#10-attacks)
11. [Crawler daily life](#11-crawler-daily-life)
12. [Fairness tells](#12-fairness-tells)
13. [Settings](#13-settings)
14. [Performance budget](#14-performance-budget)
15. [VERIFY points and the zombie-lab tests](#15-verify-points-and-the-zombie-lab-tests)
16. [Build steps](#16-build-steps)
17. [Changes to the approved design](#17-changes-to-the-approved-design)
18. [Risks](#18-risks)
19. [Decisions for the author](#19-decisions-for-the-author)

---

## 1. What we know

### 1.1 Measured in phase 0

| Fact | Number | What it means here |
|---|---|---|
| Bacon moves every member to a waypoint at RUN | 3.7 m/s (heavy 4.9, exploder 4.1); group centre 3.5 m/s | A plain order always jogs. Walking needs a lever (section 2) |
| Chase speed | 3.6–3.7 m/s (heavy 4.8) | Bacon chases at RUN, not sprint. Prefab maximums reach 4.2 (civilian) to 5.4 (military), so sprint exists |
| Sight is a chance, not a radius | Day: 66% engaged within 30 s at 25 m, then about 25% per 30 s at 50–200 m. Night about 17%. A hill blocks it completely | Our own sense model should match these odds for hunters, so the tuned simulator stays valid |
| Spawn order | A body spawned after its target is in reach charges at once from 300 m (12 of 12) | Blind steering sets factor 0 in the spawn frame; B1 shows that's in time |
| Perception factor | Only `SCR_AICombatComponent.SetPerceptionFactor` sticks. Factor 0 listeners never came. Factor 0.25 by day: 0% engaged | Factor 0 is our "blind" switch. B1: set in the spawn frame it holds down to 6 m (0 of 24 engaged) |
| Sharing | Reaches 30–40 m, needs an infected that can see, doesn't chain past 55 m | Bacon's own pack logic is short. Long-range coordination has to be ours |
| Memory | A chaser kept walking toward the target's **real** position for 2 min and 380 m | A cheat. While our code drives, memory is ours and uses the last-known position |
| Gunfire | Blind listeners came from 80–170 m, never from 40 m or from 230 m or more | Our hearing for steered bodies uses 170 m for a shot as its base |
| Waypoints | Followed. Idle in place without one. Back to the waypoint 6 s after a kill. A held waypoint works only when removed and re-added | The held waypoint (`RZ_GroupOrders`) stays the group-level order |
| Re-order mid-fight | B4: 0 of 9 left a live fight for a 150 m order; they took it 7.5-10.6 s after the kill | Waypoints can't steer a fight. Taking a body back goes through our modes |
| Budget | 100 agents: p99 11.6 ms. 140: 14.5 ms. Engine active-AI limit 128 by default. Agents stayed at LOD 0 with no player near | The layer must stay cheap. LOD is a lever nobody uses yet |
| Health | `SetHealthScaled` applies (it returns false). No regeneration | Wound reactions can read health |

### 1.2 What Bacon's behaviour trees do

Read from the extracted `.bt` files. Every infected character runs `InfectedCharacterRoot.bt`.

| Step | Node (scripted ones in **bold**) | What it does |
|---|---|---|
| Think rate | **`GetAILODWait`**, then Idle | Waits 0.1 + 0.2 × the agent's LOD seconds each loop |
| Targets | `EvaluateTargets.bt`: `AITaskPickTarget` (last seen within 8 s), then **`AIBroadcastTarget`** | Picks a target it has seen. Shares it every 2–4 s as a danger event (`BaconZombies_TargetSharing`) |
| Suspects | `FindTarget.bt`: `AITaskPickTarget` (15 s), else **`AIGetShooter`** | Hears gunfire (suppressed only within a random 10–50 m), takes shared targets (accepted if closer than a random 10–50 m), and doors moved by an enemy within 15 m. Keeps a heard target 10 s |
| Climb (always, in parallel) | **`AIRequestClimb`**, then `CharacterJump` | Only when the body's velocity is exactly 0. Runs the engine's climb test and jumps if it says yes. No height limit of its own |
| Fight (when it has a target) | `Combat.bt` | Under 1.5 m with a clear trace (**`AICanMeleeTarget`**): crouch if the target's eyes are below its hip + 1 m (**`AIShouldCrouch`**), then the `CharacterMelee` action. Otherwise, if it saw the target within 0.5 s: `AITaskMove` to the target's aim position (0.5 s lead) with `SCR_CreateAIExtraMoveParams` (straight-path cost ×2). **`AIUpdateStance`** runs each loop |
| Roam (no target) | `Roam.bt` | With a suspect: SPRINT to the suspect's aim position (its real position). With a group waypoint: a point 3–8 m from it (**`ComputeLoiterPointInRadius`**), at RUN, then idle 3–4.5 s. Without one: a point 3–43 m from its loiter point, WALK if under 25 m away, else RUN, then idle 3–8 s |
| Voice | **`AIInfectedTaskSpeak`** | Idle, target and combat sounds by chance, through `PlaySoundServer` |

Other code that matters:
- **The heavy** (`InfectedCharacterHeavy`) trips other infected it touches at 4 m/s or more, and with
  `m_bDestroysThings` breaks any `SCR_DestructibleEntity` it touches.
- **Clumsiness.** Infected that touch each other can trip (`TryTrip`, unconscious for 1 s) when their
  prefab's clumsiness is under 1.
- **Stance.** Each body gets a random dynamic stance of 0.82–0.92 (hunched) through `UpdateStance()`.

**Two things follow, worth checking in the lab:**
- **Waypoints can't steer a fight.** Roam is the only branch that reads the group's waypoint, and it
  runs only while the body has no target. A re-order mid-fight should be ignored until the target is
  gone (B4).
- **Why it never forgets (measured, B5).** While Bacon holds a target, Combat's `AIUpdateLoiterPoint`
  copies the target's *real* position into the body's loiter point. The target drops after 8 s and the
  suspect after 15 s. Then Roam resumes, but it picks points 3-40 m round that loiter point, so it RUNs
  toward wherever the target really was. That's the 380 m in 2 minutes. A take-back (our slot) puts
  Roam on our point about 9 s later. An OFF body needs its loiter point reset (`SetLoiterPoint`).

### 1.3 Built code this leans on

| Code | Used for |
|---|---|
| `Core/RZ_GroupOrders.c` (held waypoint per group) | Group-level orders: marching, going home, searching |
| `Zombies/RZ_Stealth.c` (visibility 0–1 per player, perception factors, search and give-up) | Our sight model reads visibility. Stealth stops setting factors on bodies we drive |
| `Zombies/RZ_BiteDetector.c` (health drop with an infected in reach) | Confirming a hit, for our attacks |
| `Interactions/RZ_DoorBlockers.c`, `RZ_DoorLocks.c` (blockers, navmesh rebuilds, banging heat) | Doors |
| `Director/RZ_HeatMap.c` (heat, the last 16 remembered noises) | Hearing |
| `World/RZ_Land.c` (250 m land grid: trees, rivers, crossings, ridges) | Trails and cover |
| `UI/RZ_PlayerUIComponent.c` (owner-only RPCs on the player controller) | The pattern for client-side player effects |

---

## 2. The levers

Each behaviour below names its lever. **Works**, **works with limits** and **doesn't work** are zombie
lab results (B tests, 2026-11-11). **Feasible** means the API exists and phase 0 or the code shows it
works. **Spike** means it should work but still needs a test (most of these need a real client). **No**
means it can't be done; the fallback is given.

| # | Lever | How | Status |
|---|---|---|---|
| L1 | **Held group waypoint** | `RZ_GroupOrders.Order`: remove, move, re-add. One per group | Feasible (built, measured) |
| L2 | **Per-member slot** | Mod `InfectedCharacter.ComputeLoiterPointInRadius` to return the member's own slot target when our code set one. Roam uses it with or without a group waypoint. Free: no entity, no order | **Works** (B3: 4 of 4 within 4 m in all 12 cases) |
| L3 | **Gait by branch** | No group waypoint and a slot under 25 m away: Roam picks WALK, then idles 3–8 s (a walk-and-stop gait). Held waypoint: RUN | **Works** (B7: WALK 1.45 m/s while moving, 0.45–0.8 m/s with the stops; slots 30 m ahead RUN at 3.5) |
| L4 | **Speed cap per body** | `SCR_ChimeraCharacter.SetSpeedLimit(source, fraction, instant)`, through the engine's `OverrideMaxSpeed`. It's a character-level cap, so it holds in a chase too. Bushes use the same call | **Works with limits** (B7). The fraction is of the *maximum* (sprint) speed: 0.2 = 0.85 m/s, 0.4 = 1.8, 0.6 = 3.0, the same for every prefab; no effect on a run above about 0.7. The animation stays in its run state, so probably slow motion (needs a client to see) |
| L5 | **Movement type** | `AICharacterMovementComponent.SetMovementTypeWanted(SPRINT)` from our code during a chase. Combat's move node sets no speed | **Works with limits** (B7): it holds only if set every tick (civilian 4.4 m/s, heavy 6.1); set once, it fades (3.6). Also needed at every release (B6) |
| L6 | **Stance** | Mod `InfectedCharacter.UpdateStance()`: per-kind dynamic stance (`SetDynamicStance`), and `ForceStance` for lying up | **Works** for the dynamic stance (B7: 0.6 and 0.7 read back exactly). Lying up: a prone `SetStanceChange` didn't hold (B12); `ForceStance` untested |
| L7 | **Blind switch** | `SCR_AICombatComponent.SetPerceptionFactor(0)` through stealth's `RZ_TrySetPerceptionFactor`, set in the spawn frame | **Works** (B1: 0 of 24 engaged at 150/50/15/6 m; the combat path is ready in the spawn frame) |
| L8 | **Target intake gate** | Mod `AIGetShooter.ProcessEvent`: a body we drive ignores gunfire, shared targets and doors (our senses handle them). An engaged body hears by its kind | **Works** (B2, B18: steered bodies' events are blocked; they came only by our table) |
| L9 | **Share gate** | Mod `InfectedCharacter.AIBroadcastTarget`: share only if this body itself saw the target within 2 s. That ends the real-position chain. Crawlers share at half range | **Works** (B2: fires and blocks; B5's take-back uses it) |
| L10 | **Release injection** | Broadcast our own `BaconZombies_TargetSharing` event at the body, naming the target | **Not needed at a release** (B6: a steered body already has the target in its perception, so restoring the factor engages it in 0.2–0.5 s with or without the event). Still useful to name a target a body has never perceived (B10, B11) |
| L11 | **Climb gate** | Mod `AIRequestClimb.EOnTaskSimulate`: run the same `DoClimbTest`, read the grab point's height (`m_ClimbGrabPointLS` through `m_ClimbParent`), refuse above the kind's limit | **Works** (B8). Limits are *grab* heights: a 1.97 m rail fence is climbed at its 0.85 m rail. A refused body stops dead, so a refusal must count as a stuck tick |
| L12 | **Melee gate** | Mod `AICanMeleeTarget.TestFunction`: false while our own attack's wind-up runs | **Works** (B14: 93 swings held in 10 s) |
| L13 | **Think rate** | `AIAgent.SetPermanentLOD` (Bacon's own wait follows the LOD), not a hook on `GetAILODWait` | **Works** with `SetPermanentLOD` (B12: it sticks; loops fall from 7.4 to 1.8, 1.05 and 0.67 a second at LOD 2, 4 and 6; p99 −2.3 ms at LOD 6). A wait hook set the loop rate but left 3 of 5 orders unanswered for 50 s: not used |
| L14 | **Sleep** | `AIAgent.DeactivateAI()` for bodies lying up, `ActivateAI()` to wake them | **Works with limits** (B12: the tree stops and the active-AI count drops; awake in 0.2 s, moving in 6.5 s. The prone pose didn't hold) |
| L15 | **Voice** | `InfectedCharacter.PlaySoundServer("SOUND_IDLE" / "SOUND_TARGET")`. Mod `AIInfectedTaskSpeak` to keep stalkers quiet | **Works** (B3: muted bodies played 0 sounds) |
| L16 | **Our danger events** | A scripted `AIDangerEventWeaponFire` pulls engaged bodies the way a shot does (measured) | Feasible |
| L17 | **Spawn placement** | Phase 0's rule: never within 300 m of a player in terrain line of sight | Feasible (designed) |
| L18 | **Navmesh snap** | `AIPathfindingComponent.GetClosestPositionOnNavmesh` | **Doesn't work as written** (B9: no `AIPathfindingComponent` on Bacon's group, character or agent). Fallback: no snap (B3's unsnapped slots all worked); a vanilla group's or the AI world's navmesh is untested |
| L19 | **Out-of-sight move** | `SetOrigin` on a body nobody can see, for getting unstuck | Feasible; never in view |
| L20 | **Player effects, server side** | `SCR_CharacterDamageManagerComponent.AddParticularBleeding`; the bite chance | Feasible |
| L21 | **Player effects, client side** | A new `RZ_PlayerEffectsComponent` on the player controller, with owner-only RPCs: `SetSpeedLimit` on the player's own character, `ForceStance`, `SetDisableMovementControls` for a moment. A player's movement is predicted on their own machine, so these must run there | Spike (needs a real client; not tested in the lab) |
| L22 | **Scripted jump and melee** | `CharacterControllerComponent.SetJump(1)`, `SetMeleeAttack(true)` on an AI body | Jump **works** (B14; a jump in place with little forward travel). `SetMeleeAttack` **doesn't work** (never registers), and `IsMeleeAttack()` never reads true on the server |
| L23 | **Pathing bias** | Mod `SCR_CreateAIExtraMoveParams` only for Bacon bodies: straight-path cost per kind (×1 young, ×3 hunters), so a chase curves | Spike: the hook loads and fires (B2); its effect isn't tested |
| — | Editing Bacon's `.bt` files | Overriding Bacon's tree resources in our mod | **Not used.** It would break on every Bacon update. The hooks above avoid it |
| — | Dragging a player | Moving a player's position from the server fights their machine's prediction | **No.** Fallback: the grab holds you in place (section 10) |
| — | A real per-body path planner | Our own A* over the navmesh | **No** (cost). We pick points and the engine paths between them |

**Where it lives.** Every modded Bacon class goes in one file, `Zombies/Behaviour/RZ_BaconHooks.c`,
and each hook only reads fields our code set on the character. That keeps the Bacon-specific surface
small and easy to fix when Bacon updates.
- **Fields on the character** (a modded `Bacon_622120A5448725E3_InfectedCharacter`): mode, kind
  profile id, slot target and its time, gait wanted, stance wanted, attack state.
- **No map lookups in hooks.** A hook runs every behaviour-tree loop of every agent, so it only reads
  those fields.

---

## 3. Who controls when

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): the four modes, releases, take-backs and RECOVER. Still designed:
the release's reaction delay of 0.2-1.2 s per body.

---

## 4. The kinds

**Built** for the young, hunters, scouts, the guard and the alpha ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): caps,
stance, sight, memory, reach, climb limits and individual variation. Still designed: the crawler's
profile in use (the crawlers aren't driven yet, section 11), blood scent radii, the young's erratic
slot jumps and stares at houses, the hunter's freeze when you look its way outside a stalk, the alpha's
roar before a charge.

---

## 5. Senses

While our code drives, our senses decide. They work per group, not per body: one leader's eye, once a
second, only for groups with a player within 250 m. Engaged bodies use Bacon's perception (with
stealth's factor), so the close fight still feels like Bacon.

### 5.1 Sight: the notice meter

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)). Still to do: calibrate **Notice rate** to phase 0's ladder in a
play-test and re-run `tools/tribe_sim.py` with it.

### 5.2 Hearing

A sound gives a group a **lead**: a position with an error of 15% of the distance. A lead starts an
investigation (walk-stop toward it), never a fix on you.

Built: the unsuppressed (170 m) and suppressed (30 m) shot, as a lead. The rest of this table is
designed:

| Sound | Base radius | Source |
|---|---|---|
| Unsuppressed shot | 170 m | `RZ_Stealth.OnPlayerShot`; phase 0 measured Bacon's 80–170 m |
| Suppressed shot | 30 m | The same |
| Sprinting / running / walking / crouched | 30 / 20 / 8 / 3 m | Stealth's speed per player |
| A vehicle with its engine running | 200 m | In a vehicle (heat already counts it) |
| A door opening, a forced lock, a search | 40 m | Door and search code |
| Remembered noises (8+ heat at once: bells, noisemakers, sirens) | heat × 10 m, at most 400 m | `RZ_HeatMap`'s last 16 |
| Banging at a door | 60 m | The door blockers |

Each kind multiplies the radius (section 4). Crawlers are the good listeners.

### 5.3 Blood

A bleeding player (the damage manager's bleeding state) leaves a scent: their last 6 positions, one
every 10 s, kept 60 s after the bleeding stops. A group within its kind's blood radius of the newest
point gets a lead at that point, with ±20 m of error, every 10 s. Blood never gives a fix on its own:
it pulls them along your trail. *"It's following the blood."*

### 5.4 Memory, shrieks and hand-offs

- **Memory.** An aware group keeps your last-known position and heading. Unseen for the kind's memory
  time, it goes to that spot, searches it (stealth's built search, 45 s), and gives up. It always uses
  the last-known spot, never your real one.
- **Shrieks.** An aware young (half the time) or a scout shrieks: a sound, plus
  *"A shriek from the ridge, and an answer from the wood."* Groups of the same tribe within 200 m by
  day or 300 m at night get a lead (±20 m). A group passes a shriek on at most once every 30 s, so it
  never chains across the map. The built Aftermath screamers keep their heat; they're the town
  version.
- **Hand-off:** built ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)).

### 5.5 Line of sight without seeing through walls

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): terrain samples, a trace within 80 m, 12 a second. Still
designed: a 2 s cache per check.

---

## 6. Formations and flanking

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): slots, file, line, wedge, pairs, ring, the spread, the fan on a
strike, the flank with its sidestep and cover choice, the stalk shadow and its freeze, the 3 m minimum.
Still designed: the crawler herd blob, a re-plan only when the target moves 25 m (the code re-plans each
second), an ETA so the slower element starts first.

## 7. Navigation and getting unstuck

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): water checks on slots, stairs (wait below), stuck detection, the
unstick ladder and stuck marks, the climb gate per kind. Still designed: **forest edge points** in the
land cache (the code uses the square's tree class and its open neighbours), low ground preferred on a
leg, and the navmesh snap (B9: not on Bacon's bodies).

### 7.5 Doors

Built: locked places get doorway blockers and navmesh rebuilds, and infected at a locked door bang on
it (heat).

| Behaviour | How | Status |
|---|---|---|
| **Bang** | Built (`RZ_DoorBlockers`) | Built |
| **Find another way in** | After 30 s at a shut door, the group's slots move to another outside door of the same building within 20 m (`RZ_DoorLocks` knows them) that is open or unlocked. If none, they wait outside until the siege cap (10 minutes, approved), then leave | Feasible |
| **Break a weak door** | Only an **alpha**, or a bold tribe's night hunt with 3 or more bodies, banging for 45 s with loud tells. Only doors whose prefab name matches **Weak door words** (default `Shed,Barn,Garage,Wood`). Never a holding's, safehouse's, outpost's, quartermaster's or the lab's door (the "can't burn or be locked" list). The door goes to the built **Forced** state (open and broken until the reset) | Feasible; the word list needs a play-test |
| **Unlocked closed doors** | **They open them** (B10: a shut, unlocked door opened in about 9 s, the target inside reached in 11 s) | Measured |

Hiding indoors is a real escape only behind a **locked** door (the built locks and blockers): B10
showed the infected open shut, unlocked doors. Behind a lock, only the alpha or a bold hunt at night can
break a weak door. That trade-off is one of the decisions for the author (section 19).

---

## 8. Hunt strategies

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)): the state machine, a moment, Rush, Stalk, Pincer, Encircle, Flush,
Bait, Herd, Ambush, Give up and the hand-off, with the temperament weights and their needs. Still
designed: a regroup 60–100 m back in cover once per target, and at most 3 elements and 2 strategies
near one player cluster.

---

## 9. Reactions

| Event | Crawler | Young | Hunter | Alpha (by temperament) |
|---|---|---|---|---|
| **Gunfire heard** (5.2) | Lead: drift toward it. Its town is stirred | Rush the lead at RUN | Freeze 3 s, then flank toward the shooter (a Pincer from 2 sides) rather than straight in | Bold: comes, with a roar. Cautious: holds or withdraws. Others: like hunters |
| **Fire** (burning buildings, molotovs) | Slots never within 30 m of a burning building. An engaged body within 6 m of fire is taken back and moved 20 m away | Same | Same | Same |
| **Losses: group down by half** | Keeps coming (crawlers don't care) | Keeps coming | Regroup 60–100 m back in cover, once; then give up | Bold: presses. Cautious: withdraws. Approved commitment cap and long call above this |
| **Wounded body** (health under 40%) | Keeps coming | Keeps coming | Breaks off to 50–80 m and holds; comes back in the next approach | Under 50%: goes home with the guard (approved) |
| **Hit while steered** | Release (section 3) | Release | Release | Release |
| **Flashlight or a lamp on them** | Stealth already raises the player's visibility | Same | Same | Same |
| **Player in a vehicle** | Ignore beyond 20 m | Chase up to 60 m, then give up | Ignore beyond 30 m | Charge if within 30 m and on foot speed (under 6 m/s); else ignore |
| **Bodies of their own** (approved vigil) | — | — | — | The vigil: stand round the alpha's spot unsteered |

---

## 10. Attacks

**Built:** the hunter's lunge and bite, the young's leap as the lab allows it (a sprint burst into the
swing, no stagger), the alpha's charge with its stumble (no knockdown), the attribution of a hit, and
the per-player limits ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)). The rest below is designed: the ankle grab, and the
client effects (hold, stagger, knockdown) after a client test.

**The rule.** Bacon's melee stays the hit and its damage stays the damage. Each kind adds one special
move, made from animations Bacon already uses (melee, jump, crouch, sprint, the heavy's contact) and
scripted effects. The effects are status (a hold, a stumble, a knockdown, a little bleeding), never
extra damage. Nothing stacks past the bite chance's cap of 0.6.

**How a move runs:**
1. **Gate.** Our overlay on an ENGAGE body checks the trigger once a second.
2. **Wind-up.** The melee gate (L12) holds Bacon's swing for up to 1.5 s, and gait and jump do the move.
3. **Hit.** Bacon's melee lands or misses.
4. **Confirm.** The bite detector already sees a player's health drop with an infected in reach. It now
   attributes the hit to a body whose melee test (the L12 hook on `AICanMeleeTarget`) passed within 1 s
   and 2.2 m, else the nearest in reach. (B14: `IsMeleeAttack()` never reads true on the server.)
5. **Effect.** On a confirmed hit after a move, its effect applies.

| Move | Kind | Trigger | Made of | Effect on a hit | Counterplay | Cooldown and caps | Status |
|---|---|---|---|---|---|---|---|
| **Ankle grab** | Crawler | Target within 2 m, and the crawler is low (stance 0.6) or the target is prone or crouched | Bacon's crouch melee (`AIShouldCrouch` already crouches for a low target) | **Held:** the player's speed capped at 0.25 for 1.5 s (client effect, L21). A second hit within 4 s already counts as "grabbed" (×1.5 bite chance, built). *"Something has your ankle."* | Kill it, step back before it's in reach, don't go prone among crawlers | 20 s per crawler; one hold on a player at a time; never two in a row from different crawlers within 5 s | Spike (needs a client; not tested in the lab). **Drag: no.** Fallback without the client component: the line and the built grab chance only |
| **Leap** | Young | Target 3–6 m ahead (within ±25°), the young running at 3 m/s or more | Sprint (L5), then `SetJump(1)` (L22), then Bacon's melee | **Stagger:** speed capped at 0.6 for 0.8 s | Sidestep: the leap goes straight | 15 s per young; at most one stagger per player per 6 s | The jump works but goes up, not forward (B14: 1.3 m in 1.5 s). Use the fallback: a sprint burst (SPRINT re-set each tick, B7) into a normal swing. The stagger needs a client |
| **Lunge and bite** | Hunter | Target 6–10 m away, from cover or out of the target's view (more than 60° off their facing) | Sprint for 1.5 s (L5), then Bacon's melee | **A bite attempt:** the bite chance ×1.5 (still capped at 0.6), and light bleeding on an arm or a leg (`AddParticularBleeding`, server side) | Watching your back. Bandages | 20 s per hunter; at most one bleed per player per 30 s | Feasible (server only); the sprint holds only re-set each tick (B7) |
| **Charge and knockdown** | Alpha | Target 12–30 m away, in line of sight, a clear line at waist height (one trace), the target on foot | A roar (a tell), sprint straight (about 6 m/s with SPRINT re-set each tick, B7; up to 4 s). Its contact already trips infected (Bacon); it doesn't break fences (B17) | **Knockdown** if it reaches within 1.6 m: the player forced prone, movement off for 1.2 s (client effect), then Bacon's swing. **A miss:** the alpha's speed capped at 0.2 for 2.5 s (it overruns and stumbles). That's the window to hurt it | Sidestep at the last moment, or get an obstacle between you | 30 s per alpha. **Never** while 3 or more other infected are within 10 m of the player. At most once per player per 30 s. Never in the gentle start | Spike (the knockdown needs a real client; the sprint and the stumble's cap are measured, B7). Fallback: the charge and the stumble without the knockdown |
| Guard | — | — | Bacon's melee | — | — | — | — |
| Scout | — | — | The shriek (5.4) | — | — | — | Feasible |

**Limits for every move:**
- at most one special move landing on a player every 10 s, from all infected together;
- none during the approved gentle start;
- **Zombies > Behaviour > Own attacks** off gives plain Bacon melee.

**The client component.** `RZ_PlayerEffectsComponent` goes on the player controller prefab, next to
`RZ_PlayerUIComponent`. It has owner-only RPCs:
- `Hold(seconds, speedFraction)`, through `SetSpeedLimit` on the player's own character;
- `Knockdown(seconds)`, through `ForceStance` prone and `SetDisableMovementControls`.

Every effect ends on a timer on the client and is cleared on death or respawn. Without the component,
hold, stagger and knockdown turn off, and bleeding and bite chance stay. It's a WORKBENCH_SETUP item.

---

## 11. Crawler daily life

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#crawler-life)): lying up with the AI off beyond 40 m
(L14), waking with a groan, the dusk herd round the doorway, the night herds after an anchor's loop,
stirred anchors toward the noise, giving way to a pack, and the crawler speed cap as a setting (off
until a client has seen it). Still designed: a forced prone pose (`ForceStance` untested; a prone
stance change didn't hold in B12), the crawler steered into a fight within 12 m, and the ankle grab
(section 10).

---

## 12. Fairness tells

| Rule | How it's kept |
|---|---|
| **Heard before seen** | Every approach makes a sound before the strike. A suspicious body (meter 0.4) makes one sound. A strike order comes with a roar or a shriek from the leader, 1–2 s before the release. Stalkers stay quiet (L15), but a stalk always gives one tell (a growl at 40–60 m, or a line such as *"The crows went up off the hedge."*) before it strikes. No strike ever comes from total silence |
| **Never spawn in view** | Phase 0's rule stays: never within 300 m of a player with terrain line of sight. Blind spawning (B1) removes the instant charge, but not this rule |
| **No seeing through walls** | Our sight needs terrain and building line of sight (5.5). Bacon's melee already needs a clear trace. Bacon's own engine sight behind a hill measured 0%, and inside a house 0 of 6 (B19) |
| **No real-position cheat** | While we drive, memory is the last-known spot. The share gate (L9) stops the chase chain. The take-back rule ends a chase once our senses lose you |
| **Blind isn't harmless** | Contact (8 m with line of sight, 3 m without), a hit, or a shot within 30 m releases a body at once |
| **No target switching** | A hunt stays on its player (approved). Hand-offs pass the same target |
| **Escapes are real** | Indoors behind a door (except an alpha or a bold night hunt at a weak door), upper floors, 300 m, no man's land, a vehicle, or a give-up that lasts 10 minutes |
| **Knockdown can't be fatal by design** | Never with 3 or more others within 10 m. 30 s apart |
| **Special moves are rare** | One per player per 10 s from all infected together |
| **New players** | No strategies other than Rush, and no special moves, in the gentle start |

---

## 13. Settings

**Built** as **Zombies > Behaviour** (docs/WORKBENCH_SETUP.md, Settings objects), Sleep lying crawlers
among them. Still designed, with their parts: Order gap and Orders per 5 s (fixed at 5 s a group), Weak
door words.

## 14. Performance budget

**Built** within the lab's measured budget ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)), crawlers asleep beyond 40 m
among it. Still designed: the resting ring asleep with its AI off (L14).

---

## 15. VERIFY points and the zombie-lab tests

These join WORKBENCH_SETUP's VERIFY list (as items 16 onward) when the code goes in. Each test runs in
`tools/zombie_lab/` on Everon with Bacon 1.8.0 (`ONLY=behaviour`), unless it says a client is needed.
**All twenty ran on 2026-11-11** (one session of about 55 minutes of server time in three boots, 0 script
exceptions).

| # | VERIFY | Pass | Result | Verdict |
|---|---|---|---|---|
| **B1** | Factor 0 set in the spawn frame keeps a body blind | No engagement at 15 m or more; at 6 m only through our release | 0 of 24 engaged at 150/50/15/6 m in 30 s; released at 6 m: engaged in 0.3-0.6 s, contact 1.8-5.2 s. Control (factor 1): 2 of 2 engaged | **Works** |
| **B2** | Modded Bacon classes load and run | Every hook fires; no exceptions | Every hook fired (counts in the results); 0 exceptions in 3 runs | **Works** (L23's override wired, effect untested) |
| **B3** | Per-member slots through `ComputeLoiterPointInRadius` | Each body within 4 m of its slot | 4 of 4 in all 12 cases (file, line, wedge; with and without a held waypoint); mean 0.0-0.8 m; settled in 12-36 s | **Works** |
| **B4** | A re-order mid-fight doesn't pull a group off a live target | Keeps fighting; takes the waypoint within 10 s of the kill | 0 of 9 left; took it 7.5-10.6 s after the kill | **Works** as predicted |
| **B5** | What keeps a chase alive; how long a take-back takes | Target gone and Roam on our slot within 20 s of the take-back | The loiter point holds the target's real position. Target dropped ~8 s after it vanished; after the take-back Roam on our slot in ~9 s | **Works** |
| **B6** | Our injected share event engages a body at once | Engaged within 2 s with injection | Engaged in 0.2-0.5 s with *or without* it. But the released body walks (0.94 m/s): no contact from 30 m in 20 s, 13.8 s from 15 m, 8.7 s from 10 m | Injection **not needed**; release **works with limits** (set RUN at release; release inside ~15 m) |
| **B7** | Gait levers | Fraction to m/s; a sprint that holds 2 s or more | Cap 0.2/0.4/0.6/0.8 = 0.85/1.8/3.0/no effect m/s (a fraction of max speed, every prefab). Animation stays in its run state. SPRINT re-set each tick: 4.4 m/s (heavy 6.1); set once it fades. Walk-stop 0.45-0.8 m/s. Stance 0.6/0.7 holds | **Works with limits** (looks need a client) |
| **B8** | The climb gate | Vaults at or under the limit, goes round above it | Gate refuses by grab height (1.2 m limit refused a 1.39 m fence 223 times); a 1.97 m rail fence was climbed at its 0.85 m rail; refused bodies stop dead, never go round | **Works** (grab heights; refusal = stuck tick) |
| **B9** | Stuck detection, the ladder; the cost of a snap | Through within 60 s; snap under 0.1 ms | All bodies past runtime walls in 17-21 s, no stuck tick, ladder unused; no `AIPathfindingComponent` on Bacon bodies | Ladder **not exercised**; snap **doesn't work** (go unsnapped) |
| **B10** | Shut, unlocked doors | Record | They open them: in ~9 s, target reached in ~11 s | **Measured**: only locks stop them |
| **B11** | Stairs | Record | 0 of 2 reached a target 3.8 m up in 70 s (one got 2.7 m up) | **Doesn't work** reliably: wait-below rule |
| **B12** | LOD and sleep | LOD sticks; wake under 1 s; a frame saving | `SetPermanentLOD` sticks; loops 7.4 → 0.67 a second; p99 −2.3 ms at LOD 6. Our wait hook left 3 of 5 orders unanswered. `DeactivateAI` stops the tree; wake 0.2 s; prone didn't hold | LOD **works**; wait hook **not used**; sleep **works with limits** |
| **B13** | Line-of-sight cost | Terrain under 0.05 ms, trace under 0.2 ms | 0.002 ms and 0.021 ms | **Works** |
| **B14** | Attack pieces | Each works | `SetJump(1)` jumps (in place). `SetMeleeAttack` and `IsMeleeAttack()` never true on the server. Melee gate held 93 swings. Client effects not tested | Jump **works**; melee read **doesn't work**; client part **untested** |
| **B15** | Frame time with the layer | 100 agents: p99 at or under 12.5 ms | 100: p99 11.93, mean 4.41 (layer off in the same run: 13.17, 4.56); 140: p99 13.03 | **Works** |
| **B16** | Water | No body enters water outside crossings | Raw slots across a river were waded (0.4-0.7 m deep); checked slots and edge slots stayed dry. No pond found | **Works** (and is needed) |
| **B17** | The heavy's contact | Fences broken | `m_bDestroysThings` on, 142 contacts, fences 5000 → 5000 HP; it climbed over | **Doesn't work**: the alpha goes round |
| **B18** | Our hearing against Bacon's | Steered bodies follow our table only | Steered: came from 100 and 170 m by our table, not 230; Bacon's events blocked. Engaged: 100 m 2 of 2, 170 m 1 of 2, 230 m 0 | **Works** |
| **B19** | Seeing through walls | Engine sight respects walls | 0 of 6 saw a target inside a house from 10-30 m | **Works** (respects walls) |
| **B20** | A formation reads as a pack | Spread and timing as section 6 says | Fan 63 m wide at 52 m out; pincer pairs 134-138° apart, in position 10 s apart; pair members came within 0.8 m | **Works** (enforce the 3 m minimum in code) |

**A play-test after the lab** (needs a client, through the pc-setup skill on the author's PC):
- a solo day crossing past a stalking day party;
- a pincer at night;
- a crawler street lying up and waking;
- an alpha charge and knockdown;
- an ankle grab.

Each one checks the tells are heard before the strike.

---

## 16. Build steps

They fit the approved tribes phases. Every step ends with `tools/compile_check.sh` (exit 0, no
ReforgerZ warnings), and the runtime test where the phase says so. Docs go in the same commit.

| Step | When | What | Lines (rough) |
|---|---|---|---|
| **0b. The behaviour spike** | Before phase 1 | **Done 2026-11-11**: B1–B20 in `tools/zombie_lab/` (B14's client part waits for a play-test). Lab scripts only, nothing in `scripts/Game/ReforgerZ` | Lab: about 4,900 |
| **1a. Hooks and modes** | Phase 1, with the plumbing | `Zombies/Behaviour/RZ_BaconHooks.c` (the modded Bacon character and nodes: L2, L6, L8, L9, L11, L12, L15; the lab's `RZL_Hooks.c` is the tested starting point). `RZ_Behaviour.c`: modes, the control rule (releases set the movement type), kind profiles, individual variation, gait levers, LOD bands through `SetPermanentLOD`. Stealth told about modes | 900 |
| **1b. Navigation** | Phase 1 | `RZ_Unstick.c`: stuck checks, the ladder, stuck marks. The climb limit. Doors: find another way, weak doors. Water checks on slots. Land survey edge points | 500 |
| **1c. Crawler life** (built, with tribes phase 3) | Phase 1, with crawlers | Lying up, waking, herd blobs, night drift loops, scavenging bodies, giving way, the crawler's speed cap | 350 |
| **2a. Senses and formations** (built, but blood, shrieks, hand-offs and the full hearing table) | Phase 2 slice | `RZ_BehaviourSenses.c` (meters, hearing table, blood scent, memory, shrieks, line-of-sight cache). `RZ_Formations.c` (slots, spread, fan, flank points with cover, stalk shadow) | 750 |
| **2b. The first strategies** (built, but the alpha's charge) | Phase 2 slice | `RZ_HuntPlan.c`: the state machine, Rush, Stalk, Pincer, Give up. The hunter lunge and the alpha charge (server effects only: bleed, the miss stumble). Play-test the slice (the approved list, plus a stalk and a pincer) | 700 |
| **2c. Everything else** (built, with tribes phase 3, but the reactions table and the client component) | Phase 2, all tribes | Encircle, Flush, Bait, Herd, Ambush, Hand-off; temperament weights; reactions table. `RZ_PlayerEffectsComponent` and the hold, stagger and knockdown after the client test. Re-run `tools/tribe_sim.py` with our sight model | 900 |

Total about 4,100 lines of game code on top of the approved estimate. The lab scripts (about 4,900
lines with the hooks) are written.

**Docs, as CLAUDE.md requires:**
- **`docs/INFECTED.md`** (created in phase 1) gains a section, **"How they move and hunt"**: the
  control rule, kinds, senses, formations, navigation, strategies, attacks and crawler life. It owns
  the topic.
- **DESIGN.md:** a short paragraph in "Stealth" with a link. Stealth no longer touches bodies our code
  drives.
- **WORKBENCH_SETUP:** the Behaviour settings, `RZ_PlayerEffectsComponent` on the player controller,
  the new VERIFY items and count, and the log line.
- **MOD_STACK:** the Bacon version the hooks are written against, and the list of hooked classes.
- **ROADMAP:** Next and play-test, and the Dropped list gets "dragging a player: the server can't move
  a player smoothly".
- **README:** the feature line.

---

## 17. Changes to the approved design

| Approved | Becomes |
|---|---|
| *"Bacon Zombies' AI does the fighting, untouched. The script only chooses where and when bodies appear."* | Bacon does the fighting inside attack range. ReforgerZ hooks a few of Bacon's scripted nodes (where to walk, climb height, hearing, sharing, melee timing, think rate, voice) and drives bodies beyond attack range |
| *"Inside P, nothing that matters may depend on a waypoint ... all choreography near players comes from where and when bodies appear."* | Inside P, steered bodies are blind and our code places them by slots, so flanks, rings and stalks are steered, not only spawned. Placement rules still decide where bodies first appear |
| *"Within 80 m of a player a group is never redirected."* | Replaced by the control rule (section 3). Engaged bodies aren't redirected; steered ones are, down to contact. OFF-mode bodies keep the 80 m rule |
| *"No perception hooks beyond stealth's factor."* | Our own senses for steered bodies; stealth's factor for engaged ones |
| Day parties *"hop and hold"* at a run | Hop and hold kept as the data rhythm, walked at walk-stop |
| Crawlers *"need a slow prefab in the stack"* | The speed cap makes them slow (B7), but a capped body probably runs in slow motion: the slow prefab stays until a client check |
| *"Hiding in a house is a real escape"* | Only behind a locked door (B10: unlocked doors are opened), and then except an alpha or a bold night hunt at a weak door |
| Stealth's search loses hidden players for every tracked body | Only for ENGAGE and OFF bodies. Steered bodies use our memory |

---

## 18. Risks

1. **Bacon updates.** The hooks name Bacon 1.8.0's classes and methods. A rename breaks the compile,
   because Bacon is a hard dependency. Mitigation: one file of hooks, each tiny; MOD_STACK pins the
   version; the compile check catches it at once.
2. **Blind steering looks dumb.** A steered body that ignores a player in plain view at 40 m might read
   as broken. Mitigation: the suspicious state turns and freezes them, the contact and shot rules
   release them, and blind bodies only exist while a plan is running. Watch it in play-tests.
3. **Blind steering feels like cheating the other way.** Strikes that come with no warning. Mitigation:
   the tells (section 12) are rules, not flavour. Play-test that every strike was heard first.
4. **Take-back doesn't take.** Retired by B5: a take-back puts Roam on our slot in about 9 s. The
   remaining risk is OFF bodies, which keep running to a lost target's real position (their loiter
   point) until it's reset.
5. **Slots fight Bacon's own loop.** Roam idles 3–8 s between moves, so formations move in steps.
   That's fine for stalking, poor for a fast fan. Fast moves use marches (held waypoint, RUN).
6. **Speed caps look like slow motion.** B7: under a cap the animation's movement state stays "run",
   so it most likely does. Crawlers keep a slow prefab until a client check.
7. **Performance.** Hooks run very often. They must only read fields. B15 gates it.
8. **Fairness of special moves.** A knockdown in a crowd is a death sentence. That's why it has the
   "3 or more others within 10 m" rule and one special per player per 10 s. Watch deaths by cause.
9. **Our sight model drifts from what the simulator assumed.** Re-run `tools/tribe_sim.py` with the new
   numbers before tuning tribes (step 2c).
10. **Stealth and the layer both own perception factors.** Clear rule: stealth sets factors only on
    ENGAGE and OFF bodies. A bug there gives blind bodies in a fight, or sighted bodies while steered.
    Log modes on the status line.
11. **Workshop maps.** Navmesh holes, odd fences and missing land data (no forests found). The ladder
    and the "give up" step cover holes. Cover falls back to terrain only.
12. **Other zombie mods.** The layer only works with Bacon (and Special Zombies, which builds on it).
    With anything else every body stays OFF, and the approved design's placement-only behaviour is
    what ships. The lab found that with Special Zombies as a hard dependency in `addon.gproj`, the 1.9
    server compiles none of ReforgerZ's scripts, without an error (results file); that needs fixing
    before any of this is built.
13. **Releases that walk in.** B6: a released body walks unless our code sets RUN or SPRINT. A missed
    call makes every strike a stroll. The release sets it in one place, and the debug line counts
    releases that didn't close in 10 s.

---

## 19. Decisions for the author

| # | Decision | Recommended |
|---|---|---|
| 1 | Blind steering (perception 0 while our code drives) | **Yes**, with the contact, hit and shot releases, and the suspicious tell |
| 2 | Engage radii per kind (section 4) | As in the table; tune in the slice |
| 3 | Weak doors break, but only for an alpha or a bold night hunt | **Yes**, with the word list and protected buildings. Or no, and hiding behind a *locked* door stays absolute (B10: unlocked doors are opened by any infected) |
| 4 | The alpha's knockdown | **Yes**, with its caps, after the client test (its charge can't break fences: B17) |
| 5 | The crawler's drag | **Dropped** (the hold instead) |
| 6 | Moving a stuck body out of sight (ladder step 4) | **Yes**, only out of every player's view |
| 7 | The encircle always leaves a gap | **Yes** |
| 8 | Hearing and sight numbers (sections 4 and 5) as defaults | Yes; calibrate the hunter to phase 0's ladder in the slice |
