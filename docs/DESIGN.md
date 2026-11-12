# Systems design

How every ReforgerZ system works, organised by system. Everything here is written in code
unless it says **planned**; *Simplified* notes say where the code does less than the idea.
What's next and what was dropped is in [ROADMAP.md](ROADMAP.md). Factions, standing and
AI hostility are in [REPUTATION.md](REPUTATION.md); every mission is in [MISSIONS.md](MISSIONS.md).
Code paths are under `scripts/Game/ReforgerZ/`.

## Contents

1. [Principles](#principles)
2. [Settlements, spawn points and the map](#settlements-spawn-points-and-the-map)
3. [The Director: outbreak stages and spread](#the-director-outbreak-stages-and-spread)
4. [Heat and noise](#heat-and-noise)
5. [Stealth](#stealth)
6. [Zombies, bites and infection](#zombies-bites-and-infection)
7. [Infection adapts](#infection-adapts)
8. [The Perimeter](#the-perimeter)
9. [Time and environment](#time-and-environment)
10. [Backgrounds and personal goals](#backgrounds-and-personal-goals)
11. [Factions in the background](#factions-in-the-background)
12. [Haven safehouses](#haven-safehouses)
13. [Wanderers](#wanderers)
14. [Leaders](#leaders)
15. [Talking to people](#talking-to-people)
16. [Missions](#missions)
17. [Story arcs and headlines](#story-arcs-and-headlines)
18. [Questlines](#questlines)
19. [World facts](#world-facts)
20. [The shared engine](#the-shared-engine)
21. [Town memory](#town-memory)
22. [World interactions](#world-interactions)
23. [New-player guidance](#new-player-guidance)
24. [Cure research and the season's end](#cure-research-and-the-seasons-end)
25. [Season pacing](#season-pacing)
26. [Seasons](#seasons)
27. [Loose items](#loose-items)
28. [Persistence](#persistence)

## Principles

You're a lone civilian in a sealed Quarantine Zone ([LORE.md](LORE.md)). You can survive
alone, team up with strangers, or prey on them. The server is one shared story that moves
from Early Outbreak to Aftermath, and the only way to win is together: research the cure.

**Pillars**

1. **Every life is different.** A background from the spawn menu, a random town, random
   missions, and a world that changed since your last visit.
2. **The world moves without you.** The Director spreads infection, moves the server
   through stages and sends hordes toward noise.
3. **One shared goal.** Cure research is server-wide, and also something to fight over.
4. **Factions remember.** The Haven, Task Force Bastion, the Red Hand and Halcyon remember
   what *this character* did ([REPUTATION.md](REPUTATION.md)).
5. **Any map.** Everything is placed relative to settlements found at start.
6. **Use the Workshop** for zombies and hunger ([MOD_STACK.md](MOD_STACK.md)).

**Rules every system follows**

- **Simulate as data, spawn bodies only near players.** Population, wanderers, leaders and
  territory are numbers on the server. AI and props exist only within a few hundred metres
  of a player. The one exception is supply trucks: at most one real truck per faction.
- **Effects are capped and recover.** Nothing a player or the Director does wipes out a
  faction, a town or a safehouse for good. Strongholds never fall.
- **Background, not foreground.** Territory, adaptation and faction life are slow systems
  you notice over a session, not something you manage.
- **No physical traces.** Nothing is left lying in the world: journals, burned towns and
  looted shelves are data.
- **One move waypoint per AI group.** Every order the mod gives a group (a horde toward noise, a
  wanderer's walk, a truck's route, a mission's crew) moves the same waypoint and puts it back on
  the group's list, after any waypoints of the group's own. It's deleted with the group, or on the
  next tick when a group dies out by itself. Code: `Core/RZ_GroupOrders.c`. Measured with Bacon
  Zombies (`tools/zombie_lab/`): a finished waypoint drops off the group's list but stays in the
  world, and moving one that's still on the list does nothing until it's added again. So before
  this, every order left a waypoint behind. The status line counts the waypoints in the world.
- **Nothing for free.** No free loot, no free Fame. No sleep and no time skip.

## Settlements, spawn points and the map

Code: `World/RZ_SettlementScanner.c`, `World/RZ_Settlement.c`, `Core/RZ_GameModeComponent.c`, `Core/RZ_MapMarkers.c`, `UI/RZ_MapIcons.c`.

- **Settlements** come from the map's city, town and village labels plus clusters of buildings
  (a 300 m grid cell with 8 or more buildings). A cluster takes the name of the nearest local
  place label within 400 m (a hamlet or a point), or "<name> outskirts" if that's taken, or a
  grid reference ("Outskirts 13-29"). Label names are string-table keys, translated on the
  server. Each has a centre, a size class and an infection level (0–100). Placing any `RZ_SettlementMarkerEntity` turns detection off and uses the markers.
- **Spawn points:** 6 at a time in random towns, and one is moved to a new town every
  20 minutes. Nobody picks one: the spawn screen asks for one at random, and with openings on the
  new character is moved on to a place rolled for its background
  ([Openings](#openings-how-every-life-starts)). No safehouse is a spawn point.
- **Missions are announced** with distance and direction ("1.2 km NE"). Messages arrive a few at
  a time, the most important first, as styled notice cards or vanilla hints without the UI; the
  notebook's Radio tab keeps the radio, mission and world ones ([UI.md](UI.md#notices)).
- **The map** shows each player what they're allowed to know, with the ReforgerZ icons: the lab,
  the safehouses (not to exiles), every stronghold, the outposts of factions that know them or that
  they've been near, the power and radio stations and the sealed checkpoints. **Of the jobs, only
  the tracked one's current objective is on it**, for its player alone: a diamond on a known spot, a
  search area round a rough one. No other contract, event or errand is. This needs the icon layer
  ([UI.md](UI.md#the-map)); without it, world events and the lab are vanilla text markers for
  everyone (a toggle).

**The land grid** (`World/RZ_Land.c`): what each 250 m square of the map is like (water, height,
trees, roads, bridges) and which squares are wild land, read once and cached. Crawlers stand in its
fields, loose ones walk to its lair candidates, and the infected tribes live in its wild land and
walk its trails. It's described in [INFECTED.md](INFECTED.md#the-land).

## The Director: outbreak stages and spread

Code: `Director/RZ_Director.c`, `Director/RZ_OutbreakStage.c`.

The server moves through four stages over a season of about 45 real days ([Season pacing](#season-pacing)).
A stage ends when its time is up or when average infection passes its threshold. **The
outbreak's clock only runs while someone is playing**: stage time and spread stand still on an
empty server. Stage lengths are divided by the **season speed** and spread is multiplied by it.
Each stage's numbers can be overridden on the game mode. The days below are for 5 regulars
(about 8 hours a day with someone online).

| Stage | Lasts (hours played) | Or when average infection reaches | Density | Special chance | Spread / min | Missions | Bastion presence |
|---|---|---|---|---|---|---|---|
| Early Outbreak | 40 h (about days 1–5) | 45 | ×0.6 | 1% | 0.01 | 3 | ×1.0 |
| Collapse | 80 h (days 6–15) | 65 | ×1.0 | 1.5% | 0.016 | 4 | ×0.6 |
| Overrun | 120 h (days 16–30) | 85 | ×1.5 | 2% | 0.02 | 4 | ×0.25 |
| Aftermath | until the cure (from about day 31) | — | ×0.8 | 2.5% | 0.006 | 5 | ×0.1 |

- **Spread.** Every town creeps up slowly. A town at 60+ infection also spreads to every
  town within 2.5 km, in proportion to its own infection. Cure breakthroughs slow spread.
- **Pushing back.** Each zombie kill takes 1.5 infection off its town. Missions clear,
  burn, seal and shell towns.
- **Missions at once:** the stage's count, plus one per 4 players. "Bastion presence"
  (`RZ_WorldFacts.BastionPresence`) is how much of Bastion is still out on the roads: some
  Bastion contracts need a minimum of it, and every Bastion contract comes as often as it allows
  (the random pick's weight ×(0.3 + 0.7 × presence): ×1 in Early Outbreak, ×0.37 in Aftermath;
  Retake the Town, Last Helicopter and Checkpoint Duty scale their own weight instead,
  `m_bPresenceWeighted`). Bastion's road posts go up on the same chance
  ([REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps)). A balance call to play-test.
- Each stage change is announced to everyone in a line of the Zone's voice, except the move to
  Aftermath that comes with the cure (the cure's own news is the line everyone hears).

## Heat and noise

Code: `Director/RZ_HeatMap.c`.

Heat is how much attention an area has drawn, on a 250 m grid. It decays 8 a minute and
caps at 200 per square.

| Source | Heat |
|---|---|
| A player being somewhere | 0.5 a tick |
| In a vehicle | +4 a tick |
| A shot | 3 × the background's noise multiplier; ×0.3 with a suppressor (detected by attachment name) |
| Noisemakers, bells, sirens, generators, forcing locks, infected at barricades and locked doors, fire | see [World interactions](#world-interactions) |

- **What the infected do with it:** heat draws extra infected to a town, 80 heat near a player
  sends a horde (from the nearest town, with Tribes on), and a burst of 8+ heat at once is
  remembered as a noise that crawlers and mission infected walk to. All of that is in
  [INFECTED.md](INFECTED.md#heat-hordes).
- **Being noticed** (puzzles and unarmed missions): heat where you stand above a threshold,
  or an infected within 10–12 m of you for 6–8 s (slower while you're hard to see:
  [Stealth](#stealth)).
- Other mods can add heat with `RZ_GameModeComponent.ReportNoise(pos, heat)`.

## Stealth

Code: `Zombies/RZ_Stealth.c`. Settings: **Zombies > Stealth**.

How visible each living player is, worked out on the server once a second (not per frame), and
what the infected near them make of it. It works with any zombie mod. If the mod's infected use
the engine's perception, stealth changes how fast they recognise a player and reads who they're
after. If they use a plain aggro radius, their senses don't change and losing them goes by
distance. **Stealth off:** infected perception is never touched, nobody counts as hidden, and
missions work exactly as before.

### Visibility

Every living player has a visibility from 0 (hidden) to 1 (in plain sight). It's the stance and
speed times the surroundings (light and weather), plus recent noise, capped at 1.

| Input | Default |
|---|---|
| Stance | standing ×1, crouched ×0.6, prone ×0.5 (stealth can't see cover, so lying in the open in daylight isn't hidden on its own) |
| Speed (how far you moved since the last update) | still (under 0.4 m/s) ×0.7, walking (under 2.6) ×0.85, running (under 5) ×1, sprinting ×1.25 |
| Light | 60% the engine's light on you (sun, moon, lamps, your own torch) and 40% the clock (day 1, dusk 0.6, night 0.25). Turned into a multiplier from ×0.4 in the dark to ×1 in daylight. If the engine's light can't be read, the clock alone |
| Weather | fog or a fog bank ×0.75; rain ×0.85, or a storm ×0.7 instead; the Fog rule set's shorter sight (×0.85) |
| Surroundings floor | light × weather never counts for less than ×0.55, so darkness and bad weather alone never hide someone moving upright: only getting low and keeping still do |
| Noise | each shot adds to your noise and shots add up: one shot fills it, a suppressed one adds 0.35 (so about three in a row fill it too), times your background's noise (Hunters quieter). Full noise is +0.5 visibility, fading over 10 s. A loud noise within 15 m (a bell, a forced lock, a noisemaker at your feet) sets it to at least 0.7. While any noise is left, the infected can't lose you |
| In a vehicle | always 1 |

Some results: walking in daylight 0.85; crouched and still in daylight 0.42; prone and still in
daylight 0.35 (in rain 0.3); walking at night about 0.47 (never less, whatever the weather);
crouched and walking at night 0.28; prone and still at night 0.19. Under **0.35** counts as
hidden: that's the HUD's HIDDEN, and the only state in which the infected can lose you. Other
systems ask `RZ_Stealth.GetVisibility(playerId)` (1 with Stealth off) and
`RZ_Stealth.IsHidden(playerId, threshold)` (always false with Stealth off). A new life starts at 1.

### The infected near you

Every update, for the infected the spawner tracks (groups and their members) with a player within
**60 m** of a member, each member's perception factor is set to:

> its own factor × the base for its kind × a factor from the **most visible** player within 150 m of any member of its group, never more than ×1.2 on top of its own

- **Base by kind** (with the infected's roles, [INFECTED.md](INFECTED.md#plumbing):
  a tribe's leader and guard count as specials, a hunt on its trail as a horde that must keep
  coming, crawlers and loose ones as common): common ×1, special ×1.2 (sharper), horde member ×0.9 (a crowd on the move
  looks ahead).
- **From the player:** ×0.3 for someone fully hidden, rising to ×1 at 0.6 visibility. From 0.6 to
  0.9 (standing or walking in daylight) it's ×1, as without stealth. Only above 0.9 (running or
  sprinting in daylight, just after a shot, a vehicle: the HUD's IN PLAIN SIGHT) does it rise, to
  ×1.2. Prone and still at night gives about ×0.52, crouched and walking at night ×0.63, prone and
  still in daylight ×0.71, walking at night ×0.85.
- **The most visible player counts**, out to 150 m, not just the ones within 60 m. A friend lying
  prone close to them can't hide you while you stand further off.
- With nobody within 60 m, a group goes back to its base. At most 40 infected are touched an
  update; the rest wait their turn.
- **Bodies the behaviour layer steers** (the tribes', [INFECTED.md](INFECTED.md#how-they-move-and-hunt))
  are left alone: they're blind on purpose while our code drives them, and their own senses read
  `GetVisibility` instead. Stealth sets the factor only on their bodies that are engaged (Bacon's), and
  its search never runs on a group the layer drives. The tribes' scouts see with
  `SightRadius(player, Scout sight)` ([INFECTED.md](INFECTED.md#scouts)).
- **Never lowered for infected that must keep coming:** every horde tied to a mission (the holds
  and waves, and most event and questline hordes), and everything within 1.5 km of the lab during
  the final stand. They keep their own factor. Three hordes are the exception, because staying out
  of sight is the point there: Lead Them Away, The Clockwork Horde and The Tribe Moves event.
- With the vanilla `SCR_AICombatComponent` on the infected, the factor is set as its multiplier.
  The AI's own update applies it on top of its alert state, and would overwrite a factor set
  straight on the perception. If that component has no vanilla utility AI behind it (a mod's own
  agent), nothing else updates the perception, so the factor goes straight on the
  `PerceptionComponent`, as it does without the component. Without a `PerceptionComponent` nothing
  changes. Perception that still refuses the factor after 5 updates in a row counts as none. Each
  case is logged once.
- The factor only changes how fast they recognise what they see. Hearing is the engine's own. The
  engine already sees stance and light, so stealth's factor comes on top, which is why it never
  goes below ×0.3.

### Losing them

A group that is after a player can lose them:

1. **Who it has:** the player among its members' engine targets it has recognised as an enemy
   (not just a hint of someone), seen or heard most recently. For infected without engine
   perception: a member touching a player (4 m), a member within 20 m of a player who has just made
   noise, or the group coming at a player within 20 m at a run (2 m/s or more, faster than toward
   where a horde is headed, and not while walking where stealth sent it). It stays on that player
   for up to 53 s (8 + 45) after.
2. **Lost:** that player has stayed hidden (under **0.35**), unseen, unheard and with no noise left
   for **8 s**, counted from when the group last had them. A group that had a player a while ago
   and isn't after them now has nothing to lose, and a contact it has already searched from is
   never lost twice.
3. **Search:** the group walks to where it last had them. A group with nothing else to do then
   looks about for **45 s**, a new spot 5–20 m away every 10–15 s. A group with waypoints of its
   own (a horde on its way) carries on with them once it has been there. If it sees or hears
   anyone in that time, its own AI takes over.
4. **Give up:** stealth's waypoint goes, and the group carries on with its own. One with none left
   goes back: a horde to where it was sent, anything else toward where it spawned.

Stealth's waypoint is put in front of the group's own and taken back afterwards: nothing of the
group's (a zombie mod's wander or patrol, a busy-spot visit) is removed. Never for mission infected
(their mission steers them) or the final stand. It needs a group and the move waypoint (**Zombies
> Move waypoint**); without them only the senses change. The waypoint stealth gives a group is
deleted with the group.

### Stealth in missions

Missions use the score only to go easier on someone lying low, never harder. A player at
visibility 0.6 or more (standing in daylight, even keeping still) is seen exactly as before, by
these checks and by common infected (who only see better than before someone in plain sight).

| Check | With Stealth on |
|---|---|
| Being noticed ([Heat and noise](#heat-and-noise)): puzzles, unarmed missions, the questlines' sneak | The heat limit is unchanged. The "infected close to you for 6–8 s" timer runs at full speed at 0.6 visibility and up, down to 0.35 of it for someone fully hidden: a little over half as fast lying still in the dark |
| People who see anyone within N m: Haven lookouts and the Haven wanderers near them, the storekeeper, the walker, Halcyon's courier, the Long Table's family, Quiet Count's specials (50 m) | N at 0.6 visibility and up, down to 0.6 × N for someone fully hidden: Quiet Count's 50 m is about 36 m lying still in the dark |

Shots nearby still spoil a sneak. In a questline stage, a person who sees anyone within N m checks
`RZ_Stealth.SightRadius(playerId, N)` (see [Writing a questline](#writing-a-questline)).

### The HUD

Above the noise meter, five segments and a word: **HIDDEN** (under 0.35: the infected can lose
you), **LOW**, **SEEN**, **IN PLAIN SIGHT** (0.9 and up: the infected see you better than before).
It's sent only when it changes, and not shown with Stealth off ([UI.md](UI.md#screens)). There's no
cue when a group loses you or gives up: you see them go.

*Simplified:* speed is how far you moved between updates, so a sprint that stops just before an
update reads as still. The stance and the engine's light are read on the server (both on the
VERIFY list). Stealth has no cover input: a wall or a hedge only counts through the engine's own
sight. Infected without engine perception are judged "on you" by distance and by how they move,
so a group that runs at you along its own path (a horde coming straight down the road you're on)
isn't counted as chasing, and never loses you. A lone infected character that isn't in a group
never searches, because it has no waypoints.

## Zombies, bites and infection

Code: `Zombies/RZ_ZombieSpawner.c`, `Zombies/RZ_BiteDetector.c`, `Core/RZ_GameModeComponent.c`.

Where the infected come from, how many there are, the towns' crawlers, the countryside's loose ones,
the agent budget and how bodies are counted are in [INFECTED.md](INFECTED.md). Specials (the stage's
chance) drop a cure sample when killed. This section is about what they do to you.

A bite is survivable if you act on it. Code: `Zombies/RZ_BiteDetector.c`, `Cure/RZ_Infection.c`, `Cure/RZ_Meds.c`.

### Bites

Works with any zombie mod: a player's health dropping with an infected within 2.5 m is a hit.
**Most hits are scratches** ("Nails, not teeth", told at most once a minute). Each hit is an
infecting bite with the **Survival > Bite chance** (10%), changed by what's happening:

| Situation | Bite chance |
|---|---|
| Surrounded: 3 or more infected within 5 m | ×2 |
| Grabbed: a second hit within 4 s | ×1.5 |
| Tired: stamina under a quarter (needs the stamina hook) | ×1.5 |
| Hurt: under 40% health | ×1.5 |
| Wearing a jacket / gloves / a helmet (worn prefab names) | ×0.8 / ×0.8 / ×0.85 |
| Never above | 60% |

A careful player in a jacket and gloves who isn't cornered is bitten about one hit in sixteen;
someone pinned by three infected at low health, nearly half the time (60% at most).

### The golden window

A bite **doesn't take for 2 minutes**, and the timer stands still. Cleaning it in that window can
stop it, once per bite:

| How | Stops it |
|---|---|
| "Clean the bite" with alcohol or antiseptic (prefab-name keywords) | 70% |
| "Disinfect and bandage it" (disinfectant or antiseptic and a bandage, or a first-aid kit) | 90% |
| Declaring the bite at a safehouse | 90% |

The "Clean the bite" spot (an interaction spot) follows the bitten player's feet and is never
refused by the spot cap; anyone can use it with their own supplies, and the bitten player has to
stay next to them until it's done. Nobody is told whether it worked until the window closes: then
"it didn't take", or the infection starts. Until then the HUD still shows the window, and meds,
an antidote and a safehouse answer the same either way (an antidote used then is spent).

### Infection stages

Once it takes, **60 minutes** to turning (**Backgrounds > Bite minutes**) × the background's
multiplier (Nurse ×1.5, Doctor ×2.5, Ex-Con ×0.8) × the cure's (×1.25 after 25%, ×1.5 after 75%).

| Stage | When | What it does |
|---|---|---|
| **Fever** | the first half | A line every 4 minutes. Nothing else |
| **Weakness** | the second half, until Final | Stamina drains every 30 s (fallback: a little noise, laboured breathing). Players within 15 m are told every 3 minutes that you look sick. Safehouses can catch a hidden bite (half chance) |
| **Final** | the last 10 minutes (at most 30% of the whole) | Stamina drains every 20 s (fallback: noise). A countdown every minute. Others within 15 m see it every 2 minutes; full chance to be caught in a safehouse |

Meds and safehouse slowing add time, so a player can drop back a stage. A player who turns
becomes the **Turned Player** world event: they roam with their gear.

*Simplified:* stamina is drained on the server (whether the player feels it is a VERIFY);
without it the stages make noise instead. "Visible to others" is a message to players nearby, not a change to how you look.

### Treatments

Bite treatment exists from the start of a season. Cure research only changes **supply, price
and odds**; the world-level breakthroughs (spread, timers, the final stand) are in
[Cure research](#cure-research-and-the-seasons-end).

| Treatment | Where | Does |
|---|---|---|
| **Antidote** | Halcyon's quartermaster (Trusted), from the lab's stock; the Red Hand's stolen doses; very rare finds | Cures |
| **Trial dose** | Halcyon's quartermaster: **Ask for the trial dose** in its talk (greyed with what it needs until it applies), or "Volunteer for the trial dose" without the talk screen: anyone bitten, free, once per life | Cures 80% of the time; otherwise the time left is halved. +3 Halcyon Fame either way. Refused to people Halcyon hunts |
| **Stabiliser** | Halcyon's quartermaster (Known), from a small stock; rare in pharmacies and hospitals | Pauses the infection for 20 minutes, once per life |
| **Declaring the bite** | Any safehouse | Once per life: time left ×1.15 (Shelter), ×1.25 (Community), ×1.4 (Settlement), ×1.5 (Town). A fresh bite is cleaned instead; one cleaned already uses nothing ([details](REPUTATION.md)) |
| Doctor wanderer, Nurse or Doctor player | On the roads (the wanderer); a Nurse or Doctor treating you with vanilla's medical items ([Treating each other](#treating-each-other)) | ×1.25 / ×1.15 of the time left, once per life each |

**Supply by research:**

| Research | Antidote stock | Stabiliser stock | Stabiliser price | Finds per pharmacy's loot (antidote / stabiliser) |
|---|---|---|---|---|
| 0% | 1 per 30 min, 2 at most | 1 per 90 min, 1 at most | ×2 | 0.4% / 1% |
| 25% | 1 per 20 min, 3 | 1 per 45 min, 2 | ×2 | 0.8% / 1.5% |
| 50% | 1 per 15 min, 4 | 1 per 45 min, 2 | ×1 | 1.2% / 2% |
| 75% | 1 per 10 min, 5 | 1 per 15 min, 5 | ×1 | 1.6% / 2.5% |

Finds are doubled behind a lock. Infected bodies (some were Halcyon couriers) have half the
pharmacy's antidote chance. Power the Lab still doubles antidote production for 2 hours.
Players Known with Halcyon hear when a dose is ready.

### Low-grade meds

Buy time, never a cure (except a lucky cut dose). **Each works once per life**, and between them
they add **30 minutes at most** (a stabiliser doesn't count). Used like the antidote: drop it and
"Take" it, or from the inventory as a consumable. Each has a guidance tip on first use.

From the inventory, the antidote and the meds only work on yourself, and only while your HUD shows
a bite (meds: once it has taken), so a healthy player can't use one up. The server still refuses a
med already taken this life; from the inventory that dose is lost (from the ground it isn't).

| Med | Where | Does | Side effect (10 min) |
|---|---|---|---|
| Expired antibiotics | Common: pharmacies, houses, bodies; every safehouse hands them out for 5 Haven Favor | +10 min; 30% nothing | Nausea: a burst of lost stamina every 90 s (fallback: a little health) |
| Veterinary antiviral | Farms and vet clinics | Pauses 20 min | Coughing fits: 12 heat every 60–90 s |
| Red Hand cut dose | The Red Hand's quartermaster, Known, cheap | Cures 50%; otherwise time left ×0.6 | Fever: 1.5% health every 30 s, never below 30% |
| Fever suppressant | Pharmacies, some shops | Nothing to the timer. Hides the symptoms for 30 min: nobody nearby sees them and a safehouse can't catch the bite | — |
| Home remedy | Brewed on a stove from alcohol and herbs | +5 min | Drunk for 5 min: sway and blur (fallback: stumbling, stamina and noise) |

*Simplified:* there's no known script hook for sway or blur, so the home remedy always uses the
fallback. The side effects use health, stamina and noise, not animations.

## Infection adapts

Code: `Director/RZ_Adaptation.c`. Small, capped changes over the season: a few more specials after
each breakthrough (×1.15 at most), one new trait per stage (runners, helmets, screamers), and
infected that come to look at a spot players never leave. The rules are in
[INFECTED.md](INFECTED.md#infection-adapts).

## The Perimeter

Code: `World/RZ_Perimeter.c`, `World/RZ_CheckpointMarkerEntity.c`. The story is in [LORE.md](LORE.md), "The Perimeter".

The edge of the playable world is the quarantine line, on any map. It's worked out from the
world's bounds, pulled in to the outermost dry land plus a 200 m sea margin (plus one step of the
coarse land sampling, so the real margin is never less), so nothing is
placed by hand. **Nothing is built along the edge** except the road checkpoints.

- **Warning zone** (300 m from the edge): a drone appears above *you* and follows you, and a
  loudspeaker says *"You are approaching the quarantine line. Turn back."* It leaves when you
  do. Never more than one drone per player, and none anywhere else. On foot it's attached to you;
  in a vehicle or a boat it's moved over you on each tick instead.
- **Kill zone** (100 m): a warning shot (a sound and a near miss), then 8 seconds later you're
  shot by a sniper you never see.
- **Sealed road checkpoints.** Up to 6 by default, 8 at most, at least 800 m apart, the widest
  roads first. The road ends in concrete blocks and a closed barrier, with 0–4 guards behind it
  who can't be hurt (bodies only while a player is within 600 m, as road posts; they go once
  nobody is within 700 m or can see them). Nobody passes in either direction. Within 150 m the loudspeaker warns you;
  closer than 60 m the guards shoot. Placed `RZ_CheckpointMarkerEntity`s win; otherwise the road
  network gives them, once at start:
  - **Roads that cross the line** (60 m inside the edge): the guards stand outside the playable area.
  - **Roads that end at the shore**, filling what's left: where the edge runs at sea all round (an
    island, as Everon and Arland are), no road crosses it, and the ways out are the quays, slipways and
    beach roads. A road counts if it's a dead end (no other road within 15 m of its end) with the
    sea within 40 m past it. The barrier stands 20 m back up the road, facing the sea, the guards
    between it and the water. One with another road within 75 m of the barrier is left out, so
    it never closes a road that goes on. On the dedicated server Everon gets 6 this way; Arland
    gets none (its few shore roads all branch off roads that go on), so its line is the drones and
    the snipers only, unless markers are placed.
  - Walking away from a barrier (after the cure, the codas, errands at the line) goes back up its road.
- **Blind spots.** The drones run a rotation; The Blind Spot mission opens a 250 m gap for
  ten minutes ([MISSIONS.md](MISSIONS.md), "Perimeter missions").
- **Sea edges.** Where the edge runs out at sea it sits at least the sea margin offshore, so
  the kill zone is water and the warning zone only reaches the beach where the land comes
  within 300 m of the line. Swimming or sailing out gets you warned and shot as on land.
- **Missions at the line** measure from where the land starts, not from the edge. The first
  time one is considered, points every 100 m along the edge (away from the corners, at most
  400) are walked inward in 10 m steps to the first dry ground, up to 600 m; points with no land
  that close are left out. A mission picks among these, preferring spots under the drones;
  one that finds nowhere on a map is never offered there (worked out once per run).
- Other missions, spawn points and quartermasters keep out of the warning zone.
- **After the cure the line stands down** ([LORE.md](LORE.md#endings)): no drone comes down,
  nobody fires a warning shot or a sniper round, at the edge or at a checkpoint. In the warning
  zone the loudspeaker reads the new order instead (*"The quarantine order for this area has been
  lifted. Remain where you are. Processing will begin at the road checkpoints."*), once per
  approach. The checkpoint barriers and their guards stay: the gates open with the wipe. The line
  isn't open either: whoever walks into the kill zone, or up to a barrier, is turned back to the
  warning zone, 40 m clear of it (a voice from past the line, no shot, no drone). The one exception
  is the Walk Out: in the epilogue's last day (processing), at the north road's barrier
  (`day2.checkpoint`) only, an unarmed person hears *"Name?"* instead and may walk out for good
  ([EPILOGUE.md](EPILOGUE.md#the-closing-work)). The Perimeter
  missions, Learning the Drones, the basement spawn scene and the perimeter tip stop with it
  (`RZ_WorldFacts.PerimeterSealed`); one running when the cure comes out is called off.

## Time and environment

Code: `Director/RZ_Environment.c`, `Puzzles/RZ_GameTime.c`, `World/RZ_PowerGrid.c`, `World/RZ_RadioStation.c`, `Director/RZ_Broadcast.c`.

Time is never skipped. The environment is read every 30 s and nudges other systems.

### The clock

Code: `Core/RZ_GameModeComponent.c`, `Puzzles/RZ_GameTime.c`.

- **Day length** (top level, 4): a whole game day takes this many real hours, set with the engine's
  day duration at start (×6: a night of about 85 real minutes, dusk about 20). The engine caps it at
  ×12, so 2 hours or more. 0 leaves the server's own clock (phase 0 measured the default as real time,
  ×1). The infected's design assumes about 4 ([INFECTED.md](INFECTED.md#the-daily-rhythm)).
- **Keep time of day** (on): the time of day is saved in `state.json` and set again at start, because
  the engine's clock goes back to the scenario's start on a restart. The day carries on where it
  stopped; nothing is skipped.
- The log says both at start: *"Clock: a game day takes 240 real minutes; the time of day is 21:40
  (saved 21:40)."* With no time and weather manager in the world, every hour reads as day.

### Night

The game clock's night is 20:30 to 05:00.

- **They hear further:** a quarter of every bit of heat also reaches the four neighbouring squares.
- **They see less:** you're harder to see in the dark, unless you carry a light ([Stealth](#stealth)).
- **Specials** are ×1.25 as common.
- **Human AI patrols less:** each holding shows one guard fewer (never under one), standing
  closer to its centre; half the hunter-squad rounds are skipped.
- Some missions only run at night ([MISSIONS.md](MISSIONS.md)).

### Weather

- **Rain** (0.3+) dampens noise: heat added ×0.7.
- **Fog** (0.4+) is the engine's own fog; infected may spawn a little closer (×0.75 of the minimum distance).
- **Storms** (the Storm event, or rain 0.6+ with wind 10 m/s+) ground Bastion's helicopter
  (no Last Helicopter contract), and wanderers shelter.
- Players hear one line when rain or fog sets in, at most every 20 minutes each.
- Fog, fog banks, rain and storms also make you harder to see ([Stealth](#stealth)).
- **Fog banks** (the Fog Bank event): one region at a time where heat added is ×0.6 and infected
  may spawn twice as close. The engine's fog is global, so it isn't touched; an optional mist
  prop is the only look.

*Simplified:* weather is read from the time and weather manager (rain and fog 0 to 1, wind in
m/s); on a map without one, only night and the Storm event apply.

### The power grid

**The station.** One power station per map, at real infrastructure (`World/RZ_PowerSite.c`): a
placed `RZ_PowerStationMarkerEntity`; else the best power site within 500 m of the twelve biggest
towns, by prefab name (a power plant, then a transformer station or substation, then a 110 kV or
40 kV transformer yard, then a lone transformer); else a factory hall or boiler house, a sawmill
or concrete plant, a warehouse or workshop within 200 m of a town (buildings only: a barrel under
`Props/Industrial` is no factory); else a real building on the edge of the biggest town. Its name
says what it is: "the power station outside Morton", "the substation in Levie", "the factory's
switch room outside Tyrone", "the switch room on the edge of Morton". A station saved while still
dark is picked again at the next start (an older pick may have been a field); one saved later
keeps its place. Only a map whose biggest town has no building at all gets a bare point.

**The control.** Never a block in a field. The map's own electrical cabinet at the station (a
110 kV control box, an electric cabinet, a large cabinet set, a breaker or utility box, outside and
not behind a lock) within 45 m carries it, as an invisible interaction spot on the face toward
the town (**Use cabinets**, needs **Interactions > Spot prefab**). Else the control prefab (a
vanilla cabinet set) stands against the station's wall, a step to the side of an outside door
and facing out; with no door, against the wall nearest the town; at a lone transformer, beside
it. It exists while a player is within 300 m.

**Restoring it** takes three steps, all in [the power panel](UI.md#the-power-panel) (one prompt,
**Open the panel**):

1. **Force the door** (loud: heat 10).
2. **Set the breakers and start the generator.** The card inside the door names three breakers to
   close in order and one burnt one to leave; the board has six. A wrong one trips them all (heat
   2; the burnt one bangs, heat 8). With the board READY, **Start the generator**: a horde is sent
   at it, and someone has to stay within 60 m for 90 s while it spins up (nobody there for 30 s
   and it dies, and the breakers trip).
3. **Throw the main switch.**

Then every town within 2.5 km is **Powered** and everyone hears it. Power gives a lights hook
(`RZ_PowerGrid.IsPoweredAt`; no lamps are actually switched on), more radio chatter (every
6 minutes instead of 10), faster radio station broadcasts, and electronics ×1.5 as likely in
loot there. But the generator hums: 10 heat a minute at the station. It **slowly fails**
over about four hours with players online (a warning at 20%). **Refuel and grease** restores
60% with a fuel item or 25% without, once per player every 10 minutes. When it fails, it's back
to step 2 with the breakers tripped. No Fame for any of it: the power is the reward.

**Power surges** (the Power Surge event): one dark town outside the region gets power for 10
minutes, with the same lights hook and electronics bonus, and no generator to keep running.

*Simplified:* the breakers are a puzzle on the screen, not switches in the world; the card is
dealt at each server start; the generator, the door and the board are told, not modelled. With
**UI > Power breaker layout** empty the control keeps its old world prompts and the breakers are
done in one go with the generator ([fallbacks](UI.md#the-power-panel)).

### The radio station

One broadcast point per map: a placed `RZ_RadioStationMarkerEntity`, else a real radio mast
near a town, else high ground near a mid-sized town. Take it over by staying at the console
for 30 s with no infected within 15 m; it's yours until you walk 150 m away. Then send one of
four preset messages to everyone, signed with your name and filled in with real places: a
horde warning, food for the hungriest safehouse, meet here, and "still here". One broadcast
every 10 minutes server-wide (5 with power). **Cosmetic and social only:** no Fame, no Favor.

### The government broadcast

A looping flavour message to everyone every 25 minutes, and in the notebook at each player's first spawn (no card: [UI.md](UI.md#notices)).
Its tone follows the season mood, the one ladder every voice reads ([WRITING.md](WRITING.md#the-season-mood);
[LORE.md](LORE.md), "The outside world: silent, watching"; its lines:
[WRITING.md](WRITING.md#the-government-broadcast)).
Its day count ("Day 14") is 11 plus the real days since the season started, so a season of
about 45 days runs from Day 11 to about Day 56; without the server's clock, a day is 8 hours
played. Notes, journals, the radio station and Bastion's orders use the same count.

## Backgrounds and personal goals

Code: `Backgrounds/RZ_Background.c`, `Backgrounds/RZ_Objectives.c`, `Backgrounds/RZ_PlayerProfile.c`.

You pick a background on our spawn screen, which replaces vanilla's deploy menu (each background is a
loadout); it shows its perk, flaw, starting gear, starting standing and whether it's locked, and
the kind of place it starts at, never where ([UI.md](UI.md#the-spawn-screen),
[Openings](#openings-how-every-life-starts)). Gear comes from the
loadout; the perk and flaw change ReforgerZ systems, so they work with any other mods.
Starting standing is Fame (+) or Infamy (−) ([REPUTATION.md](REPUTATION.md)).

| Background | Perk | Flaw | Starting standing |
|---|---|---|---|
| Nurse | Medical supplies. Treats other players in 8 s, even with a bandage. Infection takes 50% longer | Unarmed | Haven +10 |
| Mechanic | Wrench and jerrycan. Hotwires a car in 15 s and rarely fails | 25% louder | — |
| Hunter | Hunting rifle, 25% quieter | Little ammo | Bastion −10 |
| Police Officer | Sidearm | — | Bastion +20 (Known), Red Hand −25 (Watched) |
| Ex-Con | Knife | Infection kills 20% faster | Red Hand +25 (Known), Bastion −25 (Watched), Haven −10 |
| Lab Technician | Samples worth double research | Unarmed, 15% louder | Halcyon +20 (Known) |
| Radio Operator | Radio; horde warnings and intel map-wide; cracks secure channels | 20% louder | Haven +10 |
| Unemployed | +50% Fame from missions | Starts with nothing | — |
| **Soldier** (Champion with Bastion) | Service rifle and armour | — | Bastion +20, Red Hand −15 (Watched) |
| **Doctor** (Champion with Halcyon) | Infection takes 2.5× as long; treats others in 6 s | Unarmed | Halcyon +20 |
| **Raider** (Champion with the Red Hand) | Black market open from the start | — | Red Hand +40 (Trusted), Haven and Bastion −15 (Watched) |
| **Elder** (Champion with the Haven) | Trusted by the Haven from the start | 30% louder | Haven +40 (Trusted) |
| **Drifter** (10 Legacy points) | Known with every faction | No gear | +15 with all four |

The last five are unlocked through **Legacy**, which carries across lives. The server won't
assign or spawn a locked background (**UI > Lock loadouts**, on by default), whatever the client
asks for, so it holds with a gamepad or a modified client too; the spawn screen also shows it as
locked. With the lock off, or if a locked pick gets through another way, the player keeps
its gear for that life but plays as Unemployed, and is told how to unlock it.

**Personal goals.** Each life gets one goal, flavoured by background (the Hunter heads for
their cabin, the Ex-Con goes back for an old stash): reach a town 1.2–4 km away, survive a while,
deliver samples, take part in missions, or become Trusted with a faction. Completing it gives a
**Legacy** point and 10 Haven Fame, never loot, so a fresh life can't farm crates. A player's first life that lasts 10 minutes is always
**Find the Haven**: reach a safehouse ([New-player guidance](#new-player-guidance)), unless its
opening runs the whole chain, whose walk takes you where your background belongs.

## Factions in the background

How the four factions hold the map, move supplies and change over a session. What they
offer players, and how morale, needs, truces and feuds work, is in [REPUTATION.md](REPUTATION.md).

### Territory

Code: `Factions/RZ_Territory.c`.

- **Most of the map is abandoned.** Expansion stops once about a fifth of all settlements
  are held (strongholds included). The rest are empty, looted or infested: that's where the
  scavenging, the missions and the danger are.
- **One Stronghold per faction**, at its quartermaster (Halcyon's at the lab). It can never be
  captured or destroyed. At worst it's **Under Pressure** for 25 minutes: while a faction's
  strength is 25 or less, or its morale 20 or less, each territory tick has a 50% chance to
  start it. No supply truck leaves, and the stronghold's Champion shop tier is locked. It ends
  on its own (+5 strength, +5 morale), with a two-hour pause before it can happen again.
- **Outposts, by map size:** 1 per faction under 24 settlements (Arland), 2 under 48, else 3
  (a setting overrides it). Halcyon never has more than one: its field station. Each faction
  can grow one past its starting count.
- **Towns that make sense.** "On the roads" is read from how many towns lie within 1.6 km, so
  it works on any map:

  | Faction | Prefers |
  |---|---|
  | Bastion | towns (size 2+), hubs, names with "air", "base", "field" or "port" |
  | Red Hand | hubs (crossroads) and small places (outskirts), some infection |
  | Haven | quiet villages: small, low infection, few neighbours, away from the Red Hand |
  | Halcyon | quiet, low-infection places |

  Never: a town already held (or within 400 m of a holding), a safehouse town, the lab town,
  a town within 1.5 km of the [Long Table](#the-long-table)'s farm,
  a town at 85+ infection, or anything near the quarantine line. Not under 700 m from the
  faction's own stronghold, and rarely over 6 km.
- **The territory tick**, every 45–75 minutes while players are online (divided by the season
  speed), does at most **one**
  thing. First the floor (a faction with no outpost founds one), then each faction in random
  order by its strength:

  | Strength | Does (first that applies) |
  |---|---|
  | 60+ | win back a town it lost (after 30 min, within 6 h; the Haven and Halcyon only re-found one that is empty again); found an outpost near its holdings if under its count; at 65+ take the nearest outpost of a neighbour at least 10 weaker (any weaker during a feud), within 4 km |
  | 41–59 | under its count: found one (half the time); else 20%: drop an outpost with almost no traffic that has stood 3 hours |
  | 40 or less | 60%: drop its quietest outpost |

  The Haven and Halcyon never take towns. Nobody takes or drops an outpost a player is near,
  a faction's last outpost, a placed `RZ_OutpostMarkerEntity`, or one from a faction it has a
  truce with.
- **Traffic** is player-minutes within 400 m, plus 5 for each contract done for the faction
  within 1.5 km; it fades by a quarter every tick.
- **Faction strength** (10–90, starts at 50): +2 for the giver of a finished contract, −1 for
  a rival it hurt, −1 for a failed one (at most 6 an hour from contracts); +1 per truck
  delivered, −2 per truck lost; +3 / −5 when a town changes hands. It drifts 2 points an hour
  back toward 50.
- **Headlines only.** Every change is a headline, never a server-wide alert.
- **Bodies only near players.** Within 300 m of a holding: its guards (one fewer while its relief
  is missing, [the faults table](ERRANDS.md#the-faults-table)), two of its three **named people**
  (the cook, the sentry, the toll man...) and, at an outpost, the faction's quartermaster. Named
  people are saved and come back on every visit; they answer **Anything need doing?**
  ([ERRANDS.md](ERRANDS.md#givers)).
- **Routines:** shift change at 05:30 and 17:30, the dusk meal at 18:30, night watch from
  20:30. Players near a holding hear the routine change, and its people act it out
  ([Camp life](#camp-life)).

*Simplified:* towns are judged by size, infection, name and neighbours, not real roads or
airfields. A town "taken" changes hands as data while nobody is there: no battle to watch.

Bastion and the Red Hand also put temporary **road posts** on the roads near their holdings, and
their camps warn off the people they watch: [REPUTATION.md](REPUTATION.md#checkpoints-roadblocks-and-camps).

### People's looks

Code: `Factions/RZ_Looks.c`; prefabs: `Prefabs/ReforgerZ/People/<faction>/`, written by
`tools/people_looks.py`. A faction's people don't all wear the same clothes. Every slot a faction
person spawns from has an optional **Looks** list next to it: variant prefabs of that person in
other clothes. Each body picks one from a stable seed, so the same person always wears the same
look and the next one along wears another. An empty list keeps the slot's own prefab.

| Who | Seed (the same seed, the same look) |
|---|---|
| A holding's guards, named occupants and outpost quartermaster | the holding and their place in the row |
| A safehouse's occupants and guards; its quartermaster | the house and the row; the house's spot |
| Camp life's extras (guards and workers who sleep and wake) | the place and how many are already out |
| A road post's crew and speaker | the post |
| A supply truck's crew, a hunter squad, a stronghold quartermaster, a mission's faction group (the finale's stand, Authors, the Red Hand vans, Bastion's gate squad) | the spot it spawns at |
| A council head or outpost leader | the seat and the generation: a successor wears another look |

Stronghold leaders are one person each and keep their one prefab. Faces aren't set by the looks:
the vanilla identity picks one from the faction's identity at each spawn, so a face can change
between visits.

Each faction keeps one identity in its looks ([LORE.md](LORE.md), the Workshop mods in
[MOD_STACK.md](MOD_STACK.md#the-camps-look)):

| Faction | Looks |
|---|---|
| The Haven | farm and work clothes: overalls and dungarees, a mechanic's suit, raincoats and fisherman's trousers, hard hats, knitted and trucker caps; a carer in a medical mask and gloves |
| Task Force Bastion | worn US kit: BDUs sleeves up or down, dark BDUs and M65 trousers, the night-camo jacket, the raincoat, patrol, woodland and boonie caps, the M1 and PASGT helmets with scrim or goggles |
| The Red Hand | dirty scavenged clothes: bandit jackets, tracksuits, sweatshirts, cargo trousers and jeans, balaclavas, baseball caps, wool and leather gloves, a black PASGT |
| Halcyon | hazmat suits with masks (the beaks, [LORE.md](LORE.md)), lab staff in grey and black, paramedic kit, medical masks and nitrile gloves; its guards in dark kit with gas masks or hazmat |

A look changes clothes only: the person's weapon, actions and components are the slot's own. When
a look swaps a garment its parent packs starting items into (a guard's magazines in his trousers),
the items go into the new garment, or into the pack or vest when the new one covers the legs.

### Camp life

Code: `Factions/RZ_CampLife.c`; settings: **Territory > Camp life**. Holdings, Haven safehouses,
Halcyon's field stations and road posts look lived in, and their people do things. It's
scenery and behaviour only: nobody new decides who is there, and nothing is saved. Each faction
moves into the building in its town that suits what it does and lays its camp out in the yard
and at the road entries; with no building that fits, it lays it out on open ground facing the road.

**What stands there.** Each faction has a **kit** for each kind of place: a **plan**, the camp
drawn whole, of compositions built from vanilla outdoor props
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits); the plans are drawn in
`tools/camp_kit/camp_plans.html`):

| Place | Built up with |
|---|---|
| Bastion post | a sandbag nest and sandbag runs at the approach, tents, the roster board, a flagless pole, a fire pit, a table, a cook pot and a woodpile, a shaving stand, ammo-crate seats |
| Red Hand camp | the toll barrier, a scrappy nest, a scrap barricade and the red hand painted on corrugated iron at the approach, an oil-drum fire, a card table, a stolen sofa, a cook pot and a woodpile, tents and a scrap shelter, bags and bottles |
| Haven safehouse and outpost | a table outside, a cook pot and a woodpile, a water barrel and a wash stand, a washing line, a garden bed, the bear stencil on a door board; an outpost adds a candle memorial with names, a tent, a children's corner, a fence and a vegetable stall |
| Halcyon field station | a field table, the ward tent (the plain FIA tent: the medical one carries a heal station), a decon shower screened with tin sheets, a cold store on a pallet, a generator, an antenna, a bench of notebooks, the plague-doctor stencil board (the lab grounds too, when Halcyon's stronghold sits at the lab: a sorting table under a net, two generators, the radio table) |
| Strongholds | the faction's set, more of it (20 pieces against an outpost's 13): more tents, bags and bottles, a drying line, a latrine screen, and each faction's extras (Bastion's duty board, the Red Hand's trophy wall and kennel, the Haven's fire pit and chairs round a crate, Halcyon's sorting table and radio table) |
| Road posts | 4 pieces on the verges either side of the barrier: a fire barrel, a nest or a card table, a tarp, a barricade |
| Every place with a quartermaster | his **counter**: a table between him and his customers (Bastion's stores window, the Red Hand fence's table, the Haven's stores table, Halcyon's issue table) |

Every piece stands outdoors, so every prop in one is an outdoor prop: no cabinets, shelves,
sinks or hospital furniture, only the tables and chairs people dragged out to use. The kits
(eleven: each faction's stronghold and outpost, the Haven's safehouse, Bastion's and the Red
Hand's road posts) are in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#camp-life-the-compositions-and-kits).

**The building it moves into** (setting **Buildings**, on). A holding, a safehouse or a field
station reads the buildings round its spot with the town code's own building data (the building
query, each building's box, and the prefab-name words of door marks and landmarks:
`RZ_DoorSlots.KindOf`, `RZ_LandmarkFinder.Matches`) and picks one by kind, then size, then
distance, then roads. Road posts don't (they stand beside their barrier).

| Faction | Stronghold | Outpost |
|---|---|---|
| Bastion | the school, town hall, police station or barracks (another public building will do; never the church), the largest, on a road | a farm or a petrol station, else a house, on a road |
| Red Hand | a garage or petrol station (a farm or a factory will do; never the church or a clinic), where roads meet | the same, or a shop or café by the road |
| Haven | the church (a school or a farm will do, then a house) | the same; a safehouse is its own house, the nearest one |
| Halcyon | the lab or a clinic (another public building will do), on a road | a house: the field station |

- **Which building.** Within 60 m of a stronghold's spot (its quartermaster), 90 m of an outpost's
  (its town's middle), 30 m of a safehouse's. At least 100 m² for a stronghold, 45 m² for an
  outpost or a field station, 30 m² for a safehouse; never a side over 70 m (a hangar, a factory
  hall), under 5 m, or over four times as long as it's wide (a pier, a wall), a ruin, a pier, jetty,
  dock, bridge, pontoon or wharf, or a building that is scenery. The best six by kind, size (the largest for a
  stronghold, a middling one elsewhere) and distance are then scored by the roads: Bastion and
  Halcyon's lab want a road within 25 m of the building, the Red Hand roads meeting (each further
  road within 45 m counts). Any building on the shortlist will do when none of the faction's kind
  is there; with none of the size in reach, any building of 25 m² up to half as far again (a camp by
  a house beats a camp in a field); with none at all, open ground (below). It's read once a run and kept; the log says which
  (`Camp life: ... moves into ...`).
- **Its shape.** The **front** is the wall with the outside door nearest the way in (the
  building's door children), else the side facing it. The **road entries** are where roads cross
  a ring round the building (50 m for a stronghold, 40 m for an outpost, 28 m for a safehouse), at
  most five; the **gate** is the one toward the nearest other town (the main road in), the others
  are **closed**. The **yard** is behind the building (the Red Hand's is its forecourt).
- **Its people** come out in the yard: the territory spawns a holding's guards, named people and
  an outpost's quartermaster there (a stronghold's quartermaster stays where it's placed, and turns to serve at its counter). Named
  people stay within 20 m of the yard, more beside a long building. An outpost's **leader** sits in
  the yard too: registered there when the outpost appears or changes hands (a placed marker's seat
  stays where it's placed), and kept clear of pieces like any seat.
- **Lived-in ground.** What keeps off lived-in places (scenes and traces, shelling, lair moves,
  the doors the infected break) measures a camp from its yard out to its farthest piece (the
  building, its entry ring and the pieces standing), not from the town's middle; a camp whose
  building isn't read yet counts 120 m round its middle (60 m for a safehouse). Road posts count
  as lived in while they stand.
- **The plan, laid down whole.** A kit is drawn as one camp (`tools/camp_kit/pieces.py`): each
  piece's spot and turn, metres from the camp's middle, its front toward the road in. The game lays
  the plan down as it's drawn, never piece by piece at random, so the camp keeps its shape: the
  working area at the heart, the sleeping area at the back, the cooking to one side, the
  perimeter at the front. Left for right by the seed, so neighbouring camps differ. In plan order
  (the most important first) until the place's piece count.
- **By a building.** Tents aren't put up (they sleep indoors). The rest go by each kit entry's
  **At** (written by `tools/camp_kit/build.py` from `pieces.py`), or Auto:

  | At | Where | Auto for | Set for |
  |---|---|---|---|
  | **Yard** | the plan's yard part as drawn, its front against the building's back wall (the Red Hand's back against its front: the forecourt), 2.5 m off it | everything but the approach pieces | |
  | **Front** | along the front wall, 2.5 m out, its back to the wall and facing out, one each side of the middle | | the Red Hand's trophy wall, the Haven's vegetable stall |
  | **Door** | beside the front door, 1.2 m out, facing out: right, left, then further along | | the bear board, the memorial, the decon shower |
  | **Gate** | on the verge at the gate, its near edge 4.5 m off the road's line, facing out along the road; the next on the other verge | watch and lean pieces on the approach: the nest, the toll barrier | the red hand sheet, the plague board |
  | **Closed** | on the verge at the other entries, one entry after another | the other approach pieces: the barricades | |
  | **Counter** | in front of the place's quartermaster (below) | | the counters |

  The yard is tried as drawn and shifted out (2.5 and 5 m) and sideways (4 m); the way most of it
  fits wins, its first pieces counting most. If under 60% fits behind the building, its sides
  (then its front) are tried too. With no road entries, the gate and closed pieces stand 9 m out
  in front.
- **The front faces land.** The camp's front (its approach) is the way to the nearest road, else
  the next town, but never toward water or a drop: if sea level or ground 8 m lower lies within
  80 m that way (or 25° either side), the front turns to the nearest way that has neither (a coast
  road can run between the camp and the sea). A nest, a barrier, a barricade or a sign is left out
  if it would look at water or a drop within 50 m, wherever it stands; with no road entries by a
  building they stand off its first side that faces land.
- **On open ground** (no building near, or buildings are off): the plan's middle at the place, its
  front landward, then pulled back 4 or 8 m, pushed on 4 or 8 m, shifted 4 m sideways or turned
  12°, whichever fits most. **At a road post** the plan's front runs along the road and
  its pieces stand on the verges either side of the barrier (shifted 3 m along it if that fits better).
- **Every piece fits, its whole footprint.** Each kit entry carries its piece's footprint (its
  props' boxes and a person round each seat, from `pieces.py`). A piece stands only where all of it
  is on dry, flat ground (the ground under its corners, edges and middle rises at most about 12°
  across it), away from the line, every point 4 m from a road's centre line, outside every
  building's box by 0.8 m (so nothing large ever stands in a building or through its wall),
  0.8 m from the other pieces, 2 m from a quartermaster, the medic or the barrier, 3.5 m from a
  leader's seat, 1.5 m from the front door's step and 1 m from any other system's scenery (Traces'
  scenes and boards). Then **nothing may stand in it**: flat traces along its four sides and
  across both diagonals at knee and chest height must meet no wall, fence, tree, car or prop.
  One that doesn't fit is **nudged** up to 1.6 m (3 m by the road or the front wall) and checked
  again, else **left out**: it is never moved somewhere else. The log says how much of each plan
  fit, where, and how many were left out facing water (`Camp life: ... of ... planned pieces fit,
  laid out ...`). A place within 60 m of
  one already dressed (the Haven's safehouse at its own stronghold) isn't dressed twice.
- **The quartermaster's counter.** Every quartermaster at the place (an outpost's or a safehouse's,
  and the faction's own when its stronghold is here) turns to serve: of sixteen ways, the most open
  one (traced up to 5 m), toward his customers when it's about as open, never a way with a wall
  within 2 m, so he doesn't face a wall or stand into a corner. His customers come from out of
  the building when he stands by it (his back to the wall, serving into the yard), from the front
  door when he's inside it, else from the yard's middle. Then his faction's counter stands just in
  front of him across that way, if it fits: outdoors by the rules above, indoors (a kiosk, a shop,
  a hall) only with a floor under every corner and nothing in its way. The spot in front of it is
  kept clear for customers. With no counter (no room, or the place's count is reached) he still
  turns to serve.
- **Nothing to abuse.** `RZ_DressingGuard` checks every piece and its children as it spawns and
  deletes any part with an arsenal, any inventory storage, a pick-up item, a support station
  (resupply, heal, fuel, repair), supplies, fuel, a vehicle or vehicle spawner, a spawn point,
  a body (a character, an AI group or anything AI-controlled), or any ReforgerZ component or
  action, with a log line. The interaction scan and the puzzles'
  landmark finder ignore the pieces: a decor fire isn't a stove, a decor generator isn't a
  generator, a barrel, a wash stand or a shower isn't water, a radio on a table is only a
  shape, an antenna isn't a mission's landmark. The quartermaster and
  a safehouse's own services stay what they are and are never scenery.
- **Only near players.** Pieces spawn and go with the place's bodies (a player within the
  owner's visible radius: 300 m for holdings). The layout is worked out at once, but the pieces
  spawn **4 per server tick** for all places together (a 20-piece stronghold is up in about 25 s
  at the 5 s tick); when a place's last piece is up, its people are given their jobs again. At
  most **90** of camp life's own pieces stand or wait to spawn on the server (Traces' props,
  boards and sound looks are scenery too but don't count toward it).

**What people do.** The bodies are the ones the built systems spawn (guards, named people,
safehouse occupants, road-post crews); camp life gives them **jobs**. Only at the **3 places
nearest players** (Live places); elsewhere they stand.

| Job | How (vanilla AI) |
|---|---|
| Sentry | the nest's watch point (a smart action), else a spot toward the approach (vanilla's observation post waypoint where an observation-post smart action stands within 6 m, otherwise the loiter waypoint: a smart-action waypoint is never given to a group without its seat); by a building, the **lookout** at a front corner (the one nearer the gate first, a second watcher the other; a safehouse's at its door) |
| Patrol | a loop of four points 40–80 m out (patrol waypoints in a cycle) |
| Sit | a free seat at the fire or a table (a sit smart action) |
| Work, lean | a free loiter or lean point at the line, the garden, the cold box, the barrel, the barrier |
| Gather | the roster board, else the centre's table or fire: everyone stands or sits there |
| Wash (mornings, 05:30–07:00) | the wash stand, the shaving stand or the decon shower |
| Chop (by day) | the chopping block at the woodpile |
| Cook (17:30–18:30, before the meal) | the cook pot |
| Stroll, courier | a walk out and back: to the road (by a building, the gate), or 60–90 m to the edge |
| Off shift | to the tents (by a building, in at the front door); with nobody within 150 m of the place and nobody who could see them they **sleep**: despawned as data, back in the morning (with nobody that near or in sight of the yard), out of the same tent or door |

A piece's smart actions are found by tag (`RZ_Sit`, `RZ_Work`, `RZ_Watch`, `RZ_Gather`, `RZ_Lean`,
`RZ_Cook`, `RZ_Wash`, `RZ_Chop`), and each person takes a seat of their own: the waypoint is given
that sentinel itself, and only once it's found on the spawned piece (a waypoint pointing at
nothing stops vanilla's waypoint tree with "Waypoint is not set properly"). A job's waypoints come
off the group before its piece goes, and anyone whose piece is gone loses the job (the next
shuffle gives another). A chore with no piece of its own (no cook pot in the kit) falls back to a seat, a work or a lean
point. Without a free seat, the smart-action waypoint or the piece, they loiter beside it. Every
spot someone loiters or keeps watch at is kept out of the place's building and **0.4 m off every
wall** (traced as the pieces are), so a vanilla lean there doesn't put a head in the wall. A lone character (an occupant, a
single guard) is put in an empty AI group of its own faction so it can take waypoints; without
the group prefab it stands.

| Faction | By day | Moments | Night |
|---|---|---|---|
| Bastion | the nest, two on the loop (by a building, the lookout between them), one at the board, one chopping wood, one at the fire, the rest at work | **05:30 and 17:30**: the watch changes hands (someone else takes the nest and the loop); one shaves in the morning, one cooks from 17:30. **20:30, the roll**: everyone at the board for 3 minutes, the routine line read out | the nest and one at the fire; under the season's **curfew**, the nest and the lookout, nobody at the fire; the rest asleep |
| Red Hand | two at the barrier taking the toll (at a stronghold's building, then the lookout), one strolling to the road and back, one chopping wood, the rest at the fire, the table or the sofa | one cooks from 17:30 (nobody washes) | one lookout, the rest drinking at the fire |
| Haven | the lookout at the door, one chopping wood, chores (the line, the garden, the barrel, the stall) | one washes in the morning; one cooks from 17:30; **18:30, the dusk meal**: everyone at the table but the lookout; a name is read (the routine line) | the lookout; the rest inside (asleep) |
| Halcyon | a guard pair at the door, researchers at the tables, work at the cold box and the racks | one through the decon shower in the morning; the **courier** walks out to the edge and back every 20 minutes | one guard, one at the cold box; the rest asleep |
| Road posts | the crew leaning at the barrier | — | the same |

- **Nobody just stands.** Whoever finds no free seat or work point **walks the camp**: a loop of
  three pieces (named people's near home), standing a while at each (loiter waypoints in a cycle).
  Only with fewer than two pieces up do they stand about near home.
- **Every few minutes** (Shuffle minutes, 3) anyone idle goes back to work (back from an errand,
  a fight, or a talk) and a third of the seated and working (at least one) move on: from a seat to
  work, from work to a seat, or round the camp when nothing is free.
- **The jobs log.** At each phase (and once a place's pieces are all up) one line says who is on
  which job and how: at a seat, walking the camp, standing at a spot, on the move, or standing
  still with no AI or group (`Camp life jobs: ...`).
- **Combat** is the AI's own danger behaviour; hostility is unchanged.
- **Talking** to a named person or an occupant makes the nearest one stop their job for 45 s.
- **Named people** stay within 20 m of the centre (they may cook, wash and chop) and never patrol, keep watch, run errands or
  sleep, so they can always be found and talked to. **Quartermasters, leaders and the medic**
  never get a job.
- **Extras:** each live place may add **1** more person by day (its guard prefab, at most 6 on the
  server), coming out of a tent (or the building's door) while nobody is within 60 m, but only
  while the place has fewer people than it has **in play**: Bastion's stronghold 12, its outposts
  6; the Red Hand's stronghold 10, a crew camp 4; the Haven's stronghold 8, an outpost 5, a
  safehouse 3; Halcyon's lab 6, a field station 2. They join the routine like anyone else, are the
  first pick for the courier's walk, and are removed with the place.
- **Routine lines** are the territory's notices at the phase change, not subtitles yet.

*Simplified:* a guard prefab that is an AI group gets one job for the whole group. Sleep is a
despawn, not an animation. Props spawned at run time may not cut the navmesh (the VERIFY list).
By a building: the lookout stands on the ground at a corner, not on the roof or in the bell
tower; a closed entry is a barricade beside the road, not across it (the road stays open for the
supply trucks and players' cars); the building's rooms aren't dressed inside (its stores are the
quartermaster at his counter, its mess the meal at the yard's table, its barracks the sleepers); a
piece that doesn't fit leaves a gap in the plan rather than moving; the yard is laid against one
side of the building's box, so a building of odd shape (an L, a courtyard) gets a plainer yard;
the quartermaster is turned, not walked to a serving window, and a window's glass may read as a
wall; the grass isn't cut under a camp (no vanilla prefab clears clutter at run time: the resource
index has none), so the only ground cover is Bastion's duckboards; there are no
graves, stockade or dogs; and no searchlight (no vanilla light prop is checked as safe yet, and
nothing sweeps one). The people in play stay within the guard, occupant and extras caps, so a
stronghold seldom reaches its count. Kinds are read from prefab names, so a map whose buildings
are named otherwise gets the largest building that fits.

### Supply trucks

Code: `Factions/RZ_SupplyLines.c`.

- **A real truck does the resupplying.** Outposts and Haven safehouses only restock when a
  real vehicle with an AI crew drives in from the faction's Stronghold and parks within 50 m.
  At most one truck per faction is on the road. Places within 300 m of their stronghold
  never need one.
- **Departures** every 60–90 minutes, divided by the season speed (the first 10–40 minutes after a start), only while
  players are online and not while the stronghold is Under Pressure. The truck goes where it's
  needed most: a short place first, then the least stock, then the longest wait.
- **Boarding:** the truck waits in its yard until its crew group has spawned every member (they
  appear over several frames). Then the crew within 30 m of it get in, the driver first; nobody
  is pulled in from further away. It goes once they're all aboard, or after 90 s with at least a
  driver; anyone not aboard stays in the group and follows on foot, and is removed with the
  truck. With nobody to drive it, it waits on (the log says so once) until it's late. A crew
  killed in the yard counts as robbed.
- **Stuck:** a truck that hasn't moved 8 m in 90 s, with no player within 1 km, is quietly
  moved 80–150 m on. With a player near, it stays stuck.
- **Failure:** destroyed, robbed (its crew killed) or late. The destination is then **short**
  until the next delivery: about 30% of its shop's items are out, prices are +25% to +50%,
  half the time the Champion tier is locked, and an outpost's stock drops to 0. Players within
  600 m are told.
- **Destination lost:** if the outpost changes hands or is abandoned while a truck is on its
  way, the run is called off: no delivery, no reward, no shortage, and the truck is removed like
  a finished one. The outpost's stock and shortage go with its old owner. The roads hear about
  it: a headline and a rumour ("Bastion's truck for Morton turned round on the road"). A mission
  waiting for that truck, or for one asked for and not yet gone, is called off
  (`TRUCK_CALLED_OFF`, [World facts](#world-facts)).
- **Stock:** a delivery fills an outpost to 8 units; it uses one every 45 minutes. Bastion's
  Requisition draws one unit per item. **Haven trucks carry food:** +6 at a safehouse.
- **Players and trucks.** Ride along within 150 m for 40% of the trip and see it arrive: +4
  Fame and +5 Favor with the owner. Its crew dead, drive it in yourself: +8 Fame. Neither pays
  anyone who fired within 150 m of the truck while its crew lived with no infected about, or
  whose shot hit the crew from any range (seen by retaliation, when it's on), nor anyone in a
  shooter's group. Or drive it
  into a rival's stronghold: +6 Fame with the Red Hand, who fence the cargo (with the receiver, for
  a Red Hand truck), and +10 Infamy with the owner. A truck a contract watches (Convoy Escort,
  Hijack the Convoy) pays none of this Fame or Favor: the contract pays ([MISSIONS.md](MISSIONS.md)). Killing the crew counts as shooting the faction's people ([REPUTATION.md](REPUTATION.md), "What makes AI shoot you").
- **No traces.** A finished truck stays parked while anyone is within 300 m (up to 15
  minutes), then it and its crew are removed. A robbed truck can be taken for 20 minutes.

*Simplified:* trucks drive straight to one waypoint (the AI picks the road) and don't drive
back. A restart takes a truck off the road with no shortage. Without a faction's truck and
crew prefabs, or with territory off, that faction's places count as always supplied.

### Faction life

Code: `Factions/RZ_FactionLife.c`, `Factions/RZ_FactionVoice.c`.

Capped, background and cheap. Needs, morale, truces and feuds are in
[REPUTATION.md](REPUTATION.md), "Morale, needs, truces and feuds"; they feed the following.

- **Radio chatter** with real information, every 10 minutes, one faction at a time, on its
  open channel: the next truck leaving, where a truck is heading, a short place, a shift or
  night watch, the Red Hand's toll hours, Halcyon's samples wanted. The Haven's channel reaches
  everyone with a radio; the others reach players Known with that faction, Radio Operators,
  and anyone with a radio near a running generator.
- **Secure channels**: what Bastion, the Red Hand and Halcyon say where nobody's meant to hear
  (`RZ_FactionLife.GetSecureTraffic`). Radio Operators crack them ([World interactions](#world-interactions)).
- **Life events** in the headlines, one every 35 minutes or so: funerals for leaders and truck
  crews, Bastion promotions, recruiting in a strong faction's town, a researcher walking out of
  Halcyon when its morale is low.
- **Remembered moments**: the night the lab nearly fell, the convoy that burned, the day
  Bastion lost a town, the day the lab got the antidote right (50%). The last 8 are saved. Leaders and named people bring
  one up a quarter of the time ("Not since the night the lab nearly fell.").
- **Rumours**: a leader's news and a wanderer's talk include gossip about another faction
  (rivals twice as often). About a rival it's biased or false 35% of the time; otherwise 10%.
  A false rumour inverts strength, invents a lost town, names the wrong truck destination or
  invents a need.

### The Long Table

Code: `World/RZ_LongTable.c`; settings: **World > Long Table**. One small cannibal family in one
farmhouse, some seasons. Who they are is in [LORE.md](LORE.md#the-long-table); how players deal
with them (no reputation tree, the dealings, the guest rule, shooting them, what each faction does
when they're found out) is in [REPUTATION.md](REPUTATION.md#the-long-table); their two
questlines are in [QUESTLINES.md](QUESTLINES.md#the-long-table) and [MISSIONS.md](MISSIONS.md#questlines).

- **Not a faction.** No territory, strength, morale, truck, leader seat or stronghold, and never a
  stronghold. One house a season at most: **Chance** (0.75) is rolled once, at the season's first
  start (a wipe rolls again). Off, there's no house, nothing is taken and its questlines never start.
- **Where.** The small settlement (size 1) with the fewest towns within 1.6 km, infection under 40,
  at least 2 km from every holding, leader seat, quartermaster, safehouse and the lab, off the
  quarantine line, and not a town a running mission or story is using. Small maps relax it a step
  at a time (1.5 km and infection under 50, then 1 km, then 700 m and any size, then 500 m and 400 m for maps as small as Arland, size 2 at most); if nothing fits,
  there's no house that season. The house is the
  farthest farm building there (barn, farm, mill, stable, granary, cowshed or silo, by prefab
  name), else the building farthest out, else a point on the edge. Saved. Territory never founds or
  takes an outpost within 1.5 km of it, and world events that want a town nobody lives in leave
  its town alone.
- **Data.** One record: 4–7 people by role (the mother, the host, the quiet son, the boy, and up to
  three others; names seeded), **hunger** (0–100, starts at 45, +6 an hour of play × season speed)
  and **suspicion** (0–100, starts at 10). Ticked every 10 minutes of play. Saved with what it took
  last, its tells on the road and what it remembers of each character.
- **Bodies only near players.** Within 300 m (Puzzles > Civilian): the host and one or two others
  outside, the mother at the door, the boy half the time. While a questline's stage mission is at
  the farm, it has the farm: the house's own people stand aside. A body that's a character and is
  killed is one of them gone.
- **The host on the road.** Every 30 s, a player within 1.5 km of the farm (not at it, not in a
  vehicle, nobody else within 150 m) has a 1 in 5 chance to meet him, once a session: he steps out
  35 m ahead with a flask, true road news and an invitation, and goes home after five minutes or
  when they've walked 350 m on. Never to a character who walked away from him or crossed the house.
- **What it takes, capped.** Every 3 hours of play (÷ season speed; twice as long once its buyer
  is gone or it was paid to move), if hunger is over 50: a wanderer as data (no body, no player
  within 600 m, not the Kid, not bitten, held or escorted) whose straight route passes within
  1.5 km of the farm, at most one a real day; else someone who walked out of a hungry safehouse
  alone in the last six hours (upkeep's leavers who found no other house). Never from a safehouse
  or a holding, never a player. H2's exile walked to the crew (`door.exile_sold`) is one more,
  spoken of as "the man the Haven put out for taking bread" (only the flag crosses, not his name;
  no satchel, and C1 doesn't follow his road). Each lowers hunger by
  40, raises suspicion by 10 and leaves its tells: *"never arrived"* in the news (a quiet headline;
  Haven-Known players are told; not a death, though the roster replaces the wanderer a real day
  later as it does the dead), a boot on the verge where they were last seen (a note prop while a
  player is within 150 m, for two real days), and for a courier or a trader (else 30% of the time)
  their satchel at a Red Hand fence (the Red Hand's open channel laughs about it; a rumour).
- **Suspicion** also rises with tells found (+5 a boot, +2 the boots by the door at a first meal,
  +15 the guest rule), +30 once H1 reads the List out (`list.read`), and with other questlines'
  flag `table.suspicion_add`. Wanderers talk about
  anyone who never arrived in the last three real days (half the time); at 30 the rumours start
  too (*"There's smoke past Levie most evenings. Nobody lives out there. Nobody should."*), and
  someone who hears either from a wanderer, or finds a boot, then starts
  [C1](QUESTLINES.md#c1-the-house-that-went-quiet). Past 80 with neither questline running, the
  house moves. At 100 it's found out.
- **Found out** when a questline's proof reaches a faction, at suspicion 100, or by R2's
  `table.bastion_knows` (to Bastion) or `cargo.told_haven` with the farm as the buyer (to the
  Haven): a headline, each faction's reaction
  ([REPUTATION.md](REPUTATION.md#the-long-table)), and no more takings. If nobody ends it within a
  real day (C1's last stage holds that clock), Bastion burns it while Bastion is around (its
  stronghold stands): Missions > Explosion at the farm if a player is within 600 m, as data if not.
  Otherwise it moves, or scatters if it already moved.
- **Moving**, once a season: paid to go, past 80 suspicion with nobody after them, or its town
  overrun (85 infection). Quiet for a real day (no bodies, no takings), then the next place that
  fits 2–5 km away (or the same one if none), with suspicion 20.
- **The fight** (the guest rule's, or a questline's): the family's men as one AI group (Missions >
  Events > Rival group, else Missions > Survivor group) hostile only to the players it's set on, by
  the Haven's bit as Rival Survivors are. Each death is one of them, the host last. All of them
  down: the mother is gone by morning. The players gone for a minute: the group goes, the dead stay
  dead. Without the prefab there's no fight (the guest rule is a narrow escape).
- **Broken up**: without the mother, or with fewer than two grown-ups, or by a questline's ending,
  it's gone for the season. Burnt: its town counts as Burned (town memory) and loses 15 infection.
  The boy, if he got out, becomes the Kid wanderer (or one more at the nearest safehouse if a Kid is
  already on the roads); the host may become a Scavenger wanderer.
- **World flags** (season-wide, for any content): `table.exists`, `table.town` (its settlement's
  index + 1; 0 none), `table.suspicion`; and, written only once true, `table.exposed`,
  `table.exposed_to` (the faction, -1 everyone), `table.broken`, `table.burnt`, `table.moved`;
  `table.supplier_cut` once the Red Hand purges its buyer. What it reads from other lines is in
  [QUESTLINES.md](QUESTLINES.md#crossings). The Haven's The List finds its house through the code
  (`MouthsFarm`, `LivesAt`): it was the farm the mouths were sent to.
- **Next season** is wiped like everything else, and rolls again.

*Simplified:* the takings and the family are data; a taken wanderer just isn't on the roads any
more, and routes are straight lines. The family's people stand where they're spawned. The jar is
any food item in the loot pool.

## Haven safehouses

Code: `World/RZ_Safehouses.c`. Who the Haven are is in [LORE.md](LORE.md); house rules,
exile and what they cost you are in [REPUTATION.md](REPUTATION.md), "Haven house rules and exile".

A safehouse is a single Haven-held spot in an otherwise abandoned town. A rescued survivor
isn't kept as an AI body: they become **+1 population** at the nearest safehouse.

- **Where.** At the first start of a season, up to 3 safehouses (one per 8 settlements, at
  least 1): the first at the Haven quartermaster, the rest in quiet, low-infection villages at
  least 1.2 km apart, away from the lab and the quarantine line. Saved. Placing any
  `RZ_SafehouseMarkerEntity` turns the picking off.
- **Few bodies.** While a player is within 300 m, a safehouse shows 3–6 occupants (never
  more than its population), its guards, its medic and its own quartermaster. A new tier's
  people (more guards, the medic) come once nobody is within 300 m, so nobody sees the house
  empty and fill again.
- **Population tiers** (cap 30; past it, new arrivals go to the next safehouse):

  | Population | Becomes | Effects |
  |---|---|---|
  | 0–4 | Shelter | Shop sells Known items only; 3 occupants |
  | 5–11 | Community | Trusted stock, one guard, 4 occupants |
  | 12–19 | Settlement | A medic (heals for Favor) and a radio (missions within 3 km); 5 occupants |
  | 20–30 | Town | Champion stock, two guards, 6 occupants |

- **Upkeep.** Every 3 hours of real time with players online (divided by the season speed),
  a safehouse eats one food per 5 people. Players hand food items to the quartermaster (2 Haven Favor each). Short of
  food, 1–3 people leave, to a safehouse that can feed them or onto the roads (one may become
  a named wanderer). Upkeep never takes a safehouse below 2 people.
- **Food in and out.** Haven trucks bring 6. Errands bring 2 at a time (The Pot, Water for
  {house}; [ERRANDS.md](ERRANDS.md)). Bastion's Requisition contract takes 4; so do some leader
  jobs ([MISSIONS.md](MISSIONS.md)).
- **Named people.** Each safehouse has four (a cook, a lookout, a storekeeper, a clerk); the first
  two wear its first two occupants and give its errands ([ERRANDS.md](ERRANDS.md#givers)).
- **Stragglers.** A fed safehouse under 12 people takes in one person every 4 hours with
  players online (divided by the season speed), so a house reaches Settlement in about four
  days of play even if nobody rescues anyone. Rescues are the fast way.
- **Rescues add population.** Survivors Trapped +3 (+4 with a ★ Nurse or Doctor), Rescue the
  Kids +3, escorting the Kid wanderer +1, and several unarmed and leader jobs. About 1 in 10
  rescued survivors won't stay behind walls and becomes a named wanderer instead.
- **Bites at every safehouse.** Any safehouse takes a bite declaration and hands out expired
  antibiotics for 5 Haven Favor ("Ask for something for the fever"); the bigger the house, the
  more declaring slows the bite ([Treatments](#treatments)). House rules are in
  [REPUTATION.md](REPUTATION.md), "Haven house rules and exile".
- **A notice board** at every safehouse (`World/RZ_NoticeBoard.c`): short notes players leave
  for each other between sessions. Cosmetic and social only.
  - **Read** it with "Read the notice board" at the safehouse's quartermaster. With **UI > Board
    menus** on (the default), reading opens the board menu: a box to write in, and ready-made notes
    a pad pins up without typing ([UI.md](UI.md#controllers)). **Write** also by typing `/note` and
    the message in chat while at a safehouse (within 60 m); "Leave a note on the board" says how.
  - The newest 8 notes per board, 160 characters at most, one note per player every 10 minutes
    (on any board), the author's name on each, and notes come down by themselves after 3 real
    days. A **note filter** (Haven settings) blanks listed words out with asterisks. Exiles can't
    write. Server admins clear a board with `/noteclear` (the nearest within 1 km) or every board
    with `/noteclear all`. Notes are saved with the safehouse and go with a season wipe.
  - *Simplified:* the filter is a plain word list, matched anywhere in the text; no way for
    players to take a note down.

## Wanderers

Code: `World/RZ_Wanderers.c`.

A small roster of named survivors travels the map: 6–10 per server (6, plus one per 6
settlements above 8). Names come from a seed; roster, positions and memories are saved.

| Role | What they do | Typical reaction to you |
|---|---|---|
| Trader | Moves between safehouses; sells goods for Haven Favor, cheaper if you've helped them, or for a piece of jewellery | Trades if you're not Hunted by the Haven |
| Doctor | Patches you up, or slows an infection (×1.25 of the time left, once per life) for Favor | Refuses the Red Hand's Trusted |
| Scavenger | Sells a rumour pointing at a puzzle mission or a stash, for Favor or one food item | Swaps a rumour for food |
| Deserter | Ex-Bastion, paranoid; tells you what Bastion is up to, and where Struck Off's man sleeps | Friendly if Bastion hates you, refuses if Bastion likes you, draws on Bastion's Champions |
| Preacher | Walks between towns; while he rests, his bells add heat there | Harmless; stops the bells for an hour if asked |
| Courier | Hands out an unarmed delivery job (letters to a safehouse) | Any non-Hunted player |
| Kid | A lost child | Follows you to a safehouse if you show the way; runs from gunfire |
| Mechanic | "Ask about cars": the nearest fixable car and what it needs, and the Car Keys job; sells a part for 12 Haven Favor | Hands over that car's keys if they like you |

- **Need a hand?** Their free third reply asks for an errand: the Doctor's Before Dark, Wanted
  Poster, Barter Chain and the lost cards ([ERRANDS.md](ERRANDS.md#the-wanderers-asks)).
- **Travel:** travel → rest (10–20 min at a safehouse, 3–8 min in a town) → somewhere new,
  0.6–3.5 km away. Routes never cross water or the quarantine line.
- **They react to the world** (every 30 s): an overrun, loud or line-side destination is
  dropped; a horde or noise nearby makes them hide; Every Ridge Calls or a storm makes them
  shelter. Rarely, an infected bites one: players within 1.5 km hear about it, the wanderer
  heads for a safehouse, and an antidote within 20 minutes saves them: a player chooses **Give
  them your antidote** (it takes the place of the Mechanic's "Ask about cars" while bitten).
  Nobody's dose is taken without asking. Otherwise they turn, and a special appears if players
  are near.
- **They react to you.** Attitude is your Haven standing (Bastion, reversed, for the Deserter)
  plus their **memory of you**: *helped me* (better prices, warnings), *robbed or shot at me*
  (they refuse and run, or shoot if armed), *saved my life* (a one-off gift of Favor).
  Standing effects are in [REPUTATION.md](REPUTATION.md).
- **Talking** gives one piece of road news. Actions: the role's special, "Share some food"
  and "Rob". Robbed (or frightened off), they run: a lone body is put into a group (vanilla
  `Group_Base`) and sent 250 m away, and goes only once nobody is near, never in front of you.
- **Death is permanent and makes the news.** A replacement shows up a real day later. Who
  killed them is read from the body where the engine records it; otherwise the player who shot
  near them in the last 30 s is blamed, but never while the infected are on them (so nobody is
  framed for a death to the infected).
- **Never arrived.** One whose road passes the [Long Table](#the-long-table)'s farm may be taken
  off it as data: no death in the news, only that they never arrived; replaced a real day later.
- **Performance.** With no player within 400 m, a wanderer is a position moving in a straight
  line on the tick. At most two bodies near each player.

- **A lost outpost.** When a faction loses an outpost (taken or abandoned), wanderers headed
  within 600 m of it pick somewhere else, and nobody heads there for 30 minutes. For an hour the
  loss is the likeliest road news ("Heard Bastion lost Morton to the Red Hand"), and the losers'
  people are in a mood: Haven wanderers open with it and keep twice as close to the safehouses;
  the Deserter gloats when Bastion loses one; Haven people talk low when the Red Hand moves in.

*Simplified:* routes are straight lines, not roads. A body without AI stands still while
someone is near and walks on unseen. Only a wanderer whose body is an armed AI group fights
back. The mood after a lost outpost is one at a time, the latest, and isn't saved.

## Leaders

Code: `Factions/RZ_Leaders.c`, `Factions/RZ_LeaderVoice.c`. Traits, loyalty, memory and how
to talk to them are in [REPUTATION.md](REPUTATION.md), "Faction leaders"; their personal jobs
are in [MISSIONS.md](MISSIONS.md), "Leader missions".

| Seat | Where |
|---|---|
| Stronghold (one per faction) | At the faction's quartermaster; Halcyon's at the lab. Without either, a town that suits the faction: never another stronghold's town, a quartermaster's town or the lab's. Never dies, never replaced |
| Haven safehouse | One council head per safehouse |
| Outpost | One per territory outpost. With territory off, one **interim leader post** per faction for Bastion, the Red Hand and Halcyon instead |

On Arland that's about 9–10 leaders; on Everon about 16–20.

- **Data first.** A leader is a saved record. A body exists only while a player is within
  200 m of their seat. At a stronghold or safehouse the quartermaster *is* the leader unless a
  leader prefab is set.
- **Off-screen deaths** (bites, raids) only with nobody within 400 m, every half hour; a new
  leader takes the chair 2 hours later. Deaths, replacements and loyalty changes are headlines,
  and leader news is also announced to everyone as it happens (territory and faction life stay quiet).
- **Personal jobs:** every two minutes, an Independent or Rogue leader with a player near and
  no job on has a 35% chance to think of one (at most one per 30 minutes per leader).

*Simplified:* interim posts (territory off) are leader seats only, with no guards or shop.
A leader's "people" who can see a rogue job are their faction's other seats within 60 m and,
for the Haven, its wanderers.

## Talking to people

Code: `Dialogue/`; the screen is `UI/RZ_TalkMenu.c`. Owned by [DIALOGUE.md](DIALOGUE.md): the
talk screen and its engine (one engine that wraps every person's built Talk handler and replies,
with what each reply needs shown before you choose and no dice), and the conversation trees on top
(**Dialogue > Trees**, on in the repo's game mode, the attribute's default off: topics, checks shown before you pick, regard and memory, the
`.rzd` text format and its tools, [the conversations](DIALOGUE.md#the-conversations)). How the screen looks is in
[UI.md](UI.md#the-talk-screen).

## Missions

Two kinds: **world events**, announced to everyone, and **contracts**, offered only to
players who qualify. There's no "accept" button: eligible players who take part get paid.
Every mission, its requirements and its status are in [MISSIONS.md](MISSIONS.md).

### Mission building blocks

Code: `Missions/RZ_MissionBases.c` (base classes), `Missions/Blocks/` (helpers a mission owns
and ticks, so they combine with any base). Limits and optional prefabs are in **Missions >
Contracts** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).

| Block | What it does | Used by |
|---|---|---|
| Hold, kill tagged, visit points, destroy, deliver, choice | The base classes: hold an area (optional waves and heat), kill what the mission spawned, stand at points, destroy a prop, carry the mission item to a drop, choose whose drop gets it. The first hand-in wins. If the item vanishes while it's in an inventory (the carrier left the server and took their character with them, or a body was cleared away), a new one is put on the ground where it was, up to 3 times, and everyone involved is told where; lost from the ground, it fails | most contracts |
| **Escort** (`RZ_Escort`) | People who walk a route or follow one player. Data until a player is within 300 m, then an AI group (it walks to move waypoints) or a standing figure. They only move while a player is within 150 m and no infected is within 25 m of them; the mission can stop and resume them. If the player they follow leaves or dies, they follow the nearest player within 20 m instead (or wait for one). An AI body that dies fails the job. At most 3 escorts at once | Refugee Column, Rescue the Pilot, Escort the Scientist, Secure the Shipment, Struck Off, Bust Out, Desertion Route, Learning the Drones, The Pit's Debtor; questline stages in The Three Weeks and The Other Two |
| **Live capture** (`RZ_LiveCapture`) | A cage trap and a special, both spawned only when a player is near. Lure it into the cage and drop the door within 4 m, or wound it to a third of its health and get within 3 m to tie it. Hold it for 30 s while it fights (heat), then a transport case (the mission item) to deliver before it wakes (12 min). A case lost with a carrier who left is put back where they were, still sedated (up to 3 times). It leaves the world without a kill, so no sample. Killing it fails. At most 2 at once | Live Capture, Private Collection; The Three Weeks (Dr Wolff) |
| **Vehicle zone** (`RZ_VehicleZone`, `RZ_ConvoyWatch`) | A specific vehicle, or any vehicle whose prefab name has a keyword (or is one of a list), driven into a zone by a player. Works with vehicles a mission spawns, fixed cars and supply trucks. The convoy watch asks the supply system for a faction's next truck to an outpost and reports whether it arrived, was robbed, destroyed, stolen or late | Recover the Bus, Car Theft, Mobile Lab, Tied Down and the Cargo questline (one van, `RZ_CargoVan`); Convoy Escort, Hijack the Convoy; questline stages in The Three Weeks and The Other Two |
| **Health cost** (`RZ_HealthCost`) | Takes a share of a player's health (the bite detector's health accessor against the hit zone's maximum), refuses below a floor, once per life per giver. The bite detector forgets its last reading afterwards, so the drop never counts as a bite | Blood Drive |
| **Step chain** (`RZ_StepChain`) | Ordered steps across towns. Only the current step is live: its infected and its clue appear within 250 m of it. A step is done when someone has stood there long enough and read its clue. A clue that disappears before it's read is put back (a few times) | Patient Zero, Struck Off (his pack), The Three Weeks, Tied Down; The Book (questline) |
| **Borrowed kit** (`Factions/RZ_Disguise.c`, `RZ_Ways.PassAs`) | A claim in a faction's kit, judged by the fixed look (`RZ_Checks.LookScore`); a mission sets the difficulty, may give its paper as the cover detail and registers its people as watchers (`AddWatcher`). Caught: the one cost ([REPUTATION.md](REPUTATION.md#borrowed-kit)) plus the stage's own outcome. Approach `TRICK` | Bust Out (the claim at the stockade and the second look); road posts and gates |

*Simplified:* escorted people without an AI body can't be hurt (only a body can die), and
move in a straight line. A caught special is carried as a case, not dragged in its cage.

### Story blocks

Code: `Missions/Story/` (`RZ_MissionStory.c`, `RZ_StoryKit.c`, `RZ_MissionReasons.c`, `RZ_Mirror.c`);
settings **Missions > Story**. The story jobs on it (Protection Racket, Lost Colleague, Crash Site,
Distress Call, Struck Off, Survivors Trapped, Missing Patrol, Missing Scout, Smuggler's Run, and the
twenty on the story job's kit) are in [MISSIONS.md](MISSIONS.md#who-gets-offered-what).

- **The story kit.** A mission may carry `m_Story` (null by default: it runs exactly as before),
  with a story key that is saved in records and never renamed. The mission forwards its tick to it.
  It holds who took part and who decides, who benefits, the world records it reads (pinned while it
  runs), and "What you've found" for the journal.
- **Who took part, who decides** (`RZ_Deciding`): took part is 60 s within 150 m, or a step done.
  Who may decide: someone it was offered to (anyone, for a world event) who took part. **With the
  decider**: the decider, their vanilla group within 150 m, and anyone who chose Stand with. A scene
  decision is a 5-second hold announced to the room (30 m) that any other eligible participant may
  object to once per job (by identity): Protection Racket's Side with the house, Distress Call's Side
  with the crew, Survivors Trapped's where they go. `RZ_StoryKit.MayDecide` checks it for an option
  that decides the job, and tells anyone else *"It's not your call. You weren't hired for this."*
  **Leave**, for you only: "Leave it to them" on a one-choice job's stand-in (its use slot) takes you
  out of the job: no pay, no blame, and you can't decide it. **Amends need clean hands**: a Press, a
  Trick, a switch or a hidden bite in a story job voids its Infamy work-off for that player (they're
  paid Fame instead, as when the budget's spent). **Offline participants**: a story job done pays
  anyone who took part (a step, or 5 minutes in the area), isn't online when it ends and was eligible
  for it then, at their next join, and only to the same life (the errands' ledger keeps it; not for a
  switch, a leaver or a leader's in-person job). It pays as online pay does, without the background
  bonus: the base x the leader's multipliers, amends through `Forgive()` with clean hands, the job's
  own rival Infamy (then no rival table) and any Infamy the decider's choice put on those with them.
  At the join the giver's tier is checked again: Hunted, or below the job's tier, and nothing is paid.
- **Who benefits** (`RZ_Beneficiaries`): 2–4 parties a result can go to. The side chosen pays those
  with the decider whom it doesn't hunt; a **switch** (the result to another side than the giver)
  pays at most the base, its Fame doesn't count toward season Fame (`RZ_RepFlag.SWITCH`), the giver
  adds Infamy and its leader doesn't count it as help; everyone else who did the work is paid half
  the Favor by the giver, with no Fame and no Infamy (*"You held it. Someone else sold it."*).
  "Nobody" pays nobody. The result is a world record (`<key>.went_to`, by the town). Every built
  choice mission (`RZ_Mission_Choice`: Crash Site, Research Data, Requisition) is built on it.
- **Truths** are the truth deck ([World memory](#world-memory)): `m_Story.Truth(subject, ...)`
  draws (the same for everyone at that subject while it's kept); a truth whose gate fails (no house
  near, no Red Hand holding, no prefab) is given no weight, and a truth about a fixed fact is never
  drawn: it reads canon. The leader who voices the job leans the deck by at most ±50% on the hidden
  truths: Greedy and Paranoid givers hide more, Merciful, Idealist and Pragmatist ones less, Ruthless
  and Veteran ones leave things out "by procedure"; a Talker's hidden truth leaks into the rumours. `Reveal` tells the finder and the
  room (30 m) and adds the line to the journal. A background read (`RZ_StoryKit.Read`) goes to that
  player only and to the journal. The errand Sleeve Down and the six story jobs use them.
- **Background reads.** Each background reads one kind of thing, the same in every story job, so
  players learn it once; a read is never the only way to a truth, and a job never writes one for a
  background it locks out:

  | Background | Reads | Never reads |
  |---|---|---|
  | Nurse, Doctor | bodies and bites: who's bitten, how long ago, what killed them | papers |
  | Mechanic | machines: engines, wrecks, black boxes, rigged locks | people |
  | Hunter | the ground: tracks, how many, how old, which way | papers |
  | Police | whether someone in a scene is lying (that they are, not what about) | numbers |
  | Ex-Con, Raider | crews: marks, prices, who's really in charge | Bastion |
  | Lab Tech | numbers: ledgers, manifests, readings | people |
  | Radio Op | channels: live or a loop; at most one secure-net line a job | bodies |
  | Elder | families: names, who's kin to whom, a house's own story | crews |
  | Soldier | Bastion: rosters, unit codes, orders | crews |
  | Drifter | road news: one extra hint from wanderers | — |
- **Signed thank-yous** (`RZ_StoryKit.BoardThanks`): a story job can leave a note on a house's board
  signed by the people it's from (the notice board's system notes, `PostSigned`): Survivors Trapped's
  family, *"Thank you for the barricade, and for taking it down. We're all here."* The Novaks.
- **Records** (`RZ_StoryKit.Record`): how a story job ended, as a world record `<key>.<what>` about its
  subject, with who did it and, for some, an epilogue line (`racket.<house>`, `colleague.fate`,
  `crash.box`, `distress.saved`, `struck.fate`, `trapped.family`).
- **Lies in a job** go to the one lie store ([Lies and witnesses](#lies-and-witnesses)): covering a
  collection (found at the next collection there), saying a colleague turned or walking a deserter to
  the Haven (found by a visit), selling a distress call to the crew (found when Tied Down frees a
  truck), a Nurse hiding a bite at a house's door (found within a season day; the life record "hid a bite" names the house, and that house's people make her checks one step harder for the rest of the life: its errands' talk and Protection Racket's persuade, `RZ_StoryKit.HouseHarder`), the commander's stove
  (found by the rumour round).
- **Season counts the story jobs keep** (season flags): Bastion's civilians sheltered
  (`bastion.sheltered`, read on the call to those who brought them), and Struck Off's men walked home
  or still out (`struck.home`, `struck.out`), read at the last roll in Bastion's coda
  ([QUESTLINES.md](QUESTLINES.md#the-majors)).
- **Leaders in a story job** (`RZ_StoryKit.Remember`, `Loyalty`): a story job may make one leader
  remember a player as helped or crossed, and move one leader's loyalty at most 5 (never a
  stronghold's, never below the seat ledger's floor of 30).
- **Failure is a branch.** A failed story job can open one **fallout errand** at its subject
  (`RZ_StoryKit.Fallout`, `RZ_Errands.Fallout`): dealt to the job's participants first for 20 minutes,
  then to anyone for 3 hours, by a giver within 3 km, paying at most 30% of the job's base. None after
  a withdrawal. Protection Racket opens The Pot at the house, Crash Site Buy It Back, Distress Call and
  Survivors Trapped The Name on the Tag ([ERRANDS.md](ERRANDS.md#fallout)). Jobs without one have a
  **fallout line** (`RZ_MissionReasons.FalloutLine`): what the world says next, told after the fail
  text. The Tier 0 audit's 27 jobs each have one, write `t0.<key>` at their town when they end (read
  into the next one's brief there, `Tier0Read`), and keep their choices to someone they were offered
  to ([MISSIONS.md](MISSIONS.md#who-gets-offered-what)).
- **Story groups** (`Missions/Story/RZ_StoryGroups.c`): the AI groups a story job adds on top of its
  base mission's own (Crash Site's recovery team): at most 4 on the server and 2 a job, spawned only
  within 400 m of a player, counted in the one body budget as story bodies, deleted when the job ends.
  No prefab, or no room: the job goes on in words.
- **Choices on a plain mission's people**: a non-puzzle mission can tag a person with choice and use
  slots (`RZ_PuzzleObjectComponent`) and get them through `HandleObjectChoice` and `HandleObjectUse`
  (Survivors Trapped's mother, the one-choice stand-in's Leave).
- **One real choice** (`Missions/Blocks/RZ_OneChoice.c`, `m_Choice`): when a one-choice job's work
  is done (its state turns to succeeded), it stays open and a stand-in person (the civilian puzzle
  prefab) appears beside it with the options as choice slots (object index 90). Picking one goes
  through the decider rule and the 5-second hold; nobody in 90 s, or no prefab, takes the default.
  The mission's `OnChosen` does what the option does, then it ends as done. A **manual** choice is
  asked by the mission mid-job (Fight Pit's round 3, Kill the Officer's aide), and may leave the job
  running. Never as a questline's stage (`AsksChoice`). Helpers: pay or blame those with the decider,
  morale through the shared budget, the hungriest house, and **where the horde drifts**
  (`RZ_HordeWhere`), one shared block. The jobs are in [MISSIONS.md](MISSIONS.md#one-choice-jobs-and-work).
- **The shared grey layer** (`Missions/Story/RZ_Grey.c`, `RZ_GreyChoice`, after each one-choice job's
  own outcome): the outcome's third beat, a **reaction** from the giver's people (the voice bank's
  `reaction` lines, neutral on which way it went: the job's own text says that), to those with the
  decider; **someone else made the call** (`else_call`) to those who took part but weren't with the
  decider; **nobody chose** (`nobody_chose`) when the default was taken; and the **callback deed** for
  those with the decider ([Deeds](#deeds)). `RZ_Outcome.Tell` puts the three beats and the system line
  together for any job. **The clever way closes the honest one:** once anyone in the run has pressed,
  tricked, switched or hidden something (the clean-hands list), the honest option of a job that also
  offers a trick or a betrayal refuses (*"Somebody here already played it crooked. The straight way's
  closed for this one."*): Fight Pit's fight it out, Kill the Officer's do the job, Convoy Escort's
  crates in the store, Missing Scout's tell the council. The clever options' discovery is the one lie
  store's (Convoy Escort's raiders, Distress Call's crew). *Simplified:* a clever choice found out
  later is found out on the decider, the lie's teller, not on everyone who stood with them.
- **Givers who lie** (`RZ_BriefLies`): twelve briefs whose giver may leave one thing out, the version
  picked once a season (`CanonPick`, the honest one among them), the omission added to the brief in
  the giver's voice (the jobs and their versions: [MISSIONS.md](MISSIONS.md#givers-who-lie)). The truth
  is open to everyone who takes part: 15 s at the scene (the job's spot) finds it, a listed background
  finds it within 150 m, and the room (30 m) hears it with them; a Talker giver leaks it into the
  rumours. **The outcome names it**, either way, to everyone who took part (*"He hadn't volunteered.
  His sister was in the book."*, or *"He really did volunteer."*). A caught lie has no reward of its
  own. **Taking it public** (Trees on, any leader of the giver's faction, `call:brief_public`): that
  faction's morale −1 (once per faction per real day, once per lie per season, inside the morale caps),
  and an outpost leader who gave it −5 loyalty (inside the seat ledger's limits); kept a week (the
  `knows:brieflie.<faction>` fact). *Simplified:* the truth changes no option of the job; with Trees off
  a caught lie can't be taken public.
- **Boards of record** (`RZ_RecordBoards`): late joiners read the season in the world, from the season
  flags each time, the same for everyone: the Haven council's pinned notes at a safehouse's *Ask how
  things are* (the house rule, the List read or buried, who was taken in from Bastion's line), Bastion's
  roster board at a Bastion road post's speaker (the call, the struck-off men home and still out), the
  toll sign's tally at a Red Hand roadblock's speaker (tolls paid this season, `tolls.paid`; scrubbed
  once the book burns), and the lab door's sheet in the journal's lab line (the first page, the
  subjects by number). *Simplified:* the lab's sheet is read in the journal, not on a prop.
- **Route assets and the 90% lock** (`Missions/Story/RZ_RouteAssets.c`): an option can carry a route
  asset for one of the finale's routes; the decider gets one, into the finale's store
  ([FINALE.md](FINALE.md#before-the-gate-pitches-and-route-assets) owns what an asset is, its limits and
  what it's worth). Story records give theirs on their own; one-choice options and a few jobs (the
  notebook to a Haven medic, the sums left with the crews) tag theirs by hand. After a player locks to
  a route, an option whose asset belongs to another route refuses them: *"You've picked your side.
  This isn't it."* Before the lock nothing is refused.
- **Work rows** (`Missions/Story/RZ_WorkRows.c`): the 68 work jobs each have a row: the reasons they
  answer, whether they write the **town tally** (per town in the season flags: how often work was
  done there and whose mark is on the wall now; read into the next work job's brief there), and a
  fallout line ([MISSIONS.md](MISSIONS.md#one-choice-jobs-and-work)).
- **Reasons and the reason lane.** When the world has posted a reason ([World memory](#world-memory)),
  the mission manager may start one registered contract that answers it (`RZ_MissionReasons`: The
  Harvest Guard and Protection Racket answer a hungry house; Checkpoint Duty and Convoy Escort a
  post's lost truck or a post with the infected on its doorstep; Retake the Town a town gone bad; Struck
  Off a dead leader; Rein In a Rogue leader or a skimmed toll; Trail of Clues and Medicine Run a bitten
  wanderer; Water Survey a house's low barrels; Lost Colleague and Cold Chain a Halcyon sensor gone
  quiet; Cold Chain and Power the Lab the cure's milestones and a station's freezers low on fuel; The
  Ledger two houses fallen out and a house the crew just collected from; Burn the Bodies a shelled town).
  **The reasons read from other systems** (`Missions/Story/RZ_ReasonPosts.c`, every 30 s, errands on or
  not, cleared when the fact stops): `house.water` and `house.dark` (a safehouse's water and fuel faults),
  `post.pressure` (a Bastion holding with a town the infected hold within 1.5 km), `leader.rogue`,
  `lab.short` and `lab.power` (a Halcyon station's sensor and fuel faults), `tribe.hungry` (a pack in its
  second hunger band or higher: Not Tonight and Lead Them Away answer it); left by a finished job:
  `racket.ran` (Protection Racket collected), `crew.skimmed` (Skim the Toll), `house.feud` (A Word to
  {house} carried with a threat), `horde.passed` (Every Ridge Calls, the Siren Test, Red Dusk), `lab.power`
  (a Power Surge); posted by the majors as an act opens: `major.orders` (Missing Patrol, Checkpoint Duty,
  Rescue the Pilot answer it), `major.every_name` (Refugee Column, Medicine Run), `major.book` (The
  Informant, Smuggler's Run, Dead Drop), `major.proof` (Research Data, Escort the Scientist, Cold Chain),
  at the faction's stronghold for a season day (`RZ_ReasonPosts.MajorAct`); its margin cards answer it
  too (×1.3, the act's line as their opening, [ERRANDS.md](ERRANDS.md#the-dealer)). At most one every
  **Reason lane gap** (30 min)
  server-wide, inside the contract cap and every check (`CanRunNow`, someone online eligible,
  claims), and only if it lands within 3 km of the reason; each reason starts one contract at most.
  Its brief opens with the reason's line (*"Levie's on half portions."*). Random picks stay.
- **Order per player** (`RZ_MissionReasons.OrderScore`): how far up a list a job goes for one
  player: near them (+2 within 1.5 km, +1 within 4 km), started by a reason (+1), from a leader who
  remembers they helped (+1). Only the order changes, never the odds. The journal and a
  quartermaster's list of contracts sort by it, highest first.
- **One weight clamp.** A contract's leader traits (±20%), its faction's needs (×0.85–1.2) and the
  majors' season weights multiply, and the product is clamped once to ×0.75–1.3.
- **Mirror offers** (`RZ_Mirror`): one class with two givers, rolled once when the job is set up
  (**Mirror chance**, 10%; never on a server with **PvE** on). The second side's eligible players
  hear 5 minutes later with a rough area twice as wide, and only from then can they take part. A
  hold mirror (The Cache) goes to the side with more eligible holders inside when the hold ends; a
  draw is half each (each side pays 50%, neither leader counts it). A carry mirror (The Armory) goes
  to whoever hands the case in, through who benefits. The losing side gets a line, never a fallout.
- **The journal's detail**: selecting a row asks the server for that row's detail (at most 1,500
  characters): what you've found and who it went to ([UI.md](UI.md#the-notebook)).
- **The story job's kit** (`Missions/Story/RZ_TalkScene.c`, `RZ_StoryJob`, on a mission as `m_Job`):
  the truth drawn for the job (2 or 3 values, "as they said" first and never under 30%, a value whose
  gate fails never drawn), its background reads at one place (once per player, to that player
  alone, as the table above), and its talk scene. The twenty jobs built on it each have 3 or more
  ways (the job's own, and at its person one that needs no gun and one that needs no standing: a
  tin or two, or a service), parties beyond the giver (often "nobody"), one or two records, a
  callback, a fallout line and the reasons they answer.
- **Mission goods** (`Missions/RZ_MissionGoods.c`): what a mission or an errand put in your hands as
  goods (a delivery's food, a post's ration or magazine paid in kind, an errand's carried item) is
  tagged with whose it is. It never counts for Trade, and food delivery, the fence, the quartermasters
  and every errand refuse it: *"These are Levie's. Take them back to Levie."* *Simplified:* the tag
  is a server-side list of the items (like the fence's bought list), forgotten at a restart (VERIFY:
  an item keeps its entity through inventory moves, a body and a car's boot).
- **Caught lying** (`RZ_StoryKit.LieFound`, from the lie store): on the life the lie was told in, that
  faction's **Paranoid** leaders won't give you their in-person jobs for 2 hours of your play
  (`sulk.<identity>.<life>.<faction>` in the season flags); on a dead life, that faction's next errand
  opens with *"The last one who said that lied"* ([ERRANDS.md](ERRANDS.md#the-dealer)).
- **Tricks in jobs** ([borrowed kit](REPUTATION.md#borrowed-kit) owns the look): a trick never works
  within 25 m of a leader's seat of the kit's faction (*"Not here. Not this close to the one in
  charge."*, `RZ_Ways.SEAT_CLEAR`); a major's scene choice that passes a trick sets the run flag
  `<key>.disguised` to the player for its later stages (a trick that lasts); and **a breaker**: in a
  story job, a participant on the sneak (hidden) within 25 m of another player passing as the giver's
  or the rival's people catches them (the look's Caught, once per player per job).
- **Tier 0's tags**: the audited jobs' choices that cost or check something show it in a tag like any
  other (Off-Book Requisition's and Rein In's Favor, The Pit's Debtor's debt, Honour Among Thieves'
  *[Talk - 3 in 5]*, Rival Survivors' *[Trade]*).
- **Talk scenes** (`RZ_TalkScene`): a scene person (the civilian puzzle prefab, object index 91)
  stands at the job's place, spawned once someone is within 80 m and the body budget has room (a
  `scene`; until then they wait); no prefab, no person, and the job goes on its own ways. It has 2 or 3 **nodes**: a line and up to four options on its choice slots,
  the last always "Leave it" (back to the first node). Options ask (a line, maybe a truth found,
  maybe the next node: anyone in the room may pick them), check (Trust, Fear, a trick in a kit, with
  the usual tags and one try per player per person), pay (tins, which need no standing, or Favor),
  offer a **service** (a background's work, worth 2 on the price table, a timed stand-by within 4 m),
  or **decide** (the decider rule and the 5-second hold). **The speaker**: the first eligible
  participant to pick an option holds the floor for 60 s after each pick, 3 minutes at most; others
  may only ask (*"... is talking to them. Ask, or wait your turn."*). During a hold its use slots read
  Talk, Object and Stand with; **an objector** cancels the hold and takes the floor for 60 s (the
  one-choice stand-in's use slots are Leave it to them, Object and Stand with too). **Lines**: a
  node's line goes to the speaker and anyone within 8 m, each on their own notice. **The fallback**:
  **Missions > Story > Numbered scenes** (off) lists the options by number under the line and labels
  the slots Option 1 to Option 4, for a play-test where the replicated labels don't show.

**The hourly summary** (`Missions/Story/RZ_StoryStats.c`, one line a real hour): Fame and Favor per
player-hour from the story jobs (and by job), how often a result was switched or given to nobody, food
spent on trades, fallout errands started and the Favor they paid.

Log lines: `Story | per player-hour: Fame N, Favor N | by job: ... | picks: ... | food on trades N |
fallout: started N, paid N Favor`, `Story: truth <key>.<subject> = <truth> (kept|new); found by <how>`, `Story: <key>
resolved by hand-in to <party> (...)`, `Story: <title> record <key> (<subject>) = <value>`, `Story:
<title> read by <player> (<backgrounds>)`, `Story: distress at <town> is <truth>` (and `colleague`,
`crash`), `Errands: fallout <card> at <grid> from <title> (cap N Favor, N participant(s) first)`, `Story: reason <key> (<subject>) started <id> '<title>'`, `Story: <title> asks (<options>)`, `Story: <title> chose <n> (by <player>)`, `Story: brief <key> leaves
something out (version N)` (or `is honest this season`), `Story: brief <key>'s truth found by <player>`,
`Story: brief <key>'s lie made public by <player>`, `Story: tally <town> (<name>) = <n>, mark <faction+1> (<key>)`, `Mirror: <title>
hinted to <faction>`, `Mirror: <title> won by <faction>` (or `by both, half each`).

### Errands

The small, personal jobs named people ask, each true right now: [ERRANDS.md](ERRANDS.md).

## Story arcs and headlines

Code: `Director/RZ_StoryArcs.c`, `Director/RZ_Headlines.c`.

The Director strings events into short storylines. Each step only fires if the previous one
really happened (a mission ran, a player heard a rumour, a truck left), so every server lives
through different plots. An arc whose next step can't happen fizzles out quietly.
Longer storylines, over many sessions and with choices, are [questlines](#questlines).

- **When:** one arc at a time. About every hour with players online, 60% of the time one starts:
  a random arc that hasn't run in the last four hours and can run now. The arcs' clock (steps,
  timeouts, the four hours) stands still while nobody is online. Saved; an arc waiting on
  a mission (or The Last Convoy's truck on the road) just ends at a restart; a convoy that
  hadn't left yet is asked for again.
- **The same rules as any mission.** A mission an arc starts goes through the mission manager's
  "can it run now" check (its world gates, its weight if it's also picked at random, one of
  each type, its claims), and an arc holds what it uses while it runs: Fever Town and The Quiet
  Congregation their safehouse and its town, the Preacher's two arcs the Preacher, The Deserter's
  Map its two map holders ([World facts](#world-facts)).

| Arc | Steps (each needs the one before) |
|---|---|
| **The Missing Courier** | A Halcyon courier stops reporting (the Lost Colleague contract, renamed). Once that search ends, wanderers say they saw Red Hand men with lab cases. Once a player has heard it, a headline: that camp sells antidotes in Halcyon cases (if the outpost isn't the Red Hand's any more by then, they've moved to another one). Ten minutes later the Red Hand comes for more: a **Lab Defence**, whose brief says whether stolen doses are still on the market |
| **The Last Convoy** | Bastion announces a convoy to pull an outpost's people back. Its next real supply truck goes there. Red Hand lookouts are seen; the Red Hand's Known hear it's worth robbing, Bastion's Known are asked to ride along. Delivered: the post stays. Lost: it runs dry. The post falls or is abandoned first: the arc ends with a headline ("Morton fell before Bastion's convoy arrived") and a rumour |
| **The Preacher's Flock** | The Preacher rings his bells in every town (twice the heat). After two towns, up to three small hordes come for him while a player is within 600 m. Ends when someone asks him to stop, after an hour, or if he dies |
| **The Deserter's Map** | A dying deserter's map piece names two wanderers. Each holds a piece; with both, the grid of a hidden Bastion bunker, guarded by four infected |
| **Fever Town** | A safehouse's town creeps up in infection. If a player goes near: a Halcyon contract checks the water. If that succeeds: a Haven contract to get the sick out. Done: two people move and the added infection goes; failed: two die |
| **The Quiet Congregation** (the Haven's dark side) | Rare: a safehouse that has lost 6+ people lately (to hunger, raids or worse: `m_iLost`), with the Preacher alive. He walks there and the house prays with him: half portions, bells at the gate, and gossip that it "borrows" from other houses. Players end it by bringing 6 food or asking the Preacher to stop. Otherwise after 90 minutes he walks on and two people go with him |

The missions inside arcs are in [MISSIONS.md](MISSIONS.md), "Story arc missions".

**Headlines.** The last 10 headlines are kept: leader news, territory changes, life events,
arcs, the power grid and burned towns. A player's first spawn of a session puts 3–4 lines of
server news ("Word on the roads") into the notebook's *This season* entry, no card: the latest headline, the last wanderer lost, the biggest
rescue so far, now and then a dead character's journal or a line of Haven gossip, and always
the cure percentage.

*Simplified:* the "camp selling antidotes" is a headline, not a shop. The convoy is the normal
supply truck. The flock's hordes spawn at the Preacher rather than following him from town to
town. The congregation's "borrowing" is talk only; its half portions are real.

## Questlines

Code: `Director/RZ_Questlines.c` (the engine), `Missions/Questlines/` (the author's base class
`RZ_Questline.c`, the approach helpers `RZ_QuestApproach.c`, the registry
`RZ_QuestlineRegistry.c`, and one folder per faction for the questlines themselves). Settings:
**Director > Questlines**. Every questline and its stages are in [MISSIONS.md](MISSIONS.md#questlines).

Long storylines in the New Vegas style, much longer than [story arcs](#story-arcs-and-headlines):
a questline runs over many sessions, through stages that each offer one mission or wait for
time and the world, and how players did each stage decides where it goes and how it ends.

- **Stages.** A *mission stage* offers one mission, a normal `RZ_Mission` started through the
  mission manager, so offers, rough areas, eligibility, payment and cleanup work as for any
  contract. A *wait stage* waits for play time, real time or both ("a day later"), and for the
  world (a leader alive, a town held, morale, a flag set earlier). A stage whose wait is over
  but which can't happen (its world gate is false, or its mission finds no place) keeps
  waiting, then fizzles after its give-up time (3 hours of play unless the stage says
  otherwise) with one line of text. A stage the story keeps coming back to (a retry after a
  failure) fizzles after its visit limit (3).
- **Approaches.** A stage mission can be done several ways: talk, persuade (background and
  standing), sneak (unseen, no shots near), force, bribe (Favor), betray, or side with another
  faction. The approach that decides the stage sets an *outcome*, and the questline picks the
  next stage from it. A side approach can end the mission as failed for its giver (no pay)
  while still moving the story on.
- **Who took part.** Every player who did something at a stage is recorded by identity with
  their approach and a few words ("talked him out of it"), so players can come and go across
  sessions. Personal rewards go at once to whoever did the step: the stage mission's own pay,
  standing from siding, a small extra reward. World consequences at the end are shared.
- **Endings** change the world only through existing systems, each capped per call: faction
  morale (±10), strength (±5), a town's infection (±15), a safehouse's food (±6) and people
  (±3), a leader replaced (never a stronghold's), an outpost handed over (the territory's own
  rules: never a stronghold, never the owner's last, no truce, nobody watching), a headline, a
  rumour wanderers and leaders repeat for some hours, a wanderer who remembers everyone who
  took part, and standing for everyone who took part (Fame ±10, Infamy up to 15; optionally only
  those who took one approach). Players who are offline lose Fame and take Infamy on their saved
  profile; Fame they earned waits in the errands' ledger for that life's next join, paid through
  `AddFame` then (another life: not paid).
- **How many.** At most **Max active** questlines at once (2), never two for the same faction
  (a group that isn't one of the four factions has its own slot). Every 30 minutes or so with
  players online, half the time, a questline that can start does. One that reached an ending
  doesn't come back this season; one that fizzled can start again after 6 hours of play.
- **Time.** The questlines' clock runs only while someone is online. Real-time waits use the
  server's clock, so "12 real hours" passes overnight with the server empty. **Wait scale**
  multiplies every wait and give-up time (0.1 for a quick test).
- **The journal.** A questline appears in the journal of everyone who took part and of anyone
  its current stage mission was offered to: its title, the stage, the stage's text and "what you
  know so far" (notes added as it goes). Its stage missions show the same type icon
  ([UI.md](UI.md#screens)).
- **Saved.** The stage, flags, texts, notes, timers, branch, visits and every participant are
  saved and restored. A stage mission doesn't survive a restart: its stage stays, and the mission
  is offered again from the start. A season wipe ends every questline.
- **The finale's lane.** The cure finale's five routes ([FINALE.md](FINALE.md#how-a-route-runs)) run
  outside Max active and the faction slots, start at the 90% gate, never fizzle (a stage that can't
  happen waits, one that fails is offered again) and freeze at another route's finish. Their stages
  are **open** (anyone not locked to another route) or **sworn** (only the route's locked players);
  sworn stages aren't offered on the radio, the route's players are told, and the route is in their
  journal.

*Simplified:* the "persuade" check is a fixed score, not a roll: the same player with the same
standing always gets the same answer (once per player per mission, kept by identity, so leaving
and rejoining doesn't give a second try). A stage's people stand
still (props with talk and choice actions), apart from optional AI groups.

### Writing a questline

One class in its own file, `Missions/Questlines/<Faction>/RZ_Quest_<Name>.c`, plus its stage
missions in the same file, registered with one line in `RZ_QuestlineRegistry.RegisterAll`. The
example is `Missions/Questlines/Haven/RZ_Quest_ShortWeight.c`.

1. **Define()**: `Info("haven_short_weight", "Short Weight", RZ_EFaction.SURVIVORS)` (the key is
   saved: never change it once it has run; a questline designed in
   [QUESTLINES.md](QUESTLINES.md#the-questlines) uses the key given there, which content may
   already gate on), then the stages, the first one is where it starts:
   `MissionStage(S_ASK, "Where the food goes", "...{town}...")` or
   `WaitStage(S_WAIT, "The next handover", "...")`. Tune a stage on the object it returns:
   `.Wait(playMinutes, realHours, both)`, `.GiveUp(playHours, "fizzle line")`, `.Visits(n)`.
   A group that isn't an `RZ_EFaction` passes faction -1 and calls `Slot("cannibals")`.
2. **CanStart(gm, run)**: pick the places and people and keep them on the run:
   `run.SetFlag("house", idx)`, `run.SetText("town", name)`, `run.SetPlace(pos, name)`.
   Hold what the story uses so no other story takes it meanwhile:
   `run.Claim(RZ_WorldFacts.Safehouse(idx))` (false if someone else has it; kept with the run,
   held again after a restart, let go when the run ends; a claimed house makes its town busy for
   every other mission). Return false if the world isn't right yet; spawn nothing here. Every
   `{key}` in a stage's text, a note or a line is filled in from the run's texts, then from the
   world's own tokens (`{day}`, `{cure_state}`, `{leader:id}`...).
3. **IsReady(gm, run, stage)**: world gates on top of the wait, with the helpers
   `IsLeaderAlive`, `LeaderNear`, `IsTownHeldBy`, `GetMorale`, `GetHouse`, or a flag. For
   anything else the world knows (the cure, the day, night, who holds a town, a safehouse's
   food, a leader's live name, a fixed fact of the setting) use the [world facts](#world-facts):
   `gm.GetFacts()`. **Crossings between questlines** use the world facts' season flags:
   `gm.GetQuestlines().WorldFlag("key.flag")`, `SetWorldFlag`, `HasWorldFlag` (the same store as
   `gm.GetFacts().Flag`, so contracts, arcs and voice read them too). A flag only your questline
   reads is `<questline key>.<flag>`; a flag another line reads uses its name from the
   [Crossings](QUESTLINES.md#crossings) table (short names such as `list.to_bastion`, reserved for
   those lines). Every ending sets `<key>.ending` by itself. A contract can lead into a
   questline with `GetQuestlines().TryBegin(key)`, which starts it only if the usual limits allow
   (No Reply's tape starts Await Relief). Ordinary contracts read the flags too (Last Helicopter
   stays grounded by The Last Pilot's).
4. **MakeMission(gm, run, stage)**: `return new RZ_QM_MyStage();`. The engine sets its
   `m_iQuestRun`, runs `Configure` and starts it if it can run now (the mission manager's shared
   check: world gates, one of each type, claims; [World facts](#world-facts)). A stage mission may
   use what its questline holds: check its own town with `IsTownBusy(gm, idx)` and pass `this` to
   the shared pickers. Halcyon's stage missions stop after the cure unless `m_Req.m_bAfterCure` is
   set (one running then is called off).
5. **The stage mission** extends `RZ_QuestMission` (a puzzle mission that is never picked at
   random). `Configure` sets the giver and pay as usual (`SetContract`). `SetupPuzzle` reads the
   run (`QFlag`, `QText`, `QFill`) and picks the spot; `SpawnScene` puts people and props down
   when the first player is within 400 m: `SpawnPerson(gm, pos, objIdx, talkClue, "Talk to
   ...", "Ask|Press him|Pay him")` (up to four choices), `SpawnClue`, `SpawnObject`. Approaches
   in `OnChoice`, `OnUse` and `OnClueRead`:
   - talk: `Remember(gm, playerId, RZ_EQuestApproach.TALK, "asked about the stores")`;
   - persuade: `TryPersuade(gm, playerId, faction, difficulty, "police,excon")`: the standing's
     rank with that faction (Stranger 0, Known 1, Trusted 2, Champion 3, mixed tiers one less,
     Watched -1, Hunted -2) plus 1 for a listed background must reach the difficulty; once per
     player (by identity, so a rejoin doesn't reset it);
   - sneak: `m_Quiet.Watch(pos, radius)`, then `m_Quiet.IsQuiet(gm, playerId)` (no shots in the
     radius since, not noticed by heat or the infected), `m_Quiet.Spot(playerId)` when someone in
     the scene sees them; a person who sees anyone within N m checks
     `RZ_Stealth.SightRadius(playerId, N)` ([Stealth](#stealth));
   - force: `RZ_UnitWatch` for a spawned group, `SpawnGuards` and `OnZombiesKilled` for the
     infected, `RZ_Util.IsArmed` for a threat;
   - bribe: `TryBribe(gm, playerId, faction, cost)` (spends Favor, says so if they're short);
   - side with another faction: `RZ_QuestApproach.SideWith(gm, playerId, with, against, fame, infamy)`;
   - the approach that decides the stage: `Resolve(gm, playerId, approach, outcome, "what they
     did", success)`. success = false ends the mission as failed for its giver (no pay), but the
     outcome still goes to the questline.
   Flags for later stages: `SetQFlag(gm, "caught", 1)`; journal notes: `QNote(gm, "...")`;
   a personal extra: `Run(gm).Reward(gm, playerId, faction, fame, favor, "why")` (Fame up to
   15, Favor up to 40).
6. **Next(gm, run, stage, outcome)**: return the next stage's id. The outcome is yours (10 and
   up), or `OUTCOME_DONE` (the mission succeeded without one, or a wait is over) or
   `OUTCOME_FAILED` (it failed or ran out of time). Add notes (`run.Note`), set the branch
   (`run.SetBranch`), check who did what (`run.AnyDid(approach, stage)`, `run.Flag`). End with
   `return Finish(run, ending, "line for everyone who took part")` or
   `return Fizzle(run, "line")`.
7. **OnEnding(gm, run, ending)**: the consequences, through `NudgeMorale`, `NudgeStrength`,
   `NudgeInfection`, `ChangeFood`, `ChangePopulation` (never below 2 people),
   `ReplaceLeader(gm, id, headline, run)` (pass the run to move a leader it claimed itself),
   `HandOutpost` (both return false if they can't happen: then don't claim they did), `Headline`,
   `SpreadRumour`, `RememberWanderer` and `ChangeStanding`.
8. Optional: `OnBegin` and `OnStageBegin` (an opening headline, a first note), `Rumour` (gossip
   while it runs), `OnWandererTalk` and `OnLeaderTalk` (a line when a wanderer or leader is talked
   to, which can also set a flag a wait stage waits for), `JournalText` (a brief that changes
   with the flags).
9. Add it to [MISSIONS.md](MISSIONS.md#questlines) (stages, approaches, endings, ✅ only when
   it's in code) and run `tools/compile_check.sh`. To try it quickly, set **Wait scale** to 0.1
   and call `GetQuestlines().Begin("your_key")` from a test.

Player-facing text follows [WRITING.md](WRITING.md).

### The majors

Code: `Director/RZ_Majors.c` (the clocks, `RZ_QuestVote`, the pay caps, the season story), the base
class's majors part in `Missions/Questlines/RZ_Questline.c`, the scenes in
`Missions/Questlines/RZ_MajorScene.c` and `RZ_MajorHelp.c`. Settings: **Director > Majors**. The four
stories, their acts and endings are in [QUESTLINES.md](QUESTLINES.md#the-majors); their stage
missions in [MISSIONS.md](MISSIONS.md#the-majors-acts).

A **major** is a questline that calls `Major()` in its Define: one per faction, a season long, its
climax on a cure milestone. Its stages are grouped into **acts** (`Act(id, name)` with `.Stage()`,
`.AnchorCure()`, `.AnchorStage()`, `.AnchorWipe()`, `.Need()`, `.Patience()`, `.PatienceEvenings()`,
`.SetPiece(from, to)`, `.LastCall()`, `.Climax()`, `.PastGate()`, `.Watch()`, `.Opens()`, `.Rank()`).
Each act opens on its own trigger (its first stage's `IsReady`: a milestone, a stage, the act or
climax before it) and has a **default** (`ActDefault`): what happens when nobody decides it.

- **The lane.** Majors run outside **Max active** and the one-per-faction rule, so a faction's
  smaller stories run beside its major. A major starts as soon as its `CanStart` is true (checked
  every minute, no roll), runs once a season and never fizzles: a stage that can't happen waits, a
  retry loop doesn't end it, and a stage that goes quiet takes its act's default. At most **Slots**
  (2) majors' stage missions are offered at once; a squeezed act takes a slot from one that isn't
  squeezed or engaged. Inner lines a major starts itself (HC1, HC3) are in the lane too, and never
  start on their own while the majors run (`StartsOnItsOwn`).
- **Absorbed lines.** The Book (R1) and Await Relief (B1) are their faction's majors under their own
  keys: a built ending inside the major (`Finish` in their own `Next`) becomes an act's result
  (`AfterInnerEnding`, applying the built consequences) and the major goes on. Every Name uses The
  List's (H1) stage missions under a new key; H1 stays registered (saved runs load) and isn't offered.
  With **Majors** off, The Book and Await Relief run as their built questlines, and The List, The Three
  Weeks and Authors start on their own again.
- **`ActEnd(run, act, outcome, line)`** settles an act inside a run without ending it: it sets
  `<key>.act<n>` (and `<key>.climax` for the climax), notes the line, tells the participants and
  marks a beat. `<key>.ending` is set only at the major's real end (after its coda).
- **Set pieces** hold the evening on the [set-piece calendar](#the-set-piece-calendar) from their
  offer window until they resolve; one that finds the evening held waits, at most twice. The
  climaxes keep their order through their gates: settling day, the muster, the second sitting, the
  reading, the last page (`RZ_Majors.ClimaxDone`: done, or that major isn't running).
- **Scenes.** Every new act is one stage mission class, `RZ_QM_MajorScene`, filled in by the major's
  `BuildScene`: people at places (spawned only when a player comes within 300 m), up to four choices
  each, a choice being data (a rank, a persuade, a bribe, unseen, a threat (Fear), a trick (the look),
  a trade (items handed over), a flag; a sway, a tabled option, a
  swing of voters, a flag, a side's standing, the stage's outcome). A scene may hold a place (stay
  within 40 m; the noise brings waves every 75 s; 45 s with nobody there loses it) and close at the
  end of its window with its vote's tally or a timeout outcome. Dialogue is off by default, so every
  sway, ask and persuade is an action on a person.

**Votes (`RZ_QuestVote`).** A tally kept in the run's flags: voters with a lean each, options tabled
(two from the start, offers only when carried), one sway per player per vote (by identity), and
the tally with a tie-break (the chair's lean). A conversation's `sway:<major key>.<vote>.<word>`
([DIALOGUE.md](DIALOGUE.md#several-players)) swings one voter to that option per contribution at the
tally. Errands move a vote only with **Missions > Errands > Majors read errand memory** on (off by
default): a sway on a head whose house remembers this player's errands (a thread there,
[ERRANDS.md](ERRANDS.md#threads-and-callbacks)) brings one more voter round to the same option, once
per head a season. Used by the Haven's two sittings.

#### Dynamic fallbacks (decision 10)

No major runs on fixed days. Each act's default fires from what is happening on the server, read on
the questline tick (every 10 s with players online; nothing moves on an empty server).

| Signal | What it is |
|---|---|
| Research position and pace | Worked research; its gain a real day over the last 3 real days that had play |
| Hours played a day | Hours with someone online, over the last 7 real days |
| Room left | Hours of play before the act's anchor: (milestone − research) ÷ pace × hours a day; for a stage, the stage clock's minutes to go; for a coda, real hours until the epilogue's last day. Unlimited while research stalls |
| The faction's state | Its stronghold Under Pressure (the clocks pause), its morale |
| Offered time | Play time while the act's stage mission is on the offer list (one of the **Slots**; a watched inner line while it runs), in its window for a set piece, and a player who qualifies (the act's rank with the faction) is online. Offered evenings: 30 real minutes of it in one evening |
| Touches, engagement | A touch is anything a player does in a stage (`Did`); engaged: a stage mission with someone in it, or touched in the last session (2 hours of play) |
| Beat gap | Hours of play since the major's last beat (an act opened or ended, a default) |
| Stalled | Research rose less than **Stall points** (1) over the last 3 real days with play, and the lab's backlog is empty |

A default fires on the first of these, once the guards allow it:
1. **Squeeze:** the room left is less than the need of this act and of the acts after it with the
   same anchor. A **last call** goes out (the act's own line, as a headline and a rumour, and in the
   journal), then a **grace** of **Grace sessions** (1) of offered time; with nobody who qualifies on
   for two sessions, plain hours count. The default fires when the grace runs out or the anchor
   arrives.
2. **Patience** (acts that have one): offered time or offered evenings down to the last one: a last
   call, then the grace.
3. **Nobody qualifies** (set pieces): two evenings with players online but nobody who qualifies; it
   runs as data. Other acts wait for their squeeze, because a player may grow into them.

**The guards:** an engaged act holds for at most **Hold limit** (1 session), then the default comes
anyway; at most one default per major a session unless squeezed (the breather); a patience default
that costs morale waits while the faction's morale is under 30; nothing ticks while the faction's
stronghold is Under Pressure or nobody is online. Defaults pay nobody (`RZ_Majors.Enter(key, true)`)
and a default that ran as data is told one headline a real day, in order.

**Lead:** while research is stalled, a climax (or the Red Hand's last page) may open up to **Lead
points** (5) before its milestone, if the climax before it has resolved, the major's beat gap is at
least 3 sessions and a player who qualifies is online (`RZ_Majors.Reached`). Never past the gate,
and it never moves research.

**Between beats:** a beat gap of **Between beats** (4) sessions while the next act waits on a
milestone more than 6 sessions away: that faction's smaller stories are considered first and start
twice as often, and one of its people says a line about what's coming, once a real day. It never
opens an act, pays nobody and moves no vote.

**Hard stops:** at the gate (research 90%, the stand or the cure), and in the wipe's last 24 hours
with no cure, every act still open that doesn't run past the gate takes its default at half world
effect, paying nobody (`RZ_Majors.HardStops`). Past the gate run only A
Week On, the stand acts and the codas (and with no cure at the wipe, those too).

**Pay and the budget.** While the engine runs a major's hooks (`RZ_Majors.Enter`), `NudgeMorale` and
`NudgeStrength` draw on the [season budget](#the-season-budget)'s majors lane, and standing goes
through the majors' caps (`ClipPay`): per player per faction 25 Fame, 40 Infamy and 60 Favor over one
major, 40, 40 and 80 across all majors a season. A stage mission's own contract pay is outside it.
Each major sets at most three season weights (`SetContractWeight`).

**The season story.** Each major's ending writes `RZ_Majors.SeasonStory(faction, key, ending,
branch, line, deciders, fallback)`: its epilogue line and up to four deciders by identity. It's saved
with the questlines and written into `last_season.json` (`stories`) at the cure and at a wipe without
one. At the cure the epilogue line is told as a headline: the controlling faction's major first, then
one line from each other major. Each ending's line has three versions (`RZ_Majors.LineFor`): when its
own faction controls the cure, when another does, and with no cure. The same record gives next
season's story rules ([QUESTLINES.md](QUESTLINES.md#next-season-the-story-rules)), read from the
ending as it stood at the cure (`atCure`), and the hall's lines
([QUESTLINES.md](QUESTLINES.md#the-seasons-lines-and-the-season-tab)).

**The journal.** Every player sees the four majors in their journal (the questline rows): the act,
the rule or stance in force, the voters' leans in words, and any last call, under the stage's brief.
The notebook's **The season** tab shows the same for all four at once, and each ending told
([UI.md](UI.md#the-notebook)).

**Stand events.** `RZ_Questline.OnFinalStandStart` and `OnFinalStandEnd(won)` reach every running
line when the final stand starts and when it's won or lost (`RZ_Questlines`, from
`RZ_CureResearch`); a major's hooks run inside its caps. Proof's Before the stand settles on them.

| Setting (Director > Majors) | Default | What it does |
|---|---|---|
| Enabled | on | The four majors (needs Questlines on) |
| Lead points | 5 | How far before its milestone a climax may open while research stalls (0: never) |
| Grace sessions | 1 | The grace after a last call |
| Hold limit | 1 | How long an engaged act can hold its anchor |
| Stall points | 1 | Research over 3 real days with play below which it counts as stalled |
| Between beats | 4 | The beat gap, in sessions, before a major fills it |
| Slots | 2 | Majors' stage missions offered at once |
| Red Hand lorry, Bonfire prop | empty | Optional props at Rudi's crossroads (the send-off and the last line; sent off loud), spawned only while that stage runs and a player is within 300 m; empty: words only |

**The engine's rest** (`Director/RZ_MajorRest.c`): `OnStandHolder` tells every running line when the
clean room changes holder; `OpenChannel(faction, lines, gap)` reads lines on a faction's open
channel one at a time (the director's two minutes); `SecureLine(faction, text, hours)` puts a line
on a faction's secure net for Radio Operators; `AtTime(faction, hour, text, key)` airs a one-off line
at a set game hour (Bastion's last call at 20:30, the walk at the next 05:30); `Mute(faction)` silences
a faction's open channel for one night; the per-life "on the roster" flag and Bastion's rank text
(*"Corporal Novak"*, by standing). `More()` adds each act's remaining approaches to its scene
([QUESTLINES.md](QUESTLINES.md#the-majors)), and `AtEnding` applies their effects once, at the major's
ending. With no cure on the last day the codas end as radio (the Haven's Gate at the chair's house,
Bastion's last call in the last 24 real hours, the Red Hand's ending on its current branch).
*Simplified:* stand-ins are civilians, not the leader's own body; summons don't claim seats; there's
no *Ask about...* choice on other factions' leaders while a stage is open.

## World facts

Code: `World/RZ_WorldFacts.c`, reached as `gm.GetFacts()` or `RZ_WorldFacts.Get()` (never null:
without a game mode every query gives its default). Server only.

One place where content (missions, events, arcs, questlines, rumours, voice, guidance) reads what
is true in the world right now, so a brief, a rumour or a leader's line never says something the
world contradicts. It adds no simulation: every query reads the system that owns the fact. It
keeps four small things of its own: **season flags**, **canon**, **claims** and **gossip**, and
holds the shared engine's [world memory](#world-memory).

**Queries**

| Fact | Call | From |
|---|---|---|
| The cure is out | `IsCured()` | cure research |
| The season is over and waits for its wipe | `IsEpilogue()` | the cure, or the wipe came due ([Seasons](#seasons)) |
| No cure, and the wipe is under Warn hours away (the Unsigned last day) | `IsLastDay()` | the wipe cycle ([EPILOGUE.md](EPILOGUE.md#edge-cases)) |
| The epilogue week holds every holding where it is | `HoldingsFrozen()` | the epilogue ([EPILOGUE.md](EPILOGUE.md#the-world-settles)) |
| The final stand is running; research (what the lab has worked through) | `IsFinalStand()`, `CurePercent()` | cure research |
| Research waiting in the lab's backlog; the backlog is full; research waits at 90% for a finale | `LabWaiting()`, `LabFull()`, `AtFinaleGate()` | cure research |
| Outbreak stage; how much of Bastion is still out | `Stage()`, `BastionPresence()` (0–1) | the Director's stage table |
| The day | `Day()` (the broadcast's count), `DaysSince(day)`, `DaysSinceCommand()` (Command's last signal came on Day 3, [LORE.md](LORE.md), "Task Force Bastion") | the broadcast |
| Time of day | `IsNight()` (20:30–05:00), `IsDusk()` (18:30–20:30), `IsDaytime()`, `KnowsTime()` | the game clock |
| The line is up and shooting | `PerimeterSealed()` (not after the cure) | the Perimeter |
| Bastion's helicopter can fly | `HeliAvailable()`; `SetHeliDown(days)` (days < 0: for good) | storms, the season flags `heli.down_until` (a day) and `heli.lost` |
| Who holds a town | `TownOwner(idx)`: faction and kind (lab, stronghold, outpost, safehouse, none); `IsLivedIn(idx)`; `OwnerText(idx)` | the lab, territory, safehouses |
| A town for a job | `PickTown(filter, asker)`, `CollectTowns(filter, result, asker)`: an `RZ_TownFilter` (infection, held by a faction or by nobody, not held by some factions, kind of holder, size, a distance from a point, one to leave out), busy towns skipped. A mission's shortcuts: `PickEmptySettlement(Away)` (nobody lives there), `PickSettlement(Away)NotHeldBy` (none of some factions holds it; `OthersBits(f)`: only `f` may), `PickHeldSettlementAway` (a town that faction holds) | the queries above |
| People live near a spot | `IsNearLivedIn(pos, radius)`: a safehouse, a holding or the lab (what shelling keeps away from) | safehouses, territory, the lab |
| Lights | `IsPowered(pos)` | the power grid, surges, generators |
| A safehouse now | `House(idx)`: people, food, short, medic, radio, lost lately (`m_iLost`: left, moved, taken or dead), died lately (`m_iDied`, only the dead), empty, tier | safehouses |
| A leader now | `LeaderName(id)`, `LeaderAlive(id, generation)`, `LeaderGeneration(id)`, `LeaderFree(id)` | leaders: a seat keeps its id when someone new sits down, so write `{leader:id}` in text (filled when it's shown) |
| A wanderer is free | `WandererFree(id)` (alive, not held, not claimed) | wanderers |
| Doses on the black market | `StolenDoses()` | cure research |

**Season flags.** `Flag(key, def)`, `SetFlag`, `HasFlag`, `ClearFlag`: whole numbers for the
season, named `<owner>.<flag>`. The questlines' world flags are these, so any content can read a
questline's outcome (every ending sets `<key>.ending`) and set one a questline reads. Saved in
`state.json`; a new season starts with none.

**Canon.** Fixed facts of the setting, made once a season from its seed and then the same every
time anything asks, so a mission never re-rolls the story: `CanonText(key, rule)`,
`CanonInt(key, min, max)`, `CanonPick(key, options)`, `CanonSettlement(key, rule)`,
`CanonPos(key, rule)`, `HasCanon`, `SetCanon` (the first value wins). Without a rule a key gets
its built-in rule below, else a seeded default (a number in the range, one of the options, a
town nobody lives in, a dry spot just outside it). `CanonInt` always answers inside its range,
even for a built-in or earlier value. A rule is a small `RZ_CanonRule` subclass whose
`Make(gm, facts, rng)` returns the value as text (`RZ_WorldFacts.PosText(pos)` for a spot); a
rule that asks for the fact it is making gets "" back. Saved; a new season makes new ones. A
canon town can change hands during the season: read `TownOwner` when you use it.

| Key | Kind | The fact | Built-in rule |
|---|---|---|---|
| `origin.site` | spot (`CanonPos`) | the closed site in the hills | The Three Weeks' rules: a high dry spot between two towns 1.4–6 km apart, at least 450 m past every settlement, 1.5 km from the lab, not behind a lock and not nearest the lab's town, moved onto a factory, mast or barracks within 400 m if one is as remote; else high ground beyond the town farthest from the lab, as remote as it can be |
| `origin.site.town` | town (`CanonSettlement`) | the town nearest the site | |
| `origin.clinic` | town | the clinic the first patient was brought to (the last town of Patient Zero's trail) | the town or city nearest the site, not the lab's or the site's own |
| `origin.patient` | words | his surname (Patient Zero's clinic records name him, with the site's town and the day the Institute was told) | a surname |
| `origin.told`, `origin.told.month` | number (`CanonInt`) | the date the Institute was told | day 3–24, month 1–12 |
| `bastion.orders` | words | what the last orders said (The Real Orders reads it) | `expended`, always (pinned by Await Relief; "Command meant to come and couldn't" is only what some posts believe) |
| `redhand.pit` | spot | where the Red Hand's fight pit is (Fight Pit and The Pit's Debtor hold it while they run; House Rules uses it when it's behind that crew boss's seat) | 60–150 m behind the boss's seat at the Red Hand stronghold, else a crew boss's seat, else its stronghold or an outpost, else outside a quiet town |
| `redhand.pit.town`, `redhand.pit.seat` | town; number | the pit's town; the seat it's behind (a leader id, -1 none: `LeaderName(id)` names whoever sits there now) | made with the pit |
| `redhand.crossroads` | spot | Rudi's crossroads (the Red Hand's major) | the road junction (the road point with the most others within 30 m) 300–800 m from the boss's seat, not within 400 m of a safehouse or the lab and off the line; without roads, a dry spot 300–800 m out |
| `day2.checkpoint` | spot | where the Day 2 column was turned back (the majors' codas) | the sealed road checkpoint nearest the Haven stronghold's safehouse, preferring the north; without checkpoints, that safehouse. The walks and readings halt 150 m short of it toward the stronghold, the paper's table 110 m (`RZ_MajorHelp.NorthRoad`) |

**Claims.** Who is using a subject right now, so two stories don't use the same house, leader or
wanderer at once. `Claim(subject, owner)` (true if it was free or already that owner's),
`IsClaimed`, `IsClaimedByOther`, `ClaimedBy`, `Release(subject, owner)`, `ReleaseAll(owner)`.

| Subjects | Owners |
|---|---|
| `lab`, `heli`, `pit`; `Town(idx)` "town:7", `Safehouse(idx)` "safehouse:2", `Leader(id)` "leader:12", `Wanderer(id)` "wanderer:5", `Truck(faction)` "truck:1" | `MissionOwner(id)` "mission:12", `ArcOwner(kind)` "arc:4", `QuestOwner(key)` "quest:haven_short_weight"; the cure holds `lab` during the final stand and the finale's batch hold (its route stages never claim it); the finale's routes claim their subjects as `person:<key>` ([FINALE.md](FINALE.md#how-a-route-runs)) |

Who holds which today: `lab` Lab Defence, Steal the Antidotes and Burn the Bodies at the lab (on
their own; a story that runs one holds the lab itself, `IsStandalone`); `heli` Last Helicopter and
Rescue the Pilot (both also need `m_bNeedsHeli`); `pit` Fight Pit, The Pit's Debtor, and House
Rules when the pit is behind its seat; `Leader(id)` Kill the Officer's officer. Take the Lab runs in
the final stand, inside the cure's own hold.

- **A mission** lists what it needs in `GetClaims(gm, result)`: the manager starts it only while
  they're free, holds them from before `Setup`, and lets go when it ends or `Setup` fails. What
  it picks in `Setup` it holds with `ClaimSubject(gm, subject)`. A mission started by an arc or
  a questline may use what that story holds (`m_sClaimParent`).
- **Story arcs** and **questline runs** (`run.Claim`) hold theirs until they end.
- **Who respects them:** a town is busy (`IsSettlementBusy(idx, asker)`) while another running
  mission uses it or a story holds the town, a safehouse in it, or the lab (the final stand).
  With the asking mission, what it or its own story holds counts as free, so a stage mission can
  use its questline's house. The shared pickers take the mission as their last argument:
  `PickSettlement`, `PickSettlementAway`, `RZ_EventHelp.PickEmptyTown`, `PickFreeSafehouse`
  (also skips claimed houses), `FindForMission` (claimed wanderers) and `PickTown`. A mission
  checks its own town with `IsTownBusy(gm, idx)`. A claimed leader isn't killed off-screen (the
  danger roll, `KillOffscreen`, which returns false then) or moved from the seat by anyone else
  (`RZ_Questline.ReplaceLeader(gm, id, headline, run)` may move one its own run claimed).
- **Runtime only.** Claims aren't saved. After a restart the content that is still running claims
  again: missions are offered again from the start, arcs and questline runs take theirs back.
- **Soft holds** (`SoftHold`, `SoftRelease`, `SoftHeldBy`) are the errands' own, kept apart from
  claims: an errand holds its subject so a second errand can't use it, never making a town busy for
  anyone; a claim on the subject overtakes the errand ([ERRANDS.md](ERRANDS.md#keeping-them-true)).
  Runtime only.

**Can it run now?** `RZ_MissionManager.CanRunNow(m, players, checkWeight)` (`WhyNot` says why
not, `StartIfCan` checks and starts) is the one check every starter uses: the requirements'
world gates and facts, the season's No Vehicles rule, the mission's weight if the manager also
picks its type at random, one of each type at a time (a leader's personal jobs: one per leader,
so two leaders can each offer the same kind), and its claims free. The random picks, story arcs,
questline stages, leaders' personal jobs and the Mechanic's and Courier's jobs all use it. The
Turned Player event and a leader's Rein In are systems and start directly.

**Called off.** `m.Withdraw(why)` ends a running mission that the world made pointless: at the
next tick, with `why` told to everyone it was offered to, and nobody paid, blamed or penalised
(no Fame, Infamy, strength, morale or leader memory); its arc or questline hears it as a
failure. When the cure comes out, every running mission that stops with it is called off
(`RZ_MissionManager.OnCured`: `m_bNotCured`, and Halcyon's work unless `m_bAfterCure`). When a
supply truck's post changes hands, missions get `RZ_SupplyLines.TRUCK_CALLED_OFF` (with the
holding's id as `driver`, before or after the truck left): Convoy Escort and Hijack the Convoy
are called off with it.

**Requirements' world facts** (`RZ_MissionRequirements`, checked with the stage and cure):

| Field or call | Means |
|---|---|
| `m_bNotCured` | stops once the cure is out. **Halcyon's work does by default** (it's all research); `m_bAfterCure` keeps one going |
| `m_fMinMilitary`, `m_fMaxMilitary` | Bastion presence within this range |
| `m_eTime` | `RZ_ETimeGate`: `ANY`, `NIGHT`, `DUSK_OR_NIGHT`, `DAY` (with no game clock every value passes) |
| `m_iMinDay` | the broadcast's day (`Day()`) is at least this; 0 = any (Struck Off waits for Day 13) |
| `m_bNeedsHeli` | Bastion's helicopter can fly (`HeliAvailable`: no storm, not down) |
| `NeedFlag(key)`, `NeedNoFlag(key)` | a season flag set, or not set |
| `NeedFlagWithin(key, days)`, `NeedFlagNotWithin(key, days)` | a flag that holds a day (what `Changed` sets) set no more than `days` days ago (0 = today), or not |
| `NotWhileQuest(key)`, `NotAfterQuest(key)`, `NotWithQuest(key)` | not while a questline runs, not once it reached an ending, or neither. Keys are fixed in [QUESTLINES.md](QUESTLINES.md#the-questlines); one that is neither registered nor designed is logged once as a warning |

**Text.** `Fill(text)` fills the world's tokens, so no brief, rumour or line hard-codes a fact.
Unknown tokens are left alone; a questline's own `{keys}` are filled first. **Mission briefs are
filled when they're shown** (`RZ_Mission.GetBrief`: offers, the journal, world-event notices,
leaders' and wanderers' jobs), so `m_sBrief` may hold tokens. Any other line (a drop's name, a
notice) is filled only where you call `Fill` on it.

| Token | Gives |
|---|---|
| `{day}`, `{day_text}` | 14, Day 14 |
| `{days_since_command}`, `{days_since_command_words}` | 11, eleven (since Day 3) |
| `{cure_state}`, `{cure_percent}` | "the cure is at 40 percent" (or "is out", "the lab is holding its final stand"), 40 |
| `{stage}`, `{stage_tone}` | Collapse; "the towns are going dark" (one phrase per stage, and one after the cure) |
| `{time_of_day}`, `{lab_town}`, `{stolen_doses}` | night, dusk or day; the lab's town; the black market's doses |
| `{town:N}`, `{owner:N}`, `{flag:key}` | a settlement's name, who holds it ("the Haven", "nobody"), a season flag |
| `{canon:key}`, `{canon_town:key}` | a canon fact as words (for the *words* and *number* kinds), the name of a canon *town*. Text never makes a new fact: only a built-in key or one already made is filled |
| `{leader:id}`, `{leader_short:id}`, `{leader_place:id}` | who sits in that seat now ("Major Weber", "Weber"), and the seat's place; an empty seat reads as its title ("the outpost commander in Morton") |
| `{tribe:N}`, `{alpha:N}`, `{young:N}` | a pack in a speaker's word with the place, its leader, its newest named young (`{tribe:N.bastion}` for Bastion's word; [INFECTED.md](INFECTED.md#the-words)) |

**World changes and gossip.** `Changed(key, headline, rumour, moment, rumourHours, announce)`
tells the world in one call: the headline goes to the server news (to everyone too with
`announce`), the rumour to wanderers' and leaders' gossip for some hours, the moment (a phrase
that fits "Not since %1.") to what the factions remember, and the season flag `key` is set to
today's day number. A supply truck called off because its post changed hands uses it.
`AddRumour(text, hours)` adds gossip on its own (at most 48 hours, the newest 8 kept; questline
endings use it too). Saved.

**Which call for what** (for authors):

| You want to | Use |
|---|---|
| say what day it is, or how long Command has been silent | `{day}`, `{days_since_command_words}` in `Fill`, or `Day()`, `DaysSinceCommand()` |
| stop a job once the cure is out | `m_Req.m_bNotCured = true` (Halcyon's already stop; `m_bAfterCure` to keep one); a running one is called off by itself |
| a job only at night, or only while Bastion is about | `m_Req.m_eTime = RZ_ETimeGate.NIGHT`; `m_Req.m_fMinMilitary = 0.3` |
| a job only once the season has run a few days | `m_Req.m_iMinDay = 13` |
| a job that needs Bastion's helicopter | `m_Req.m_bNeedsHeli = true`; `SetHeliDown(days)` when it crashes (days < 0: for good) |
| a job only in the days after something changed | `m_Req.NeedFlagWithin("<the Changed key>", 2)` |
| a contract that tells a questline's story | `if (m_iQuestRun < 0) m_Req.NotWhileQuest("<key>")` (or `NotWithQuest` to stay away after it too), with the key from [QUESTLINES.md](QUESTLINES.md#the-questlines) |
| a town that nobody lives in, or one a faction really holds | `PickEmptySettlement(gm, min, max, this)`, `PickHeldSettlementAway`, `PickSettlementNotHeldBy`, or `PickTown` with an `RZ_TownFilter` (`m_iHeldBy`, `m_iNotHeldBy`, `m_iKinds`, a distance) |
| keep shells, fire or a horde away from people | `IsNearLivedIn(pos, radius)` |
| name a leader in text | `{leader:id}` in the brief (or `Fill` the line where you show it); `LeaderAlive(id, generation)` before you count on them |
| say how a safehouse is doing | `House(idx)` (`m_iDied` for the dead; `m_iLost` counts everyone who left) |
| mention a fixed fact of the setting | `CanonSettlement("origin.clinic")`, `CanonPos("redhand.pit")`, `CanonText("bastion.orders")`, or `{canon_town:origin.clinic}` in text |
| keep a house, leader or wanderer for your story | `GetClaims`/`ClaimSubject` (missions), `run.Claim` (questlines); pass the mission (`this`) to the pickers and use `IsTownBusy` for its own town |
| start a mission from your own code | `gm.GetMissions().StartIfCan(m, players)` |
| end a mission the world made pointless | `Withdraw("why")` |
| record an outcome other content reads | `SetFlag("<you>.<what>", value)` |
| tell everyone something in the world changed | `Changed(...)` |

**What uses it.** The epilogue, the Perimeter, guidance, the voice lines named above, the arcs,
and the content:

- **Town pickers by who holds a town.** The faction contracts pick with the shortcuts above: work
  in a ruin (a nest, a cache, a town to retake or seal, a pharmacy, a body, a lost patrol) goes to a
  town nobody lives in; a faction's open work never goes to its rivals' holdings (the Haven's
  fields and relay, Bastion's curfew and recon, the Red Hand's car and courier); a drop, a fence, a
  stockade, a fuel depot or a relay goes where its faction really is when it holds a town in reach,
  else to a town nobody holds. Puzzles, events and most leader jobs still pick any free town.
- **Bastion presence** scales Bastion's contracts and road posts ([The Director](#the-director-outbreak-stages-and-spread)).
- **Briefs as tokens.** The days in briefs are tokens (`{days_since_command_words}`), and a brief
  that names a leader holds the seat's tokens (Kill the Officer, Rein In, Over the Fence), so the
  name is whoever sits there when it's read.
- **Canon.** Patient Zero (the origin story), The Real Orders (the last orders) and the Red Hand's
  pit read the canon table above. The Three Weeks is the Halcyon major's.
- **Claims** for the lab, the helicopter and the pit (above).
- **A questline's premise.** A standalone contract that tells part of a questline's story isn't
  offered while that questline runs (`NotWhileQuest`, set only when no questline started it); The
  Real Orders also stays away once Await Relief has ended (`NotWithQuest`). The pairs are in
  [QUESTLINES.md](QUESTLINES.md#contracts-that-wait-for-a-questline).
- **After the cure** the research jobs outside Halcyon stop (`m_bNotCured`: the Red Hand's samples,
  Bastion's shipment, the packs' jobs, Take the Lab), and the Silent Village has no Halcyon way.
- **Shelling** (Bastion Shelling, the Fire Mission contract) keeps 300 m from where people live
  (`IsNearLivedIn`), as the Fire Mission perk does.

## The shared engine

Code: `Core/RZ_Shared.c` (`gm.GetShared()`, **Shared** settings), and the files named below. Server
only.

The pieces every later system stands on (errands, the mission style, dialogue's trees, the majors,
the finale, the epilogue week), built once so no system keeps its own copy. Each is built and has its limits and save; most of their writers
and readers come with the phases that use them, and each part says which are wired today.

**The one body budget** is the infected's agent budget: a body that isn't an infected asks
`RZ_Shared.CanSpawnBody(kind, pos)` and is entered with `AddBody`, so it counts inside the same 100
agents. The kinds, their caps and the callers wired today are in
[INFECTED.md](INFECTED.md#the-one-body-budget).

### Clocks

Code: `Core/RZ_Clocks.c` (`RZ_SeasonClock`), `Puzzles/RZ_GameTime.c`. One table says which clock each
rule uses:

| Clock | What it is | Used for |
|---|---|---|
| **Season days** | The first three stages' set lengths ÷ 30 of minutes played: 480 minutes (8 hours with someone online) with the built-in stages, about one real day at the 45-day pacing, ÷ the season speed. Counted only while someone plays (`RZ_Director.GetPlayedMinutes`, saved; an older save starts from the stages' set lengths). `RZ_SeasonClock.Days()` (a fraction), `Day()` (from 1), `DaysSince` | World rhythms: how long reasons, signals and callbacks last, the contracts lane of the season budget, pinned life records |
| **Played time** | The life's `m_fLifeSeconds` | Personal pacing: when a callback comes |
| **Real time** | `RZ_SeasonClock.Now()`, unix seconds | Anti-abuse limits (a failed threat's bar), the epilogue week (a callback's 24 hours) |
| **The game clock** | `RZ_GameTime`: the hour, night and dusk, and `RealMinutesFor(hours)` / `RealMinutesUntil(hour)` turning game-clock windows into real minutes | Evenings (the set-piece calendar), windows |

The game clock's rate is read from the time manager's day duration (24 game hours ÷ its real
seconds, as the base game's own clock does), so it follows **Day length** ([The clock](#the-clock)),
a scenario's day and night acceleration and an admin's change. A window is timed at the rate the
clock runs now. If the clock can't be read, `RealMinutesFor` uses the Day length setting's rate,
and the windows that need a running clock (`IsCalibrated`, the `RealSeconds` helpers) wait. The
start-up `Clock:` log line says the day's length.

### Checks and ways

Code: `Missions/RZ_Checks.c`, `Missions/RZ_Ways.c`. One helper for every check, one set of ways
through a job. Scores are fixed, never rolled, so an option's tag can say what it needs before you
pick it. Tags show costs and requirements, never outcomes.

**Trust** (the persuade score, `RZ_Checks.TrustScore`):

| Part | Value |
|---|---|
| Your access rank with the listener's faction (mixed tiers one lower, as built) | Stranger, Uneasy 0 · Known, Tolerated, Wildcard 1 · Trusted, Respected 2 · Champion 3 |
| A background the option lists | +1 |
| From dialogue (remembered help, or a vouch) | at most +1: dialogue's trees pass regard 2 or more, or the built helped or saved memory of this life ([DIALOGUE.md](DIALOGUE.md#checks-and-tags)), or a vouch from someone they trust standing by ([DIALOGUE.md](DIALOGUE.md#several-players)) |
| A lie found out, or a questline you worked against them (the life records `lied.<f>`, `crossed.<f>`) | −1 each (nothing writes them yet) |

Hunted or worse: they won't be persuaded. People with no faction read your score with the Haven, at
most 2. The built `RZ_QuestApproach.PersuadeScore`, which questline stages use, is untouched.

**Fear** (the one threat score, `RZ_Checks.FearScore`): the higher of your **Feared band** (Infamy
earned this life with the listener's faction: 15–39 gives 1, 40+ gives 2) and your **backing**
(Earned Fame with the giver, when the giver isn't the listener: 15–39 gives 1, 40+ gives 2), +1 for a
listed background (Ex-Con, Raider, Soldier by default). Carrying a gun adds nothing. People with no
faction read your highest earned Infamy with anyone, at most 2. ([REPUTATION.md](REPUTATION.md#reputations)
explains Earned Fame and Feared.)

**A threat** (`RZ_Checks.Threaten`): Fear against the difficulty, and always +3 Infamy with the
listener's faction (a scene may name up to 8) and the life record "pressed". Refused without a check:
a stronghold leader; a person already threatened this season, by anyone; a speaker this player
failed to threaten in the last real day; and past three threats a life with that faction
(**Reputation > Threats per life**), which still costs the Infamy. A refused threat bars that speaker
for a real day. Saved in the season flags (`threat.<person>`, `threatbar.<ident>.<person>`).
Also refused while you're passing in the listener's kit. At a leader or a leader's people
(`FearDifficulty`): a Ruthless listener is one harder, a Paranoid one one easier, and a faction you
worked against in a questline ending one harder (the life record `crossed.<f>`), never under 1.
Free-standing talk threats are never made at a safehouse or inside a stronghold's or the lab's wire
(`ThreatRefusedAt`); a threat a mission writes into its own scene stays. The Ledger's and Debt
Collection's threats now run this check (+3 Infamy whatever happens; refused, they don't give in).
**The quiet success** (**Reputation > Quiet success**, on, at Fear 2): a questline stage's built threat
tries Fear first (`m_iMenaceDifficulty` set in the stage's Setup, `TryMenace`). Passed, the stage gets
its better outcome, still recorded as FORCE; failed, the built outcome, as before:

| Stage | Quiet success |
|---|---|
| Cargo: the driver (Threaten him) | He tells it all and goes back to his van: the camp isn't on guard |
| The Well Story: collect (Lean on them) | Said once, quietly: nobody comes out of the houses, no fight, no Haven Infamy for the lean |
| The Book: read the page (Lean on them) | They pay and tell nobody: no Infamy for the lean with their faction |
| The Three Weeks: the driver (Threaten him) | He names the site and won't tell the houses: no Haven Infamy, and the wanderer doesn't remember it as a wrong |

The threat's own +3 Infamy stands either way.

**The look** (`RZ_Checks.LookScore`): the disguise check, fixed and shown before you commit
([REPUTATION.md](REPUTATION.md#borrowed-kit)). **A lie** (`RZ_Checks.Lie`): a tell catches it first
(symptoms for a bite lie, other colours for "I'm with them", a different claim already held), else
Trust against the difficulty.

**Results.** Difficulty 1 (easy), 2 (normal) or 3 (hard): at or over it **Pass**; one under
**Short** (a halfway deal at the price the option names); two or more under **Refused** (that person
won't hear that approach from you again this job).

**The price table.** One value is 5 Favor; food is 1, medical 2, a car part 3 (`ValueOf`,
`FavorFor`). Tags: `TrustTag` "[Trusted - Haven]", `FearTag` "[Threaten - +3 Bastion Infamy - Fear 2 of 2]"
(or "- this makes you Hunted]" when the cost would take you there; never the outcome), `PriceTag`
"[4 food | 20 Favor]".

**Ways** (`RZ_Ways`, one per job, keyed by identity so a rejoin keeps everything): one try per person
per job per identity for every check (`TryTrust`, `TryPersuade`, `TryFear`); someone you threatened
or who refused you won't talk to you for the rest of the job; who took which approach (`Remember`,
`Did`); bribes (`TryBribe`); the quiet threat on a built FORCE option (`m_iMenaceDifficulty`,
`TryMenace`); a place's stealth (`m_Quiet`). Questline stage missions (`RZ_QuestMission`) hand their
`TryPersuade`, `TryBribe`, `Remember` and `TryMenace` to theirs. The approach enum
(`RZ_EQuestApproach`) has **PRESS, TRADE, TRICK and LEAVE** appended once, in that order, so saved
questline runs keep their numbers; built threat stages keep recording FORCE. **A trick**
(`PassAs`): a claim in someone's kit judged by the look at the mission's difficulty with its paper as
the cover detail, once per player per person per job and one mission trick per life per faction per
season day; recorded as TRICK.

### Deeds

Code: `Core/RZ_Deeds.c`. One store, two scopes:

| Scope | Keyed by | Holds | Ends |
|---|---|---|---|
| **Life** | the character (on the profile) | per faction: the best deed (the phrase of the biggest single Fame gain, with its season day), the worst (the biggest direct Infamy gain), their people killed, contracts done; up to 32 life records ("pressed"), bad ones pinned 3 season days | with the life |
| **Season** | identity (`RZ_SeasonDeeds`) | per faction: the best and worst reputation cell, the biggest deeds, kills, jobs, the Infamy forgiven; a ledger of the last 24 jobs (what, for whom, against whom) | with the wipe |

A bigger deed replaces a smaller one; an equal one keeps the first. The worst deed reads as the kill
tally ("put 9 of ours down") once three times the kills outweigh it. Written today: kills (retaliation
kills, leaders), jobs and the ledger (every paid contract), the cells (every reputation change), the
Infamy forgiven, "pressed" (threats), a leader's killing ("shot Vlasak"), running a checkpoint ("ran
the checkpoint at Morton"), and each paid contract's phrase (`RZ_Mission.m_sDeed` with `{town}`,
"brought medicine to Levie"; the rest by class, `RZ_RepDeeds`, and every questline's and major's
ending). Reputation's voice says them back ([REPUTATION.md](REPUTATION.md#what-people-call-you)).

**Callback deeds** (life scope, `RZ_Deeds.Callback`): twelve weighted slots for choices that involved
a person, a house or a leader. **Heavy** (3): a questline's or major's ending (its words), a one-choice
job about a person's life or freedom (*"decided whether he lives near Morton"*). **Medium** (2): the
other one-choice jobs (*"decided which house gets the food near Levie"*). **Light** (1): an errand
done (*"did Mara Kovac a favour"*). The same key again keeps the heavier weight. Full: the lightest,
oldest goes; light deeds hold at most four slots, and a new deed never pushes out a heavier one, so a
day of small kindnesses can't wash the record clean. A leader brings one up plainly, one time in
three after the openers that come first, at most once a visit (a real day per seat): *"You're the one
who decided where the kids go near Levie."* (`RZ_RepVoice.Opening`, Trees on). Memory outlives deeds:
leaders' memory runs across lives (earlier ones at half, and the name from an earlier life,
[REPUTATION.md](REPUTATION.md#what-people-call-you)); the deeds end with the life. *Simplified:* a deed
doesn't open a side quest by itself; errands' threads and callbacks and missions' fallout do
([ERRANDS.md](ERRANDS.md)).

### World memory

Code: `World/RZ_WorldMemory.c`, in the world facts (`facts.Memory()`), saved with them. Four
timescales:

| Part | What | Limits |
|---|---|---|
| **Reasons** | Live facts with a subject and a place (`house.hungry` at Levie), posted by the system that owns the fact: why a job is offered now. `PostReason`, `ClearReason`, `FindReason`, `ReasonsNear` | 32; each lasts at most a season day, or until cleared |
| **The signal ring** | Recent events (a shot near a farm, a body on a road) with a cause that stays on the server. `Signal`, `RecentSignals` | 48; 8 of a kind |
| **The town log** | Per settlement the best and the worst thing that happened there, each with its kind, salience (1–5, +1 when three or more took part), day and up to 2 names frozen at the deed; and up to 16 identities who did something there. `LogTown`, `Visit`, `TownBest`, `TownWorst` | a higher salience replaces, ties keep the older; an identity is named in one entry a town and 6 in all, then written unnamed |
| **World records** | One per story key and subject: a value, the day, up to 4 players with frozen names, an optional epilogue line and its carrier. `Record`, `FindRecord`, `Pin` | 160; evicted by importance (the source: story 3, one-choice 2, errand 1; +2 a named person; +2 first of its kind; +1 a player, 3 at most; +1 an epilogue line), lowest then oldest, never one a reader holds |

**The truth deck** (in the same store): what's really going on behind a story job or a card, one
per story key and subject, drawn once from the season's seed (so the same subject gets the same
truth after a restart), "it's as they said" never under 30% of the deck, kept 3 season days or
until used up (`DrawTruth`, `FoundTruth`, `UseTruth`). 64 at most.

**The writers** (`World/RZ_WorldWriters.c`, one line in the system that owns each fact):

| Part | Written by |
|---|---|
| Signals | bites (a player's own: one per 10 minutes), deaths (the background and how long they lasted, never the name), supply trucks lost (who robbed them as the server-only cause), shells (the Fire Mission perk and contract; who called them as the cause), burned buildings, trial doses |
| Reasons | `house.hungry` (safehouse upkeep: short posts it, fed clears it), `house.truck_lost` and `post.truck_lost` (supply trucks), `town.infested` (a town turning infested or overrun, checked every 30 s), `town.shelled`, `leader.died`, `wanderer.bitten`, `cure.<percent>` (the lab's milestones), `house.bitten` (a bite declared at a safehouse, or a man walked out of it), `house.lost_to` (errands' Somebody Should Tell Them), `horde.passed` (a pack's move, The Tribe Moves, at the town nearest its way) |
| The town log | a contract or event finished in a town (the best slot: salience 2, 3 for an event or a questline stage, the paid players named) or failed there (the worst), people brought in (3), a town burned (4), an outpost taken (3) |
| World records | errands ([ERRANDS.md](ERRANDS.md)), and who benefits (`<key>.went_to`) |

**Season marks**: a town log entry of salience 3 or more puts a mark on the verge of the road into
its town ([Marks](#marks), the season pool), with the entry's line (Traces opens it with what you see
and picks its prop by kind, [TRACES.md](TRACES.md#player-traces)); with no prop (Traces' kit, else **Shared >
Season mark prefab**) it's text only, said once a session to a player passing within 40 m. Marks aren't saved:
they're put back from the log at start. The epilogue reads the log and the records (its town and
side-story beats, [EPILOGUE.md](EPILOGUE.md#beats)). Log lines:
`World memory: reason <key> (<subject>) <text>`, `World memory: town <idx> <kind> <text>`.

### Lies and witnesses

Code: `World/RZ_Lies.c` (`RZ_Lies`, `RZ_Witness`), saved with the world memory. One lie store for
missions, errands and dialogue: per player per season, 12 at most (the oldest goes), each with what
would expose it. **Found out by events first**, never by hidden dice:

| Test | What exposes it |
|---|---|
| Visit | A player Known or better with the faction lied to comes within 30 m of the subject's place |
| Road news | A wanderer passes within 300 m of it |
| The owner's tick | The owning system checks (`Lies().Test(key, subject)`): the next upkeep, the next collection |
| Rumour | Only a claim nothing in the world can test: 15% at the next half-hour round and once more a real day later (+10 for a Talker), then it's buried |

Found out, on the life it was told in: the speaker's regard *lied*, Infamy with the faction (2–5; a
mission may name more), the life record `lied.<f>` (the liar count, −1 at Trust), and a voucher answers
for it (+5). The life gone, the lie is buried with it. A lie never pays more than the truth: at most
60% and no Fame (each caller's rule). The checks run on the errands' 30 s tick, errands on or not.

**The witness rule** (`RZ_Witness.Seen`): "if seen" always means the act faction's people near the
act (its holdings and road posts within 60 m; for the Haven a safehouse within 60 m or a wanderer within
40 m), never a hidden player (stealth). Where no bodies are, a witness is rolled: 25% within 300 m of a
lived-in place, else 5%. **Knowing:** dark acts count only when you knew: the tell was named to you, or
the reply says it plainly; being tricked by a twist you never saw costs nothing.

Log lines: `Story: lie <key> told by <player> (<test>)`, `Story: lie <key> found out by <how>`,
`Story: lie <key> caught now by <speaker>`.

### Callbacks

Code: `Core/RZ_Callbacks.c`. One queue for missions' echoes, errands' follow-ups and dialogue's
"later". A callback is words only: no check bonuses, discounts or gifts.

- **A character's:** `Queue(player, title, text, subject, hours, boardNote)`: due 2–6 hours of that
  life's play later (or the hours given), told while the character is within 1 km of the subject, at
  most one per hour of play. 10 waiting per player.
- **The system pool:** `QueueWorld(title, text, subject, days)`: told to the first player near the
  subject once the season days have passed. 100 waiting.
- **Expiry:** 3 season days; in the epilogue, 24 real hours.
- **At death** (or expiry) a waiting one with a board line becomes an unsigned note on the nearest
  safehouse board, filtered like any note.

Saved with the season (**Shared > Callbacks** and the numbers above). The story jobs queue theirs
(`RZ_StoryKit.Later`), as do errands and dialogue.

- **At the door** (`Missions/Story/RZ_DoorCallers.c`): a story job's callback can be a person: when
  it comes due within 300 m of its place, a stand-in (the civilian puzzle prefab) turns up 15–25 m from
  the player, somewhere they can't see, as the line is told, and goes after 90 s once nobody can see
  them (5 minutes at most). One at a time on the server, an `asker` in the body budget
  ([INFECTED.md](INFECTED.md#the-one-body-budget)); no prefab, no room, no hidden spot, or further
  off, the line alone.

### Notice lanes

Code: `UI/RZ_NoticeLanes.c`. Per player, on the server, on top of the client's notice pacing
([UI.md](UI.md#notices)): pacing decides how a notice is shown, the lanes decide whether something
that speaks unasked may speak now.

| Lane | Rule |
|---|---|
| Critical | Urgent notices: always shown |
| Talk | Owner-only, at once: the talk screen, answers to what you asked |
| Contact | Unasked contacts: at most one every 4 minutes for a player across radio offers, leaders' pings, wanderers, the Long Table's host and errands, with errands last (they wait a minute longer, so any other due contact wins) |
| Bark | One a minute, one per speaker per 10 minutes |
| Ambient | Approach and ambient lines: once per place per session |

**Notices, one per approach:** the gate's own line first (a gate bark, with a rival-colours remark
folded into the same line, `TryGate`), then a change notice, then a tip; anything lower waits until 30 s
after the last approach line (`ApproachHolds`), and an ambient line held past 2 minutes is dropped. A
held change notice waits on its own tick and is never dropped; a held tip is kept (one per player) and
tried again each tick, dropped past 2 minutes.

**The budget is a ceiling only:** it can stop a contact, never start one. A caller asks
(`TryContact`, `TryBark`, `TryAmbient`, `TryGate`) and stays quiet to try again later. Wired today: a leader
catching your eye about work, and the Long Table's host stepping out on the road. Runtime only.
(**Shared > Lanes**, contact gap 4 min, bark gap 60 s, speaker gap 10 min.)

### Marks

Code: `World/RZ_Marks.c`. One prop spawner for every physical thing that marks the world, in separate
pools: **season** marks 40 (the town log's, memorials), **epilogue** 100, **props** 30 (missions'
props and chalk, the ambient notice prop, the Long Table's tells), **signs** 30 (the tribes'), **doors**
24 (Traces' boards, 8 a cluster, up within 60 m and down past 90 m) and **traces** 40 (Traces' graves,
ash and candles and its scenes' props, **World > Traces > Props**, 12 a cluster; [TRACES.md](TRACES.md)). Traces' two pools
put every prop through the dressing guard and count it as scenery; a caller may give the facing. A mark
is data: a place (on the verge, 6–10 m off the nearest road's centre line within 80 m, on the side the
place is on), a prefab, a line of text, a salience, how many season days it lasts. Its prop is spawned
only while a player is within 150 m, stays up at least 10 minutes, and goes once nobody is within
200 m. A full pool gives up its oldest lowest-salience mark that isn't protected; with no room, or no
prefab, a mark is text only (`TextNear`, an approach line). Nothing is saved: owners put their marks
back. The town log's season marks are the first ([World memory](#world-memory)); Traces gives them a look
and an opening line by their kind ([TRACES.md](TRACES.md#player-traces), P5).

### The season budget

Code: `Director/RZ_MajorBudget.c` (`RZ_MajorBudget`). How far anything outside the built background
simulation may move a faction, in lanes. "±20" is a band on the season's net change. Anything clipped
is logged (`Budget: <lane> morale <n> for <faction> clipped to <n>`) and becomes words only.

| Lane | Morale | Strength | Draws on it |
|---|---|---|---|
| Majors | ±20 per faction a season; ±15 per major; ±10 a call | ±8; ±5 per major; ±5 a call | the majors ([The majors](#the-majors)) |
| Contracts and errands | ±10 per faction a season day | ±4 per faction a season day | every contract's morale and strength today; errands (their morale, `RZ_Errands`) |
| The finale | ±10 per faction, never clipped by the majors | ±5 per faction | the finale's terms ([FINALE.md](FINALE.md#the-release-terms-and-deciders)) |
| The epilogue | ±5 per faction, inside the majors lane | — | the grey choices and the Last Broadcast ([EPILOGUE.md](EPILOGUE.md#the-grey-choices)) |

Also kept: **the seat ledger** (contracts never take a leader's loyalty below 30, wired today; a
major swings at most two seats and a faction three a season, never a stronghold), the one outpost a
season that may change hands through the majors, research +3 per major, and the **season weights**
(`SetContractWeight`, combined and clamped to ×0.8–1.2). Saved with the season (**Shared > Season
budget**).

### The set-piece calendar

Code: `Director/RZ_MajorBudget.c` (`RZ_SetPieceCalendar`). The one calendar. An **evening** is a game
dusk window (18:30–20:30) entered with a player online. One set piece an evening, server-wide:
`Hold(key, pos)` holds the evening until `Release(key)`; a set piece that finds it held waits for the
next free one, and after two waits (**Shared > Set piece waits**) `MustRun` says it runs at the next
free hour. Errands deal nothing within 600 m of the set piece holding the evening (`IsQuietAt`).
Saved with the season. The majors' set pieces book it ([The majors](#the-majors)), their codas in the
epilogue week too; the epilogue's own pieces (the supper reading, the 20:30 watch) are radio and scenes
([EPILOGUE.md](EPILOGUE.md#the-evening-calendar)).

## Town memory

Code: `World/RZ_TownMemory.c`.

- **Town flags:** Looted, Barricaded, Powered, Burned. Plain values per settlement, saved except
  Barricaded and Powered.
  - **Looted:** each town has a loot level (0–1). Every item taken from a house (and every find
    taken from a wreck's boot) takes a little, and the town counts as Looted under 0.5. It never drops under
    0.2 and comes back over about 2 hours.
  - **Barricaded:** while a player's barricade stands in it, or once Fortify the Safehouse has
    succeeded there. Not kept across a restart (barricades aren't saved).
  - **Powered:** set by the power grid.
  - **Burned:** once half its buildings have burned (a building count guessed by size: 12, 30
    or 60). Each burned building stops counting an hour after it burned (restored ones, an hour
    after the restart), so towns recover.
- **Hooks** for other mods: `GetLootLevel`, `ReportSearched`, `ReportBarricaded`,
  `ReportBuildingBurned`, `ReportBuildingCount` (unused so far).
- **Journals of the dead.** A life that lasted 10 minutes leaves one line (where, background,
  how long, the day, a personal line). The last six are saved; they turn up in the session news
  and in wanderers' talk. Never an object.

## World interactions

Code: `Interactions/`.

Loot lying in houses and on bodies, wrecks' boots to open, locked doors and keys, cars, crafting,
water, barricades and traps, generators, fire, bells, radios and a medic slowing a bite. Data until a
player is near, capped and decaying, nothing left behind.

**How it works on any map.**

- **Things are recognised by name.** Every 10 s the server looks at static map objects within
  30 m of each player (buildings within the loot's 55 m) and sorts them by prefab or mesh name:
  buildings by type, car wrecks, workbenches, wells and pumps, fuel pumps, stoves, bells (every
  church gets one), sirens, house radios and generators. Furniture is where loot lies.
- **Interaction spots, only near players:** an invisible prop with labelled actions ("Ring the
  bell"), at most 10 per player and 70 on the server. Buildings, furniture and bodies get none.
  A thing's spot stands at the middle of its bounds (many props have their origin at a back
  corner), at about counter height (its own height, 0.6 to 1 m), pulled up to 0.35 m toward its
  open side, the side with the most free room beyond it, so a stove's prompt is on the stove, not
  on the wall behind it. A wreck's spot is its boot instead, a storage you open with vanilla's
  inventory ([Car wrecks](#car-wrecks-one-boot-to-open)), and with **Cars > Fuel pump station**
  set a petrol pump's is a vanilla fuel point ([Fuel](#fuel)).
- **Default first, then menus** ([UI.md](UI.md#default-first-then-menus)). A stove, a generator, a
  house radio, a well and a petrol pump without the station have one prompt, their look ("Look at
  the stove"), which opens the [object menu](UI.md#the-object-menu): what you see and the options
  as a list. Bells, sirens, a locked door, a fresh bite and a doorway to burn keep their single
  prompts (they're used in a hurry); a fixable car, a workbench and (with **UI > Cook row layout**
  set) a stove open their own screens. With **UI > Object row layout** empty, every option is a
  prompt again.
- **Decided on the server.** Loot lies in houses and wrecks' boots only while a player is near and
  is data otherwise; a boot is rolled when it's first put out, a body when it dies. What was
  emptied, opened or burned is remembered as positions and times, and saved.
- **Timed actions** run on the 5 s tick: move more than 3.5 m away and they stop.

### Loot in houses

Walk into a building and several things lie on its worktops, desks and low cupboards and on its
floors. Pick them up with vanilla's **Pick up** (or from the inventory's "nearby" list). There are
no search prompts in buildings and no timers. A fresh town looks lived-in and left in a hurry; a
picked-over one is thin.

| Building type (from the prefab name) | What it tends to have | Items (slots) |
|---|---|---|
| Pharmacy (pharmacy, hospital, clinic) | Field dressings, gauze, aspirin, tourniquets, saline, medical gloves; rare: morphine, a medical bag | 8 |
| Garage (garage, workshop, repair, fire station) | Jerrycans, the wrench kit, torches, work gloves, a hard hat, overalls, maps | 8 |
| Farm (farm, barn, stable, shed, vet clinic) | Food, canteens, wool gloves and hats, raincoats, jerrycans; rare: binoculars, a compass, a backpack | 7 |
| Police (police, gendarmerie) | Pistol and rifle magazines, torches, dressings, tourniquets, caps, gloves; rare: a pistol, a vest, a helmet, a radio | 8 |
| Shop (shop, store, market, kiosk, petrol station, pub) | Food, canteens, torches, watches, maps, gloves and hats | 7 |
| House (anything else) | Food, water, dressings, aspirin, a torch, a watch, a map, clothes | 5 |

Every destructible building has loot, whatever else its name makes it (a petrol station's
building is a shop).

- **What it's rolled from.** Each type's table is the hand-set loot table for it, else the
  **Loot pool** items whose prefab names hold the type's keywords, always with the **built-in
  vanilla items** for the type added (the table above; `RZ_LootDefaults` in
  `Interactions/RZ_Search.c`), so a type's table is wide and no type is bare when the pool is
  short or empty. A list set by hand is kept as it is, topped up with the built-in items only
  while it's thin (fewer than 8 everyday finds, or 3 rare ones). The log's `Search:` line gives
  each table's size.
- **Rolled once, then data.** The first time a player comes near, each of the building's slots
  holds something **75%** of the time (**Loot > Slot chance**) × the town's loot level, half that
  in a faction's town (a holding within 150 m). The first two slots (**Sure items**) always do
  while the town's loot level is 0.5 or more, one in a picked-over town. So a house in a fresh
  town has 2 to 5 things, about 4, and one in a picked-over town one or two. No prefab is rolled
  twice in one building unless it comes in twos (food, magazines, dressings, gauze, tablets): a
  slot that keeps drawing one already there stays empty. 10% of finds are rare (20% behind a
  lock), 10% electronics (×1.5 in powered towns). A locked place (a seeded lock on outside doors
  that were found) has one more slot, +25% per slot, and better finds. There's also one roll for
  a treatment (expired antibiotics, suppressant, vet antiviral, stabiliser, antidote, from a
  configurable list: [Treatments](#treatments)), sometimes the town's fixable car's keys
  ([Cars](#cars-found-rare-and-hard-to-fix)), and 12% of the time something to read
  ([Readable things](#readable-things)).
- **One building, one roll.** The look-around also sees a building's parts whose names say
  "house": its doors, its light probe and its furniture prefab. A part counts as the building it
  hangs from, a part with no mesh (the probe, the furniture prefab: vanilla doesn't attach them to
  the building) counts as nothing, and a plain house mesh standing inside a destructible
  building's box is a part of it. So no building is rolled twice into its own rooms.
- **Kept clear.** Burned buildings have none. Nor does a faction's own place: a holding's camp as
  far as its building, yard and pieces reach, a Haven safehouse, a road post and the lab
  (`RZ_WorldFacts.IsNearLivedIn`, the same ground shelling and fire keep away from), nor anything
  within **20 m** past it (**Kept clear**) or within 40 m of a quartermaster. So a faction's own
  buildings are never free loot, and the rest of its town has loot at half the chance.
- **Where it lies.** Up to 20 spots per building, found once and kept. First vanilla's own item
  spots (the few furniture sets that carry them) and the furniture: vanilla's furniture stands in
  the building as props (some of the building's children, the rest found by a query over every
  entity in its box), recognised by name (cupboards, desks, tables, worktops, counters, shelves,
  beds, sofas, crates). On a piece no taller than 1.6 m a spot is its top (a trace down at its
  middle), else the floor beside it toward the middle of the building; furniture takes at most
  two thirds of the spots, picked in a random order. Then floor points on a 1.6 m grid over the
  building's box, on every storey: the ground floor, each height where two or more pieces of
  furniture stand (up to four storeys), or without furniture upstairs a guess at the floor above
  (2.6 to 4.6 m up). Each storey gets its share of what's left. Every spot is traced down to a
  surface inside the building's box, has a ceiling within 5 m over it (indoors: not on the roof
  or in a yard), is 0.3 m clear of walls, and is 0.5 m from the next. A spot inside a smaller
  building's box (an add-on, a garage against the wall) is that building's, so two buildings
  never put their loot in one room. A trace that starts inside a wall or a piece of furniture
  finds nothing. One item per spot, in a shuffled order. Vanilla's furniture has no inventory of
  its own, so nothing is put inside it.
- **Only near players.** A building's loot is put out when a player is within **55 m** of it
  (the look-around reaches that far for buildings, 30 m for everything else), so it's there before
  you walk in: **6** buildings a tick, the nearest first. With nobody within **80 m** for a
  minute, untaken items are deleted and kept as data: they're there again next time. At most
  **500** items lie on the server; when it's full, the building farthest from players is put away
  for a nearer one.
- **Never in front of you.** A building is only rolled fresh (refilled) with nobody within
  **20 m** (**Fresh roll distance**); until then what was left there is put out as it was.
- **Taken.** An item that goes into an inventory (or is carried more than 1.5 m off) is taken:
  it leaves the building's list, the town's loot level drops by **0.01**
  ([Town memory](#town-memory)), and from then on it's an ordinary item (dropped and forgotten,
  it's cleaned up like any [loose item](#loose-items)). Taking things is silent: no heat.
- **Refill.** An emptied building has loot again after **45 minutes**, divided by the town's loot
  level (up to three hours in a picked-over town). One with leftovers keeps them, and rolls fresh
  after three times that.
- **Saves.** Only each building's rolled and emptied times are saved, never the items: after a
  restart a building rolls again once its refill time has passed.
- **A stash someone wrote about** ([Readable things](#readable-things)) adds two good finds to
  the building nearest it, the first time it's put out (none already lying there).
- **The loot log** (**Loot log**, on): a `Loot: put out` line for each building put out (its
  key, type, building, spots and what lies there) and a `Loot:` summary every 2 minutes, for
  checking loot in play ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check)).

*Simplified:* the furniture names are guesses, a storey is read from where the furniture stands
(the props' origins at their feet), and spots are traced once per building, so an item can sit on
a low shelf or in a corner. Wrecks' boots and bodies hold their things (below).

### Bodies and car wrecks

| What | What it tends to have |
|---|---|
| Body (an infected killed within 60 m of a player) | Bandages, ammo, small things; sometimes a key or something to read |
| A wreck's boot | Jerrycans, tools, rope, batteries, tyres; sometimes a car part |

**Bodies are looted with the vanilla inventory.** There's no prompt: when an infected dies within
60 m of a player, what it carries is rolled at once: something 60% of the time, sometimes two
things, plus the treatment roll; a key 8% of the time; something to read 30% of the time. The
items go into the body's own inventory (open it as you would any body's), or lie loose beside it
when the body has none or nothing fits. A note lies beside the body and is read where it lies; a
key taped inside lies next to it. A special's sample drops where it fell, as before. At most
**Body cap** (8) bodies a minute get anything, so a horde shot down at once doesn't cover the
ground. Loose, the things are cleaned up like any [loose item](#loose-items); in the body, they go
with it. Bodies don't use up the town's loot.

#### Car wrecks: one boot to open

A wreck is a burnt-out shell: there's no fuel in it and nothing to strip. It's one container, its
boot. **Open the boot** (vanilla's open-storage action, on the wreck's rear) opens the vanilla
inventory with the boot's things in it, as for any storage box or vehicle trunk, and you drag out
what you want. There's no other prompt and no timer.

- **Data until a player is near.** While the wreck's spot is out (a player within 30 m of it, kept
  until everyone is 55 m off), an invisible storage (**Cars > Wreck boot prefab**) stands just
  behind the middle of the wreck's rear at boot height, filled from the wreck's record. When
  everyone has gone, what's left is kept as data and the storage is deleted with it.
- **Rolled the first time it's put out:** something 80% of the time (**Search > Boot chance**,
  scaled by the town's loot level) from the car boot table, and then a second thing 40% of the
  time (**Boot two chance**, scaled the same way), never the same prefab twice unless it comes in
  twos; two good finds more for a stash someone wrote about; the treatment roll; a car part (a battery, a tyre or spark plugs) 30% of the time
  (**Wreck part chance**); and 12% of the time something to read, left beside the boot (a key taped
  inside goes in the boot).
- **Taken.** On the 5 s tick a thing no longer in the boot was taken: a loot find takes 0.015 off
  the town's loot level (the part, the treatment and keys don't), and from then on it's an
  ordinary item ([loose items](#loose-items)).
- **Refill.** An emptied boot has things again after **Wreck return minutes** (2 hours). One with
  leftovers keeps them and rolls fresh after three times that. Only the time is saved: after a
  restart a boot counts as emptied then.
- **Take only.** Nothing from your own inventory goes into a boot, so a wreck is never a stash
  (something dragged in from the ground is kept as data like the rest).

*Simplified:* the boot's point comes from the wreck's bounding box, taking its rear as the box's
back (local −Z, as for vehicles); a wreck modelled the other way round has its boot at the bonnet.
A thing kept as data comes back as a fresh copy of its prefab (a magazine full, a bag empty).

### Locks and keys

About half of pharmacies and police stations, a third of garages, a quarter of shops and a tenth
of houses are locked, seeded from the building's position so the same ones are locked after a
restart. **The lock is on the building's outside doors**, and the loot is inside. Places people
use are never locked: nothing within 80 m of a safehouse, the lab or an outpost, 100 m of a
quartermaster, or 150 m of a stronghold (the same places that can't burn). **Nothing within 25 m of
a running mission's props is locked either** (its position, its props, items and drops, the current
chain step, a capture's cage): its buildings open while it runs and lock again when it ends. The
one exception is a lock that is part of the mission (the Locked Bunker's keypad).

| State | How it gets there | What it means |
|---|---|---|
| Locked (the default) | | The vanilla Open action on an outside door says **Locked** from outside |
| Unlocked | **Unlock** at the door with its key (quiet), or opening the door from inside (the latch) | Its doors open for anyone |
| Locked again | **Lock** at the door with its key | Locked; the door pulls shut once you step away |
| Forced | **Force the door** with a crowbar: 10 s, 10 heat at the start and 25 when it gives | Open and broken: it can't be locked until the reset |
| Reset | Three hours after it was unlocked, latched or forced | Locked again, its outside doors pulled shut |

- **Which doors.** Only outside doors: a door entity of the building that stands within 1.2 m of
  the building's outline (its bounding box). Interior doors never lock. Doors are data: they're
  looked at only when a player comes within 150 m of a locked place (one look per player every
  30 s, after moving 40 m), or tries one.
- **At the door.** Each outside door of a locked place gets an interaction spot on the door
  itself, with Unlock, Lock and Force (at most three per building; a double door shares one, on
  its first leaf). It sits on the shut leaf at handle height (the height of vanilla's own door
  action point when the door has one, else 1.05 m), on the leaf's middle line, 8 cm out from its
  outside face, so the prompts are on the door beside vanilla's Open door at the handle. The
  **door spot prefab** is the spot with a tiny interaction box and a 0.35 m action radius, so it
  doesn't cover the handle. A spot placed while its door stood open (on the open leaf) moves onto
  the shut leaf at the next look-around (after you've moved 10 m). Looking at it, or trying the locked door, tells you what the key's tag says.
- **Inside always opens.** Inside means anywhere in the building's bounding box, at least 0.5 m
  in from its sides, or past the door toward the middle of the building and within its width.
  The first covers a shallow closet behind a door near the outline. Shutting a door is never
  refused.
- **Enforced twice.** The client greys the action out from a replicated list of locked buildings
  within 80 m of a player; the server refuses it again. Locked doors found open (AI opened them)
  are pulled shut once nobody stands within 2.5 m.
- **Keys** open one kind of building in one town: "Pharmacy, Lindale", "Shop, back door, Morton";
  houses and farmhouses by the colour of their door, "Blue house, Morton". One lies next to one
  readable thing in five (it was taped inside), and bodies carry one 8% of the time (in the body or
  beside it). A key is only
  ever cut for a real building with its own lock on outside doors that were found, in the town
  nearest where it turns up (not burned out); a town with none gives no key. Car keys lie in houses
  in their car's town, or come from the Mechanic. **Keys are real items** (the key prefab): they
  can be dropped, traded and taken off a body, the id stays on the server, and the tag can be read
  on the ground. Without a key prefab there are no keys.
- **Mission spots stay outside.** Missions place their items, props, people and crates on random
  ground points (the shared pickers `LandPointNear` and `HighPointNear`). With locks on, a
  point inside a locked building's bounding box (plus 1.5 m, which covers its doorways) or within
  2.5 m of a doorway blocker is rejected, and the last resort is moved out past the nearest wall.
  The landmark finder skips locked buildings too. A building counts while it's unlocked as well,
  since it locks again at the reset. The one exception is the Locked Bunker event, where the
  lock is the point: it turns a locked building's lock into a keypad (no key fits and no crowbar
  forces it while the event runs; the inside latch still works) and puts its crates inside.
- **Doors only.** A building whose doors aren't found (no door entities, or no usable bounding
  box) isn't locked, and no key is cut for it. With **Search > Locks** off, nothing is locked.

#### Keeping AI and the infected out

AI and the infected don't use the door action: they follow the navmesh and open doors
themselves. Three layers keep them out of a locked place.

| Layer | What it does | Without it |
|---|---|---|
| **AI door hook** (optional file) | The AI's own "open the door" task is refused at a locked outside door, from outside. From inside they're let out, without turning the latch for anyone | Layers 2 and 3 still work |
| **Doorway blocker** | While an outside door is shut and locked and a player is within 150 m, a thin invisible collision box stands just outside the door leaf, and the navmesh around that doorway (a 5 m box) is rebuilt so AI route around it. At most 20 doorways on the server, the nearest to players first | With **Navmesh rebuild** off, blockers only: AI walk into the box and stop there. With no blocker prefab, no blockers |
| **Banging** | A tracked infected pressing within 1.8 m of a locked doorway (blocked or not) makes 4 heat, at most every 15 s per doorway. Players within 30 m are told, at most every 90 s | Silent |

- **Rebuilds are throttled.** Requests are queued, merged per doorway and sent two per 5 s tick.
  A doorway whose blocker came or went can't get one back for 60 s, so lock and unlock spam
  costs at most two rebuilds a minute per door. A key turns (Lock or Unlock) at most once every
  5 s per building, so spamming it can't flood the replicated locked list either.
- **Nobody is trapped.** A blocker appears only while the door is shut and locked, with nobody
  within 2.5 m of the door and no player inside the building. It goes at once on Unlock, Force
  or the inside latch, and on the next tick when a player is inside the building or on the
  inside of an open locked door. Blockers aren't saved; after a restart there are none until
  players come near.

*Limitations:* the AI door hook is a guess at the engine's class; if it's wrong the file is
deleted and only blockers and banging remain. Locked places more than 150 m from every player,
or past the 20 nearest, have no blocker, so AI there still open their doors (the doors are pulled
shut again once a player comes near). Each blocker costs two small navmesh rebuilds (on and off);
how much that costs on a busy server isn't measured yet. A blocker is one fixed size, so a very
wide door may leave a gap. Buildings whose doors are part of the mesh have no lock. Where the
building's box reaches past its walls (a porch, a deep recess), someone standing in front of a
recessed door, 0.5 m or more inside the box, counts as inside and can open it. Key items don't
survive a restart.

### Readable things

12% of buildings' loot (and of wrecks' boots, 30% of bodies) includes an ID card, letter,
receipt or photo in the Zone's voice ([WRITING.md](WRITING.md#the-voices)). In a building it lies
like the rest of the loot, as a note prop read where it lies, and goes with the building's loot;
a boot's is left beside the boot for half an hour, and a body's lies beside the body as long. One
in five had a key taped inside: in a building or by a body the key lies next to it, from a wreck
it's in the boot. About one in seven has a stash grid on the back (two good finds in the building
nearest it, or a wreck's boot filled there, within 90 minutes), one in ten a puzzle mission circled
on a map.
Reading one opens it in the reader, a sheet of paper you page through, as do puzzle notes, letters
and orders, guidance notes, safehouse flyers and a key's tag ([UI.md](UI.md#subtitles-and-the-reader)).
A paper read on a prop whose object menu opens (a note on a puzzle object) is that menu's text.

### Cars: found, rare and hard to fix

Cars are only ever found, never bought. One fixable car per three settlements (at most 6, one per
town); every other wreck is just a wreck, and is never fixable. A fixable car stands near players
as the car it will become (one of the **Car prefabs**, named after it: "the beige saloon"), parked
and locked so nobody can get in, with its prompts in front of its grille.

| Step | How | Settings (defaults) |
|---|---|---|
| **Look over the car** | Says what's missing: two or three of fuel, a battery, a tyre, spark plugs, all different (on the repair screen, below, where it's used) | **Min parts** 2, **Max parts** 3 |
| **Fit parts** | Battery, tyre and spark plugs (carryable items) lie in garages, are sometimes in a wreck's boot (30%) and come from the Mechanic wanderer. Fitting one needs a tool in your pack (a wrench, spanner, toolkit or toolbox), which isn't used up. Without one, the look and Fit parts say so | **Tool keywords** (empty = no tool needed); 8 s |
| **Fuel it** | Fuel from a can: a vanilla jerrycan gives 5 litres and is kept (filled at a petrol pump, [Fuel](#fuel)); an old-style full can found as loot is used up. Wrecks have no fuel in them. No tool needed | **Car fuel litres** 5 |
| **Hotwire** | Long and loud: 45 s, half its heat when you start and all of it at the end. It fails about one time in three: the starter grinds, the heat is made all the same, and you try again. A Mechanic does it in 15 s and fails one time in ten | 45 s, Mechanic 15 s, 20 heat, fail 0.35, Mechanic 0.1 |
| **Or its keys** | Only a fifth of cars have keys in a house in their town: each building rolled there holds them one time in ten, lying with its loot (picking them up is finding them). A Mechanic wanderer who likes you may hand them over | keys 0.2, find 0.1 |
| **Start the engine** | 15 heat. The parked copy becomes the drivable car with a quarter tank, and from then on it's vanilla | |

A new project turns up elsewhere six hours after one is started. Projects in a save keep the parts
they were missing.

**The repair screen.** With **UI > Car job layout** set, **Look over the car** opens the car repair
screen ([UI.md](UI.md#the-car-repair-screen)) instead of a notice, and the car's other prompts hide:
five jobs (spark plugs, battery, tyre, fuel, ignition), each with what it needs or what you have
and the button that does it, and **Start the engine**. Every job goes to the server, which checks
that the screen is open on that car and you're beside it, then runs the same code as the actions
above (the same tool, part, key and timer rules); what they would say shows in the screen. One job
fits one part (the **Fit parts** action fits every part you carry at once). A job under way goes
on if danger closes the screen. Without the layout, or where the screen can't open, the actions
work as before.

### Fuel

Fuel is vanilla's: a jerrycan with its own litres. A running car (a fixed one, or any vanilla
vehicle) is fuelled from a held jerrycan with vanilla's action; a fixable car's **Fuel it** job
drains 5 litres from one (what a fresh vanilla can holds), and the can is kept.

**Petrol pumps.** With **Cars > Fuel pump station** set, a petrol station's pump has no prompt of
ours: while a player is near, an invisible vanilla fuel point stands at it (`RZ_FuelPumpStation.et`),
and a held jerrycan fills there with vanilla's **Refuel canister**, as at any vanilla fuel point.
Each pump holds **Pump litres** (60) and is full again **Pump cooldown minutes** (20) after it was
emptied, filling back steadily; what's left is kept as data while nobody is near. Every 20 litres
drawn make **Pump heat** (10): the pump is loud. Without the slot the pump is the old hand pump, in
the object menu: an empty can (by its name), 30 s, half the heat at the start and all of it at the
end, a full can (**Fuel prefab**) given, once per pump every 20 minutes.

*Simplified:* a pump's litres aren't saved (after a restart every pump is full). Siphoning a car
isn't added: vanilla's jerrycan can be emptied with its own drain action, nothing more.

The Car Keys job ([MISSIONS.md](MISSIONS.md#unarmed-missions)) adds one more car for as long as it
runs: it already runs, needs only its keys (always lying in the one house the job names) or hotwiring,
isn't saved and doesn't count toward the replacement timer.

### Light crafting

At a workbench, eight built-in recipes, and more can be added as data (sixteen in all at most). A
recipe whose output prefab isn't set is hidden. A recipe takes its time at the bench (5 to 15 s;
moving away stops it) and some make noise (heat). Its inputs are checked when you start and again
when the work is done, each carried item counting for one input only; then each is taken from your
pack, and the item is given only if every one came out.

| Recipe | Takes |
|---|---|
| Improvised bandage | a rag or cloth |
| Molotov | a bottle, fuel, a rag |
| Tin rattle (noisemaker) | 2 empty tins, wire or string |
| Spear | a knife, a pole or plank, wire |
| Tin-can tripwire | 2 empty tins, wire |
| Spike trap | 2 planks, nails |
| Purification tablets | bleach or iodine |
| Purify water | untreated water, a tablet |

**The crafting screen.** With **UI > Craft row layout** set, **Use the workbench** opens the
crafting screen ([UI.md](UI.md#the-crafting-screen)) and the bench's **Make:** actions hide: the
recipes as a list, and for the picked one a picture of what it makes, what it takes against what you
carry, how long, and Craft. Craft goes to the server, which checks the screen is open on that bench
and you're beside it, then runs the same code as the actions; what they would say shows in the
screen. Without the layout, or where the screen can't open, **Use the workbench** lists what each
recipe takes as a notice, and the first eight recipes are **Make:** actions, as before.

**At a stove.** A stove, oven or fireplace boils untreated water (30 s), brews the **home remedy**
(alcohol and herbs, 20 s; at a stove since the workbench's eight action slots are full) and cooks
the **stove recipes** (**Interactions > Crafting > Stove recipes**, data like the bench's extra
recipes: for example Metabolism's raw meat in and its cooked meat out; none by default). Each is
hidden when what it makes isn't set; sixteen in all at most. Lighting the fire adds 2 heat and
marks a campfire for strangers, as boiling does, and a stove recipe adds its own heat when it's
done. Its inputs are checked and taken as a bench recipe's are.

**The cooking screen.** With **UI > Cook row layout** set, a stove's one prompt is **Use the stove**,
which opens the cooking screen ([UI.md](UI.md#the-cooking-screen)): the crafting screen with the
stove's list (boiled water, the home remedy, the stove recipes), what each takes against what you
carry, how long, and a button named for what it does (Boil, Brew, Cook), checked on the server
and run through the stove's own code. Without the layout, or where the screen can't open, the
stove's first four are its actions (**Boil water**, **Brew a home remedy**, **Cook: ...**) in the
object menu, as before.

### Barricades, traps and generators

| Kit | Per player | Lasts | What it does |
|---|---|---|---|
| Planks (barricade) | 3 | 60 min | Boards up a doorway. Infected against it make noise; 8 hits break it. The town counts as Barricaded |
| Tin-can tripwire | 3 | 60 min | Rattles (20 heat) when something walks through; the owner hears it within 400 m |
| Spike trap | 2 | 60 min | Kills an infected that walks onto it, three times. Players aren't hurt |
| Generator | 1 running | 30 min per fuel item | 6 heat a minute; lights hook within 40 m; a radio within 30 m hears every open channel; standing near it charges your radio and torch |

None of these are saved.

### Fire

Two ways to burn a building:

- **A thrown molotov** (25 heat) kills infected within 3 m and burns the building within 10 m.
  Only a real throw counts (the thrower's grenade-throw event names the bottle); one dropped from
  the inventory is just a bottle, and one held in the hand gives no prompt: throw it.
- **Fuel in your hands.** Hold a full fuel can and walk up to a building: **Douse the doorway and
  set it alight** appears on its nearest outside door (a building with no door entities: just
  outside the side of it nearest you). Five seconds within 3.5 m; the whole can is used up. On the
  5 s tick the server puts one invisible prompt at the nearest building within 8 m of each player
  holding fuel (two players at one building share it) and removes it when nobody with fuel is near.
  The prompt shows only to players whose own hands hold fuel, and the server checks the hands
  again when it starts and when it finishes. With **Fuel in your hands** off, a full can anywhere
  on you will do.

That one building burns: 30 heat, an optional fire effect, the town's infection −1, and for two
hours no loot there and no spawns within 30 m. Strongholds, outposts, safehouses, quartermasters
and the lab can't burn (or be locked), and never show the prompt.

The Gas Station Fire event ([MISSIONS.md](MISSIONS.md#world-events)) uses the
same fire, one building at a time; nothing burns a whole town at once.

### Water

Wells and pumps use the game's own fill action; ours (**Interactions > Well spots**) is off by default,
because both at once showed two actions at every well. Untreated water (the **Dirty water** prefab or a
name the **Untreated water keywords** match) is boiled on a stove (30 s) or purified with a tablet. What
drinking does belongs to the hunger and thirst mod ([MOD_STACK.md](MOD_STACK.md)).

### Treating each other

Treating another player is the game's own: hold a bandage, tourniquet, morphine or saline and use
it on them with vanilla's heal action. ReforgerZ adds no prompt and no timer of its own. One thing
is ours on top: a **Nurse or Doctor** who treats a bitten player that way (any vanilla health item
used on them, once the infection has taken) also slows the bite: **×1.15 of the time left**, once
per life, kept with the profile so a restart doesn't grant it again (**Interactions > Medic slows a
bite**; 1 = off). Both are told. A fresh bite (the golden window) is cleaned with the bitten
player's **Clean the bite** spot, or at a safehouse ([The golden window](#the-golden-window)).

*Simplified:* the hook is vanilla's consumable item component, where a health item's effect lands
on its target; that it runs on the server for a heal on someone else is a VERIFY
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#the-markers)).

### Bells, sirens and alarms

Ring a church bell (45 heat) or crank a siren (60 heat), once every 15 minutes each, to pull a
horde somewhere else. Players within 1.2 km hear it.

### Radios, open and secure

- **Open channels** are faction chatter and the guidance broadcasts. A **house radio** tunes
  every faction's open channel with power, two of them on weak batteries.
- **Secure channels** (Bastion's command net, the Red Hand boss, Halcyon's lab): only a Radio
  Operator can crack one, on a radio on the ground or a house radio: stand still 60 s with
  nothing loud near. Then 20 minutes of a line every 4 minutes: truck routes, watch times, short
  places, samples needed.
- **View radio** opens the radio screen ([UI.md](UI.md#the-radio-screen)), with **UI > Radio
  channel layout** set: on the wind-up radio held up in hand (vanilla's inspection, beside its
  Turn on/off) or lying on the ground, and as a house radio's fourth option. Every channel with
  what's on it now (a house set on weak batteries pulls in two open channels), the secure ones
  scrambled or cracked, and the log of what you heard this life, filtered by channel. It only
  shows; it doesn't tune or crack anything.

*Simplified:* spots stand at the middle of the thing's bounds (door spots on the door leaf, a wreck's boot at its rear). Carrying
the wounded isn't added. Item names are matched by keyword. Fuel cans are full or empty by name:
nothing with "empty" in its name counts as fuel (cars, generators, fires, the power station, the
molotov recipe).
Traps check on the 5 s tick.

## New-player guidance

Code: `Guidance/`.

The world teaches itself through things you find and people you hear, never tutorial pop-ups.
Every line is in a speaker's voice ([WRITING.md](WRITING.md#the-voices)).

### The welcome: one card, then quiet

A new life gets **one card**: the background's name and where you wake, in a line or two (the
opening's line, `RZ_OpeningText.SpawnLine`: "A farmhouse cellar in Morton. Day 11. The last bottle
is empty, so you open the door."; without an opening, the spawn scenario's, `RZ_GuidanceText.SpawnLine`:
"A cold basement in Morton. Scratches on the back of the door."; without either, "You come to east
of Levie."). Everything else at spawn goes
quietly into the notebook: the perk, flaw and goal, the scene in full with its clue, the outbreak,
the broadcast, the season with the news and the hall of fame in one entry, the radio's jobs, pay
owed while you were away in one entry. For the first 30 s nothing else is a card unless it's urgent,
and for the first five minutes cards come at most one every 45 s; the rules and settings are in
[UI.md](UI.md#notices).

### Openings: how every life starts

Code: `Guidance/RZ_Openings.c`, `RZ_OpeningPlaces.c`, `RZ_OpeningChain.c`, `RZ_OpeningText.c`,
`Dialogue/RZ_TalkOpenings.c`. **Guidance > Openings** (on).

Every life starts somewhere random that suits who you were, coming out of the place you hid in
since the seal, and within a few seconds someone who knew you before is talking to you and there is
one obvious thing to do: the first of three steps that teach the Zone and tell its story, then hand
you to the faction your background belongs with. Nobody is new: everyone has been trapped since the
roads shut. What's still to come is in [designs/OPENINGS.md](../designs/OPENINGS.md).

**Where you start.** Rolled on the server for each life; nobody picks it, and there are no chosen
starts. The spawn screen asks for one of the spawn points at random; the spawn point handler
(`SCR_SpawnPointSpawnHandlerComponent`, modded) moves the new character to the rolled place before
it's handed over, and the world streams in there.

- **Fits the background**, by the building's prefab name (the door marks' kinds and the landmark
  finder's words):

| Background | Place kinds | The spawn screen says |
|---|---|---|
| Nurse | clinic, house | A clinic, or a house near one |
| Doctor | clinic | A back room at a clinic |
| Mechanic | garage, petrol station, farm | A garage, a pump or a farm shed |
| Hunter | farm | A farm at the edge of things |
| Police Officer | police post, public building | A police post or a town hall |
| Ex-Con | garage, farm | A lock-up or a shed |
| Lab Technician | school, clinic | A school lab or a clinic |
| Radio Operator | radio mast, house | Somewhere under an aerial |
| Unemployed | house, shop | A house or a shop |
| Soldier | military building, public building | An old barracks, or a hall |
| Raider | house | A stash house |
| Elder | house | A house on a square |
| Drifter | house, shop, farm, garage, public building | Anywhere at all |

- **Safe enough to stand up in:** a town nobody holds or lives in, not the lab's, under 60 infection
  for a brand-new player (80 for others); by a building that isn't locked, 2 m out of an outside
  door, at a point 400 m from any holding, safehouse, road post or the lab, 600 m from a sealed
  checkpoint, off the line, no player within 300 m, no running mission within 500 m, 1 km from a
  pack's lair, outside the errands' quiet places, and 2 km from this player's last two starts.
- **Near where it ends:** towns within 3 km of where the chain will end are tried first, so the walk
  stays short; up to 8 towns are tried.
- **Fallback:** no fitting building, any house in one of those towns (the scene still comes);
  nothing at all, the spawn point the screen asked for, with the contact but no scene.

**The scene: out of the hiding place.** The door you came out of stands open behind you; by it, the
**Guidance > Hiding place scene** composition (empty: none) and a calendar to read, every day up to
the season's day crossed off, Day 2 ringed: ROADS SHUT. The spawn card says the place's kind, the
town and the day. The props go with the guidance clues (20 minutes once nobody is near).

**The contact: someone from before.** A few metres out of the door, facing you (**Guidance >
Contact**, empty: **Puzzles > Civilian**; counted as a story body, so the budget doesn't refuse
them; with no body at all there's no opening that life). They never say "you're new": they say
"Christ, you're alive." They talk only to you; anyone else is told they're waiting for someone.

| Background | Who they were | Background | Who they were |
|---|---|---|---|
| Nurse | a patient's mother | Radio Operator | a voice from the night band |
| Doctor | a nurse from your ward | Unemployed | the neighbour who lent you money |
| Mechanic | a regular at the garage | Soldier | a private from your unit |
| Hunter | the farmer whose land you hunted | Raider | someone you robbed |
| Police Officer | a shopkeeper on your beat | Elder | a mother from your street |
| Ex-Con | your old cellmate | Drifter | a face you half know |
| Lab Technician | a junior from the institute | | |

**They speak first.** Four seconds after the spawn, and again as each later step starts while
you're within 30 m of them, they open the talk screen on you ([DIALOGUE.md](DIALOGUE.md#who-talks)),
or say it as subtitles without it: that step's piece of the story, then the step's ask. Whatever you
answer, the step is already yours and tracked. Their one built reply is "What was it you needed?"
(the step again, short). If you leave the first step alone for two minutes and are still by them,
they ask once more. This is the **only job in the game that starts without you talking to anyone or
using anything**; its errands say so (`RZ_ErrandDef.IsOpening()`, `RZ_Errand.IsOpening()`).

**The chain: three steps** on the errand engine, handed straight to you
([ERRANDS.md](ERRANDS.md#handed-errands-the-openings-chain)), so the tracker follows each one as it
starts (the HUD and the Quests tab), with the giver's lines on the talk screen. About 15 minutes.

| Step | What you do | It teaches | The story, as it starts |
|---|---|---|---|
| 1. Across the Road | Search a house 50–250 m from them for 20 s, within 10 m of it, and come back | moving quietly, searching, the infected | The seal: "Day 11. They shut the roads on the second, and nobody's opened one since." |
| 2. Water, or The Cut | Water: stand 15 s at the nearest well or water tower (within 400 m) and come back. The Cut: dress their hand with anything medical you carry (a Nurse or Doctor with something medical always gets this one) | where water is; treating | The broadcast still saying aid is on its way, and "Nobody's coming. Say it out loud once." |
| 3. To the Haven, To Bastion, To the Red Hand, To Halcyon | Walk to the nearest place of the faction you belong with; done within 40 m of it (90 minutes to get there) | walking out, the map, the factions | The Perimeter ("The loudspeaker said his name. Then it didn't.") and the cure; their send-off says who holds the place, in a sentence |

No step 2 (no well near and nothing medical on you): straight to the walk. With **Dialogue >
Trees** on, the conversations `for opening` add their greeting by your background and things to ask
on the way ([DIALOGUE.md](DIALOGUE.md#the-conversations)).

**Where the chain ends** fits who you were:

| Who | Backgrounds | Ends at |
|---|---|---|
| Medical people | Nurse, Doctor, Lab Technician | Halcyon's nearest holding, or the lab if that's nearer |
| Trouble | Ex-Con, Raider | the Red Hand's nearest holding |
| Military and the law | Soldier, Police Officer | Bastion's nearest holding |
| Everyone else | Mechanic, Hunter, Radio Operator, Unemployed, Elder, Drifter | the nearest Haven safehouse |

A faction that hunts you, or holds nothing, sends you to the nearest safehouse instead. On arrival
someone there says one line, and you get **Guidance > Opening Fame** (10) with them as small pay
(not while they hunt you; never season Fame). The steps themselves pay nothing: no Favor, no items,
no hall-of-fame helped line, no deeds.

**When it ends early.** The contact killed during steps 1–2: whoever finds you says so, the step
ends, and the chain goes straight to the walk to the nearest safehouse. A step that lapses, death or
leaving ends it. Nobody waves you over for errands while it runs. One opening per life, runtime only
(a restart ends it). None in the epilogue week (the place is still rolled). The contact leaves once
nobody is within 150 m after the walk starts, or 45 minutes on with nobody within 50 m.

**Log lines:** `Openings: started for ...` (background, place, town, grid, level, who, chain, where
it ends), `... spoke first to ... at step N (the talk screen | subtitles)`, `... step N (key) for
...`, `... step N done by ... in N s`, `... died`, `Openings: ended for ...: why (step, minutes)`,
and the fallbacks' `no ... place fits`, `no place`, `no contact could stand`.

*Simplified:* the hiding place is the only reason; one chain shape for every background (step 2 is
water or the hand, not the background's own system); one kind of contact (someone from before, by
background); the guidance levels only shorten it to one step; no scene composition ships (the slot
is empty until the author makes one), so the calendar is the scene.

### Experience levels

Each player has a **guidance level** from lives that lasted at least 10 minutes. It fades but
never disappears.

| Level | Counted lives | What they get |
|---|---|---|
| 0, brand new | 0 | Everything: every tip in full, the gentle start, a wanderer who comes looking, scenarios leaning toward the Haven, the radio every 12 min |
| 1, learning | 1–2 | Every tip in full, full scenario scenes, wanderer gossip every time |
| 2, familiar | 3–7 | Most tips; one-line scenario scenes; gossip now and then |
| 3, veteran | 8+ | The floor: a scenario clue, the open channels (every 42 min), and critical tips as one short line |

A player who comes back after 30+ real days gets two levels more guidance for two lives.
With an [opening](#openings-how-every-life-starts), levels 0 and 1 get its whole chain and levels 2
and 3 its first step only; the scenario and the wanderer who comes looking are for a life without one.

### Contextual tips

When something happens, the nearest plausible person says **one line**: a faction leader within
40 m or someone at a safehouse; else a wanderer within 60 m; else a voice on your radio; else a
plain hint. Each tip comes at most once per life, with a cooldown across lives and 90 s between
tips (critical ones don't wait), and none that isn't critical in a life's first 90 s
(**Guidance > Tips wait after spawn**): tips come from what's happening to you, one at a time, never
a stack at spawn. The table is data (**Guidance > Tips**, defaults in
`RZ_GuidanceTips.c`); more can be fired with `RZ_Guidance.Report(playerId, id)`.

| Tip | Trigger | Critical |
|---|---|---|
| clean | a fresh bite ("Clean it, now. Before it takes.") | ✅ |
| bitten | the bite takes hold | ✅ |
| med_stabiliser, med_antibiotics, med_vet, med_cutdose, med_suppressant, med_remedy | the first use of each treatment (once per player) | |
| special | a special dies within 80 m of you (not after the cure) | |
| samples | samples in your pack (not after the cure) | |
| tier | your first rise to Known, Trusted or Champion | |
| watched | Watched by a faction | |
| hunted | Hunted or Kill on Sight | ✅ |
| perimeter | near the edge or a checkpoint, while the line is sealed | |
| loud | heat 45+ where you stand | |
| bleeding | you're bleeding (checked every 10 s; VERIFY `Guidance/RZ_Guidance.c:465`) | |
| quartermaster | within 25 m of a quartermaster of a faction that doesn't hunt you | |
| night | the first night | |
| safehouse | your first safehouse | |
| shortage | at a place that's short of food or supplies | |
| clue | your first puzzle clue | |
| channel | a Radio Operator near a faction's chatter or starting to crack a channel | |
| car | your first fixable car | |
| tags | the first options you're shown with a tag on them (an errand's ways, a one-choice job's question) | |
| truth | the first hidden truth you find in a story job | |
| read | your background's first read in a story job | |
| hold | the first time someone near you is about to decide a job (the 5 s hold) | |
| hungry | **hook only:** waits for the hunger mod (or a bridge) to call `Report(playerId, "hungry", place)` | |

### Gentle start and first-life goal

For the first 15 minutes of a brand-new player's life, zombie spawns near them run at half
density. Their first counted life gets the goal **Find the Haven**, unless its opening runs the whole
chain (its last step is the walk to where they belong).

### Spawn scenarios

A life without an opening (**Guidance > Openings** off, no contact could stand, or the epilogue
week) starts a few metres from a small clue (brand-new players lean toward the Haven):

| Scenario | The clue | Points to |
|---|---|---|
| A back road | a car in the ditch; the dashboard note: "Gone to the safehouse in *town*." | the nearest safehouse |
| A basement (not after the cure) | scratched into the door: the roads out end in concrete, nobody gets out | the Perimeter |
| Church notice (not after the cure) | Halcyon's flyer, "Infected samples wanted. Reward." | the lab |
| A farmhouse kitchen | a wind-up radio playing the broadcast (after the cure, the new order), then the Haven's open channel | the Haven |
| An attic | a dead raider with a red hand on his jacket and a list of toll roads | the Red Hand's boss |
| Behind the barricade | a dead soldier's orders: "Hold the line. Await relief." | Bastion's stronghold |

Clues are removed after 20 minutes once nobody is near, and after 40 minutes anyway.
*Simplified:* you don't really wake in a basement; the scene says so, and the clue lies next to
wherever the spawn point put you.

### The wind-up radio and faction voices

- Every new life gets a **compass** (vanilla's Adrianov, **UI > Compass prefab**) and a **paper map**
  (vanilla's folded map, **UI > Map prefab**) a few seconds after spawn, unless its loadout already
  has one; one that doesn't fit lies at your feet.
- Every new life gets a **wind-up radio** (unless they carry one). It picks up the **open
  channels**: the Haven ("If you can hear this, *town* is open. Bring water if you can. Bring
  yourself either way."), Bastion's net and the Red Hand's taunts. About every 12 minutes for a
  brand-new player, up to 42 for a veteran.
- **Bastion loudspeakers** near Bastion seats, **Red Hand taunts** near Red Hand seats (each at
  most every 30 minutes), **Halcyon flyers** in every safehouse, and **wanderer gossip** about a
  faction you haven't met.
- **First contact** with a leader ([REPUTATION.md](REPUTATION.md), "Talking to leaders").

### A wanderer finds you

Two to seven minutes into a brand-new player's life without an opening, the nearest Courier or
Scavenger comes looking for them, waves, and waits 5 minutes. Their talk includes faction gossip.

## Cure research and the season's end

Code: `Cure/`, `Core/RZ_GameModeComponent.c`.

```
special infected killed ─▶ sample item drops ─▶ carried to the lab ─▶ "Deliver samples" ─┐
research missions (data, readings, live specimens) ──────────────────────────────────────┤
                                                                                          ▼
              the lab's backlog, at most 4 days of work ─▶ worked through at 3% a real day
                                                                                          │
 25% Field antiviral   spread -15%, bites take 25% longer                                ◀┘
 50% Bite treatment    (antidotes and stabilisers cheaper and more plentiful)
 75% Immunity serum    spread -45% total, bites take 50% longer
 90% the finale's gate: five routes race to finish the batch (FINALE.md); the seal takes it to 100
100% FINAL STAND       hold the lab 15 min against waves every 2.5 min; the clean room decides control
       win  ─▶ infection everywhere -70%, Aftermath, season over
       lose ─▶ nobody at the lab for 90 s: the batch is ruined, research drops to 90%
```

- **What a delivery is worth.** Each sample is 1% research (×2 for a Lab Technician, +25% for
  Halcyon's Champions; never more than ×2 together). Research from missions counts ×0.75 of what
  the mission names. Both are multiplied by the **Cure pace** (0.3), the **pace by play** (below)
  and the **season speed**, so on a server of five regulars a sample is about a third of a percent
  ([Season pacing](#season-pacing) has the numbers).
- **Pace by play.** The server measures how much it really plays: the player-hours (players in
  the world × hours) on each real day, averaged over the last 7 whole days. Research counts
  ×(**Pace by play** ÷ that average), between ×0.25 and ×2.5. The setting, 12.5, is the design
  point (5 regulars at 2.5 hours), so two regulars get about ×2.4 and ten about ×0.5, and a season
  takes about as long on either. It follows the server as regulars come and go; days the server
  was down count as unplayed. It's ×1 for the season's first two days (the first is partial).
  Fame and Favor for samples don't change with it. 0 turns it off.
- **The lab's capacity.** Nothing counts at once: what's delivered goes into the lab's
  **backlog**, and the lab works through it at **3% research a real day** (**Lab research per
  day**, × season speed). It works in real time, also while nobody is online or the server is
  down (a restart catches up at once, 30 days at most). The breakthroughs fire when the *worked*
  research crosses 25, 50 and 75%, so a big delivery shows over the next days. Lab research per day
  0 turns the capacity off: research counts at once.
- **Breakthrough news waits for listeners.** The lab often crosses a milestone with nobody online.
  Its effects (spread, bite timers, the antidote stock) start at once; its news ("Breakthrough:
  Field antiviral", and the 90% gate's) is told the first time a player is in the world, in
  order, and is saved across a restart. The broadcast's "Reports of research inside the zone are
  being evaluated" runs for 90 minutes of play from then.
- **The cold store is full.** The backlog holds at most **4 days of the lab's work** (**Lab
  backlog days**; 12% at the defaults), a backstop for spikes: with the pace by play on, the
  simulator never fills it from 2 to 20 regulars. While it's full the lab refuses samples ("The
  cold store is full. We can't take another sample until the lab works through what's waiting.
  Come back tomorrow."): the player keeps them, with no Fame for them, and the action shows "The
  cold store is full". A hand-in that only partly fits takes what fits and the rest stay with the
  player. Mission research beyond the cap is lost, and the mission's result line to everyone says
  so ("The lab's cold store is full: only part of it could be kept."). The Red Hand's dead drop
  still buys samples.
- **An idle lab.** The part of a delivery that fills the backlog up to **half a day of the lab's
  work** (1.5% at the defaults) counts **+25%** (**Empty lab bonus**), so the lab's idle days
  aren't wasted. It's continuous: waiting for the lab to empty and then handing everything in at
  once gains nothing.
- **What players see.** The lab's **Check research progress** (a card, or the lab's object menu
  with **Deliver infected samples**), the journal and the first-spawn
  news give the lab's line: "Research 63 percent. Waiting: 14 samples' worth, about 4 days of
  work." (and "The cold store is full" while it is). Every delivery's line to everyone ends with it.
  Halcyon's secure chatter about the samples still needed says when the cold store is full or the
  work has stopped at the gate instead.
- **The finale's gate.** Research stops at **90%** while the cure finale's five routes are
  registered (`RZ_Questline.IsCureFinale`) and **Director > Finale** and questlines are on. The last
  tenth is the finale's: the routes, the lock, the batch, the seal and the stand as a fight for the
  clean room are in [FINALE.md](FINALE.md). The cure percent everyone sees is 90 plus the furthest
  route's track (`ShownProgress`). Research passes the gate only through one
  `RZ_CureResearch.AddFinaleResearch(amount, faction, reason)` call at the seal: straight into the
  worked research, not through the backlog, the Cure pace, the pace by play or the season speed,
  credited to the finisher's faction (saved for the ending). Meanwhile the backlog still fills (and
  then refuses). After **a real week at the gate** (÷ season speed) the built fallback, **1% a real
  day** (**Finale fallback**, × season speed), feeds the finale's lab-alone track instead of research,
  so a season whose routes nobody finishes still ends, about 17 days after reaching 90%. Once
  research has reached 100% the gate stays open: after a lost stand the backlog can make the next
  batch, which then waits for its seal.
- **The lab** is a placed prefab, or spawns in a random town and stays there across restarts.
- **Antidotes** are used as a world action or from the inventory; they cure an infection. The
  lab makes doses into a **stock** from the first day, faster with each breakthrough
  ([Treatments](#treatments)): Halcyon's quartermaster sells them (a shop action whose item is
  the antidote prefab), one dose per sale, and refuses when the lab has none. The Red Hand's
  Steal the Antidotes moves up to two doses from the lab to the Red Hand's black market (4 at
  most), and a Red Hand antidote shop action sells only those. Stabilisers are a second, smaller
  stock at the same quartermaster. All three stocks are saved.
- **The final stand** sends one horde every 2.5 minutes, plus one more per 6 players at the lab
  (3 at most), so about five steady players can hold it. Its clock stands still while nobody is
  online; a restart during the stand starts it again from 15 minutes, and players joining are told
  it's running. After the breakthrough or a restart, the stand (its clock, its waves and the 90 s
  "nobody at the lab" rule) only starts once a player reaches the lab.
- **The lab never falls.** It's Halcyon's stronghold; losing the final stand ruins the batch, not the lab.
  With the finale, the stand is also a fight for the **clean room** and **Take the Lab** is the Red
  Hand route's once-a-season holder change at the loading bay ([FINALE.md](FINALE.md#take-the-lab-and-the-coat-run)).
  With the finale off, Take the Lab steals the batch during the stand (hold the clean room for 90 s):
  research back to 90 percent, plus two doses on the black market (within its cap).
- **Contracts that touch the lab** (not saved across a restart): Power the Lab makes doses come
  twice as fast for 2 hours; Mobile Lab opens a **field annex**, a second lab prefab in a safer
  town where samples can be handed in for 3 hours, then it packs up. The lab itself never moves.
- **After the cure** (the epilogue, until the wipe): research is over. Nothing drops samples
  (specials or mission rewards), the lab and the Red Hand's dead drop refuse them (and say so),
  the sample tips stop, the lab makes no more doses, and Halcyon gives no more work by default
  ([World facts](#world-facts)). Work that stops with the cure and is running when it comes out is
  called off at once, with no pay and no penalty. The Perimeter stands down
  ([The Perimeter](#the-perimeter)); the broadcast stops its loop, and a first spawn and the
  wind-up radio give the new order instead. The week that follows (the record, beats, the dose
  outlets, the queue at the north road, the closing work, the Walk Out) is
  [EPILOGUE.md](EPILOGUE.md).
- **The season's end.** When the final stand is won, **whoever holds the clean room controls the
  cure** ([FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room)): the faction whose rules apply
  in full is the controller's (the Independent's only by its "one side" terms). **Season Fame** (the
  Fame the server gave each faction this season, summed over all players, plus the finisher's capped
  chunk) is the fallback: when nobody holds the room (the slow finish with no take) the cure is handed
  over to the faction with the most, and with the finale off it picks the winner as before. The winner
  is chosen before the stand's own Fame is handed out (with the finale: +30 for the controller's
  holders and +20 Halcyon for everyone who held the lab; without: Halcyon +50 and the Haven +30), so
  the last fight's pay can't decide it. The result, the route, how it was won and the terms are saved
  to `last_season.json`. What each ending means is in [LORE.md](LORE.md), "Endings"; what it leaves
  the next season, the wipe and the hall of fame are in [Seasons](#seasons).

## Season pacing

Code: `Core/RZ_GameModeComponent.c` (**Season speed**, **Cure > Cure pace**, **Cure > Pace by play**
and the lab's settings), `Cure/RZ_CureResearch.c`, `Director/RZ_OutbreakStage.c`. Simulator:
`tools/pacing_sim.py`.

A season is tuned for **about 45 real days (six and a half weeks) to the cure**, with 5 regular
players, each playing 2–3 hours a day, as the design point. The **pace by play**
([Cure research](#cure-research-and-the-seasons-end)) holds that from about 2 to 20 regulars
without any tuning by the admin. Three kinds of clock move a season, and only the first two were
stretched for the longer season:

| Clock | Runs on | Set by |
|---|---|---|
| Research | deliveries, then the lab's work in real time | **Cure pace**, the pace by play, the lab's settings, the season speed |
| Outbreak stages and spread | hours with someone online | the stage list, the season speed |
| Moment to moment: safehouse upkeep, supply trucks, territory, the Long Table, wanderers, spawns | their own minutes of play, as before | the season speed for upkeep, trucks, territory and the Long Table; nothing for wanderers and spawns |

**Assumptions** (the design point; none of these are measured yet; the simulator uses them):

| | Value |
|---|---|
| Regular players | 5, each 2–3 hours a day, at a random time in a 12-hour stretch of the day |
| Player-hours | about 12.5 a day (the **Pace by play** setting: ×1 there) |
| Hours a day with someone online ("hours played", the clock stages run on) | about 8 (4.4 with 2 players, 10 with 10) |
| Infected killed | about 20 per player-hour (travel, looting and missions mixed) |
| Specials (with night and adaptation) | 1.1% of kills in Early Outbreak, 1.6% in Collapse, 2.2% in Overrun, 2.7% in Aftermath |
| Samples that reach the lab | about 60% (deaths, the Red Hand's dead drop, pockets) |
| Research missions finished | 0.14 per player-hour (about 1.7 a day), 2.5 named points each |
| Lab Technicians and Halcyon Champions | about +20% on samples |
| The finale | about 5 days from 90% to 100% (an uncontested route: 4–5 days and the batch hold) |

**How research adds up for 5 players** (a sample is 1% × 0.3 Cure pace; missions ×0.75 × 0.3;
the pace by play is ×1):

| Stage | From samples | From missions | Delivered a day |
|---|---|---|---|
| Early Outbreak | ~0.6% | ~1% | ~1.6% |
| Collapse | ~0.9% | ~1% | ~1.9% |
| Overrun | ~1.2% | ~1% | ~2.2% |
| Aftermath | ~1.5% | ~1% | ~2.5% |

Plus 25% on the part of each delivery that fills the backlog up to half a day of work. The lab
works through 3% a day, so the backlog is a delay of a day or so and never fills. Other server
sizes deliver about the same a day: the pace by play scales each delivery by how much the server
plays.

**What the simulator gives** (`python3 tools/pacing_sim.py`, the defaults, the median of 60 runs;
day 1 is the season's first day). "Pace" is the average pace by play at a hand-in. The cure comes
on three days: with a finale questline done in 5 days, with one registered that nobody finishes
(the fallback only), and with none registered (no gate: today's build):

| Players | Pace | 25% | 50% | 75% | 90% | 100% with a finale | finale never done | no finale | Collapse | Overrun | Aftermath | Delivered a day | Lab full |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 2 | ×2.4 | day 15 | 27 | 39 | 45 | 50 | 62 | 50 | 9 | 27 | after the cure | 2.1% | never |
| 3 | ×1.7 | day 15 | 25 | 36 | 42 | 47 | 59 | 47 | 7 | 21 | 42 | 2.2% | never |
| 5 | ×1.0 | day 13 | 24 | 33 | 39 | **44** | 56 | 43 | 6 | 16 | 31 | 2.4% | never |
| 7 | ×0.7 | day 12 | 23 | 32 | 38 | 43 | 55 | 41 | 5 | 14 | 27 | 2.5% | never |
| 10 | ×0.5 | day 12 | 21 | 31 | 36 | 41 | 53 | 40 | 5 | 13 | 24 | 2.6% | never |
| 20 | ×0.3 | day 9 | 19 | 28 | 34 | 39 | 50 | 37 | 4 | 11 | 22 | 2.8% | never |

Bigger servers are still a little faster: they spend more hours online, so the stages (and with
them the share of specials) come sooner. Without the pace by play (0), the same defaults give two
regulars the cure on about day 90 and ten on about day 36, with 54% of the ten's hand-ins partly
refused at a full cold store.

**How the soft cap works.** Three layers, from the one players should feel to the one they
shouldn't:

1. **The pace by play** sets how much a delivery is worth. It measures the player-hours of the
   last week, so research a day stays near the design point (about 2.4%) whether 2 or 20 people
   play, and it follows the server as regulars come and go. It stops at ×2.5 (one regular is on
   their own about twice as long) and ×0.25 (beyond about 20 regulars the lab starts to fill).
2. **The lab's capacity**, 3% a real day, is a hard ceiling on how fast research can move: no
   server reaches 90% in under 30 days, or the cure in under about 35, however much it delivers.
3. **The cold store** (4 days of the lab's work) is the backstop for spikes the first two miss:
   a weekend event, a new crowd before the week's average catches up. While it's full the lab
   turns samples away (players keep them) and missions lose research.

So day to day the season moves at the pace by play, never faster than the lab, and the cold
store only fills when a server suddenly plays far more than it did the week before.

**The final stand** lasts 15 minutes with one horde every 2.5 minutes, plus one per 6 players at
the lab: six hordes for five defenders, so about five steady players can win it. A lost stand
(back to 90%) costs about three or four days of the lab's work.

**The other clocks** (with players online only, unless it says otherwise):

| Clock | Every | Over a season |
|---|---|---|
| Outbreak stages | 40, 80 and 120 hours played (÷ season speed) | Collapse about day 6, Overrun day 16, Aftermath day 31 |
| Infection creep | a quarter of the stage's spread a minute: 0.15–0.3 a played hour | as much per stage as the old 10-day tuning: +60 or so by Aftermath if nobody pushes back (1.5 per kill) |
| The lab's work | 3% a real day, online or not (× season speed) | 90% in 30 days at the very least |
| The finale fallback | 1% a real day after a week at the 90% gate, into the finale's lab-alone track | the cure about 17 days after 90% if no route finishes |
| Territory tick | 45–75 min (÷ season speed) | about 8 a day, at most one change each |
| Supply trucks | 60–90 min per faction (÷ season speed) | about 6 a day per faction |
| Safehouse upkeep | 3 h (÷ season speed) | about 2–3 meals a day |
| Safehouse stragglers | 4 h (÷ season speed), up to 12 people | a Settlement (medic, radio) by about day 4 without rescues |

The moment-to-moment clocks keep their pace: a longer season means more of the same days, not
slower ones. Treatment supply (antidotes, stabilisers) is on real time, not the season clock
([Treatments](#treatments)); with the breakthroughs later, the antidote stock stays at its slowest
(a dose every 30 minutes) for about the first two weeks. The stages follow hours played, not the
pace by play, so a small server stays in the earlier stages longer (two regulars never reach
Aftermath before the cure).

**Settings by server** (run the simulator with your own numbers, e.g. `--players 8 --cure-pace 0.25`):

| Regular players | Set | The cure, with a 5-day finale |
|---|---|---|
| 1 | **Season speed** 2 (the pace by play stops at ×2.5) | about day 46 |
| 2–20 | the defaults | day 39–50 (2: 50, 5: 44, 10: 41, 20: 39) |
| More than 20 | **Cure pace** about 6 ÷ the number of regulars (0.2 for 30) | about day 38 |
| Any, for a longer or shorter season | **Cure pace** (research only; the lab's 3% a day still sets the shortest season) or **Season speed** 1.5 (about a month) or 2 (about three and a half weeks), for 5 players | day 31 or 25 with the season speed |
| Pace by play off (0) | fit it by hand: **Season speed** 2 for 2 regulars, **Cure pace** about 1.5 ÷ regulars above 5 (0.15 for 10) | day 42–48 |

The season speed also makes upkeep, trucks, territory and the Long Table faster, as it always did;
on a small server that keeps the world busy. The Cure pace and the pace by play change only the
research. A lower **Lab research per day** lengthens the shortest season; **Lab backlog days** only
changes how much can wait before the lab refuses.

*Simplified:* the simulator counts the stages' clock only (a server that doesn't push back reaches
the infection thresholds sooner), each player hands in once at the end of a session, the same
regulars play every day, and a finale is modelled as 5 days. The pace by play counts whole UTC
days, so a server whose evening crosses midnight UTC splits it over two days (the week's average
evens it out). A kill still takes 1.5 infection off its town while the season is longer, so over a
stage players take off relatively more than creeps in; if towns clean up too easily, lower
**Zombies > Kill infection**. Check real kills, specials and research per player-hour in the first
sessions (the status log shows research, the backlog, the pace by play and the stage every few
minutes) against the assumptions, and adjust the Cure pace.

## Seasons

Code: `Core/RZ_Seasons.c`, `Persistence/RZ_HallOfFame.c`. Settings: **Season** (top level).

A season runs from a fresh outbreak to the cure, then the server wipes and a new outbreak
starts. By default **only the cure ends a season**: there is no timed wipe unless the admin picks
one ([The wipe cycle](#the-wipe-cycle)). Three things carry across: the last winner's starting rules,
the rule sets the admin picks, and the hall of fame. Every effect nudges a system that already
exists, and the combined effects are capped (below), so nothing runs away.

**The season's number and clock.** A fresh season (no state, or after a wipe) numbers itself
after the last one and starts its real-time clock. Both are saved, so a restart continues it.

### Next-season rules from the last winner

A fresh season reads `last_season.json` and starts with what the winning faction left behind
([LORE.md](LORE.md#endings)). **Season > Winner rules** (on) turns it off. A season that ran out
of time without a cure leaves nothing, and neither does a record older than the last season the
hall of fame knows (a wipe by hand after a season without one).

| Last winner | This season |
|---|---|
| The Haven | 2 more safe towns (the quietest start at 5% infection), 1 more safehouse, Haven strength +5 |
| Bastion | 1 more road post, 70% of posts are Bastion checkpoints, a **curfew** (Bastion keeps its full guard at night and its checkpoints go up first after dark), rare finds at police stations and on bodies ×1.3, Bastion strength +5 |
| The Red Hand | 1 more road post, 70% of posts are Red Hand roadblocks, the **black market everywhere** (the fence opens at Known instead of Trusted and holds two more stolen doses), Red Hand strength +5 |
| Halcyon | Starting infection ×0.8, spread ×0.9, specials ×1.15, Halcyon strength +5 |

With the cure finale ([FINALE.md](FINALE.md#endings-and-next-season)) the winner is the controller.
An **Independent** cure goes by its terms instead: open, half the Haven's and half Halcyon's rules;
one side, that faction's at half; sold, only the Red Hand's black market; kept back, nothing. Then
**at most one half rule set** goes on top: a deal partner's, else the season's Fame leader's when it
isn't the winner (**Finale > Fame leader rule**). A half rule set moves every number half-way to
neutral and leaves its switches off (`RZ_SeasonRuleSet.Half`), with its own line in the welcome
(*"Some of Bastion's season carries over, at half: a little more military kit about."*; the
Independent's say which terms it was). The winner's line says what the faction left behind, never
how it handed the cure out. Last come the majors' **story rules**
(one small rule per major that ended live, **Season > Story rules**; the table is in
[QUESTLINES.md](QUESTLINES.md#next-season-the-story-rules)). Everything is combined and capped as
below.

### Rule sets

**Season > Rule sets** names the rule sets for this season, comma-separated (empty = none). Each
is a list entry with the same fields as the winner's rules, so admins can add their own, or
replace a built-in one by giving theirs the same name (**Season > Rule set list**).

| Rule set | What it does |
|---|---|
| **Winter** | At night they hear further (heat spread ×1.5) and specials come out more (×1.2); safehouses eat 30% more food. Shelter matters more through existing systems: food, the safehouse's walls, the dark |
| **Fog** | Fog counts from thinner fog (threshold ÷1.6), Fog Bank events 2.5 times as often, and infected spawn 15% closer all the time |
| **Military Collapse** | Bastion strength −15 and Red Hand +10 (they start and settle there), 75% of road posts are roadblocks, rare finds at police stations and on bodies ×1.5 |
| **No Vehicles** | No fixable cars, no supply trucks (so no shortages, and the Haven's truck food doesn't come), no contracts that hand players a vehicle (Recover the Bus, Car Theft, Mobile Lab, the car keys job) |

*Simplified:* **Winter** doesn't make anyone cold: the mode has no temperature. Run a weather or
temperature mod alongside it for real cold and snow; Winter only adds the rules above. **Fog**
can't shorten the real view distance: the engine's fog is global and the mode never changes it.
Set the server's weather foggy for that; the rule set only changes how the mode reads fog and
spawns infected. The No Vehicles rule doesn't remove vehicles placed in the world or brought by
other mods.

**Caps** once everything is combined: starting infection ×0.6–1.3, safe towns 3, spread ×0.75–1.25,
specials ×0.8–1.4, night specials up to ×1.3, night hearing up to ×2, upkeep up to ×1.5, fog
threshold up to ÷2, fog banks up to ×3, sight down to ×0.7, safehouses +2 (never more than one per
5 settlements), road posts +2 (6 at most), rare finds up to ×2 (half at most), strength ±20.
Starting infection, safe towns, extra safehouses and starting strength apply only to a fresh
season; the rest apply at once and follow the setting if it's changed.

### The wipe cycle

**Season > Wipe**: *On cure* (the default), *After weeks*, *whichever comes first*, or *Off* (by
hand: **Wipe on start**, or delete `state.json`). Only *On cure* is on by default: the two timed
modes stay available for admins who want a fixed calendar, and are never used unless picked.

- **On cure:** the world stays open for an **epilogue** of one week, 168 real hours (Aftermath, with the
  ending, the line stood down) after the final stand is won ([EPILOGUE.md](EPILOGUE.md)). Its warnings
  and its last line speak of processing at the checkpoints; the gates open with the wipe, except the
  north road's in its last day (the Walk Out). **Epilogue > Test scale** shortens the week with
  every other epilogue clock.
- **A late cure:** a cure under **Minimum epilogue hours** (56) before an After-weeks wipe pushes
  that wipe back to the cure plus the minimum, and says so (also in *whichever comes first*).
- **The Unsigned last day:** no cure and the wipe under **Warn hours** away: `IsLastDay()`; the
  epilogue's record is built without a route ([EPILOGUE.md](EPILOGUE.md#edge-cases)).
- **After weeks** (off unless picked): 10 real weeks after the season started (**Weeks**), cure or
  not; a season to the cure takes six or seven weeks with 5 regulars ([Season pacing](#season-pacing)).
  With no cure, the season record says nobody won, and the next season gets no winner's rules.
- **Warnings:** in the last 24 real hours, the broadcast says so every hour ("Residents are
  advised that the zone will be reprocessed in about 6 hours."), with a plain line under it, and
  first spawns are told.
- **When it's due:** the season record is written (`last_season.json`), the hall of fame archives
  the season, a marker (`wipe.json`) is written, and everyone is told the Zone resets at the next
  restart. Nothing is deleted while the server runs.
- **On the next start:** the old `state.json` is copied to `state_season<N>.json`, and a fresh
  outbreak starts on the same map (new settlements' infection, safehouses, leaders, wanderers,
  territory; profiles, standing and Legacy start over too). The hall of fame and the season
  record stay.
- **By hand:** **Wipe on start** does the same on every start while it's on: the old
  `state.json` is backed up and the new season numbers itself after it, hall of fame or not.
  Deleting `state.json` by hand leaves only the hall of fame and the season record to count from.

*Simplified:* the wipe only happens on a restart; the mode never restarts the server itself.
Real time is the server's clock (`System.GetUnixTime`).

### Hall of fame

Saved to `hall_of_fame.json`, which no wipe touches. **Season > Hall of fame** (on).

| Record | Counted when |
|---|---|
| Most samples delivered | samples handed in at the lab or an annex |
| Held the final stand | at the lab when the final stand is won (everyone there is listed); with the finale, the controller's holders and the bystanders who held the lab half the stand, never a rival side |
| Most people rescued | a rescue brings people in (to a safehouse or onto the roads), for each rescuer |
| Longest life | a life's clock, checked every minute (lives over 10 minutes) |
| Most missions | each mission a player is paid for |
| Champion of each faction | the most Fame anyone reached with that faction in one life |

It's kept for **this season** and **all time**, plus a few lines for each of the last 10 seasons.
With the finale the season's lines open with who controls the cure and the season's Fame leader:
*"The cure: the Haven, by force. The Zone's Fame: Bastion, 1640."* **The records freeze at the
cure**: the epilogue week adds only lines to the season block, by fixed rules
([EPILOGUE.md](EPILOGUE.md#the-hall-of-fame-and-the-season-block)). `/anonymous` keeps a player's
name out of it across seasons ([EPILOGUE.md](EPILOGUE.md#names)).
Players find it in the notebook: the first spawn of a session adds it to the *This season* entry (no
card), and The season tab shows it in full at the bottom ([UI.md](UI.md#the-notebook)).

*Simplified:* players are matched by their backend identity and shown by the name they last
played under; at most 500 records are kept per list (the emptiest go first).

## Loose items

Code: `Core/RZ_LooseItems.c`.

Things the mode spawns for players are tracked: reward crates, dropped samples, key items, shop
and requisition purchases, search finds, what infected bodies carry, loot taken from houses, crafted items and treatments, the wind-up radio, the
Wedding Party's jewellery and The Last Pilot's wreck. One that lies untouched
for **30 minutes** (**World > Loose item minutes**; 0 = off) with no player within 150 m is
removed. Never removed: anything in a pack, a container or a vehicle's storage (its clock starts
again when it's put down), vehicles (a fixed car isn't litter), and anything within 300 m of a
running mission. At most 300 are tracked; past that the oldest carried entry is forgotten, else
the oldest abandoned item goes early. Not saved: after a restart, older items are left alone.

*Simplified:* "in a pack" means the item has a parent entity or sits in an inventory slot. If a
carried item is ever removed, set the minutes to 0.

## Persistence

Code: `Persistence/RZ_SaveData.c`.

Mode state is saved every 2 minutes to `$profile:ReforgerZ/state.json`: the stage, every
settlement's infection and town memory, cure progress, the lab's backlog and clock, breakthrough news nobody has heard yet, the pace by play's player-hours of the last week, antidote and stabiliser stock, player profiles (standing, Legacy,
unlocks, guidance level), quartermaster positions, safehouses, wanderers, leaders, territory,
supply stock, faction life, the Long Table, the power grid and radio station, story arcs, questlines,
headlines, world interactions, the crawlers' ledgers, stir and kill sites, the time of day
([The clock](#the-clock)), the world facts (the season's seed, flags, canon, gossip and world
memory; claims aren't saved) and the shared engine (waiting callbacks, the season budget, the
set-piece calendar, the season's deeds; notice lanes and marks aren't saved), and the epilogue week
(its record, beats, closing work and outlet stocks), the bounty board's postings, who last killed
whom, the Requisition cooldown and the fence's hour (by identity and real time), and errand pay
waiting for a truck. A save from another map is ignored. A profile that never spawned and holds nothing
lasting (no lives, Legacy, unlocks or guidance) isn't saved, and an event for a player id that
left before the server knew them (a wound described after a disconnect) gets a blank profile
that is never stored, so no junk `local_` profiles pile up. Player bodies, inventories and vehicles use
vanilla persistence ([MOD_STACK.md](MOD_STACK.md)). The season's number, clock and rules and the
notice boards' notes are in `state.json` too. A season wipe ([Seasons](#seasons)) backs `state.json` up to
`state_season<N>.json` and ignores it; `last_season.json` (the last season's result) and
`hall_of_fame.json` are kept.
