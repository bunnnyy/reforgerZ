# Infected: tribes and crawlers, what's left

**Status: designed, not built.** The tribes and crawlers are built, phases 1 to 5 and their rest
([docs/INFECTED.md](../docs/INFECTED.md) owns them: crawlers, the land, bodies and the budget, the
packs' daily life and hunts, the life cycle, the people's side, the great one, signs and sounds, Lead
Them Away's pack on the move, Stock the Pit's cage). This file keeps only the small rules still to
build. Approved by the author on 2026-11-11. The finale's live tissue's rest is
[FINALE.md](FINALE.md)'s; the epilogue reading the last week's beats is [EPILOGUE.md](EPILOGUE.md)'s.

**Player text never says "alpha", "tribe" or "crawler"** ([WRITING.md](../docs/WRITING.md#writing-the-infected)).

## Contents

- [The model's rest](#the-models-rest)
- [Tribes](#tribes)
- [Day parties](#day-parties)
- [Bodies](#bodies)
- [People](#people)
- [The behaviour layer's rest](#the-behaviour-layers-rest)
- [After the cure: the last week](#after-the-cure-the-last-week)
- [From Grounds to tribes: the rename table](#from-grounds-to-tribes-the-rename-table)
- [Phase 0 results](#phase-0-results)
- [Decisions](#decisions)

## The model's rest

| Rule | Still designed |
|---|---|
| The taken (rule 4) | A leader's off-screen bite, or a Bastion patrol, counted as the taken |
| Trails (rule 9) | Trail scoring by prey (wear is built) |
| Borders (rules 10–18) | Bastion's *"Papa-four: no contact"*; the border's offset moving the ranges |
| Fed by people (rule 19) | The moat rule stopping for a pack whose leader a finale route took alive, until its succession (with the finale) |
| Memory (rules 20–23) | An abandoned lair's finds as items from the search table (a line today) |
| Clans | The relay players hear pass along the ridges; all of a clan hunting toward one ground on frenzy nights |
| The great one | Caught alive it counts +3 on a finale route's track instead of +2 (with the finale) |
| Next season | After a Halcyon win, seeds could start in Prime too (an option) |

## Tribes

- **The daily rhythm and the call:** the leader's echo (who it was) shifting the times; the stragglers
  after a bad night (06:00–07:30); the Hunter's reading of the nearest call; two callers when restless;
  *"a short-handed call after a quiet day"*; relayed calls ridge to ridge.
- **The hunt:** a cunning pack putting a group on your back trail.
- **Who they are:** the factions' voices for a leader's history (the Haven's *"The Quarry lot are Josef
  Hruby's now. He drove the school bus."*, Bastion's contact log, the Red Hand's new boss, a Haven
  elder's warning on the road); where it lies up by its look (dust-grey under cover, outdoor with the
  widest range and highest posts); the Radio Operator and the Hunter naming a pack from its call pattern;
  hunting grounds following the prey; breaking a habit by taking its prey away (rerouting trucks,
  stopping the walks).

## Day parties

A Furious pack's day parties walking the capture road.

## Bodies

- Scouts moving to posts above tonight's trails at dusk, and sitting lower in fog; the building query
  (a player inside is never seen; the code uses its sight trace within 80 m).
- The corpse kept from the engine's cleanup (VERIFY 15), and the samples kept from the loose-item
  cleanup during the vigil.
- A second infected faction hostile to the first, so fights could be seen (behind a setting).
- Loose ones as starving walkers; members crossing between ranges at night; the capture case's pack
  following it.

## People

- **The jobs' rest:** Clear the Trail on a truck road; Burn the Lair's splinter when 4 or more died;
  Bait the Crew's Zombie Bait Infamy when either side lands on a faction's people; Track Them Home's
  "not closer than 80 m"; couriers and columns needing an escort.
- **Trucks taken** on any road a trail crosses (decision 15).

## The behaviour layer's rest

The layer is built ([docs/INFECTED.md](../docs/INFECTED.md#how-they-move-and-hunt)), with the zombie
lab's B1–B20 results in [docs/WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#zombie-lab-bacons-real-ai-measured).
What's left waits on a test; the levers the lab disproved are in the roadmap's Dropped list.

| Part | Still designed | Why it waits |
|---|---|---|
| Sight | Calibrate **Notice rate** to phase 0's ladder in a play-test and re-run `tools/tribe_sim.py` with it | Needs a play-test |
| Crawlers | A forced prone pose (lever L6, `ForceStance`: a prone `SetStanceChange` didn't hold, B12); crawlers steered into a fight within 12 m | Untested lever: needs a lab check on a client first |
| Pathing | Lever L23: a straight-path cost per kind (×1 young, ×3 hunters) in `SCR_CreateAIExtraMoveParams`, so a chase curves | The hook loads and fires (B2); its effect needs a client check |
| Danger events | Lever L16: a scripted `AIDangerEventWeaponFire` to pull engaged bodies the way a shot does | Untested lever: needs a client check, and hearing's leads cover it so far |

## After the cure: the last week

The last week is built ([docs/INFECTED.md](../docs/INFECTED.md#the-last-week)) with its beats kept for
the epilogue, Walking with it among them. Still designed: the epilogue reading them. The closing work
that touches the infected (Quiet the Ridges, Bring Them Home as closing work, Take Back a Town) is
[EPILOGUE.md](EPILOGUE.md)'s.

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

## Phase 0 results

Phase 0 is merged and its measurements are in [docs/INFECTED.md](../docs/INFECTED.md) and
[WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#zombie-lab-bacons-real-ai-measured). Three of its
proposed retunes still wait for the author (`python3 tools/tribe_sim.py --proposed`): seeds on every
unkept slot (at most 4), the ones who ran at 6 infection points a young, and weakened packs posting no
scouts.

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
