# The infected behaviour layer

**Status: designed, not built.** Approved by the author on 2026-11-11 (the behaviour ideas). Lab tests B1–B20 have run; their verdicts are in each lever below and in the zombie lab section of [docs/WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md).


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

**The rule.** Beyond attack range, our code drives. Inside it, Bacon fights. Each body is in one of
four modes, stored on the character.

| Mode | Who drives | Perception factor | Bacon's intake (L8) | How it moves |
|---|---|---|---|---|
| **OFF** | Bacon, as today | Stealth's | Open | Mission infected, relentless hordes and the final stand stay here, unchanged |
| **STEER** | Our code | 0 | Closed | Slots (L2), gait (L3–L6), held waypoint for marches (L1) |
| **ENGAGE** | Bacon | Stealth's × the kind's base | Open, filtered by kind | Bacon's chase and melee. Our overlay: gait in the chase (L4, L5), our attacks (section 10), stuck watch, fire avoidance |
| **RECOVER** | Our code, waiting | 0 | Closed | Taken back from Bacon. Its slot is the last-known spot or home. It becomes STEER once Bacon's own target has cleared (B5: the target clears 8 s after it was last seen, the suspect at 15 s; Roam is on our slot about 9 s after the take-back) |

**Release: STEER to ENGAGE.** Any one of these, per body:

| Trigger | Why |
|---|---|
| The hunt strategy orders the strike (section 8) | The plan says go |
| A target within the kind's **engage radius** E (section 4), with line of sight by our check | Attack range |
| Any player within **8 m** with line of sight, or within **3 m** without | Contact. A blind body must never stand next to you |
| The body takes damage | It fights back. Never a punching bag |
| A player fires an unsuppressed shot within 30 m of it | Too close to ignore |
| A member of its own group within 15 m was released | The group moves together (our sharing, one hop only) |

**Every release sets the movement type** (RUN, or SPRINT re-set every tick: L5). Without it a
released body walks in at about 1 m/s, because Combat's move sets no speed and the last one set was
Roam's WALK (B6: from 30 m it stopped 11-17 m short in 20 s; from 15 m contact took 14 s, from 6 m
2-5 s). A released body engages within 0.5 s whatever the distance, because a steered body already has
you in its perception: no share event is needed (L10). Until RUN-at-release is tested, keep releases
inside about 15 m.

**Take back: ENGAGE to RECOVER.** Any one of these, after at least 10 s in ENGAGE:

| Trigger | Why |
|---|---|
| The target is beyond 2.5 × E for 8 s | Out of attack range: back to us |
| Our senses lost the target for the kind's memory time (section 5), and it's more than 20 m away | Honest memory, not the real-position cheat |
| The hide rule, the long call, the leash or the commitment cap (approved design) | Breaking off is ours |
| The target died or left the server | Nothing to fight |

**Never take back** a body that has a player within 15 m in line of sight: turning away mid-fight
reads as a glitch. A body spends at least 5 s in RECOVER before it can be released again, so it
can't flicker.

**What engaged bodies still get from us:**
- gait in the chase (bursts, the crawler's cap);
- the kind's own attack;
- stuck watch;
- moving away from fire;
- hearing by kind.

**A whole-group view.** Modes are per body, but strategies work per group. A group is "engaged" while
any member is. Hand-offs between groups (section 8) are about groups.

**If blind steering fails (B1).** It didn't: 0 of 24 blind bodies engaged at 150, 50, 15 and 6 m, and
the factor could be set in the spawn frame every time. The fallback stays on record: if factor 0 ever
notices at close range, or the spawn frame is too late, STEER uses a factor of 0.25 instead of 0 (measured: 0% engaged by day,
standing, in the open), and everything inside 25 m becomes Bacon's. Formations and flanks still work
out to 25 m. The worst case is the approved design: placement and spawn order, with steering only
beyond 250 m.

---

## 4. The kinds

Speeds are targets in m/s. **B7:** a speed cap is a fraction of the maximum (sprint) speed, about
4.4 m/s for every prefab: 0.2 gives 0.85 m/s, 0.4 gives 1.8, 0.6 gives 3.0, and above about 0.7 a run
isn't slowed. So the crawler's 0.8-1.2 is a cap of 0.2-0.27 and its 1.6-2.2 a cap of 0.37-0.5. Gait
**walk-stop** is Roam's no-waypoint branch: walk (1.45 m/s) up to 25 m, stand 3–8 s, walk on: **0.45-0.8 m/s
on average** (B7), less than first assumed. **March** is a held waypoint at RUN with a speed cap.
Sprints and charges hold only while our code sets SPRINT every tick (B7: 4.4 m/s, the heavy 6.1).

| | **Crawler** | **Young** | **Hunter** | **Scout** | **Guard** | **Alpha** |
|---|---|---|---|---|---|---|
| Prefabs (approved design) | Crawlers list, else Common | Young list | Hunter list | Scout list | Guard list | Alpha list (Bacon's heavy) |
| Out of a fight | Walk-stop, capped at 0.8–1.2 | Walk-stop, wandering slots | Walk-stop when stalking; march at 2.0 | Still: no slot changes, loiter radius 0 | March at 2.0, in the ring | Heavy walk-stop, capped at 1.4; march at 2.0 |
| In the chase | Capped at 1.6–2.2 | RUN 3.7, bursts to sprint (about 4.4, B7) for 2–4 s, every 8–15 s | RUN 3.7, a lunge sprint from 6–10 m (section 10) | RUN 3.7 | RUN 3.7 | Heavy RUN 4.9, a charge sprint (about 6, B7) for up to 4 s |
| Stance (dynamic) | 0.60–0.70, hunched | 0.85–0.95 | 0.72–0.82 stalking, Bacon's 0.82–0.92 in a fight | 0.9, standing | Bacon's | Bacon's (the heavy is big already) |
| Sight, day / night (section 5) | 60 / 40 m | 180 / 150 m | 220 / 190 m | The scout rule (approved): 120 m × stealth, 5 checks | 200 / 170 m | 160 / 140 m |
| Hearing (× the base table) | ×1.5 | ×1.2 | ×1.0 | ×1.0 | ×1.0 | ×0.8 |
| Blood scent | 30 m | 80 m | 80 m | — | 60 m | 80 m |
| Memory after losing you | 20 s | 30 s | 90 s | — (it calls) | 60 s | 120 s |
| Engage radius E (above 15 m only with RUN set at the release: B6) | 12 m | 35 m | 25 m (after a stalk) or the strike order | 15 m (it calls; fights only if you come) | 30 m | 40 m |
| Climbs (L11; grab height, B8) | Never: no climb at all | Up to 1.4 m | Up to 1.4 m | Up to 1.4 m | Up to 1.2 m | Never climbs: goes round (its contact doesn't break fences: B17) |
| Own attack (section 10) | Ankle grab | Leap | Lunge and bite | — (the shriek) | — (Bacon's melee) | Charge and knockdown |
| Voice | Groans often (Bacon's chances) | Loud, shrieks when it notices you | Quiet while stalking; one tell before the strike | The shriek | Normal | The call; a roar before a charge |
| Quirks | Herds; lies up by day (section 11) | Erratic: slot jumps 6–12 m sideways every 4–8 s on approach. 15% chance per 30 s to stop and stare at a house | Freezes when you look its way (section 8) | Faces the trail | Stays within 25 m of the alpha | Its contact trips other infected (Bacon's code), so its slot is kept 8 m clear |

**Individual variation.** Each body gets, from its seed:
- a speed factor of 0.88–1.0 on its kind's caps (caps only go down, so variation goes down too);
- a stance offset of ±0.04;
- a reaction delay of 0.2–1.2 s before it acts on a release or an order;
- a slot jitter of 1–3 m.

So a pack strings out on its own and no two move alike.

**Day parties** are hunters and young. They use their kind's profile and the strategies allowed by day
(section 8). Phase 0's hop and hold stays as the data rhythm: a 100–200 m leg every 4–6 minutes, now
walked at walk-stop (about 2–3 minutes), then a hold. The average stays near the simulator's
assumption, so its day-crossing numbers still hold. Don't let day parties march all the time: phase 0
measured that groups always moving raise day fights to 1.7 a crossing.

---

## 5. Senses

While our code drives, our senses decide. They work per group, not per body: one leader's eye, once a
second, only for groups with a player within 250 m. Engaged bodies use Bacon's perception (with
stealth's factor), so the close fight still feels like Bacon.

### 5.1 Sight: the notice meter

Each group keeps a meter from 0 to 1 for each player within its sight radius R (section 4). Every
second:

> meter += 0.15 × vis × facing × (1 − d / R)^1.5 × LOS, and it falls 0.1 a second when there's nothing to add

| Input | Value |
|---|---|
| vis | Stealth's visibility of the player, 0–1 (stance, speed, light, weather, noise). Already built |
| facing | 1 within ±70° of the leader's heading, 0.25 behind it |
| LOS | 1 or 0, from section 5.5. Never through terrain or buildings |
| d | Distance from the leader's eye to the player |

| Meter | State | What players see |
|---|---|---|
| Under 0.4 | Unaware | Nothing changes |
| 0.4 to 1 | **Suspicious** | It stops and turns toward you. Hunters freeze and crouch. Crawlers drift your way. One sound (a tell, section 12) |
| 1 | **Aware** | The group knows your position. The strategy starts (section 8). Young and scouts shriek |

**Close in.** Within 12 m with line of sight, the meter fills in 2 s whatever the rest says. Within
8 m the contact rule releases the body anyway (section 3).

**Calibration.** For a hunter by day: a walker (vis 0.85) at 100 m in front notices in about 20 s.
At 150 m it's about 45 s, and at 25 m about 9 s. Phase 0 measured Bacon at about 25% per 30 s at
50–200 m. Before phase 2 closes, tune the 0.15 so a hunter matches that ladder, then re-run
`tools/tribe_sim.py` with the new sight. Crawlers are much weaker by design.

### 5.2 Hearing

A sound gives a group a **lead**: a position with an error of 15% of the distance. A lead starts an
investigation (walk-stop toward it), never a fix on you.

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
- **Hand-off.** A group that loses you passes its last-known spot and heading to the nearest group of
  its tribe that is closer to that spot, which then makes its own approach.

### 5.5 Line of sight without seeing through walls

1. **Terrain.** Sample `GetSurfaceY` every 10 m along the line, and test it against the line's height.
   This is pure script with no physics. Up to 25 samples.
2. **Buildings.** Within 80 m only, one physics trace from eye to head against buildings and walls.
   Trees don't block it.
3. **Budget.** At most 12 sight checks a second for the layer, nearest players first. The scout rule
   keeps its own 12 (approved). Each check is cached 2 s.

VERIFY 12 in the approved design (the trace's cost) covers this. **B13:** a terrain check costs
0.002 ms and an 80 m physics trace 0.021 ms, so 12 a second cost nothing.

**Integration with stealth (built).** Stealth stays the one judge of how visible a player is. It stops
setting perception factors on STEER and RECOVER bodies, and its "losing them" search doesn't run on
them (our memory covers it). It sets factors on ENGAGE bodies as today. Its 40-agents-an-update cap
now counts only engaged bodies.

---

## 6. Formations and flanking

### 6.1 Frames and slots

A group has an **anchor** A (where it's going next), a **heading** h (a unit vector toward its goal),
and its side vector p = h rotated 90°. Each member i has a slot offset (x_i to the side, z_i forward)
and stands at:

> S_i = A + x_i · p + z_i · h + jitter_i, snapped to the navmesh (L18), never in water

Our tick writes S_i into the character's slot field every 1–2 s (L2). Roam then walks or runs the body
there. The anchor moves by the group's held waypoint (march) or by our tick (walk-stop).

**Minimum spacing is 3 m, and 8 m round an alpha.** Bacon's infected trip each other on contact, and
the heavy trips anyone it touches at speed. Spacing also stops the "single file" pile-up the author
dislikes. **B20:** slots alone don't keep it (members of a pair came within 0.8 m), so the slot code
enforces the minimum between members. B3 and B20 measured the slots themselves: bodies sit within
0.1-0.8 m of them, the fan was 63 m wide at 52 m out, and the pincer's pairs arrived 10 s apart.

### 6.2 The formations

n is the number of members, i = 0 … n−1, and c = (n − 1) / 2.

| Formation | When | Slots (x_i, z_i) |
|---|---|---|
| **File** | On a trail at night (the lore's "in file"), crossings, bridges | (±0.5 jitter, −i · 5) |
| **Loose line** | Crossing open ground, sweeping a field | ((i − c) · w, ±2 jitter) |
| **Fan** | The approach once aware | On an arc round the target (below) |
| **Wedge** | A hunter group led by its strongest, or the alpha with its guard | (±⌈i/2⌉ · w, −⌈i/2⌉ · 4); the alpha at the point with 8 m clear |
| **Flanking pairs** | The pincer | Two pairs, each in a short file (0, 0) and (2, −4), at the flank points (6.4) |
| **Crawler herd** | Town drift | A blob: each slot a random point 6–25 m from the anchor, fixed per body, loiter radius 6 m |
| **Ring** | Lair by day, kill sites, the vigil | The approved feeding order and ring distances, as angles round the centre |

### 6.3 Spreading wider on the approach

The side spacing w grows as the group gets nearer the target, d being the distance from the group
centre to the target:

> w(d) = clamp(4 + 0.1 · (200 − d), 4, 20) m

| d | 200 m | 150 m | 100 m | 50 m | 0–40 m |
|---|---|---|---|---|---|
| w | 4 m | 9 m | 14 m | 19 m | 20 m |
| Front of a group of 4 | 12 m | 27 m | 42 m | 57 m | 60 m |

**The fan.** Once w × (n − 1) is more than 30 m, the line bends into an arc of radius d round the
target T, so everyone keeps the same distance:

> θ_i = (i − c) · w / d, and S_i = T − d · rotate(h, θ_i)

### 6.4 The flank and the pincer

From the target T and the group centre G:

- u = (T − G) / |T − G|, the line of approach;
- p, the side vector;
- s = −1 or +1, the side.

The flank point for side s, at radius R_f and angle α off the line of approach:

> F_s = T − R_f · (cos α · u) + R_f · (sin α · s · p)

**Defaults:** R_f = 130–150 m in the open by day, 90–110 m at night or in bush; α = 70°.

**Two legs.** Going straight from G to F_s would cross the target's front. So:
1. **Leg 1 (the sidestep):** W1 = G + s · p · R_f · sin α, held back.
2. **Leg 2:** to F_s.

**Cover.** For each leg end, try 5 points within ±15° of the arc. Prefer one whose terrain line to the
target is blocked (5.5, terrain only). Then prefer a land-grid forest or sparse cell. That is 5 cheap
checks per re-plan.

**Re-plan** when the target moves 25 m from where the plan assumed, at most once every 10 s.

**Timing.** Each element's ETA is the length of its route divided by its gait speed. The slower element
starts first. Both are released together:
- when both are within E of the target, or 45 m;
- or after 60 s from the first one's arrival, whichever comes first. A pincer never waits forever.

**Worked example.** A target in a field, a group 200 m away, R_f 140 m, α 70°.
- Each flank point is 140 m from the target, 48 m back toward the group's side and 132 m out to the
  side. The two are 263 m apart.
- Leg 1 is a 132 m sidestep, about 1–2 minutes at walk-stop.
- The whole move takes about 3–4 minutes. That suits a stalk, not a sprint.

### 6.5 Stalking at 80–150 m

A shadow point keeps the group behind and to the side of a moving target, v being the target's
velocity:

> bearing b = the direction from T to the group, turned toward −v by up to 60°, and toward the nearest cover cell
>
> S = T + R_s · b, with R_s = 80–150 m by day and 50–100 m at night

- **Gait:** walk-stop, the anchor 15–20 m ahead (Roam walks under 25 m: L3).
- **When you turn toward them:** if your facing is within 30° of the group and you're within 120 m,
  they freeze. That means a slot unchanged, a crouched stance (L6) and silence for 3–6 s, or a drop
  back to the last cover point. *That's the moment a careful player catches them.*
- **Re-target:** every 5 s, with an order budget of one held-waypoint change per 8 s if marching.

---

## 7. Navigation and getting unstuck

### 7.1 Where they go

| Scale | Who decides | How |
|---|---|---|
| Across the range (250 m squares) | The tribe data (approved) | Trails of 4–6 land-grid points. They stay on land, cross water only at crossings, and avoid fear places and burned squares |
| Along a leg (10–250 m) | Our code | Leg ends at cover: forest-edge and bush cells. Low ground preferred (the lowest of a square's 5 samples). Navmesh-snapped and dry |
| Step by step | The engine | Bacon's `AITaskMove` paths on the navmesh between our points |

**Along hedges, forest edges and ditches: what's really possible.**
- **Forest edges: feasible.** The land survey already reads tree entities per square. Add one **edge
  point** per forest or sparse square that borders an open one (the mean of trees within 30 m of the
  open side). That's a few KB in the cache. Legs end at edge points.
- **Hedges: partly.** Bushes are `BaseTree` too (phase 0). We can't trace a hedge line cheaply, so
  "along hedges" means through bush squares, with leg ends at bush clusters. It doesn't mean hugging
  one hedge.
- **Ditches: no** at the grid's scale. The fallback is a preference for low ground.

**Water.** Every point is checked with `RZ_Util.IsWater` and `TryGetWaterSurface` (phase 0's
method), and so is the straight line to it (1 m steps). Rivers are crossed only at the land grid's
bridges and fords, in file. **B16:** this is needed: the engine paths straight through a shallow river
(raw slots across were waded at 0.4-0.7 m), while checked slots and slots 1.5 m from the water stayed dry.

**Stairs and upper floors.** **B11:** they don't climb stairs reliably (in 2 runs, a target 3.8 m
upstairs wasn't reached in 70 s; one body got 2.7 m up). So this is the rule: if a target is more than
2.5 m above a body, within 15 m across, and not reached for 20 s, the body waits at the stair foot or the
doorway (*"they wait below"*). After the leash it gives up. A roof or an upper floor is a fair escape.

### 7.2 Stuck detection

Checked every 5 s for each STEER or ENGAGE body with a player within 300 m (the cheap ones first):

| Case | Stuck when |
|---|---|
| STEER | It has moved under 1.5 m in 10 s while its slot is more than 8 m away |
| ENGAGE | It has closed under 1 m on its target in 8 s, the target is within 40 m, and it's not in melee range |
| Climb refused (L11) | It counts as a stuck tick at once (B8: a refused body stops dead against the wall and never goes round on its own) |

### 7.3 The unstick ladder

Each step gets one try, then the next.

| Step | What | Time |
|---|---|---|
| 1. Sidestep | Slot moved 4 m sideways (left first, then right), as Bacon's `GetSideStepPosition` does | 6 s |
| 2. Back off | Slot 10–20 m back along its route, navmesh-snapped | 10 s |
| 3. Another way | A new leg 45° off the old line, on the side with fewer stuck marks | 20 s |
| 4. Out of sight | Only if no player is within 60 m and none has terrain line of sight within 150 m: `SetOrigin` to the nearest navmesh point up to 15 m ahead on its route | Instant |
| 5. Give up | STEER: drop the slot, rejoin the group's anchor. Out of sight beyond 250 m: back to data with a silent delete (approved). ENGAGE: take back (RECOVER) | — |

- **Stuck marks.** A 10 m square where a body got stuck twice is marked for 30 minutes, and new legs
  avoid it. That's data only, at most 64 marks.
- **Never in view.** Step 4 never happens where a player could see it.
- **Lab (B9).** Against tall walls the navmesh didn't know, no body ever met the stuck rule: they slid
  along the walls and round their ends in 17-21 s, rebuilt navmesh or not. The ladder wasn't exercised;
  its numbers stay untuned. The real stuck case seen in the lab is a refused climb (B8).

### 7.4 Fences: vault low, not tall

Bacon climbs whatever the engine's climb test allows, and only when it has stopped dead against
something. Our climb gate (L11) adds a height limit:

> allowed when the grab point's height above the body's feet is at or under the kind's limit (section 4)

**B8:** the gate works (a 1.2 m limit refused a 1.39 m fence every time), but the grab point isn't
always the top: a 1.97 m rail fence was climbed through its 0.85 m rail at every limit. Heights below
are grab heights.

| Obstacle | Young, hunter, scout | Guard | Crawler | Alpha |
|---|---|---|---|---|
| Low wall, fence or hedge up to 1.2 m | Vault | Vault | Goes round | Goes round |
| Fence 1.2–1.4 m | Vault | Goes round | Goes round | The same |
| 1.4 m and up (garden walls, chain-link, the perimeter) | Goes round | Goes round | Goes round | The same |

A refused climb is a stuck tick, so the ladder routes them round. **B17:** `m_bDestroysThings` is on
for `Zombie_Heavy_Tank` and its contacts happen (142), but they didn't damage wooden fences (5000 of
5000 HP); the heavy climbed over instead. So the alpha goes round like the rest. Breaking fences would
need our own damage call (untested).

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

### 8.1 The state machine (per hunt, per target)

```
 TRAIL / DAY WALK ──notice (meter 1, a shriek, a lead with heat 30+ at night)──▶ CHOOSE
 CHOOSE ──picks a strategy (8.3)──▶ APPROACH
 APPROACH ──in position, or the strategy's timeout──▶ SET
 SET ──a moment (8.2), or timeout──▶ STRIKE (release: section 3)
 STRIKE ──engaged; members fight as Bacon──▶ PRESS
 PRESS ──group down to half, or target lost for memory time──▶ REGROUP or HAND-OFF
 REGROUP ──60–100 m back in cover, 20–40 s──▶ CHOOSE (once per target per hunt) or GIVE UP
 HAND-OFF ──another group closer to last-known takes it──▶ (the other group's CHOOSE)
 GIVE UP ──search done; long call if a hunt──▶ HOME
```

| State | What the bodies do | Timeout |
|---|---|---|
| CHOOSE | Freeze 2–5 s (a tell: the sound of them stopping) | — |
| APPROACH | The strategy's formation and legs, in STEER | Per strategy (8.3) |
| SET | Hold in cover at the set distance, walk-stop toward the strike side | 20–90 s |
| STRIKE | Release with injection. The leader roars (a tell) | — |
| PRESS | Bacon fights. Our overlay: bursts, attacks, stuck watch | — |
| REGROUP | Take back. Slots in cover 60–100 m from the target | 20–40 s |
| GIVE UP | Take back. Search the last-known spot (45 s), then home | — |

**A real give-up.** Once a group gives up on a player, it can't target them again for 10 minutes
unless:
- it notices them afresh within 50 m;
- or they fire within 60 m of it.

That's the approved hide rule, with teeth: going home means going home.

### 8.2 "A moment": when they strike

The SET state ends when any of these happens:
- the target stops moving for 10 s or more (looting, aiming, bandaging);
- the target goes prone or crouches in bush, or enters a wood or a field edge;
- the target's facing turns away from the strike side (more than 100° away);
- the target starts shooting at something else (busy);
- all elements are in position (a pincer, a ring);
- night falls, or fog comes in (the stalk's own clock).

### 8.3 The strategies

| Strategy | Needs | Approach | Strike | Timeout |
|---|---|---|---|---|
| **Rush** | Anything. Young groups by default | Fan at RUN | At once | — |
| **Stalk** | 1 group | Shadow at 80–150 m by day, 50–100 m at night (6.5) | At a moment, from cover, from behind | 4 min day, 6 min night, then Rush or give up |
| **Pincer** | 2 groups, or 1 group of 4+ split into 2 | Two flank points (6.4) | Both together | 60 s after the first arrives |
| **Encircle** | 5+ bodies, open or mixed ground | A ring of radius 120 m with a **60° gap**, closing 0.7 m/s at walk-stop | At 35 m from every slot, or a moment | 3 min |
| **Flush** | Target hidden in bush, a wood or a building | 1–2 bodies push in noisily from one side; the rest wait 40–60 m off on the far side | When the target moves or runs out | 90 s |
| **Bait** | A young plus a hunter group. Cunning only | The young goes noisily at the front (released early). The hunters come blind up the target's back trail | When the target engages the bait, or turns to it | 90 s |
| **Herd** | A crawler town within 500 m | An encircle whose gap faces the town. The young press from the far side | At the town's edge; the town is stirred (+20) | 4 min |
| **Ambush** | A crossing (bridge, ford, road gap, gate) or a habit place within 300 m ahead of the target's heading | One group holds still 20–40 m off it, behind terrain or trees | When the target passes within 30 m | 5 min, then stalk |
| **Hand-off** | Another group of the tribe closer to the last-known spot | Passes last-known and heading | (theirs) | — |

**Fairness inside the strategies:**
- **The ring always has a gap.** A ring with no way out reads as a trap you couldn't have seen. The
  gap is where the herd and the ambush come from, so reading the gap is the counterplay.
- **The bait is always loud.** It shrieks and runs straight in. Shooting it is fine. Turning round
  is the counter.

### 8.4 Which strategy a tribe picks

Weights by temperament (approved temperaments). A strategy whose "needs" aren't met has weight 0. The
roll takes the top two and picks between them 70/30.

| Strategy | Bold | Cautious | Cunning | Territorial | Nomadic |
|---|---|---|---|---|---|
| Rush | 3 | 0 | 0 | 1 | 1 |
| Stalk | 1 | 3 | 2 | 1 | 2 |
| Pincer | 2 | 1 | 2 | 3 | 1 |
| Encircle | 3 | 0.5 | 1 | 2 | 1 |
| Flush | 2 | 1 | 1 | 2 | 1 |
| Bait | 0 | 0 | 3 | 0 | 1 |
| Herd | 1 | 1 | 2 | 0 | 2 |
| Ambush | 0 | 2 | 3 | 2 | 0 |

| Situation | Allowed |
|---|---|
| **By day** (day parties, a scout's pack of at most 4) | Rush, Stalk and Pincer (2 groups) only. Never the alpha (approved) |
| **Rising alphas** | Cautious weights (approved: every tribe hunts cautiously while its alpha is Rising) |
| **Crawlers** | No strategies: herd drift to noise, then Rush at E |
| **Habits** (approved) | A habit place within 300 m adds 2 to Ambush |

**Caps:**
- one strategy per tribe per target;
- at most 3 elements;
- at most 2 strategies running on the server near one player cluster;
- everything inside the approved commitment cap and first-wave sizes.

The placement rules for hunts (the young ahead, flankers out of sight, the core held back) stay.
Strategies start from where bodies appear.

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

The approved crawler placement (rests, streets and fields by time of day; drift to noise; stir;
scavenging) gets bodies that behave it.

| When | What they do | Levers |
|---|---|---|
| **Day** | Lying up in doorways and back rooms: forced prone or crouched at their building rest, AI asleep (L14) beyond 40 m of a player. They wake (stand, AI on) on a lead within their hearing, or a player within 40 m. **B12:** AI off and on works (awake in 0.2 s, moving in 6.5 s), but a prone stance change didn't hold; the pose needs `ForceStance` (untested) | L6, L14 |
| **Waking** | Up over 2–4 s (forced stance up), a groan, then walk-stop toward the lead. Waking takes time, and that's the player's warning | L6, L15 |
| **Dusk** | Standing in yards and doorways, a herd blob round the rest with a 6 m loiter radius | L2 |
| **Night** | Herds drift the streets: an anchor walks a slow loop between the town's squares (0.5 m/s average), each crawler in its blob slot. Drift to remembered noises (approved), at walk-stop | L1–L4 |
| **Stirred** | Herds out into the streets, anchors moving toward the last noise | Approved stir |
| **After a hunt** | 2–4 walk out to the kill site and stand round it until dawn (approved scavenging) | L2 |
| **A tribe passing** | Herds give way: anchors move to the side away from the trail (approved) | L1 |
| **In a fight** | ENGAGE within 12 m. Capped at 1.6–2.2 m/s. Ankle grab. Bacon's swing | Sections 3, 10 |

**What players learn.** Crawlers are slow, they're many, they hear well and see badly. A town by day
is a street of figures lying in doorways. Walk past quietly and they stay down. Shoot, and the whole
street gets up.

**Honest note.** The speed cap makes crawlers slow whatever the prefab is (B7: a cap of 0.2 gives
0.85 m/s). But the animation's movement state stays "run" under a cap, so they probably run in slow
motion. Until a client confirms how it looks, the approved design's slow prefab stays in the stack.

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

A new block, **Zombies > Behaviour** (`RZ_BehaviourSettings` on `RZ_GameModeComponent`). As
CLAUDE.md says, no `[Attribute]` field gets an initialiser: the attribute's default string below is
the only default.

| Setting | Default | What it does |
|---|---|---|
| Enabled | 1 | Off: every body is OFF mode, exactly today's Bacon behaviour plus stealth |
| Blind steering | 1 | Off: STEER uses the fallback factor and only drives beyond 25 m (section 3) |
| Steer factor | 0 | The perception factor while steered (the fallback uses 0.25) |
| Contact radius | 8 | m, with line of sight (3 m without is fixed) |
| Take-back multiple | 2.5 | × E before an engaged body is taken back |
| Order gap | 5 | s between held-waypoint changes for one group |
| Orders per 5 s | 16 | Server-wide held-waypoint changes (the approved 8 plus the layer's 8) |
| Slot update | 1 | s between slot writes for groups with a player within 150 m (3 s beyond) |
| Sight checks a second | 12 | The layer's line-of-sight checks (the scout rule keeps its own) |
| Notice rate | 0.15 | The 0.15 in the meter (5.1); tuned in phase 2 |
| Hunter sight day / night | 220 / 190 | m. Other kinds use the section 4 table, which scales with these |
| Shot hearing | 170 | m, the base for a shot |
| Spacing min | 3 | m between slots (8 round an alpha) |
| Spread start / max | 200 / 20 | m: w(d) in 6.3 |
| Flank radius day / night | 140 / 100 | m |
| Flank angle | 70 | degrees |
| Stalk distance day / night | 80-150 / 50-100 | m |
| Strategies | 1 | Off: every group Rushes at E |
| Own attacks | 1 | Off: plain Bacon melee |
| Special move gap | 10 | s between special moves landing on one player |
| Weak door words | Shed,Barn,Garage,Wood | Door prefab words an alpha or a bold night hunt may break |
| Climb limit | 1.4 | m for young, hunters and scouts (guard 0.2 less; crawlers and alphas never climb) |
| Unstick out of sight | 1 | Allows step 4 of the ladder |
| LOD bands | 60,150,300 | m: LOD 0 / 2 / 4 / 6 beyond (L13) |
| Sleep lying crawlers | 1 | AI off for crawlers lying up beyond 40 m of a player |
| Debug log | 0 | One line a minute: bodies per mode, strategies running, releases, take-backs, unsticks, moves landed |

---

## 14. Performance budget

Bodies still exist only near players, and the approved budgets (100 infected agents, the engine limit
at 160, spawns per tick, props) stay. The layer adds:

| Work | When | Cap | Estimate |
|---|---|---|---|
| Behaviour tick (modes, slots, strategies) | 1 s, on the game mode's fast loop | 30 groups near players; groups with no player within 300 m skip it | Vector maths only. Target under 0.3 ms a second |
| Senses (meters) | 1 s per group with a player within 250 m | 30 groups × players in range | A few multiplies each |
| Line-of-sight checks | Cached 2 s | 12 a second (terrain samples), building traces only within 80 m | B13: 0.002 ms a terrain check, 0.021 ms a trace |
| Hooks in Bacon's nodes | Every behaviour-tree loop of every agent | 100 agents × about 7.4 loops a second (B12) | One field read each. B15: no cost above the noise |
| Held-waypoint changes | Marches, home, search | 16 per 5 s; one per group per 5 s | No RplComponent (phase 0) |
| Stuck checks | 5 s | Bodies with a player within 300 m | Two distance tests each |
| Navmesh snaps | — | — | Not available on Bacon bodies (B9); slots go unsnapped (B3 shows they work) |
| Climb gate | Only while a body stands still against something | — | One extra climb test |
| Data | — | Stuck marks 64; scent trails 6 points per bleeding player; edge points in the land cache | A few KB |

**Savings the layer brings:**
- **Think rate.** Bodies 60–150 m from every player get LOD 2 (a 0.5 s loop), 150–300 m LOD 4
  (0.9 s), and beyond that LOD 6 (1.3 s), through `SetPermanentLOD`. Today all stay at LOD 0 (phase 0).
  B12: all 100 at LOD 6 cut p99 from 11.9 to 9.6 ms; order latency is dominated by Roam's 3–8 s idles
  at every LOD.
- **Sleep.** Crawlers lying up and the lair's resting ring beyond 40 m have their AI off.
- **Fewer chases.** Blind bodies don't chase on their own, so there's less pathing at 100–250 m.

**The target.** With 100 agents and the layer on, the lab box's p99 frame time stays at or under
12.5 ms (phase 0 measured 11.6 ms without it), and the mean at or under 4.6 ms. **B15: met.** 100
agents with half in steered walking formations: p99 11.93 ms, mean 4.41 ms (the same 100 on held
waypoints in the same run: 13.17 and 4.56). 140: p99 13.03, mean 4.75. Writing all slots takes
0.04-0.2 ms a second.

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
| **1c. Crawler life** | Phase 1, with crawlers | Lying up, waking, herd blobs, night drift loops, scavenging bodies, giving way, the crawler's speed cap | 350 |
| **2a. Senses and formations** | Phase 2 slice | `RZ_BehaviourSenses.c` (meters, hearing table, blood scent, memory, shrieks, line-of-sight cache). `RZ_Formations.c` (slots, spread, fan, flank points with cover, stalk shadow) | 750 |
| **2b. The first strategies** | Phase 2 slice | `RZ_HuntPlan.c`: the state machine, Rush, Stalk, Pincer, Give up. The hunter lunge and the alpha charge (server effects only: bleed, the miss stumble). Play-test the slice (the approved list, plus a stalk and a pincer) | 700 |
| **2c. Everything else** | Phase 2, all tribes | Encircle, Flush, Bait, Herd, Ambush, Hand-off; temperament weights; reactions table. `RZ_PlayerEffectsComponent` and the hold, stagger and knockdown after the client test. Re-run `tools/tribe_sim.py` with our sight model | 900 |

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
