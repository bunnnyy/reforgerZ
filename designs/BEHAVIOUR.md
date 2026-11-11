# The infected behaviour layer: what's left

**Status: designed, not built.** The layer itself is built ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)
owns it): Bacon's hooks, the four modes, the kinds, sight with its cache, hearing, blood and shrieks,
memory, hand-offs, formations and flanking, every hunt strategy, the reactions and the regroup, doors,
navigation and the climb gate, LOD and the resting ring asleep, the kinds' moves and their effects on
the player's machine, and the crawlers' daily life. The zombie lab's B1–B20 results are in
[docs/WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#zombie-lab-bacons-real-ai-measured). This file
keeps only what's still to build. Approved by the author on 2026-11-11.

## What's left

| Part | Still designed | Why it waits |
|---|---|---|
| Sight | Calibrate **Notice rate** to phase 0's ladder in a play-test and re-run `tools/tribe_sim.py` with it | Needs a play-test |
| Formations | A re-plan only when the target moves 25 m (the code re-plans each second); an ETA so the slower element starts first | Every strategy reads the target's last-known spot directly; worth it only if play shows jitter |
| Navigation | **Forest edge points** in the land cache (the code uses the square's tree class and its open neighbours); low ground preferred on a leg | A change to the land grid's cache (`World/RZ_Land.c`) |
| Kinds | The young's stares at houses | A tell with no lever yet (facing a body isn't set by script) |
| Crawlers | A forced prone pose (lever L6, `ForceStance`, untested: a prone `SetStanceChange` didn't hold, B12); crawlers steered into a fight within 12 m | They stay Bacon's own; needs a lab test first |
| Pathing | Lever L23: a straight-path cost per kind (×1 young, ×3 hunters) in `SCR_CreateAIExtraMoveParams`, so a chase curves | The hook loads and fires (B2); its effect isn't tested |
| Danger events | Lever L16: a scripted `AIDangerEventWeaponFire` to pull engaged bodies the way a shot does | Not needed so far |

**Not to build** (the lab showed they don't work; their fallbacks stay): heavy fence breaking (the alpha
goes round, B17), following up stairs (they wait below, B11), `SetMeleeAttack` and a scripted jump
(B14), and the navmesh snap on Bacon's bodies (L18, B9).

## Risks

1. **Hearing everything makes them omniscient.** Every sound gives a lead with error, never a fix;
   tune the radii in play (the play-test's hearing line).
2. **Weak doors take away the last safe room.** Protected buildings and the word list keep it rare;
   play-test **Weak door words** on each map.
