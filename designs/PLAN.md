# The plan: one season, end to end

**Status: designed, not built.** Approved by the author on 2026-11-11.

This file is the one combined plan. It joins the New Vegas plan (the seven pieces) and the finale's plan (the
four majors, the finale, the infected), and fixes them where the author's decisions and the 50 conflicts say
so. It owns the order of the work, who owns what, and the rules that cross designs. Each system's detail
lives in its own design file, linked below.

## What changed in review

- **Two plans became one.** The New Vegas plan's overview, shared engine, decisions and risks, and the finale
  plan's conflicts, build order and summary are merged here. Where they disagreed, the later finale plan and
  the author's decisions win.
- **All sixteen decisions are applied as the author chose them** ([the table](#the-decisions-as-decided)).
  Three changed from the recommendation:
  - **Decision 10:** the majors' fallbacks are dynamic. They fire from what's actually happening on the
    server, never on fixed days. The fixed "season days 13, 22 and 35" are gone ([MAJORS.md](MAJORS.md#dynamic-fallbacks)).
  - **Decision 12:** the epilogue lasts one week, 168 hours, and gets closing work that lets every story
    finish inside it ([The epilogue week](#the-epilogue-week)).
  - **Decision 14:** a stranger finds you only where one would make sense. The interruption budget is only
    a ceiling, never a timer ([ERRANDS.md](ERRANDS.md#where-a-stranger-can-find-you)).
- **Day parties are in**, as the author clarified them ([INFECTED.md](INFECTED.md#day-parties)). They replace
  conflict 45's first proposal (parties of 3–5 with a guard).
- **Grounds is gone.** The approved tribes and crawlers design replaces it everywhere: in the season, the owner
  table, the shared engine and the build order. The names follow the rename table in INFECTED.md (conflict 43).
- **The build order is updated.** The 45-day pacing and stealth are built and merged. Step 0 now lists the
  built bug fixes being done now, the tribes' phase 0 (done on its branch, not merged), and the questline test.
  The finale's revision round is done: [FINALE.md](FINALE.md) is the revised design.
- **The 50 conflicts are listed with where each was applied** ([the table](#the-50-conflicts)).
- **Removed as outdated:** the earlier finale draft's shape (any side, switching, a route Fame bonus, a
  most-Fame winner), the 48- and 72-hour epilogue, the 10-day season's column, "Proof" as a route name,
  "lore to confirm" marks, tone's 3.8 table of names, and every open option that's now decided.
- **One new edge, found while writing this plan:** the talk screen being built now (ROADMAP "Talking to
  people") and dialogue's engine are one engine. See [Built alongside](#built-alongside-the-work-in-roadmap-next).

## Contents

- [Built today, and carrying the rest](#built-today-and-carrying-the-rest)
- [The shape](#the-shape)
- [One season, end to end](#one-season-end-to-end)
- [The epilogue week](#the-epilogue-week)
- [One owner per thing](#one-owner-per-thing)
- [The finale's edges](#the-finales-edges)
- [Where the pieces disagreed, and how it's settled](#where-the-pieces-disagreed-and-how-its-settled)
- [With the four majors](#with-the-four-majors)
- [With the infected](#with-the-infected)
- [The shared engine](#the-shared-engine)
- [Shared, per player, late joiners, any map](#shared-per-player-late-joiners-any-map)
- [Built alongside: the work in ROADMAP "Next"](#built-alongside-the-work-in-roadmap-next)
- [The build order](#the-build-order)
- [The decisions as decided](#the-decisions-as-decided)
- [The 50 conflicts](#the-50-conflicts)
- [Risks](#risks)
- [Docs](#docs)

---

## Built today, and carrying the rest

Checked against the code on 2026-11-11 (branch `claude/sleepy-volta-nwm433`).

| What | Where |
|---|---|
| The 45-day season: research paced by how much the server really plays, the lab's 3% a real day, the cold store | `Cure/RZ_CureResearch.c`, commit b78615a ([DESIGN.md](../docs/DESIGN.md), "Season pacing") |
| The 90% gate: research stops at 90% while a finale is registered (`IsAtFinaleGate`), and a built fallback finishes the cure about 17 days later if nobody else does | `Cure/RZ_CureResearch.c` |
| The 15-minute final stand at the lab, and Take the Lab (`RZ_Mission_TakeTheLab`, on `RZ_Mission_Hold`) | Missions, the cure |
| Stealth: how visible you are, the infected's senses, and how they lose you; nothing is lowered within 1.5 km of the lab during the stand | `Zombies/RZ_Stealth.c`, commit 6110ee3 |
| The questline engine and its 15 questlines, world facts, flags, canon and claims | `Missions/Questlines/`, `RZ_WorldFacts` ([QUESTLINES.md](../docs/QUESTLINES.md)) |
| Per-player AI hostility (`SURV_H1`–`SURV_H15`), leaders with traits, a simple uniform disguise, safe zones, Live Capture | Factions, leaders |
| The new lore: Bastion's roster ("nobody is coming; they're all they have"), the Red Hand's book (Rudi Kasper, killed by the boss), the Haven's bear and Halcyon's "beaks", and the emblems | [LORE.md](../docs/LORE.md), commits 4262a23, 8a2501f, 8ace533 |
| Today's epilogue: the line stands down, research stops, the gates open with the wipe. Its default is 48 hours (`Core/RZ_Seasons.c`, the **Epilogue** attribute) | `Core/RZ_Seasons.c` |
| The build-review fixes (shops, people, tablets, the server-side loadout lock) and the retaliation watch fix | commits cfbcff4, 00ff654 |

**One honest caveat.** The last runtime test started none of the 15 questlines, because their start conditions
weren't forced (`/root/rz_tools/runtime_pacing.txt`). Convoy Escort, Hijack the Convoy, Mobile Lab and the
Locked Bunker didn't start either. The majors and the finale are built on that engine and reuse those
missions, so that test comes first (step 0.4).

---

## The shape

**The season's spine:**
- **The infected** decide where the danger is, and why: crawlers in the towns, tribes in the wild, day parties
  by day ([INFECTED.md](INFECTED.md)). Designed.
- **The four majors** give each faction one season-long story, with climaxes on cure milestones and fallbacks
  that read the live server ([MAJORS.md](MAJORS.md)). Designed.
- **The cure** runs to the 90% gate. Built, as a hook.
- **The finale** decides who controls the cure: five routes, a lock, a race, and a fight for the clean room
  ([FINALE.md](FINALE.md)). Designed.
- **The epilogue week** closes the season: 168 hours in which every story can reach an ending, then the wipe
  ([EPILOGUE.md](EPILOGUE.md)). Today's 48-hour epilogue is built; the week is designed.

**The seven New Vegas pieces sit on that spine in four layers:**

| Layer | Pieces | What a player gets |
|---|---|---|
| Who you are | Reputation ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)), disguise ([DISGUISE.md](DISGUISE.md)) | A name with each faction ("Merciful Thug"), deeds people quote back to you, and borrowed kit that buys a moment, never a living |
| What you do | Missions ([MISSION_STYLE.md](MISSION_STYLE.md)), errands ([ERRANDS.md](ERRANDS.md)), dialogue ([DIALOGUE.md](DIALOGUE.md)) | Several ways through every job; small jobs that come out of what's true right now; conversations with checks, lies and memory |
| How it sounds | Tone ([TONE.md](TONE.md), becomes `docs/WRITING.md`) | One standard for grey choices and dark humour, a season mood, one voice bank and one lint |
| What the Zone remembers | World memory (shared by missions, errands and the epilogue), the epilogue ([EPILOGUE.md](EPILOGUE.md)) | Callbacks all season, and an ending told by places and people, not slides |

**Dialogue is the delivery layer for almost everything.** All of these are conversations in one engine:
- mission scenes and errand asks;
- reputation's parley and the Haven's knock;
- disguise claims;
- the finale's commit;
- the epilogue's leader topics and the closing work's scenes.

Nobody builds a second talk system. The talk screen being built now is that engine's first layer
([Built alongside](#built-alongside-the-work-in-roadmap-next)).

---

## One season, end to end

At the design point: 5 regulars on the built 45-day pacing, Season speed 1. **No day in this table is fixed.**
The cure's milestones come from research, the stages from hours played, and the majors' defaults from live
signals (decision 10). On 2 regulars the gate comes about day 45 (later if research stalls); on 15, about day
35; on 20, about day 34. [MAJORS.md](MAJORS.md#three-seasons) walks three seasons through.

| When | The infected | The majors | The finale | Everything else |
|---|---|---|---|---|
| **Early Outbreak** (days 1–5) | Crawlers in the towns, lying up by day. The seed tribes (10–14 each) in the hills; by day a few walk the woods in twos and threes; one or two voices at dusk | The Haven's family at the first dusk (Every Name, Act I). Halcyon's Day One at the first change of shift (Proof, Act I) | — | Errands about family and doors. The town log and season marks start teaching the vocabulary (a sheet on posts, candles, chalk). First contact in dialogue. Season mood: Early |
| **Collapse** (from about day 6) | Dying towns feed the tribes; new tribes form, at most one a season day. Bastion's pullback loses posts, and one becomes a military tribe. Day parties grow with their tribes | Bastion's "reply received" after a nightly call. The Red Hand's send-off on Collapse's second evening (about day 7). The Haven's summons, then the first sitting (about day 9). The torn page; Bastion's Acts II–III; Halcyon's Three Weeks | — | Story jobs with truths and lies. Chores at holdings. Disguise looks at barriers. Threats, the parley, settling up. Tribes post reasons ("the Quarry lot are hungry", each faction in its own words). Strangers at roads, approaches, fires and town edges, never in empty country. The mood follows the stages |
| **Overrun** (from about day 16) | The caps fill. Border Nights at the bridges. The first split (a clan in about half of Everon seasons, days 18–30). Hungry tribes' day parties come to the fields at town edges, never into a town or no man's land | The hungry week (Every Name, Act IV). Bastion's count (Effective Strength). The Red Hand's gap filled by its smaller stories (between beats) | — | Margin cards around the majors' set pieces. Mission outcomes start adding **route assets** |
| **50%** (about day 24) | — | **Settling day** (the Red Hand's climax) | — | — |
| **60%** (about day 28) | — | **The muster** (Bastion's climax). Bastion's count is squeezed a day or so before it if a post is still untouched | — | — |
| **Aftermath** (about day 31) or 70% | Fewer tribes, bigger and older. Bastion's culls stop (presence 0.1) | **The second sitting** (the Haven's climax), never before the muster has resolved | — | — |
| **75%** (about day 33) | — | **The reading** (Halcyon's climax) | Route topics answer *"Ask me again when it's nearly done"* | — |
| **80%** (about day 35) | — | **The last page** (the Red Hand). Halcyon's "Before the stand". The Haven's "A Week On" when a house goes short | Pitches on demand from 80%, and once on air at 85% | Hand to Hand promises for the Independent. The Red Hand's book on the race never names a favourite. **Nothing locks** |
| **90%: the gate** (about day 39) | Every route's Act III wants live tissue: an alpha, a lone one, two guards, or two specials | Hard stop: every climax is settled by now; anything still open resolves at half effect | Research stops. Five routes open. Players lock at a leader (the Independent on the radio), on a card that states the route's worst truth | The lock card repeats the route's admission. Route marks on the verge (private: equal shares). Looks −1 with the other routes. Parley and settling close with routes fighting yours. Errands that aren't route work carry on |
| **The last 10%** (gate + 4–5 days uncontested, 6–9 contested) | Each alpha caught means a succession and a Furious tribe on the lab road | The majors' forces get ready for the lab | Route tracks, doors, Last Mile contracts. The first finish makes the batch: 12 real hours and 2 hours of play | Route work only for your own route. Face-to-face route lines (cool, fading) |
| **The stand** (about days 44–49) | The waves from towns within 3 km and the nearest tribe's hunters, kept at 70–130% of today's | At most two of the majors' groups, the finisher's faction first, plus Halcyon's guard | **The fight for the clean room**: 15 minutes, at most 3 of overtime. The side holding it controls the cure, then chooses the terms. Take the Lab (the Red Hand route, once) and the Coat Run (Independent, once) | No errands or scenes within 1 km. No disguise within 1.5 km |
| **Nobody finishes** | — | — | The built fallback makes the batch about 17 days after the gate (about day 56). The room can still be fought over | — |
| **The epilogue week** (168 real hours) | Nobody new turns. The tribes run down; the last alphas call with fewer answering. Crawlers don't refill, so a cleared town stays cleared | Last Rounds: the four codas on the first free evenings | The route, how it was won, and the terms. Locks and the contest's grudges end at the cure | The closing work ([below](#the-epilogue-week)). The hall of fame freezes at the cure |
| **The wipe** | — | — | — | The voice at the barrier asks *"Name?"* |

---

## The epilogue week

**The author's rule (decision 12):** after the cure the world stays open for **one week, 168 real hours**, and
the week holds quests and missions that let you actually finish everything. [EPILOGUE.md](EPILOGUE.md) owns the
week: its phases, its calendar, its beats, the dose outlets and the closing work. This section is only the
shape, and who supplies each part.

**The week's three phases:**

| Phase | Runs | What it's for |
|---|---|---|
| **A. The news** | From the cure to the first game dusk with a player online (1–6 real hours) | The cure is announced. The route marks become final. The queue forms. Leaders give their verdicts. The dose outlets open |
| **B. The days after** | Until the last day | The closing work and the codas. Evolving beats step at a third and at two thirds |
| **C. Processing** | The last 24 real hours (**Season > Warn hours**) | Anything still open closes as data, with its standing version. The queue at its longest. The built wipe warnings carry the processing line |

**The closure set.** Every item can be finished inside the week, by a busy server or a quiet one. None pays
anything that carries into next season except hall-of-fame lines. Next season's rules are fixed at the cure,
and nothing in the week changes them.

| Closing work | What it is | Designed in | How it always finishes |
|---|---|---|---|
| **Last Rounds** | Each faction's closing arc, playable: the Haven reads the List at the north road; Bastion's last call and last roll at the barrier; Halcyon's paper goes out; the Red Hand closes the book, paid or not | [MAJORS.md](MAJORS.md#last-rounds-the-codas-in-the-epilogue-week) (the scenes), EPILOGUE.md (the calendar) | Live on its first free evening with a player who could take part; if it still hasn't run when processing begins, it runs as data and its standing version goes up |
| **Loose Ends** | Every questline still unfinished at the cure gets a short closing stage, so every story reaches an ending, even a rushed or bitter one | EPILOGUE.md, on the built questlines ([QUESTLINES.md](../docs/QUESTLINES.md)) | One stage, then the ending; at processing it closes as data. The majors never need one (every climax is settled by the gate); "A Week On" closes with its ending's line. Errands are never loose ends |
| **Bring Them Home** | The tags, ID cards and keepsakes of the named dead (players' dead who ran with tribes among them) carried back to their houses and posts. A wall of names fills | EPILOGUE.md, with [INFECTED.md](INFECTED.md#after-the-cure-the-last-week) (the named young), [ERRANDS.md](ERRANDS.md#after-the-cure-the-epilogue-week) (His Effects) and MAJORS.md (Sedlak's box) | Each delivery is one trip. What nobody carries is listed on the wall as "not brought home" at processing |
| **Quiet the Ridges** | Nobody new turns after the cure. The last alphas call with fewer answering. Hunt them, or let them die out | INFECTED.md (the run-down), EPILOGUE.md (the work) | Starving tribes lose a fifth every 12 real hours; on the last dusk every living alpha calls once, nothing answers, and it lies down at dawn |
| **Take Back a Town** | Crawlers no longer refill, so a town cleared stays cleared. A candle in a window for each | INFECTED.md (no refill, no rise), EPILOGUE.md (the candles) | Each town is its own small finish. At most 8 towns get an epilogue mark |
| **The Long Table** | If it was never exposed: the family's last week. Find it out, or watch them pack the jars and leave | EPILOGUE.md, on the built Long Table | Found, or gone by processing. Never described; no ambient line hints at it |
| **The Walk Out** | The Perimeter stands down. Escort people to the north road barrier. Your character can walk out for good, retired with a hall-of-fame line | EPILOGUE.md | Escorts only while `PerimeterSealed()` is false; never the same hour as a coda at the barrier |
| **Settle Accounts** | Clear bounties, pay what you owe the book, give back borrowed kit, make amends | [REPUTATION_NAMES.md](REPUTATION_NAMES.md#14-the-last-week-settling-accounts), ERRANDS.md (Come In), MAJORS.md (lines marked "Paid" at the last line) | Through `Forgive()` and its budget; open promises get a closing line |
| **The Last Broadcast** | Decide what the radio station tells the outside about the season: the names, the three weeks, the truth | EPILOGUE.md, with DIALOGUE.md (radio) and WRITING.md (one line for everyone) | A tally, closed before processing; no player names on air except honour lines |
| **Personal Goals** | Each background's goal gets a finisher | EPILOGUE.md, on the built backgrounds | One errand-sized finisher per background |
| **The Last Entry** | Write your own line on the memorial board | EPILOGUE.md, with the names policy | Any time in the week; filtered, and `/anonymous` is honoured |

**What else the week holds** (from the epilogue design, kept): about 75 small beats and lines at the places
they're about (the lab, the queue at the north road, the strongholds, the safehouses); leaders' topics "The cure"
and "Me"; the four stronghold grey choices; the last errands as the epilogue deck; one dose per life for the
bitten, given on the spot, priced from one outlet table that reads the finale's terms (conflict 40); and the
hall of fame frozen at the cure. No slides, no new screens.

**The built default changes:** **Season > Epilogue hours** goes from 48 to 168, as an attribute default, never
an initialiser (step 0.5).

---

## One owner per thing

| Thing | Owner | Others only read or call it |
|---|---|---|
| Tiers, the 16 names, the faction's word, crew names, life deeds and tallies, the forgiveness budget, the parley, settling up, `CanSideWith`, the season record | Reputation ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)) | Everyone |
| Kit and colours, the look, catches, word gets round, vouching at barriers, the nail list | Disguise ([DISGUISE.md](DISGUISE.md)) | Dialogue (claims), missions (`PassAs`), errands (cards), tone (lines), the finale (the Coat Run's look) |
| Conversations, sessions, owner-only delivery, regard, claims, knowledge, scenes, sways into tallies, the commit node | Dialogue ([DIALOGUE.md](DIALOGUE.md)) | Everything that talks |
| The story kit: ways, the persuade and Fear checks, who benefits, who decides, truths, fallout, route assets, reasons, the price table | Missions ([MISSION_STYLE.md](MISSION_STYLE.md)) | Errands, the majors' stages, the finale's stages |
| Small jobs: the dealer, routines, cards, named people, locals, strangers and their spots, the footfall grid, the interruption budget | Errands ([ERRANDS.md](ERRANDS.md)) | Missions' odd jobs and found hooks, reputation's and disguise's side jobs, and the epilogue's last errands all become errands |
| The text standard, season mood, temper, grief, joke memory, the voice bank, the lint | Tone ([TONE.md](TONE.md), becomes `docs/WRITING.md`) | Every line |
| The epilogue week and its closing work, the town log, the record, beats, carriers, the dose-outlet table, epilogue scenes, the hall of fame freeze, the names policy | Epilogue ([EPILOGUE.md](EPILOGUE.md)) | Everyone |
| The season budget and its lanes, the set-piece calendar, votes (`RZ_QuestVote`), pages, `SeasonStory`, the dynamic fallbacks, the codas' scenes | Majors ([MAJORS.md](MAJORS.md)) | Missions, errands, the epilogue, tone, the finale |
| Routes, the lock, the doors, live tissue's order, the stand, how it was won, terms, the Fame chunk | Finale ([FINALE.md](FINALE.md)) | Everyone |
| Tribes, crawlers, day parties, lairs and ranges, the land grid, the agent budget, signs, the run-down after the cure | Infected ([INFECTED.md](INFECTED.md)) | Missions, errands, the epilogue, dialogue, tone, reputation, the finale |
| Visibility, the infected's senses, losing them | Stealth (built, `Zombies/RZ_Stealth.c`) | Tribes' scout rule and bodies, errands' witnesses, the finale's stand |
| Season pacing, the gate, the cure's fallback | The cure (built) | The majors' signals, the finale |
| The order of the work, the cross-design rules, this table | This plan | — |

---

## The finale's edges

[FINALE.md](FINALE.md) owns the fight. This plan fixes only the edges the other pieces are built against.
Conflict 24 brought these in line with the final finale.

**Before 90%, nothing locks.**
- Mission outcomes can add a **route asset**: an ally, a proof or a standing. At most one per outcome, and 3 per
  player per route a season.
- Errands' Hand to Hand lets a player promise up to 6 locals a dose by hand (Independent). Promises ease only the
  eight's yes and shorten the lab-gate sit-in; they never ease a stronghold leader (conflict 22).
- Assets and promises tilt routes. Nothing closes one: a mark makes a check at most one step harder; a sulk lasts
  at most 2 hours of play; Infamy works off as usual.
- Anyone not locked elsewhere may do a route's **Acts I and III** and keep the pay (decision 8). Acts II and IV are
  sworn: only that route's locked players.

**At 90%: the commit.** Research stops at the built gate. Five routes open: the Haven (The One Who Didn't Turn),
Bastion (Held for Collection), the Red Hand (The Cook), Halcyon (The Lower Store) and Independent (One Each).
- **Where.** A dialogue confirm node tagged [Point of no return]. Faction routes: at that faction's stronghold
  leader, or any Loyal seat leader, as an `always` topic, so a leader who won't talk to you can't block it.
  Independent: on the open channel, at any powered house radio or a carried one. Without the dialogue engine: two
  action-menu steps, then the three lines as an owner-only notice.
- **Who.** The engine decides (`CanCommit`), never the text. It reads `CanSideWith`: yes; by parley (you're
  owed); by price (the route's price door: a debt in the Red Hand's book, signing on with Bastion as a civilian,
  the Haven's "In pencil"); Independent is always open.
- **How long.** Per identity, until the cure. Dying doesn't undo it. The undecided and late joiners can commit
  later, but a lock counts at a stand only if it came before the batch was ready.
- **Never blind.** The lock card states the route's admission, what you bring and what you leave.

**After the lock**, every piece applies the same rules:
- No errand or mission offers you route work for another route.
- An option that leans toward another route refuses you when you pick it. Labels stay the same for everyone.
- Disguise looks are −1 with every faction whose route you didn't choose (all four for the Independent).
- Parley and settling close with factions whose route is fighting yours.
- The word they use for you gains a one-word overlay ("the other crew", "an uncontrolled variable"). Bastion's
  overlay never says "attached", which now means "Civilians, attached".
- Route lines are face to face only, cool on the first visit, back to normal over two real days. Shared
  channels never single out anyone's route; on air the Independent is "nobody" (conflict 27).
- Route marks count only active locks, and they're private: every route with an active lock gets the same share.
- Finale stages never write QUESTLINES' crossing-lines mark (conflict 17).
- Killing a locked player is free at the stand between locked sides; outside it, +3 Infamy with the victim's
  route faction (the Haven, for an Independent), 12 an hour at most (decision 9).

**The winner rule changes.** Today the cure goes to the faction with the most season Fame, chosen before the
stand's own Fame "so the last fight can't decide it" (DESIGN.md, "The season's end"). Under the finale, the
side holding the clean room when the stand is won controls the cure.
- The first route to finish gets a capped chunk of season Fame for its faction (about 375 on five players;
  decision 4). The Independent gets none, unless it controls and chooses "one side"; then that faction gets
  half (decision 5).
- Season Fame then decides the hall of fame's "Zone's Fame" line, who the cure is handed over to when nobody
  holds the room, and the Fame leader's next-season rules at half (when it isn't the controller and there was
  no deal).
- Next season: at most one half rule set (the deal partner if there was a deal, else the Fame leader), then the
  majors' story rules, all through `RZ_SeasonMods.Add` and `Cap` (conflict 19).
- This changes DESIGN.md, REPUTATION.md and LORE.md's Endings in the finale's commit.

**What the finale hands everyone else** (one interface):
- `RouteOf(identity)`, `CanCommit`, `Commit`;
- active locks: 2+ hours played this season, and online in the last 2 real days;
- `finale.route`: 0 nobody (handed over), 1–4 a faction, 5 Independent; `finale.finisher`: 0 the lab alone
  (the slow finish), else the route (conflict 39);
- `finale.how` (held, force, deal, betrayal, theft), `finale.terms`, allies, and up to 4 deciders with names
  frozen at the deed;
- the stand's holders, with their seconds at the lab; stands lost; batches taken;
- `finale.marks_public` (0: private);
- `Asset(route, kind, identity, key)`;
- at the cure: locks end, and every grudge and squad the contest started stands down once.

**The Independent route, across the pieces:**

| What | Where it comes from |
|---|---|
| Its subject | The eight who walked out of the Day 6 trial, and the radio ham's numbers ([FINALE.md](FINALE.md), §6.5) |
| Its pitch | The Doctor wanderer (tone); if the Doctor is gone, the Courier, else an unsigned sheet at the lab gate |
| Its commit | The open radio (dialogue) |
| Its standing rule | Always open (reputation) |
| Its assets | The open channel, "nobody", letting people go (missions) |
| Its promises | Hand to Hand (errands) |
| One way to take the room | The Coat Run: re-label the batch inside the lab (decision 7; the look is disguise's) |
| Its terms | Open, one side, sold, or kept back (the deciders) |
| Its dose outlet | The ONE EACH crate at the queue (epilogue) |

---

## Where the pieces disagreed, and how it's settled

**One side-quest system.** Five pieces each invented small jobs: missions' 25 odd jobs and found hooks,
errands' routines and cards, dialogue's asks and strangers, reputation's 11 seeds, disguise's 10 side jobs,
and the epilogue's last errands. They become one system: **errands**.
- Missions' **reasons** are its Why. Dialogue is how an ask is spoken. Odd jobs become routines.
- Seeds, side jobs and found hooks become cards. The last errands are the epilogue deck.
- **Duplicates merged:** The Pot + Short This Week; Water Barrels + Water for {house}; The Wire + Wire; The
  Freezers + Cold Box; Short Change + Old Habits; Spare Magazine + Fell Off the Inventory; Paint + Name Day's
  Brush; the two "Vouch" cards; The Latrine + Make It Right + Clean Slate + Come In (one amends family); The Name
  on the Tag + Whose Satchel; The Name on the Tape + The Locker + His Lighter + The Tags + The Wreck's Name (one
  "bring their effects home" family, a variant per faction, feeding Bring Them Home after the cure); Notebook 31
  + His Notebook + Into the Record.
- **Premise collisions re-angled or cut**, by errands' premise rule: Contact (vs Contact Report); Tag One and
  The Tag (vs Index Case); Volunteers (vs Blood Drive and The Other Two); The Book and Collections (vs The Book
  Has to Balance and Debt Collection); Civvies (vs Struck Off and Desertion Route); Somebody Should Tell Them and
  the walking-with ask (vs Bring Them Home): read the marks and hand off, no keepsake, no player names.
- Trail of Clues stays a mission story job.

**Leaders never give errands.** Missions' council-head odd jobs and reputation's crew-boss dares move to named
people; the leader says who to see. Reputation's local deeds at a seat are errands' "threads": words a leader
says, never attitude.

**One threat score.** Missions' Press, reputation's quiet threat and dialogue's `threaten` become one **Fear**
score:
- **The score:** the higher of your Feared band (Infamy earned this life) and backing (your Earned Fame with the
  giver), +1 for a listed background. No bonus for being armed.
- **The cost:** +3 Infamy, as the built FORCE threat costs. A scene may name more, up to 8.
- **The limits:** once per person, by anyone; three per life per faction; a failed threat is barred at that
  speaker for a real day; never on stronghold leaders.
- Free-standing talk threats are never offered in safe zones or safehouses. A threat a mission writes, such as
  Protection Racket's, stays.
- **The enum:** PRESS, TRADE, TRICK and LEAVE are appended once, in that order. TRICK is disguise's and
  missions' shared value. Built threat stages keep recording FORCE.

**One persuade score (decision 11).** Dialogue's `DialogueScore`, used as the Trust check everywhere:
- your access rank with the faction, with mixed tiers one lower;
- +1 for a listed background;
- at most +1 from dialogue (remembered help or a vouch);
- −1 for the liar count; −1 for crossing lines.

Missions' Pass / Short / Refused results sit on top of it. The built `PersuadeScore` is untouched. The
trusted-and-feared feel comes through Fear.

**One look for disguises.** The disguise piece owns it.
- Dialogue's random challenge odds and missions' trick score fold into the fixed look, shown before you commit.
- **Cover details**, each +1 at the look: Bastion's word at the wire (heard on the secure net, or told by a friend
  over voice; a wrong word is an instant fail); a real name you've heard (a leader's, or a dead man's from the
  nightly call); a crew name, or a notebook number; a mission's paper.
- **Kit kinds:** own, issued (a real day's grace when your standing slips below Known), looted, signed-for, a dead
  man's, off the turned. A dead Red Hand man's kit passes only if it was bought from the fence (conflict 33).
  Halcyon's full kit is a coat plus a hood or mask with a respirator: the beaks (conflict 35).
- **One cost when caught:** +3 Infamy, never taking you into Watched on its own; the kit is taken; one of 3
  catches per life per faction. Missions' fixed +10 is dropped.
- Errands' and dialogue's definitions of a disguise are replaced by the disguise piece's.
- Tone's lines follow the look's results; there's no suspicion meter.
- No disguise works within 1.5 km of the lab during the stand (conflict 13).

**One `AddInfamy`.** Reputation and disguise each added a parameter. They become one set of flags:
- SHARE: a rival share. It isn't direct Infamy and is never doubled.
- ACT: done in sight. It may double while you're passing as that faction.
- An optional deed text.
- The credit rule: at Fame 75+, each Infamy gain also costs the same Fame.

**One forgiveness budget.** 30 Infamy per faction per life, counted after the leader's temper. Everything that
works Infamy off goes through `Forgive()`: amends contracts; paying in Favor (stops at 15); settling up in Fame;
surrender, the finale's "Hand it in" included (conflict 23); parley amends; errand amends (2 at a time, 8 a life,
never below 15); dialogue's Make it right (a leader at −2 offers an amends contract, which clears that leader's
built crossed bit once a season); the epilogue's Come In and Settle Accounts.

**One vouch.** Dialogue's +1, disguise's sign-in at the barrier, and the Vouch cards use one mechanic with one
liability: +5 Infamy for the voucher, once per vouch; 2 hours without vouching; regard −1 if a lie was told
under the vouch.

**Lies.** One lie store, with an exposing condition on every record. Lies are found out by events first: a visit
by a player of that faction, road news, the owning system's tick. Dialogue's two 15% discovery rolls become the
"rumour tick", only for claims with nothing in the world to test. "A lie pays at most 60%, no Fame" is one rule.
Missions' leader bit `MEM_CAUGHT_LYING` is dropped: dialogue's regard *lied* covers it.

**Memory.**
- Reputation's life-stamped leader memory is adopted: earlier lives count at half, and are voiced once as the
  name. Dialogue's "built memory unchanged" changes to match.
- Errands write regard only on named-person keys, never on leaders.

**Names.** The epilogue's and tone's policy applies everywhere: names are frozen at the deed and pass the note
filter; `/anonymous` and the admin strike are honoured; a name is never a joke's butt; a name is never on air,
except a player's own broadcasts and honour lines. So disguise's "Some clown ... Name of {player}" loses the
name, and reputation's gate guards may name a player face to face, filtered, unless they're anonymous. The epilogue
occupant's thanks use a seeded name that isn't in LORE (conflict 30).

**Owner-only text.** `NotifyPlayer` is a broadcast RPC that every client filters for itself
(`Core/RZ_GameModeComponent.c`, `RpcDo_Notify`). So every personal line rides one owner-only payload: errand
offers, look scores, the budget left, lock lines, epilogue personal lines, talk. Errands' "talk lane" is that
payload. The fix is being built now (step 0.5).

**Deciding in multiplayer.**
- Missions' 5-second hold, with one objection per player per job and Stand with, replaces dialogue's "first pick
  wins" and its 10-second window.
- Anything that matters to the season is a tally in `RZ_QuestVote`.
- **Who took part:** 60 s within 150 m, or a step done.
- **Who hears it:** overheard lines at 8 m; scene results and holds to the room at 30 m.
- Errands' helpers (same vanilla group, within 50 m) get half the Favor.

**Echoes, follow-ups, callbacks.** Missions' echoes, errands' follow-ups and dialogue's `later:` become one
callback queue: words only; 10 per player and a system pool of 100; each expires after 3 season days (24 real
hours in the epilogue week); at death, a waiting one becomes a board note. Dialogue's promises (2 hours of the
promiser's own play, at most 3 real days) stay a separate kind of held errand.

**Words that meant two or three things:**

| Word | Was | Becomes |
|---|---|---|
| lean | missions' route tilts; the epilogue's `lean_public`; the majors' vote lean | **route assets**; `finale.marks_public`; the majors keep "lean" for votes |
| mood | tone's season mood; reputation's line groups; a tribe's temperament | `SeasonMood()`; **manners**; tribes keep "temperament" |
| marks | missions' world marks; the epilogue's props; the tribes' faction marks | missions' become **world records**; "marks" means the physical things only |
| faces | errands' remembered people; dialogue's and reputation's "they know your face" | errands' become **locals**; "face" means recognition |
| stories | errands' hand-written layer; missions' story jobs | errands' become **cards**; missions keep story jobs |
| Proof | Halcyon's major and Halcyon's finale route | the major stays **Proof**; the route is **The Lower Store** (decision 15) |
| struck off | Bastion's deserter; the book's settled line | only Bastion's deserter, struck off as KIA with his name still on the board; the book says **paid** (conflict 7) |
| board | Bastion's roster; the tribes' list | "the board" is the roster; tribes go on **the contact board** (conflict 48) |

**What they call you.** Reputation's word table and crew names win; tone's 3.8 table is dropped. Crew names
are given at Known, "after something you'd rather they forgot". They're what the crew holds on you, never an
honour (conflicts 31 and 32). Bailiff's trigger is "laid down a tribe's leader". "Nanny" (for walking the Kid
home) mocks a kindness, so it's reworded or cut.

**Tags show costs and requirements, never outcomes.** Reputation's threat tag keeps its cost and the
Hunted-cliff warning, and drops "he'll tell the post".

**The Long Table.**
- The Jar Run's "pork" joke becomes silence.
- Strangers never appear within 2 km of the farm, never carry a flask and never offer food.
- Errands' Processing card ends at the queue (150–250 m from the barrier), never at the barrier itself.

**Doses after the cure.** The epilogue's outlets replace the earlier draft's capped dose stocks: given on the
spot, never as an item; only to the bitten; one per life, from all outlets together. **One outlet table**, owned by
the epilogue, has a column per finale term; the finale's route lines and errand card 41 read it (conflict 40).

**The broadcast at the gate.** Tone's gate line (*"The status of this area is under review."*) is used, and the
stall timers don't count at the gate. The built gate news is reworded: *"What's missing isn't in a sample case,
and every faction in the Zone thinks it knows where it is."* (conflict 11).

**Checkpoints.**
- Reputation's "search the whole car" applies before any look. A disguised member of a searched car gets the look
  at the search.
- The Wildcard toll and the lab-coat toll take the higher of the two (20), never both multiplied.

**Strangers (decision 14).** A stranger is dealt only at a **spot**: a road or crossroads people use, a
wanderer's route, the approach to a safehouse or outpost, a campfire, a stopped car, near a fresh event or a body,
a town edge at dusk (fog counts as dusk), their own place, or the line. Never in the empty middle of nowhere, never
because a timer ran out, never with another player outside your group within 300 m. The old "one every 15–25
minutes" stays only as a ceiling: it can stop a stranger, never start one. [ERRANDS.md](ERRANDS.md#18-strangers)
owns the rules.

---

## With the four majors

- **One calendar.** The majors' set-piece calendar is the only one. One set piece an evening, server-wide.
  - The epilogue adds two airings and a standing version per piece.
  - Errands deal nothing on a set-piece evening at its place, or within 600 m.
  - Dialogue's public scenes hold up to 16 sessions for a sitting.
- **One budget (`RZ_MajorBudget`), in lanes:**

  | Lane | Morale | Strength | Covers |
  |---|---|---|---|
  | Majors | ±20 per faction | ±8 per faction | The majors' world effects |
  | Contracts and errands | ±10 per faction per season day | ±4 | Mission extras and errands |
  | The finale (conflict 18) | ±10 per faction | ±5 per faction | The finale's world effects, never clipped by the majors |
  | The epilogue | ±5 per faction (inside the majors' lane) | — | The grey choices and closing work |

  Contracts never take a leader below 30 loyalty, never make a Rogue, and never touch a stronghold. Anything
  clipped becomes words only.
- **Votes stay tallies.** Dialogue's `sway:` writes to `RZ_QuestVote`. Errands and their memory never move a vote.
- **Flags that drive voices.** `orders.stance`, `haven.rule`, `book.burnt`, `book.named` and `majors.pages_open`
  drive tone's running jokes, reputation's crew names and `SetWord` ("Sergeant {name}"), Crash Site's pay, and
  rivals' gossip about pages.
- **Dynamic fallbacks (decision 10).** No major runs on fixed days. Each act's default fires from live signals:
  research position and pace, hours played a day, the stage, the faction's state, engagement (offered time and
  touches), the beat gap and the calendar. A **squeeze** keeps a busy server's climaxes on time; **patience**
  counts only offered time; a **last call** and a grace come before every default; a **lead** of at most 5 points
  helps a stalled server; **between beats** fills long gaps with what already exists. Nothing fires on an empty
  server or while a stronghold is Under Pressure. [MAJORS.md](MAJORS.md#dynamic-fallbacks) owns the rules.
- **The stand.** Each major decides how its faction turns up at the lab: at most two of the majors' groups, the
  finisher's faction first, plus Halcyon's guard (conflict 26). The finale owns the stand and decides who controls
  the cure. Halcyon's Act V and the finale's stand are one event (conflict 2).
- **The ending.** The majors supply `SeasonStory`, the codas with standing versions (Last Rounds), and their hall
  lists ("At the barrier", "On the paper"), which merge into the epilogue's "The season" block.
- **The majors' stages use the story kit:** several ways through, failure as a branch. Await Relief's Act IV gets
  disguise's roster trick, within the questline cap.
- **The Bastion record.** The nail list, the roster and "struck off" are one record. A deserter is struck off as
  KIA, his name stays on the board, nobody answers for him at the roll, and someone is sent to bring him home.
  Side jobs add names to it; only the majors and the built nightly call read names on the air. Struck Off and Hunt
  the Deserter never use Private Marek Fiala's name (decision 3).
- **Stage-only missions.** Our Crossroads, Coup Talk and Training Day leave the random pool but keep their classes,
  because the majors reuse them; Training Day stays a Bastion leader job (conflict 37).

---

## With the infected

The approved tribes and crawlers design replaces Grounds. The hooks the seven pieces wrote are generic (a named
leader and its fate, a home, the season day, reasons, signs, lay-down), so they plug into the tribes with the
rename table in [INFECTED.md](INFECTED.md#from-grounds-to-tribes-the-rename-table) (conflict 43).

- **The season day** is one shared clock: the first three stages' hours played ÷ 30, about one real day at the
  45-day pacing. It's built once, in phase 1.
- **Reasons.** Tribes post them: a hungry tribe, a moving tribe, a leader down, a scatter. Missions, errands and
  dialogue answer them.
- **Signs.** The tribes' signs and the epilogue's marks share one prop spawner (`RZ_Marks`), with separate pools.
- **Contracts.** Lay Them Down becomes Bring Them Home; Still on the Roster becomes Bastion's lost posts; Drive Them
  becomes the culls ("Not our sector"). The rest are work rows.
- **Words.** Player text never says "alpha", "tribe" or "crawler"; each faction has its own words, always with
  the place. Tone's lint carries the list ("strays" means only the Haven's exiles). Bastion and Halcyon kit taken
  off the turned fails that faction's look.
- **Lines and names.** The epilogue's tribe beats: still calling, brought home, taken alive for the lab, killed or
  burned out, walking with it, the great alpha (conflict 42). Dialogue's `{alpha:N}` in each faction's own word.
- **Day parties.** Each tribe keeps a day party out: hunters and young only, at most a quarter of the tribe (a
  fifth by default), in groups of 2–4, at most 6 bodies near one player cluster, on its trails, field edges and
  woods inside its range. Scouts on the ridges. The alpha and guard stay at the lair by day with a resting ring.
  They fight normally when they see you, but by day the tribe never sends the full hunt: a scout's day call brings
  at most 4, never the alpha; day noise draws only the day party. Hungry tribes about double by day and come to
  town edges, never into no man's land. The day after a feed nobody is out. Fog counts as dusk. Towns by day stay
  the crawlers'.
- **Stealth judges sight.** The built stealth system decides whether infected see a player. The scout rule reads
  `RZ_Stealth.GetVisibility` and `SightRadius`; tribe bodies get stealth's perception factor; a hunt on its trail
  and the Furious last wave are exempt from losing-them, day parties are not (conflict 46).
- **Live tissue** credits a route's track, never `AddFinaleResearch`: a locked player's own, an unsided one the
  route they choose; an alpha +2, a great one +3, never past 9 before Act IV (conflict 44).
- **The lower store and the cold store** are no-lair ground (300 m), and Halcyon's Act II claims the site tribe
  as HC1 does (conflict 50).
- **Until the tribes are built,** every tribe key reads false, and tribe cards and beats stay off. Crawlers ship
  first (phase 1).

---

## The shared engine

Built once, in phase 1 unless noted, and used by every piece. No piece builds its own copy.

| Piece | What it is | Used by |
|---|---|---|
| **Owner-only payload** | One owner-only RPC for all personal text (dialogue's kind 6). It fixes the built leak: `NotifyPlayer` broadcasts every player's text to every client. The talk screen being built now uses the same kind 6 (`PAYLOAD_TALK`): they're one payload | Dialogue nodes, errand offers and replies, look scores, reputation's notices, lock lines, epilogue personal lines, the journal's detail-on-open |
| **Notice lanes and the interruption budget**, per player | Critical (always shown); talk (owner-only, at once); unasked contacts, at most one every 4 minutes across radio offers, leader pings, wanderers, the Long Table host and errands, with errands last; barks (one a minute, one per speaker per 10 minutes); approach and ambient lines (once per place per session). The budget is a ceiling only: it never deals anything by itself | Everything that speaks unasked. Reputation's "one notice per approach" and the epilogue's "one line a minute" are rules of these lanes |
| **`RZ_Checks`** | One helper for every check: Trust (decision 11), Fear, the disguise Look (score, tags, instant fails), the price table (one value = 5 Favor; food 1, medical 2, a car part 3), Pass / Short / Refused. Tags come from the mechanics and show costs and requirements only. One try per person per job per identity; threats once per person by anyone. The built `PersuadeScore` is untouched | Missions, questlines, the majors, the finale's stages, errands, dialogue |
| **`RZ_Ways`** | The approach helpers moved out of `RZ_QuestMission` and keyed by identity: `TryPersuade`, `TryBribe`, stealth use, `Remember`, `Resolve`, the quiet threat (`m_iMenaceDifficulty`), `PassAs`. `RZ_EQuestApproach` gets PRESS, TRADE, TRICK and LEAVE appended once, so saved runs keep their numbers | Missions, questlines, the majors, the finale's stages, errands |
| **Standing calls with flags** | `AddInfamy(player, faction, amount, why, flags, deed)` with SHARE and ACT; the credit rule at Fame 75+; `AddFame` marking SMALL pay (errands), left out of season Fame, the Trusted goal and records. `Forgive(player, faction, amount, why)` is the 30-per-life budget | Everything that pays or forgives |
| **`RZ_Deeds`** | One store, two scopes. Life: reputation's best and worst deed per faction, kills and jobs; tone's 12 weighted deed slots; missions' life records (pressed, covered, hid a bite). Season, by identity: the ledger (what you did, for whom, against whom, what the giver hid); reputation's season record (best and worst cell, the cell at the lock, crew name, route) | The epilogue's truth beats, reputation, tone |
| **`RZ_WorldMemory`** | In world facts, four timescales: **reasons** (live facts with a subject and place); **the signal ring** (recent events with per-kind quotas and a server-only cause); **the town log** (the best and worst per settlement, plus up to 16 visits); **world records** (per story key and subject, 160, evicted by importance, each with an epilogue line and its carrier) | Missions, errands (strangers' fresh-event spots), the epilogue |
| **Lies, truths and knowledge** | The lie store (per player per season, each record with an exposing condition, plus the rumour tick); the truth deck (per subject, seeded by story key, subject and season, kept 3 season days or until used); knowledge (`knows:` keys, 96 evidence and 32 passing facts); `RZ_Witness.Seen(act, pos, faction)` (bodies within 40–60 m through stealth's noticing, else a small roll near lived-in places); the knowing flag | Dialogue, missions, errands |
| **The callback queue** | 10 per player, a system pool of 100; words only; on played time, near the subject; 3 season days (24 real hours in the epilogue week); at death, a board note | Missions, errands, dialogue |
| **People, in one record set** | Speaker adapters: leaders, quartermasters, wanderers, occupants, named people (territory's built cook, sentry and toll man, plus errands' 4 per holding), a post's speaker at the barrier, quest people, the Long Table, strangers, radio channels. **Locals** dressed onto existing occupant bodies. One scene-people spawner for missions' `RZ_SceneProps`, the epilogue's `RZ_Scenes`, errand scenes and strangers | Dialogue, errands, missions, the epilogue |
| **One body budget** | Non-combat: scene people and askers 16; errand scenes 10 and strangers 6; epilogue scenes 3 with bodies, 6 people each; post speakers within 100 m of a player. One AI-group budget shared with the tribes' agent budget (100 agents; the engine's active-AI limit raised to 160, from phase 0) plus missions' 4 story groups. Every option that needs a body names a text fallback | Everything that spawns |
| **`RZ_Marks`** | One prop spawner: the epilogue's season marks 40, epilogue 100, props 30; the tribes' signs in their own pool of 30; tone's ambient notice prop; missions' props and chalk; the Long Table's tells. On the verge 6–10 m off a road's centre line, spawned within 150–200 m, up at least 10 minutes, never saved. With no prop, text | The epilogue, the tribes, missions, tone |
| **One text pipeline** | `.rzd` for conversations, greets and barks; the voice bank (`RZ_VoiceDefaults.c`, overridable by id, with a disable list) for single-line pools. One lint (structure, checks, effects, the never list, the Long Table and tribe word lists, pronouns, names, information tokens) with a baseline that only shrinks. One simulator family. One token table: `Fill` plus `FillFor` | Every line |
| **Tone's voice controls** | `SeasonMood()` (Epilogue, Hope, Bleak, Grim, Early, with holding rules), which the broadcast reads too; temper per faction, with gaps so it doesn't flicker; `RZ_ToneGuard` grief (by place face to face, by faction on radio); joke memory (per player on the profile, per channel with the season). The channel cap and the mood share combine by taking the lower | Every line picker |
| **Clocks** | One table in DESIGN.md says which clock each rule uses: **season days** for world rhythms; **played time** for personal pacing; **real time** for anti-abuse limits and the epilogue week; `RZ_GameTime` turns game-clock windows into real minutes, calibrated once (VERIFY the time acceleration; phase 0 proposes the mod sets the day length) | Errands, dialogue's routines, the majors' calendar and signals, the epilogue's evening calendar, the tribes |
| **The season budget** | `RZ_MajorBudget` with its lanes ([With the four majors](#with-the-four-majors)) and the seat ledger: never a stronghold leader, never below 30 loyalty from contracts, never past one Rogue. One set-piece calendar | The majors, contracts, errands, the epilogue, the finale |
| **Participation and deciding** | Who took part (60 s within 150 m, or a step); the decide-scene hold (5 s, one objection per player per job, Stand with); tallies through `RZ_QuestVote`; "with the decider" pay; half Favor to the others who did the work; offline participants paid at their next join | Missions, dialogue scenes, errands, the epilogue's grey choices, the finale |
| **One vouch** | +1 within the dialogue cap, or a sign-in at the barrier. +5 Infamy for the voucher once per vouch, then 2 hours without vouching, plus regard −1 when a lie was told under it | Dialogue, disguise, the Vouch cards |
| **One small-pay ledger** per identity | Errand Favor 40 an hour, with diminishing returns; talk-only Favor at most 20 a faction a real day; errand Fame at most 2 each, never above 39 with a faction, 10 a faction a season day, and amends instead at 15+ Infamy. Mission goods are tagged and refused for pay at quartermasters, food delivery, the fence and errands | Errands, missions, dialogue, the epilogue week |
| **The finale interface and the season record** | [The finale's edges](#the-finales-edges); `SeasonStory` from the majors; `TitleFor(identity, faction)` from the best life; the hall of fame freeze and "The season" block; `last_season.json` | The epilogue, everyone |
| **The names policy (`RZ_Names`)** | Names frozen at the deed; the note filter (a match gives the title or "a stranger"); `/anonymous`, stored by identity in `hall_of_fame.json`; `/epilogue strike`; no platform names in errands; never a joke's butt; never on air except a player's own broadcasts and honour lines | Everyone who shows a name |
| **Journal and UI feeds** | Row kinds: contracts, errands, promises, "What you know", the finale page, the season tab with "As the Zone tells it"; the detail-on-open payload (at most 1,500 characters); the standing card fields and the 16 `RepCell` widgets; `RZ_Hud_Look`; the dialogue panel. Each goes into UI.md and UI/LAYOUTS.md. Every feature has a vanilla fallback | The notebook being built now, every piece |
| **The infected adapter** | One read API the tribes fill: the nearest tribe, its alpha with name and fate, its lair, its hunger and temperament, reading days, the keepsake, the walking-with names (through the names policy). Writes go through it too: the Haven's reading day (by a real reading only), kills through the ledger, reasons posted. With no tribes built, every key reads false | Missions, errands, the epilogue, dialogue, tone, reputation |
| **One group-waypoint helper** (step 0.3) | `RZ_GroupOrders`: push, pop, the held waypoint, delete with the unit. Stealth's search waypoint goes through it and is never pushed over a tribe's held waypoint during a hunt (conflict 47) | Tribes, stealth, the spawner, missions |
| **Settings and tooling** | Every new settings class has no initialisers and is made with `RZ_Util.NewWithDefaults`: Dialogue, Reputation, Hostility (disguise), Missions > Story, Missions > Errands, Voice, Epilogue, Finale, Majors, Zombies > Tribes and Crawlers. One log-line family per system. The hourly economy summary (Fame and Favor per player-hour, by way and by family). Admin commands: `rep`, `/talkdebug`, `/errands`, `/epilogue preview\|phase\|strike\|rebuild`, `/tribes`. The runtime-test additions | Every system |

---

## Shared, per player, late joiners, any map

- **Shared, picked once for everyone:** the broadcast, radio, headlines, notices and boards; the route outcome and
  the season story; scene decisions and the majors' tallies; truths about a subject; the Last Broadcast.
- **Per player:** standing and title, the lock, deeds, regard, claims and lies; errands held, joke memory;
  face-to-face lines, personal epilogue lines; strangers (spawned for one player and their group).
- **Late joiners** catch up through boards of record and first-spawn news; the epilogue's empty kitchen; being
  courted once a visit while they're unsided; the journal's full "As the Zone tells it" list in the epilogue's
  phase C. A late joiner in the epilogue week can still do most of the closing work.
- **Any map.** Everything is placed from settlements, holdings, safehouses, the lab and canon spots, with marks on
  the verge. Every optional prefab falls back to text. Most of the map stays abandoned (at most 8 towns get an
  epilogue mark). Arland has no wild land for a tribe; the crawlers carry its season (INFECTED.md).

---

## Built alongside: the work in ROADMAP "Next"

ROADMAP's "Next" items are approved and being built now (`/root/rz_tools/next_plan.md`): notice pacing, the
notebook, subtitles and the reader, talking to people, burning with fuel in your hands (built: commit 86432ae),
loose loot, and lived-in outposts and camps. They're not part of this plan's phases, but several are the first
layer of a shared piece, so the plan builds on them instead of beside them.

| Being built now | Is the first layer of | What the plan does with it |
|---|---|---|
| Owner-only notices (`RpcDo_Notice`, `RpcDo_Payload` on `RZ_PlayerUIComponent`) | The owner-only payload | Step 0.5's leak fix is this change. Dialogue's `PAYLOAD_DIALOGUE` and the talk screen's `PAYLOAD_TALK` are both kind 6: they're one payload, named once |
| Notice pacing (cards by importance) | Notice lanes and the interruption budget | The lanes extend it; importance becomes the lane |
| The notebook (Standing, Journal, Radio, Map key, Hall of fame tabs) | The journal and UI feeds | New row kinds and the season tab go into the notebook's tabs |
| Talking to people: `RZ_Dialogue`, sessions, the talk screen, needs shown before you choose, no dice | Dialogue's engine | DIALOGUE.md's trees, checks, regard, lies and scenes are built on this `RZ_Dialogue`, never a second one. The approved modal screen replaces dialogue's earlier HUD panel idea; today's slots stay the fallback |
| Subtitles and the reader | Barks and readables | Barks, spoken asks and the epilogue's lines become subtitles; notes open in the reader |
| Lived-in outposts and camps (people with routines) | Named people and routines at holdings | Errands' chores and named people stand on these routines |

**One setting to settle.** The talk screen's plan sets **Dialogue** on by default, because a client without the
layout falls back to today's actions. Decision 13 sets dialogue **off** by default until the per-player label test
and a play-test pass. Both can be true if they're two switches: the talk screen (which only wraps built handlers
and adds no content) on, and **Dialogue > Trees** (DIALOGUE.md's new conversations, checks and lies) off until
decision 13's tests pass. This is this plan's proposal; the author confirms it before phase 2.

---

## The build order

Sizes are lines of Enforce Script; today's scripts are about 113,000 lines. Missions (about 11,800) and tribes
(about 7,300, before day parties and the last week) are the designs' own estimates; the rest are estimates scaled
from the built questlines (1,500–2,500 lines each). **Every step ends with `tools/compile_check.sh` at 0, the
runtime test after cross-system changes, and the docs in the same commit.**

### Step 0: now, in parallel (no new features)

| # | What | State | Size |
|---|---|---|---|
| 0.1 | **Get it running** (ROADMAP "Get it playable"): the Workbench project, prefabs from the generator, a 2-hour Arland session. Every VERIFY spike and play-test below needs it | Under way on the author's PC | Workbench evenings, no code |
| 0.2 | **Tribes phase 0** (the zombie lab): runtime groups, Bacon speeds, frame time at 60/100/140 agents, silent kills, the land survey, the tribe simulator | Done on its branch (`worktree-wf_34724fc3-3cf-4`), not merged. Left: the day-party test (a group of 2–3 hopping and holding past a player at each stance with stealth on), the player-killed event, a second hostile faction, the sight trace, facing, replication. The proposed retunes wait for the author ([INFECTED.md](INFECTED.md#phase-0-results)) | Tests and tools |
| 0.3 | **One group-waypoint helper** (`RZ_GroupOrders`): the tribes' held waypoint, the `DeleteUnit` leak, stealth's push and pop, in one commit (conflict 47) | Done on the phase 0 branch except stealth's push and pop | About 200 |
| 0.4 | **Runtime test with questline preconditions forced** (task 37): every built questline starts and ends once, and Convoy Escort, Hijack the Convoy, Mobile Lab and the Locked Bunker start (conflict 20). The majors and the finale stand on these | Planned | About 300 test code |
| 0.5 | **The built bug fixes**, listed below, plus the VERIFY spikes | Being done now | About 1,500 with the lint |
| 0.6 | **A text pass on the design files** (old lore, names, prices, keys) and the author's lore confirmations | Done: these design files carry the fixes. The confirmed lore goes into LORE.md in the finale's commit (conflict 10) | No code |

**The built bug fixes (0.5), being done now:**

| Bug | Fix |
|---|---|
| Personal text is broadcast to every client: `NotifyPlayer` uses a broadcast RPC with client filtering | The owner-only payload for personal lines (shared with the talk screen's owner-only notices) |
| `RZ_Checkpoints.BastionContact` gives a Hunted, unarmed player an endless stand-down | Only while closing in, 90 s at most, closed while a squad is within 1 km |
| `m_aPersuaded` (`RZ_QuestApproach.c`) is keyed by player id, so a rejoin resets it | Key it by identity |
| `RZ_Mission_Choice` switch pay | A switch pays at most the base |
| Amends paid uncapped on the Hunted path in `RewardPlayer` | Capped, through the forgiveness budget once it exists |
| Disguise doubling also hits rival shares | The SHARE flag |
| The lab gate turns away Watched bitten players | Always let a bitten, unarmed player through for the trial dose |
| `RZ_Util.DeleteUnit` leaks move waypoints | Through the group-waypoint helper (0.3) |
| Tone's four built text contradictions: the Red Hand meal aside that hints at the Long Table (`RZ_FactionVoice.c`), the false Halcyon line in `HintAfterCure` (`RZ_LeaderVoice.c`), the Turned Player quip, the shared "Habit." | Rewritten |
| **Season > Epilogue hours** defaults to 48 (`Core/RZ_Seasons.c`) | 168, the attribute's default string (decision 12). The wipe-warning text reads the setting already |

**The VERIFY spikes**, one Workbench evening each: per-player action labels and `PerformAction` running on the
client (gates decision 13); replacing a shown hint; FileIO on addon files on the dedicated server; worn kit read
from loadout slots and told apart from carried items; a sight trace; reading the time acceleration; an
out-of-sight spawn (strangers, day parties); road width; decals; Bacon Zombies following move waypoints (phase 0:
yes, always at a run).

**Docs and tools in step 0:** `tools/strings_inventory.py` and one text lint with a baseline file, so it passes on
today's tree. `docs/WRITING.md` (the writing guide moved out of LORE.md, every link fixed) is added in the same
commit as the lint, not before (decision 16).

*Playable: today's game, fixed.*

### Phases 1–8

| Phase | What | Needs | Size | Playable after it |
|---|---|---|---|---|
| **1. Shared foundations** | `RZ_Checks` and `RZ_Ways` keyed by identity (the approach enum appended once); `AddInfamy`/`AddFame` flags, the credit rule and `Forgive()`; `RZ_Deeds`; `RZ_WorldMemory` (reasons, the signal ring, the town log, world records); the callback queue; the clocks (the season-day helper used by tribes, majors and errands; `RZ_GameTime`); notice lanes and the interruption budget (on the notice pacing being built now); ONE body budget shared with the tribes' agents; the `RZ_Marks` spawner; `RZ_MajorBudget` with lanes for majors, contracts and errands, the epilogue and the finale; the set-piece calendar. Reputation's phase 1: the 16 names with "counts as", Earned Fame and Feared, the budget across every built route, the debounced change notice, the card and its fallback text, the Wildcard search and toll, the admin `rep` read-out | 0.5 | About 5,000 | The standing screen shows the mixed picture; amends are capped; Champions pay Fame for new Infamy. Nothing else visible changes |
| *Tribes 1, in parallel* | Crawlers, the land grid, the plumbing, the agent budget, both off switches. `docs/INFECTED.md` is added here | 0.2, 0.3, the shared body budget | About 1,500 | Towns feel slow and full |
| **2. Talk** | Dialogue's trees on the talk screen's `RZ_Dialogue` (**Dialogue > Trees** off by default, decision 13): hubs, `.rzd` text with the lint, the simulator and `shipped.lock`, regard, claims and knowledge, `dialogue.json`, about 30 conversations (leaders in four voices, the 8 wanderer roles, occupants, two named people, two post speakers, the Long Table, three asks). Reputation's voice: `RZ_RepVoice`, deed phrases and tallies, the faction's word and crew names (the new lore), `Picture`, life-stamped memory, local deeds. Tone's voice bank: `SeasonMood()` and temper (the broadcast reads the same table), grief and joke memory, the top pools moved into the bank, `tools/voice_sim.py`. The dialogue engine comes before every scene, sitting and commit node that uses it; all keep an action-menu fallback. `docs/DIALOGUE.md` is added here | Phase 1; the talk screen | About 6,000, plus about 1,000 lines of text | People talk in trees, in their own voices, and remember you this life |
| *Tribes 2, in parallel* | One tribe end to end on Everon (the lair and resting ring, scouts and the scout rule on `RZ_Stealth`, **the day party**, the dusk call, a hunt, the dawn count), play-tested; then every tribe, with hunger's day doubling and the town-edge fields | Tribes 1 | About 1,800 | The wild has a rhythm, by day and by night |
| **3. Small things everywhere** | Errands' core: the dealer, named people folded into territory's built ones and the camps' people, soft holds and the faults table, 24 routines at the four holding kinds, wanderer asks, 16 cards, telemetry and `/errands`. Missions' wave 1: the story kit, who benefits (`RZ_Mission_Choice` rebuilt on it), who decides, truths, world records, the reason lane and one weight clamp, the journal's detail payload. The 7 missions that become routines leave the pool as their routines land. **The epilogue's town log and season marks start recording here**, because the ending needs a whole season of them | Phases 1–2 | About 7,000 | Many small, relevant side quests; every built choice mission paying fairly |
| *Tribes 3, in parallel* | The life cycle: succession, splits, borders, clans, lair moves, temperaments, habits | Tribes 2 | About 1,500 | The map changes week to week |
| **4. The mission style in full: the stop point** | Missions' wave 3: the six worked examples, the 9 merges, the 15 drops, the 3 stage-only missions (Our Crossroads, Coup Talk, Training Day, conflict 37), the Tier 0 audit. Reputation's phase 3: the quiet threat, the parley and the Knock, `IsEmptyHanded`, settling up, the company you keep. Lies and witnesses: lie checks and tells, the lie store with events first, the witness helper and the knowing flag. Errands' phase 2: **strangers at their spots** (the footfall grid, decision 14), locals, twists, board and radio asks, 24 more cards. Dialogue's multiplayer layer: overheard lines, decide and tally scenes, vouch, call-outs, point-out. Disguise's phase 1: kit from slots, colours, looks at barriers and gates, caught and held, `PassAs` and TRICK, Bust Out and Double Agent. Reputation comes before disguise: the look reads `Notoriety()`, `KnowsFace()` and the forgiveness budget | Phase 3 | About 9,000 | Several ways through every job, givers who lie, failure as a branch, disguises that are hard to abuse. **After this phase the game is whole** |
| *Tribes 4, in parallel* | People and content: each faction's words and marks, skills, knowledge, the 13 contracts and 9 events, your dead run, Bring Them Home, the moat, the questline hooks, the cure milestones and the call box, the stand's waves, the last week's run-down | Tribes 3; the voice bank and errands | About 2,000 | Everyone has a word and a job for the infected |
| **5. The four majors** | Engine first: the major lane, `RZ_QuestVote`, `ActEnd`, gates and windows, **the dynamic fallbacks** (signals, squeeze, patience, last calls, leads, between beats, hard stops; decision 10), the season story record. Then the Red Hand and Bastion (climaxes at 50% and 60%), the Haven, Halcyon. Dialogue's sitting scenes and sways; tone's running jokes on the majors' flags; errands' margin cards (2–4 per major, never on set-piece evenings); Await Relief's Act IV disguise trick; Crash Site reading `majors.pages_open`. `tools/pacing_sim.py` gains the majors' milestones and the order check. Missions' wave 4 (the other 22 story jobs) runs alongside, a faction at a time | Phase 1's budget and calendar, phase 2's sittings, phase 3's story kit, phase 4's disguise trick, and 0.4 | About 10,000, plus about 1,200 lines of text | Each faction has a season-long story with climaxes on the cure's milestones, on time on a busy server and never rushed on a quiet one |
| *Tribes 5, in parallel* | The great alpha, signs, call sounds, Red Dusk, the range ring icon. Finished before phase 6 | Tribes 4 | About 500, plus Workbench | — |
| **6. The finale** | 6a The controller (saved), locks, six tracks, the lab-alone track and the built cure changes (research held at 90, one `AddFinaleResearch` at the seal, milestone 4 through the controller, stand Fame, the winner rule, the new gate line); playable as the slow finish end to end (about 3,000). 6b Finale items, people and claims (Person, Tray; never LAB) (about 2,000). 6c The routes one at a time, each playable alone: the Red Hand (most built blocks), Bastion, Halcyon, the Haven, Independent (about 8,000). 6d The stand as a control fight: the room and side-aware counts, take, overtime, the bay hold, groups and hostility bits, the safe-zone and disguise exemptions, Tally mode (about 1,500). 6e Pacts, deals, the moves, Last Mile, the Coat Run (about 1,500). Then the scripted runtime finale at Wait scale 0.1. What the pieces add at the lock: route assets counted, pitches at 80% and 85%, the lock card's admission, route marks, lock cards and Hand to Hand, looks −1, parley and settling closed, the word overlays, face-to-face route lines (for the unsided and the losing routes too). `docs/FINALE.md` is added here | The owner-only payload, the majors' flags and `RZ_QuestVote`, the dialogue confirm node, `CanSideWith`, `Asset()` and the decide hold, the disguise look, the tribes' live tissue (all but the payload have fallbacks) | About 15,000, plus about 1,500 lines of text: the largest single feature | The last 10% is a fight over who controls the cure, with a real point of no return |
| **7. The epilogue week** | The record snapshot at the cure; beats checked on every showing; every carrier (leaders' epilogue topics, residents, wanderers, radio and the reel, boards, readables, news, the journal); the dose-outlet table reading `finale.terms`; the queue and lab-gate scenes with their safety rules; the four stronghold grey choices on the decide hold; the hall of fame freeze and "The season" block; `IsLastDay` and the late-cure push-back. **The closure set** (decision 12): Last Rounds on the calendar, Loose Ends for every built questline, Bring Them Home and the wall of names, Quiet the Ridges and Take Back a Town on the tribes' run-down, the Long Table's last week, the Walk Out, Settle Accounts, the Last Broadcast, Personal Goals, the Last Entry; the epilogue deck through errands' dealer. Reputation's season record and `TitleFor`; disguise's epilogue lines; tone's variants for every ending, including the slow cure and no cure | Phase 6's interface; the town log from phase 3 | About 8,000, plus about 900 lines of text (this plan's estimate: the 72-hour design's 6,000 and 530, plus the closure set) | When the cure is out, the whole Zone reacts for a week, and every story can be finished |
| **8. Growth, as play-tests allow** | Missions' wave 5 (16 one-choice jobs and 67 work rows); disguise's phases 2 and 3 (again-looks with the sight trace, roll call, word gets round, the Rogue's price, a dead man's kit and kit off the turned, signed-for kit, confiscation, the remaining side jobs); errands to about 60 cards; dialogue's strangers and radio speakers in full | Play-tests after phases 3 and 4 | About 6,000 or more | — |

**In all:** roughly 77,000–82,000 lines (about two thirds of today's code), plus about 4,600 lines of player text.
The game is whole after phase 4; phases 5–7 are the season's story; phase 8 is optional. If the author wants the
new ending sooner, 6a could follow phase 1 with every fallback, but that ships the winner-rule change before any
route exists, so this plan doesn't recommend it.

**The runtime test grows with the phases:** each check result, each truth, each Overtake, a lock refusal, the
majors' fallback checks (three empty days fire nothing; a squeeze sends a last call, then a default), the scripted
finale at Wait scale 0.1, and the epilogue week at Test scale 0.02 with every closing item reaching its end.

---

## The decisions as decided

The author's decisions, numbered as approved. The last column gives the number the decision had in the finale
plan's "Decisions for you", which the author answered by number.

| # | Decision | What the author chose | Was |
|---|---|---|---|
| 1 | Where the designs go | Into the repo, as these design files | — |
| 2 | The finale's new lore | Confirmed: the captain's Day 2 log (*"13:10. Results to Command by radio: 4 R / {count}. Ack."*), Sedlak leading the Day 1 cordon detail, the woman bitten on Day 4 who never turned, the eight who walked out of the Day 6 trial, and the radio ham. It goes into LORE.md in the finale's commit | 1 |
| 3 | Twice | Private Marek Fiala, alive, struck off as KIA. Bastion's route can bring him home; his term is *"A bottle, and my KIA rubbed out."* | 2 |
| 4 | The finisher's Fame chunk | Kept. Season Fame decides the hall of fame's "Zone's Fame" line, who the cure is handed over to when nobody holds the room, and next season's rules at half for the Fame leader | 3 |
| 5 | The Independent and the chunk | None, unless it controls and chooses "one side"; then that faction gets half | 4 |
| 6 | Take the Lab | Kept, as the Red Hand route's once-a-season theft. Any Red Hand-locked player who is Trusted with the Red Hand can start it (built: Champion); every Red Hand-locked player counts for the bay hold | 5 |
| 7 | The Coat Run | It re-labels the batch inside the lab. The batch never leaves the cold | 6 |
| 8 | Open work before the lock | Acts I and III are open to anyone not locked elsewhere | 7 |
| 9 | Killing locked players | Free at the stand between locked sides. Outside it, +3 Infamy with the victim's faction, 12 an hour at most | 8 |
| — | Day parties | As clarified with the author: hunters and young, at most a quarter of the tribe, groups of 2–4, at most 6 bodies near one player cluster, on trails, field edges and woods inside the range; scouts on ridges; alpha and guard at the lair with a resting ring; a scout's day call brings at most 4, never the alpha; day noise draws only the day party; hungry tribes about double and come to town edges, never no man's land; nobody out the day after a feed; fog counts as dusk; towns by day stay the crawlers'; the built stealth judges sight | 9 |
| 10 | The majors' fallbacks | **Changed by the author: dynamic.** No fixed days. Each fallback fires from live signals (research pace, the stage, the faction's state, whether players are engaging with the major, how long since the last beat), so a busy server gets its climaxes on time and a quiet one isn't left waiting or rushed | 10 |
| 11 | The persuade score | Your access rank, with mixed tiers one lower. The built score stays | 11 |
| 12 | The epilogue's length | **Changed by the author: one week, 168 hours**, with quests and missions that let you actually finish everything (the closure set) | 12 |
| 13 | Dialogue by default | Off, until the per-player label test and a play-test pass | 13 |
| 14 | Strangers | **Changed by the author:** strangers never barrage players. A stranger finds you only where one would make sense: a road or crossroads people use, a wanderer route, near a safehouse or outpost, a campfire, a stopped car, near a fresh event or a body, a town edge at dusk. Never in the empty middle of nowhere, never on a timer. The budget stays only as a ceiling | 14 |
| 15 | Halcyon's route name | "The Lower Store". "Proof" stays the major's | 15 |
| 16 | New docs | FINALE.md, INFECTED.md, DIALOGUE.md and WRITING.md are added to `docs/` only as each is built, each with the Doc map and README in the same commit | 16 |

**Smaller choices carried from the New Vegas plan**, taken as recommended:

| Choice | Taken |
|---|---|
| Route marks after 90% | Private: every route with an active lock gets the same share of places (`finale.marks_public` 0) |
| The infected | Tribes and crawlers, not Grounds, with its own 16 decisions ([INFECTED.md](INFECTED.md#decisions)) |
| The four stronghold grey choices | Shared, with the hold and one objection per player |
| Mirror offers | 10%, off on PvE servers ([MISSION_STYLE.md](MISSION_STYLE.md#mirror-offers)) |
| Route Fame | Decided by decision 4: the finale's capped chunk stays, and the earlier draft's route Fame bonus is gone |

---

## The 50 conflicts

Found when the designs were checked against the code, the lore and each other on 2026-11-11. Every resolution is
applied in the design file named, and this plan follows them.

| # | Where | Resolution | Applied in |
|---|---|---|---|
| 1 | The majors' timings (season table; Bastion Act I, the Red Hand's acts 1, 2, 4, 5) vs the built 45-day pacing | The acts move off the old 10-day season, and the send-off is on Collapse's second evening. The real-day fallbacks first became season days 13, 22 and 35; decision 10 replaced every fixed day with dynamic fallbacks. The season table is rewritten. `tools/pacing_sim.py` gains the 60, 70 and 80% milestones and checks the order 50% → 60% → Aftermath/70% → 75% → 80% → 90% from 2 to 20 regulars (on 2, Aftermath comes after the cure, so the second sitting uses 70%) | [MAJORS.md](MAJORS.md) |
| 2 | The majors' stand and winner text vs built HC3 (`RZ_Quest_Authors.c`) | The finale owns the stand; control picks the winner; Take the Lab under a finale is a holder change; Proof's author list uses the finale's bystander rule; HC3's ending reads `finale.how` and `finale.route`, not the batch counters | [MAJORS.md](MAJORS.md), [FINALE.md](FINALE.md) |
| 3 | Await Relief: two men walk off to a safehouse; "Names attached" | The two are struck off ("KIA. Body not recovered"), nobody answers for them at the roll, and Struck Off is weighted up at that post for a season day. "Names attached" becomes "List to follow" | [MAJORS.md](MAJORS.md), [FINALE.md](FINALE.md), [MISSION_STYLE.md](MISSION_STYLE.md) |
| 4 | Twice's terms ("my name back on the board") | *"A bottle, and my KIA rubbed out."* | [FINALE.md](FINALE.md) |
| 5 | Twice living near Bastion's store, with no Bastion way to use it | Bastion's route gets a way to bring him home: find him, walk him to a post, the sergeant rubs out his KIA, he answers "Here", the captain releases his tube. His yes is one step harder for anyone in Bastion green. Struck Off and Hunt the Deserter never use his name (decision 3) | [FINALE.md](FINALE.md), [MISSION_STYLE.md](MISSION_STYLE.md) |
| 6 | The proof's volunteers ("the crews never took their own cut") | *"The crews take their own cut only when a man's worth less than a real dose, and every man knows it."* | [FINALE.md](FINALE.md) |
| 7 | "Strike his line"; unpaid lines "struck off" | "Struck off" is only Bastion's word. "Mark his line paid"; the unpaid lines are "marked paid" | [FINALE.md](FINALE.md), [MAJORS.md](MAJORS.md) |
| 8 | The two Bastion men at the lab "since Day 4" | "Since Day 5" | [FINALE.md](FINALE.md) |
| 9 | The Day 3 clipboard leaves out Wolff's team | *"Day 3. Chained. Soldiers gone. Four of us staying."* HC1's three bodies and the driver's line stay. The clipboard hangs at the lower store, not HC1's keypad store | [FINALE.md](FINALE.md) |
| 10 | Sedlak's Day 1 chain order and the captain's 13:10 log add lore | Confirmed by the author (decision 2); written into LORE.md in the finale's commit | [FINALE.md](FINALE.md), [MAJORS.md](MAJORS.md) |
| 11 | The built gate news ("isn't in a special's blood") vs every route's live-tissue act | *"What's missing isn't in a sample case, and every faction in the Zone thinks it knows where it is."* In the finale's commit | [FINALE.md](FINALE.md) |
| 12 | Take the Lab's standing (built: Champion) | Any Red Hand-locked player who is Trusted can start it; every Red Hand-locked player counts for the bay hold (decision 6). Stated in MISSIONS.md when built | [FINALE.md](FINALE.md), [MAJORS.md](MAJORS.md) |
| 13 | "No disguise at the lab during the stand" vs built `RZ_Disguise` | Within 1.5 km of the lab during the stand, `RZ_Disguise` clears no bits: one `IsFinalStand` check, like stealth's | [FINALE.md](FINALE.md), [DISGUISE.md](DISGUISE.md) |
| 14 | Several route stages at the lab vs one LAB claim | Route stages never claim LAB. The controller claims it only for the batch hold and the stand. Stages claim their own subjects: Person(key), Tray(n), the store, the still | [FINALE.md](FINALE.md) |
| 15 | Bastion's route vs Bastion's 10% presence in Aftermath | Route stages bring their own pieces as stage blocks: up to 2 road posts, one truck and crew, one squad. While Bastion's route has an active lock, `SetPresenceFloor(Bastion, 0.25)` applies | [FINALE.md](FINALE.md) |
| 16 | The finale vs "Limits every questline keeps" | FINALE.md states its own limits. QUESTLINES.md's rule becomes "No faction wins through a questline except the cure finale, and none falls" | [FINALE.md](FINALE.md) |
| 17 | Route acts vs QUESTLINES' crossing-lines mark | Finale stages never write it; the lock's rules replace it and end at the cure | [FINALE.md](FINALE.md) |
| 18 | The finale's world effects vs the spent majors' budget | The finale gets its own lane: morale ±10, strength ±5 per faction | [FINALE.md](FINALE.md), [MAJORS.md](MAJORS.md) |
| 19 | Two half rule sets plus four story rules next season | At most one half rule set (the deal partner, else the Fame leader), then the story rules, through `RZ_SeasonMods.Add` and `Cap` | [FINALE.md](FINALE.md) |
| 20 | The last runtime test started no questline and none of the reused missions | Task 37 before phase 5: force the preconditions; each reused mission and every questline starts and ends once | [FINALE.md](FINALE.md), [MAJORS.md](MAJORS.md), this plan (step 0.4) |
| 21 | Two mechanics called "the knock" | The Haven's price door is "In pencil", built on reputation's Knock: an exile who isn't owed can offer 4 food and a name. It forgives nothing | [FINALE.md](FINALE.md), [REPUTATION_NAMES.md](REPUTATION_NAMES.md) |
| 22 | Hand to Hand promises easing a stronghold leader | Promises ease only the eight's yes and shorten the lab-gate sit-in (a minute per promised local present). The director's check counts locals at the gate | [FINALE.md](FINALE.md), [ERRANDS.md](ERRANDS.md) |
| 23 | "Hand it in" taking 8 Bastion Infamy off outside the budget | The surrender goes through `Forgive()`. With the budget spent, the box is still taken in, and no Infamy comes off | [FINALE.md](FINALE.md), [REPUTATION_NAMES.md](REPUTATION_NAMES.md) |
| 24 | The New Vegas plan's Independent (the fifth track, any subject, commit until the stand ends) | The final finale: its own subject (the eight and the ham), at most one unit from another route's subject, the lab-alone track as a sixth track for the slow finish only, a lock counting at a stand only if it came before the batch was ready | [FINALE.md](FINALE.md), this plan ([The finale's edges](#the-finales-edges)) |
| 25 | Drop route Fame (New Vegas plan) vs the finale's capped chunk | Decision 4: keep the chunk | [FINALE.md](FINALE.md), this plan |
| 26 | Three faction groups at the stand | At most two of the majors' groups, the finisher's faction first, plus Halcyon's guard | [DISGUISE.md](DISGUISE.md), [MAJORS.md](MAJORS.md), this plan |
| 27 | The Independent's radio line names the player | *"This is nobody. The cure goes to everyone."* The reply stays. The confirm says *"Every faction hears that somebody said it."* | [DIALOGUE.md](DIALOGUE.md) |
| 28 | Bastion's confirm node: "until the season ends", and the lock card's lines missing | "From now until the cure...", with the route's admission and the bring and leave lines | [DIALOGUE.md](DIALOGUE.md) |
| 29 | The toll man's old Rudi story ("before dark, like he asked"; Day 8) | *"Rudi? Rudi got bitten. Day 7. Everybody knows that. Nice try. It's fifteen now."* Tell note: dead since Day 7. The official lie, said flatly | [DIALOGUE.md](DIALOGUE.md) |
| 30 | "Mrs Halas" at a Haven house | A seeded name that isn't in LORE | [DIALOGUE.md](DIALOGUE.md) |
| 31 | Crew names "an honour" | Crew names are what the crew holds on you. The table mostly fits; Bailiff's trigger becomes "laid down a tribe's leader"; Nanny is reworded or cut; the quoted code is fixed | [REPUTATION_NAMES.md](REPUTATION_NAMES.md), [ERRANDS.md](ERRANDS.md) |
| 32 | Tone's old lore (crew names, grey reasons, Rudi as a toast, names leaving the roster) | Red Hand: *"Nobody was coming, and nothing taken ever has to be given back. Everyone would sell you, so keep something on everyone."* Bastion: *"They're all they have: feed your own, look for your own, bring your own home."* Rudi's end is never a joke; the lie is said flatly. *"Someone got to him first. His KIA's rubbed out."* | [TONE.md](TONE.md), [MISSION_STYLE.md](MISSION_STYLE.md) |
| 33 | Disguise: a dead man's boots, deserters off the roster | A dead Red Hand man's kit passes only if bought from the fence (*"Bought fair. It's in the book."*). "A deserter struck off as KIA, his name still on the board"; the call-back is "hands his tag to the sergeant" | [DISGUISE.md](DISGUISE.md), [ERRANDS.md](ERRANDS.md) |
| 34 | Disguise's Coat Run walks the batch out | Disguise keeps only the look (difficulty 3, the lock's −1, call-outs harder) and links to the finale; the batch is re-labelled inside the lab (decision 7) | [DISGUISE.md](DISGUISE.md), [FINALE.md](FINALE.md) |
| 35 | Halcyon kit words ignore the respirator | Halcyon's full kit is a hood or mask with a respirator, plus a coat. Respirator and gas-mask words go into the attribute's default string (never an initialiser) | [DISGUISE.md](DISGUISE.md), [MISSION_STYLE.md](MISSION_STYLE.md) |
| 36 | Crash Site's Day 5 manifest, "Bastion paid the first toll", "Ask the Jandas" | The manifest is dated Day 9 or later. *"Signed correct, every one. Turns out they were signed to us."* *"We paid. Ask Janda at the pit."* | [MISSION_STYLE.md](MISSION_STYLE.md) |
| 37 | Missions drops Our Crossroads, Coup Talk and Training Day, which the majors reuse | Questline stages only: out of the random pool, classes kept. Training Day stays a Bastion leader job | [MISSION_STYLE.md](MISSION_STYLE.md), [MAJORS.md](MAJORS.md) |
| 38 | Errands' lockers "for when relief comes"; "on the roster, or struck off" | "For when the gates open." "He's on the roster as KIA, or struck off." | [ERRANDS.md](ERRANDS.md) |
| 39 | Errands' and tone's reads of the finale's keys | Read `finale.route` (0 nobody) and `finale.finisher` (0 the lab alone) as the finale defines them, and `RouteOf(identity)` everywhere | [ERRANDS.md](ERRANDS.md), [DIALOGUE.md](DIALOGUE.md), [MISSION_STYLE.md](MISSION_STYLE.md), [TONE.md](TONE.md) |
| 40 | Three different dose prices | One outlet table, owned by the epilogue, with a column per finale term (for example: the boss's price 30, strangers ×1.5; pay when you can, 15 now and 15 in the book; houses first, free at the sister's house; the book first, free plus 30 on your line). Halcyon always cures and serves in the terms' order; home batches only under Published. Exiles are served under "by the bite book", the Independent's "all of them", or the chair's grey choice. Route lines and card 41 read it | [EPILOGUE.md](EPILOGUE.md), [FINALE.md](FINALE.md), [ERRANDS.md](ERRANDS.md) |
| 41 | The epilogue's "Relieved in Place"; the Deserter "whether he's on it" | "The Call Stands"; "whether anyone answers for him at the roll" | [EPILOGUE.md](EPILOGUE.md), [MISSION_STYLE.md](MISSION_STYLE.md) |
| 42 | The director's "first in isolation"; Grounds beats | "With the index in isolation" (Act III's specimen). Grounds beats become tribe beats: still calling, brought home, taken alive for the lab, the great alpha | [EPILOGUE.md](EPILOGUE.md), [INFECTED.md](INFECTED.md), and the seven pieces |
| 43 | Grounds names in all seven pieces | One rename table: first → the tribe's leader (each faction's word), den → lair, ground → tribe or range, the Eldest → great alpha, Every First Calls → Every Ridge Calls, `ground.hungry` → `tribe.hungry`, `first.down` → `alpha.down`; Lay Them Down, Still on the Roster and Drive Them → Bring Them Home, Bastion's lost posts, the culls. One season day, built once in phase 1 | [INFECTED.md](INFECTED.md), and the seven pieces |
| 44 | Tribes crediting `AddFinaleResearch` | Tissue credits a route's track: a locked player's own, an unsided one the route they choose; a great alpha +3 | [INFECTED.md](INFECTED.md), [FINALE.md](FINALE.md) |
| 45 | Day parties not designed | The author's version replaces the first proposal: hunters and young, a quarter at most, groups of 2–4, 6 bodies near a cluster, the rest as in [the decisions](#the-decisions-as-decided). Added to the phase 0 tests | [INFECTED.md](INFECTED.md), [FINALE.md](FINALE.md) |
| 46 | Tribes vs built stealth: two systems judging one player | The scout rule reads `RZ_Stealth.GetVisibility` and `SightRadius`; tribe bodies get stealth's perception factor; hunts on their trail and the Furious last wave are exempt from losing-them; day parties are not | [INFECTED.md](INFECTED.md) |
| 47 | Three owners for one group's waypoints | One helper owns group waypoints; stealth's search waypoint goes through it. One commit, in step 0 | [INFECTED.md](INFECTED.md), this plan (step 0.3) |
| 48 | Tribes' Red Hand and Bastion words ("the boss is the one who won"; "the board") | *"The Postmen up the quarry. Same as us: the boss is whoever's got something on the rest."* Nicknames "after something they'd rather forget". "The contact board" every time | [INFECTED.md](INFECTED.md), [ERRANDS.md](ERRANDS.md) |
| 49 | "Land the 45-day pacing branch first"; the epilogue's pacing table | The step is dropped (merged, b78615a), and so is the 10-day column. The epilogue length is now decision 12's week | [INFECTED.md](INFECTED.md), [EPILOGUE.md](EPILOGUE.md), this plan |
| 50 | The lower store and the cold store vs lairs and scout ridges | Halcyon's Act II claims the site tribe as HC1 does (bodies keep 150 m from the lower store while it runs). The land build marks 300 m round the lower store and the cold store as no-lair | [INFECTED.md](INFECTED.md), [FINALE.md](FINALE.md) |

---

## Risks

1. **The engine everything stands on is untested end to end.** The last runtime test started none of the 15
   questlines and none of the four reused missions. Mitigation: step 0.4 before phase 5, and no major or route
   built on a stage that hasn't run once.
2. **Scale.** Together this is roughly 77,000–82,000 lines, about two thirds of today's code: missions about
   11,800, the dialogue engine and about 30 conversations, the finale about 15,000, the epilogue week about 8,000,
   about 4,600 lines of text, 30 routines and about 60 cards, disguise's phases, the tribes. Mitigation: the shared
   engine is built once (phase 1); phase 4 is a stop point after which the game is whole; tier-1 text ships first;
   lines are built from parts.
3. **Engine unknowns gate the delivery layer:** per-player action labels and `PerformAction` on the client;
   replacing a hint; FileIO on addon files; worn kit from loadout slots; a sight trace; the time acceleration; an
   out-of-sight spawn; decals; Bacon Zombies following waypoints (phase 0: they do, at a run). Each has a fallback:
   numbered `/t` options, disguises off, a 10 m look, assuming 1:1 time, a road-bend spawn, a board note, placement
   by mood. Step 0 runs the spikes before any code depends on them.
4. **The dynamic fallbacks are harder to predict than fixed days** (decision 10). More signals mean more ways for a
   major to fire early, late or never on an odd server. Mitigation: every default needs a last call and a grace;
   nothing fires on an empty server; the hard stops at the gate and the wipe's last 24 hours; the order guard;
   `pacing_sim.py` from 2 to 20 regulars; the runtime test's fallback checks; one log line per decision; and
   settings (Lead points, Grace sessions, Hold limit, Stall points, Between beats) the author can tune.
5. **The epilogue week is long** (decision 12). On a quiet server players may drift away before the closing work
   is done; on a busy one a week of content can run thin. Mitigation: every closing item runs live first and closes
   as data at processing with a standing version; the closing work is small and can be done in a week of normal
   play; late joiners can take most of it. Admins with fixed restart days or an After-weeks wipe need the 168-hour
   default and its interaction with the wipe in the changelog.
6. **The finale changes a built rule** (the winner) and brings PvP to a stand where about 5 players hold today:
   balance, griefing, bounty hunting at the lab and body budgets all need a play-test. Small servers may have one
   player per route. Mitigation: every act has a solo way; Tally mode on PvE; the kill rule (decision 9); the slow
   finish and "handed over" as fallbacks.
7. **Interfaces drifting apart** across seven pieces, the majors, the finale and the tribes. Several names meant two
   or three things (lean, mood, marks, faces, stories, Proof, struck off, board). Mitigation: the renames and the
   owner table here, one shared-engine list, and a review of every API name before each phase.
8. **Two talk systems by accident.** The talk screen is being built now from next_plan.md; dialogue's trees come in
   phase 2. If they don't share `RZ_Dialogue` and the kind 6 payload, there will be two. Mitigation: [Built
   alongside](#built-alongside-the-work-in-roadmap-next), and the two-switch proposal for decision 13.
9. **Day parties or strangers barraging players.** Both are the author's explicit worry. Mitigation for day
   parties: the caps (a fifth by default, groups of 2–4, 6 bodies near a cluster, at most 4 on a day call, never the
   alpha) and the phase 2 play-test of real day crossings, lowering **Day party share** if days feel like nights.
   For strangers: spots only, the footfall grid, the same spot never twice a real day, the ceiling only stopping.
   The opposite risk, strangers too rare on a quiet map or Arland, is acceptable: nobody asking is the rule when
   nothing is true near you.
10. **The tribes' numbers.** Phase 0's simulator says the design's own numbers make tribes too hungry and too few on
    the measured land; its retunes wait for the author. Arland has no wild land for a tribe. Mitigation: the
    retunes, the **Slots** override, and the status log of members and hunger per season day on live servers.
11. **Balance and economy.** More ways through, many errands, story-job extras, settling up and dose outlets all move
    Fame and Favor per hour, and with them the cure's pace. Mitigation: the one small-pay ledger, the Fame ceiling,
    the forgiveness budget, the contract lane, the hourly economy summary, and play-tests after phases 3 and 4.
12. **Performance.** The tribes' 100 agents, missions' story groups, scene people, strangers, post speakers, epilogue
    scenes and two prop pools (up to 60 props) could all arrive near one hub, such as the lab or the queue.
    Mitigation: one body budget and one prop spawner with priorities and text fallbacks. Measure frame time at 20
    players and 60 props.
13. **Multiplayer griefing:** first-decider grey choices; objections used to stall; call-outs and point-outs; vouch
    liability dumped on a friend; shooting scene people; misinformation through lies. Mitigation: the 5-second hold
    with one objection, tallies for anything that matters, liability limits, lies that never write data, the witness
    and knowing rules, and Infamy for harming scene people.
14. **Save size:** `dialogue.json` about 3–5 KB per active player; world memory about 40 KB; the epilogue record and
    town log up to about 85 KB; errands about 50 KB per 100 players plus about 20 KB for locals and counters; the
    tribes about 15 KB. Measure `state.json` after each phase, and cap seen nodes and records where needed.
15. **Readability.** Tiers and names, looks and tags, budgets and route locks: a new player could drown. Mitigation:
    the card always shows "counts as"; tags state needs and costs; guidance tips for first tags, truths, holds, odd
    jobs and looks. Play-test whether new players still understand "needs Trusted".
16. **Privacy and names.** Names appear on boards, the wall of names, the memorial board, posters, stand lists and
    gate lines. A name can pass a word filter and still offend. Mitigation: names frozen at the deed, `/anonymous`,
    the admin strike, caps per name, and admins told about the commands in the changelog.
17. **Tone at scale.** Hundreds of new lines from several authors can break voices, mock the losing side or a dead
    player, or leak the Long Table. Mitigation: one lint with a baseline that only shrinks, the simulator's repeat
    and share checks, minimum pool sizes falling back to plain lines, the three humour tests in every text commit,
    and a never-found Long Table season tested for silence.
18. **Built rules change**, and each needs its doc in the same commit: the season's winner; life-stamped leader
    memory; the forgiveness budget and credit rule; the Wildcard search and toll; disguise doubling and
    `IsDisguisedAs`; the `BastionContact` hold; the trial-dose gate; Make it right; the epilogue's 168 hours; Take
    the Lab moving into the finale; the 25 missions leaving the pool; the gate news.
19. **Spoilers.** `.rzd` text and generated packs ship in the addon, so lies and twists can be read from a client's
    disk. Mitigation: truths are server state, drawn per season with honest variants, and optional `$profile`
    overrides keep twists server-only.

---

## Docs

Every phase updates its docs in the same commit as its code. Decision 16 adds four new docs, each only when its
system is built, and each with CLAUDE.md's Doc map and the README's doc list in that commit:

| New doc | Added in | Owns |
|---|---|---|
| `docs/WRITING.md` | Step 0 (with the lint) | The writing guide, moved out of LORE.md, and tone's rules |
| `docs/INFECTED.md` | Tribes phase 1 | Crawlers, tribes, day parties, the land, bodies |
| `docs/DIALOGUE.md` | Phase 2 | Conversations, checks, tags, memory, lies, scenes |
| `docs/FINALE.md` | Phase 6 | Routes, the lock, the stand as a control fight, terms |

**Everything else lands in the docs that already own the topic:**

| Design | Lands in |
|---|---|
| [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | REPUTATION.md (names, the credit rule, the budget, the parley), WRITING.md, DESIGN.md, UI.md |
| [DISGUISE.md](DISGUISE.md) | REPUTATION.md and DESIGN.md (the look, catches), WORKBENCH_SETUP.md |
| [MISSION_STYLE.md](MISSION_STYLE.md) | DESIGN.md ("Story blocks"), MISSIONS.md (the block format, the 15 drops, the merges), QUESTLINES.md |
| [ERRANDS.md](ERRANDS.md) | DESIGN.md ("Errands"), MISSIONS.md ("Errands") |
| [MAJORS.md](MAJORS.md) | QUESTLINES.md (no new doc), DESIGN.md (questlines, the fallbacks, the budget), LORE.md |
| [EPILOGUE.md](EPILOGUE.md) | DESIGN.md ("The season's end"), MISSIONS.md (the closing work, ✅ only when in code), LORE.md's Endings |
| This plan | ROADMAP.md ("Next", play-test priorities, and the Dropped list) and README.md (status) |

**Every phase also updates** UI.md and UI/LAYOUTS.md for any screen or widget a script looks up, WORKBENCH_SETUP.md
(settings, the VERIFY count and prefab list, log lines, the play-test checklist), and ROADMAP's Dropped list (for
example the 15 dropped missions, Grounds, and the earlier finale draft's route Fame bonus), each with one line of
reason. Nothing designed here is written into `docs/` as built until it's in code; until then the docs say
"planned".
