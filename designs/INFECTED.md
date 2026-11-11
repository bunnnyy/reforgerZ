# Infected: tribes and crawlers

**Status: designed; phases 1 to 5 are built** ([docs/INFECTED.md](../docs/INFECTED.md)): crawlers, the
land, bodies and the budget, the tribes' daily life and hunts, their life cycle, the people's side
([People](../docs/INFECTED.md#people)), the great alpha, signs and sounds ([The great
one](../docs/INFECTED.md#the-great-one)). What's left here is designed: the small rules each section
still lists, the people's rest ([People](#people)), and the finale's live tissue (it goes in with the
finale). Approved by the author on 2026-11-11.

## What changed in review

- **Day parties** (the author's change; conflict 45 as the author clarified it). Each tribe keeps a
  small party of hunters and young out by day, while the alpha and guard lie up at the lair, and by
  day it never sends the full hunt. It's model rule 25 and [Day parties](#day-parties), and every
  place that said the wild is quiet by day has been reworked.
- **One judge of sight: stealth** (conflict 46). The scout rule reads `RZ_Stealth.GetVisibility` and
  `SightRadius` instead of its own light, weather and stance table. Tribe bodies get stealth's
  perception factor. A hunt on its trail and the Furious last wave are never lost through stealth's
  search; day parties and everything else are ([Stealth and the tribes](#stealth-and-the-tribes)).
- **One owner of group waypoints** (conflict 47): push, pop, the held waypoint and delete-with-the-unit
  in one helper, shared with stealth's search, in phase 0. Stealth's waypoint never goes over a
  tribe's held waypoint during a hunt.
- **The cure pacing is built** (conflict 49, commit b78615a). Phase 0's "land the pacing branch first"
  is gone, and so is every reference to the old 10-day pacing: a faster season is now a **Season
  speed** above 1.
- **Finale tissue credits a route's track** (conflict 44), not `AddFinaleResearch`, which the finale
  now calls once, at the seal. [FINALE.md](FINALE.md) owns the order of specimens; this doc keeps the
  tribes' side.
- **Words** (conflict 48): the Red Hand's lines no longer say "the boss is the one who won", crew
  nicknames come "after something they'd rather forget", and Bastion's tribe list is always "the
  contact board" ("the board" is the roster). "Strays" is fixed to match LORE.md: the crews' word for
  the people the Haven turned out.
- **The Grounds rename table** (conflict 43) is [here](#from-grounds-to-tribes-the-rename-table), and
  the epilogue's tribe beats (conflict 42) are in [The last week](#after-the-cure-the-last-week).
  Conflict 41 changes nothing in this doc.
- **The lower store and Bastion's held store are no-lair land** (conflict 50, which touches this doc
  too).
- **The epilogue lasts one week** (decision 12). [The last week](#after-the-cure-the-last-week) says
  what tribes and crawlers do in it, so that every story with the infected in it can be finished:
  nobody new turns, tribes run down on the epilogue's own clock, a town cleared stays cleared, and
  on the last dusk the alphas call and nothing answers.
- **Phase 0's measurements** are folded in where the approved design said phase 0 would set the
  number: the perception range P, the speeds, the pack's timing, the slot divisor, frame time, the
  land queries, alpha health, the silent kill, steering (go) and visible fights (no-go). Phase 0's
  other proposed retunes wait for the author ([Phase 0 results](#phase-0-results)).
- **Decision 16:** `docs/INFECTED.md` is added to the repo only when phase 1 is built.
- **The 16 tribes decisions stay as recommended.** Decision 13 had no recommended option; this doc
  takes (b), because decision 2 keeps "alpha" out of player text ([Decisions](#decisions)).
- **Lore kept consistent with LORE.md:** the Haven's bear on its doors, Halcyon's "beaks" and their
  plague-doctor stencil, Bastion's roster and "No reply", and the Red Hand's book.

The infected's ecology: wild packs with leaders (tribes) and the slow infected of the towns
(crawlers). Code (planned): `World/RZ_Land.c` (the land grid), `Zombies/RZ_Tribes.c` (records and
the life cycle), `Zombies/RZ_TribeBodies.c` (parts, steering, the scout rule), and crawler placement
in `Zombies/RZ_ZombieSpawner.c`. Settings go under **Zombies > Tribes** and **Zombies > Crawlers**.

## Contents

- [Where it's written, and the words](#where-its-written-and-the-words)
- [The pitch](#the-pitch)
- [What the infected do (lore)](#what-the-infected-do-lore)
- [The model](#the-model)
- [Tribes](#tribes)
- [Day parties](#day-parties)
- [Towns: the crawlers](#towns-the-crawlers)
- [The land](#the-land)
- [Bodies](#bodies)
- [Stealth and the tribes](#stealth-and-the-tribes)
- [People](#people)
- [The season](#the-season)
- [After the cure: the last week](#after-the-cure-the-last-week)
- [From Grounds to tribes: the rename table](#from-grounds-to-tribes-the-rename-table)
- [Build phases](#build-phases)
- [Phase 0 results](#phase-0-results)
- [Decisions](#decisions)
- [Risks](#risks)

## Where it's written, and the words

**Where.** A new doc, `docs/INFECTED.md`, owns the topic: what the infected do, the land, tribes,
crawlers, bodies and signs. At about 800 lines it's too big for a section of
[DESIGN.md](../docs/DESIGN.md), and CLAUDE.md allows a new doc when a topic outgrows its section. It
goes into the repo with phase 1, when the first of it is built (decision 16). Then:
- DESIGN.md's "Zombies, bites and infection", "Heat and noise" and "Infection adapts" each keep a
  short paragraph and link to it;
- [LORE.md](../docs/LORE.md) gains the setting ("What the infected do");
- CLAUDE.md's Doc map and the README's doc list gain the file in the same commit.

**Words used in the docs and the code:**

| Word | Means |
|---|---|
| **crawlers** | A town's slow infected |
| **tribe** | A pack in the wild |
| **alpha**, **guard**, **hunters**, **scouts**, **the young** | A tribe's roles |
| **day party** | The hunters and young a tribe keeps out by day, in twos, threes and fours |
| **resting ring** | The hunters and young lying up round the lair by day |
| **lair**, **trail**, **range**, **hunting ground**, **kill site** | A tribe's places |
| **clan** | Tribes of one line |
| **great alpha** | A clan's leader |
| **lone one** | An alpha or challenger with no tribe |
| **splinters** | Loose packs after a succession |
| **the ones who ran** | Young a tribe gains when a town near it goes quiet |
| **no man's land** (new to the docs) | The 300 m round every lived-in place, where no hunt and no day party goes |
| **counted night** | A game night with players online |

**Player text never says "alpha", "tribe" or "crawler"**, because every speaker has its own words
([People](#people)). "Strays" means only the people the Haven turned out (the crews' word for them,
as in LORE.md), and is never used for the infected.

This replaces the Grounds design's town firsts. Several things from Grounds are kept: its data
model, budgets, signs and phasing, its waypoint-leak fix (now one owner of every group's waypoints)
and its lore that nothing is born. The names that changed are in the
[rename table](#from-grounds-to-tribes-the-rename-table).

## The pitch

The infected stop being one kind of thing. There are two kinds now, and you learn to tell them apart.

**The towns hold the crawlers.** They're the town's own people: the ones who turned lying still, in
a bed or a cellar or a locked bathroom. They came up slow and swollen, and they never left. By day
they lie up in doorways and back rooms. At night they come out into the streets and drift toward any
noise, because noise used to mean people. There are a lot of them. They're easy to see and easy to
outwalk, and they're the reason you don't loot a street with a shotgun.

**The wild belongs to the tribes.** These are packs of the ones who turned running. Each is led by an
alpha: a big, heavy one that turned early and kept feeding.
- **Home.** A tribe has a lair in a wood, a quarry or a culvert. It walks the same trails every night.
  Its range is one or two kilometres across, and its neighbours respect it.
- **Knowing one.** You can tell a tribe by where it is and by how it moves: in small groups, in file.
  Once one of them has seen you, they run.
- **Day.** The alpha lies up at the lair with its guard and a ring of the rest round it. A few
  hunters and young are out all day in twos and threes, walking the tribe's paths, field edges and
  woods. They fight if they see you, but by day the tribe never comes in full. Scouts sit on the
  ridges, and a scout that sees you brings a handful, never the alpha. A hungry tribe has more of
  them out, and they come as far as the fields at a town's edge.
- **Dusk.** The alpha calls, and the next ridge answers: that's a border. A scout that sees you and
  isn't silenced brings the pack within the minute.
- **Night.** The tribe hunts along its trails, through the edges of towns. Afterwards the crawlers
  come out to where it fed.
- **Dawn.** It walks home, and you can follow it.

**Tribes live, and you can hear it.** Nothing is born: every infected was somebody in the Zone.
- **Growth.** A tribe grows by taking people, and from the towns near it: when a town goes quiet, not
  everyone in it stayed. A tribe next to a dying town gets big. A tribe in empty country goes hungry
  and comes looking. Players who push a town back starve the tribe that lived off it.
- **The call.** It tells you how the tribe is doing:
  - long when it's hungry;
  - silent the day after it fed, when nobody is out on its paths either;
  - two voices when the guard is restless;
  - a voice nobody knows after a succession.
- **Breaking off.** A hunt that loses too many of its own breaks off with one long call, and you know
  you won that night.
- **Death of an alpha.** The tribe stays by the body until dawn. Then the guard fight it out, a few
  splinter off, and one of the guard becomes the new alpha, with a name.
- **Splits.** Tribes split when they get too big, and the halves become a clan.
- **Neighbours.** Tribes push at each other's borders with their calls, and fight over the bridges
  when they're hungry.
- **Learning.** They learn the supply truck's hour and the Haven's back door. They keep away from the
  wood where twelve of them died, until they forget.
- **Character.** A tribe looks like where it grew up, and keeps small echoes of who its alpha was.

**The factions.** Each lives with the tribes its own way:
- Bastion culls them, and they end up in the Haven's country.
- The Red Hand fattens one as a moat.
- Halcyon tags them, and needs one alive for the finale, a prize every route wants.
- The Haven knows who some of them were.

Your own dead character may run with one.

**The guardrails hold.**
- Tribes are data. Bodies exist only near players.
- Bacon Zombies' AI does the fighting, untouched. The script only chooses where and when bodies
  appear, and stealth (built) judges whether they see you.
- Every effect is capped and recovers, and holdings never fall.
- Day is the lighter danger: a day party is at most 6 bodies near you, and never the alpha.
- Both layers switch off, back to today's spawner.

**How many.** The number of tribes comes from the wild land the map really has (measured in phase 0):
- Arland has too little wild land for a tribe: crawlers only, unless the server sets **Slots**.
- Everon has room for four, with a clan when one earns it.
- Cain has room for three.
- A Chernarus-size map gets up to fifteen, with clans and, some seasons, a great alpha.

## What the infected do (lore)

**Moved:** the setting is in [LORE.md](../docs/LORE.md#what-the-infected-do), the writing rules in
[WRITING.md](../docs/WRITING.md#writing-the-infected), and the faction words are under
[docs/INFECTED.md](../docs/INFECTED.md#the-words). The Lone One (decision 13, option b) is built.

## The model

Everything is data on the server, and bodies exist only near players ([Bodies](#bodies)). Two new
owners run from the game mode's tick: `RZ_Land` (the land grid) and `RZ_Tribes` (tribes, alphas,
lone ones, splinters and kill sites). Crawlers are a change to `RZ_ZombieSpawner`, not a new
population.

**Reused as they are:**
- **The world:** settlements, infection and `AddInfection`; the Director's stages and spread; heat and
  its remembered noises; the landmark finder; town memory; the environment.
- **The spawner:** its spawn checks and tracked units.
- **Stealth** (built, `Zombies/RZ_Stealth.c`): each player's visibility, the infected's perception
  factor, and losing a hidden player ([Stealth and the tribes](#stealth-and-the-tribes)).
- **Helpers:** `RZ_Util.RedirectGroup`, `LandPointNear` and `NewWithDefaults`.
- **World facts:** claims, canon, flags, `Changed`, `Fill` and `CanRunNow`.
- **Deaths that make infected:** wanderers, trucks, the Turned Player, and leaders' off-screen bites.
- **Content plumbing:** Live Capture, the step chain, and the voice and journal systems.

**Built: the 45-day season pacing** (commit b78615a, in [DESIGN.md](../docs/DESIGN.md), "Cure research
and the season's end" and "Season pacing"). It brings:
- the Cure pace and the pace by play;
- the lab's capacity (3% a real day) and the cold store (4 days of the lab's work);
- the 90% finale gate, `RZ_CureResearch.AddFinaleResearch`, and the fallback that finishes the cure
  about 17 days after the gate;
- `tools/pacing_sim.py`.

Every research number below assumes it.

**Two layers, and two counts that never overlap**
- **A town's crawlers** come from its infection, as today: size × 4 × infection/100 × stage density ×
  event multipliers. From that, take off the kill ledger and any crawlers lent out to a kill site or a
  horde. Town infection stays the one number every mission reads.
- **A tribe's bodies** come from its members, a count of its own. Only the tribe content reads it.
- The two exchange only through named rules:
  - the taken become young (rule 4);
  - the ones who ran leave a town going quiet (rule 5);
  - starving members walk into a town and add to its infection (rule 6);
  - a hunt passing through stirs a town's crawlers ([Towns](#towns-the-crawlers)).

**Clocks.** Each rule says which one it uses.

| Clock | What it is | Runs |
|---|---|---|
| **Played minutes** | Real minutes with someone online | Spawns, the scout rule, the data tick's movement, the kill ledger, stir, the leash, day parties' hops |
| **Counted nights** | A game night (20:30–05:00) that counts once, at 05:00, if someone was online for 20 real minutes of it (**Zombies > Tribes > Counted night minutes**). Nights on an empty server don't count | Hunger, takings and their caps, post-feed sleep, habits, worn trails, fear places, marks, the three-empty-nights rule, a hunt's commitment |
| **Season days** | The first three stages' lengths added up, divided by 30. That's 8 hours played at **Season speed** 1 (about a real day with 5 regulars), and less at a higher season speed. They stand still on an empty server | Ageing, new tribes, splits, clans, the great alpha, lair moves, vacuums, disturbance decay, lone ones' days, growth caps, frenzy spacing |
| **Time of day** | The environment and world facts (`IsDusk`, `IsNight`, `KnowsTime`) | Phases: day, dusk, night, dawn |
| **Epilogue hours** | Real hours since the cure, online or not | The last week ([After the cure](#after-the-cure-the-last-week)) |

**Assumed time acceleration** (decision 8). A game day of about 4 real hours (×6), with no separate
night speed. At ×6 a night is about 85 real minutes, dusk about 20 and dawn about 10. WORKBENCH_SETUP
states this beside **Season speed**.
- **With 5 regulars at Season speed 1** that's about 2 counted nights a season day. The design is
  tuned for this.
- **At a higher season speed** a season day is shorter, so the same arc has fewer nights in it: fewer
  hunts per alpha.
- **What gets logged.** The status log prints counted nights per season day as measured.
- **What phase 0 found.** The engine's default clock is real time (×1), and it goes back to the
  scenario's start on a restart. The server has to be set up for the assumption to hold
  ([Phase 0 results](#phase-0-results) has the proposed fix).
- **With no game clock** every phase reads as day: day parties walk, scouts post, nobody hunts at full
  strength, and tribes show through day parties, scouts and text.

**Three ticks:**
- the **data tick**, every 60 s: parts move along trails, phases change, and bodies write their
  positions back;
- the **life-cycle tick**, every 10 played minutes: needs, borders and successions, with at most two
  changes on the server;
- the **land build**, once (below).

**The land grid** (`RZ_Land`). Its cells are the heat map's 250 m squares. Phase 0 built and measured
it on its own branch, off by default ([Phase 0 results](#phase-0-results)).
- **Building it.** It's built at the first start, one square at a time: about 4.5–5 minutes on Everon
  and Cain (each second's work stays at or under about 14 ms).
- **The cache.** It's saved to `$profile:ReforgerZ/land_<world>_<hash>.json`, where the hash comes from
  64 sampled terrain heights (about 51 KB on Everon; it loads in 3 ms). Restarts load it, and a
  mismatch rebuilds it.
- **While it builds,** tribes are dormant: no lines, no spawns. Crawlers work as normal. The log says
  so.

| Field | From |
|---|---|
| Water share | Sample points: `ChimeraWorldUtils.TryGetWaterSurface` tells sea, pond and river apart (`RZ_Util.IsWater` sees only the sea). Rivers come from river entities (`RiverEntity.GetExistingInstances`: each river's centre line and width). Lakes over about 100 m are found; ponds aren't |
| Height and relief | Terrain height at the sample points. Relief is the height above the mean of the 8 neighbours |
| Trees (none, sparse, forest) | Tree entities found by prefab path (bushes are `BaseTree` too). Fallback: none, and lairs use landmarks only |
| Buildings | The settlement scanner's building cells |
| Road | Road segments, which need an `SCR_AIWorld`; else none |
| Bridge | A road segment over water or a dip. Road points sit on the terrain even on a bridge, so the bridge is the line between two points (about 90% right, finding 60–80% of real bridges) |
| Wild | True when the cell is dry and outside all of these: every settlement's radius + 100 m; the 300 m no man's land round every lived-in place's edge (holdings, safehouses, the lab); 700 m round the Long Table's farm; and the warning zone plus 200 m |

Worked out from the grid:
- **Ridges:** relief of 25 m or more, or the highest cell within 1 km.
- **Valley floors:** a line of cells lower than the cells 500 m either side by 15 m or more.
- **Forests:** forest cells. A map's woods are usually one or two huge ones, so lairs and trails use
  **forest edges** (a forest cell next to open land), not the middle.
- **Fields:** open, flat cells with no trees, within 800 m of a settlement. They're scarce (Everon 25,
  Cain 5, Arland none), so day parties walk forest edges and sparse ground too.
- **Rivers:** from the river entities and the water query. On a map without river entities, a valley
  line through two or more bridges counts as one. Otherwise there are none, and borders follow ridges
  and valley floors.
- **Crossings:** bridges, and fords where a river is 30 m across or less.
- **Lair candidates:**
  - **what counts:** forest edges, plus barns, mills, silos, quarries, culverts, sheds and ruins from the
    landmark finder;
  - **where:** 600 m or more from every settlement centre and 800 m or more from every lived-in place.
    On maps with under 25 km² of land: 400 m and 600 m;
  - **size of the range:** a radius of 500–1,000 m, or 350–700 m on those small maps.
- **No-lair land** (conflict 50): 300 m round the finale's lower store (`origin.site.lower`) and round
  Bastion's held store (`held.store`). It's an overlay, like burned land, applied once those canon
  spots are decided, so the cache never needs a rebuild. No lair and no scout post goes there.

The build logs what it found, for example (Everon): *"Land: 2,652 cells, 24 km² wild. Rivers 28
(14.9 km), lakes 2, bridges 22, fords 2, valleys 10. Lair candidates 35."*

**Records**

| Record | How many | Fields | Saved |
|---|---|---|---|
| Tribe (`RZ_Tribe`) | 0–15, by wild land | id; seed; place name; status (settled, nomad, restless, weakened, furious); members by role; the names of the taken (12, newest first, each with how they're shown) and their bonds; the ones-who-ran bank; lair (position, kind, quality) and up to 3 lair candidates (nomads loop through them); range (centre, radius); hunting grounds (up to 4); 2 scout posts; temperament; look; call pattern; hunger; disturbance; habits (3); fear places (4) and recently expired ones; tonight's trails and each trail's wear; today's day party (who, in which groups, and their walks); the post-feed flag; borders (neighbour, age, offset); clan and mother ids; day founded; alphas lost; alpha samples given this season; tissue given (season day) | yes, about 1.4 KB each. Trails and day walks are worked out again from the lair, the range and the hunting grounds |
| Alpha (inside its tribe) | one per tribe, or none during a succession | name, and one line on who they were; kind (soldier, driver, farmer, runner, courier, resident, a player's character), which sets the echo; the day it turned; the day it began to lead; stage; health 0–1; people taken; keepsake text; history (its last 6 events as short lines); tag (who planted it, and on which day); great or not; fate (for the epilogue) | yes |
| Part (`RZ_TribePart`) | 4–7 per tribe, plus 1–3 day-party groups by day (up to 5 when hungry) | role group; members (ids); data position; trail or walk step; target; last order | no: rebuilt from the tribe and the clock, then moved by the write-back while it has bodies |
| Tracked unit (`RZ_TrackedZombie`, existing) | per spawned group | adds: tribe, part, role, member ids with their entity references, the held waypoint, silent kills pending. Stealth's state (`m_Sense`) is already on it | no |
| Lone one | 2 at most | name; history; health; position; where it's walking; days alone | yes |
| Splinter | 4 at most | count; position; target; the tribe it came from; day formed | yes |
| Kill site | 6 at most | position; tribe; who (a name or a description, and one line); day; feeding until | yes, until the second dawn |
| Mark | 4 per tribe | the character (not the player), strength | yes; cleared when that character dies |
| Each settlement (existing) | every settlement | infection; the kill ledger; stir 0–100; crawlers out; last infection (for the ones who ran) | ledger, stir and last infection |
| Land lists | — | burned cells (which kind, and until when); fear places; no-lair overlays | yes |

Saved in `state.json`: about 28 KB at most, on a Chernarus-size map. Bodies are never saved. A save
from before tribes existed seeds them on load, as at a season's start.

**Update rules**

1. **Wild land and slots.** At start: slots = wild km² ÷ **Wild km² per slot**, rounded, between 0 and
   15. The design started at 6; phase 0 measured the land and sets it at **5**, so that Everon gets 5
   slots, room for 4 tribes and one kept for a split (at 6 it got 4 slots and room for 3).
   - **The floor:** at least 1 when there are 1.5 km² of wild and a lair candidate. Under that, tribes
     are off and the log says so. Arland measured 0.4–1.1 km², so it has no tribe.
   - **Kept for splits:** 1 at 3 or more slots, 2 at 9 or more, 3 at 13 or more.
   - **The override:** **Zombies > Tribes > Slots** (0 = from the land) replaces the count on any map.
     The runtime test sets 2 on Arland, to exercise the code only.
   - The log line gives the measured values ([Scaling by map](#scaling-by-map)).
2. **New tribes:** **built** ([New tribes](../docs/INFECTED.md#new-tribes)).
3. **Roles, the taken, the ones who ran, hunger, hunts as data, the night's commitment, post-feed
   sleep** (rules 3–9) and their life-cycle parts (the guard's place, the young coming of age, bonds,
   trucks taken as data, starving, trail scoring by wear, habits and echoes): **built**
   ([Tribes](../docs/INFECTED.md#tribes), [The life cycle](../docs/INFECTED.md#the-life-cycle)). Still designed: a leader's
   off-screen bite or a Bastion patrol as the taken; trail scoring by prey; empty nights (three running:
   drop the worst trail).
10. **Disturbance, splitting, borders, absorbing and merging, ageing, succession, lone ones,
    displacement and vacuums** (rules 10–18): **built** ([The life cycle](../docs/INFECTED.md#the-life-cycle),
    [Borders](../docs/INFECTED.md#borders)). Still designed: Bastion's *"Papa-four: no contact"*, frenzy nights as a
    border-fight cause, the finale's live tissue, and the border's offset moving the ranges.
19. **Fed by people:** **built** ([The dark corners](../docs/INFECTED.md#the-dark-corners)). Still designed:
    the moat rule stopping for a tribe whose alpha a finale route took alive, until its succession.
20. **Memory, abandoned lairs, seasonal pull and burned land** (rules 20–23): **built**
    ([Memory](../docs/INFECTED.md#memory), [The life cycle](../docs/INFECTED.md#the-life-cycle)). Still designed: an abandoned lair's
    finds as items from the search table (a line today), and tracks always fresh in snow.
24. **After the cure:** **built** ([The last week](../docs/INFECTED.md#the-last-week)).
25. **Day parties:** **built** ([Day parties](../docs/INFECTED.md#day-parties)). Still designed: the Furious
    tribe's walking the capture road.

**Caps:** **built** ([docs/INFECTED.md](../docs/INFECTED.md)).

### Scaling by map

The slot count comes from the wild land the grid measures, never from the map's name. Phase 0 measured
Arland, Everon and Cain over 4–6 starts each; Chernarus-size is still an estimate. The log line reads,
for example: *"Tribes: 24 km² of wild land, 5 slots (1 kept for a split), 1 clan possible. Override:
none."*

| Map | Wild land | Slots (÷5) | Room for tribes | Kept for splits | Clans | Great alpha | How it feels |
|---|---|---|---|---|---|---|---|
| Arland (about 4 km across) | 0.4–1.1 km² (measured) | 0: under the floor | none, unless **Slots** is set | — | none | never | Crawlers only. With **Slots** set, phase 0's simulator seeds the tribe in only about half of seasons (Arland has few lair candidates), and it often breaks up |
| Everon (about 12 km) | 23.8–25.8 km² (measured) | 5 | 4 | 1 | about half of seasons at 5 players (phase 0's simulator) | in principle (a clan of 3); expect never | Hills with neighbours, rivers (28 found) and bridges, and some seasons one family of tribes |
| Cain | 19.7–21.3 km² (measured) | 4 | 3 | 1 | sometimes | never | Fewer, larger ranges |
| Chernarus-size (about 15 km) | 80–120 km² (estimate) | 15 | 12–14 | 3 | up to 3 | possible from Overrun | Real politics in the woods: clans, and some seasons a great alpha |
| A small custom map | under 1.5 km² | 0 | — | — | — | — | Crawlers only. Tribes are off, and the log says so |

**How Everon gets its clan:** built (the kept slot, the room kept round a big tribe, the daughter
taking a weak neighbour's wood); the simulator gives a clan in about half of Everon seasons at 2–5
players ([its numbers](../docs/INFECTED.md#the-simulator)).

### Clans

**Built** ([Splits and clans](../docs/INFECTED.md#splits-and-clans)). Still designed: the relay players hear pass along
the ridges, and all of a clan hunting toward one ground on frenzy nights.

### The great alpha

**Built** ([The great one](../docs/INFECTED.md#the-great-one)). Still designed: caught alive it counts +3
on a finale route's track instead of +2 ([FINALE.md](FINALE.md)), with the finale's live tissue.

### Claims

`Tribe(id)` ("tribe:3") becomes a world-facts claim subject.
- **What a claim holds still:** a running mission or questline stage that uses a tribe holds it. No lair
  move, split, merge or absorption happens while it runs.
- **Successions still happen:** if its alpha dies, the mission ends with it.
- **HC1:** its site stages claim the site tribe, so its bodies keep 150 m from the store.
- **Halcyon's finale route, The Lower Store** (conflict 50): its Act II claims the site tribe as HC1
  does. While it runs, the tribe's bodies keep 150 m from the lower store (`origin.site.lower`); noise
  still draws them ([FINALE.md](FINALE.md)).

### Admin visibility

- **The status log** prints one line per tribe every 10 played minutes: id, name, members by role,
  hunger, alpha stage and health, lair cell, hunting tonight or not, day party out (groups and bodies),
  and samples given this season. The server line adds the active AI count (phase 0: the engine's limit
  covers all AI, not just the infected).
- **Growth and counts:** it also prints the ones who ran per season day, counted nights per season day,
  and samples per player-hour by source.
- **`/tribes`** (admins) dumps every tribe, part and record to the log.
- **The land build** logs what it found.

## Tribes

### Roles

| Role | How many | Who they were | What they do | Prefabs (**Zombies > Tribes**) | When the list is empty |
|---|---|---|---|---|---|
| **Alpha** | 1 | Turned in the open early, and kept feeding and leading | Lies up at the centre of the lair by day, and never leaves it by day. Calls at dusk. At night it's on the trail behind the hunters (a bold one leads). Eats first at a kill | Alpha list: a heavy (Bacon's heavy, the fastest Bacon character at about 4.9 m/s; or a Special Zombies brute). A character prefab, put into its own group at runtime ([Bodies](#bodies)) | The Special prefabs. If those are empty too, the alpha is data only and the alpha jobs turn off |
| **Guard** | 2–4 | The oldest hunters | Ring the alpha 10–25 m away by day and on the trail. Stay at the lair all day. Fight hardest at the lair. Candidates in a succession | Guard list: Bacon's military and helmeted | The Special prefabs |
| **Hunters** | 6–12, in groups of 3–5 | Grown members | Lead the trail at night. Come at a target from two sides. By day some walk the range as the day party, and the rest lie in the resting ring | Hunter list: Bacon's civilian and military characters, or Special Zombies' runners if the stack has them (Bacon 1.8.0 has no runner prefab, and every Bacon character runs to an order at about 3.7 m/s) | Adaptation's runner list, then Common |
| **Scouts** | 1–2 | The lightest hunters | Sit on ridgelines by day and at dusk. The scout rule ([Bodies](#bodies)) | Scout list: light runners, ideally a screamer variant | The Hunter list |
| **The young** | 0–8 | The recently taken, and the ones who ran | Erratic. Scattered ahead of the hunt and to the sides. First to any noise, and reckless. By day some walk with the day party | Young list: runners, in the look of where they were taken | The Hunter list |

**Looks.** Promised, because Bacon has them: **civilian** and **military**, plus the heavy for alphas.
Optional lists under **Zombies > Tribes > Looks**: **work clothes**, **dust-grey** and **outdoor**.
They're used only if the mod stack has such prefabs, and an empty list falls back to civilian.

### Samples

**Built** ([Samples](../docs/INFECTED.md#kill-sites-the-vigil-and-the-alpha), lone ones and staged fights too). Still
designed: the cap of 4 guard samples per played hour on the server, and the status log's samples per
player-hour by source.

### The daily rhythm, the call and the hunt

**Built** ([The daily rhythm](../docs/INFECTED.md#the-daily-rhythm), [The call](../docs/INFECTED.md#the-call), [How a hunt comes for
you](../docs/INFECTED.md#how-a-hunt-comes-for-you)). Still designed: the alpha's echo shifting the times; the
stragglers after a bad night (06:00–07:30); the Hunter's reading of the nearest call; two callers when restless; *"a short-handed call after a quiet day"*; a cunning tribe
putting a group on your back trail; heat that a hunt lost you at ignored within 300 m for the rest of
that night; a hunted player who logs out (the hunt waits 10 real minutes).

### Temperament, habits, memory, who they are

**Built** ([Tribes](../docs/INFECTED.md#seeds-and-places), [Memory](../docs/INFECTED.md#memory), [Who they are](../docs/INFECTED.md#who-they-are),
[The signs](../docs/INFECTED.md#the-signs)). The signs table now lives in the docs. Still designed:
- the factions' voices for an alpha's history (the Haven's *"The Quarry lot are Josef Hruby's now. He
  drove the school bus."*, Bastion's contact log, the Red Hand's new boss, a Haven elder's warning on
  the road): their names and dusk voices are built ([People](../docs/INFECTED.md#people));
- where it lies up by its look (dust-grey under cover, outdoor with the widest range and highest
  posts); the Radio Operator and the Hunter naming a tribe from its call pattern;
- relayed calls ridge to ridge, the stragglers after a bad night (06:00–07:30), the reckless young
  going to any noise within 400 m ahead of a hunt;
- hunting grounds following the prey;
- breaking a habit by taking its prey away (rerouting trucks, stopping the walks), which needs those.

## Day parties

**Built** ([Day parties](../docs/INFECTED.md#day-parties)). Still designed: the Hunter's tracks line for a day party
(*"Three, this morning, along the hedge toward the quarry. Young ones."*) is built as the general
tracks line; the Haven's warning about day parties in the fields when a tribe is hungry; Furious tribes'
day parties on the capture road.

## Towns: the crawlers

**Who the crawlers are.** They're a town's own people: the ones who turned lying still. They're slow,
swollen and many. They never leave the town where they turned, except to drift to noise and to scavenge
after a tribe. Towns have no leaders, because leadership lives in the wild. That replaces the Grounds
design's town firsts. Towns by day stay the crawlers': no tribe's day party comes inside a settlement's
radius.

**Prefabs.** The **Zombies > Crawlers** list takes Bacon's slow civilian prefabs, and its bloated ones if
the pack has them. When it's empty, Common is used. From Overrun, up to 5% of the crawlers in towns at
70+ infection can come from Bacon's exploding variant, if one is set: the old, swollen ones in dark
towns. *Honest note:* phase 0 measured every Bacon 1.8.0 character at about 3.7 m/s on an order and in a
chase, so "slow" needs a slow prefab in the stack (VERIFY in MOD_STACK.md's prefab list). Without one,
crawlers read as many and loud, not slow.

**How many.** Exactly today's formula: infection × size × stage density × events. Two things from
Grounds are kept:
- **The kill ledger.** Each kill adds 1 to its town's ledger, and still takes 1.5 infection off. The
  ledger halves every 30 played minutes, and it's saved, so walking 1.1 km away and back never refills a
  town. After the cure it stops halving ([The last week](#after-the-cure-the-last-week)).
- **Reinforcements.** What refills walks in from the buildings: crawlers spawn at building rests 200 m
  or more from the player and walk into the street. A cleared street stays clear for a while, and then
  you watch it fill again.

Every kill in a town also makes that town's rise in infection smaller, so the tribe next to it gains
fewer of the ones who ran (model rule 5). Clearing a town starves its neighbour.

**Where they stand** (every spot still passes `FindSpawnPos`)

| When | Building rests and doorways | Streets and yards | Elsewhere |
|---|---|---|---|
| Day | 70% | 30% | — |
| Dusk | 50% | 50% | — |
| Night | 30% | 60% | 10% out in the fields at the town's edge |
| Stirred (below) | 20% | 50% | 30% along the tribe's trail and at its kill site |

- **Building rests** are points 2–6 m outside a building's doors (from the door system) and in its yard.
  Never at a building's origin. Spawning inside is an option only if Bacon's AI doesn't stick indoors
  (VERIFY).
- **Drift to noise.** A crawler group within 300 m of a remembered noise (the last 16) is sent to it
  once, through its held waypoint ([Bodies](#bodies)), and stays about 90 s. Today only mission infected
  do this; now every crawler does, slowly. It stops when a player is within Bacon's perception range of
  the group, because by then Bacon's AI has it.
- **Stealth** (built) applies to crawlers as to any infected the spawner tracks: the common base, and
  they can lose a hidden player.
- **The fields.** By day a few crawlers stand in the fields at a town's edge, where they drifted in the
  night. They're slow, visible from far off and easy to go round: a warning before the town, not a
  threat. A Hungry tribe's day party can come as far as these fields.
- **The nest** (Burn the Nest, Specimen from the Nest) is where a town's crawlers lie up thickest. It's
  unchanged apart from Burn the Nest's brief.

**Who a crawler makes.** Someone bitten or killed by crawlers in a town becomes a crawler there
(infection, as today), never a tribe's young. A tribe only gets the people its own bodies take, or who
turn out in the open (model rule 4).

**Stir** (0–100 per town; falls 20 an hour played)

| Cause | Stir |
|---|---|
| A tribe's hunt passes through at night | +30 |
| A tribe's kill in town | +30 |
| A border fight within 1 km | +20 |
| A scout's call within 600 m | +10 |

At 50 or more the town is stirred. Its crawlers come out into the streets and drift toward the last
noise, and its centre square makes 5 heat a minute. Stir heat stops at 60, below today's horde trigger
of 80, so a stirred town alone never brings a horde. A player's own noise on top of it can. Stir never
spreads from town to town.

**Scavenging.** A kill site within 1 km of a town draws 2–4 of its crawlers the next night. They walk out
to it and stand there until dawn: *"The slow ones came out to where it happened."* They count as `out` of
the town. Their bodies exist only near players.

**When a tribe passes through**
- A tribe's hunting grounds include town edges and squares, so its trails cross 1–3 towns at night.
- **Giving way.** While a tribe's part is in a town, the crawlers give way: placement shifts half indoors
  and to the side away from the trail. *"Even the slow ones keep out of their way."*
- **Behind it.** After it passes, the town is stirred, and the crawlers come out behind it.
- **Separate.** Crawlers never join a tribe, and a tribe never lies up in a town, except in Winter (model
  rule 22). By day a tribe never enters one.
- **Starving walkers** do end up as crawlers: +2 infection each.

**Doors and sieges.** Infected at a locked door still bang on it and make heat, as today. That heat never
gives a hunt a target. Once a hunt has lost its target, it ignores heat within 300 m of where it lost it
for the rest of that counted night. Crawlers at a door give up after the leash's 10 minutes, and the door
holds. So hiding in a house is a real escape, not the start of a siege until dawn.

**What it means for players.** The danger is clear:
- **Towns** are the slow danger: many, visible, and loud if you're loud. By day they're where the loot
  is. At night a tribe may come through.
- **The wild** is the fast danger: a few of them by day, in twos and threes on the paths and never the
  leader; the whole tribe after dark.
- **No man's land,** the 300 m round every lived-in place, is the safest walk in the Zone. Hunts and day
  parties stop short of it, and crawlers in held towns behave as today.

**Hordes and lookers.** Today's heat horde (80 heat within 600 m of a player, at most every 10 minutes)
now comes from somewhere:
- **near a town:** from the nearest town within 1.5 km, as crawlers spawned 300–450 m out on that town's
  side. They're slow, and they come from a direction;
- **by day inside a range, away from towns:** from that tribe's day party only, inside its 6-body cap;
- **at night inside a range:** from that tribe's hunters, fast, along its trail, within the hunt's
  commitment (model rule 8).

Favourite-spot lookers come from the nearest town's crawlers by day (or the day party, inside a range),
and from the tribe's young at night inside a range. Spread between towns is unchanged.

## The land

**The land is read, never placed by hand.** The land grid ([The model](#the-model)) gives every cell
water, height, trees, buildings, roads and bridges, and marks it wild or not. Everything below comes from
it, so it works on any map. Where the grid can't see something, the doc says so plainly.

**What each kind of land is for**

| Land | Role | What players notice |
|---|---|---|
| **Rivers and lakes** (river entities and the water query; the sea is always found; ponds aren't) | Borders. A range never spans water, and trails cross only at crossings | You can sit on the far bank and listen to a tribe that won't cross |
| **Valley floors** (always found) | Borders where there's no river: two ridges, and the low line between | Calls answering across a valley |
| **Bridges and fords** | Contested. Two tribes facing each other over water share a crossing border, and Border Nights happen there. Blow the Bridge picks a tribe crossing one, when the grid found one | The worst place to be after dark, and the best place to ambush |
| **Hills and ridges** | Scout posts: the two highest cells in a range with a view of its trails | A still figure on the skyline at dusk |
| **Forests** | Lairs (at forest edges), along with barns, mills, quarries and culverts. Trails and day walks keep to forest edges and field margins | Birds gone quiet. A gate swinging. By day, two or three moving along the edge of the trees, and little else |
| **Fields** | Crawlers that drifted out of town stand in them by day. Day parties walk their edges; a Hungry tribe's comes to the fields at a town's edge. Tribes cross open fields in force only at night | By day: slow ones, far off and easy to go round, and now and then two quick ones along the hedge. At night: don't be in the open |
| **Roads** | Prey. Tribes cross them square, and follow them only for a road habit, as nomads, or behind a driver alpha | A truck's hour becomes a tribe's hour |
| **Towns** | Crawlers' land, and tribes' hunting grounds at night | See [Towns](#towns-the-crawlers) |
| **Burned land** | Event fires: no lair and no trail for 3 season days. A player's fire: no lair, and trails avoid it for one night | A safe corridor for a few days after a big fire |
| **No man's land** | The 300 m round every lived-in place. No hunt and no day party goes in, and testing the wire stops at its edge plus Bacon's perception range | The safest walk in the Zone |
| **The line** | Nothing settles within the warning zone plus 200 m | *"Even they don't go near the line."* |
| **The lower store and Bastion's held store** | No lair within 300 m (conflict 50) | Nothing lies up next to the finale's stores |
| **The Long Table's farm** | No wild land within 700 m, and trails and day walks turn back 1 km short | A Hunter: *"The tracks come up to the old wall and turn back."* (+3 suspicion, once per character) |

**Weather and light.** How well the infected see a player in each is stealth's job (built): light, rain,
fog and storms lower a player's visibility. The table says only what tribes and crawlers do.

| Condition | Tribes | Crawlers |
|---|---|---|
| Clear night | Hunt as normal | Out in the streets |
| **Rain** | Hunts are shorter (home by 03:00): a good time to cross a range. Day parties keep to the forest edges. Tracks read fresh after rain (the old ones are washed out) | Drift less (rain already makes heat ×0.7) |
| **Fog and fog banks** | Count as dusk: scouts sit lower, and a scout's call brings the dusk pack, so the alpha of a bold tribe can come by day in the fog. Hunters get closer before you see them | Spawn closer, as today |
| **Storm** | Everyone lies up, no day party, and nobody hunts: the Storm event's "a good time to move" is literally true | Indoors |
| **Winter** (the rule set) | Lairs move into empty villages' barns from Overrun. A Hunter always reads tracks as fresh in snow | Thicker in the dark towns |
| No game clock | Always day: day parties walk, scouts post, nobody hunts | As by day |

**Frenzy nights** (Every Ridge Calls and Red Dusk, ignoring fear places and borders) and **the bright
moon**: **built** ([docs/INFECTED.md](../docs/INFECTED.md#contracts-and-events),
[Signs and sounds](../docs/INFECTED.md#signs-and-sounds)). Still designed: the crawlers' ×2 at a
frenzy's height lasting the night.

**Chain reactions:** built ([docs/INFECTED.md](../docs/INFECTED.md)) but for Siren Test's other tribes keeping away
from that town that night (the bells habit's tribes coming is built).

**Animal tells** (text only; once per range an hour, at most one a minute per player):
- over a lair by day: *"No birds in this wood."*
- a day party close by, out of sight: *"The crows went up off the hedge line and didn't come back down."*
- before a hunt passes a village: *"Every dog in Levie is barking."*
- a Hunter gets the detail: *"No birds, and the grass is laid flat toward the quarry."*
- the day after a feed the birds are back: *"Rooks over the quarry again."*

## Bodies

Bodies follow the data, inside today's spawner rules. With **Zombies > Tribes** and **Zombies >
Crawlers** both off, the spawner is exactly today's.

**The rule that shapes everything: perception range P.**
- **What phase 0 measured.** Bacon's sight is a chance, not a radius. By day 2 in 3 infected engage a
  standing player within 30 s at 25 m, then about 1 in 4 per 30 s from 50 to 200 m (about 1 in 6 at
  night). A walker passing one in the open is seen across a band about 210 m wide each side by day and
  190 m at night. A hill blocks it completely. Crouched or prone cuts it to about 1 in 8; running raises
  it. A gunshot brings blind infected from 80–170 m. And spawn order is the real switch: an infected
  spawned after a player is already in its sight charges at once, from 300 m.
- **So P = 250 m in the open, and 0 behind terrain.**
- **Beyond P,** parts are steered with waypoints.
- **Inside P,** nothing that matters may depend on a waypoint, because Bacon will charge whatever it
  sees. So all choreography near players comes from **where and when bodies appear**:
  - flanking comes from spawn points;
  - the core holds back by not existing yet;
  - a scout that calls simply charges;
  - wire-testing trails end at no man's land plus P.
- **No body spawns within 300 m of a player in terrain line of sight.** A part spawns at the first
  moment its trigger distance is met while it's out of every player's sight (behind terrain) or 300 m
  away. A part whose trigger is 300 m or less (the lair at 300 m, post-feed sleep at 150 m, the core in a
  hunt) that would be in sight spawns as the player crosses 300 m instead. The scout's pack uses spawn
  order on purpose: it appears out of sight and comes straight at the seen player.
- **Within 80 m of a player** a group is never redirected: the fight is Bacon's. (Whether a new order
  pulls a group off a live target wasn't measured, so the rule stays.)

**Budgets** (settings under **Zombies > Tribes**)

| Budget | Value | Today |
|---|---|---|
| Living infected agents on the server | 100, counting group members. Phase 0 measured 100 agents at a p99 frame of 11.6 ms on a shared 4-vCPU box, and 140 at 14.5 ms | none: 60 tracked entries, which can be 200+ AI with group prefabs |
| The engine's active-AI limit | It defaults to 128 for **all** AI (faction AI, wanderers and crews share it). Raised to 160 with `SetLimitOfActiveAIs` when Tribes is on (phase 0) | 128 |
| Per player cluster (players within 600 m share one) | the share: 28, or 100 ÷ the number of clusters if that's less. At night in the wild a tribe's presence comes first; in a town at least 10 are crawlers | none |
| One tribe at one cluster | 14 × share ÷ 28. Its first wave is 8 × share ÷ 28, never under 3. When the server binds, every presence shrinks; no tribe is dropped to data while a player is in its range | — |
| Day parties | 6 bodies near one player cluster at most, from every tribe together, out of the cluster's share | — |
| Alpha bodies | 3 on the server. Never two within 1 km of one player, except in a staged Border Night or Clash. Over the cap, the guard comes without its alpha | — |
| Tracked entries | 60. Order: alphas, scouts, the tribe part nearest a player, day parties, crawlers, loose ones | 60 |
| Spawns per tick | 8 agents (not entries), one tribe group at a time, so a 14-body presence arrives over 2–3 ticks, never in one hitch | 4 entries |
| Waypoint orders | 8 per 5 s tick on the server, nearest players first. One per group every 20 s; a day party's hop every 4–6 minutes. Never for a group with a player within 80 m. Orders move a held waypoint; they don't spawn one | — |
| Scout sight checks | every 1 s (the bite check's callback), only for spawned scouts with a player within 200 m: 6 scouts and 12 line-of-sight traces a second at most | — |
| Scout calls | One per tribe every 10 minutes. A pack at one player cluster at most every 10 minutes, and 2 day packs per tribe per game day | — |
| Props (kill sites, abandoned lairs, marks) | 6 per tribe within 200 m of a player, 30 on the server. Removed beyond 250 m, never saved | — |
| Data | 60 s tick: 15 tribes, about 100 parts plus up to 3 day-party groups a tribe (5 when hungry), 2 lone ones, 4 splinters. 10-minute tick: about 105 border pairs (neighbour lists built once). Land build: one square at a time, at the first start only, then the cache | — |
| Save | about 28 KB | — |

Mission infected count toward the caps but are never trimmed, as today.

**Plumbing, built first** (phase 1, except the waypoint owner, which is phase 0):
- **Tribe fields on `RZ_TrackedZombie`:** tribe, part, role, member ids with their entity references,
  the held waypoint, and silent kills pending.
- **Telling who died.** A member missing from its group's living agents has died (phase 0: the count
  drops within a second, the group deletes itself when its last member dies, and the bodies stay). That
  tells the code which named young, bonded pair, guard or alpha it was.
- **Runtime groups.** Role and look lists are character prefabs, so a part's group is an empty
  `SCR_AIGroup` filled with `AddAIEntityToGroup`. This is the main path, and phase 0 passed it in every
  case (VERIFY 1): an empty group survives until its members arrive, they join active, and a one-member
  group takes waypoints. The alpha is a one-member group made the same way.
- **One owner of group waypoints** (conflict 47). One helper owns every AI group's waypoints:
  - **the held move waypoint:** one per group, spawned on the first order, then moved and put back on the
    group's list for every order (phase 0 measured that moving it while it's on the list does nothing).
    `RZ_Util.SendGroupTo` and `RedirectGroup` go through it;
  - **stealth's search waypoint:** `RZ_Util.PushGroupWaypoint` puts it in front of the held one and
    `PopGroupWaypoint` takes it back, through the same helper. It's never pushed over a tribe's held
    waypoint during a hunt (hunts don't search, below). For a day party or anything else, the group
    carries on with its held waypoint once stealth's goes;
  - **delete with the unit:** `RZ_Util.DeleteUnit` deletes the group's waypoints with it, and a sweep
    each tick deletes those of groups that died out on their own.

  That fixes the leak today's code has (phase 0 confirmed it: finished waypoints drop off the list but
  stay in the world, and a deleted group leaves its pending one behind), and it stops each order
  spawning and deleting a replicated entity. The move waypoint prefab has no RplComponent (checked).
  *Where it stands:* phase 0 built the held waypoint as `RZ_GroupOrders` on its branch (not merged):
  `SendGroupTo`, `RedirectGroup` and `DeleteUnit` go through it, and the spawner's tracked records own
  their group's. Stealth's push and pop still spawn their own, and move in before it merges, in the same
  commit.
- **A silent kill path.** `KillSilently(entity)` marks the tracked record before it acts, and
  `UpdateTracked` takes silent kills off the drop before `OnZombiesKilled`, so they give no samples, no
  infection change, no ledger, no searchable body and no mission credit. Phase 0 found two ways, and both
  are used:
  - **delete** (out of sight): no body, no loose item, and the group cleans itself up;
  - **kill and mark** (a Border Night's losers a player can find): a body and its one loose item, left for
    the loose-item cleanup.
- **Per-member samples.** Alpha and guard samples are dropped where that member died: 2–3 for the alpha,
  1 for a guard. Not today's one sample per kill event at the leader's position.
- **Death attribution:**
  - the bite detector stores the tribe of the body that hit each player, and when;
  - a player-killed event reads it (VERIFY 9, not measured yet);
  - a turning reads the position.

**Parts, when and where they spawn, and a hunt by placement:** **built** ([Tribe bodies](../docs/INFECTED.md#tribe-bodies),
[How a hunt comes for you](../docs/INFECTED.md#how-a-hunt-comes-for-you)).

**Steering beyond P** (trails, day walks, the dusk set-out and the dawn walk):
- **Trails** are 4–6 points from the lair to a hunting ground. They stay on land, cross water only at
  crossings, avoid fear places, burned cells and no-lair overlays near the finale's stores, prefer forest
  edges and field margins, and cross roads square. They're worked out from the land grid (at most 20 a
  tick) and cached in memory. Day walks are made the same way inside the range.
- **Movement.** Phase 0 measured that Bacon runs every member to a waypoint (about 3.7 m/s; a group's
  centre covers about 3.5 m/s), that groups follow move waypoints, idle in place without one, and go back
  to their waypoint about 6 s after a fight. So:
  - a group is sent to its next point when its centre is within 15 m of it (an order completes when the
    first member is within 5 m), or when it hasn't moved 8 m in 90 s;
  - what reads as fast is the chase and the scout's pack; what reads as a tribe is how they move: small
    groups, in file, out of the towns;
  - day parties hop and hold, so the wild by day is a few figures that stop and start, not a stream.
- **The leash.** A hunting part goes home after 10 minutes at a target, or once it's 1.5 × its range
  radius from the lair, so players see it go.
- **The hide rule.** A hunt that has lost its target for 60 s goes home and never switches to another
  player. Its target is lost when they're indoors behind a closed door, 300 m or more away, inside no
  man's land, or out of the server (below). Bacon's own perception never forgets a target (phase 0: a
  chaser kept walking toward the target's real position for 2 minutes and 380 m), so this is scripted:
  the group gets a held-waypoint order home once it's more than 80 m from every player, and once it's out
  of sight beyond 250 m the part goes back to data with a silent delete. Heat within 300 m of that spot
  can't target it again that counted night.

**The scout rule and the alpha's body:** **built** ([Scouts](../docs/INFECTED.md#scouts), [Kill sites, the vigil and
the alpha](../docs/INFECTED.md#kill-sites-the-vigil-and-the-alpha)). Still designed: scouts moving to posts above
tonight's trails at dusk and sitting lower in fog; a marked player seen from 1.5 times as far; the
building query (a player inside never seen; the code uses its sight trace within 80 m); a pack re-sent
after 20 s (the behaviour layer follows the last-known spot instead); the corpse kept from the
engine's cleanup.

**Kill sites, the vigil, staged fights, splinters and moving tribes:** **built**
([docs/INFECTED.md](../docs/INFECTED.md#kill-sites-the-vigil-and-the-alpha), [Borders](../docs/INFECTED.md#borders), [The life
cycle](../docs/INFECTED.md#the-life-cycle)), and their props ([Signs and sounds](../docs/INFECTED.md#signs-and-sounds)).
Still designed: the samples kept from the loose-item
cleanup during the vigil; the corpse kept from the engine's cleanup (VERIFY 15); a second infected
faction hostile to the first, so fights could be seen (behind a setting); loose ones as starving
walkers and members crossing between ranges at night; the capture case's tribe following it.

**Leaving the server**
- **A hunted player who logs out.** The hunt's data waits at the spot for 10 real minutes. If they come
  back in that time, it resumes from 150–250 m out of sight.
- **The capture case.** If its carrier leaves, the case is left where they stood at once, as Live Capture
  already does. Players within 300 m are told, and the tribe following it goes there.

**Restarts**
- **What's lost:** the bodies.
- **What loads from `state.json`:** tribes, alphas (with their health), lone ones, splinters, kill sites,
  marks, ledgers and stir. Today's day party is worked out again from the clock.
- **Claims** are taken again, as missions do.
- **Hunts and vigils:** a hunt that was running restarts from the lair at the next dusk, and a vigil that
  was running ends.
- **The game clock:** phase 0 found it goes back to the scenario's start on a restart. Until the time of
  day is saved ([Phase 0 results](#phase-0-results)), the phases follow the reset clock.
- **The land grid** loads from its cache. Tribes stay dormant until it's ready.

**What Bacon Zombies does and doesn't do**
- **Its AI is untouched:** fighting, chasing, banging on doors and animations are all Bacon's. ReforgerZ
  only chooses where groups spawn, what's in them, their waypoints, and when they're removed. Stealth
  (built) sets their perception factor.
- **Speed, toughness and look come from prefabs.** Each role and look's prefabs are listed in
  MOD_STACK.md.
- **Infected never fight each other** (one faction), so fights are heard, then found.
- **No perception hooks beyond stealth's factor.** Scout sight reads stealth; Bacon's own perception is
  measured (phase 0) and designed round.

**Turning off**

| Off or empty | Effect |
|---|---|
| **Zombies > Tribes > Enabled** off | No tribes. Loose ones and heat hordes as today (crawlers still work if on) |
| **Zombies > Crawlers > Enabled** off | Today's town spawns, exactly |
| Both off | Today's spawner, exactly |
| Under 1.5 km² of wild, and no **Slots** override | Tribes off, logged |
| **Day party share** 0 | No day parties: the wild by day is scouts and the lair only, as the design was before the author's change |
| **Zombies > Stealth** off | Scouts see anyone out to **Scout sight**; tribe bodies keep their own perception; nobody is lost through stealth (hunts still end by the hide rule) |
| The land grid can't read trees, rivers or bridges | Lairs at landmarks and open lie-ups; borders on ridges and valleys; straight trails that avoid the sea; logged |
| The Alpha list | The Special prefabs. All empty: alphas are data and the alpha jobs turn off |
| Hunter, Young, Scout and optional Look lists | The role's fallback ([Tribes](#roles)) |
| Runtime groups fail | Group prefabs only, with no per-member tracking: no named young, bonds, ID cards or per-guard samples. Alphas are data |
| No move waypoint prefab | Steering off. Placement by phase only. Packs still spawn at the call point |
| Tag item, call box item | Plant a Tag and the call box turn off |
| **Your dead run** off | Player deaths never make young |

**VERIFY, riskiest first** (15 items for the WORKBENCH_SETUP list; the stance read is a standard API and
isn't one). Phase 0 measured most of them with the zombie lab on Everon, with Bacon 1.8.0:

| # | What | Phase 0 |
|---|---|---|
| 1 | **Runtime groups.** An empty `SCR_AIGroup` made at runtime takes character prefabs with `AddAIEntityToGroup`, doesn't delete itself before its members arrive, and a one-member group takes waypoints | Passed |
| 2 | **Waypoints.** Bacon groups and move waypoints: idle, without one, and after a fight. Whether a completed waypoint leaves the group's list and stays in the world, and whether moving a held waypoint retargets the group | Measured: they follow, idle in place, resume after a fight; the leak is real; a held waypoint must be re-added to retarget |
| 3 | **Perception.** Bacon's sight, hearing and gunfire reaction distances (P) | Measured: P = 250 m in the open, 0 behind terrain; gunfire 80–170 m |
| 4 | **Speed.** The waypoint's movement type and whether Bacon honours it, and the real speeds of runners and the heavy | Measured: always a run, about 3.7 m/s; the heavy 4.9; no runner prefab |
| 5 | **Frame time** at 60, 100 and 140 agents | Measured: 100 is fine; raise the engine's AI limit |
| 6 | **Silent kills.** Killing an AI cleanly from script, with the spawner reading it as silent. Optionally, a second infected faction hostile to the first | Silent kill: two ways, both work. Second faction: not tested |
| 7 | **Alpha health:** reading and setting it | Works; no self-healing |
| 8 | **The game clock.** The configured time acceleration and night length, and whether the clock survives a restart | Default real time; cap 12×; resets on restart |
| 9 | **Player deaths.** The player-killed event and what it says about the killer, for attribution | Not measured |
| 10 | **Land queries.** Tree query cost, the road network's point heights (the bridge test), and a river or lake query, over all of Everon | Measured: rivers and water found; bridges from road segments; trees by prefab path |
| 11 | **A waypoint prefab with no RplComponent** works on a dedicated server | `AIWaypoint_Move` has none |
| 12 | **The line-of-sight trace:** terrain-only, and its cost at 12 a second (now also used for spawning) | Not measured |
| 13 | **Facing.** Idle AI keeps the facing it spawned with | Not measured |
| 14 | **Replication.** One-shot call sounds and sign props replicate cheaply | Not measured |
| 15 | **Optional:** keeping a corpse from the engine's garbage cleanup | Not measured (the moon-phase reading exists) |

## Stealth and the tribes

Stealth is built (`Zombies/RZ_Stealth.c`; [DESIGN.md](../docs/DESIGN.md), "Stealth"). Conflict 46 makes
it the one judge of whether the infected see a player, so a player lying low is judged the same way by a
scout, a day party, a crawler and a mission.

**What stealth does to each tribe body.** Every tribe group is a tracked unit, so stealth sets its
members' perception factor like any other: its kind's base, times a factor from the most visible player
near it. Whether stealth's search can lose a player depends on what the group is doing.

| Bodies | Base (Stealth > Base factors) | Lost through stealth's search? |
|---|---|---|
| Alpha and guard | special, ×1.2 | Yes at the lair by day (they search, then go back to the lair); no in a hunt |
| The resting ring, scouts, the young | common, ×1 | Yes |
| **Day parties** | common, ×1 | **Yes.** Hidden, unseen and unheard for 8 s, they search 45 s where they last had you, then go back to their walk |
| A scout's pack (day, dusk or fog) | common, ×1 | Yes. Then it goes home |
| **A hunt on its trail** (from the dusk set-out to the dawn walk) | horde, ×0.9 (a crowd on the move looks ahead) | **No.** Like a mission horde, its own rules end it: the hide rule and the commitment |
| **The Furious last wave** at the final stand, and everything within 1.5 km of the lab during the stand | never lowered (built) | **No** |
| The Tribe Moves | horde, ×0.9 | Yes: staying out of its sight is the point |
| Crawlers | common, ×1 | Yes; town hordes as today |

**How the scout rule uses it.** The scout's radius is `RZ_Stealth.SightRadius(playerId, Scout sight)`
([The scout rule](#bodies)). Its own stance, light and weather multipliers are gone: stealth already has
them. It keeps only what stealth doesn't know (a mark, a vehicle's noise, the Hunter's eye, buildings).

**One owner of waypoints.** Stealth's search waypoint and a tribe group's held waypoint go through the
same helper ([Plumbing](#bodies)). During a hunt stealth never searches, so it never pushes over the
held waypoint. For anything else, its waypoint goes in front of the held one and comes off after the
search, and the group carries on where it was going.

**Limits to remember.**
- Stealth touches at most 40 infected an update; the rest wait their turn. With day parties, a lair and
  crawlers near one player, watch that cap in play-tests.
- Stealth has no cover input: a hedge or a wall counts only through the engine's own sight, and the
  scout's trace is terrain-only. Buildings are the exception, through the building query.

## People

**Built** ([docs/INFECTED.md](../docs/INFECTED.md#people)): the words and their tokens, voices, marks
(with their props), knowing and the ways to learn (with the map's range rings), skills, your dead run,
the dark corners, the factions' sides, the moat rule, the 13 contracts (Stock the Pit among them), The
Lone One, the events, the changed missions listed in [MISSIONS.md](../docs/MISSIONS.md), frenzies
ignoring fear places and borders, and HC1's and H3's hooks ([QUESTLINES.md](../docs/QUESTLINES.md)).
Still designed:
- **Lead Them Away's** drifting horde as a moving tribe (today it's a pack's hunting party, or a horde).
- **The jobs' rest:** Clear the Trail on a truck road; Burn the Lair's splinter when 4 or more died;
  Bait the Crew's Zombie Bait Infamy when either side lands on a faction's people; Track Them Home's
  "not closer than 80 m"; a Furious tribe's day parties walking the capture road, and couriers and
  columns needing an escort.
- **Questline hooks:** B1 Await Relief (the Day 3 runner's road crosses the column's range, and his page
  can be at an old kill site); R1 The Book (a page for "the crew in the woods", the moat's feeding, as a
  rumour); HC3 Authors (tagged alphas numbered in the paper's appendix); the finale routes' Act III live
  tissue and The Lower Store claiming the site tribe in Act II ([FINALE.md](FINALE.md)); seeds for later
  questlines (the Haven's Bring Them Home chain, Bastion's Not Our Sector, the Red Hand's The Moat,
  Halcyon's Index).

## The season

**Pacing** (built: [DESIGN.md](../docs/DESIGN.md), "Season pacing").
- **Which clock runs what.** Slow rules run on season days, so the arc lands on the same stages whatever
  the season speed. Nightly rules run on counted nights, so hunger and takings stay in step with real
  hunts ([The model](#the-model), Clocks).
- **The day numbers** below are for 5 regulars at Season speed 1, the built design point: Early Outbreak
  days 1–5, Collapse from about day 6, Overrun from about day 16, Aftermath from about day 31, the 90% gate
  about day 39, and the cure about day 44 with a finale. A season day is about a real day there, with
  about 2 counted nights.
- **Other servers.** The stages follow hours played, so a small server stays in the earlier stages longer
  (two regulars never reach Aftermath before the cure), and a bigger one gets there a few days sooner. At
  a higher Season speed the same stages come sooner, with fewer counted nights a season day: tribes grow
  mostly from the ones who ran, hunt less often per alpha, and show fewer Clashes. The arc holds, and less
  of it is seen.

**The arc**

| Stage | Tribes | Crawlers | What players notice |
|---|---|---|---|
| **Early Outbreak** (days 1–5) | The seeds: the column on its road, the site tribe in the hills, and on bigger maps one or two more. 10–14 each, with Rising alphas. By day a few of them walk the woods in twos and threes; at dusk, one or two voices | The towns' own, as today, lying up by day | A call from the hills at dusk: *"That's not a dog."* Two figures along a wood's edge at noon that don't move like people. A wanderer who never arrived. The first kill site on a road |
| **Collapse** (from about day 6) | Towns climbing past 60 feed the tribes next to them, and new tribes form from the dead in the open and from towns gone dark, at most one a season day, until the unkept slots fill. Bastion's pullback (presence ×0.6) loses posts, and one becomes a military tribe. Tribes reach 12–20, and their day parties grow with them. The first road habits on truck routes, the first lair moves, the seed alphas reaching Prime, the first old borders | More of them as infection climbs; towns stirred at night | The wild has borders. The Haven channel starts naming them. The Haven truck waits for dawn. Bastion's nightly call starts reading contacts. Players learn that clearing Levie quietens the Quarry |
| **Overrun** (from about day 16) | The caps fill. Border Nights at the bridges when hungry. The first split once a tribe passes 25 with a kept slot and room: Everon's clan, if it comes, around days 18–30. The seed alphas reach Decline around days 20–26, bringing Clashes, new voices and lone ones. Hungry tribes test the outposts' wire by night, and their day parties come to the fields at the towns' edges. Bastion's culls push one into Haven country | Thick in the dark towns. Starving tribes' walkers swell them. The first bloated ones that burst | The map changes week to week: the Quarry lot hold the river now. Somebody's dead character runs with them |
| **Aftermath** (from about day 31 to the cure) | Fewer tribes, bigger and older, with ranges drifting toward the living. Bastion's culls stop (presence 0.1). Winter (the rule set) brings them into the villages. On a Chernarus-size map a great alpha can rise. From the gate, every finale route wants live tissue | Fewer towns, and fuller ones | *"Whatever is left in here has had time to learn the roads."* The choices: which alpha to take alive for the lab, who gets to carry it, and which to leave as a moat |

By map:
- **Arland:** no tribe, because its wild land is under the floor (phase 0). The crawlers carry the season.
  With **Slots** set, one tribe that grows mostly from the ones who ran and, with nowhere to split, goes
  restless, so its big story is a Clash and a voice nobody knows, if it lasts.
- **Everon:** two seeds, room for four by mid-Collapse, rivers and bridges for borders, and a clan in
  Overrun in about half of seasons.
- **Cain:** two seeds and room for three.
- **Chernarus-size:** four seeds, a dozen tribes by day 15, two or three clans in Overrun, and some seasons
  a great alpha in Aftermath.

**Time of day** is in [Tribes](../docs/INFECTED.md#the-daily-rhythm), [Day parties](#day-parties) and [The land](#the-land)
(weather and light). Bastion's routines are unchanged, and soldier alphas echo them.

**Events through the season:** built ([MISSIONS.md](../docs/MISSIONS.md#world-events)).

**The cure.** The 25%, 50%, 75%, final stand, won and lost rows are built ([The cure](../docs/INFECTED.md#the-cure)). On the built research: the Cure pace, the pace by play, the lab's capacity of 3% a real
day, the cold store, and the 90% finale gate.

| Research | Tribes and crawlers |
|---|---|
| 90%, the finale's gate | **Live tissue.** Each finale route's Act III needs a live specimen, carried to the lab in Live Capture's case. [FINALE.md](FINALE.md), "Live tissue", owns the order of specimens and the acts. The tribes' side: the best specimen is an alpha, Prime or older (at its lair by day while its day party is out, or on its trail at night), else a lone one, else two guards (they stay at the lair by day); with tribes off or none in reach, two Live Capture specials. The case can be taken from whoever carries it, like a robbed truck. **Whoever hands it in credits a route's track: a locked player their own, an unsided one the route they choose** (conflict 44). An alpha counts +2, a great alpha +3, never past 9 before Act IV. The same tribe can't give tissue twice within 6 season days, so a one-tribe map's next route takes a lone one or guards. Each alpha caught means a succession and a Furious tribe on the lab road. No route can be locked out: every route has a fallback |

Research never changes the infected themselves. It changes what people know, and how many of the living
turn. Tribe samples are budget-neutral: town specials are scaled down by the tribes' share, so specials
per player-hour stay near Season pacing's assumptions, and the lab's capacity is the hard ceiling whatever
happens.

**Next season.** Tribes are wiped with everything else, and next season's alphas are new people in a new
Zone. A Halcyon win already gives specials ×1.15 (built, "Next-season rules from the last winner"). An
option is for its seeds to start in Prime too.

## After the cure: the last week

**Built** ([The last week](../docs/INFECTED.md#the-last-week)), with the beats kept for the epilogue. Still
designed: the epilogue reading them, and one more beat: **Walking with it** (up to 3 named young still
running with each tribe, players' characters among them by the name setting). The great alpha's rise and
fall are kept as beats (built). The closing work that touches the infected (Quiet the Ridges, Bring Them Home as closing
work, Take Back a Town) is [EPILOGUE.md](EPILOGUE.md)'s.

## From Grounds to tribes: the rename table

Tribes and crawlers replaced the Grounds design's town firsts. The New Vegas designs (dialogue, missions,
side quests, reputation, disguise, epilogue) were written against Grounds' names. Their hooks are generic
(a named leader and its fate, a home, the season day, reasons, signs, lay-down), so they fit, but the names,
flags and contracts would ship wrong. This one table is applied in the tribes commit (conflict 43).

| Grounds | Tribes |
|---|---|
| a ground | a tribe (the pack) or its range (the land) |
| a ground's first | the tribe's alpha (in code and docs); each faction's own word in player text: the Haven's name or "the big one", Bastion's "the heavy", the Red Hand's "the boss", Halcyon's "the index" |
| a den | a lair |
| a garrison ground | a military tribe, founded at a lost Bastion post |
| the Eldest | the great alpha |
| a scatter | splinters |
| Every First Calls | Every Ridge Calls |
| `ground.hungry` | `tribe.hungry` |
| `ground.moving` | `tribe.moving` |
| `first.down` | `alpha.down` |
| `ground.loose` | `tribe.splinter` |
| `{ground:N}`, `{first:N}` | `{tribe:N}`, `{alpha:N}` |
| Lay Them Down | Bring Them Home |
| Still on the Roster | Bastion's lost posts (a military tribe's young, still on the roster) |
| Drive Them | Cull (*"Not our sector"*) |
| Clear the Ground | Clear the Trail, or Cull when it's the alpha |
| Box It In | Clear the Trail on a truck road |
| Not Through Here | Not Tonight |
| The Dawn Walk | Track Them Home, or Escort at Dawn |
| Index Case, Old Tissue | Capture the Index |
| Tag One | Plant a Tag |
| Lone Second (the mission rework's name for Alpha Sighting) | The Lone One |
| a "landlord" (the Red Hand's Bailiff crew name) | Bailiff's trigger becomes "laid down a tribe's leader" |
| Retake the Town "goes to towns with no living first" | Retake the Town goes to towns (crawlers); Clear the Trail and Cull go to tribes |
| Burn the Nest "is a den now, and burning it moves the den" | Burn the Nest stays the crawlers' nest in a town, with its new brief; burning a lair is Burn the Lair |
| The grounds' signs (a pool of 30) | The tribes' props (kill-site things, abandoned lairs, faction marks): 30 on the server |
| The epilogue's grounds beats: still walking, laid down, caught, the Eldest | Still calling, brought home, taken alive for the lab, the great alpha ([The last week](#after-the-cure-the-last-week)) |
| The director's grey choice "with a first in isolation" | "With the index in isolation" (the finale's Act III specimen) |

The season day keeps one definition (the first three stages' hours played ÷ 30), built once, in the New
Vegas plan's phase 1, whichever is built first.

## Build phases

Every phase after phase 0 ends with `tools/compile_check.sh` and `tools/runtime_test/run.sh` (with
**Slots** 2 on Arland, so tribes run in the test), and ships with its docs in the same commit, keeping
WORKBENCH_SETUP's VERIFY count and prefab list current. The original estimate was about 7,300 lines over
five phases; day parties and the last week add to it.

**Phase 0, before any code** (about three days). The old first step, landing the 45-day pacing, is gone:
it's built and merged (commit b78615a).
1. **One owner of group waypoints** (conflict 47), in one commit: the held waypoint per group, moved and
   re-added on each order and deleted with the unit; `SendGroupTo` and `RedirectGroup` go through it;
   `DeleteUnit` deletes with the unit and a sweep catches groups that died out; stealth's
   `PushGroupWaypoint` and `PopGroupWaypoint` go through it too. *Done on the phase 0 branch as
   `RZ_GroupOrders`, except push and pop.*
2. **Tests on Everon with the zombie lab**, riskiest first: runtime groups built with
   `AddAIEntityToGroup`, and one-member groups taking waypoints; one Bacon group with one move waypoint
   for 30 minutes (idle, without one, after a fight, whether completed waypoints stay in the world);
   Bacon's sight, hearing and gunfire reaction distances (P); the waypoint's movement type and the real
   walking and chasing speeds; frame time at 60, 100 and 140 agents; a clean silent kill and, optionally,
   a second hostile infected faction; reading and setting health; the game clock's acceleration and
   whether it survives a restart; the player-killed event; the land grid's tree, road (bridge) and river
   or lake queries over the whole map. **Added in review:** a day party (a group of 2–3 hopping and
   holding along a walk) passing a player at each stance with stealth on, to check that it sees, fights
   and loses a player the way stealth says (conflicts 45 and 46). *Done on the branch except the
   player-killed event, the second faction, the sight trace, facing, replication and the day-party test.*
3. **Measure wild land** on Arland and Everon with a log-only land build. *Done, with Cain too.*
4. **The simulator** (`tools/tribe_sim.py`) with the tribes' rules: takings (including players at the
   capped rate), the ones who ran from measured infection gains, hunger per counted night, player kills at
   20 per player-hour with a measured share in the wild, the night's commitment, splits and the kept
   slot, and day parties. Run it at Season speed 1 and faster, at Arland, Everon, Cain and Chernarus-size
   wild land. It must show Everon's 4 unkept slots filled, a clan in roughly half of seasons, no tribe
   wiped on its first night, and a day crossing lighter than a night one. The results set the data
   speeds, the pack's distance, P, the slot divisor and whether steering and visible fights ship. *Done
   on the branch.*

**Phase 1, crawlers, the land and the plumbing** (about 1,500 lines; playable on its own).
**Zombies > Crawlers**: prefabs, placement by time of day, drift to noise, the kill ledger and
reinforcements, stir and scavenging as data, the siege cap at doors. The agent budget with the
per-cluster share, agents (not entries) per tick, and the engine's active-AI limit. Honest heat hordes
from towns. The land grid with its cache, its log line, wild land and the no-lair overlays. The plumbing:
tribe fields on `RZ_TrackedZombie`, per-member tracking, runtime groups, the silent kill path (both
ways), per-member sample drops, death attribution in the bite detector, the data write-back, and
stealth's kinds for tribe bodies. Admin visibility (`/tribes`, the status line). Loose ones that walk
somewhere. Burn the Nest's new brief. Both off switches. Towns feel slow and full.
Docs: **the new `docs/INFECTED.md`** (crawlers, the land, bodies), added now and not before (decision
16); DESIGN.md's Zombies, Heat and Adaptation sections shortened with links, and its Stealth section
told about tribe bodies; CLAUDE.md's Doc map and the README's doc list; WORKBENCH (settings, log lines,
VERIFY items, the time acceleration assumption); ROADMAP (Next and play-test).

**Phase 2, one tribe end to end, then all of them** (about 1,800 lines). First a vertical slice on Everon:
one seeded tribe with bodies. That means the lair and its resting ring, scouts and the scout rule (1 s
checks, line of sight, stealth's sight radius), the day party (its walks, hop and hold, the 6-body cap,
the day call's 4), the dusk call, a hunt by placement (the young ahead, the flankers, the core held back),
the night's commitment and the long call, the scripted hide rule, the dawn walk and the dawn count, the
alpha's body, health and samples, kill sites in feeding order, the vigil (30 minutes at most) and the
Hunter's tracks. Play-test the slice (a solo day crossing past a day party, a solo night crossing, a
three-player squad at night, a day pass by the lair) before going on. Then every tribe: slots and seeds
(with the island column), the daily rhythm, day parties with hunger's doubling and the town-edge fields,
hunger per counted night, the taken with attribution and the player caps, the ones who ran, data
takings, data hearing, post-feed sleep, weakened tribes, the call as a status channel with one line per
player per dusk, and the journal's names under the place anchor.
Docs: INFECTED (tribes, day parties), LORE (What the infected do), WORKBENCH (VERIFY items 1 to 9, 12 and
13), MOD_STACK (which Bacon and Special Zombies prefabs fill each role and look; civilian, military and
heavy promised, the rest optional; a slow prefab for crawlers), README.

**Phase 3, the life cycle** (about 1,500 lines; **built**, [docs/INFECTED.md](../docs/INFECTED.md#the-life-cycle)). Succession and splinters. Lone ones. Ageing and the Clash
(the loser walks away). Borders: calling contests, old and reopened borders, Border Nights heard then
found, absorption and merging. Splits, the kept slot and its room, the daughter taking the wood, clans.
Lair moves, abandoned lairs, displacement with its caps, vacuums. Seasonal pull, Winter, burned land
(event and player fires). Starvation into crawlers. Temperaments (and where each one's day party walks),
habits, fear places and getting used to them, marks per character, echoes from who the alpha was, worn
trails, bonds, call patterns, mixed looks. Named alphas' histories in headlines and voice, always with the
place. Check every rule against the signs table: a rule without a sign waits. Re-run the simulator with
the real rules at Season speed 1 and faster and all map sizes before tuning.
Docs: INFECTED, ROADMAP.

**Phase 4, people and content** (about 2,000 lines; **built** but for [what's left](#people) and the
live tissue, [docs/INFECTED.md](../docs/INFECTED.md#people)). Each faction's words, voice and marks, with the place
anchor. Skills. Knowledge levels and the ways to learn them. The 13 contracts (offered only within an hour
of their window; Alpha Sighting renamed The Lone One) and the 9 events (2 reworked). The changed missions
with their fallbacks, and the wanderer, truck, hunter-squad and spawn-point rules. Your dead run with its
caps, the name setting, the opt-out and the name filter, and Bring Them Home with its exclusions. The dark
corners: walk-outs, the fat moat, the Long Table, tags and frequencies. The moat rule on tribes. Questline
hooks (HC1's claim, H3's edge). The cure milestones, the call box with its limits, the final stand's waves,
and the live-tissue stage with its fallbacks, which goes in with the finale ([FINALE.md](FINALE.md)). The
last week: nobody new turns, the run-down on epilogue hours, the last call, crawlers that don't refill,
and the beats handed to the epilogue. The rename table, applied to every New Vegas piece that's built.
Docs: MISSIONS (new and changed rows, with checkmarks only once in code), QUESTLINES (the hooks),
REPUTATION (Halcyon Infamy for a tagged alpha, the night toll, Bailiff's trigger), LORE (the faction
words), WORKBENCH (the tag, call box and ID card items).

**Phase 5, the great alpha and signs** (**built**: [The great one](../docs/INFECTED.md#the-great-one),
[Signs and sounds](../docs/INFECTED.md#signs-and-sounds)), with phase 4's rest but for what [People](#people)
still lists.

## Phase 0 results

Phase 0 ran on its own branch, `worktree-wf_34724fc3-3cf-4`, which isn't merged yet: the land grid and
survey (commit cd9193c), the tribe simulator (ac3c6e6, retuned in f8144b5), the zombie lab, and the held
waypoint (ee79814). It ran with all 16 tribes decisions on their recommended options and with the
author's day parties.

**Built there, not merged:** `Core/RZ_GroupOrders.c` (the held waypoint), `World/RZ_Land.c` (off by
default) with `tools/land_survey/`, `tools/zombie_lab/`, and `tools/tribe_sim.py`.

**Folded into this doc,** because the approved design said phase 0 would set them:

| What | The design's starting value | Phase 0 |
|---|---|---|
| P | 120 m | 250 m in the open, 0 behind terrain; no spawn within 300 m in sight |
| Data speed | 1.5 m/s, walking | 3.5 m/s: Bacon runs every member to an order |
| The scout's pack | about 30 s | 150–200 m out, arriving in 40–55 s |
| Slot divisor | 6 km² a slot | 5 (Arland 0, Everon 5, Cain 4) |
| Steering | go, if Bacon follows waypoints | go; the "re-send every 30 s" fallback isn't needed |
| Visible fights | heard then found, unless a second faction works | heard then found; the second faction wasn't tested |
| Frame time | measure at 60, 100 and 140 | 100 agents is fine; raise the engine's AI limit to 160 |
| The land grid | about 2 minutes; rivers inferred | 4.5–5 minutes the first time; rivers from river entities; bridges from road segments; valleys 500 m across; lairs at forest edges |
| Alpha health | a fallback if there's no setter | the setter works; healing is scripted |
| The silent kill | one path | two: delete, or kill and mark |
| The hide rule | Bacon gives up by itself | scripted, because Bacon's perception never forgets |

**Proposed retunes, for the author when phase 0 merges.** These are numbers the design chose itself, so
this doc keeps the design's values until the author agrees:

| Number | Design | Proposed | Why |
|---|---|---|---|
| Seeds | slots ÷ 3, at most 4 | every slot not kept, at most 4 | Busy servers keep towns from going dark, so too few tribes formed |
| The night's commitment | a third of losses, or two waves | also: no wave goes in that could take the night's losses past half | Worst first night 31–36%, worst first hunt 46–50%, none wiped |
| Weakened tribes | scouts call with no pack | post no scouts | Their scouts died one by one |
| Bastion's Cull | any tribe | tribes of 12 or more | It kept naming tribes already under 8 |
| Hunger a counted night (Decline) | 5 (7) | 3 (5) | Takings are about 0.06 per tribe a night |
| A town gone dark sends a tribe after | 5 season days, 3 km | 3, 2 km | So Chernarus-size fills its slots |
| Infection points per young from the ones who ran | 4 | 6 | At 4 every tribe sat at its cap |
| Split at | 25 | 26 | A clan in half of Everon seasons at 5 players |
| The game clock | the server sets a 4-hour day | the mod sets it (`SetDayDuration(14400)` from a **Day length** setting) and saves the time of day in `state.json` | The default is real time, and the clock resets on a restart, so a night never counts across one |
| Your dead run | on by default | off by default until the player-killed event is measured | Attribution can't work without it |

With these, the simulator gives Everon 4 tribes in 88–100% of seasons, a clan in about half at 5 players,
tribes Hungry about 40–47% of the time, and no tribe wiped. With the design's own numbers on the measured
land, Everon got 2–3 tribes, tribes were Hungry 63–89% of the time, and some were wiped on their first
hunt.

**Not measured yet:** the player-killed event (VERIFY 9), a second hostile infected faction, the sight
trace's cost, facing, replication, corpse cleanup, whether a new order pulls a group off a live target, a
real player (all targets were spawned civilians), darker nights (the moon was 0.89), other zombie mods,
Workshop maps, and perception and spawn order inside a forest.

## Decisions

The 16 tribes decisions, as approved (recommended options, except where noted):

| # | Decision | Approved |
|---|---|---|
| 1 | Where it's written | A new `docs/INFECTED.md`, with short linking paragraphs in DESIGN.md, and the Doc map and README updated; added when phase 1 is built (decision 16) |
| 2 | Player-facing words | Nobody says alpha, tribe or crawler; each faction has its own words, always with the place |
| 3 | Fights between infected | Heard, then found: Border Night losses counted off out of sight, and the Clash's loser walks away alone |
| 4 | Tribe slots | Wild km² ÷ a divisor, at least 1 from 1.5 km², one kept for splits from 3 slots, and a **Slots** override; measured in phase 0, which sets the divisor at 5 |
| 5 | Everon's clan | The kept slot, the room kept round a big tribe, the daughter able to take a weak neighbour's wood, and growth from the ones who ran: a clan in roughly half of seasons |
| 6 | Growth | The ones who ran: a tribe gains young from nearby towns' rise in infection, 2 a season day at most |
| 7 | The night's commitment | A hunt breaks off after losing a third of what it brought or two waves, with the long call, and doesn't come back to that cluster that night; under 8 members a tribe goes quiet and rebuilds |
| 8 | Time acceleration | A game day of about 4 real hours, and a night counts only with 20 minutes of play in it; stated in WORKBENCH_SETUP (phase 0's proposal on setting it is above) |
| 9 | The scout rule | 5 s in sight and in terrain line of sight, checked every second, chasing only the seen player. Changed in review: the sight radius comes from stealth (conflict 46); by day the pack is at most 4 and never the alpha (day parties); the pack's timing is phase 0's |
| 10 | Your dead run in public text | Described by default (*"a man in a blue jacket"*), named on the ID card, in their own journal and to their group; the server can switch to names, and players can opt out |
| 11 | Samples from tribes | Alphas 2 in Prime and 3 in Decline, none while Rising, 3 alphas per tribe a season; guards 1 each under a Prime-or-older alpha, capped per played hour; only with a player within 150 m; town specials scaled down by the tribes' share |
| 12 | Finale tissue | Best first: a Prime-or-older alpha, else a lone one, else two guards, else two Live Capture specials; the same tribe at most once in 6 season days; a great alpha counts more. Now owned by [FINALE.md](FINALE.md), crediting a route's track (conflict 44) |
| 13 | Alpha Sighting | No option was marked recommended. **(b) renamed The Lone One**, because decision 2 keeps "alpha" out of player text and the mission rework had already renamed it. If the author prefers (a), only the title changes |
| 14 | Bastion off-screen | Culls only through contracts and events players see |
| 15 | Data takings with nobody near | A wanderer per tribe and a truck per server every 3 counted nights, on any road a trail crosses, only with no player within 1 km (and only at night) |
| 16 | Optional extras | All kept: bloated crawlers that burst at 5% in towns at 70+ from Overrun; Winter lairs in villages only with the Winter rule set; three knowledge levels with a range ring icon; the bright moon (a reading exists); +2 Halcyon Infamy for killing a tagged Prime alpha |

The author's day-party clarification sits on top of all of them: hunters and young only, a quarter at
most, groups of 2–4, 6 bodies near a player cluster, the alpha and guard at the lair, at most 4 on a
scout's day call, day noise drawing only the day party, hungry tribes about doubling as far as town edges,
nobody out the day after a feed, fog as dusk, and towns by day the crawlers'.

## Risks

1. **Bacon's own perception decides the close fight.** Phase 0 measured it: P is about 250 m in the open,
   and a body spawned in a player's sight within reach charges at once. Flankers, the held-back core and
   the lair must spawn behind terrain or before a player is within 300 m. The worst case is that hunts
   arrive as waves from two sides, which still reads.
2. **Waypoints.** Phase 0 found Bacon groups follow them, idle in place and go back to them after a fight,
   but always at a run, and their perception never forgets a target. So the break-off and the hide rule
   must be scripted, and day parties must hop and hold or they stream through the wild.
3. **Speed.** Everything runs at about 3.7 m/s on an order, so the wild can look like crawlers in
   different clothes, and crawlers can look fast. Fast has to show in the chase and the scout's pack, and
   in how they move: small groups, in file, out of the towns. Crawlers need a slow prefab in the stack.
4. **Runtime groups** are the main path. Named young, bonds, ID cards, per-guard samples and the alpha's
   body all need `AddAIEntityToGroup` on an empty `SCR_AIGroup`. It passed with Bacon 1.8.0; another
   zombie mod may differ, and then those features turn off and the design falls back to group prefabs.
5. **Ecology rates:** player kills (about 20 per player-hour) against takings, the ones who ran and the
   commitment cap. A tribe could still die to one busy squad, or sit fat on a quiet server. Phase 0's
   simulator says the design's own numbers make tribes too hungry and too few on the measured land; its
   retunes are waiting for the author. The status log must show members, hunger and the ones who ran per
   season day on live servers.
6. **Wild land.** Measured: Arland has no tribe, Everon room for 4 at the new divisor, Cain 3. Say so
   plainly in the docs. The **Slots** override exists, but on Arland the tribe seeds in only half the
   seasons and often breaks up.
7. **Fairness of the scout rule.** Line of sight is terrain-only, so trees and walls don't block a
   scout's sight, only buildings do. Mitigations: stealth's sight radius (lying low shortens it), the
   Hunter's warning line, the shriek as a clear tell, no calls in the gentle start, and a pack that chases
   only the seen player.
8. **Lethality at night.** The first wave (8 at most), the per-cluster share, the core held back, targets
   only for the seen, loud or on-trail, the hide rule, the commitment cap and safe no man's land must keep
   a careful solo crossing possible. Play-test solo night crossings on Everon in the phase 2 slice.
9. **Day parties barraging players** (new). The author's rule is that day stays the lighter danger. The
   caps (a quarter at most, a fifth by default, groups of 2–4, 6 bodies near a cluster, at most 4 on a day
   call, never the alpha) and the measured sight keep a day crossing at 0.9–1.4 fights in the simulator.
   Watch real day crossings in the phase 2 slice, and lower **Day party share** if days feel like nights.
10. **The agent budget** when 4 or more clusters are out at night, with hunts passing through towns
    while crawlers are out, and by day with parties, lairs and towns near the same players. Shares shrink
    to 25 or below. Check that a shrunken presence still reads as a tribe, and that 28 per cluster still
    reads as a town in Overrun. The engine's active-AI limit (128 by default) covers faction AI and
    wanderers too.
11. **Stealth and tribes together** (new). Stealth touches at most 40 infected an update; with a lair, a
    day party and crawlers near one player, some wait their turn. A hunt that can't be lost by hiding may
    feel unfair: the hide rule (indoors, 300 m, no man's land) must be clear in play.
12. **Readability.** If the calls, tracks, marks and lines are weak, tribes read as random fast spawns
    with extra steps, and day parties as random roamers. The phase 2 slice puts bodies behind the calls
    from day one. The place anchor ties the four faction names together. Play-test whether a new player
    can name the tribe whose land they're on after one evening.
13. **Research pacing.** Tribe samples, Plant a Tag, Capture the Index, lone ones and abandoned lairs all
    add research. The pacing is built, with the lab's capacity as the hard ceiling. Tribe samples are
    capped, Prime-only, player-credited and budget-neutral against town specials. Watch samples per
    player-hour by source, and the cold store, against Season pacing.
14. **Runaway ecology.** Clans, absorption and great alphas could snowball, or starvation, culls and busy
    players could empty the wild. The caps, the kept slot and its room, weakened tribes rebuilding, death
    by age, vacuums and the once-a-season-day limits need the simulator at Season speed 1 and faster and
    on all map sizes before tuning.
15. **Faction balance.** Bastion culls push tribes onto the Haven (capped at 2 a season per faction). The
    Red Hand's moat delays Bastion's takes (it gives way on the third try). Hungry tribes test the outposts'
    wire (it stops at no man's land plus P), and their day parties come to the town edges. Watch Haven
    morale and Bastion strength across a season.
16. **Data and bodies drifting apart:** a hunt as data walking into a player, a day party spawning in
    sight, a staged fight interrupted, a restart mid-vigil, a capture case dropped or taken. Bodies appear
    out of sight and behind, the write-back keeps positions, survivors are counted back, and the save keeps
    the data right.
17. **Exploits and griefing.** Covered so far: feeding tribes with your own deaths (a 10-minute life, once
    a season day, 3 per tribe); Bring Them Home collusion (exclusions, one a season day); farming alphas
    through succession (Prime only, 3 a season); farming staged fights (no samples) and outpost kills
    (player within 150 m); farming day parties (hunters and young drop no samples); permanent safe corridors
    (fear wears off, and player fires don't block trails); marks as lures (no target switching); the call
    box and Bait the Crew near other players (distance rules); displacement as a weapon (capped); logging
    out (the hunt waits). Each fix needs a runtime test case.
18. **Mission collisions:** a lair move, split or succession under a running mission or questline.
    `Tribe(id)` claims hold the tribe still, and HC1 and Halcyon's finale route claim the site tribe. The
    runtime test must include a succession mid-mission.
19. **Tone, originality and privacy.** Named player dead, walk-outs, the Long Table's offerings and the Red
    Hand's feeding must follow the writing guide. Kill sites are never gore, and nothing is copied from
    films. Player names in public text are off by default, with an opt-out and a name filter.
20. **Scope:** more than any system so far, with 13 contracts, 9 events and a new doc. It must ship in
    phases, each with its docs and each rule with its sign, or the docs will drift and the world will
    change where nobody can see why. Old saves seed tribes on load.
21. **Pacing.** A faster season speed has fewer counted nights a season day, so it shows less of the life
    cycle, and a server that rushes to Aftermath sees few Declines, clans or great alphas. Two regulars
    never reach Aftermath before the cure. Check both, and Arland with **Slots** set, where a single
    restless tribe would carry the whole season.
22. **The last week** (new). Tribes running down on real hours must feel like an ending, not a cleanup:
    check that a busy server can still hunt the last alphas before they lie down, that a quiet one hears
    the last call, and that crawler towns cleared after the cure really stay cleared.
23. **Dependencies on unbuilt work.** The finale (for the live-tissue stage), the epilogue (for the closing
    work), and phase 0's merge. Each has a fallback, and the tissue stage ships with the finale, not
    before.
