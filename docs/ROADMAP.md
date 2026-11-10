# Roadmap

What's built, what's next, what to test first, and what was dropped. How each system works
is in [DESIGN.md](DESIGN.md).

**Status:** everything under "Built" is written in Enforce Script (about 48,500 lines) and
**has never been compiled**. It needs Workbench, the 105 `VERIFY` fixes and a play-test
before any of it can be called working ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).

## Built

| System | Design |
|---|---|
| Settlement detection, rotating spawn points, mission announcements, map markers | [Settlements](DESIGN.md#settlements-spawn-points-and-the-map) |
| The Director: four outbreak stages, infection spread | [The Director](DESIGN.md#the-director-outbreak-stages-and-spread) |
| Heat from presence, vehicles, gunfire (suppressors quieter), noisemakers; hordes toward noise | [Heat and noise](DESIGN.md#heat-and-noise) |
| Zombie spawner for any zombie mod, bite detection, infection timer, the Turned Player | [Zombies](DESIGN.md#zombies-bites-and-infection) |
| Light adaptation: specials after breakthroughs, one trait per stage, favourite spots | [Infection adapts](DESIGN.md#infection-adapts) |
| The Perimeter: warning and kill zones, one drone per player, sealed road checkpoints | [The Perimeter](DESIGN.md#the-perimeter) |
| Night, rain, fog and storms; the power grid; the radio station; the government broadcast | [Time and environment](DESIGN.md#time-and-environment) |
| 13 backgrounds (5 unlocked through Legacy), personal goals, Legacy points | [Backgrounds](DESIGN.md#backgrounds-and-personal-goals) |
| Fame, Infamy and Favor; tiers; rivals; amends; quartermaster shops; Requisition; cheap perks; hunter squads; bounties | [REPUTATION.md](REPUTATION.md) |
| Per-player AI hostility (hidden faction variants), retaliation, Outlaw fallback | [REPUTATION.md](REPUTATION.md#what-makes-ai-shoot-you) |
| Territory: strongholds, outposts, the territory tick, faction strength | [Territory](DESIGN.md#territory) |
| Real supply trucks with AI crews, shortages, escort, drive-in and theft | [Supply trucks](DESIGN.md#supply-trucks) |
| Faction life: needs, morale, chatter, life events, truces and feuds, moments, rumours | [Faction life](DESIGN.md#faction-life), [REPUTATION.md](REPUTATION.md#morale-needs-truces-and-feuds) |
| Haven safehouses: population tiers, upkeep, rescues, house rules, exile | [Haven safehouses](DESIGN.md#haven-safehouses) |
| Named wanderers: travel, reactions, memory, deaths in the news | [Wanderers](DESIGN.md#wanderers) |
| Named leaders: traits, loyalty, memory, voiced briefs, deaths and replacements | [Leaders](DESIGN.md#leaders), [REPUTATION.md](REPUTATION.md#faction-leaders) |
| 120 missions: 42 contracts, 10 world events, 17 puzzles, 10 unarmed, 3 Perimeter, 3 Haven house contracts and the Courier's letters, 30 leader jobs and Rein In, 3 arc missions | [MISSIONS.md](MISSIONS.md) |
| Six story arcs and session headlines | [Story arcs](DESIGN.md#story-arcs-and-headlines) |
| Town memory and journals of the dead | [Town memory](DESIGN.md#town-memory) |
| World interactions: searching, locked doors (the vanilla door action hooked) and keys as real items, readable things, fixable cars, crafting, traps, generators, fire, water, treating others, bells, secure radio | [World interactions](DESIGN.md#world-interactions) |
| New-player guidance: experience levels, tips, spawn scenarios, wind-up radio, faction voices | [New-player guidance](DESIGN.md#new-player-guidance) |
| Cure research, antidotes, the final stand, the season's winner | [Cure research](DESIGN.md#cure-research-and-the-seasons-end) |
| Save and load of mode state | [Persistence](DESIGN.md#persistence) |

## Next

### Get it running
- Set up the Workbench project, fix every `VERIFY`, and get it compiling.
- Make the required prefabs and the factions, then play a 2-hour session on Arland
  ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).

### Characters and UI
- A custom deploy-menu panel that shows each background's perk and flaw, and hides locked ones.
- A trait-points system on top of backgrounds (positive and negative traits).

### Factions
- Remaining perks: Haven **Community** respawn with a supply bag, Bastion **Fire Mission**,
  Red Hand **Fence** and **Bounty board**, Halcyon **immunity booster** and **medic**.
- Paying Favor to work off Infamy.
- Bastion checkpoints and Red Hand roadblocks on the roads as AI (with the Known perks: pass a
  checkpoint unsearched, pay the toll instead of being shot).
- Disguises (faction uniforms) and surrendering at checkpoints.
- AI camps that warn and refuse by standing, and safe-zone entry.
- Reputation clues other players can see (e.g. a bandit armband).
- Wanderers react when their faction loses an outpost.

### Missions
- The 38 designed faction contracts and 15 world events not yet built ([MISSIONS.md](MISSIONS.md)),
  starting with the convoy and escort ones (Convoy Escort, Refugee Column, Hijack the Convoy,
  Rescue the Pilot).
- **Car Keys** (unarmed): the fixable cars and keys it needs exist.
- Leader jobs **Desertion Route** (needs people who walk with you) and **Private Collection**
  (needs a trap for a live special).

### Player to player
- A notice board at each safehouse for short notes between sessions.
- Contracts that give a rough area, with the exact spot shown within 500 m (Radio Operators
  and Trusted players see it from the start).

### Seasons
- Next-season starting rules from the last winner ([LORE.md](LORE.md#endings)).
- A wipe cycle tied to cure completion, or a fixed number of weeks.
- Season rule sets: Winter (with a cold mod), Fog, Military Collapse, No Vehicles.
- A hall of fame: who delivered the most samples, who held the final stand.

## Play-test priorities

In this order, because later ones depend on earlier ones:

1. **Compile** and fix the `VERIFY` list, riskiest first ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#6-the-verify-list)).
2. **Per-player hostility.** A player Hunted by Bastion near a Bastion AI group: does it fire?
   Do the deploy menu, chat and groups stay on `SURV`? If AI ignores the variants, switch to Outlaw.
3. **The core loop on Arland** for 2 hours: zombie density, spread rate, stage lengths, cure pace.
4. **Supply trucks:** one truck from stronghold to outpost. Is the crew seated, and does it drive?
5. **Noise:** how far a thrown bottle really leads infected; the favourite-spot rate.
6. **Puzzles** one by one, and whether replicated action labels work.
7. **Searching:** do spots appear in houses? Search rates against town loot; how often a car is fixed.
   **Door locks:** does the hooked door action compile, refuse only from outside, and get inside
   right on several building types? Do door spots stand at the right doors? If hooking fails,
   turn **Door locks** off (the lock goes back on the loot).
8. **The Perimeter** on an island map and a land-locked one.
9. **Safehouses:** upkeep timing, food keywords, the medic and bite declarations.
10. **Leaders:** loyalty drift and rogue-job discovery rates.
11. **Guidance:** is anything too chatty for a level-0 player? Do the right speakers say it?
12. **Story arcs** end to end (start one with `RZ_StoryArcs.Begin(kind)`) and how often they come.
13. **Weather** accessors and the generator's noise.

## Dropped

| Idea | Why |
|---|---|
| Supply drops, airdrops, the Supply Plane Crash event | The Zone is sealed: nothing comes in from outside |
| A visible wall, fence and watchtowers at the map edge | The cordon is felt, not seen: drones, loudspeakers, unseen snipers and sealed checkpoints |
| Sterilisation, "Patience" and other outside deadlines | The outside never acts; urgency comes from inside (stages, spread, adaptation) |
| Evac as an escape or a way to win | Nobody leaves the Zone; the only ending is the cure |
| Crates from the evac helicopter | Free loot; the Last Helicopter pays Fame and Favor instead |
| The free hourly Requisition crate | Free loot; Requisition is a half-price purchase |
| A personal hideout per player | Leaves persistent physical traces; the Haven's safehouses are the shelter |
| Factions hostile to all players or none | Replaced by per-player hostility through hidden faction variants |
| Haven guards that never shoot | They shoot armed exiles near a safehouse; unarmed exiles just find the door shut |
| Shop System as the faction traders | Our own quartermasters gate stock by standing; Shop System stays optional for general trading |
| DarcMissions | Its convoys, occupations and crash sites assume forces from outside; they don't fit the sealed Zone |
| Ambient Zombies | Overlaps the ReforgerZ spawner |
| Sleep and time skip | Out of scope by design |
