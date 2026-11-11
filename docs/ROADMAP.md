# Roadmap

What's built, what's next, what to test first, and what was dropped. How each system works
is in [DESIGN.md](DESIGN.md).

**Status:** the systems under "Built" are written in Enforce Script (about 151,000 lines) and the
game scripts **compile cleanly** against Arma Reforger 1.9. The game mode runs with no script
exceptions on the server, in a test world with a fake player
([runtime test](WORKBENCH_SETUP.md#runtime-test-without-workbench)), and in Workbench play on Arland
with the first Workbench build. Before it can be called working it still needs proper prefabs in
place of the stand-ins, the behavioural `VERIFY` checks and a play-test with a game client
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).

## Built

| System | Design |
|---|---|
| Settlement detection, rotating spawn points, mission announcements, per-player map icons (vanilla text markers without them) | [Settlements](DESIGN.md#settlements-spawn-points-and-the-map) |
| The Director: four outbreak stages, infection spread, paced for a season of about 45 real days from about 2 to 20 regulars (research scaled to the last week's player-hours; season speed and cure pace for other season lengths; `tools/pacing_sim.py` checks the tuning) | [The Director](DESIGN.md#the-director-outbreak-stages-and-spread), [Season pacing](DESIGN.md#season-pacing) |
| Heat from presence, vehicles, gunfire (suppressors quieter), noisemakers; hordes toward noise | [Heat and noise](DESIGN.md#heat-and-noise) |
| Stealth: each player's visibility (stance, speed, light, weather, noise), the infected near them seeing the most visible player, losing a hidden player (search, then give up), gentler mission sight and notice checks for someone lying low, visibility on the HUD; any zombie mod (perception, or by distance) | [Stealth](DESIGN.md#stealth) |
| One move waypoint per AI group: every order moves the same waypoint (no order leaves one in the world), deleted with the group; checked on the server with Bacon Zombies | [Principles](DESIGN.md#principles) |
| Zombie spawner for any zombie mod, bite detection (scratches and bites, protective clothing), the golden window and wound cleaning, staged infection, the Turned Player | [Zombies](DESIGN.md#zombies-bites-and-infection) |
| Treatments from day one: antidote and stabiliser stocks scaled by research, Halcyon's trial dose, safehouse bite declarations, five low-grade meds with side effects | [Treatments](DESIGN.md#treatments) |
| Light adaptation: specials after breakthroughs, one trait per stage, favourite spots | [Infection adapts](INFECTED.md#infection-adapts) |
| The infected, phase 1 of the tribes design: the towns' crawlers (placed by the time of day, drifting to noise, the kill ledger, reinforcements from the buildings, stir and scavengers as data, giving up on locked doors), loose ones that walk to a town or a lair, heat hordes from the nearest town, the agent budget (100 agents, a share per player cluster, 8 a tick, the engine's active-AI limit at 160; also the one body budget, with escorts and wanderers counted in it), no spawn in a player's sight within 300 m, runtime groups, per-member tracking (samples where a special fell), data parts with write-back, the silent kill, hit attribution, `/tribes` and the status line; both switches off gives the old spawner | [INFECTED.md](INFECTED.md) |
| The Perimeter: warning and kill zones, one drone per player, sealed road checkpoints | [The Perimeter](DESIGN.md#the-perimeter) |
| Night, rain, fog, fog banks and storms; the day's length (4 real hours by default) and the time of day kept across restarts; the power grid and power surges; the radio station; the government broadcast | [Time and environment](DESIGN.md#time-and-environment) |
| 13 backgrounds (5 unlocked through Legacy), personal goals, Legacy points | [Backgrounds](DESIGN.md#backgrounds-and-personal-goals) |
| Fame, Infamy and Favor; tiers; the 16 reputation names with what each counts as (the card, the debounced change notice, `/rep` for admins); Earned Fame and Feared; the credit rule at 75 Fame; rivals; amends (contracts, paying Favor, surrender, settling up in Fame, the parley's reserved job) from one forgiveness budget of 30 a life; the parley for the owed at Bastion, Red Hand and Halcyon gates and the Haven's knock (empty hands, alone, once per 2 real hours); quartermaster shops; every perk (Requisition, Fire Mission, the fence, the bounty board, Halcyon's medic and immunity booster, the Haven Champion's Safe Harbour and Remembered...); hunter squads; bounties; reputation clues | [REPUTATION.md](REPUTATION.md) |
| Bastion checkpoints and Red Hand roadblocks inside the Zone (unsearched passage, tolls, searches; the Wildcard tier searched and tolled double; the company you keep: the whole car searched), surrender, camps that warn and refuse, safe zones | [REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps) |
| Per-player AI hostility (hidden faction variants), retaliation, Outlaw fallback | [REPUTATION.md](REPUTATION.md#what-makes-ai-shoot-you) |
| Borrowed kit, disguise phase 1: kit read from the loadout slots (colours, full kit, tells, own colours), claims judged by one fixed look shown before you commit (cover details: the word at the wire, a crew name, a name you've heard, a mission's paper), passage and a moment only, caught (the kit, +3 Infamy, 3 catches per life per faction, blown an hour, held), colours at posts and camps, leaders, the wire and the stand see through it, doubling only for acts seen while passing; `PassAs` and the TRICK way; Bust Out's claim and second look, Double Agent's colours rule | [REPUTATION.md](REPUTATION.md#borrowed-kit) |
| Territory: strongholds, outposts, the territory tick, faction strength | [Territory](DESIGN.md#territory) |
| Camp life: each faction's scenery round its holdings, safehouses, field stations and road posts (seeded, any map, only near players, nothing saved; a guard deletes anything usable and the scans ignore it), and its people's routines at the 3 places nearest players (sentry, patrol, sit, work, gather, stroll, the courier, sleep as a despawn; Bastion's shift change and roll, the Haven's supper), with one extra person per live place. The 21 compositions and the kits are still to build in Workbench | [Camp life](DESIGN.md#camp-life), [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits) |
| Real supply trucks with AI crews (the truck leaves its yard once the whole crew has spawned and got in), shortages, escort, drive-in and theft | [Supply trucks](DESIGN.md#supply-trucks) |
| Faction life: needs, morale, chatter, life events, truces and feuds, moments, rumours | [Faction life](DESIGN.md#faction-life), [REPUTATION.md](REPUTATION.md#morale-needs-truces-and-feuds) |
| Haven safehouses: population tiers, upkeep, rescues, house rules, exile, notice boards (notes between sessions, `/note`) | [Haven safehouses](DESIGN.md#haven-safehouses) |
| Named wanderers: travel, reactions, memory, deaths in the news, lost outposts (rerouting, rumours, mood) | [Wanderers](DESIGN.md#wanderers) |
| Named leaders: traits, loyalty, memory, voiced briefs, deaths and replacements | [Leaders](DESIGN.md#leaders), [REPUTATION.md](REPUTATION.md#faction-leaders) |
| 148 missions: 66 faction contracts with the 3 hard-edge contracts (No Reply, Struck Off, The List) (on shared building blocks: escorts, live capture, vehicle zones and convoys, health cost, step chains), 3 Red Hand hard edges (Learning the Drones, The Pit's Debtor, Tied Down), 22 world events, 15 puzzles, 6 unarmed, 3 Perimeter, 3 Haven house contracts and the Courier's letters, 27 leader jobs and Rein In, 3 arc missions (merged, dropped, errand and stage-only ones left out) | [MISSIONS.md](MISSIONS.md) |
| Errands, phase 3 of [the plan](../designs/PLAN.md#phases-18): named people at safehouses (four each, two shown) and holdings (territory's) answer **Anything need doing?** and wave passing players over (inside the interruption budget); the wanderers answer **Need a hand?**; the dealer (asked and unasked rules, anchored candidates checked cheapest first, scored, the top three), soft holds and overtaking, the faults table, 24 routines at the four holding kinds, the wanderers' asks (Before Dark, Wanted Poster, Barter Chain as an ask), 18 cards, the small-pay ledger (Fame at most 2, never above 39, 10 a faction a season day, small pay; Favor inside 40 an hour, diminishing; amends at 15+ Infamy), the "Most people helped" hall line, the journal rows, telemetry and an admin view on the talk screen | [ERRANDS.md](ERRANDS.md) |
| The mission style, waves 1 to 3: the six story jobs (Protection Racket, Lost Colleague, Crash Site, Distress Call, Struck Off, Survivors Trapped: truths, ways, records, lies and fallout errands; simplified, [MISSIONS.md](MISSIONS.md)), the 9 merges, mirror offers, the Tier 0 audit's fallout lines; the story kit, who took part and who decides (the 5 s hold, Object, Stand with), who benefits (the choice missions built on it: a switch pays at most the base and leaves season Fame alone; half the Favor to the others who did the work), the truth deck, the reason lane and the one weight clamp, the journal's per-row detail | [Story blocks](DESIGN.md#story-blocks) |
| The world memory's writers: bites, deaths, lost trucks, shells, fires and trial doses in the signal ring; reasons from upkeep, trucks, towns, leaders, wanderers and the lab; the town log from missions, rescues, burned towns and taken outposts, with season marks on the verge; world records from errands and choice missions | [World memory](DESIGN.md#world-memory) |
| Six story arcs and session headlines | [Story arcs](DESIGN.md#story-arcs-and-headlines) |
| Questlines: long storylines over many sessions, in stages (a contract or a wait for time and the world), with approaches (talk, persuade, sneak, force, bribe, betray, side with another faction) that set the outcome, capped endings through the existing systems, the journal, saved across restarts. Fifteen built, about 90 stage missions: The List, The Price of the Door, The Edge of Town (the Haven); Await Relief, The Census, The Last Pilot (Bastion); The Book, Cargo, The Well Story (the Red Hand); The Three Weeks, The Other Two, Authors (Halcyon); the Long Table's two; and the example Short Weight. They cross through shared flags | [Questlines](DESIGN.md#questlines), [MISSIONS.md](MISSIONS.md#questlines) |
| The Long Table: some seasons, one small family in a farmhouse past the emptiest village; data with bodies only near players, capped takings (never from a safehouse, a holding or a player) and their tells, suspicion and rumours, the host on the road, the guest rule, dealings (eat, a jar, declare a bite), found out with each faction's reaction, moving once, broken up; its two questlines, The House That Went Quiet and A Place at the Table | [The Long Table](DESIGN.md#the-long-table), [REPUTATION.md](REPUTATION.md#the-long-table) |
| World facts: one layer content reads (the cure and the epilogue, stage, Bastion presence, the day, night, who holds a town and a filtered town pick, safehouses, live leader names), season flags shared with the questlines, canon facts made once a season, claims so two stories don't use the same house, leader or wanderer, `Fill` tokens for text (briefs filled when shown), `Changed` for news, one "can it run now" check every mission starter uses, and missions called off when the world makes them pointless | [World facts](DESIGN.md#world-facts) |
| The epilogue after the cure: the Perimeter stands down (no shots; whoever walks up to the line is turned back; the gates open with the wipe), research stops and running research and line jobs are called off, and the broadcast, guidance and the factions' lines follow | [The Perimeter](DESIGN.md#the-perimeter), [Cure research](DESIGN.md#cure-research-and-the-seasons-end) |
| The shared engine (phase 1 of [the plan](../designs/PLAN.md#phases-18)): the clocks (season days, `RZ_GameTime` windows in real minutes), one check helper (Trust, Fear, threats, the price table, Pass / Short / Refused) and the ways through a job keyed by identity (the approach enum with PRESS, TRADE, TRICK and LEAVE), the deeds store (life and season), world memory (reasons, the signal ring, the town log, world records), the callback queue, notice lanes and the interruption budget, the marks spawner, the season budget in lanes with the seat ledger, and the set-piece calendar. Contracts already draw on the budget and leaders' pings and the Long Table's host on the lanes; most writers and readers come with the later phases | [The shared engine](DESIGN.md#the-shared-engine) |
| Town memory and journals of the dead | [Town memory](DESIGN.md#town-memory) |
| The infected tribes, phase 2: seeds on every slot (the column, the site tribe), ranges, hunting grounds, trails and scout posts from the land grid; the daily rhythm (day parties of a fifth in twos and threes with hop and hold, hungry ones to the town-edge fields; the dusk call as one status line per player; the set-out, testing the wire when hungry; the dawn walk and count); hunts by placement (the young ahead, flankers, the core held back) with the night's commitment, the long call and the hide rule; hunger per counted night, the taken with attribution and Your dead run's caps, the ones who ran, wanderers taken and noises heard as data, post-feed sleep, weakened tribes; kill sites in feeding order, the vigil, the alpha's body with saved health, samples with the town specials scaled; the scout rule on stealth's sight radius and scouts' packs; the Hunter's tracks; tribes stir towns and leave kill sites; `/tribes hunt` and `/tribes call` | [Tribes](INFECTED.md#tribes) |
| The infected behaviour layer: Bacon hooks (slots, stance, intake and share gates, the climb gate, mute), blind steering with release at reach or on contact and take-back, per-member slots in formations with the 3 m minimum enforced, the notice meter on stealth's visibility, shot leads, rush, stalk (along cover from the land grid), flanking pairs and encircle by temperament, give up; the water line check, stairs as wait below, the unstick ladder, speed caps per kind, LOD bands; the hunter's lunge and bite. Built only on the lab's B1-B20 verdicts | [How they move and hunt](INFECTED.md#how-they-move-and-hunt) |
| The land grid (on with Zombies > Tribes): water, height, trees, buildings, roads and inferred bridges in 250 m squares, ridges, valleys, forests, fields, lair candidates, wild land and no-lair overlays, read once and cached, tribe slots at 1 per 5 km² with an override; measured on Arland, Everon and Cain (`tools/land_survey/`) | [The land](INFECTED.md#the-land) |
| World interactions: loose loot in houses (rolled as data, put out on furniture and floors only near players, taken with vanilla's Pick up, refilled after an hour; keys, letters and mission items lie loose too), bodies looted with the vanilla inventory (what they carry goes into the body, or beside it; no search prompt), searching car boots, locked doors (the vanilla door action hooked; AI kept out by doorway blockers with local navmesh rebuilds, an optional AI door hook, and infected banging on the door) and keys as real items, readable things, fixable cars (only ever found: two or three missing parts, a wrench or toolkit to fit them, long loud hotwiring that can fail, rare keys; the tyre and planks are carryable items), crafting, traps, generators, fire (a thrown molotov, or a full fuel can in your hands at the doorway), water, treating others, bells, secure radio | [World interactions](DESIGN.md#world-interactions) |
| New-player guidance: experience levels, tips, spawn scenarios, wind-up radio, faction voices | [New-player guidance](DESIGN.md#new-player-guidance) |
| Cure research, antidotes, the final stand, the season's winner; the pace by play (research ×0.25 to ×2.5 for the last week's player-hours); the lab's capacity (a backlog worked through at 3% a real day, capped at 4 days of work as a backstop: samples refused and mission research lost while it's full, +25% on what fills a near-empty backlog to half a day); breakthrough news held until someone is online; the finale's 90% gate as a hook, used only once a finale questline is registered, with a fallback of 1% a real day after a week there | [Cure research](DESIGN.md#cure-research-and-the-seasons-end) |
| Seasons: next-season rules from the last winner, rule sets (Winter, Fog, Military Collapse, No Vehicles, or your own), the wipe cycle (only on the cure after an epilogue by default; a timed wipe after N weeks for admins who pick it; backup and announcements), the hall of fame | [Seasons](DESIGN.md#seasons) |
| Contracts that give a rough area (a 500 m circle) until you're close; Radio Operators and Trusted players get the spot | [MISSIONS.md](MISSIONS.md#who-gets-offered-what) |
| Cleanup of loose items (crates, samples, keys, purchases and finds left lying 30 min) | [Loose items](DESIGN.md#loose-items) |
| Save and load of mode state | [Persistence](DESIGN.md#persistence) |
| The UI: an original icon set (61 icons, SVG and PNG), mockups of every screen, and the scripts for the HUD (infection timer, secure-radio indicator, noise meter, styled notice cards with the vanilla hint as fallback), the notebook (Standing, Journal, Radio, Map key and Hall of fame as tabs, opened with N, the pause menu's Notebook button or the notebook item every life carries; gamepad and mouse), notices paced a few at a time by importance and sent only to their player, a talk screen for every person who talks (their lines, your replies with what each needs shown before you choose; gamepad, keys 1–7 and mouse), the car repair screen (a fixable car's job card beside its side view, each job lit where it is, checked on the server; the car's actions without its layout), the quartermaster's shop screen (categories as tabs, each item's price and the standing it needs, a turnable picture of the item, Buy through the same checks as the actions; the actions without its layout), spoken lines as subtitles at the bottom of the screen, notes and letters in a paper reader (danger closes it), the notice board menu, chat commands (`/note`), a deploy-menu background panel with the loadout's gear and locked loadouts unavailable, and the marker icons on the map, fed per player from the server | [UI.md](UI.md) |
| Talk, phase 2 of [the plan](../designs/PLAN.md#phases-18): conversation trees on the talk screen (**Dialogue > Trees**, off by default until the per-player label test and a play-test pass): hubs with the built replies as verbs and topics by priority, nodes and choices with checks shown before you pick (Trust, Fear, prices, backgrounds, standing; no dice), regard, claims and knowledge per player and per character in `dialogue.json`, a speaker at the barrier of road posts, the `.rzd` text format with its lint, simulator and `shipped.lock`, and 30 conversations (leaders in four voices, the 8 wanderer roles, occupants, two named people, two post speakers, the Long Table, three asks). Talk never pays Fame. Reputation's voice: the word for you, Red Hand crew names, the whole picture, what a leader makes of you in four voices, the opening line, deed phrases for 37 contracts, local deeds, life-stamped memory. Tone: the season mood (the broadcast reads it), temper, grief near deaths, the voice bank with joke memory, `tools/voice_sim.py` | [DIALOGUE.md](DIALOGUE.md), [REPUTATION.md](REPUTATION.md#what-people-call-you), [WRITING.md](WRITING.md#the-season-mood) |
| Text tools: `tools/strings_inventory.py` lists every line of player text in the scripts (channel, faction, a register guess, repeats), and one text lint, `tools/voice_lint.py`, checks it against the writing rules, with a baseline of today's findings so only new or changed text fails | [WRITING.md](WRITING.md#checking-text), [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#text-lint) |
| Setup tooling: the prefab manifest (every prefab, slot, settings object, faction and loadout, with a vanilla or Workshop prefab for 152 of its entries and the 13 loadouts), the Workbench plugin **ReforgerZ: Create prefabs** (run for real; a second run changes nothing) with a dry run and a checklist fallback, `tools/icon_atlas.py` for the icon atlas and imageset, and a checker that keeps the manifest and the setup doc in step with the scripts | [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#1-create-the-prefabs) |
| The first Workbench build: 159 prefabs (156 the plugin made from the manifest, many of them on stand-in bases, plus the player controller with the HUD and the faction and loadout managers), the Arland world with the game mode, the four factions and their hidden variants and the 13 background loadouts, its scenario, the 12 UI layouts, the icon atlas and imageset, and the screens and their keys in the menu and input configs. It starts in Workbench play with no script exceptions | [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md), [UI.md](UI.md) |

## Next

The build-review fixes are done. Everything below is approved. The order of the larger work is
[designs/PLAN.md](../designs/PLAN.md#the-build-order), which also lists what it builds alongside these items.

### Get it playable
- **Special Zombies on a Linux server (open issue).** With it as a dependency, a Linux dedicated
  server loads none of ReforgerZ's scripts, without an error
  ([MOD_STACK.md](MOD_STACK.md#special-zombies-stops-reforgerz-loading-on-a-linux-server)). Until
  it's resolved, host on Windows or drop Special Zombies. Find a fix (ask its author, or check
  whether a newer server finds both folders), then confirm a Linux server with it loads the mode.
- **Untested in play:** the fixable cars' names, the parked cars' lock, the tyre and planks as
  items (built by text on vanilla `Item_Base`), and the unarmed zombie groups
  (`Prefabs/ReforgerZ/Zombies/`).
- **Refine the stand-in prefabs.** Many bases are stand-ins (five items share the aspirin bottle,
  the molotov is a smoke grenade, the drone is a radio box), and none is checked in play yet
  ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#4-the-prefab-manifest)).
- **The manual steps** the plugin still lists ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)); the
  loadouts' gear, shop stock, the consumable effects and the reward crate were filled in by text
  edits and need a play-test.
- **Camp life in Workbench:** the 21 compositions, the smart-action waypoint and the kits (two
  evenings; [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits)).
- **Play it with a game client** on a local server, then a 2-hour session on Arland
  ([PC_SETUP.md](PC_SETUP.md#6-play-testing)).

### UI
- **Controllers:** check the whole mod with a gamepad and fix what doesn't work
  ([UI.md](UI.md#controllers)). Planned.

### World interactions (from play-testing)
- **The deploy panel and the map's icon layer:** the deploy-menu override that adds the background
  panel, the map config override with `RZ_MapIconsUI` and the map icon layout (until it's set, the
  vanilla text markers stay), and textures for the icons the scripts never swap ([UI.md](UI.md)).

### The season's designs
- The approved designs for the rest of the game (the infected tribes and crawlers, the four majors,
  the cure's finale, the epilogue week, dialogue, errands, the mission style, reputation names,
  disguise and tone) are in [designs/](../designs/README.md). Their build order, from step 0 (now)
  to phase 8, is [designs/PLAN.md](../designs/PLAN.md#the-build-order); it isn't restated here.
- Already built for them: most of phase 4, the mission style in full (the six story jobs, reputation's
  parley and settling up, lies and witnesses, dialogue's scenes for several players, borrowed kit,
  fallout errands and strangers at their spots: [MISSIONS.md](MISSIONS.md), [ERRANDS.md](ERRANDS.md#strangers);
  locals, twists, board and radio asks and the other cards are still to build); phase 3, small things everywhere ([ERRANDS.md](ERRANDS.md), the mission
  style's [story blocks](DESIGN.md#story-blocks) and the world memory's writers; play-test the
  errands' pay per player-hour with the telemetry line); phase 2, talk ([DIALOGUE.md](DIALOGUE.md),
  reputation's voice and tone's mood and voice bank; play-test it with **Dialogue > Trees** on
  before it's turned on by default);
  phase 1, the shared engine and reputation's names
  ([DESIGN.md](DESIGN.md#the-shared-engine), [REPUTATION.md](REPUTATION.md#reputations)), with one
  body budget shared with the tribes' agent budget; the cure's 90% hook (`IsCureFinale`,
  `AddFinaleResearch`; [Cure research](DESIGN.md#cure-research-and-the-seasons-end)), phase 1 of the
  tribes (the towns' crawlers, the land grid in use, the agent budget and the plumbing) and its
  phase 2 (the tribes and the behaviour layer, [INFECTED.md](INFECTED.md)), one held waypoint per
  group, and `tools/tribe_sim.py`,
  which simulates the tribes as data over a season at 2, 5 and 10 players on the wild land the land
  survey measured, and prints the numbers it tuned against the design.

### Content on the world facts
- Switch the rest of the content to the world facts ([DESIGN.md](DESIGN.md#world-facts), the
  *Planned* line): contract town pickers by who holds a town (`PickTown`), Bastion contracts and
  road posts by Bastion presence (a balance call to play-test: Aftermath's presence is 0.1),
  briefs' days and copied leader names as tokens, the origin story, the last orders and the pit
  as canon, the lab and the helicopter as claims, standalone contracts that share a questline's
  premise kept out while it runs (the keys are in [QUESTLINES.md](QUESTLINES.md#the-questlines)),
  research jobs outside Halcyon off after the cure, Bastion Shelling kept off lived-in places.

## Play-test priorities

In this order, because later ones depend on earlier ones:

1. **AI in Workbench play.** The first plays had no AI world in the world, so no AI or infected
   spawned and there was no road network. With Arland's AI world added, `No road network found` is
   gone. The next two plays crashed in the first frame after the first infected spawned, both at
   the same address: the world had no perception manager, which every AI needs. The world now has
   it, with the radio manager, the chat and the item preview manager that vanilla's game-mode
   worlds also place. Play again and read the log ([PC_SETUP.md](PC_SETUP.md#logs)): do the
   infected, AI groups and supply trucks spawn, and does it crash again? No road crosses the line
   on Arland, so it has no road checkpoints; road posts inside the Zone use the same road network.
2. **With a game client.** It has only run in Workbench play so far. Check the `VERIFY`
   behaviours, riskiest first ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#6-the-verify-list));
   several only show on a dedicated server. Host it on Windows, or on Linux without Special
   Zombies: on Linux with it, the mode doesn't load at all ([Next](#get-it-playable)).
3. **Per-player hostility.** A player Hunted by Bastion near a Bastion AI group: does it fire?
   Do the deploy menu, chat and groups stay on `SURV`? If AI ignores the variants, switch to Outlaw.
4. **The core loop on Arland** for 2 hours: zombie density, spread rate, cure pace. Then the
   season's first days: are kills, specials and research per player-hour near the assumptions
   in [Season pacing](DESIGN.md#season-pacing)? Run `tools/pacing_sim.py` with what you saw and
   adjust the Cure pace. Over the first weeks: does the status log's pace by play match how much
   the server plays, and does a small group still feel its deliveries count? Is the lab's backlog
   a fair wait, or does a "come back tomorrow" feel like a wall? Do towns players work in clean up
   too easily over the longer season (Zombies > Kill infection)?
5. **Supply trucks:** one truck from stronghold to outpost. Is the crew seated, and does it drive?
6. **Noise:** how far a thrown bottle really leads infected; the favourite-spot rate.
   **Stealth:** does the zombie mod take the perception factor and list players as targets (the
   log line, the VERIFY list)? Is lying low worth it without being a way to skip the infected
   (the checklist's 30/60/100 m test, day and night, against **Stealth** off)?
7. **Puzzles** one by one, and whether replicated action labels work.
8. **Loose loot:** do things lie on furniture and stay put, in which buildings do they float or
   sink, does a town feel worth walking into, and what does putting a town out cost the server's
   frame time? Find rates against town loot; how often a car is fixed
   now that it takes two or three parts, a tool and a hotwiring that can fail.
   **Door locks:** does the hooked door action compile, refuse only from outside, and get inside
   right on several building types? Do door spots stand at the right doors? If hooking fails,
   turn **Locks** off (nothing is locked then). **Keeping AI out:** does the AI door
   hook compile (else delete it)? Do blockers stand in the doorway, make AI route around a locked
   door after the navmesh rebuild, and cost little server time? Can anyone get trapped?
9. **The Perimeter** on an island map and a land-locked one. On the island: do the three Perimeter missions put their spots on the shore (the `Perimeter shore:` log line), and is the coast under the drones often enough?
10. **Safehouses:** upkeep timing, food keywords, the medic and bite declarations.
    **Bites:** how often hits become bites in real fights (the protective clothing keywords),
    whether the golden window is long enough to clean a bite mid-fight, whether the stamina hook
    works (else the stages only make noise), and whether the meds' side effects are worth it.
11. **Leaders:** loyalty drift and rogue-job discovery rates.
12. **Guidance:** is anything too chatty for a level-0 player? Do the right speakers say it?
13. **Story arcs** end to end (start one with `RZ_StoryArcs.Begin(kind)`) and how often they come.
    **After the cure:** the line stands down (no drone, no shots, the new loudspeaker line), no
    samples drop, the lab refuses them, no Perimeter or Halcyon jobs appear.
    **Questlines** end to end with **Wait scale** at 0.1 (`RZ_Questlines.Begin(key)`): a restart
    mid-way, two approaches to the same stage, a player who joins late. **The Long Table:** where
    it lands on Arland and a bigger map, how soon suspicion reaches 30 (hunger and takings), whether
    the guest rule's timing is fair and its fight fires, and whether the writing stays on the right
    side of the line.
14. **Weather** accessors and the generator's noise.
    **The infected** (phase 1, the checklist's "The infected"): do towns read as full and slow, or
    just as many and fast (Bacon has no slow prefab)? Is 28 a cluster enough for a town in Overrun,
    and 100 on the server enough with several groups out? Does the sight rule (the physics trace's
    cost, the status line's frame) keep spawns out of view without starving towns near players? Do
    loose ones lead somewhere worth following, and does a siege's end feel fair?
    **The tribes** (phase 2, the checklist's "The tribes and how they move"): a solo night crossing,
    a squad at night and a day pass by the lair. Is a careful crossing survivable, does day stay the
    lighter danger (lower **Day party share** if days feel like nights), is every strike heard before
    it lands, do blind bodies ever look broken, do formations and flanks read as a pack, does the
    notice rate match Bacon's ladder (a walker at 100 m in about 20 s; then re-run
    `tools/tribe_sim.py` with it), and can a new player name the place a call came from after one
    evening?
15. **Road posts and camps:** do posts go up on real roads ahead of players, and do the search,
    the toll, running and surrender fire at the right distances? Is the timed grudge enough to
    make the crew shoot, and does a surrender's stand-down stop them? Is a Fire Mission's horde
    check too strict or too loose; does the fence take the right things?
    **Camp life:** do our groups use the compositions' smart actions and loop their patrol, do lone
    characters join a group, do AI walk through run-time tents, and what do 3 live places cost in
    server frame time?
16. **Seasons:** a wipe end to end (backup, fresh start, the winner's rules, the hall of fame
    kept), and whether the rule sets feel different enough without being unfair. Are rough
    contract areas fun to search, or too vague at 500 m?
17. **The UI** with a game client and a gamepad: does the owner-only feed arrive, do notices come
    a few at a time, does the timer agree with the stage messages, do the notebook (N, the pause
    menu, the item), the talk screen and the reader open, fill, take clicks and the pad, and close
    in danger, do the map icons sit on their
    spots while panning and zooming (once the map layer is set up), and does the deploy menu grey
    out locked loadouts ([UI.md](UI.md))?
18. **Reputation names and the forgiveness budget:** do new players still understand "needs
    Trusted" when the card says Favourite Rascal? Does 30 a life feel fair to a long-lived loyalist
    whose rival shares pile up, and does the credit rule's Fame cost read as fair at Champion? Does
    the Wildcard search feel like the company you keep, or a nuisance? Does the contracts lane of
    the season budget ever clip on a busy evening (the `Budget:` log lines)?

## Dropped

| Idea | Why |
|---|---|
| Supply drops, airdrops, the Supply Plane Crash event | The Zone is sealed: nothing comes in from outside |
| A visible wall, fence and watchtowers at the map edge | The cordon is felt, not seen: drones, loudspeakers, unseen snipers and sealed checkpoints |
| Sterilisation, "Patience" and other outside deadlines | The outside never acts; urgency comes from inside (stages, spread, adaptation) |
| Evac as an escape or a way to win | Nobody leaves the Zone; the only ending is the cure |
| Crates from the evac helicopter | Free loot; the Last Helicopter pays Fame and Favor instead |
| The free hourly Requisition crate | Free loot; Requisition is a half-price purchase |
| Vehicles, car parts and fuel in the quartermasters' shops and requisitions | Cars are only ever found and fixed; the server refuses such stock |
| The Haven Champion's supply bag on respawn | Free loot; replaced by Safe Harbour and Remembered |
| Dialogue's HUD panel, action-menu answer donors and the numbered `/t` chat fallback | One modal talk screen works with the pad and the mouse; a chat command would break the console rule |
| A personal hideout per player | Leaves persistent physical traces; the Haven's safehouses are the shelter |
| Factions hostile to all players or none | Replaced by per-player hostility through hidden faction variants |
| Haven guards that never shoot | They shoot armed exiles near a safehouse; unarmed exiles just find the door shut |
| Shop System as the faction traders | Our own quartermasters gate stock by standing; Shop System stays optional for general trading |
| DarcMissions | Its convoys, occupations and crash sites assume forces from outside; they don't fit the sealed Zone |
| Ambient Zombies | Overlaps the ReforgerZ spawner |
| Sleep and time skip | Out of scope by design |
| The infected dragging a player | The server can't move a player smoothly against their own machine's prediction; a grab that holds you in place is the planned alternative |
| A trait-points system on top of backgrounds | Not wanted; backgrounds, personal goals and Legacy unlocks carry character choice |
| Danger money on shop prices for mixed reputations | A name opens and costs nothing; the Wildcard tier pays at the posts (the search, the double toll) instead |
| Clear the Road | Kills along a straight line; the tribes' Clear the Trail clears roads for a reason |
| Signal Fires | Needs three players online at once, and has no story in it |
| Restore Comms | A parts fetch and a hold at a mast; Radio Relay is the same job |
| Clear the Airfield | Existed only to make the helicopter run more often, on a field that isn't an airfield |
| Toll Road | The Red Hand's built roadblocks are the toll roads; Skim the Toll keeps the grey part |
| Quarantine Breach | Ten minutes of killing in a safe town, with no choice in it |
| Crashed Helicopter | The same wreck as Crash Site; The Last Pilot's ending starts Crash Site instead |
| Burning Town | The second fire event; Gas Station Fire keeps the set piece |
| Paces from the Tower | A third stash hunt beside Stash Note and Two Bearings |
| Hot and Cold | A radio stash hunt; The Numbers Station keeps the radio puzzle |
| The Old Clinic | A generic hold |
| Field Station | A three-minute hold for one research point |
