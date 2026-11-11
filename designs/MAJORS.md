# The four majors: what's left

**Status: built in part (phase 5), simplified.** Approved by the author on 2026-11-11. The engine
(the major lane, acts, `ActEnd`, `RZ_QuestVote`, set pieces on the calendar, the dynamic fallbacks,
the pay caps and the budget's majors lane, the season story record, the story rules next season, the
stand's inputs and events, the epilogue lines by controller, the hall's lines and the season tab) and
the four majors' acts, defaults and endings (with Proof's Before the stand) are built and live in [docs/QUESTLINES.md](../docs/QUESTLINES.md#the-majors) and
[docs/DESIGN.md](../docs/DESIGN.md#the-majors). This file keeps only what is designed and not built yet.
Each item names the major and act it belongs to; the built tables in QUESTLINES.md say what each act
does today.

**Contents**
- [Shared engine still to build](#shared-engine-still-to-build)
- [The Book Has to Balance](#the-book-has-to-balance)
- [Await Relief](#await-relief)
- [Every Name](#every-name)
- [Proof](#proof)
- [Last Rounds: the codas in the epilogue week](#last-rounds-the-codas-in-the-epilogue-week)
- [The other questlines' crossings](#the-other-questlines-crossings)
- [Guardrails that still bind the rest](#guardrails-that-still-bind-the-rest)

---

## Shared engine still to build

- **Story rules' last pieces:** the Book balancing's crew-name greeting for a returning player (needs
  crew names), Counted's one Bastion checkpoint on a safehouse town's road (needs road posts placed by
  town), Paid at the Door's first-day exile leak at 30% (needs the exile leak, below).
- **Stand events:** an event when the clean room changes holder (`OnFinalStandStart` and
  `OnFinalStandEnd` are built).
- **Leaders and scenes:** `SpawnLeaderAt(gm, leaderId, pos)` stand-ins in the leader's own body
  (today the scenes stand civilians in for them); run claims on seats from a summons to the tally;
  an "Ask about..." choice on other factions' stronghold leaders while a stage is open.
- **Channels:** `OpenChannel(faction, lines, gapSeconds)` for two-minute readings (the director's
  paper, the chair's List); `SecureLine(faction, text, hours)` for Radio Operators; a one-off line at a
  set time (Bastion's 20:30 call); a one-night mute of a faction's open channel.
- **Profiles:** a crew name for the life, a Bastion rank text (*"Sergeant {name}"*), and a per-life
  "on the roster" flag (the operator reads the name with Bastion's dead).
- **Optional props:** **Questlines > Red Hand lorry** and **Bonfire prop** (WORKBENCH_SETUP.md entries
  and the VERIFY count), at the send-off, the fire and the last line.
- **`tools/pacing_sim.py`:** the 60, 70 and 80% milestones and the order check from 2 to 20 regulars
  (Aftermath comes on or before 60% from about 7 regulars, which the second sitting's rule covers).
- **Errands' margin cards** (2–4 per major, never on a set-piece evening at its place), dealt first
  between beats ([ERRANDS.md](ERRANDS.md)); Crash Site reading `majors.pages_open`.
- **The runtime test's other checks:** stalled research leads a climax by at most 5 points;
  Aftermath before 60% makes the second sitting wait for the muster; a set piece nobody qualifies for
  runs as data after two free evenings; the gate resolves an open climax at half effect.

## The Book Has to Balance

- **Act 1:** stand-ins for each crew boss at the crossroads; a crew boss counts only if the hold
  stands; a Ruthless or Paranoid crew boss needs Trusted; Loyal ones come on their own; with two or
  more crew bosses and the hold: the boss remembers you helped. The noise of horns and shots as a
  heat source (VERIFY: a sound event or a radio prop).
- **Act 2 (R1 stages 1–2):** the thief's one motive (his sister's house, the rival's tin of diesel);
  persuading him out names who paid him (`book.rival_known` *(you)*).
- **Act 3 (R1 stages 3–6):** a one-session wait between stages 3 and 4; a headline per line dealt
  with; `door.debt_yours` adds your own line, and with `haven.rule` 4 the paying houses are on the
  page; burn it at the chair's table (30 Haven Favor, not once the List is read); sell it to the
  rival (40 Red Hand Favor, `book.to_rival`); forge it at the barn (60 s unseen; spotted 35% without ★
  Police or Ex-Con; `book.forged`); taking the thief's place in the ring gives a crew name (`book.named`).
- **Act 4 (settling day):** the rival's crew picked again if he leaves the chair; the forgery and the
  real page in his hands; "keep it" brought home at half the Fame; own it or blame him when it's
  burnt or with Bastion; raising R2's `cargo.cut`; the Long Table's jar money (`cargo.buyer` 1,
  persuade 2: C1 starts at stage 4); the raid's outpost (the season's one major outpost, Bastion
  stronger by 10); stand with him as a real fight with the Red Hand group (Rogue and Rein In on a win);
  slip off with the real page in the noise.
- **Act 5 (the last page):** first refusal (30 minutes for whoever decided settling day); one mission
  shared with Authors' stage 4; the bill as a carried delivery with two doses moved (Steal the
  Antidotes' transfer) and the lab road at the stand, needing Trusted with Halcyon after a turn-down;
  the sale by the page as three deliveries in 60 minutes with a courier's "last seen" and lines taken
  off the carrier; `weeks.public` makes the bill one step harder and halves the director's price;
  `proof.pages` 5 (the lab's line in the book) settled by each ending; the book lifted at supper with
  no disguise at the table, +25 Infamy, and the act staying open if caught.
- **Endings' extras:** crew names and ×0.9 at the boss's quartermaster for the deciders; +10 Halcyon
  Fame for the bill's carrier; the line-by-line effects of Sold by the page (leaders +15 loyalty,
  that faction's morale +3, where each line goes by stance and rule); Sent off loud's item for the
  persuader and every page leader remembering the participants as having saved them; Bastion's file's
  outpost, the commander's +25 loyalty (or the stove under `orders.stove`), the nightly call line,
  Red Hand Talk checks one tier harder and the opener *"You're the one who gave Bastion the book."*;
  The book doesn't balance's Rogue swing.
- **Crossings:** R2 and R3 pick the rival's crew while the major runs; a Rein In on the rival counts
  as pulled into line; between beats, the book's line for "the crew in the woods" (the tribes).

## Await Relief

- **Act I:** time pressure (an operator logs "Command answered" if nobody compared the tape by the
  next call: morale +6, then −8, `orders.false_hope`); ★ Hunter or Police grid references on the
  runner's road; buy the satchel at a crew's fence (15 Red Hand Favor, `orders.rh_knows`); report the
  commander to the duty officer (`orders.reported`; a squad races you to the body, else the stove and
  act II skipped). The giver rule's "nobody qualifying met him in 2 sessions: any commander".
- **Act III:** two men who walk off are struck off as "KIA. Body not recovered" (nobody answers for
  them at the roll; Struck Off weighted up at their post for a season day).
- **Act IV:** a per-post padding pick from what the world has (a deserter struck off, rifles off the
  inventory, an unwritten contribution); report it (Rein In's terms) or cover for the commander
  (`orders.padded` +1); the disguise roster trick (a dead man's name off the nail as cover, two looks,
  the muster's call-back); the fence's rifle count (20 Red Hand Favor); "already counted" posts (R1's
  `book.bastion`, `haven.counted`, B2's Counted, `list.to_bastion`); "and the lab knew"; The Last
  Convoy run for the weakest post (`RZ_StoryArcs.StartFor`). At most two seats moved.
- **Act V (the muster):** collecting every copy before "keep the call" (bribe the clerk 30 Favor, the
  crew's recording past two lookouts, your own `orders.copy`; +5 Fame for the last one); walking the
  chair's clerk to the radio after curfew; Bastion, Command's curfew walk in a Haven town (Curfew
  Patrol); the commanders answering as text from their seats.
- **Endings' extras:** Sedlak writing the decider on the roster; the presence floor (a quarter) under
  The Call Stands; the kettle line on air every night; All Stations' two struck-off men and no curfew
  under Counted; Bastion, Command's rank, curfew and the Rogue pulled into line; Expended's Deserter
  wanderers, one silent night while the Red Hand reads "expended" hourly, the Census unable to start
  and the Haven's cards lapsing; HC3's terms settled by the stance at the stand.
- **Act VI (the coda):** the escort with Sedlak and one Bastion group (only within 400 m of a player;
  Sedlak can't be killed), the Haven walking first (All Stations or `orders.haven`), Deserters
  handing in their tags to answer "Here" (and `{count}` +1), Marek Fiala answering if the finale's
  route brought him home, the captain and the four R numbers (the finale's lore), "Continue." after
  "Expended." with the lab's log in Bastion's file.
- **Readers of the stance:** `RZ_LeaderVoice` and `RZ_FactionVoice` (the call's opening), the presence
  floor, this season's curfew through `RZ_Seasons`, B2 (the call's wording, the forged-count risk,
  can't start after Expended), B3 (talking him down one tier harder under 2 or 4), C1 (how fast an
  exposed house is burnt), No Reply losing its "Command answered" choice once a stance is set.

## Every Name

- **Canon to LORE.md with it:** the Long Table's dates a day earlier (the truck missed on Day 6, the
  radio Day 7, Mr Halas dead on Day 8, the bite found on Day 9, the house crossed off on Day 10); the
  chair's fixed backstory line (*"Forced the doors of the school gym on Day 2, for a busload the line
  had turned back."*) in `RZ_LeaderVoice`.
- **Act I:** the family as generated people at the house's edge, the cough a chest cold for ★ Nurse
  or Doctor, the bear on the door (a line in the news) if the house had no children; the two walked
  back left on the road as data the Long Table's takings can find.
- **Act II:** the council's stand-ins at the stronghold safehouse (within 300 m) and the run's claim on
  it and the voting heads; leans from loyalty, the house's food, act I and memory of players in that
  order; a lean line per voter when you talk to them; persuading a head who leans the other way; ★
  Elder hearing who each head would call a mouth; asking Bastion's or the Red Hand's stronghold
  leader about the council; ★ Radio Op hearing what each offer is for (repeating it takes two heads
  away); offers tabled by `door.tins`, `door.paid`, `walk.cut_dose`, `door.bastion`; the betrayal (a
  crew raids the gate at supper; hold it or let it happen).
- **Act III (H1 stages 1–4):** the dissenting head as the giver (*"Ask {chair_short} about the farm."*);
  under workers first the husband crossed out this week; one cross that doesn't fit (*"Leave that
  one."*; the finale's woman); the radio log at night (the whole truth, and the winter store,
  `every_name.store_found`); the bus driver (`every_name.witness`); the boy of ten (`every_name.boy`).
- **Act IV:** carrying 4 food from the best-fed house within 90 minutes (and the Red Hand's raid on
  the cart, or 25 Red Hand Favor), riding the Haven truck, Fortify the Safehouse's Barricaded flag,
  sneaking two mouths into another house at night, handing the stranger to the crew's drop
  (`door.exile_sold`).
- **Act V (the second sitting):** the sitting scattering when the hold is lost (twice at most); the
  chair reading it (`list.chair_read`); gathering the others' pages before supper (page six and its
  eleventh line, the Red Hand's strays, Bastion's Day 3 page with `orders.haven`; or the director's
  offer to strike page six); the winter store opened at supper.
- **Act VI (A Week On):** each ending's full scene (Local Law, the far house's people walked home,
  Curfew Patrol, the collection's warnings, raids and feuds), the pot lapsing a day if it fails, the
  Closed Book's lasting risk coming due.
- **Act VII (the Gate):** escorting the houses' people (at most 3 groups), reading a name of one of
  your own dead characters, the woman's name read last if the finale found her, a Bastion officer
  under Counted and a Red Hand lorry under Paid at the Door, the walker reading his eleven
  (`walk.walks_stay`).
- **Endings' extras:** the deciders' shelf item, the asking head remembering them, the chair
  remembering a reading over the chair's head; Every Name's +1 person at two houses and Halcyon −3
  for page six; the Closed Book's lasting risk; the Far House's people moving, upkeep ×0.9, leavers
  on the roads, trucks to the far house last, walk-offs ×1.25, The List ×1.2; Counted's +2 food, the
  contribution and curfew, B2 counted as done; Paid at the Door's tins, the 20% chance a session of
  being found out, paying houses losing food, the leak at 35%.
- **Readers of `haven.rule`:** `RZ_Safehouses` (the pot's transfer, upkeep, `FindFedSafehouse`,
  stragglers, walk-off, paying houses' food, `MovePeople` within the floor and cap), `RZ_SupplyLines`
  (the far house last), Requisition (one weight per house, shared with B2 and the stance), Protection
  Racket and Red Hand raids skipping paying houses, the exile leak at 25–35%, `LapseRule(realHours)`.

## Proof

- **Act I as Patient Zero's Ledger** handed in at HC1 stage 4's seats; while it runs, Patient Zero's
  Ledger and Patient Zero aren't offered on their own; the Red Hand's reading on the open channel
  (30 Red Hand Favor); both Haven and Red Hand making it public with "Halcyon made this" for 8 hours.
- **Act II:** HC1's new choice (keep the director's three pages, `proof.pages` 2 *(you)*, noticed 25%,
  40% for a Paranoid director); the site's tribe kept 150 m from the store; Authors waiting while HC1
  is engaged, up to 55%.
- **Act III:** HC3 losing its 90% cut-off and reading the finale for its ending; its stage 3 answer
  provisional until the muster; the CO's brief mentioning the clinic file or the notebook.
- **Act IV (the reading):** stand-ins seated by the season's flags (the duty officer, the field lead,
  the chair's clerk, the adjutant, the boss's man); the director's two minutes on the open channel
  as a reading; the leverage's 25% (40%) chance the lab hears; ×0.9 at the quartermaster for standing
  with the director.
- **Before the stand's rest:** the swap after 20:30 only, in a Halcyon coat (the duty officer the
  watcher, a notebook number the cover), a ★ Lab Tech finding the draft at once, +8 Halcyon Infamy if
  seen; +10 Red Hand Infamy when seen lifting the pages; the Red Hand's last page reading the lab's
  line in the book (`proof.pages` 5).
- **Act V:** the author list by the finale's bystander rule.
- **Act VI (the Paper):** who carries the case (the adjutant, the field lead, or a beak courier);
  the author list with the Day 6 cohort and, with tribes, the tagged leaders; the radio ham's *"Got
  it."*; siding with the Haven (the author list read at supper); putting the pages in the case; the
  Red Hand crew on the road and a dead carrier's page (Two Tins a Copy off the road).

## Last Rounds: the codas in the epilogue week

The four codas run today as one scene each on the epilogue's evenings, in order, and as data when the
epilogue's last day nears ([QUESTLINES.md](../docs/QUESTLINES.md#the-majors)). Still to build, with
[EPILOGUE.md](EPILOGUE.md)'s calendar:
- **Standing versions** until the wipe: the List pinned on every house board and a clerk who reads it;
  the last call's transcript on the roster board and an operator who plays it back; the tags back on
  the nail and "AT THE BARRIER" on the board; the case on the table and the lab gate's notice; the
  lorry with the book nailed open and a lad on a kitchen chair who reads you your line.
- **Second airings:** words only on the faction's channel, on the next free evening at least 8 real
  hours later (the epilogue schedules them).
- **The last call at 20:30 as radio** (not taking the evening's slot), and the walk at the next 05:30.
- **Bodies:** escorts only while `PerimeterSealed()` is false, never into the turn-back zone, at most
  3 escort groups, only within 400 m of a player.
- **No cure:** the Haven's Gate at the first wipe warning, held at the stronghold safehouse; Bastion's
  last call in the last 24 real hours; the Red Hand's major ending at the wipe warning on its current
  branch; Proof Unsigned (built).
- **The other closing work** meeting the codas: Bring Them Home's tags in Sedlak's box, Settle
  Accounts' "Paid" at the last line, the Walk Out never sharing a coda's hour, the Last Broadcast
  reading what the majors put in the open.

## The other questlines' crossings

| Questline | Still to build |
|---|---|
| H2 The Price of the Door | Its flags table offers and tilt one head's vote at both sittings; `door.exile_home` as proof at the reading; its house as act IV's house |
| H3 The Edge of Town | `walk.walks_stay`: the walker reads his eleven at the Gate |
| Short Weight | Full Weight leans that head to the pot, Short Weight to the arrangement; starts twice as often during the Haven's acts II–IV |
| B2 The Census | Counted and Forged feed the muster's count; reads the stance back; can't start after Expended |
| B3 The Last Pilot | `pilot.line`'s name read in the last call; `pilot.stays` flies Bastion's group in (words) |
| R2 Cargo, R3 The Well Story | Pick the rival's crew while the major runs; `cargo.freed` as proof at the Haven's reading; `well.stood` against the arrangement |
| HC2 The Other Two | Its flags put names or numbers on the paper, ease the director's persuade, put page six in the open at the Haven's reading |
| C1 | `table.walked_out` as proof at the second sitting; how fast an exposed house is burnt by stance |
| Contracts | No Reply loses "Command answered" once a stance is set; Struck Off never uses the finale's canon name (Fiala) |

## Guardrails that still bind the rest

- **Sealed Zone.** Command never answers. Every coda stops short of the barrier (150 m, the table at
  about 110 m). Nothing and nobody crosses; the case stays on the Zone's side until the gates open.
- **No sleep, no time skip.** Every gate is a milestone, a stage, a game-clock hour or a played wait.
- **Strongholds and their leaders** are never replaced or killed; their traits are only read. Settling
  day's coup always fails; at most one crew walks away; the faction never splits.
- **Leaders and outposts:** each seat moved at most once a season, two per major, three per faction,
  never past one Rogue per faction; one outpost a season through the majors.
- **Nothing overpowered:** every effect inside the budget's lanes and the pay caps; defaults pay
  nobody; no research beyond +3 per major; at most two of the majors' groups at the lab.
- **Light carry-over:** one small story rule per major, only after a cure; the codas carry nothing
  over but hall-of-fame lines.
- **Performance first:** majors are data; bodies only near players (council stand-ins within 300 m,
  crew-boss stand-ins, holds and escorts within 400 m); props leave no trace.
- **Tone:** nobody is a villain in their own story; a stronghold leader never jokes about their own
  page; Rudi's end is never a joke and the lie about it is said flatly; the Long Table is never
  described.
