# Openings: how every life starts

**Designed, not built.** Approved by the author on 2026-11-12. When it's built, its content moves into
[DESIGN.md](../docs/DESIGN.md) ("Backgrounds and personal goals", "New-player guidance"),
[DIALOGUE.md](../docs/DIALOGUE.md) (the conversations), [UI.md](../docs/UI.md) (the spawn screen and the
tracker) and [WRITING.md](../docs/WRITING.md) (the openings' voice), and this file is deleted.

## The idea

Every life starts somewhere random that suits who you were, in a small scene with a reason to come out
*today*, and within ten seconds someone is talking to you and there is one obvious thing to do. That
first thing is the first step of a short chain (three jobs, about 15 minutes) that teaches the Zone's
systems one at a time and tells its story as it goes, then hands you to the Haven or a faction with
the tracker already pointing at the next job.

Nobody is new. Everyone has been trapped since the seal: the opening is about why *you* step out now.

## What it replaces

| Today | With openings |
|---|---|
| The spawn screen's **Where you start** picker | Gone. The place is rolled ([Where](#where-you-start)); the screen says only the kind of place ("A cellar in a farming village") |
| **Spawn scenarios** (six clue scenes: the ditch car, the basement door, the church flyer, the farmhouse radio, the attic raider, the dead soldier) | The openings' scenes. The six clues come back as props and lines inside them |
| **A wanderer finds you** (2–7 min) | The first contact, who is already there |
| The first-life goal **Find the Haven** | The chain's last step (most chains end at a safehouse) |
| Kept as they are | The gentle start (half density for 15 min), the wind-up radio, the contextual tips, the guidance levels |

## Where you start

- **Random, always.** Each life rolls a place, never one the player can pick.
- **Fits the background.** Each background has place kinds (table below), found by prefab name and
  the settlement scanner: a clinic or pharmacy, a garage, a farm, a police post, a church, a radio mast,
  a school, a shop.
- **Safe enough to stand up in.** Only in a town nobody holds (not within 400 m of a stronghold,
  outpost or safehouse, not at a checkpoint or the line), with no player within 300 m and no live
  mission within 500 m. Infection under 60 for a brand-new player.
- **Not twice in a row.** Not within 2 km of the player's last two starts.
- **Fallback.** No fitting place within the rules: any unheld town's house, with the background's
  scene props; then a vanilla spawn point with only the contact (no scene).
- **No chosen starts.** Never a safehouse or any other place the player picks (the author's call,
  2026-11-12).

## Why today: the reasons

The frame is almost always **out of the hiding place**; the other two are variety.

| Reason | Weight | The scene | The contact |
|---|---|---|---|
| **Out of the hiding place** | 70% | You've been holed up since the gates closed (a cellar, an attic, a back room, a locked shop). Your last water bottle is empty on the floor; a calendar with the days crossed off since the seal. You open the door | Usually **someone from before** (below); sometimes a stranger who heard the door |
| **Your group is gone** | 18% | Bedrolls for four, one of them yours, a map with a circled town, a note: "Back by dark." Three days of dark | Someone who saw them go, or the one who came back |
| **The fever broke** | 12% | A barricaded room, a bucket, a bite on your arm that's only a scar now. On the door: "If you're reading this, you're you. Come find us." | Whoever wrote it, waiting outside |

**Who the contact is.** In about three of four openings it is **someone from before**: a person who knew
you before the outbreak, through your background. They don't say "you're new"; they say "Christ,
you're alive."

| Background | Someone from before |
|---|---|
| Nurse | a patient from the clinic, or their mother |
| Mechanic | a regular from the garage, whose van you fixed |
| Hunter | the farmer whose land you hunted |
| Police Officer | a shopkeeper on your beat, or someone you once arrested |
| Ex-Con | a cellmate, or the guard who was decent to you |
| Lab Technician | a junior colleague from the institute (Halcyon's people know your name) |
| Radio Operator | a ham who talked to you every night on the band |
| Unemployed | the neighbour who lent you money |
| Soldier | a private from your unit who didn't make it back to the line |
| Doctor | the nurse from your ward |
| Raider | someone you robbed, who wants something from you more than revenge |
| Elder | a family from your street; their child calls you by name |
| Drifter | a face from every town; everyone half-knows you |

Later lives can bring back **someone you met in an earlier life** (a wanderer or occupant who
remembers your old name, [memory](../docs/DIALOGUE.md)): "You look like someone I knew."

## The chain: three steps

Every opening's chain has the same shape; each step teaches one system and carries one piece of the
story. The steps are errand steps ([ERRANDS.md](../docs/ERRANDS.md#ways-and-steps): Give, Do, Talk,
places within 1.5 km), so the tracker, the journal and the talk screen already show them.

| Step | Teaches | Example (Nurse, hiding place, a patient's mother) |
|---|---|---|
| **1. Close by** (within 300 m) | Moving quietly, searching a house, the noise meter, the infected | "My boy's burning. There's a pharmacy two streets over. They'll hear you if you run." Search the pharmacy |
| **2. A system** | Your background's thing: treating, water, a car, the radio, a lock, samples | Treat the boy with what you found (the Nurse's quick treat), or boil water if the pharmacy was empty |
| **3. Somewhere that matters** | Walking out, the map, a faction or the Haven, the tracker | Walk them both to the safehouse in the next town. The cook knows the mother: Haven Fame, and the house's jobs show up |

**The story, told on the way.** Each step's lines carry one beat, in the contact's voice and never as
a speech ([WRITING.md](../docs/WRITING.md)):

| Beat | Told as |
|---|---|
| The seal | "Eleven days. They closed the roads in two." (The day count is the season's real day) |
| The broadcast | The contact's radio, or your own, playing "Aid is on its way" while they snort |
| The Perimeter | "My brother tried the river road. The loudspeaker said his name. Then it didn't." |
| Nobody's coming | "Nobody's coming. Say it out loud once, it helps." |
| The factions | The last step's place: who holds it, in a sentence ("The soldiers count you at the door. Let them.") |
| The cure | Halcyon's flyer in the pharmacy, or the contact: "The lab people say there's a way out. A cure. Proof." |

**Where the chain ends** fits who you were (the author's call, 2026-11-12):

| Who | Backgrounds | Ends at |
|---|---|---|
| Medical people | Nurse, Doctor, Lab Technician | Halcyon's field station or the lab |
| Trouble | Ex-Con, Raider | a Red Hand camp (the contact may not come in) |
| Military and the law | Soldier, Police Officer | a Bastion outpost |
| Everyone else | Mechanic, Hunter, Radio Operator, Unemployed, Elder, Drifter | a Haven safehouse |

## Per background: places and the chain's step 2

| Background | Place kinds | Step 2 (the system) |
|---|---|---|
| Nurse | clinic, pharmacy, a house with a sickbed | treat someone (treatments, infection) |
| Mechanic | garage, farm shed, petrol station | get a car to start, or the hand pump (cars, fuel) |
| Hunter | farm, forest cabin, hunting stand | track and avoid a pack, quietly (stealth, tribes) |
| Police Officer | police post, town hall | open a locked door with the station's keys (locks, keys) |
| Ex-Con | the prison bus, a lock-up | get through a Red Hand toll without a fight (standing, disguise) |
| Lab Technician | school lab, clinic, a vet's | take a sample from a dead special (samples, the cure) |
| Radio Operator | radio mast, a house with an aerial | find the open channel and answer (radio, channels) |
| Unemployed | any house, a shop | trade with a wanderer (Favor, trading) |
| Soldier | a lost post, a wrecked truck | hold the gate for a minute (fights, the infected's heat) |
| Doctor | clinic | stabilise someone bitten (antidotes, the clock) |
| Raider | a stash house | collect a debt (Infamy, the Red Hand's ways) |
| Elder | a house on a square | settle an argument between two families (talk, checks) |
| Drifter | anywhere | three people, three small asks (a sample of everything) |

## The first ten seconds

1. The screen fades in on the scene, already close: the door you're opening, the bedrolls, the bucket.
2. The contact speaks first, New Vegas style, on the talk screen's compact box: one or two lines and
   three replies at most ("I'm alive. What do you need?", "Who else made it?", "Not now.").
3. Any reply but "Not now" gives step 1: the tracker shows it with an arrow and distance, the map shows
   the step's place, the journal has it. "Not now" still tracks it, and the contact asks again once.
4. If the contact dies (the infected, a stray shot), the chain ends with a line from whoever finds you
   and the tracker points to the nearest safehouse. Nothing else depends on them living.

## By experience (the guidance levels)

| Level | Opening |
|---|---|
| 0, brand new | The full opening and all three steps; brand-new players lean to the hiding place; the ending still follows the background |
| 1, learning | The full opening; step 1 may be shorter |
| 2, familiar | The scene and the contact; one step, then the tracker picks the nearest job |
| 3, veteran | The scene as a one-line read and a tracked nearby job; a contact only one time in three |

## Pay and limits

- Small: about 10 Fame with the ending's faction over the chain, and the items the steps need. Never
  guns, ammo or loot beyond what a step uses. Nothing for free ([Design guardrails](../CLAUDE.md)).
- One opening per life. Not after the cure's epilogue starts; then the epilogue's own start applies
  ([EPILOGUE.md](../docs/EPILOGUE.md)).
- Scenes are a few props near players only; removed after 20 minutes once nobody is near (as today).
- Works on any map: places by prefab name and the settlement scanner, never coordinates.

## What it needs

| Part | Where | New or reused |
|---|---|---|
| The place roll | spawn logic (`RZ_SpawnMenu`, the spawn point mover), `World/` | new; the spawn screen drops its picker |
| The scenes | compositions in `tools/camp_kit/pieces.py` (three per reason), plus per-background props | new compositions, built as the camp pieces are |
| The contact | a body near the scene, a talk-screen speaker | reuses wanderers' bodies and the talk screen |
| The chains | three-step errands with their own cards, one set per background (13 × reasons) | reuses the errand engine and its steps |
| The lines | conversations in `dialogue/openings/*.rzd` | new; voice per [WRITING.md](../docs/WRITING.md) |
| The tracker | picks the chain's current step by itself | reuses the HUD tracker |
| Telemetry | per opening: started, each step, ended where, minutes | new log lines |

## Build order

1. The place roll and removing the start picker (with the fallback), the hiding-place reason only,
   one contact type (someone from before), and one chain shape for all backgrounds.
2. Per-background steps 2 and endings, and the someone-from-before people.
3. Group gone and fever broke, the earlier-life contact, and the experience levels.
4. Lines for every background × reason, and the voice pass.

## Decided

- No chosen starts, ever (2026-11-12).
- The chain ends where the background belongs (the table above, 2026-11-12).
- About 15 minutes for the chain, to be checked in play (2026-11-12).
