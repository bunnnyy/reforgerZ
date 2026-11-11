# The epilogue: what's left to build

**Status: designed, not built.** Approved by the author on 2026-11-11. Phase 7 of the plan is built,
simplified, and lives in [docs/EPILOGUE.md](../docs/EPILOGUE.md): the week's phases on Epilogue hours
and Test scale, the record's snapshot and the hall's freeze, beats checked on every showing and their
marks, the carriers (leaders' season replies, residents, wanderers, the open channels, the reel,
boards, readables, the news, `/season`), the one dose-outlet table on the finale's terms, the queue
and the lab gate with the safe-ground rule, the four stronghold grey choices, the whole closure set
(Last Rounds, Loose Ends, Bring Them Home, Quiet the Ridges, Take Back a Town, the Long Table's last
week, the Walk Out, Settle Accounts, the Last Broadcast, Personal Goals, the Last Entry), the season
block's lines, the names policy, the Unsigned last day and the late-cure push-back. This file keeps
only what isn't built. The rules that hold for all of it (one week, everything closes, nothing carries
over but hall lines, the outside never answers) are in the doc and [PLAN.md](PLAN.md#the-epilogue-week).

## Contents

- [The record and your lines](#the-record-and-your-lines)
- [Beats and marks](#beats-and-marks)
- [The evening calendar](#the-evening-calendar)
- [Places, people and scenes](#places-people-and-scenes)
- [Leaders and the truth](#leaders-and-the-truth)
- [Finishing everything: what's left](#finishing-everything-whats-left)
- [The epilogue deck: last errands](#the-epilogue-deck-last-errands)
- [Edge cases and settings](#edge-cases-and-settings)
- [Writing the epilogue](#writing-the-epilogue)
- [Risks](#risks)

## The record and your lines

- **The fuller record.** Per faction: trucks delivered and lost, towns won and lost, leaders lost (a
  count and the last 3 names); per outpost: its owners this season (3 at most, in order) and commander
  generations; per town: infection and crawlers at the cure; a wanderer death list with how and
  where; the best journal of the dead per identity; each identity's best life per faction
  (reputation's season record, [REPUTATION_NAMES.md](REPUTATION_NAMES.md)). Each counter saves the day
  it started tracking; a beat reading one that started late uses its version without numbers.
- **Your three lines**, in priority order: the route and whether you decided it; a major's decision;
  a truth the deeds store holds about you; a questline ending you decided; holding the final stand; a
  frozen hall record; Champion or Wanted; your rescues; your dead characters. Line 1 at the commit,
  line 2 from the first leader whose faction it touched, line 3 from a wanderer who remembers you, a
  grave or a poster.
- **Titles** from `TitleFor(identity, faction)`: the identity's best life this season with that
  faction, never the current one; a speaker uses its own faction's; the queue, residents and
  wanderers the Haven's; "From <town>" from the highest-salience town log entry. *"Don't know the
  face. Know the name."*
- **Notables**: up to 12 named in the world, scored (decided the route +5, a major +4 each to 8, held
  the stand +3, Champion +3 a faction to 6, Wanted +2, a top-3 record +2, Remembered +2, a questline +1
  each to 3, a crew name or rank +1), with epithets from the biggest deed; each name in at most 3 beats
  (2 on a server with fewer than 6), and at least half the beats name nobody. On a server with fewer
  than 24 identities with deeds, at most half of them are named.
- **Wanted** means Kill on Sight for 4 or more hours played in that life before the cure, with the
  bounty unpaid.

## Beats and marks

- **Weights and slots in full:** the change factor (×1.5 when a place ended up different: a Shelter
  that became a Town, an outpost that changed hands, a town that burned) and the choice factor (×1.25
  for a beat from a choice someone made); one beat per faction (its major's ending, else its season);
  hubs taking up to 3 beats (the lab, the queue, each safehouse) and strongholds 2; roads (the last 4
  lost trucks as wrecks, The Last Convoy's end); stations (the power station: who restored it last,
  which towns have lights); the characters slot (the Kid, the Preacher, leaders who died); since-the-cure
  beats for a house that ran dry. About 85 beats on Everon and 55 on Arland.
- **Places in full:** outposts' coats of paint in text (*"Under the red hand there's a Bastion stencil,
  and under that, in chalk, the Haven's KEEP OUT."*), a lost post's last notice, a commander's grave in
  the faction's way; the safehouses' rescued-in line (*"The nurse brought six of us in from Brod."*),
  the house rule's door notice (`haven.rule`), bunting; towns' marks by the town log's kind (a sheet on
  two posts for a rescue, Bastion's stencil for a shelling, tape for a Quarantine Line, a cross of fence
  palings for a mass grave, the riddle's note for a puzzle); the tribes' other beats as marks (brought
  home, taken alive, killed or burned out, walking with it, the great one); the Long Table's other
  states once exposed (moved, the boy got out, the mother at the lab).
- **Route marks** from 90% (private: the same share of places for every route with an active lock),
  the losing routes' marks taken down at the first dusk of the week, the winner's made final.
- **Mark placement in full:** at least 25 m from a junction, nothing within 3 m, road width from the
  network (VERIFY), never in the warning zone but the north road's; 8 props per player cluster and 30 on
  the server, 3 in any 300 m, the cure's, lab's and queue's home marks first; props up 10 minutes at
  least.

## The evening calendar

The epilogue's own set pieces on the majors' calendar, after Last Rounds: **the Haven's supper
reading** (the chair reads the season's dead at 18:30 on free evenings once the Gate has run; the
names on the house board and a clerk who reads them on request as its standing version), **Bastion's
20:30 watch** after the last call (five minutes on the net, *"Bastion listening. Nothing heard.
Bastion out."*, a stroke on the roster board; radio only, every evening), **second airings** (every
piece again, words only, on the next free evening at least 8 real hours later) and **standing versions**
for every piece, so a lunchtime player still gets them. The calendar works out how many evenings phase
B holds from the time acceleration at the commit.

## Places, people and scenes

- **Scenes that move:** the queue's people shuffling up and new ones joining from the houses; the lab
  gate's group by route (two Haven medics with a cold box, a Bastion group with stencilled crates, a
  beak behind the table); the queue walking to the barrier at each change of shift in processing with
  one escort group; scene clearance (free of vehicles and players within 8 m, sliding 30 m along the
  verge; VERIFY the query); the queue's people talking each in their own line.
- **The Long Table's family as bodies** in the queue (three inside its 6), the mother's grace over a
  bowl she doesn't share, the boy talked out of the queue becoming the Kid or a child at a house.
- **Residents and named people** in full: at least 8 plain lines per seat per ending for the week
  ([TONE.md](TONE.md#312-repeats)); the cook, the sentry and the toll man's first-person lines; seeded
  names that never use LORE's people.
- **Wanderers** preferring beats they've passed and closing work near them; the Preacher reading a
  town's recorded dead where he rests and ringing his bells at dusk; the Courier's last letters; a
  wanderer who remembers you saying your line first.
- **Radio:** the Haven channel's answer to the broadcast (*"Mine's in a car on the north road."*); the
  secure channels' grey truth under the open line for Radio Operators; the reel spread one per season
  day first; the Last Broadcast taking the reel's slot on the hour.
- **The newcomer's scenario,** *The empty kitchen*: a first spawn in the week at a kitchen table with
  the note, pointing to the queue and offering The Key Where It Was; the first leader's three lines.
- **Road posts** keeping their route lines (*"for the record"*; the toll *"one tin, or one dose,
  whichever's cheaper this week"*) and preferring roads that lead to the queue.

## Leaders and the truth

- **Leaders' topics as conversations** when Dialogue > Trees is on (the cure, this place, me, the
  dead, where to look, the grey choice), coloured by traits (a Greedy leader prices it, a Merciful one
  asks if you've eaten), outpost commanders and council heads by loyalty.
- **"What you didn't tell me"**: the deeds store's season scope (what was done, for whom, against whom,
  what the giver hid) lands as up to 6 truth beats, picked by harm; the persuade check gets the truth,
  a Rogue giver lies again; one personal line a player at most; errands' unresolved lie records come
  here on the last day. Needs the deeds store's truths.
- **After the point of no return:** a losing route's people carrying on with dignity (*"We stood on the
  lab road. We lost. The book says we're square."*).

## Finishing everything: what's left

| Item | Still to build |
|---|---|
| Last Rounds | The finale's routes' closing lines carried on the codas (the Haven's channel says her name, Bastion's last call reads the four R numbers, the cook at the lorry with `cook.line_paid`, *"Reference stock: wanted"*, *"WE TRIED."* on the lab gate); "At the barrier" and "On the paper" as epithets in the season block ([MAJORS.md](MAJORS.md)) |
| Loose Ends | Each way's own checks and holds (the Fear check for the Well Story, the hold of the lane, the flight inside the warning zone); half the built ending's pay and its world effect at half through the epilogue's lane; a set piece's 600 m quiet |
| Bring Them Home | More keepsakes: Bastion tags from His Effects' bodies and off a turned one from a military pack, a crewman's lighter, a debtor's last tin, ID cards off a named young laid down by name, researchers' notebooks, a driver's papers from a wreck (The Wreck's Name), a player's character who turned; Sedlak's box at the last roll; the deserter's KIA rubbed out; buying a keepsake back from the fence (the grey way) |
| Quiet the Ridges | The factions' jobs: Bastion's Cull of a contact near a post, the Haven's Bring Them Home for a leader it knew, Halcyon's last reading at a lair at dusk (the paper's appendix), the Red Hand's book on which pack calls last (15 Favor a player); sitting the last call; the moat's last feeding; up to 3 names in the hall line |
| Take Back a Town | Its givers (a resident's *"I'd like to see the kitchen once more"*, Retake the Town for towns with crawlers, Bastion's stencil job, the crews' loot rights); a resident taken home for an hour; the candle only while fewer than 8 towns carry marks; the house board's *CANDLES* count; up to 3 names in the hall line |
| The Long Table | The basket as part of C1's proof; C1's and C2's last stages at the farm and the queue; at the queue, telling the corporal or the chair |
| The Walk Out | The order by the route's terms (the roster, the houses' order, the authors), the Red Hand's place near the front walking you with the next shift, escorts of the queue up the road with One Last Toll on a Red Hand road, the safe ground on the road in processing, Bastion's CO and the boss staying behind, the fallback with no checkpoint (the land edge nearest the Haven's stronghold house) and off on an island map |
| Settle Accounts | Borrowed kit handed back to its buyer's quartermaster, a leader at −2 made right, promises kept or owned up to, borrowed food carried back; Come In ([REPUTATION_NAMES.md](REPUTATION_NAMES.md#14-the-last-week-settling-accounts)) |
| The Last Broadcast | The factions lobbying face to face; the lad selling airtime; bringing a name, a page or a reel recording as the say's price; the radio ham writing it down if the finale found him; the decide hold on the tally |
| Personal Goals | The backgrounds not built yet (Soldier, Doctor, Raider, Elder, Drifter) and the built ones' full goals (the Nurse sitting with a bitten resident, the Hunter's cabin and the Ex-Con's stash in their goal's town, the Police Officer's truth taken to the person it's about) |
| The Last Entry | The Haven's clerk reading a few at supper in processing |

**The dose outlets' rest:** the census taking your name and background, Day 2 slips served first and
free under the column, the search below Known, the cook's sister's house under the first pan, "her
house" as the woman's, errand card 41 (What the Cure Costs Here), the Independent's open crate going
to whoever reaches it first with names chalked on the lid.

## The epilogue deck: last errands

Errands' epilogue deck, dealt by errands' dealer ([ERRANDS.md](ERRANDS.md#after-the-cure-the-epilogue-week)).
Each sets `m_bAfterCure`, is offered only when its beat was picked (`NeedFlag("epilogue.beat.<key>")`)
or its condition holds, pays through errands' small-pay ledger once per identity per beat, isn't
limited by the season's two cards a day, and is picked per player (up to 2 at once, within 3 km).
Bring a Cup and Read Them Out are built as the week's places' choices ([docs/EPILOGUE.md](../docs/EPILOGUE.md#places-and-scenes)).

| Card | Giver | Built on | Offered when | The grey way |
|---|---|---|---|---|
| **Carry the Dose** | the route's people | a sealed cold box to a named holdout in an abandoned town | the route has outlets and a holdout is named | open it: your dose (if bitten), none for the holdout, 10 Infamy |
| **Pack for the Queue** | a resident (Haven Known) | two things from their old house in the town they were rescued from | the house has rescued-in people | bring one and say you couldn't find the other |
| **The Key Where It Was** | the kitchen note | find the key, give it to Petr in the queue | the note was read | use it yourself |
| **Borrowed** | a fed house's head | 3 food back from the house that "borrowed" it | the season had borrowing | say you couldn't find them |
| **Take a Name to the Board** | a resident or the Preacher | a dead person's name to the stand board, a house board or the Names board | a board exists and the record has dead nobody has written up | — |
| **The Toll Man's Last Debt** | a Red Hand toll man | 3 food to the Haven house the crew took from | the Red Hand's route, or the book balanced | pocket it: 5 Haven Infamy |
| **Sign Here** | Bastion's corporal | three names from the queue | the roster or census terms; Bastion Known | forge them |
| **Copies for the Far House** | Halcyon | an unarmed delivery of the method to the farthest house | Published | sell the copies to the fence |
| **His Effects, epilogue variants** (The Tags; The Wreck's Name) | the faction that lost the man or truck | the tags or papers to its nearest post | a lost man or truck | the fence buys the photo frame |
| **Clear the Way** | the route's faction, or the Haven | loose ones on the road to the queue | infected within 1 km of the queue road | — |
| **The Last Watch** | an outpost commander (Known) | hold the gate 10 minutes at the evening change of watch | the outpost held all season, or lost its commander | leave early |
| **Last Letters** | the Courier | an unarmed delivery from the houses to the queue | the queue exists | — |
| **One Last Toll** | a crew boss | a toll on the queue road, or talk the crew out of it | the Red Hand's route, or the book balanced | the toll |
| **Into the Record** | Halcyon | a dead researcher's notebook to the paper's table or the lab | a researcher or courier died | — |
| **Somebody Should Tell Them** | a resident | their relative runs with a pack: ends in Bring Them Home, or a grave | packs with names walking with them | — |
| **The Boy** | a council head | escort the Kid to the queue or a house | the boy got out | — |
| **Processing** (errands) | a pinned local | walk them to the queue, never to the barrier | a checkpoint within 1.5 km | talk them into supper |
| **What the Cure Costs Here** (card 41) | a sick local | the outlet table decides the price | the terms keep this local from a dose | errands' card |
| **Come In** (errands' amends) | a faction that hunts you | walk unarmed to its outlet or gate and wait | Watched or Hunted, before processing | — |

## Edge cases and settings

- **No stand in the last hours** (Season, off): for admins with fixed restart days, the final stand is
  held back in the last hours instead of the wipe being pushed.
- **Leaders' off-screen deaths** at a quarter of the usual rate while the world settles; contracts that
  take or raid an outpost not offered.
- **The Unsigned last day in full:** the broadcast's long-stall line in place of the warnings, the
  majors' no-cure codas.
- **Graves** for notable deaths before this shipped, from the journals of the dead; a death in the week
  replacing a less notable grave.
- **Settings still to add:** Season marks and their days, mark props per cluster and on the server,
  scenes with bodies (2 lasting, 1 evening). **Optional prefabs** still to add: sheet on posts, stencil
  board, price board, notice on a post, grave marker, wanted poster, trestle table, crate, bunting,
  ribbon, truck wreck, tape between posts, handcart, names board (one board prefab stands in for them).
- **The journal's tab:** "As the Zone tells it", "Unfinished" and "And you" in the majors' "The season"
  tab of the notebook (today a notice), whose whole list opens in processing.
- **`last_season.json`:** the Last Broadcast's version and the 10 strongest beats as plain lines.

## Writing the epilogue

[TONE.md](TONE.md) owns how it sounds (its 3.5 is the epilogue's phrasing). The rules this design adds:
one object, one name, one day, one mistake; every route has its grey side and its dark joke, from a
rival, aimed at institutions, never at a named player's death; a player named for something they did,
once per beat; the outside never answers and never changes its words; a rushed or bitter ending said
plainly; a walk-out is a goodbye, never a joke; no pronouns for seeded names; the Long Table never
described. **Still to write** (about 600 of the design's 885 lines): the route-neutral beats' echoes
for every beat type, outpost commanders by loyalty, door notices and readable clauses by route, the
deck's 19 cards (brief, success, failure, grey way), the lobbying lines, the standing versions.

## Risks

- **A week is long:** measure in play-tests how much closing work gets done, and when.
- **The Walk Out** changes the Perimeter and the lore's "the gates open with the wipe": keep it to the
  north road, in processing, outward, one name at a time; VERIFY the retirement.
- **The outlets at the very end:** check bite rates after the cure's −70%, and that no outlet
  trivialises the last days (the free crate, by need, the book first, the Red Hand's free processing).
- **Contradiction:** test with a restart mid-week, a leader killed by a player, an outpost taken while
  settling is off, and a Long Table exposed during the week.
- **Props and save size:** measure the epilogue pool beside the tribes' signs, and `state.json`'s
  growth over the week.
- **Names in public places:** the Last Entry is the most exposed; admins need to know `/epilogue
  strike`.
- **Small servers:** a few names could fill everything until notables and their caps are built.
