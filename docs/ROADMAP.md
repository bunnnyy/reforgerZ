# Roadmap

What's built, what's next, what to test first, and what was dropped. How each system works
is in [DESIGN.md](DESIGN.md).

**Status:** everything under "Built" is written in Enforce Script (about 71,000 lines) and the
game scripts **compile cleanly** against Arma Reforger 1.9. It also runs on the server in a test
world with a fake player, with no script exceptions ([runtime test](WORKBENCH_SETUP.md#runtime-test-without-workbench)).
It still needs prefabs, the behavioural `VERIFY` checks and a real play-test before it can be called working
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)). The Workbench plugin hasn't been compiled.

## Built

| System | Design |
|---|---|
| Settlement detection, rotating spawn points, mission announcements, per-player map icons (vanilla text markers without them) | [Settlements](DESIGN.md#settlements-spawn-points-and-the-map) |
| The Director: four outbreak stages, infection spread, paced for a season of about 10 days (season speed for other server sizes) | [The Director](DESIGN.md#the-director-outbreak-stages-and-spread), [Season pacing](DESIGN.md#season-pacing) |
| Heat from presence, vehicles, gunfire (suppressors quieter), noisemakers; hordes toward noise | [Heat and noise](DESIGN.md#heat-and-noise) |
| Zombie spawner for any zombie mod, bite detection (scratches and bites, protective clothing), the golden window and wound cleaning, staged infection, the Turned Player | [Zombies](DESIGN.md#zombies-bites-and-infection) |
| Treatments from day one: antidote and stabiliser stocks scaled by research, Halcyon's trial dose, safehouse bite declarations, five low-grade meds with side effects | [Treatments](DESIGN.md#treatments) |
| Light adaptation: specials after breakthroughs, one trait per stage, favourite spots | [Infection adapts](DESIGN.md#infection-adapts) |
| The Perimeter: warning and kill zones, one drone per player, sealed road checkpoints | [The Perimeter](DESIGN.md#the-perimeter) |
| Night, rain, fog, fog banks and storms; the power grid and power surges; the radio station; the government broadcast | [Time and environment](DESIGN.md#time-and-environment) |
| 13 backgrounds (5 unlocked through Legacy), personal goals, Legacy points | [Backgrounds](DESIGN.md#backgrounds-and-personal-goals) |
| Fame, Infamy and Favor; tiers; rivals; amends (contracts, paying Favor, surrender); quartermaster shops; every perk (Requisition, Fire Mission, the fence, the bounty board, Halcyon's medic and immunity booster, the Haven Champion's Safe Harbour and Remembered...); hunter squads; bounties; reputation clues | [REPUTATION.md](REPUTATION.md) |
| Bastion checkpoints and Red Hand roadblocks inside the Zone (unsearched passage, tolls, searches), surrender, camps that warn and refuse, safe zones | [REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps) |
| Per-player AI hostility (hidden faction variants), retaliation, disguises, Outlaw fallback | [REPUTATION.md](REPUTATION.md#what-makes-ai-shoot-you) |
| Territory: strongholds, outposts, the territory tick, faction strength | [Territory](DESIGN.md#territory) |
| Real supply trucks with AI crews, shortages, escort, drive-in and theft | [Supply trucks](DESIGN.md#supply-trucks) |
| Faction life: needs, morale, chatter, life events, truces and feuds, moments, rumours | [Faction life](DESIGN.md#faction-life), [REPUTATION.md](REPUTATION.md#morale-needs-truces-and-feuds) |
| Haven safehouses: population tiers, upkeep, rescues, house rules, exile, notice boards (notes between sessions, `/note`) | [Haven safehouses](DESIGN.md#haven-safehouses) |
| Named wanderers: travel, reactions, memory, deaths in the news, lost outposts (rerouting, rumours, mood) | [Wanderers](DESIGN.md#wanderers) |
| Named leaders: traits, loyalty, memory, voiced briefs, deaths and replacements | [Leaders](DESIGN.md#leaders), [REPUTATION.md](REPUTATION.md#faction-leaders) |
| 184 missions (every one designed so far): all 80 faction contracts and 5 hard-edge contracts (No Reply, Struck Off, The Three Weeks, The Author List, The List) (on shared building blocks: escorts, live capture, vehicle zones and convoys, health cost, step chains), 3 Red Hand hard edges (Learning the Drones, The Pit's Debtor, Cargo), all 25 world events, 17 puzzles, 11 unarmed, 3 Perimeter, 3 Haven house contracts and the Courier's letters, 32 leader jobs and Rein In, 3 arc missions | [MISSIONS.md](MISSIONS.md) |
| Six story arcs and session headlines | [Story arcs](DESIGN.md#story-arcs-and-headlines) |
| Town memory and journals of the dead | [Town memory](DESIGN.md#town-memory) |
| World interactions: searching, locked doors (the vanilla door action hooked; AI kept out by doorway blockers with local navmesh rebuilds, an optional AI door hook, and infected banging on the door) and keys as real items, readable things, fixable cars, crafting, traps, generators, fire, water, treating others, bells, secure radio | [World interactions](DESIGN.md#world-interactions) |
| New-player guidance: experience levels, tips, spawn scenarios, wind-up radio, faction voices | [New-player guidance](DESIGN.md#new-player-guidance) |
| Cure research, antidotes, the final stand, the season's winner | [Cure research](DESIGN.md#cure-research-and-the-seasons-end) |
| Seasons: next-season rules from the last winner, rule sets (Winter, Fog, Military Collapse, No Vehicles, or your own), the wipe cycle (on cure after an epilogue, or after N weeks; backup and announcements), the hall of fame | [Seasons](DESIGN.md#seasons) |
| Contracts that give a rough area (a 500 m circle) until you're close; Radio Operators and Trusted players get the spot | [MISSIONS.md](MISSIONS.md#who-gets-offered-what) |
| Cleanup of loose items (crates, samples, keys, purchases and finds left lying 30 min) | [Loose items](DESIGN.md#loose-items) |
| Save and load of mode state | [Persistence](DESIGN.md#persistence) |
| The UI: an original icon set (60 icons, SVG and PNG), mockups of every screen, and the scripts for the HUD (infection timer, secure-radio indicator, noise meter, styled notice cards with the vanilla hint as fallback), the standing screen, journal, radio log and map key (keys and mouse), the notice board and hall of fame menus, chat commands (`/note`, `/hall`), a deploy-menu background panel with the loadout's gear and locked loadouts unavailable, and the marker icons on the map, fed per player from the server. Layouts are still to be built in Workbench | [UI.md](UI.md) |
| Setup tooling: the prefab manifest (every prefab, slot, settings object, faction and loadout), the Workbench plugin **ReforgerZ: Create prefabs** with a dry run and a checklist fallback (never run), and a checker that keeps the manifest and the setup doc in step with the scripts | [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#1-create-the-prefabs) |

## Next

### Get it running
- Set up the Workbench project (the game scripts already compile on 1.9).
- Run **ReforgerZ: Create prefabs** as a dry run, fix what fails until it makes prefabs, then
  run it for real (or follow the manual checklist). Make the minimum playable subset and the
  factions, then play a 2-hour session on Arland ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).

### Characters and UI
- Build the UI layouts and the icon imageset in Workbench ([UI/LAYOUTS.md](../UI/LAYOUTS.md)).

### Designed, not built
- **Questlines:** twelve faction questlines and two for the Long Table, over many sessions
  ([QUESTLINES.md](QUESTLINES.md)).
- **The Long Table:** a tiny cannibal family in one farmhouse; no reputation tree, a few
  dealings, exposed and broken up through its questlines ([DESIGN.md](DESIGN.md#the-long-table-planned)).

## Play-test priorities

In this order, because later ones depend on earlier ones:

1. **Compile in Workbench** (the game scripts already compile on the server), then the plugin:
   a dry run, then a real run; note which editor calls needed fixing. Check the `VERIFY`
   behaviours, riskiest first ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#6-the-verify-list)).
2. **Per-player hostility.** A player Hunted by Bastion near a Bastion AI group: does it fire?
   Do the deploy menu, chat and groups stay on `SURV`? If AI ignores the variants, switch to Outlaw.
3. **The core loop on Arland** for 2 hours: zombie density, spread rate, cure pace. Then the
   season's first days: are kills, specials and research per player-hour near the assumptions
   in [Season pacing](DESIGN.md#season-pacing)? Adjust research per sample or the season speed.
4. **Supply trucks:** one truck from stronghold to outpost. Is the crew seated, and does it drive?
5. **Noise:** how far a thrown bottle really leads infected; the favourite-spot rate.
6. **Puzzles** one by one, and whether replicated action labels work.
7. **Searching:** do spots appear in houses? Search rates against town loot; how often a car is fixed.
   **Door locks:** does the hooked door action compile, refuse only from outside, and get inside
   right on several building types? Do door spots stand at the right doors? If hooking fails,
   turn **Door locks** off (the lock goes back on the loot). **Keeping AI out:** does the AI door
   hook compile (else delete it)? Do blockers stand in the doorway, make AI route around a locked
   door after the navmesh rebuild, and cost little server time? Can anyone get trapped?
8. **The Perimeter** on an island map and a land-locked one. On the island: do the three Perimeter missions put their spots on the shore (the `Perimeter shore:` log line), and is the coast under the drones often enough?
9. **Safehouses:** upkeep timing, food keywords, the medic and bite declarations.
   **Bites:** how often hits become bites in real fights (the protective clothing keywords),
   whether the golden window is long enough to clean a bite mid-fight, whether the stamina hook
   works (else the stages only make noise), and whether the meds' side effects are worth it.
10. **Leaders:** loyalty drift and rogue-job discovery rates.
11. **Guidance:** is anything too chatty for a level-0 player? Do the right speakers say it?
12. **Story arcs** end to end (start one with `RZ_StoryArcs.Begin(kind)`) and how often they come.
13. **Weather** accessors and the generator's noise.
14. **Road posts and camps:** do posts go up on real roads ahead of players, and do the search,
    the toll, running and surrender fire at the right distances? Is the timed grudge enough to
    make the crew shoot, and does a surrender's stand-down stop them? Is a Fire Mission's horde
    check too strict or too loose; does the fence take the right things?
15. **Seasons:** a wipe end to end (backup, fresh start, the winner's rules, the hall of fame
    kept), and whether the rule sets feel different enough without being unfair. Are rough
    contract areas fun to search, or too vague at 500 m?
16. **The UI**, once the layouts exist: does the owner-only feed arrive, does the timer agree with
    the stage messages, do the menus open, fill and take clicks, do the map icons sit on their
    spots while panning and zooming, and does the deploy menu grey out locked loadouts ([UI.md](UI.md)).

## Dropped

| Idea | Why |
|---|---|
| Supply drops, airdrops, the Supply Plane Crash event | The Zone is sealed: nothing comes in from outside |
| A visible wall, fence and watchtowers at the map edge | The cordon is felt, not seen: drones, loudspeakers, unseen snipers and sealed checkpoints |
| Sterilisation, "Patience" and other outside deadlines | The outside never acts; urgency comes from inside (stages, spread, adaptation) |
| Evac as an escape or a way to win | Nobody leaves the Zone; the only ending is the cure |
| Crates from the evac helicopter | Free loot; the Last Helicopter pays Fame and Favor instead |
| The free hourly Requisition crate | Free loot; Requisition is a half-price purchase |
| The Haven Champion's supply bag on respawn | Free loot; replaced by Safe Harbour and Remembered |
| A personal hideout per player | Leaves persistent physical traces; the Haven's safehouses are the shelter |
| Factions hostile to all players or none | Replaced by per-player hostility through hidden faction variants |
| Haven guards that never shoot | They shoot armed exiles near a safehouse; unarmed exiles just find the door shut |
| Shop System as the faction traders | Our own quartermasters gate stock by standing; Shop System stays optional for general trading |
| DarcMissions | Its convoys, occupations and crash sites assume forces from outside; they don't fit the sealed Zone |
| Ambient Zombies | Overlaps the ReforgerZ spawner |
| Sleep and time skip | Out of scope by design |
| A trait-points system on top of backgrounds | Not wanted; backgrounds, personal goals and Legacy unlocks carry character choice |
