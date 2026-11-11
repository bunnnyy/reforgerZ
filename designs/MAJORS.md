# The four majors

**Status: designed, not built.** Approved by the author on 2026-11-11.

One season-long story per faction. Each climax lands on a cure milestone, each stronghold leader
has one page they'd rather burn, and every page put in the open makes the next one easier to read.
The majors grow out of five built questlines (H1, B1, R1, and Halcyon's HC1 and HC3), which they
absorb. The other questlines stay as smaller stories beside them.

**What changed in review**

- **Decision 10 (changed by the author): the fallbacks are dynamic.** No major has a fixed day.
  The four real-day fallbacks (Bastion's Act I "by real day 3", the Red Hand's act 2 "by real
  day 5", settling day "or real day 5", the last page "or real day 8") are gone, and so are the
  season days 13, 22 and 35 that conflict 0 had put in their place. Every default now fires from
  live signals: research pace, the stage, the faction's state, whether players are engaging, and
  the time since the last beat ([Dynamic fallbacks](#dynamic-fallbacks)). In-act windows count
  offered time, not calendar evenings.
- **Conflict 0: the 45-day pacing.** The acts are moved off the old 10-day season. The send-off
  is on Collapse's second evening. The season table uses simulated days for 2, 5 and 15 regulars.
  The majors' milestones (60, 70 and 80%) were run through a copy of `tools/pacing_sim.py` for 2
  to 20 regulars. The order 50% → 60% → 70% → 75% → 80% → 90% holds everywhere, but from about 7
  regulars Aftermath comes on or before 60%. So the Haven's second sitting now also waits for
  Bastion's muster. On 2 and 3 regulars Aftermath comes at or after the gate, so the sitting uses 70%.
- **The majors finish before the gate** ([FINALE.md](FINALE.md), §12). Every climax and the Red
  Hand's last page must be settled by 90%. The season-end resolve now fires at the gate, not at
  the final stand.
- **Conflict 1: the finale owns the stand.** The Red Hand's act 6, Proof's Act V, the stand hooks
  and the guardrails no longer say the stand and the winner never change. Control of the clean
  room picks who has the cure. Under a finale, Take the Lab is a holder change, not a stolen
  batch. Proof's author list uses the finale's bystander rule. HC3's ending reads `finale.how`
  and `finale.route` instead of the batch counters.
- **Decisions 4 and 6.** Fame from the majors counts toward season Fame like any other Fame.
  Season Fame decides the hall of fame's "Zone's Fame" line, who the cure is handed over to when
  nobody holds the room, and the Fame leader's next-season rules at half. Take the Lab stays the
  Red Hand route's once-a-season theft: any Red Hand-locked player who is Trusted with the Red Hand
  can start it (conflict 11), and every Red Hand-locked player counts for the bay hold.
- **Conflict 2: struck off.** The two men who walk off (All Stations, and B1's wait after "The
  men were told") are struck off as "KIA. Body not recovered". Nobody answers for them at the
  roll, and Struck Off is weighted up at their post for a season day. Counted's call reads "List
  to follow", not "Names attached". Expended's Hunt the Deserter weight becomes Struck Off's,
  because the missions design merged the two.
- **Conflict 17: the finale gets its own budget lane** (morale ±10, strength ±5 per faction).
  The budget table now shows all four lanes.
- **Conflict 36.** Our Crossroads, Coup Talk and Training Day leave the random pool but keep their
  classes as questline stages, so the Red Hand's acts 1 and 4 still reuse them. The old line "the
  rival doesn't offer Coup Talk during the major" is gone, because nothing offers it now. Training
  Day stays a Bastion leader job.
- **Decision 2: the finale's lore is confirmed**, so the majors carry its clauses: the woman's
  cross on the List, Sedlak on the Day 1 cordon detail, the captain and the four R numbers at the
  last roll, the cook's line on Rudi's first page, and the ham who writes the numbers down
  ([The confirmed lore](#the-confirmed-lore-in-the-majors)).
- **Decision 3.** Private Marek Fiala (Twice) answers "Here" at Bastion's last roll if Bastion's
  route brought him home.
- **Decision 12: the epilogue lasts a week.** The four codas are the closure set's "Last Rounds".
  All four are playable now (the Red Hand's last line gains a scene: the boss closes the book,
  paid or not), Bastion's walk ends in the last roll at the barrier, and each runs on the first
  free evening with someone online. None of them carries anything into the next season except
  hall-of-fame lines, so Proof's next-season rule is fixed by the paper as it stood at the cure.
- **Decisions 11, 13 and 15.** Persuade uses the built fixed score with your access rank (mixed
  tiers one lower). Dialogue is off by default, so every sway, ask and persuade also works from the
  action menu. "Proof" stays the major's name; Halcyon's finale route is The Lower Store.
- **The paper's table moves from 75 m to just outside the kill zone** (about 110 m). The built
  checkpoints' barriers stand at the map edge, the kill zone runs 100 m in from it, and after the cure
  the built line turns back anyone who walks into it, so a table at 75 m couldn't be reached.
- **Small fixes.** Crew names follow LORE (the crew names you after something you'd rather it
  forgot). Every Name's "read it out" works with either result of Act III (it said both "only with
  `list.truth` 1" and "one harder with `list.truth` 2"). The other approved designs add three things
  here: Await Relief's Act IV gets the disguise design's roster trick, the tribes design's hooks
  land on the absorbed lines, and the errands' margin cards fill the gaps between beats.

**Contents**
- [The four majors in one page](#the-four-majors-in-one-page)
- [The season](#the-season)
- [Rules every major keeps](#rules-every-major-keeps)
- [Dynamic fallbacks](#dynamic-fallbacks)
- [The Haven: Every Name](#the-haven-every-name)
- [Bastion: Await Relief](#bastion-await-relief)
- [The Red Hand: The Book Has to Balance](#the-red-hand-the-book-has-to-balance)
- [Halcyon: Proof](#halcyon-proof)
- [Last Rounds: the codas in the epilogue week](#last-rounds-the-codas-in-the-epilogue-week)
- [The confirmed lore in the majors](#the-confirmed-lore-in-the-majors)
- [The other questlines](#the-other-questlines)
- [Engine needs](#engine-needs)
- [Guardrails](#guardrails)
- [Docs in the same commit](#docs-in-the-same-commit)

---

## The four majors in one page

| Major | Faction | Grows out of | Climax | The page the leader would rather burn | The question |
|---|---|---|---|---|---|
| **The Book Has to Balance** | The Red Hand | R1 The Book (key `book`) | **Settling day**, 50% | The book's back page | Who holds the book, and what will the Red Hand be owed when the gates open? |
| **Await Relief** | Bastion | B1 Await Relief (key `orders`) | **The muster**, 60% | The Day 3 orders | What is Bastion, now that it knows nobody ever meant to come? |
| **Every Name** | The Haven | H1 The List (new key `every_name`) | **The second sitting**, Aftermath or 70%, after the muster | The List | When there isn't enough, who is the Haven for? |
| **Proof** | Halcyon | HC1 The Three Weeks (`weeks`) and HC3 Authors (`authors`), as inner lines | **The reading**, 75% | The three site-log pages | What goes on the paper's first page? |

The climaxes come in that order, each one stage of the cure after the last. Each page put in the
open makes the next leader's "read it" one step easier, once (`majors.pages_open`). A Red Hand
book read whole at the crossroads helps Bastion read its orders. Bastion's All Stations helps the
Haven read its List. The List read at supper seats the chair's clerk at the lab, and lets the
director write the whole of it. A page kept, sold or burnt closes those doors.

After the cure each major has a closing scene at the north road, where the Day 2 column was
turned back: the List read towards the barrier, Bastion's last call and the last roll, the paper
left on a table, and the boss closing the book at Rudi's crossroads.

---

## The season

The majors run on milestones, stages and play, never on dates. The days below are the medians of
60 simulated seasons on the built pacing (`tools/pacing_sim.py`, with the majors' milestones
added). The dynamic fallbacks move a beat by an evening or two either way.

| Beat | 2 regulars | 5 regulars (design point) | 15 regulars |
|---|---|---|---|
| Hours with someone online, a day | 4.4 | 7.7 | 10.8 |
| **Early Outbreak.** The Haven's family at the first dusk. Halcyon's Day One opens | day 1 | 1 | 1 |
| **Collapse.** Bastion's "reply received". The Haven's summons and first sitting. The Red Hand's send-off on Collapse's second evening, then the torn page | 9 | 6 | 4 |
| **25%.** Day One closes; The Three Weeks | 15 | 13 | 11 |
| **Overrun.** The hungry week. Bastion's count | 27 | 16 | 12 |
| **50%.** **Settling day** (the Red Hand). Authors starts | 27 | 24 | 20 |
| Aftermath | after the gate | 31 | 23 |
| **60%.** **The muster** (Bastion) | 32 | 28 | 24 |
| **The second sitting** (the Haven): Aftermath or 70%, whichever first, after the muster | 37 (70%) | 31 (Aftermath) | 25 (the evening after the muster) |
| **75%.** **The reading** (Halcyon) | 39 | 33 | 29 |
| **80%.** The Red Hand's last page. A Week On. Before the stand | 41 | 35 | 31 |
| **90%.** The gate. Every major's climax is settled | 45 | 39 | 35 |
| The finish, the batch, the stand, the cure ([FINALE.md](FINALE.md); about 5 days) | about 50 | about 44 | about 40 |
| **The epilogue** (one week): the codas on its first evenings | 50–57 | 44–51 | 40–47 |

**What is at stake.** Every stronghold leader has a page they'd rather burn:
- the boss: the book's back page;
- the CO: the Day 3 orders;
- the chair: the List;
- the director: the three site-log pages.

**How the majors feed each other**
- **The Haven's first sitting** (Collapse) sets `haven.rule`. Bastion's muster reads it: under
  the cards, "civilians sheltered" is true. Under the arrangement, the Red Hand's book gains a line
  for the Haven's paying houses.
- **Settling day** decides whether Bastion's guns raided the rival's camp. If they did, the boss
  won't risk selling Bastion's orders at the muster.
- **The muster's stance** decides:
  - whether the Haven's cards survive;
  - whether Bastion's adjutant sits at Halcyon's table, and whose name it signs in;
  - which Bastion group stands at the lab, and whether the Red Hand's road group is warned off.
- **Halcyon's reading** can put the director's pages in the boss's book. The Red Hand's last page
  settles them a few days later: forgiven in the bill, sold back, burnt with the book, or handed to
  Bastion. Sending the book off loud also ends the Haven's arrangement.

**At the stand.** The finale owns the stand ([FINALE.md](FINALE.md), §9). The majors only decide
how their factions turn up: Bastion's group by its stance, the Red Hand's lab-road group or its
theft by the last page, the Haven's group at the gate by `proof.haven_gate`. At most two of the
majors' groups come (the finisher's faction first), plus Halcyon's guard. What each ending changes
in the finale's routes is the finale's to say (its §12).

**The epilogue** comes back to the north road. On the first free evening the chair reads the List
150 m from the barrier. That night Bastion's last call goes out, and at dawn Sedlak walks the tags
to the same barrier and reads the last roll. At the next change of shift the lab's paper is left
on a table just outside the kill zone. Then, at Rudi's crossroads, the boss closes the book. The four
endings go into the epilogue line and the hall of fame's season block and, after a cure, four
small capped rules for the next Zone. Nobody behind the concrete ever answers.

---

## Rules every major keeps

**Lanes and starts**
- Each faction has one major, in a lane of its own: outside **Max active** (2) and outside the
  one-line-per-faction rule, so a faction's smaller stories run beside it.
- At most two majors' stage missions are offered at once, server-wide. A story can't claim the
  same house or leader as its faction's major.
- A major starts with priority once its gate is met (no half-hourly roll), runs once a season, and
  never fizzles for good. It ships at a season wipe.
- Every act has a default. Defaults read the stronghold leader's traits or the rule in force, and
  **pay nobody**. When a default fires is the next section's job.

**The set-piece calendar and the season budget** are built (`RZ_SetPieceCalendar` and `RZ_MajorBudget`, with
their lanes, the seat ledger, the one outpost, research +3 per major and the season weights):
[DESIGN.md](../docs/DESIGN.md#the-set-piece-calendar), [DESIGN.md](../docs/DESIGN.md#the-season-budget). What the
majors add on top:
- A stage marked as a set piece holds the evening from its offer hour until it resolves (`Hold`, `Release`). One that
  falls due on a held evening, or while that faction's stronghold is Under Pressure, waits for the next free one, at
  most twice (`MustRun`), then runs at the next free hour. Nothing is skipped or slept through.
- A set piece runs live on the first free evening with a player online who qualifies for it. It runs as data only by
  the rules in [Dynamic fallbacks](#dynamic-fallbacks).
- Dialogue's public scenes hold up to 16 people for a sitting.
- **The seat ledger's majors part:** each outpost or safehouse seat is replaced or swung by at most one major a season
  (`TakeSeat` counts two per major and three per faction).
- **Truces and feuds** go through faction life's caps: one per pair, and a second start extends the first.
- **Safehouses:** food and people ±2 at up to three houses per call, never below the floor of 2 or past the cap of 30.
- **Standing.** A major's endings pay above the engine's per-call 10 Fame / 15 Infamy through a major-only path, up to
  25 Fame, 40 Infamy and 60 Favor per player per faction over one major. Across all majors in a season: at most 40
  Fame, 40 Infamy and 80 Favor per player per faction. This Fame counts toward season Fame like any other (decision 4).

**Season weights.** The helper is built (`RZ_MajorBudget.SetContractWeight(type, mult)`, saved with the season,
combined and clamped to ×0.8–1.2); every major's multiplier on a contract, a leader job or Requisition goes through it,
and the mission manager will read it with the one weight clamp (phase 3). Requisition keeps one weight per house, shared by `haven.rule`, B2's `census.pressed` and
`orders.stance`. A curfew from any source (Counted, Bastion, Command) is one curfew. At most three
season weights per major.

**Checks**
- **Persuade** uses the built fixed score (`TryPersuade`), never a roll: your access rank with the
  faction, one lower for a mixed tier (decision 11), ★ background +1, then the steps each act
  lists, kept between 1 and 4.
- **Sneak** uses the questline sneak (`m_Quiet`), the heat limit and the built stealth
  (`RZ_Stealth.SightRadius`). "Past two lookouts" means two stage lookouts who notice by that rule.
- **Bribes** use `TryBribe`. A reward item comes only from the giver's shelf at the player's tier.
- **Talk without the dialogue engine.** Dialogue is off by default (decision 13). Every sway, ask
  and persuade here also works as an action on the leader or stand-in, and votes stay tallies in
  `RZ_QuestVote` either way.

**The journal.** A "The season" tab every player sees: each major's current act, the rule or
stance in force, the voters' leans in words, and any last call ([below](#the-last-call)). Stage
missions are still offered only to players who qualify.

---

## Dynamic fallbacks

**The author's rule (decision 10):** no major runs on fixed days. Each fallback fires from what is
actually happening on the server, so a busy server gets its climaxes on time, and a quiet one is
neither left waiting nor rushed.

Every act still opens on its own trigger: a cure milestone, an outbreak stage, the act before it,
or something in the world. Its fallback is its **default**, what happens when no player decides
it. Defaults pay nobody. This section says when a default fires, and when a climax may come early.

### What the majors read

All of it is read on the existing questline tick. Nothing polls per frame.

| Signal | What it is | From |
|---|---|---|
| **Research position** | Worked research, and the lab's backlog | Built (`RZ_CureResearch`) |
| **Research pace** | Worked research gained a real day, averaged over the last 3 real days that had at least one session of play | New reading of built data |
| **Hours played a day** | Hours with someone online, averaged over the last 7 real days | Built (the stage clock; the pace by play's day log) |
| **Room left** | How much play is left before an act's anchor. For a cure milestone: (milestone − worked research) ÷ research pace × hours played a day. For an outbreak stage: the stage clock's hours still to go. "Lots" while research is stalled | New |
| **The stage** | The outbreak stage, and the hours played in it | Built (the Director) |
| **The faction's state** | Stronghold Under Pressure; morale; strength; the faction's need; its leaders and their traits; the rule or stance in force | Built (faction life, leaders); the majors' flags |
| **Engagement** | For each act: its offered time, its touches, and how many players have touched the major this season | New, kept in the run's flags |
| **Beat gap** | Hours played since the major's last beat (an act opened or ended, a set piece, a default) | New |
| **The calendar** | Which evenings are held, and by whom | The set-piece calendar |

**Words used here**

| Word | Means |
|---|---|
| **Session** | 2 real hours with players online (built) |
| **Qualifies** | A player who could decide one of the act's approaches now: the tier, background and flags it asks for, not exiled, not hunted by the giver |
| **Offered time** | Play time while the act is in the offer list (one of the two major slots) and a player who qualifies is online. An empty server adds nothing. Neither do hours when only players who can't take it are on, or hours while the faction's stronghold is Under Pressure |
| **Offered evening** | A set piece's offer window (for example 16:00–21:00 game time) on a free evening, with a player who qualifies online for at least 30 real minutes of it |
| **Touch** | A player took, finished or gave up one of the act's stages; carried an offer; swayed a voter; talked to the act's people on its topic; or stood in its scene |
| **Engaged** | A stage of the act is running, or the act was touched in the last session of play |
| **Anchor** | The beat the act must be settled before: its major's climax, another major's climax, or the gate |
| **Need** | Roughly how much play the act takes, in sessions or evenings ([the table](#every-acts-clock)) |
| **Stalled** | Worked research rose less than 1 point over the last 3 real days that had play, and the backlog is empty |

### When a default fires

An act's default fires on the first of these that comes true, once [the guards](#the-guards)
allow it.

1. **Squeeze: the anchor is close.** The room left before the act's anchor is less than this act's
   need plus the need of every act of that major not yet settled before the same anchor. The act
   gets its [last call](#the-last-call). Its default fires when the grace runs out, or when the
   anchor arrives, whichever is first. A squeezed act also jumps the queue: it takes one of the two
   major slots from an act that isn't squeezed. This is what keeps a busy server's climaxes on time.
2. **Patience: nobody took it.** Only for acts with a short natural window: set pieces, stages
   with a built give-up, and the few acts the table gives a patience. Patience counts offered time
   or offered evenings only. Every other act stays open until it's squeezed, so a slow server's
   players keep their story for as long as there's room.
3. **Nobody qualifies** (set pieces only). Two free evenings pass with players online but none who
   qualifies: the set piece runs as data at its hour. A server with no Bastion players still hears
   Bastion's muster at the right time, and it never holds the other majors up. Other acts don't run
   as data early, because a player may grow into them: they stay open, the squeeze settles them in
   time, and while nobody qualifies their grace counts plain hours played.
4. **The world settles it.** A trigger the act already has: the Haven's food, its morale under 45,
   a giver who has left the chair, a page that was burnt.

### The guards

A default never fires:
- **While the act is engaged.** A running stage always finishes. An engaged act can hold its anchor
  for at most one offered evening (a set piece) or one session (anything else). Then the anchor
  opens anyway, and the act takes its default for whatever nobody reached.
- **On an empty server.** Offered time and hours played stand still. Anything decided as data while
  nobody is on (only possible at a hard stop) is told the first time a player is in the world, in
  order, like the lab's breakthrough news.
- **While the faction's stronghold is Under Pressure.** A crisis is its own beat. The clocks pause;
  the hard stops still apply.
- **Twice for one major in one session** (the breather), unless it's squeezed. Defaults decided
  together, such as a run of acts on a server where nobody qualifies, are told one headline a real
  day, in order.
- **Out of order.** Climaxes keep their order: settling day, the muster, the second sitting, the
  reading, the last page. No default and no lead jumps another major's climax.
- **Off the calendar.** One set piece an evening, server-wide.
- **At a faction's low ebb, by patience alone.** A patience default that costs a faction morale
  waits while that faction's morale is under 30. A squeeze still fires it.

### The last call

No default is a surprise. When an act is squeezed, or its patience is down to its last offered
evening or session, it puts out a last call in its own voice: a leader's line, a rumour or a
headline. Then it gives a **grace** of one session of offered time, or the next offered evening for
a set piece. If nobody who qualifies has been on for two sessions of play, the grace counts plain
hours played instead. The journal's "The season" tab shows the last call.

Some last calls:
- the family at the edge of town: *"{head} says they'll be walked back to the road tomorrow at dusk."*
- the hungry week: *"{head} says the house will do what the rule says, tomorrow at dusk."*
- Bastion's count: *"The CO reads the count at tomorrow's call, with or without the {post} post's numbers."*
- the torn page: *"Somebody's selling a page at a barn. Not for long."*
- Day One: *"If nobody brings the clinic's file in by the change of shift, the field lead goes for it herself."*

### Lead: when research stalls

On a server whose research has stalled, a climax (or the Red Hand's last page) may open up to
**5 points** before its milestone, when all of these hold:
- every earlier act of that major is settled;
- the climax before it in the order has resolved;
- the major's beat gap is at least 3 sessions;
- a player who qualifies is online at the offer hour.

A lead never passes the gate, and never moves research. It doesn't apply to the 25% and 50% gates
of Halcyon's inner lines, which are built conditions. A busy server never stalls, so it never leads.

### Between beats

When a major's beat gap passes 4 sessions while its next act waits on a milestone more than 6
sessions of room away, the major fills the gap with what already exists:
- its faction's smaller stories start twice as often, and prefer the major's people as their
  crossings already say (the rival's crew, Act IV's house);
- its margin cards ([ERRANDS.md](ERRANDS.md)) are dealt first;
- one of its people says a line about what's coming, at most once a real day (*"The boss has been
  counting the book again."*).

Between beats never opens an act early (only a lead can), pays nobody and moves no vote.

### Hard stops

- **The gate (90%).** The majors finish before the gate, because the finale reads their flags there.
  Any climax or last page still open resolves on its current branch at half world effect, with no
  player rewards and no next-season rule (`ResolveAtSeasonEnd`).
- **The wipe's last 24 hours** with no cure: the same.
- What runs past the gate: the Haven's A Week On, Proof's Before the stand, the stand acts (which
  the finale reads), and the codas.

### Every act's clock

The Default column is what happens when a default fires by the rules above. "—" under Patience
means the act has none: it stays open until it's squeezed. Patience counts offered time or offered
evenings only.

**The Haven: Every Name**

| Act | Opens | Anchor | Need | Patience | Default |
|---|---|---|---|---|---|
| I. The Ones Who Can't Work | Early Outbreak's first evening (set piece); the family is back at dusk each evening | Collapse | 1 evening | 3 offered evenings | A Merciful or Idealist head takes all four; any other turns two away |
| II. The First Sitting | The summons; it sits at the second supper after it, with a session of offered time between | The hungry week (Act IV, in Overrun) | 1 evening | Sits on the first offered evening | The council sits as data |
| III. The List | The first sitting's vote, once a house has lost people or The List contract is done | The second sitting | 4 sessions | Each stage: 6 offered hours, offered again twice | None (knowledge only): open stages close |
| IV. The Hungry Week | Overrun's first supper after the first sitting; sooner at an upkeep floor or morale under 45 | The second sitting | 2 sessions | 3 sessions | The rule's own answer |
| V. The Second Sitting | Aftermath or 70%, whichever first; never before the muster has resolved; a session after Act III's last stage | The gate | 1 evening | Sits on the first offered evening; a lost hold scatters it twice at most | Votes as data |
| VI. A Week On | A house goes short, or the beat gap reaches 2 sessions | The stand won (then skipped) | 1 session | 3 offered hours | The ending's numbers stand |

**Bastion: Await Relief**

| Act | Opens | Anchor | Need | Patience | Default |
|---|---|---|---|---|---|
| I. Reply received | Collapse, after a nightly call. The Independent commander or the tape post gives it; if neither exists, any commander at once; if one exists and nobody who qualifies has met him in 2 sessions of offered time, any commander | The muster | 2 sessions | — | A stronghold patrol walks the runner's road; the page goes in the stove |
| II. Who reads it | After Act I | The muster | 1 session | — | The stove |
| III. The reckoning | After Act II and B1's one-session wait | The muster | 2 sessions | — | The branch settles itself at half effect |
| IV. Effective Strength | Overrun, a session after Act III, at a nightly call | The muster | 3 sessions | — | The count stays padded; a Paranoid or Ruthless CO writes up the worst post (words only) |
| V. The muster | 60% (or a lead); a session after Act IV; an evening after settling day | The gate | 1 night | 2 offered nightly calls | The CO decides alone, by Act III's result and the CO's traits |

**The Red Hand: The Book Has to Balance**

| Act | Opens | Anchor | Need | Patience | Default |
|---|---|---|---|---|---|
| 1. Send him off | Collapse's second evening (set piece) | Settling day | 1 evening | 2 offered evenings | The crews send Rudi off without players; the page is found missing the same way |
| 2. The torn page | After act 1 | Settling day | 1 session | — | The thief burns the page himself and runs (`book.burnt`, `book.thief` 5) |
| 3. Who gets the page | Straight after act 2 | Settling day | 3 sessions | — | The page stays out; the boss's men put the thief in the pit |
| 4. Settling day | 50% (or a lead); a session after act 3 | The muster (order); the gate | 1 evening | 2 offered evenings or 6 offered hours | Page in the book: the rival is laughed off. Otherwise he's purged half the time, and a real page in his hands comes back half the time |
| 5. The last page | 80% (or a lead); a session after settling day; after the reading | The gate | 1 evening | 2 offered evenings | The boss's traits decide |

**Halcyon: Proof**

| Act | Opens | Anchor | Need | Patience | Default |
|---|---|---|---|---|---|
| I. Day One | The first change of shift | 25% | 2 sessions | — | The field lead finds the file herself; Day 1 stays the lab's (`proof.day_one` 1) |
| II. The Three Weeks (HC1) | A session after Act I, or at 25%, or at once after Act I's copy | Authors' start: 50%, held to 55% at most while engaged | 4 sessions | HC1's own give-ups | Not started when squeezed: it gives up, because nobody asked (`proof.weeks` 0). Still open and untouched for 2 sessions after 50%: it ends at HC1's own default |
| III. Authors (HC3 stages 1–4) | 50%, once Act II has ended or given up | The reading | 3 sessions | HC3's own give-ups | A stage that fizzles counts as its default |
| IV. The reading | 75% (or a lead); never in a stand | The gate | 1 evening | 2 offered evenings | The director's draft stands |

The codas have their own clock in the epilogue week ([Last Rounds](#last-rounds-the-codas-in-the-epilogue-week)).

### Three seasons

**2 regulars: the quiet server.** 4.4 hours online a day. Ana plays the Haven and later the Red
Hand. Jiri plays the Red Hand and is Known with Halcyon. Nobody is Known with Bastion.

| Day | What happens | Which rule |
|---|---|---|
| 1 | Ana is on at dusk. The family comes in (Act I, talk). Day One opens | Natural triggers |
| 9 | Collapse. There's no Independent commander, so any commander gives "reply received" at once. Nobody qualifies, so it adds no offered time and sits quietly | Giver rule |
| 10 | A house runs short: the summons | World trigger |
| 10–12 | The send-off is offered on day 10, but Jiri isn't on at dusk. Day 11's evening is the first sitting's (Ana carries the pot). Jiri holds the crossroads on day 12 | Only offered evenings count; one set piece an evening |
| 13–18 | Jiri does the torn page and the page's fate. Day One is done on day 14 by the copy, so The Three Weeks opens at once | Natural triggers |
| 15–27 | The Three Weeks runs slowly, two sessions a week | No patience: it isn't timed out |
| 19–26 | The Red Hand's beat gap passes 4 sessions with settling day far off. R3 The Well Story starts at the rival's crew. A rumour every other day | Between beats |
| about 28 (53%) | Bastion's Acts I–IV (8 sessions of need) are squeezed before the muster. Nobody qualifies, so after the grace they settle as data together: the page goes in the stove, the reckoning at half effect, the count padded. One headline a day, days 28–31 | Squeeze; nobody qualifies; the breather |
| 27 | 50% and Overrun on the same day. Settling day runs live on the first offered evening. The Three Weeks is at stage 6 and engaged, so Authors waits | Milestone; the engaged hold |
| 29 | The Three Weeks ends. Authors opens | The hold ends |
| 30–33 | Ana spends four evenings on the hungry week and hands nothing in. Research stalls at 57% | Stalled |
| 33 | The muster leads at 57%: Bastion's acts are settled, settling day is done, its beat gap is past 3 sessions. Nobody qualifies, so after two free evenings the CO decides alone (The Call Stands, from Act III's result and the CO's traits) | Lead; nobody qualifies |
| about 40 (70%) | No Aftermath on 2 regulars: the second sitting at 70%, live. (Days from here run about three later than the table, because of the stall) | The sitting's gate |
| about 42 (75%) | The reading. Jiri misses the first evening; it's held again and he settles it | Patience: 2 offered evenings |
| about 44 (80%) | The last page. Jiri decided settling day, so he gets it first | First refusal |
| about 48 (90%) | The gate. Only A Week On and Before the stand are still open | Hard stop |
| the epilogue | The Gate on the first free evening. The last call runs as data (its transcript on the roster board). Ana walks the tags with Sedlak at dawn. The paper next, the last line after | Last Rounds |

**5 regulars: the design point.** 7.7 hours online a day.

| Day | What happens | Which rule |
|---|---|---|
| 1 | The family comes in; Day One opens | Natural triggers |
| 6 | Collapse. The Independent commander gives "reply received" | Giver rule |
| 7 | The send-off, on Collapse's second evening. A house is short the same day: the summons | Natural triggers |
| 9 | The first sitting: the second supper after the summons, with a session of offered time between | The sitting's rule |
| 8–14 | The torn page and its fate; Bastion's Acts I–III; Day One by the copy, then The Three Weeks | Natural triggers; the breather keeps one beat per major a session |
| 12–24 | The Red Hand's gap: R2 Cargo at the rival's crew, margin cards, a rumour every other day | Between beats |
| 16 | Overrun. The hungry week and Bastion's count open | Natural triggers |
| 24 | 50%: settling day, live | Milestone |
| about 27 (58%) | Bastion's count: two posts done, the third untouched. Squeezed: the last call goes out; a session's grace; the third post stays padded | Squeeze; last call |
| 28 | 60%: the muster, live (All Stations) | Milestone; an evening after settling day |
| 31 | Aftermath: the second sitting (the muster has resolved) | The sitting's gate |
| 33 | 75%: the reading. Nobody settles it the first evening; the second offered evening, they do | Patience |
| 35 | 80%: the last page. Sent off loud plays at the next dusk | Milestone |
| 39 | The gate | Hard stop |
| 44–51 | The epilogue week: the Gate, the last call and the roll, the paper, the last line on its first three or four free evenings; the rest of the week for the other closing work | Last Rounds |

**15 regulars: the busy server.** 10.8 hours online a day.

| Day | What happens | Which rule |
|---|---|---|
| 1 | The family is decided within minutes | Natural trigger |
| 4 | Collapse; "reply received"; a house short: the summons | Natural triggers |
| 5 | The send-off | Collapse's second evening |
| 6 | The first sitting | The sitting's rule |
| 5–9 | Bastion finishes Acts I and II in one evening; Act III waits for its own session anyway. The torn page is done the night after the send-off, not the same night | The breather |
| 5–12 | The Red Hand and Bastion hold the two major slots. The Haven's Act III waits in the queue without losing anything, because offered time only counts while it's offered | Offered time |
| about 19 (48%) | The page's fate (act 3) has sat untouched for days. Squeezed: *"Somebody's selling a page at a barn."* A player takes it that night | Squeeze; the act jumps the queue |
| 20 | 50%: settling day is due, but the thief is in the pit (act 3's last stage is running). Settling day waits one offered evening | The engaged hold |
| 21 | Settling day | — |
| 23 | Aftermath comes before 60%. The second sitting waits for the muster | Order |
| 24 | 60%: the muster | Milestone |
| 25 | The second sitting, the evening after the muster | Order; one set piece an evening |
| 29 | 75%: the reading | Milestone |
| 31 | 80%: the last page | Milestone |
| 35 | The gate | Hard stop |
| 40–47 | The epilogue week | Last Rounds |

### Settings, log lines and tests

**Settings**, under **Majors** (attribute defaults only, never initialisers):

| Setting | Default | What it does |
|---|---|---|
| Lead points | 5 | How far before its milestone a climax may open when research stalls (0: never) |
| Grace sessions | 1 | The grace after a last call |
| Hold limit | 1 | The offered evenings (or sessions) an engaged act can hold its anchor |
| Stall points | 1 | Research gained over 3 real days with play, below which research counts as stalled |
| Between beats | 4 | The beat gap, in sessions, before a major fills it |

**Log lines**
- *"Majors: book act 3 squeezed (room 5.6 h, need 6 h). Last call."*
- *"Majors: orders acts 1–4 default as data (squeezed; nobody qualifies)."*
- *"Majors: orders act 5 leads at 57% (stalled 3 days)."*
- *"Majors: every_name act 5 waits for orders (Aftermath before the muster)."*
- *"Majors: book act 4 held 1 evening (act 3 engaged)."*

**Tests**
- `tools/pacing_sim.py` gains the 60, 70 and 80% milestones and an order check from 2 to 20
  regulars (conflict 0). Today's result: the order holds; Aftermath comes on or before 60% from
  about 7 regulars, which the second sitting's rule covers.
- The runtime test, once task 37 forces questline preconditions, runs a scripted season at Wait
  scale and checks that:
  - three real days with nobody online fire nothing;
  - stalled research leads a climax by at most 5 points;
  - a squeeze sends a last call, then a default after the grace;
  - Aftermath before 60% makes the second sitting wait for the muster;
  - a set piece nobody qualifies for runs as data after two free evenings, and an ordinary act
    nobody qualifies for waits for its squeeze;
  - the gate resolves an open climax at half effect.

---

## The Haven: Every Name

### What it's built on

H1 The List becomes its spine.
- H1's four built investigation stages (`RZ_Quest_TheList.c`) run as Act III, and its wait and
  decision become Act V.
- Its built flags keep their names (`list.husband_home`, `list.found_table`, `list.read`,
  `list.buried`, `list.to_bastion`, `list.to_redhand`, `list.sold_known`, `list.head_replaced`), so
  B2, R2 and C1 read them unchanged.
- The key is new, `every_name`, because `list` has already run on servers. It ships at a season
  wipe, and H1 is no longer offered on its own.
- Built blocks reused: Shut the Door, A Quiet Arrangement's lookouts and envelope, Over the Fence,
  Trader Caravan's raid, Trail of Clues, Patient Zero's Ledger's deduction, Our Own First, Fortify
  the Safehouse, Local Law, B2's Curfew Patrol, The List contract, hold and escort.

**Changes that make it fit the season**
- The second sitting moves off 75% to Aftermath or 70%, whichever comes first, and never before
  Bastion's muster has resolved. So it lands after the muster and before Halcyon's reading.
- The Gate moves to the canon spot `day2.checkpoint`, which it shares with Bastion's and Halcyon's
  codas.
- Bastion's cards read `orders.stance`, the arrangement is written into the Red Hand's book, and
  every number sits inside the season budget.

**Canon fixes** (to LORE.md, in the major's commit)
- The Long Table's dates move a day earlier: the truck is missed on Day 6, the radio message is
  Day 7, Mr Halas dies on Day 8, the bite is found on Day 9, and the house is crossed off on Day 10,
  the night before the season opens. "They learned on Day 10" stays true.
- The chair gets a fixed backstory line: *"Forced the doors of the school gym on Day 2, for a
  busload the line had turned back."*

H2, H3 and Short Weight stay as smaller stories.

### Premise

On Day 2 the Haven had one rule: you get in if you're willing to help. In the first hungry week
the council added a second rule and never wrote it down: houses with workers come first. Under
that rule, on Day 3, it sent its mouths (the old, the sick, a man in a wheelchair, a boy of ten) to
Ada Moravec's farm past the last village. On Day 7 the council's radio said there would be nothing
for the farm that week. On Day 10 the chair crossed the whole house off the List. The season opens
the next morning.

Now the houses are filling and the trucks come less often. The chair calls every council head to
the stronghold to write the second rule down, for every house. The council sits twice: once when
the food runs short, to write the rule, and once when the hungry days turn, to keep it or change
it. In between, someone finds out what the first council did, and whose pencil it was.

When the line stands down, the chair walks the Haven to the north road, where the Day 2 column was
turned back, and reads the List towards the barrier. Whose names are on it, and whether the crossed
ones are read, is what the Haven was this season.

### Why it's the Haven's major

The List is the proof, not the prize. The stake is the Haven's own question, which its "Mouths"
hard edge had no story for: when there isn't enough, who is the Haven for?

- **The rule shapes the whole season.** The rule the council writes is a season flag,
  `haven.rule`. Upkeep, trucks, stragglers, leavers, Requisition, the Red Hand's rackets and the
  exile leak all read it, each inside its own caps, so players feel it from Collapse to the wipe.
- **The stronghold leader is at the centre.** The chair calls both sittings, breaks ties, held the
  pencil on Day 10 and answers for it. The chair is never replaced.
- **Every faction has a stake.** Bastion offers ration cards for names. The Red Hand offers tins
  and keeps track of the strays. Halcyon wants page six left as numbers. The Long Table lives on
  what the Haven sends down its roads.
- **Every season has a Haven story.** Every act has a default, and the defaults lean towards the
  frightened Haven repeating its oldest mistake.

H2, H3 and Short Weight are one house's version of the same question, and their flags tilt the votes.

### Act I. The Ones Who Can't Work

**When**
- Early Outbreak, from the season's first evening. It's offered from 17:00, and the family waits at
  the edge of town at dusk (18:30). If nobody comes, they're back the next dusk.
- At the safehouse nearest the stronghold that isn't the stronghold (on a one-house map, at the
  stronghold). Any standing; not while exiled.
- The season's first set piece.
- **Clock:** anchor Collapse; patience 3 offered evenings. Default, which pays nobody: a Merciful or
  Idealist council head takes all four; any other head turns two away.

**Goal.** A family of four waits at the edge of town: two who can work, a grandfather in a
wheelchair, and a girl of eight with a cough. The council head says: *"We can take two. The two who
can work. Not with that cough."* It's the first hungry week again, in small.

What you do sets that head's lean at the first sitting. The four names go on the house's List,
crossed out or not. Uses Shut the Door and escort. Sets `every_name.family` (1 all in, 2 two to
Bastion, 3 two turned away). If the girl comes in and the house had no children, it paints the
bear on its door (a line in the news; no prop).

**Approaches**
- **Talk:** persuade the head (difficulty 2: Trusted, or Known with ★ Elder or ★ Nurse; one easier
  with a Merciful head). With ★ Nurse or ★ Doctor, you find the girl has a chest cold, not a bite.
  All four come in (+4 people, −2 food), and the head leans to the shared pot.
- **Bribe:** put 3 food or 20 Haven Favor into the house's stores. All four come in, as above.
- **Side with Bastion:** escort the grandfather and the girl to the nearest Bastion seat within
  3 km, where they become civilians sheltered (+5 Bastion Fame). The two who can work come in.
  Bastion's cards are tabled at the first sitting by themselves.
- **Do as the head asks:** the two who can work come in, and at dusk you walk the other two back to
  the road (the head pays 10 Haven Favor). Theirs are the season's first crossed names. They become
  data on the road, where the Long Table's built takings can find them. The head leans to workers
  first.

### Act II. The First Sitting

**When**
- Collapse. **The summons** goes out when any safehouse is short or has lost people to upkeep, or
  when the Haven's need has been Food for 2 hours of play. At the start of Overrun it goes out anyway.
- The council sits at the stronghold safehouse at the **second supper after the summons**
  (18:30–20:30), with at least one session of offered time between, so someone can carry an offer.
  It's a set piece: it never sits while the stronghold is Under Pressure, or on an evening another
  major's set piece holds; then it moves to the next supper.
- Swaying a head needs Known; anyone can carry an offer.
- **Clock:** it sits on the first offered evening. With nobody there who qualifies, it sits as data
  (by the rules in [Dynamic fallbacks](#dynamic-fallbacks)).

**Goal.** On the Haven's open channel: *"All houses. The council sits at {stronghold} tomorrow, at
supper. Every head comes, or sends someone. Bring your count. Bring yourself either way."*

On the table, in the Haven's own words:
- Everyone eats (the shared pot).
- Workers first.
- Bastion's cards (names for bread), only if a player carries the offer.
- An arrangement with the crossroads (tins, and nobody asks the price), only if a player carries
  the offer.

**The voters** are every council head, topped up to three with the stronghold's named people. The
chair breaks ties. Each voter's lean is shown in words, in the journal and when you talk to them.
It comes from, in order:
1. Their trait: Merciful or Idealist, the pot; Ruthless or Paranoid, workers first; Pragmatist,
   the cards; Greedy, the arrangement. Loyal heads follow the chair.
2. Their house's food.
3. What they did in Act I, and what they remember of you.

The vote is taken at 20:30, or when a hold ends. The winning rule is `haven.rule` until the second
sitting. Uses `RZ_QuestVote` (new), A Quiet Arrangement's lookouts, Over the Fence and Trader
Caravan's raid.

**Approaches**
- **Talk** (Known): speak for a rule to one head, and their vote moves one step your way. A head who
  leans the other way needs persuading (difficulty 2; one easier if they remember you helped;
  ★ Elder +1). ★ Elder: each head tells you, by name, who in their house they'd call a mouth. The
  word itself is never said.
- **Bribe:** put 3 food or 20 Haven Favor into a short house's stores. Its head votes for the rule
  you speak for (never for workers first if it's the far house).
- **Sneak:** at night, read the best-fed house's real stores past two lookouts. It has kept back
  more than its share, and the proof turns its head to the pot. Seen: +8 Haven Infamy, and that head
  remembers you crossed them.
- **Side with Bastion or the Red Hand:** ask their stronghold leader about the Haven's council
  (Known with them; ★ Police or ★ Ex-Con without), then carry the terms to the chair before supper.
  - Bastion's cards: ration cards for every house on the census, a curfew, and a contribution when
    Bastion is short (+5 Bastion Fame).
  - The arrangement: tins until the trucks come back (15 Red Hand Favor). It's written into the Red
    Hand's book as a line.
  - Some offers are tabled by themselves: the cards with `every_name.family` 2 or `door.bastion`;
    the arrangement with `door.tins`, `door.paid` or `walk.cut_dose`.
  - ★ Radio Op hears on a cracked secure channel what each offer is really for. Bastion: *"Haven
    count puts every name on the board."* The boss: *"Strays are worth more than tins."* Repeating
    the line at supper takes two heads away from that offer.
- **Betray:** tell a crew boss when the heads sit (20 Red Hand Favor; +15 Haven Infamy if a lookout
  sees you). A crew comes for the gate during supper (Trader Caravan's raid, 3 minutes).
  - Hold the gate: every head who saw it held votes with the players who held it (+5 Red Hand
    Infamy). The player who signalled earns no votes this way.
  - Let the raid happen: the frightened heads vote for guns: the cards if tabled, else the
    arrangement, else workers first.

### Act III. The List (H1 stages 1–4)

**When**
- From the first sitting's vote, once any house has lost people this season or The List contract
  has been done (H1's built start). It runs through Overrun.
- The dissenting council head gives it: an Independent or Rogue head, or the one who lost the vote.
  *"Ask {chair_short} about the farm. Ask about Day 7."*
- Known with the Haven; not while exiled.
- **Clock:** each stage gives up after 6 hours of offered time and is offered again, at most twice.
  Anchor: the second sitting. No default: this act only gives knowledge, so stages still open when
  it's squeezed simply close.

**Goal.** A woman finds her husband's name crossed out while he's still out on a supply run. Under
workers first it was crossed out this week: his truck stopped, and a driver without a truck is a
mouth. Under any other rule it's a cross from the first days.

Following it leads to the Haven's oldest shame: the Day 3 farm, the Day 7 radio message, the Day 10
cross, and the chair's initials on the farm's line. H1's four built stage missions run unchanged,
with three new approaches.

Sets:
- `list.truth`: 1 means the farm and whose hand; 2 means only "a farm out that way".
- `list.husband_home` and `list.found_table`, as built.
- `every_name.witness`, `every_name.boy` and `every_name.store_found`.

**Approaches**
- **Find the husband** (H1 stage 1, Trail of Clues). Half the time he's alive and comes home (+1
  person, `list.husband_home`, which counts as proof). A Scavenger sells his last sighting for 1
  food; ★ Radio Op hears his last call.
- **Three copies, then the pattern** (H1 stages 2–3).
  - The copies: talk to council heads; sneak a copy (+8 Haven Infamy if seen); bribe a house (3
    food); or buy the Red Hand's list of strays (15 Red Hand Favor).
  - The pattern: the Ledger deduction (★ Elder or ★ Police: the method spelled out), or ask the chair
    (Trusted): *"We sent them somewhere they'd be fed. Somebody had to decide. Eat something."*
  - **One cross doesn't fit** (confirmed lore). On all three copies, one name was crossed out on
    Day 6, in the pencil the houses use for the dead, with no ration note and no farm. Asked, the
    head who holds that copy says only: *"Leave that one."* It's the seed for the finale's Haven
    route: nothing in this major says who she is, or why.
  - New: sneak the stronghold's radio log at night, past two lookouts. Day 7, in the chair's hand:
    *"Nothing for the farm this week. Houses with workers come first."* That's the whole truth
    without the deduction (`list.truth` 1). The same storeroom holds the winter store the other
    houses were never told about (`every_name.store_found`). Seen: +8 Haven Infamy, and the store is
    moved before the second sitting. ★ Radio Op reads the log in the radio set's own notebook, with
    no storeroom, no lookouts and no store.
- **Talk to someone who was there** (new): the bus driver, who sat on the first council and now
  lives at the stronghold. Needs Trusted with the Haven, ★ Elder, or Known after asking the chair. He
  tells it straight and will stand up at the second sitting (`every_name.witness`, which counts as
  proof).
- **Go and look** (H1 stage 4). In a Long Table season, the host meets you at the gate and sets a
  place (C1 starts at stage 2; `list.found_table`). Otherwise it's an empty farm: nine graves, and
  one too few.
  - New: find the boy of ten who came with the mouths. He's the Kid wanderer if one is on the
    roads, otherwise a quiet boy at a safehouse. Talk to him unarmed (★ Nurse or ★ Elder: he says one
    Haven name and no more), or show the Kid the way to the stronghold (+1 person). Either counts as
    proof (`every_name.boy`). In a Long Table season, C1's stage 3 talk counts.

### Act IV. The Hungry Week

**When**
- Overrun. It opens at Overrun's first supper after the first sitting, or sooner if a house hits
  its upkeep floor or Haven morale falls under 45.
- At Act I's house if the family came in, else at the hungriest house (H2's house, if H2 ran first).
- Known with the Haven.
- **Clock:** anchor the second sitting; patience 3 sessions of offered time. Default, which pays
  nobody: the rule's own answer happens.

**Goal.** The rule's cost lands on real people. The house can't feed its mouths this week (−2 food
at once if it isn't already short). Who comes with an answer depends on the rule:
- the pot: the chair asks the fed houses to send food;
- workers first: a council head asks someone to walk the mouths to the far house at dusk;
- the cards: the nearest Bastion seat offers to take them in;
- the arrangement: a crew boss offers to take a stranger off the house's hands (A Quiet
  Arrangement's envelope).

Every approach is open whatever the rule. Sets `every_name.week`. Each head who saw the rule work,
or fail, leans the way it went at the second sitting.

**Approaches**
- **Feed them:** carry 4 food from the best-fed house within 90 minutes, or ride the next Haven
  truck (Our Own First). The Red Hand smells a full cart: hold off its raid for 3 minutes, or bribe
  the crew boss 25 Red Hand Favor. Or pay it yourself: 4 food or 30 Haven Favor. The head leans to
  the pot.
- **Fight:** hold the house while three of Overrun's waves come (Fortify the Safehouse). The house
  keeps its people, and counts as Barricaded until the next restart.
- **Move them:** escort two mouths to a Bastion seat as civilians sheltered (+6 Bastion Fame; the
  head leans to the cards). Not after Expended, when Bastion's posts take nobody in. Or sneak them
  at night into another house past two lookouts: +2 people there, and its head leans to the pot.
- **Give them up:** walk up to three mouths to the far house at dusk (escort; this house never
  drops below 2 people). It's workers first's own answer. With one safehouse, they're left on the
  road as data. Or hand the stranger over at the crew's drop (+15 Haven Infamy if seen;
  `door.exile_sold`, as in H2).

### Act V. The Second Sitting (the Haven's climax)

**When**
- At the start of Aftermath or 70% research, whichever comes first, and **never before Bastion's
  muster has resolved**. From about 7 regulars Aftermath comes on or before 60%; then the sitting
  takes the first free supper after the muster. On 2 and 3 regulars it's always 70%.
- It needs at least one session after Act III's last stage (H1 stage 5's wait). It falls after
  Bastion's muster and before Halcyon's reading.
- The council sits at supper (18:30–20:30) at the stronghold safehouse. It's a set piece: if
  another major holds that evening it moves to the next free supper.
- **Clock:** it sits on the first offered evening; a lead may open it from 65% when research stalls.
  Hard stop: the gate. Default, which pays nobody: the sitting votes as data, and the standing rule
  usually stands. The List is kept, unless `list.truth` is 1 and the chair is Merciful or Idealist:
  then the chair reads it out unasked.

**Goal.** The rule was written for the hungry days. Now the chair calls the houses to keep it or
change it. If Act III found the truth, the copies are on the table too.

The houses vote again on the four rules. The two offers are on the table only if a player carries
them again, or if one of them won the first sitting. The cards are never on the table after
Bastion's Expended: Bastion has nothing left to count for.

This vote sets `every_name.ending` and the rule for the rest of the season. The List's fate sets
H1's built flags: `list.read`, `list.buried`, `list.to_bastion` or `list.to_redhand`.

**Approaches**
- **Read it out** (once Act III found the truth, `list.truth` 1 or 2). Persuade the Haven:
  difficulty 2, Trusted.
  - One easier for each proof, two at most: `list.husband_home`, `every_name.boy`,
    `every_name.witness`, the Day 7 log, `cargo.freed`, `door.exile_home`, `table.walked_out`.
  - One easier, once, if another stronghold's page is already in the open (`majors.pages_open`:
    the Red Hand's book read whole at settling day, or Bastion's All Stations).
  - One harder with `list.truth` 2.
  - Known is enough with B2's `census.refused`, as built. ★ Elder +1.
  - Or persuade the chair to read it, one step harder (`list.chair_read`; the chair remembers you
    helped).
  - Every head who isn't Ruthless swings to the pot. The chair answers once: *"We'd all be dead. I
    decided, and I'd decide it again. Read the next one."* Then hold the reading at the gate while
    the bells bring two waves (3 minutes, as built). If the hold is lost, the sitting scatters to the
    next supper, twice at most; after that the List is kept.
- **Gather the others' pages before supper,** so the reading names them too:
  - The lab's page six: the two who didn't walk out of the Day 6 trial. Persuade a field lead
    (Trusted with Halcyon, or ★ Lab Tech), sneak the ledger on the night shift (+8 Halcyon Infamy if
    seen), or bribe a courier (20 Halcyon Favor). With `other_two.named` or `other_two.haven_names`
    it's already out. Page six has an eleventh line too: *"did not present"*, beside a Haven name.
    Read at supper it's one more name, and nobody at the table says what it means.
  - The Red Hand's strays, from Act III's tally man.
  - With `orders.haven`, Bastion's Day 3 page, which the chair already holds.
  - Each adds a line to the headline (`every_name.page_six_read` for page six), and that faction's
    stronghold leader remembers you crossed them. Or carry the director's own offer instead (Known
    with Halcyon): 20 Halcyon Favor, and page six stays numbers (`every_name.page_six_struck`).
- **Keep it quiet:** give the copies to the chair (`list.buried`). The chair remembers you helped,
  and the Loyal heads vote with the chair's lean. You can still speak for a rule, or bribe a short
  house, as at the first sitting.
- **Sneak:** during supper, while the lookouts are at the table, open the winter store (60 seconds
  unseen; already open with `every_name.store_found`) and walk a head in. Every head sees there is
  food, and the pot gains two heads. Seen: +8 Haven Infamy, and workers first gains one head.
- **Betray the reading:** before the vote, carry the copies past two Haven lookouts.
  - To a Bastion seat (`list.to_bastion`; +10 Bastion Fame, +15 Haven Infamy): the cards gain two
    heads. Not after Expended.
  - To a crew boss (`list.to_redhand`; 40 Red Hand Favor): the arrangement gains two heads.
  - Seen by a lookout: the copies are taken back (+8 Haven Infamy, as built).

### Act VI. A Week On

**When**
- After the second sitting: the next time any safehouse goes short, or once the major's beat gap
  reaches 2 sessions of offered time, whichever comes first.
- One stage mission, chosen by the ending. It gives up after 3 hours of offered time, and is
  skipped if the final stand is won first. It may run past the gate.
- It changes the ending's numbers and its epilogue line, never the rule.

**Goal.** The first real cost of the decision shows up:
- the shared pot: a short house "borrowed" from the best-fed one in the night;
- workers first: the far house has stopped answering the radio;
- the cards: the first curfew night in a Haven town;
- the arrangement: the crew's first collection wants a name, not tins.

**Approaches**
- **The pot** (Every Name, The Closed Book): hear both houses and split it (Local Law; Known), or
  carry 4 food from the best-fed house to the short one within 90 minutes. If it holds: Haven morale
  +5 (`every_name.held`). If it fails: the pot lapses for one real day. Under The Closed Book, a name
  is also crossed out again at the short house (The List contract); telling the family brings the
  lasting risk due.
- **The far house:** go there (★ Nurse or ★ Elder: its head tells you who left). Carry 4 food there
  (`every_name.far_fed`: it isn't last for two upkeeps). Or walk two of its people home at dusk past
  the infected (escort): the chair says *"I didn't see that."*
- **The curfew** (Counted): patrol the first curfew night in a Haven town (Curfew Patrol). Enforce it
  (Bastion Fame, +3 Haven Infamy), patrol it gently (★ Police, or Known with the Haven), or let a Red
  Hand toll in (Known with the Red Hand): the next contribution is skipped once. Under All Stations
  there's no curfew: Bastion sends a medic to the first counted house instead, and this act is skipped.
- **The collection** (Paid at the Door): pay it in Favor instead of a name (25 Red Hand Favor). Or talk
  the boss's man round (Trusted with the Red Hand, or ★ Ex-Con): *"A debt is a debt, but it can be
  yours"* (`every_name.debt_yours` *(you)*). Or warn the stranger (`exile.warned`). Or refuse and hold
  the house against a raid for 3 minutes: the arrangement ends early, and a feud starts under the
  usual rules. If the book was sent off loud (`book.debts_dead`), there's no collection, and this act
  is skipped.

### Act VII. The Gate (coda)

The first of the Last Rounds: [the codas in the epilogue week](#last-rounds-the-codas-in-the-epilogue-week)
has the calendar, the standing version and the no-cure case.

**When.** The epilogue's first free evening (dusk, 18:30). Bastion's last call follows at 20:30 the
same evening. It runs once, can't change the ending, and carries nothing into the next season but
its hall lines. Open to any player the Haven hasn't exiled.

**Goal.** The chair walks the Haven to `day2.checkpoint`: the barrier where the Day 2 column was
turned back, and where the bus driver went looking for somewhere to put his passengers. At the last
bend, 150 m short of the barrier, the chair reads the List. The voice past the line turns back anyone
who goes closer. Nobody behind the concrete answers, just as on Day 2.

Whose names are read shows the ending.
- Under Counted, a Bastion officer reads the Haven's number along with Bastion's own.
- Under Paid at the Door, a Red Hand lorry is parked on the verge.
- If the finale's Haven route found the woman (`unturned.named`), the chair reads her name last,
  uncrossed ([FINALE.md](FINALE.md), §12).
- It adds one line and up to three names to the season story.

**Approaches**
- **Escort** the houses' people from the stronghold to the bend (at most 3 groups, with the
  infected on the road), then stand with the chair.
- **Hold the road** while the names are read (2 waves; ordinary hold pay).
- **Read a name** of one of your own dead characters, from the journals of the dead. The chair
  repeats it.
- **Read the crossed names yourself,** when the ending isn't Every Name (needs `list.truth` 1, plus
  Trusted or ★ Elder). The ending stays the same. The epilogue line gains *"...and somebody read the
  crossed ones at the barrier"*, and The Closed Book's lasting risk comes due.

### Endings

**Every Name** (the pot, and the List read)
- **You get:** the deciders are whoever read it out, held the reading, or swung a head to the pot at
  either sitting. They get Haven Fame +15, 30 Haven Favor and one item from the Haven shelf at their
  tier, within the major's caps. The council head who gave Act III remembers them as having saved
  them. If they read over the chair's head, the chair remembers being crossed. Everyone else who
  took part: Haven Fame +5.
- **The world:**
  - Haven morale −8 at the reading, then +5 if A Week On holds.
  - The council head whose house crossed out the most names steps down (`list.head_replaced`). That
    uses a seat on the ledger, and it's never the chair.
  - Long Table suspicion +30 (`list.read`, as built).
  - Some of the crossed-out come home: +1 person at up to two safehouses.
  - If page six was read by name: Halcyon morale −3.
  - It counts as a page in the open (`majors.pages_open` +1).
  - Headline: *"Every Haven house read the List at supper, crossed names and all. {chair} read the
    first one."*
- **The season:** the rule is the shared pot (`haven.rule` 1). At upkeep, a short house takes up to
  2 food from the best-fed one, never the giver's last meal. Stragglers ×1.2. The List contract never
  picks a living name.
  - Epilogue line: *"The Haven read every name on its List this season, the crossed ones too, and
    shared the pot until the gates opened."*
  - Hall of fame: "Read every name".
  - Next season (after a cure): the Long Table's chance ×0.5, and the first safehouse's notice board
    opens with the chair's last line at the barrier (words only; the default line if the Gate ran as
    data).

**The Closed Book** (the pot, and the List kept)
- **You get:** the deciders are whoever gave the copies to the chair, or swung heads to the pot
  without a reading. They get Haven Fame +10 and 20 Haven Favor, and the chair remembers they helped.
  Everyone else who took part: Haven Fame +5.
- **The world:** Haven morale +3. `list.buried`.
  - **The lasting risk:** if the List comes out later, Haven morale −10 at once, with the headline:
    *"The Haven kept a list of the people it sent away. Somebody else read it first."* It can come out
    through Bastion's nightly call (with `list.to_bastion`), a Red Hand sale that's found out, a family
    told in A Week On, or someone reading the crossed names at the barrier.
- **The season:** the same shared pot as Every Name (`haven.rule` 1), but The List contract keeps its
  usual weight.
  - Epilogue line: *"Every Haven house shared the pot this season. Nobody read the List."* If it came
    out: *"...until somebody else read it."*
  - Next season: only one line of Haven gossip.

**The Far House** (workers first)
- **You get:** whoever spoke for workers first, or walked mouths to the far house: Haven Fame +10 and
  20 Haven Favor, and the chair remembers they helped. Whoever fed the far house, or walked its people
  home in A Week On: Haven Fame +8.
- **The world:** Haven morale +5 and strength +3. If the List was read and the houses chose workers
  first anyway: Haven morale −8 instead.
  - People: up to 3 move to the far house (cap 30, floor 2), "where there's room". With one
    safehouse, 2 leave onto the road as data.
  - Leaders: the dissenting head who gave Act III steps down, if they voted against (a seat on the
    ledger).
  - Headline: *"Every house that works will eat. The rest are going to {far_town}, where there's room."*
- **The season:** the rule is workers first (`haven.rule` 2).
  - Upkeep ×0.9, because mouths get smaller portions.
  - A fed house no longer takes in another house's leavers. They go onto the roads, where the Long
    Table's built takings can meet them.
  - Haven trucks go to the far house last.
  - Rescued survivors walk off ×1.25.
  - The List contract ×1.2.
  - Epilogue line: *"The far house in {far_town} was last on every list this season."* With A Week
    On's food: *"...but somebody went."*
  - Next season (after a cure): the Long Table's chance ×1.5 (never over 1).

**Counted** (Bastion's cards)
- **You get:** whoever carried Bastion's offer or the copies, or spoke for the cards: Bastion Fame +15
  and 30 Bastion Favor, plus Haven Fame +5. Whoever carried the copies to Bastion: Haven Infamy +15,
  and the chair remembers being crossed.
- **The world:** every Haven house is on Bastion's census (`haven.counted`): +2 food once at up to
  three houses. Haven morale −5; Bastion morale +5 and strength +3, inside Bastion's season budget. B2
  counts as done: if it hasn't run, it starts at its stage 3.
  - Headline: *"Bastion read the Haven's names to Command last night, {haven_people} of them. No reply."*
- **The season:** the rule is Bastion's cards (`haven.rule` 3).
  - Requisition picks counted houses ×0.8. Each house has one Requisition weight, shared with B2 and
    `orders.stance`.
  - One contribution each time Bastion's need turns to Supplies, at most once a real day.
  - A curfew at dusk in counted towns. It's the same curfew as Bastion's and Command's, never applied
    twice, and there's none under All Stations.
  - If the muster ends Expended after this, the cards lapse at once: no curfew, no contribution, and
    the rule goes back to the pot.
  - Epilogue: Bastion's call reads *"Civilians sheltered, {haven_people}. List to follow. Over."*
  - Next season (after a cure): one Bastion checkpoint starts on a safehouse town's road, inside the
    road-post cap.

**Paid at the Door** (the arrangement)
- **You get:** whoever carried the Red Hand's offer or the copies, or spoke for the arrangement: Red
  Hand Fame +15 and 40 Red Hand Favor, plus Haven Fame +5. Each session for three sessions, there's a
  20% chance it's found out: then the carrier takes Haven Infamy +20, and the headline names them.
- **The world:** the first tins arrive at once: +2 food at each of the two hungriest houses. Red Hand
  strength +3 and morale +5. Haven morale −5, or −10 if it's found out by headline. The paying houses
  become a line on the book's back page (`haven.paying`), which the Red Hand's major can collect, sell
  or burn.
- **The season:** the rule is the arrangement (`haven.rule` 4). Every Haven safehouse is a paying house:
  - it's never picked by Protection Racket or a Red Hand raid, but loses 1 food at every second upkeep;
  - a hungry house sells an exile's location 35% of the time instead of 25%;
  - A Quiet Arrangement ×1.2;
  - if the Red Hand sends its book off loud (`book.debts_dead`), the debts die: the tins stop costing,
    and the leak goes back to 25%.
  - Epilogue line: *"The Haven ate Red Hand tins this season. Everyone knows what they cost now."*
  - Next season (after a cure): for the first real day, exiles' locations sell 30% of the time.

### Crossings

**Bastion's major** (it feeds the muster):
- `haven.rule` 3 from the first sitting makes the muster's "civilians sheltered" true, and lets
  "Answer it yourself" go at Known.
- With `orders.read` or `orders.mocked`, tabling the cards needs a player Trusted with the Haven.
- `orders.haven` lets the Day 3 page be read with the List.
- It reads the stance back:
  - stance 2: the cards become "civilians first", tabled at Known, with no curfew;
  - stance 3: Ruthless and Paranoid heads lean to the cards;
  - stance 4: the cards can't be tabled, and Counted lapses.

**The Red Hand's major:**
- The Haven council head on the torn page is a voter who leans to the arrangement. After `book.burnt`,
  that head remembers who saved them.
- A line turned in to the chair (`book.turned_in` Haven) lets the chair make that head step down at
  the second sitting (a seat on the ledger).
- Paid at the Door writes the paying houses into the book; Sent off loud (`book.debts_dead`) ends
  their cost.
- A rival crew walked to a Haven door at settling day (`book.settled` 4) adds up to 3 people at one
  house, and that head votes with whoever brought them.

**Halcyon's major:**
- Page six is gathered in Act V.
- Every Name, or any list read, counts as a page in the open, so the director's persuade at the
  reading is one step easier (*"They read theirs."*).
- Any list read gives the chair's clerk a seat at Halcyon's reading.
- `every_name.page_six_read` puts the volunteers on the paper by name; `every_name.page_six_struck`
  keeps them as numbers.
- With `weeks.public`, the director makes no offer to strike page six.

**The Long Table** (C1, C2):
- `list.found_table` and `list.read` work as built.
- `table.walked_out` before the second sitting counts as proof.
- Under workers first, the turned-away and the far house's leavers go onto the roads, and next
  season's chance rises.

**The Haven's stories:**
- H2: `door.tins` or `door.paid` tables the arrangement, `door.bastion` tables the cards, `door.shut`
  makes that head vote against the arrangement, and `door.exile_home` counts as proof.
- H3: `walk.cut_dose` tables the arrangement; `walk.walks_stay` has the walker read his eleven at the
  Gate.
- Short Weight: Full Weight leans that head to the pot, Short Weight to the arrangement. It starts
  twice as often during Acts II–IV.

**The finale** reads `haven.rule`, `list.read`, `list.buried` and `proof.haven_gate`: what each changes
in its routes is in [FINALE.md](FINALE.md), §12.

**Publishes** `haven.rule` (1–4), `haven.counted` and `haven.paying` (one bit per safehouse),
`every_name.ending`, `list.truth`, `every_name.page_six_read` and `.page_six_struck`, and H1's built
`list.*` flags.

### Engine needs

- **`RZ_QuestVote`,** used at both sittings:
  - voters: every council head, topped up to three with the stronghold's named people;
  - options: two always, plus up to two when tabled;
  - leans from trait, loyalty, the house's food, Act I and memory of players; `Sway(voter, option)`
    moves a lean, one sway per player per sitting;
  - a lean line per voter, for talk and the journal;
  - the tally at 20:30, or when a hold ends; the chair breaks ties; the same tally runs as data when
    nobody is there;
  - kept in run flags.
- **The council scene:** stand-ins of the voting heads at the stronghold safehouse during a sitting
  (`SpawnLeaderAt`; only within 300 m of a player). From the summons to the tally, the run claims the
  stronghold safehouse and every voting head.
- **Readers of `haven.rule`,** each inside its own caps:
  - `RZ_Safehouses`: the pot's transfer, upkeep ×0.9, `FindFedSafehouse` skipped under workers first,
    stragglers, walk-off, the paying houses' −1 food, `MovePeople` keeping the floor of 2 and the cap
    of 30;
  - `RZ_SupplyLines`: the far house last;
  - Requisition: the shared per-house weight;
  - Protection Racket and Red Hand raids skip paying houses;
  - the exile leak at 25–35%;
  - also `LapseRule(realHours)`, and `{far_town}` as the smallest safehouse farthest from the stronghold.
- **Canon and people:** `haven.store` (the winter store, 8–14 food, data only), `haven.driver` (the bus
  driver), the chair's fixed backstory line in `RZ_LeaderVoice`, the boy (the Kid, or a generated boy
  at a safehouse), and the family of four as generated people.
- **`RZ_Quest_TheList.c`'s stage classes** reused with the major's run texts and flags (`{wife}`,
  `{husband}`, `{askerhouse}`, `{chair}`, `{farmwhere}`). H1's own Define and Next are retired: the line
  stays registered but is never offered. It ships at a wipe.

---

## Bastion: Await Relief

### What it's built on

B1 Await Relief grows into it and keeps its key, `orders`, its code
(`Missions/Questlines/Bastion/RZ_Quest_AwaitRelief.c`), its title and its built flags (`orders.stove`,
`.read`, `.mocked`, `.haven`, `.copy`, `.tape`), so R1, B2 and Requisition read them unchanged.
- Acts I–III are B1's five built stages, and B1's four endings become Act III's results through `ActEnd`.
- Acts IV–VI are new. They reuse built scenes: the radio by the CO's seat at the nightly call (B2
  stage 6), Struck Off (with Hunt the Deserter merged in) and its nail, Rein In, the padded ledgers of
  Burn the Ledger and Rifles for Sale, A Quiet Arrangement's lookouts, The Last Convoy, H1's hold,
  Curfew Patrol, Act I's relay hold, Refugee Column and Escort, and HC3's Bastion guard group.

**Changes that make it fit the season**
- The muster moves from 50% to 60%, so it falls after the Red Hand's settling day and before the
  Haven's second sitting.
- The stance now settles Bastion's part at the final stand, whatever HC3 stage 3 said earlier.
- The dawn walk goes to `day2.checkpoint`, the canon spot shared with the Haven's and Halcyon's codas.
- Every number sits inside the season budget.

**Canon:** `bastion.orders` is pinned to "expended". B2 and B3 stay as Bastion's two smaller stories.

### Premise

Every night for {days_since_command_words} days, the operator at every Bastion post has read the
numbers to Command and written "No reply". Then the posts on the hill side start hearing an answer
after the call: a long burst, too broken for voice. The stronghold's order on the secure net is
"Log nothing."

The burst is Bastion's own Day 3 signal, still looping off the relay on the hill because nobody ever
acknowledged it. It leads to the runner who never arrived, and to the page in his pocket: *"...hold
the line... no relief will... authorised to... expended."*

The commanding officer holds the bigger truth. The stronghold's own operator decoded the same burst
on Day 3 and handed it over. The CO read the Day 3 text, burned it and said nothing, and that night
gave the units a name on the radio, so Command would have something to call them. On Day 5 the far
side was relieved in place, and that night the CO read Sedlak's log to Command as a last test. On Day
6 the CO told every post "Nobody is coming", and never told them the rest: nobody ever meant to. The
roster has held Bastion together since Day 6.

Now the page is loose, the posts are padding their numbers, and the cure is coming. Before the gates
open, the CO has to say, in front of every post, what Bastion is: a roster that still asks for relief
out of habit, a line held for the people inside it, the Zone's own Command, or a roster the Red Hand
reads out as expended. Command never answers, in any version.

### Why it's Bastion's major

Every Bastion ritual rests on the roster: they're all they have. It explains the roll, the call,
"signed for", "struck off" (nobody says "deserter" on the net), the census and the bunker.
"Expended" says what the roster was worth to Command. Await Relief is the only line that asks
whether anyone ever meant to come.

The stronghold leader has known since Day 3. The CO is never replaced and stays Loyal, so what
changes is Bastion's doctrine, set in public at a muster of every post. The stance
(`orders.stance`) decides, inside the caps:
- Bastion's morale and strength;
- its curfew, and how Requisition and checkpoints treat civilians;
- whether its posts stay manned;
- who Bastion stands at the lab for in the final stand;
- what it does at the north road when the gates open.

Every faction has a stake. The Haven was turned back at the north road on Day 2 by men under these
orders. The Red Hand's best joke is Bastion asking a dead radio for relief every night. Halcyon
learns why Command wanted the lab secured (*"Priority remains Institute site and personnel"*).

B2 and B3 are each one consequence of the roster, and neither can carry the season.

### Act I. Reply received

**When**
- Collapse, after the nightly call (20:30). Once due, it starts with priority.
- The giver is an Independent outpost commander (the one R1's `book.bastion` names, if he's still in
  the chair), or No Reply's tape post; both are built. If neither exists, any commander gives it at
  once. If one exists but nobody who qualifies has met him in 2 sessions of offered time, any
  commander gives it.
- **Clock:** anchor the muster; no patience (it stays open until squeezed). Default, which pays
  nobody: a stronghold patrol walks the runner's road, and the page goes into the stove.

**Goal.** B1 stages 1–2: prove the "reply" is the Day 3 burst on a loop, then find the runner and his
page.

Opening headline: *"Three Bastion posts logged a reply after the call last night. The stronghold's
order on the secure net: log nothing."*

**Time pressure.** If nobody has compared the tape by the next nightly call with a player online, an
operator logs "Command answered", and every post hears it. Bastion morale +6 for the night, then −8
when the log says "No reply" again (`orders.false_hope`).

**Approaches**
- **The tape** (built stage 1): play it into the relay on the hill, 0.7–1.6 km from the post, and stay
  within 30 m for 3 minutes while a horde comes after the first minute. ★ Radio Op: twice as fast. The
  relay's log names the runner and the hour he left.
- **Sneak** (built): copy the tape unseen instead of taking it (`orders.copy`, recorded by identity).
  It's the only proof that survives the stove, and it counts again in Act V.
- **The runner** (built stage 2): his satchel, his fire (2 infected) and his body (5 infected, one a
  special), along Struck Off's trail (the step chain). New: ★ Hunter or ★ Police get grid references at
  each step. With tribes, his road crosses a tribe's range and his body can lie at an old kill site
  ([INFECTED.md](INFECTED.md)).
- **Side with the Red Hand** (new; Known with the Red Hand): the satchel has turned up at a crew's
  fence. Buy it for 15 Red Hand Favor and skip the first step. The boss now knows what Bastion is
  hunting (`orders.rh_knows`).
- **Betray the commander** (new; Trusted with Bastion): report him to the CO's duty officer
  (`orders.reported`). A Bastion squad heads for the body (the squad prefab, or with none, data on a
  25-minute timer). Get there first, or the page goes straight into the stove, and Act II is skipped.

### Act II. Who reads it

**When**
- After Act I (built B1 stage 3: 60 minutes, once more if the page is lost).
- **Clock:** anchor the muster. Default: the stove.

**Goal.** Decide who reads the orders first. Each recipient answers on its own channel: the
stronghold on the secure net, the commander at his shift change, the crew on the open channel, the
chair at supper. So every faction hears that Bastion's orders have turned up. Sets `orders.stove`,
`.read`, `.mocked` or `.haven`.

**Approaches**
- **Talk** (built; Trusted, or ★ Soldier): the duty officer walks the page through to the stove.
  *"These never arrived. Thank you."*
- **Talk** (built): give it to the commander who sent you. He reads it to his men at the shift change.
- **Side with the Red Hand** (built): a crew boss's radio man within 6 km reads it on the open channel
  (40 Red Hand Favor, +8 Bastion Infamy).
- **Side with the Haven** (built): the chair's clerk keeps it as proof of what Bastion was told after
  Day 2 (+5 Haven Fame). The chair can read it with the List at the Haven's second sitting.

### Act III. The reckoning

**When**
- B1's built one-session wait, then built B1 stages 4–5 (usually late Collapse).
- **Clock:** anchor the muster. Default: the branch settles itself at half effect.

**Goal.** Settle the post the burst came from. B1's four built endings (Into the stove, The men were
told, On the open channel, The Haven's leverage) are applied here through `ActEnd` as the act's
result (`orders.reckoning` 1–4), and the line goes on. Its leader changes count toward the major's
two seats on the ledger.

After The men were told, B1's wait has two men walk off the post to the nearest safehouse (+2
people, as built). Under the roster that makes them deserters: they're struck off as "KIA. Body not
recovered", nobody answers for them at the roll, and Struck Off is weighted up at that post for a
season day.

**Approaches**
- **The men were told** (built). Choose one:
  - fight for the commander (+15 Bastion Infamy per squad member; he goes Rogue and keeps his post,
    and Rein In follows);
  - walk him to the stronghold yourself;
  - talk the CO down on the sergeant's radio (Champion, or Trusted and ★ Soldier: loyalty +25);
  - stand aside (half pay).
- **On the open channel** (built): find the operator who "leaked" it. Bring him in, or walk him to a
  safehouse (+1 person, +5 Haven Fame; he's struck off too).
- **Into the stove** (built): whoever copied the tape can still give the copy, once, to the commander,
  a crew or the chair, which moves the branch. Or they can put it in the stove too.
- **The Haven's leverage** (built): carry a ration crate from the commander's post to the chair's clerk
  (+4 food). That's the chair's price for keeping quiet.

### Act IV. Effective Strength

**When**
- Overrun, one session after Act III. At the nightly call, at the radio by the CO's seat (offered from
  19:00; the call runs 20:30–22:30). Not while the stronghold is Under Pressure.
- For whoever decided Act II or III, or anyone Trusted with Bastion or ★ Soldier.
- **Clock:** anchor the muster; no patience. Default: the count stays padded, and a Paranoid or
  Ruthless CO writes up the worst post (words only).

**Goal.** The CO's confession, then the count.

The confession: *"That wasn't the first copy to reach my stove."* The CO says what the whole Day 3
text said, and why it burned: *"They could carry 'nobody is coming'. They had each other. Tell them
nobody ever meant to come, and they'll stop asking anyone for anything. They'll take it."* Then the one
line the runner's page lost: *"Priority remains Institute site and personnel."* Then the ask: *"Every
night I tell Command thirty-eight. Before those gates open, I want to know what I'd be telling them if
I told the truth."*

The count: get the true number from the three posts nearest the stronghold. With fewer posts, the
stronghold's own stores make up the rest. Each post pads its number one way: a deserter struck off as
killed in action, rifles off the inventory, or a contribution never written down. The true number
fills `{count}` (`orders.count`, `orders.padded`).

When the count is in, the CO pulls in the weakest post, and The Last Convoy runs for it, as built. That
gives a story to why Bastion's work thins out in the second half of the season.

**Approaches**
- **Talk,** at each post (a 40-minute contract at the commander's seat, Known): a Loyal commander shows
  you the board. An Independent one needs persuade 2 (Trusted; ★ Police or ★ Soldier +1). A Rogue one
  lies, so use another approach at his post.
- **Sneak:** read the roster and the stores ledger at night, past two lookouts. Seen: +8 Bastion
  Infamy, and that post stays padded.
- **Trick** (the disguise design, [DISGUISE.md](DISGUISE.md)): a player below Known with Bastion (for
  example, whoever read the page on the open channel in Act II) reads a post's roster at night in
  Bastion kit.
  - The duty board is in the radio room, at least 25 m from the commander's seat.
  - Two looks: the gate sentry (difficulty 2), and one more at the board (difficulty 3).
  - The cover detail is a name off the nail. A dead man's name works anywhere except at his own post.
  - Caught: the act's +8 doubles, within the questline cap of 15, plus +3 for the impersonation. That
    post stays padded.
  - Called back at the muster: *"And whoever signed the roster as {dead_name}: he's still dead."* That
    player's persuade at the muster is one step harder.
- **Bribe, or side with the Red Hand.** The post's quartermaster sells his own copy for 15 Bastion
  Favor. Or for 20 Red Hand Favor, the fence tells you how many rifles every post sold.
- **Already counted:**
  - the post on R1's `book.bastion`;
  - "civilians sheltered", with `haven.counted`, B2's Counted or `list.to_bastion`.
  - With `proof.day_one` 3 or `weeks.bastion`, the CO adds "and the lab knew" to the count.
- **The act's real choice,** for each padded number:
  - Report it: an Independent or Rogue commander gets Rein In's terms (loyalty +25), and a struck-off
    man who is alive becomes a Struck Off contract.
  - Cover for the commander: he remembers you helped, and `orders.padded` goes up by 1.

The major moves at most two seats. A seat another major already moved this season is only written up.

### Act V. The muster (Bastion's climax)

**When**
- At 60% research, one session after Act IV, and at least one evening after the Red Hand's settling
  day. Offered from 19:00; the call runs 20:30–22:30 at the radio by the CO's seat.
- It's a set piece. Every outpost commander answers on the secure net as text from his own seat, with
  no bodies spawned.
- **Clock:** patience 2 offered nightly calls; a lead may open it from 55% when research stalls. Hard
  stop: the gate, where it resolves at half effect. Default: the CO decides alone, by Act III's result
  and the CO's traits.

**Goal.** The CO calls every post onto the net: *"Everyone on this net has heard a word by now.
Tonight I read the numbers to Command, or I read them something else. Tell me which."*

What the CO reads sets `orders.stance` (1–4) for the rest of the season, and that stance is the
major's ending. The CO is never replaced and stays Loyal: what changes is what Bastion does, not who
leads it.

**Approaches**
- **Keep the call** (Known; it's what the CO wants). First, any copy still out has to come back: bribe
  the chair's clerk (30 Bastion Favor), take the crew's recording from their radio shack past two
  lookouts, or hand in your own `orders.copy`. Whoever brings in the last copy gets +5 Bastion Fame.
  Ending: The Call Stands.
- **Read it to them:** persuade 3 (Champion, or Trusted and ★ Soldier).
  - One easier for each of: Act IV's true count; proof (`orders.copy` or B3's `pilot.line`); a Merciful
    or Idealist CO; HC1's `weeks.public`; and, once, `majors.pages_open` (the Red Hand's book read whole
    at settling day).
  - One harder with a Paranoid CO. Never easier than 2.
  - Then hold the stronghold's gate for 3 minutes while the loudspeakers carry the address and the
    noise brings two waves.
  - Bribe: an extra ration for every post (30 Bastion Favor) halves the morale hit.
  - With `orders.haven`, walk the chair's clerk to the radio past a checkpoint after curfew: one step
    easier.
  - Ending: All Stations.
- **Answer it yourself:** persuade 2 (Trusted, or Known and ★ Soldier). Known alone is enough with any of:
  - padded commanders reported in Act IV;
  - `haven.rule` 3, or B2's Counted;
  - proof the lab knew (`weeks.bastion` or `proof.day_one` 3);
  - a Ruthless or Paranoid CO.

  The CO signs off as the stronghold: *"Bastion, Command. Hold the line."* Then walk that night's
  curfew in a Haven town (Curfew Patrol): enforce it (+3 Haven Infamy at each notice), or walk it gently
  (★ Police, or Known with the Haven) for half the pay. Ending: Bastion, Command.
- **Betray: side with the Red Hand.** Needs Trusted with the Red Hand, or Known with `orders.rh_knows`,
  `orders.mocked` or B3's `pilot.redhand`. It always needs Trusted after `book.settled` 5: once Bastion
  has raided the rival's camp, the boss won't risk the relay with Bastion's guns out.
  - From 20:25, hold Act I's relay for the crew's radio man for 3 minutes while a Bastion squad comes
    for it (without the squad prefab, it's the horde). At 20:30 every post hears "expended" read over
    the CO's call.
  - Anyone on Bastion's side who breaks the hold gets +10 Bastion Fame, and the muster goes on.
  - Ending: Expended.

### Act VI. The last call and the last roll (coda)

One of the Last Rounds: [the codas in the epilogue week](#last-rounds-the-codas-in-the-epilogue-week)
has the calendar, the standing version and the no-cure case.

**When.** The epilogue's first nightly call (20:30), the same evening as the Haven's Gate; the call is
radio, so it doesn't take the evening's slot. Then the next 05:30 shift change. Default: the CO reads
the call alone, and Sedlak walks whoever's left, as data. It carries nothing into the next season but
its hall lines. During the final stand itself, the stance sets Bastion's part (see the endings).

**Goal.** Bastion's last nightly call of the season, and the walk to `day2.checkpoint`, where
Lieutenant Hana Sedlak's column was turned back on Day 2. Sedlak led the Day 1 cordon detail and
walked the Day 2 column (confirmed lore); she keeps the roster board at the stronghold, and she leads
the walk. The CO stays at the radio: *"Somebody has to be on the net."*

The column halts 150 m short of the barrier; much closer, and the voice past the line turns walkers
back. There Sedlak reads **the last roll**: the whole roster, out loud. The living answer *"Here."* For each of the
dead, everyone at the halt answers together. For a struck-off man, nobody answers. The loudspeaker reads
its processing order, and it doesn't say "Bastion". Nothing in the world changes. Sets `orders.last_line`.

With the finale's Bastion route ([FINALE.md](FINALE.md), §6.2), the captain walks beside Sedlak with his
empty box, and the four R numbers are read last, by name with `held.named`.

**Approaches**
- **Talk,** at the radio (20:30–22:30): read the last call: effective strength `{count}`, civilians
  sheltered, the new names on the nail. Close with the stance's line:
  - *"Command, Bastion. Holding at the barrier. Requesting relief. Over."*
  - *"All stations, all houses, this is Bastion. Civilians first. Bastion out."*
  - *"Bastion, Command. Hold the line."*
  - *"Effective strength {count}. Bastion out."*

  With the lab's site log in Bastion's file (`proof.pages` 4), the CO reads "Continue." after
  "Expended." The line goes into the season's record with your name.
- **Escort,** at 05:30: carry the box of tags from the nail to the barrier with Sedlak and one Bastion
  group (spawned only within 400 m of a player; Sedlak can't be killed). Tags brought home during the
  epilogue's Bring Them Home ([EPILOGUE.md](EPILOGUE.md)) go into the box if they reach the nail before
  05:30. Everyone at the halt goes on the "At the barrier" hall list.
- **Side with the Haven** (All Stations, or with `orders.haven`): bring the nearest safehouse's people to
  walk first (Refugee Column's walking group). It finishes Sedlak's Day 2 walk. +10 Haven Fame.
- **Bring a man home for the roll.** Each Deserter wanderer who hands his tag to the sergeant has his
  KIA rubbed out and answers *"Here"* at the last roll, and `{count}` goes up by one. If the finale's
  Bastion route brought Private Marek Fiala home (Twice; decision 3), he answers *"Here"* to his own
  name, the first time since Day 7.

### Endings

The stance the muster sets is the ending. What each stance changes in the finale's routes is in
[FINALE.md](FINALE.md), §12.

**The Call Stands** (stance 1)
- **You get:** whoever decided the muster: Bastion Fame +20 and 40 Bastion Favor. Whoever brought in
  the last copy gets +5 Bastion Fame more. Sedlak writes the decider's name on the roster: *"One
  civilian, attached."* A fallback pays nobody.
- **The world:** Bastion morale +6 and strength +3.
  - One Independent commander is pulled into line (loyalty +25; a seat on the ledger).
  - For the rest of the season, Bastion's presence never falls below a quarter (the same floor the
    finale uses for Bastion's route).
  - The Red Hand's *"Still waiting for relief? We'll keep the kettle on."* runs every night.
  - `orders.stance` 1.
  - Headline: *"Bastion read the numbers to Command last night, as every night. Effective strength
    {count}. No reply."*
- **The season:** at the stand, one Bastion group at the lab gate, and HC3's locked crate with a key.
  - Epilogue line: if Bastion controls the cure, *"Command, Bastion. Cure secured. Requesting relief.
    Over."* A long hiss. *"Bastion out."* If another faction does: *"At the north road, Bastion formed up
    at the barrier in its best order and read its roster out loud. Nobody came to relieve it."*
  - Hall of fame: "Bastion kept the call."
  - Next season (after a cure): Bastion's share of road posts is 0.6, never past Bastion's own winner value.

**All Stations** (stance 2)
- **You get:** whoever decided it: Bastion Fame +20, 30 Bastion Favor and Haven Fame +10. Whoever held
  the gate gets +5 Bastion Fame more. The reader is named in the season's story ("read the orders") and
  goes on the roster.
- **The world:** Bastion morale −8 at the address (−4 with the rations bribe), then +6 a session later.
  - Up to two men walk off to the nearest safehouse (+2 people; none if Act III already sent two).
    They're struck off as "KIA. Body not recovered", nobody answers for them at the roll, and Struck Off
    is weighted up at their post for a season day: Bastion sends someone to bring them home.
  - A Haven–Bastion truce, if faction life allows one.
  - Red Hand morale −3: they've lost their joke.
  - The address also reads *"Priority remains Institute site and personnel"*: Halcyon morale −3.
  - It counts as a page in the open (`majors.pages_open` +1).
  - `orders.stance` 2.
  - Headline: *"Bastion's commanding officer read the Day 3 orders on the open channel last night, all of
    them. Then the numbers, to everyone inside the line."*
- **The season:** Requisition ×0.8 (the shared per-house weight). Hearts and Minds ×1.2. Bastion's call
  opens *"All stations, this is Bastion"*. No curfew in Haven towns, even under Counted.
  - At the stand: one Bastion group at the lab gate, with no key (HC3's Bargain).
  - Epilogue line: *"When the gates opened, Bastion was at the north road, behind the column it walked
    there on Day 2."*
  - Hall of fame: "Bastion stopped calling Command."
  - Next season (after a cure): Requisition ×0.9.

**Bastion, Command** (stance 3)
- **You get:** whoever decided it: Bastion Fame +25 and 50 Bastion Favor, and a rank on Bastion's net
  (*"Sergeant {name}"*) for this life. Whoever enforced the curfew: Haven Infamy +10.
- **The world:** Bastion morale +8 and strength +5, inside the season budget.
  - The Rogue commander, if there is one, is pulled into line (a seat on the ledger).
  - Haven morale −8.
  - Command itself still never answers. The stronghold does.
  - `orders.stance` 3.
  - Headline: *"Bastion called Command at 20:30 last night. For the first time, someone answered. It was
    the stronghold."*
- **The season:** the curfew (the winner's curfew rule, this season only; the same curfew as Counted's,
  never applied twice). Requisition ×1.2 and Quarantine Line ×1.2, inside the shared 0.8–1.2 band.
  - At the stand: a Bastion checkpoint on the lab's road (Bastion's one group), and the CO demands the
    key to the clean room.
  - Epilogue line: *"At the north road, Bastion searched everyone who walked to the barrier, and wrote
    down every name."*
  - Next season (after a cure): Bastion strength +2.

**Expended** (stance 4)
- **You get:** whoever held the relay for the Red Hand: Red Hand Fame +20, 50 Red Hand Favor and Bastion
  Infamy +25, and the crew gives them a name (after something they'd rather it forgot). It's recorded
  *(you)* that they worked against Bastion. Whoever fought for the relay on Bastion's side: Bastion Fame
  +10. A fallback pays nobody.
- **The world:** Bastion morale −12 and strength −5, clipped at the season budget.
  - A Bastion–Red Hand feud under the usual rules: the stronger takes the weaker's towns with no
    strength margin, never a stronghold or a last outpost. An outpost that changes hands this way is the
    season's one major outpost.
  - Up to two Deserter wanderers, struck off like any deserter.
  - Bastion's channel is silent for one night while the Red Hand reads "expended" every hour.
  - Red Hand morale +5.
  - The Census can't start, and the Haven's cards lapse.
  - `orders.stance` 4.
- **The season:** Struck Off and Requisition ×1.2.
  - At the stand: Bastion sends nobody, and HC3's Under Bastion's guard can't happen.
  - Epilogue: each night the CO is alone on the net: *"Effective strength {count}. Bastion out."* If the
    Red Hand controls the cure: *"The first doses were sold at the north road, under a sign that says
    EXPENDED."*
  - Next season (after a cure): Bastion strength −2.

### Crossings

**Halcyon's major.** The stance settles Bastion's part at the final stand, whatever HC3 stage 3 said:
- 1: the crate, the key and one guard;
- 2: one guard and no key (the Bargain, at Known);
- 3: the key demand, and a checkpoint on the lab's road;
- 4: nothing, and Under Bastion's guard can't happen.

HC3's ending is read from what actually stood at the stand ([Proof's Act V](#act-v-the-final-stand-hc3-stage-6)).
At Halcyon's reading, the adjutant countersigns "for Command" under stances 1 and 3, "for Task Force
Bastion" under 2, and isn't at the table under 4. Halcyon's early flags feed the muster: `proof.day_one`
3 or `weeks.bastion` lets "Answer it yourself" go at Known, and `weeks.public` takes a step off "read
it". The site log in Bastion's file puts "Continue." next to "Expended." on the last call.

**The Haven's major.** It feeds the muster:
- `haven.rule` 3 makes "civilians sheltered" true and lets "Answer it yourself" go at Known;
- `orders.haven` brings the chair's clerk to the muster, and lets the Haven walk first in Act VI.

The stance reads back into the Haven:
- stance 2: the cards become "civilians first", with no curfew;
- stance 3: a curfew, and Ruthless and Paranoid heads lean to the cards;
- stance 4: the cards come off the table, and Counted ends.

**The Red Hand's major.** Settling day (50%) comes first:
- a whole book read out is a page in the open, one step off "read it";
- `book.settled` 5 makes the Expended betray need Trusted;
- `book.bastion` picks Act I's commander and finishes that post's count.

Expended feeds the Red Hand's "the roads are ours". With `orders.mocked`, R1's page goes back at Known
(built).

**B2 The Census** (now a story). It feeds the major: Counted makes "civilians sheltered" true, and
Forged is that post's padding. It reads the stance back:
- 1: the count goes to Command, as built;
- 2: the nightly call reads "All stations", and a forged count is found out 10% of the time instead of 25%;
- 3: the count goes to "Bastion, Command";
- 4: it can't start.

**B3 The Last Pilot** (now a story):
- `pilot.line` is the muster's strongest proof, and his name is read in the last call;
- `pilot.stays`: Bastion's group at the final stand is flown in (in words);
- `pilot.redhand` lets Expended go at Known;
- with stance 2 or 4 he has heard the orders, so talking him down needs one tier more.

**C1:** under stance 3 Bastion burns an exposed house within half a real day. Under stance 2 it waits
two real days and asks for the Haven's way first.

**Contracts:** The Real Orders stays off while the major runs (built `NotWithQuest`). No Reply picks
another post, and loses its "Command answered" choice once a stance is set. Struck Off never uses a
canon name from the finale (Fiala).

**Publishes** `orders.stance` (1–4), `.reckoning`, `.count`, `.padded`, `.rh_knows`, `.reported`,
`.false_hope` and `.last_line`.

### Engine needs

- B1's stage 5 returns Act IV instead of finishing, and its four endings are applied through `ActEnd`.
  `orders.ending` is set only at the major's real end.
- Act IV:
  - a per-post padding pick from what the world has;
  - `orders.count` as a text number worked out from Bastion strength and the reports;
  - `RZ_StoryArcs.StartFor(kind, outpostId)` for The Last Convoy (a headline under No Vehicles);
  - the disguise design's roster look (when disguise is built; until then the trick isn't offered).
- Readers of the stance:
  - `RZ_LeaderVoice` and `RZ_FactionVoice`: the call's opening;
  - a one-night mute of a faction's open channel;
  - `SetPresenceFloor(faction, 0.25)`, shared with the finale's Bastion route;
  - this season's curfew, set at runtime through `RZ_Seasons`;
  - season weights for Hearts and Minds, Struck Off, Requisition and Quarantine Line, through the
    shared helper.
- Canon: change `bastion.orders`' built rule to always "expended" (nothing reads it yet), and pin it when
  the major starts. "Command meant to come and couldn't" becomes only what some posts believe, said in
  rumours. Pinning it needs The Real Orders to read canon.
- Profile: a per-life "on the roster" flag (when that character dies, the operator reads the name with
  Bastion's dead), and a rank text for Bastion, Command.
- The last roll: Sedlak's reading at the halt, from the roster (the nail list, the roster and "struck
  off" are one record), with the R numbers and Fiala read from the finale's `held.*` and `Person` records
  when they exist.

---

## The Red Hand: The Book Has to Balance

### What it's built on

R1 The Book grows into it, and its key stays `book`, so saved runs and every `book.*` flag keep their
meaning. R1 is no longer offered on its own.
- R1's stages 1–2 become act 2, and its stages 3–6 become act 3. Its four endings become act 3's
  results through `ActEnd`; they keep their built consequences and count toward the major's caps.
- The new acts reuse built blocks:
  - acts 1 and 4: Our Crossroads' hold (with heat) and stand-ins;
  - act 4: Coup Talk's "laughed off or purged", Rein In, Raid the Red Hand and Desertion Route's escort;
  - act 5: Authors' stage 4, Steal the Antidotes' dose transfer, Smuggler's Run's courier reports and
    the choice base;
  - act 6: Authors' stand hooks and Take the Lab.
- Our Crossroads and Coup Talk are questline-stage classes only (conflict 36): out of the random pool,
  classes kept for this major.

**Changes that make it fit the season**
- The send-off moves to Collapse's second evening, so it doesn't share an evening with the Haven's
  family, and the crews have a reason to gather.
- The last page moves to 80%, after Halcyon's reading, so the lab's own line can be in the book.
- Settling day can hand over at most the season's one outpost.
- Sent off loud now also kills the Haven's arrangement and the lab's line.

**Canon.** It adds one place, Rudi's crossroads (`redhand.crossroads`), and one fact about the book:
its first pages are Rudi Kasper's diesel tally; its first line is the boss's own (*"Day 2. Ten litres,
on tick."*, marked *"Paid"* on Day 8 in the boss's hand); and under Rudi's last sum is *"Day 7. R.
Kasper. Bitten."* The line under the boss's ten litres is the stronghold cook's forty, pay later
(confirmed lore, [FINALE.md](FINALE.md), §6.3). R2 and R3 stay as the Red Hand's smaller stories.

### Premise

Nobody gives the Red Hand orders. The crews pay the boss a cut, and the boss holds the book. Its first
pages are Rudi's diesel tally, in biro, and the first line is the boss's own: *"Day 2. Ten litres, on
tick."* Beside it, in the boss's hand: *"Paid"*, added on Day 8. Under Rudi's last sum is one more line:
*"Day 7. R. Kasper. Bitten."* The boss has kept the book since Day 3, and ended two days of arguing by
reading each crew boss their own line. By the end every crew was paying into it. That's all the boss
is: the one who holds the book. "A debt is a debt" only means something because it's written down.

The cure is coming, and the crews know what it means. Once the line stands down, a toll is just a lorry
parked across a road, and the Red Hand will be nothing but what it's owed. On one back page that nobody
reads, the book holds who outside the Red Hand owes it, and for what: a Bastion commander for rifles, a
Halcyon field lead for live ones, a Haven council head for a quiet arrangement.

Rudi was never sent off: nobody wanted to stand at that shutter and make noise. Now, with the cure
coming, the boss calls every crew to Rudi's shutter at the crossroads to send him off loud, so nobody
asks how he died, and to read out over him who owed Rudi, and who owes the boss now. In front of every
crew, the book-keeper turns to the back page. It's gone. One crew boss didn't come, and laughs about it.
A boss who can't keep the book in front of every crew doesn't stay boss for long.

So at half research the boss calls a settling day: the whole book read out, and it has to balance. Then
comes the last page: what the Red Hand will be owed when the gates open.

### Why it's the Red Hand's major

This answers the author's question: why does the book matter? The book is the Red Hand's only law, its
bank, and the boss's whole claim to the chair. The boss doesn't need a say: the boss has the book. So
losing a page in front of every crew reopens a question nobody has asked since Day 10: who's boss?

The cure makes the stakes bigger. When the line stands down, the tolls stop meaning anything, and the
book is written proof of who owes the Red Hand, in every faction. So every stronghold leader cares:
- the CO wants the rifles line out of the Red Hand's book, because Bastion covers for its own;
- the chair wants a council head's line burnt, along with the Haven's shame;
- the director wants the field lead's line kept out of the lab's record, and has to decide whether
  the cure goes into the book.

The boss is in every act and is never replaced. The coup always fails, at most one crew walks away, and
the faction never splits. The major decides:
- whether the Red Hand reaches the end as one crew;
- how it turns up at the final stand: guards the lab road, takes the room, or sits it out;
- the line the epilogue and the next season remember.

### Act 1. Send him off

**When**
- Collapse's second evening. Offered 16:00–21:00 game time, with the send-off at dusk (18:30), at
  Rudi's crossroads. It's a set piece.
- The rival is picked now: the Rogue crew boss, else the Independent crew boss with the lowest
  loyalty, else the boss's second (a named person).
- **Clock:** patience 2 offered evenings; anchor settling day. Default: the crews send Rudi off without
  players (a headline). The page is found missing the same way, and act 2 opens.

**Goal.** Send Rudi off loud at his shutter, at the crossroads where the first toll was taken: horns,
music, shots in the air, so nobody asks how he died. Then the boss reads out who owed Rudi, and who owes
the boss now, and that's the book. At the reading, the back page is gone, in front of every crew. Sets
`book.sendoff` (how many crew bosses came and stayed, 0–3) and `book.rival` (the rival's seat + 1).

**Approaches**
- **Talk:** ask a crew boss to come, at their seat (persuade 1 with the Red Hand). A Ruthless or
  Paranoid crew boss needs Trusted. ★ Ex-Con counts as one rank higher. Loyal crew bosses come on their
  own. The rival never comes, and says he's busy.
- **Bribe:** 15 Red Hand Favor brings a Greedy crew boss who would have stayed away.
- **Fight:** hold the crossroads through the send-off while the horns bring the infected (4 minutes, 3
  waves; Our Crossroads' hold, with heat). A crew boss counts only if the hold stands. If it's held with
  two or more crew bosses there: Red Hand morale +3, and the boss remembers you helped.
- There's no betray or side-with approach here, on purpose: one player shouldn't decide the season this
  early. Whatever players do, the page goes missing in front of every crew.

### Act 2. The torn page (R1 stages 1–2)

**When**
- After act 1 (R1's own start, from the second real day, is always met by then). Needs Known with the
  Red Hand, and leaders of two other factions for the page (Rogue ones first). About one session.
- **Clock:** anchor settling day; no patience. Default, when it's squeezed: the thief burns the
  page himself and runs (`book.burnt`, `book.thief` 5). Last call: *"Somebody's selling a page at a
  barn. Not for long."*

**Goal.** Find the thief and the page. One change from R1: the thief now has one motive. His sister's
house is the Haven house on the page, and the rival paid him a tin of diesel and promised to strike her
house off once the book was in better hands.

**Approaches**
- **Talk:** point at the liar among three of the crew (The Rat; ★ Police or ★ Ex-Con: the night book
  clears one of them). Pick wrong and the wrong man goes in the pit (+5 Red Hand Infamy,
  `book.wrong_man` *(you)*), as built.
- **Persuade:** talk him out of the barn, unarmed (Known, or ★ Ex-Con). New: he also names who paid him
  (`book.rival_known` *(you)*).
- **Fight:** take him while his two friends fight you. He tells you about his sister, but not about the rival.
- **Bribe:** 20 Red Hand Favor and he runs (`book.thief_ran`). He leaves the page at the barn.

### Act 3. Who gets the page (R1 stages 3–6)

**When**
- Straight after act 2: first R1 stage 3, then a new wait of one session while word gets round, then
  stage 4 with three new offers, then stage 5 at the boss's pit. Stage 6 (the boss hears) still follows
  a kept page, as built.
- Outcomes pay as R1 does now and count toward the major's caps. Leader changes go through the
  season's seat ledger.
- **Clock:** anchor settling day; no patience. Default when squeezed: the page stays out (settling day
  reads it as still out), and the boss's men put the thief in the pit.

**Goal.** Decide whose hands the page ends up in. That decides what settling day can be.

**Approaches**
- **Read the page** (built). For each line: collect it by talk (60%), lean on them (+3 Infamy with their
  faction), pay it yourself (15 Red Hand Favor), or let it lie. Or carry one line home to its own
  stronghold: that leader is replaced if the seat is free on the ledger, that faction's morale −3, +10
  Fame with it and +15 Red Hand Infamy. New:
  - each line dealt with puts out a headline, so that stronghold learns one of their own is in the book;
  - `door.debt_yours` adds your own line, and `walk.cut_dose` adds the cut dose to the Haven's line;
  - with `haven.rule` 4, the paying houses are on the page.
- **Talk:** give the page back to the book-keeper (Trusted, or Known with `orders.mocked`): +20 Red Hand
  Favor (`book.returned`).
- **Side with another faction.** As built: give the whole page to Bastion's duty officer (Known with
  Bastion; `book.bastion`). New: burn it at the chair's table (Known with the Haven); the chair pays 30
  Haven Favor for the ash (+15 Red Hand Infamy; `book.burnt`). The chair doesn't bid once the Haven's
  List has been read: *"Give it back and let the boss choke on it."*
- **Betray:** keep it (`book.kept` *(you)*), as built. New: or sell it to the rival for 40 Red Hand
  Favor (`book.to_rival`).
- **Sneak** (new): forge it at the barn, unseen (60 seconds, nobody within 30 m): the right names and
  the wrong sums. The forgery goes to the rival and the real page goes back in the book (`book.forged`).
  Without ★ Police or ★ Ex-Con, the forger's handwriting is spotted at settling day 35% of the time.
- **The thief** (built stage 5). Let him go in the pit. Or take his place in the ring (4 minutes, four
  rounds of three infected). New: the crews give whoever does this a crew name for the life (after the
  ring, which they'd rather you forgot), and their word counts one step more at settling day
  (`book.named` *(you)*). Or pay his debt (30 Red Hand Favor). Or get him out to a safehouse (+1 person,
  +5 Haven Fame).

### Act 4. Settling day (the Red Hand's climax)

**When**
- At 50% research, at least one session after act 3. It's the first of the season's four climaxes.
- Offered 16:00–21:00 at Rudi's crossroads, with the reading at dusk (18:30). It's a set piece, and it
  waits out an Under Pressure stronghold.
- If the rival has left his chair since act 1, the crew boss who is now least loyal speaks instead, and
  `book.rival_known` no longer counts.
- **Clock:** patience 2 offered evenings, or 6 offered hours; a lead may open it from 45% when research
  stalls. It must resolve before the muster can run. Default:
  - with the page in the book, the rival is laughed off;
  - otherwise he's purged half the time (seeded);
  - a real page in his hands comes back into the book half the time.

**Goal.** The whole book is read out at the crossroads before the cure, loud on purpose. The rival uses
the reading to ask who should hold the book. The coup always fails. Rudi's death is never the rival's
weapon: every crew already knows. The players decide how it fails, and what that costs.

Sets `book.settled`: 1 laughed off, 2 purged, 3 pulled into line, 4 walked to the Haven, 5 camp raided by
Bastion, 6 stood with. It also records whether the book is whole. A whole book read aloud, back page and
all, is the season's first page in the open (`majors.pages_open` +1).

**Approaches**
- **Fight:** hold the crossroads while the book is read (4 minutes, 3 waves drawn by the noise). The
  crossroads is outside the camp, so the rule that guns stay quiet in camp isn't broken. If the hold is
  lost, the reading breaks up, the rival's word lands, and the boss's men walk him off anyway (Red Hand
  morale −3).
- **Talk or persuade** when the rival speaks (Trusted, or Known with ★ Ex-Con). One step easier with a
  crew name, with two or more crew bosses at the send-off, or after R3's `well.stood` or R2's
  `cargo.purge`. One step harder after R3's `well.done`. What you can say depends on where the page is:
  - page in the book: name him (with `book.rival_known`, he's purged); speak for him (+25 loyalty, and he
    owes you); or laugh it off with the crews (anyone Known);
  - the forgery in his hands: name the fake (purged). If the forger's handwriting is spotted instead: +15
    Red Hand Infamy for the forger, and the rival is laughed off;
  - the real page in his hands: turn on him, and the book is whole again (purged);
  - kept by a player: the keeper can bring it home now (Trusted), at half the Fame;
  - burnt, or with Bastion: own it, or blame him (needs `book.rival_known`);
  - if R2's `cargo.cut` is running, raise it (Trusted): the crews turn on him (purged).
- **Betray: stand with the rival.** The coup still fails: the boss's crew comes for him at the
  crossroads. This needs the Red Hand group prefab; without it, the fight isn't offered. Win the fight and
  he goes Rogue and keeps his chair, and Rein In follows. Lose it and he's purged. Either way, +15 Red
  Hand Infamy. Or sneak: if he holds the real page, slip off with it in the noise past two lookouts
  (`book.kept` *(you)*).
- **Side with Bastion:** tell the CO (Known with Bastion) that every crew boss is at the crossroads at
  dusk. That night Raid the Red Hand hits the rival's half-empty camp. His outpost goes to Bastion only
  if all three hold: Bastion's strength beats the Red Hand's by 10, the territory rules allow it, and no
  outpost has changed hands through a major this season. Otherwise, Red Hand strength −3. +10 Bastion
  Fame, +15 Red Hand Infamy. Bastion never shells a gathering.
- **Side with the Haven** (Known with the Haven, once he's been laughed off or purged): offer the
  rival's crew a Haven door. He and up to three of his people paint out the hands on their lorries and
  walk to a safehouse (Desertion Route's escort; up to +3 people, never past the house's cap). A new crew
  boss sits down, so the outpost stays Red Hand. +10 Haven Fame, +10 Red Hand Infamy. Only ever this one
  crew.

### Act 5. The last page

**When**
- At 80% research, one session after settling day, and after Halcyon's reading has resolved. At the
  boss's seat at dusk.
- First refusal: for its first 30 minutes with them online, it's offered only to whoever decided
  settling day (else whoever returned the page). If they aren't on within a session of play, or the 30
  minutes pass, anyone Trusted with the Red Hand can take it (or Known with `orders.mocked`).
- It reads Authors' stage 4 outcome if that has run; if Authors is waiting at stage 4, one mission
  serves both.
- **Clock:** patience 2 offered evenings; a lead may open it from 75% when research stalls (never before
  the reading). Hard stop: the gate. Default: the boss's traits decide (read only; the boss never
  changes):
  - Paranoid or Pragmatist: the boss sends the book off loud alone;
  - Greedy: sells it by the page if it's whole, else takes what's owed;
  - Ruthless: takes what's owed;
  - any other boss: sends the bill by their own man if the book is whole, else takes what's owed.

**Goal.** The boss opens the book at Rudi's tally and asks what the last page should say: what the Red
Hand is owed when the gates open. The answer picks the ending, and the stand plays it out.

A whole book can be balanced, sold or burnt. A broken one (the page burnt, with Bastion, or still out)
can only be burnt, or the boss takes what's owed. If the director's pages were sold to the boss's man
at Halcyon's reading (`proof.pages` 5), the lab's own line is in the book now too.

Sets `book.last`: 1 bill paid, 2 stood for nothing, 3 sold, 4 sent off, 5 to Bastion, 6 the boss takes.

**Approaches**
- **Talk** (whole book): keep it balanced and write the cure in. Carry the bill to the director (Known
  with Halcyon, or Trusted with the Red Hand; ★ Lab Tech or ★ Doctor). The live ones, the stolen cases and
  the field lead's line are all forgiven, in return for two doses (Steal the Antidotes' transfer, within
  its cap) and the lab road at the stand. The director refuses if Bastion already guards the lab
  (`orders.stance` 1 or 3). If the director turned the boss down once already in Authors, the bill needs
  Trusted with Halcyon. Ending: The book balances.
- **Persuade the boss to stand for nothing** (Trusted, or Known with ★ Ex-Con): *"A debt's a debt. We hold
  the road, and the cure owes us."* A Red Hand group stands on the lab's road at the stand, and the lab
  pays nothing. This works after the director refuses, and after `authors.batch_sold`. Ending: The book
  balances.
- **Bribe or talk** (whole book): sell it by the page. Each faction's line goes to its own stronghold:
  three deliveries in 60 minutes. Everyone is told a courier is out, with a rough "last seen" every few
  minutes, as in Smuggler's Run. A line taken off the carrier can be handed in somewhere else; handed to
  Bastion, it goes into Bastion's file (+10 Bastion Fame, +15 Red Hand Infamy). Ending: Sold by the page.
- **Persuade: send it off loud** (persuade 2): *"When the gates open, that book is a confession, and its
  first page is your own."* It's persuade 1 with a Paranoid or Pragmatist boss, if the thief lived, or if
  the book is already broken (*"If one page is ash, the rest can follow"*). Ending: Sent off loud.
- **Betray** (sneak): take the whole book off the boss's table during the crews' supper (18:30), past two
  lookouts, and walk it to Bastion's duty officer (Known with Bastion). No disguise works at the boss's
  table. If you're caught: +25 Red Hand Infamy, and the act stays open. If you deliver it: +25 Red Hand
  Infamy, enough to bring hunter squads. Ending: Bastion's file.
- **Side with Halcyon:** tear up the bill at the lab gate, or warn the director (+10 Halcyon Fame, +10 Red
  Hand Infamy). One more Halcyon guard stands at the gate for the stand, and the boss takes what's owed
  instead (`book.last` 6).
- **Talk** (personal; Trusted; with any answer but the theft): ask the boss to keep the first page out of
  it (`book.first_page` *(you)*). The epilogue line gets one extra clause. The first page keeps the cook's
  line too, so the finale's "Mark my line paid" still has a line to mark.

### Act 6. At the lab road

**When.** The finale's stand. **The finale owns the stand**: its rules, its Fame and who controls the
cure ([FINALE.md](FINALE.md), §9). This act only says how the Red Hand turns up, from `book.last`, and
which of the majors' two group places it takes. Sent off loud plays before the stand instead, at the
next dusk after act 5.

Take the Lab under a finale is the Red Hand route's once-a-season theft. It's offered only when the
last page allows it (below). Any Red Hand-locked player who is Trusted with the Red Hand can start it,
and every Red Hand-locked player counts for the bay hold. It makes the Red Hand route the holder of the
clean room (decision 6, conflict 11). With no finale registered, built Take the Lab is unchanged.

**Goal.** The Red Hand meets the night the cure is made the way the last page says, and the whole server
sees it. Sets `book.stand`: 1 the crews held the road, 2 took the room, 3 the take was beaten off, 4 nothing.

**Approaches**
- **The book balances:** fight beside the Red Hand group on the road from the Red Hand toward the lab
  through the stand. Take the Lab isn't offered at this stand. If Bastion's group is on that road
  (`orders.stance` 3, or Authors' road group), the boss won't stand beside Bastion's guns: the Red Hand
  group doesn't come, and the ending's stand clause becomes "warned off". Anyone Known with the Red Hand
  who stays on the road for the whole stand is counted for the ending.
- **Sent off loud:** before the stand, hold the fire at Rudi's crossroads for 3 minutes (2–3 waves) while
  the boss burns the book the way the Red Hand sends off its dead (an optional bonfire prop, removed
  afterwards). At the stand the Red Hand sends nothing, and Take the Lab isn't offered.
- **The boss takes** (`book.last` 6), **Bastion's file** (5), or **the book doesn't balance:** Take the Lab
  is offered to the Red Hand route, with the finale's bay hold (120 s; 90 s with `authors.batch_sold`).
  Done, it's `book.stand` 2; never done by the stand's end, it's `book.stand` 3. Stopping it is the
  finale's control fight: the holder's side keeps a player within 15 m of the bay. With Bastion's file,
  Bastion's road group is out on the Red Hand's road too, unless the stance is Expended and if a group
  place is free. With no finale registered: the built result (research back to 90%, two doses to the
  black market, the cure about a day late).
- **Sold by the page:** nothing happens. The boss says, *"We've been paid."* Take the Lab isn't offered.

A season that wipes without a cure ends the major at the wipe warning, on its current branch.

### The last line (coda)

One of the Last Rounds: [the codas in the epilogue week](#last-rounds-the-codas-in-the-epilogue-week)
has the calendar, the standing version and the no-cure case. The author's closing idea for it: the Red
Hand closes the book, paid or not.

**When.** The epilogue's first free evening after the paper's sealing (usually the third), at dusk
(18:30), at Rudi's crossroads, with the lorry and the book (optional props: the Red Hand lorry, else the
Red Hand truck, else the puzzle object). Open to anyone the Red Hand doesn't hunt; a hunted player can
listen from 100 m. It runs once, can't change the ending, and carries nothing into the next season but
its hall lines.

**Goal.** The boss closes the book at Rudi's shutter. First the last line, by the ending (below). Then
the lines still open, read out with their names, paid or not: the boss writes *"Paid"* beside each one
that is, in front of every crew, and reads the rest out unpaid. Nobody is ever out of the book, and
tonight everyone hears it.

**Approaches**
- **Talk: your own line.** Anyone with an open line (a crew name's debt, `every_name.debt_yours`,
  `proof.debt_yours`, a price door's debt) can stand at the lorry for it. Paid off by now (the epilogue's
  Settle Accounts, [EPILOGUE.md](EPILOGUE.md)): marked *"Paid"* aloud. Not: read out unpaid. Words only.
- **Fight:** hold the crossroads while the horns go for the season's Red Hand dead (2 waves; ordinary hold
  pay). Their names are read before the book is closed.
- **Talk** (Trusted, or with a crew name): ask the boss about the first line. With `book.first_page`, the
  boss says *"No."* and closes the book. Without it, the boss reads it flatly, *"Paid."* is beside it, and
  the boss says what the boss always says: *"He let people pay later. You can't run a road like that."*
- **With the finale's `cook.line_paid`:** the cook stands at the lorry, and the boss reads her line on the
  first page: *"Paid. On the first page, again."*

**By ending**
- The book balances, or Sold by the page: the boss reads the last line from the book.
- Sent off loud: there's no book. The lorry is empty, the horns go anyway, and the boss says *"Nothing
  written down."* Only the fight and the season's dead are left. With `book.first_page`, the first page
  is the only paper on the lorry, and the boss doesn't read it.
- Bastion's file: the book is in Bastion's cupboard; the boss reads the open lines from memory.
- The book doesn't balance: the boss reads up to the torn page and stops.

### Endings

What each ending changes in the finale's routes is in [FINALE.md](FINALE.md), §12.

**The book balances**
- **You get:** everyone who took part in two or more acts: +10 Red Hand Fame. Whoever answered at the
  last page, and whoever decided settling day: +15 more each (25 at most). The one who answered also gets
  40 Red Hand Favor. Each of them gets a crew name for the life, if they don't have one yet, and the boss's
  quartermaster charges them ×0.9.
  - Bill paid: whoever carried the bill gets +10 Halcyon Fame.
  - Stood for nothing: everyone Known with the Red Hand who held the road for the whole stand gets +10
    Halcyon Fame.
- **The world:** Red Hand morale +7 and strength +3, inside the season budget with acts 1 and 3. The rival
  is purged or pulled into line, never both. Debt Collection ×1.2 for the season.
  - Bill paid: the two doses have already moved.
  - Stood for nothing: a Halcyon–Red Hand truce after the stand, if faction life allows it.
  - Headline: *"The Red Hand read its book out at Rudi's crossroads. It balanced. The last line is the lab's."*
- **The season:** epilogue line, by who controls the cure:
  - the Red Hand: *"The cure was sold to the highest bidder. The book said who bid first."*
  - another faction: *"When the gates opened, the boss of the Red Hand walked out with a book under one
    arm, and everyone in it watched."*
  - no cure: *"The gates never opened. The book balanced anyway."*
  - Hall of fame: "The book balanced", with up to four names and their crew names.
  - Next season (after a cure): Red Hand strength +2, and a returning player is greeted by last season's
    crew name (words only).

**Sold by the page**
- **You get:** whoever arranged the sale: +15 Red Hand Fame and 60 Red Hand Favor. Whoever carried a line
  home: +10 Fame with that line's faction, and that leader remembers them as having saved them.
- **The world:** Red Hand strength +3 and morale +3. For each line bought home: that leader +15 loyalty (a
  seat on the ledger), and that faction's morale +3. Where each line goes:
  - Bastion's line: into the stove under stance 1, read on the call under stance 2, into Bastion's own
    file under stance 3;
  - the lab's line: into the director's stove (`proof.pages` goes from 5 back to 1);
  - the Haven's line: under Every Name, the chair reads it at supper (Haven morale −3; `book.strays_read`);
    otherwise it goes in the drawer.
  - At the stand, the Red Hand sends nothing.
  - Headline: *"Three strongholds bought their own lines back from the Red Hand this week. Nobody asked
    what the Red Hand kept."*
- **The season:** epilogue line, by who controls the cure:
  - the Red Hand: *"The cure was sold to the highest bidder. Everyone already knew the price."*
  - another faction: *"When the gates opened, everyone in the Zone had bought their own page back from
    the Red Hand."*
  - no cure: *"The gates never opened. Everyone's page is in somebody's stove."*
  - Next season (after a cure): the cross-faction trade jobs are offered ×1.1 as often.

**Sent off loud**
- **You get:** everyone who held the fire: +15 Red Hand Fame and +10 Haven Fame. Whoever talked the boss
  into it: one item from the Red Hand shelf at their tier (*"His kit's on the market before the horns
  stop. Have first pick."*). Nobody takes Red Hand Infamy, because the boss chose it. Every leader who was
  on the page remembers everyone who took part as having saved them.
- **The world:** Red Hand morale −5 that night, then +5 when the fire is out. "The night the book burned"
  becomes a remembered moment. Every debt dies with the book (`book.debts_dead`), because when the gates
  open that book is a confession, and its first page is the boss's own:
  - Protection Racket and A Quiet Arrangement ×0.8;
  - the Haven's paying houses stop paying;
  - the lab's line in the book is ash, so Two Tins a Copy can only come from the road;
  - a Haven–Red Hand truce, if faction life allows. It counts as a page in the open.
  - Headline: *"The Red Hand sent its book off loud at Rudi's crossroads last night. Half the infected in
    the valley came to see. Nobody owes anybody now."*
- **The season:** epilogue line: *"When the gates opened, the Red Hand walked out with nothing written
  down. Some of them went home."* With `book.first_page`: *"All but the first page. The boss kept that."*
  - Next season: nothing carries over, on purpose.

**Bastion's file**
- **You get:** whoever handed it over: +20 Bastion Fame (+5 more when it's read on the nightly call) and
  30 Bastion Favor. Red Hand Infamy +25 in all, and it's recorded *(you)* that they worked against the Red
  Hand. For the rest of the life, Red Hand Talk checks need one tier more, and Red Hand leaders open with
  *"You're the one who gave Bastion the book."*
- **The world:** Red Hand morale −10 and strength −5 over the whole line (R1's built −3 counts toward it);
  Bastion morale +5.
  - The rival's outpost goes to Bastion only if it's still the season's one major outpost and the
    territory rules allow it.
  - A Bastion–Red Hand feud. There's one per pair: if Expended already started one, it's extended, not
    doubled.
  - The commander on the page gets +25 loyalty, unless `orders.stove` is set: then the CO burns that line
    first.
  - Bust Out ×1.2.
  - At the stand, Take the Lab is offered to the Red Hand route.
  - At 20:30, the nightly call: *"Command, Bastion. Attached, the Red Hand's book: [n] names, [n] debts.
    Over."* A long hiss. *"Bastion out."*
- **The season:** epilogue line: *"When the gates opened, Bastion handed over a ledger with half the Zone
  in it. The Red Hand's names were on top."* With no cure: *"Bastion keeps the book in the cupboard by the
  stove. Its own lines aren't in it any more."*
  - Next season (after a cure): Bastion strength +2 and Red Hand strength −2.

**The book doesn't balance**
- **You get:** whoever kept the page: 20 Favor once from each faction collected from (60 at most, as
  built), then +15 Red Hand Infamy when the boss hears. Whoever stood with the rival has already taken +15
  Red Hand Infamy.
- **The world:** reached when the page is still out after settling day, or when it was burnt and the boss
  took what's owed. Red Hand morale −5. The leader leaned on hardest swings 25 toward Rogue (a seat on the
  ledger; never more than one Rogue per faction). The Rat ×1.2. At the stand, Take the Lab is offered to
  the Red Hand route.
  - Headline: *"The Red Hand's book doesn't balance. The boss is asking who has the page."*
- **The season:** epilogue line: *"When the gates opened, the Red Hand's book still didn't balance.
  Somebody had the page."*
  - Next season (after a cure): the new boss is seeded with the Paranoid trait (one of their two traits),
    and The Rat ×1.1.

### Crossings

**Halcyon's major.** The last page comes after Halcyon's reading:
- If the director's pages were sold to the boss's man and nobody cleared that line (`proof.pages` 5), the
  lab's line is in the book. The book balances forgives it in the bill, Sold by the page sells it back to
  the director, Sent off loud burns it, and Bastion's file hands it to the CO (`proof.pages` 4).
- Authors' stage 4 is read as built. `authors.batch_sold` means the boss has already paid (the default is
  to take what's owed, unless someone persuades the boss to stand for nothing). A turn-down makes the bill
  need Trusted with Halcyon. "Bastion told" puts Bastion's road group on the Red Hand's road.
- `weeks.public` makes the field lead's line worth less: the bill is one step harder, and in Sold by the
  page the director pays half.
- Publishes `book.halcyon_line` and `book.stand`.

**Bastion's major.** Settling day (50%) comes before the muster (60%):
- a whole book read out is a page in the open;
- `book.settled` 5 makes the Expended betray need Trusted;
- `book.bastion` picks Bastion's Act I commander and finishes that post's count.

From Bastion:
- `orders.mocked` lets returning the page and answering at the last page go at Known;
- `orders.stove` burns the rifles line first;
- Bastion's file after Expended pays +5 more Bastion Fame, because the CO wants it.

**The Haven's major:**
- The Haven line is a council head's quiet arrangement, and the thief's sister's house. The chair bids for
  the ash in act 3, but not after the List has been read.
- A line turned in to the chair lets the chair make that head step down at the second sitting.
- Under Paid at the Door, the paying houses are a line in the book; Sent off loud (`book.debts_dead`) ends
  their cost.
- With `list.to_redhand`, the strays came off a Haven list: in Sold by the page the chair pays more, and
  the supper reading costs Haven morale −5.

**R2 and R3** (now stories). While the major runs, they pick the rival's crew when they can:
- R3's `well.done` makes speaking against him one step harder; `well.stood` or R2's `cargo.purge` one step
  easier.
- If R2's purge or R3's blame takes the rival out of the chair, a new rival is picked, and
  `book.rival_known` stops counting.
- A running `cargo.cut` can be raised at settling day.
- A Rein In on the rival counts as pulled into line.

**The Long Table:** with `cargo.buyer` 1, a Trusted player can make the boss read the jar money at settling
day (persuade 2). That crew boss is purged (the act's one leader change), the house is found out to the
Red Hand, and C1 starts at stage 4.

**The tribes** ([INFECTED.md](INFECTED.md)): between beats, one of the major's rumours is the book's line for
"the crew in the woods": the moat's feeding.

**Publishes** `book.sendoff`, `.rival`, `.rival_known`, `.to_rival`, `.forged`, `.settled` (1–6), `.last`
(1–6), `.stand` (1–4), `.halcyon_line`, `.bastion_line`, `.debts_dead` and `.strays_read`, plus the built
`.returned`, `.kept`, `.burnt`, `.bastion`, `.thief`, `.turned_in` and `.wrong_man`. `book.named` and
`book.first_page` are *(you)*.

### Engine needs

- R1's Finish calls become `ActEnd` outcomes. Other R1 changes: stage 2's talk names the rival; stage 3
  puts out a headline per line; stage 4 gets the chair's table, the sale to the rival and the forgery;
  stage 5 gives the crew name; and a new one-session wait goes between stages 3 and 4.
- Canon `redhand.crossroads` (+ `.town`): the road junction nearest the Red Hand stronghold's seat, 300–800
  m out, so it's outside the camp; not within 400 m of a safehouse or the lab, and off the line. If there's
  none, Our Crossroads' spot finder.
- Optional props on `RZ_GameModeComponent`: **Questlines > Red Hand lorry** (else the Red Hand truck, else
  the puzzle object) and **Questlines > Bonfire prop**. Both spawn only near players and leave no trace;
  both need WORKBENCH_SETUP entries.
- Stand-ins for each crew boss at the crossroads, and a hold whose heat comes from the horns and shots.
  VERIFY: whether sound events or a radio prop can be the noise source; otherwise it's heat only.
- The rival, kept by seat and generation (as `RZ_RedHandQuestHelp` does for R3) and picked again if he
  leaves the chair. The forgery as a run flag, with the 35% check.
- A crew name kept on the profile for the life. Bastion's file as a one-off line on Bastion's 20:30 call.
- Take the Lab's offer goes through the finale's controller, to the Red Hand route only, and reads
  `book.last` (one decider per stand: the last page overrides Authors' stage 4).
- The last line's open-line reading: every open line by identity (the book's player lines and the price
  door's debts), read once.

---

## Halcyon: Proof

The major keeps the name "Proof" (decision 15). Halcyon's finale route is The Lower Store, and it never
re-decides the paper's first page: that belongs here.

### What it's built on

It takes in two of Halcyon's three questlines as inner lines, keeping their keys, flags, saves and stage
missions:
- HC1 The Three Weeks (`weeks`) runs whole as Act II.
- HC3 Authors (`authors`) runs as Act III (stages 1–4), alongside Act IV (stage 5, Ready the lab), and as
  Act V (stage 6, the stand).

Both pay their own endings as built. The edits they need: one new choice in HC1 stage 4 (keep the
director's pages); and HC3 loses its 90% cut-off, waits for a won stand, publishes its terms, and reads
the finale for its ending.

The major's own acts are built on `RZ_QuestHalcyonScene`:
- Act I is Patient Zero's Ledger as a stage, handed in at HC1 stage 4's seats.
- Act IV is the reading at the lab, a night in HC2's records room, and the boss's man behind A Quiet
  Arrangement's lookouts.
- Act VI is an escort to the barrier after the line stands down.

**Changes that make it fit the season**
- The reading stays on 75% and is the last of the four climaxes.
- Who sits at the reading now comes from the other majors' flags.
- Bastion's stance, not HC3 stage 3 alone, decides Bastion's part at the stand.
- Act VI's table goes to `day2.checkpoint`.

HC2 The Other Two stays a story. It's Halcyon's only one; a second, from The Last Experiment's Lazarus
terminal, is only an idea so far.

### Premise

On Day 2 the Institute's last call reached the lab: nobody leaves until the Zone is proven clean, and
proof opens the gates. Since then the research director has been writing that proof. It's a paper with
every reading, every volunteer and every author in it, and it's the only thing from this Zone that will
ever go out.

The paper's first page is a date: Day 1. The clinic's file says the first case came in three weeks
before the gates closed, with "notify the Institute" underlined twice. Dr Wolff's notebook says the
Institute was told, and wrote back one word: "Continue." Three pages torn from the site log, in the
director's own hand, say what the lab did with it: *"Day −21. Institute advises continue. Continuing."*
The director's initials are under every day after that, for three weeks. Wolff took those pages up the
hill on Day 3 and never came down.

The director continued because they were told to, and because those three weeks are why the lab had
anything to start a cure from. Nobody here is a villain: the director believes a paper that starts with
that word won't be believed. As the cure climbs from the first day to the final stand, the Zone decides
three things: what goes on the first page, who stands at the lab when the batch is made, and whose hands
the paper leaves in. The line never says what the site was for, and never makes the outside act.

### Why it's Halcyon's major

The cure is the only way a season ends, and the lore says the cure isn't just medicine: it's proof. This
paper is that proof, and writing it is why Halcyon is in the Zone. So this is Halcyon's season, and it
runs on the cure's own clock: Day 11, 25%, 50%, 75%, the stand, the epilogue.

The stronghold leader is personally at stake: their signature, their three pages, and whether their name
goes first or last on the paper. The director is never replaced; players can only make them answer.

Every other stronghold leader wants something concrete from the paper:
- the CO wants proof that the lab knew, and a signature on the cure;
- the chair wants the truth about the three weeks, and names instead of numbers;
- the boss wants something to sell.

What players give them decides who sits at the director's table and who stands at the lab gate. The
ending is the line the epilogue repeats, the hall of fame's record of who is on the paper, and one small
rule the next season starts with. Next to it, The Other Two is one family's story.

### Act I. Day One

**When**
- Early Outbreak, from the season's first change of shift until 25% research. It's the director's radio
  contract, for players Known with Halcyon.
- While the major runs, Patient Zero's Ledger and Patient Zero aren't offered on their own. Solving
  Patient Zero also finishes this act, the long way round.
- **Clock:** anchor 25%; no patience. Default at 25% (after its last call), which pays nobody: the field
  lead finds the file herself, and the date stays the lab's (`proof.day_one` 1).

**Goal.** The opening line, on Halcyon's open channel and as a headline: *"Day 11. Overnight numbers:
research at {cure_percent} percent. The paper has a first page. It says the outbreak began on Day 1."*

The clinic in `{canon_town:origin.clinic}` has a file that disagrees. The stage is Patient Zero's Ledger
as built (the ledger, three bracelets, naming who was first), handed in at one of HC1 stage 4's seats. It
pays Fame and Favor, not research, and sets `weeks.records`. This act decides who knows the date
(`proof.day_one`), and every later act reads it.

**Approaches**
- **Talk:** hand it in sealed to the duty officer, for the director. Day 1 stays (`proof.day_one` 1). +10
  Halcyon Fame, 15 Favor.
- **Persuade** the director through the duty officer (difficulty 2: Trusted, or Known as ★ Doctor or ★ Lab
  Tech). The director reads the date on the open channel at the change of shift, and nothing more
  (`proof.day_one` 2). +8 Halcyon Fame, 10 Favor, +5 Haven Fame; Halcyon morale −3.
- **Sneak:** before handing it in, copy the margin onto the field lead's desk, unseen (`proof.copy`
  *(you)*). Act II opens at once. Seen: +5 Halcyon Infamy, and no copy.
- **Side with another faction** (the contract then pays nothing):
  - Bastion: its adjutant files it, so the CO has proof the lab knew (+10 Bastion Fame, +10 Halcyon
    Infamy; `proof.day_one` 3). At Bastion's muster, "Answer it yourself" then goes at Known.
  - The Haven: the chair's clerk takes it to supper (+10 Haven Fame, +5 Halcyon Infamy).
  - The Red Hand: the boss's man reads it on the open channel (30 Red Hand Favor, +10 Halcyon Infamy).
  - The Haven and the Red Hand both make it public (`proof.day_one` 4): Halcyon morale −3, and "Halcyon
    made this" runs as a rumour for 8 hours.

### Act II. The Three Weeks (HC1, as built)

**When**
- One session after Act I, or at 25% if that comes first, or at once after Act I's copy. Usually from
  25% into Overrun. Known with Halcyon, plus HC1's own conditions: an Independent or Rogue field lead, a
  Scavenger wanderer for the driver, and the site at `origin.site`.
- At 25%, the director's line: *"Field antiviral. Chapter two. We'd like to thank the couriers."*
- HC1's site stages claim the site tribe, so its bodies keep 150 m from the store
  ([INFECTED.md](INFECTED.md)).
- **Clock:** anchor Authors' start (50%). Not started when it's squeezed: it gives up, because nobody
  asked (`proof.weeks` 0). Still running at 50%: Act III waits while it's engaged, until 55% at the most;
  once it's been untouched for 2 sessions of offered time, it ends at HC1's own default.

**Goal.** HC1's seven built stages run unchanged under their own key, `weeks`: the driver, the site and
its keypad, Dr Wolff, her notebook, who reads it, and the lab's answer. This is the only place the line
says "Continue."

One new detail: tucked into the notebook's back cover are the three site-log pages, dated from
`origin.told`, with the director's initials. HC1's endings pay as built, and the major copies them to
`proof.weeks` (1 in the record, 2 public, 3 Bastion's file) and `proof.pages` (1 into the stove, 3 read
out, 4 in Bastion's file). HC1's In the record replaces the field lead only if that seat is free on the
season's ledger.

**Approaches**
- **The driver and the site,** as built. Talk to the driver (Known with the Haven and below Trusted with
  Halcyon, or ★ Radio Op), share food with him twice, or threaten him (+3 Haven Infamy). The keypad code is
  on three bodies (★ Lab Tech: the first digit).
- **Dr Wolff,** as built: mercy; live capture (research +2, and the director speaks for two minutes about
  her on the open channel); or sneak the notebook out and leave her.
- **Who reads it,** as built:
  - the director: it goes in the record, and the pages go in the stove;
  - the field lead, the Haven or the Red Hand (40 Red Hand Favor): it goes out on the open channel, and the
    pages are read out with it;
  - Bastion: Bastion's file, pages included, and HC3 then starts with Bastion's terms already on the table.
- **New: betray the director.** Hand in the notebook but keep the three pages (`proof.pages` 2 *(you)*).
  The director pays as for the record, but 25% of the time (40% for a Paranoid director) they notice the
  pages are missing: +10 Halcyon Infamy. The pages belong to that character and are lost with the life.
  Later they can be leverage at the reading, sold to the boss's man, or put in the case.

### Act III. Authors (HC3 stages 1–4, as built)

**When**
- At 50% research (Bite treatment), once Act II has ended or given up. Trusted with Halcyon.
- The major starts HC3 under its own key, `authors`; HC3 no longer starts on its own, and its 90% cut-off
  goes. After HC1's Bastion's file, stage 1 is already done.
- The 50% line: *"Bite treatment. The method exists now. So does the question of who owns it."*
- It runs through the Red Hand's settling day and Bastion's muster, and both feed it.
- **Clock:** anchor the reading. HC3's own give-ups count; a stage that fizzles counts as its default.

**Goal.** The cure becomes something a person can own. HC3's first four stages run with their rules
unchanged. Two changes:
- words: the CO's brief mentions the clinic file or the notebook if Bastion has them;
- HC3 stage 3's answer is provisional until Bastion's muster sets `orders.stance`. The stance then
  settles Bastion's part at the stand.

HC3 publishes `authors.terms` (1 agreed, 2 bargained, 3 refused).

**Approaches**
- **The director's letter,** as built: carry the director's terms (guards at the stand, in return for the
  method) to the CO's adjutant. Or read it on the way and copy it onto the field lead's desk, unseen.
- **The field lead's plan,** as built: get the cold case to the clinic's medic within 20 minutes (★ Nurse or
  ★ Doctor: 30); or report her to the duty officer; or side with the Red Hand and sell it (Sell the Formula).
- **The CO's answer,** as built: agree for the director; bargain (Trusted with both: two guards, no key);
  or refuse (+5 Bastion Infamy). After Expended there's no answer to carry.
- **The Red Hand's offer,** as built: turn the boss's man down; take the money (40 Red Hand Favor), so
  Take the Lab is offered to the Red Hand route at the stand unless the Red Hand's last page says
  otherwise; or side with Bastion and tell the adjutant, so a Bastion group holds the lab's road (never
  under Expended). The Red Hand's last page reads this outcome.

### Act IV. The First Page (Halcyon's climax)

**When**
- At 75% research (Immunity serum), never during a stand. The last of the four climaxes, after the Haven's
  second sitting. It runs alongside HC3 stage 5.
- The 75% line: *"Immunity serum. The methods chapter is eight in ten; we write down the other two. The
  first page is read to the authors at the change of shift."*
- The reading is held at the lab at the evening change of shift (17:30–20:30). It's a set piece.
- **Clock:** patience 2 offered evenings: if nobody settles it, it's held again on the next offered
  evening. A lead may open it from 70% when research stalls (after the second sitting). Hard stop: the
  gate. Default: the director's draft stands.
- After the reading comes one more stage, **Before the stand**, which runs until the stand begins (past
  the gate, through the finale's race).

**Goal.** The director reads the first page to the authors, around a table of stand-ins seated by the
season's flags:
- the duty officer, always;
- the field lead, if she's still in her chair; if she walked out, she sits beside the chair's clerk;
- the chair's clerk, if the Haven heard the date or the notebook, HC2 ended Named or Brought home, the
  cold case reached the clinic, `walk.lab_road` is set, or the Haven read any list this season
  (`every_name.ending` 1, `every_name.page_six_read`);
- the CO's adjutant, under `orders.stance` 1–3, if Bastion has terms, the clinic file or the notebook.
  Never under stance 4;
- the boss's man, if the Red Hand's book is whole, or someone at the table holds the pages.

The director says why: *"A paper that starts with that word won't be believed, and then nothing we did in
here counts."* This act decides `proof.first_page`: 1 Day 1, 2 the whole of it, 4 Bastion's
countersignature.

The persuade starts at difficulty 3 and stays between 1 and 4.
- One step easier for each of: the Zone already knows (`proof.day_one` 4 or `weeks.public`); the pages
  were read out (`proof.pages` 3); the lab named its volunteers (`other_two.named`); the chair's clerk asks
  at this table; another stronghold's page came out this season (`majors.pages_open`, once: the Red Hand's
  book, Bastion's All Stations or the Haven's List); an Idealist or Merciful director.
- One step harder with a Paranoid or Ruthless director, or if nobody asked (`proof.weeks` 0).

**Approaches**
- **Persuade** the director to write the whole of it (★ Lab Tech or ★ Doctor count). The first page
  becomes a date three weeks before Day 1, then "Continue." with the director's initials, and the
  director's name goes last (`proof.first_page` 2). +10 Halcyon and +10 Haven Fame. The director then
  speaks for two minutes on the open channel.
- **Betray (leverage),** only with the pages in your hand: put them on the table. An Idealist or Merciful
  director reads them out, and so does any director once the Zone already knows. That's the whole of it
  (+10 Haven Fame), and the director remembers you crossed them. Otherwise the director pays you 40
  Halcyon Favor and the pages go in the stove (`proof.pages` 1; Day 1). 25% of the time (40% for a Paranoid
  director) the lab hears who squeezed the director: +15 Halcyon Infamy.
- **Talk: stand with the director** (Trusted). The first page stays Day 1 (`proof.first_page` 1), or
  "Origin: under investigation" if the Zone already knows. +10 Halcyon Fame and 30 Favor, and the
  director's quartermaster charges you ×0.9 for the rest of the season.
- **Side with Bastion:** the adjutant countersigns. This needs Known with Bastion, plus Bastion's terms
  agreed or bargained, the clinic file or the notebook. Under stances 1 and 3 the signature is "for
  Command"; under stance 2, "for Task Force Bastion"; never under stance 4. The first page then says Task
  Force Bastion secured the lab on Day 4 (`proof.first_page` 4). +10 Bastion Fame, +10 Halcyon Infamy.
- **Side with the Haven** (Known with the Haven): take up the chair's question at the table. Everyone's
  persuade at this reading is one step easier (+5 Haven Fame, +5 Halcyon Infamy). If the reading still
  ends at Day 1, the clerk carries the date to supper, and the Zone knows.

**Before the stand** (the stage after the reading; until the stand begins)
- **Side with the Red Hand** (the pages in hand, Known with the Red Hand): sell them under the table to the
  boss's man (+10 Red Hand Fame and 40 Favor, +15 Halcyon Infamy; `proof.pages` 5). The lab is now a line
  in the book, and the Red Hand's last page settles it.
- **Anyone can clear the line** before the stand:
  - Bribe: pay it at his Red Hand seat (40 Red Hand Favor), and the pages come back.
  - Talk (Trusted with the Red Hand): put the debt in your own name (`proof.debt_yours` *(you)*), and the
    pages come back.
  - Sneak: lift the pages from his satchel at night, past two lookouts (★ Ex-Con: they spot you only at
    25 m). Seen: +10 Red Hand Infamy.
  - If nobody clears it and the book isn't burnt, he reads the pages on the open channel as the stand
    begins.
- **Sneak:** after 20:30, when the generator goes to the freezers, swap the first page of the draft in the
  records room (HC2's night scene). This needs you to have taken part in Act II, to hold the pages, or for
  the three weeks to be public. A ★ Lab Tech reads the filing codes and finds the draft at once; anyone
  else searches three cabinets. It can be done in a Halcyon coat (the disguise design: the duty officer is
  the watcher, difficulty 3; a real notebook number is the cover detail). Seen: +8 Halcyon Infamy, and no
  swap. Done: `proof.first_page` 3 *(you)*. The page is true, and the director finds it as the stand begins.

### Act V. The Final Stand (HC3 stage 6)

**When.** The finale's stand at 100% research. **The finale owns it**: its rules (15 minutes, the same
waves, the 90-second abandon rule, the clean room), its Fame, and who controls the cure
([FINALE.md](FINALE.md), §9). This act only reads it.
- HC3 stage 6 waits for the first stand that's won. A lost stand (nobody at the lab for 90 s) drops
  research to 90%, as built, and the act waits for the remake's stand.
- Under a finale, Take the Lab and the Coat Run are holder changes, not lost stands, so the act doesn't
  wait for them.
- The director's line: *"We're making the batch. Whoever is at the lab in the next fifteen minutes is on
  the paper."*

**Goal.** Hold the lab while the cure is made. Who stands at the gate is what the season arranged:
Bastion's group by `orders.stance`, the Red Hand's on the road by `book.last`, and the Haven's by
`proof.haven_gate`. At most two of the majors' groups come, the finisher's faction first, plus Halcyon's
guard.

Two things can happen as the stand begins, both words and morale only:
- If the page was swapped, the director finds it. An Idealist director reads it on the open channel
  (`proof.first_page` 2; Halcyon morale −5, Haven morale +5). Any other director puts it in the stove: the
  first page goes back to what the reading set, and the director remembers the swapper as someone who
  crossed them.
- If the boss's man still holds the lab's line, he reads the site log on the open channel between the
  tolls (`proof.redhand`; Halcyon morale −5; the pages count as read out). The headline: *"The Red Hand
  read the lab's site log on the open channel as the batch started. 'Continue.' They thought it was funny."*

**Approaches**
- **Fight:** hold the lab. **The paper's author list** uses the finale's bystander rule: everyone at the lab
  for half the stand who never fired on the final holders, bystanders included.
- **Side with Bastion,** by the stance: one Bastion group at the gate under stance 1 or 2, on the lab's road
  under stance 3, none under stance 4. One more Halcyon guard if B3's pilot flew the samples
  (`pilot.flew_for`); Halcyon's guard isn't one of the two groups.
- **Side with the Haven** (new, optional): with `proof.haven_gate`, a Haven group stands at the gate with
  the defenders. It needs the survivor group prefab (**Territory > Haven guard**) and turns itself off
  without it.
- **Betray:** Take the Lab, if the Red Hand's last page offers it (`book.last` 5 or 6, or the book doesn't
  balance), by the finale's rules. It makes the Red Hand route the holder; the first page doesn't change.

**HC3's ending** reads the finale (`finale.how`, `finale.route`), not the batch counters:
- **Under Bastion's guard:** Bastion's group stood at the gate, and Bastion's side held the room or was the
  holder's pact partner.
- **Open doors:** the Haven or the Independent controls the cure.
- **The Red Hand's batch:** the Red Hand route took the room by theft and kept it.
- **The lab alone:** any other result.

### Act VI. The Paper (coda)

One of the Last Rounds: [the codas in the epilogue week](#last-rounds-the-codas-in-the-epilogue-week) has
the calendar and the standing version.

**When**
- The epilogue: from the first free change of shift (17:30) after the Haven's Gate and Bastion's walk,
  once HC3's ending is paid, until the epilogue's last day begins. Known with Halcyon. Its stage missions
  run after the cure (`m_bAfterCure`).
- If nobody has carried the case by the start of the last day, the duty officer walks it down alone at the
  next dawn (words only), and the ending follows from the flags with no personal pay.
- A season that ends without a cure never reaches this act (see Unsigned).

**Goal.** Before sealing the paper in a steel case (a sample case, plague doctor and all), the director
reads it on the open channel for two minutes, the way they speak for the dead. The last page is the list
of authors: the stand's authors, the volunteers of the Day 6 cohort (the eight who walked out and the two
who didn't, by name or by number, below), and, with tribes, the tagged leaders numbered in an appendix
([INFECTED.md](INFECTED.md)). Somewhere, a radio ham writes the reading down, as he has every number since
Day 6; if the finale's Independent route found him, he reads the last line back on the open channel:
*"Got it."* (words only).

Who carries the case depends on the paper: Bastion's adjutant under a countersignature, the field lead if
HC3 ended Open doors, otherwise the director's courier, hooded and in a respirator: one of the beaks. The
carrier walks it to `day2.checkpoint`, to a table on the road just outside the kill zone (about 110 m
from a barrier at the edge): the nearest anyone can stand once the line has stood down, since whoever
walks into the kill zone is turned back.

A voice nobody sees says: *"Leave it on the table. Step back. It will be processed with you."* The case
stays on the Zone's side until the gates open at the wipe. Nobody crosses, nothing comes over, and nothing
ever says what the outside did with it.

**Approaches**
- **Talk:** lead the carrier to the table (escort). Whoever walks beside the case when it's put down is the
  first author in the hall lines: +10 Halcyon Fame and 20 Favor (with Bastion's adjutant, +10 Bastion Fame
  and 20 Bastion Favor instead).
- **Fight:** a Red Hand crew waits on the road if the boss paid for a batch he never got
  (`authors.batch_sold` with the stand held, and `book.last` not 1–4), or if the pages were lifted from his
  man. It needs the Red Hand group prefab; without it nobody comes. Drive the crew off; it costs no Red Hand
  Infamy. If the carrier dies, they tear out the first page, the boss reads it on the open channel, and the
  ending is Two Tins a Copy. Killing the carrier yourself is a betrayal *(you)*: +15 Halcyon Infamy.
- **Persuade** the director at the sealing, before the case leaves the lab: the same check as at the
  reading, one step easier. *"The gates are opening anyway. It only has to be true now."* Success: the
  whole of it, and +10 Haven Fame. This works over Day 1 or Bastion's page, but never after the Red Hand
  has read it.
- **Betray:** put your kept pages in the case, openly in front of another player or the chair's clerk, or
  unseen at night. The paper then goes out with them, whatever the director wrote: the whole of it, and +10
  Haven Fame. The director remembers you crossed them.
- **Side with the Haven:** take the case by way of a safehouse. The council head reads the list of authors
  at supper: the volunteers by name if HC2 or the Haven's reading named them, otherwise by number. +5 Haven
  Fame. The case goes on at dawn.

**What the coda can change.** It can change what the paper says: the ending, its epilogue line and its
hall lines. It can't change the next season: Proof's story rule is the one the paper had at the cure
(decision 12: the epilogue's closing work carries nothing over except hall-of-fame lines).

### Endings

Taken in this order; the first that fits is the ending. What each changes in the finale's routes is in
[FINALE.md](FINALE.md), §12.

**Two Tins a Copy** (checked first)
- **You get:** whoever sold the director's pages was paid at the reading. At the ending they get +5 Red
  Hand Fame and are recorded *(you)* as having worked against Halcyon: Halcyon's Talk checks need one tier
  more from them this life, and its leaders open with *"You're the one who sold the director's pages."*
  Everyone else gets only their stage pay.
- **The world:** reached only if the Red Hand read the pages before anyone else made them public: either as
  the stand began (nobody cleared the line, and the book wasn't sent off loud), or off a dead carrier.
  Halcyon morale −5 more (−10 with the stand's), Red Hand morale +5. "Halcyon made this" runs as a rumour
  for 48 hours. The rest of the paper still reaches the table at the barrier.
  - Headline: *"The Red Hand's market has the lab's first page for sale, two tins a copy. The stickers are
    still on."*
- **The season:** epilogue line: *"There's a copy of the lab's first page in every Red Hand camp."*
  - Hall of fame: "the first page was sold at the market".
  - Next season (after a cure, if it stood at the cure): Red Hand strength +2, and the black market starts
    with one stolen dose.

**The Whole of It**
- **You get:** whoever settled it (persuaded the director at the reading or the sealing, used the pages as
  leverage, had the swap read by an Idealist director, or put the pages in the case): +10 Haven Fame at the
  choice, then +10 Halcyon Fame and a place in the "On the paper" hall lines at the ending. Leverage, or
  slipping the pages in, is remembered by the director as crossing them. Everyone who took part in Acts IV
  to VI: +10 Haven Fame.
- **The world:** paid when it's settled. Halcyon morale −8 (−3 if the three weeks were already public),
  Haven morale +5. A Haven–Halcyon truce, if faction life allows one. It counts as a page in the open. If it
  was settled at the reading, a Haven group stands at the gate in the stand (`proof.haven_gate`; optional
  prefab).
  - The director's two minutes on the open channel: *"Page one says the first case came to the clinic in
    {canon_town:origin.clinic} three weeks before the gates closed, and that the Institute was told. Page
    two says what it told us. I signed for that, every day for three weeks. Those three weeks are why we
    had anything to start from. They are also three weeks. My name is last."*
- **The season:** epilogue line: *"The lab's paper is on the table at the barrier. It starts three weeks
  before Day 1, with one word in the director's hand: Continue. The director's name is last."*
  - Next season (after a cure, if it stood at the cure): starting infection ×0.95, and `legacy.records`,
    so the next director reads the date on Day 11 (Act I starts at `proof.day_one` 2).

**Under Seal**
- **You get:** whoever countersigned at the reading: +10 Bastion Fame, +10 Halcyon Infamy (paid then).
  Whoever walked the adjutant and the case to the barrier: +10 Bastion Fame and 30 Bastion Favor. Everyone
  who took part in Acts IV to VI: +10 Bastion Fame.
- **The world:** only under `orders.stance` 1–3. Bastion strength +3 (inside Bastion's season budget),
  Halcyon morale −5, Haven morale −3. On the secure channel: *"All stations: the lab's paper carries our
  signature. Nobody asks."* The CO's last call adds *"The cure is secured."*, followed by *"Requesting
  relief. Over."* under stance 1, or *"Hold the line."* under stance 3.
- **The season:** epilogue line: *"The lab's paper is in a Bastion crate at the barrier, signed for. Its
  first page says the lab was secured on Day 4."*
  - Next season (after a cure, if it stood at the cure): Bastion strength +2. It doesn't stack with
    Bastion's own story rule: the larger applies. Also `legacy.terms`: the next season's Authors act opens
    with Bastion's terms already on the table.

**Day 1** (the lab's paper)
- **You get:** whoever stood with the director at the reading was paid then: +10 Halcyon Fame, 30 Favor,
  and the quartermaster at ×0.9 for the rest of the season. Whoever walked the case to the table: +10
  Halcyon Fame and 20 Favor. Everyone who took part in Acts IV to VI: +10 Halcyon Fame.
- **The world:** if the Zone didn't know: Halcyon morale +5 and strength +2; the lab stood on its own word.
  If the Zone already knew, the paper says "Origin: under investigation" instead: Haven morale −3, and "The
  lab's paper starts on Day 1" runs as a rumour for 48 hours.
- **The season:** epilogue line: *"The lab's paper is on the table at the barrier. Nobody in it was sick
  before Day 1."* Or: *"...Under 'Origin' it says: under investigation."*
  - Next season (after a cure, if it stood at the cure): specials ×1.05, and the new Zone's first rumour is
    *"They say it came from a lab last time, too."*

**Unsigned** (no cure by the wipe)
- **You get:** nothing more. Anything paid at the choices stays paid.
- **The world:** the line closes at the wipe from whatever its flags say, with one line in the journal:
  *"The paper is in the lab's safe, signed and never sent."*
- **The season:** the season record says so. No epilogue line, no hall-of-fame entry, and no story rule for
  the next season.

### Crossings

**Bastion's major:**
- Act I's `proof.day_one` 3 and Act II's `weeks.bastion` give Bastion proof the lab knew, so the muster's
  "Answer it yourself" goes at Known. `weeks.public` takes a step off its "read it".
- The stance then decides the adjutant's seat and signature at the reading, and Bastion's group at the
  stand. Under Seal needs stance 1–3.
- The site log in Bastion's file (`proof.pages` 4) puts "Continue." next to "Expended." on the last call.
- If both pages end in the open, the epilogue adds: *"Command's order to secure the lab and the Institute's
  last call came on the same day, Day 2."*

**The Haven's major:**
- The Haven's second sitting, before the reading, can read page six and the List. Any list read seats the
  chair's clerk at the reading, and counts as a page in the open (*"They read theirs."*).
- `other_two.named` or `every_name.page_six_read` puts the volunteers on the authors list by name;
  `every_name.page_six_struck` keeps them as numbers.
- The Whole of It gives the Haven a truce and a group at the lab gate. Day 1 costs the Haven morale once it
  knows the date.

**The Red Hand's major:**
- The pages sold to the boss's man become the lab's line in the book. The Red Hand's last page settles it a
  few days later: forgiven in the bill, sold back, burnt with the book, or handed to Bastion.
- Authors' stage 4 and the last page are one mission if both are waiting.
- A Red Hand book read whole at settling day counts as a page in the open, and seats the boss's man at the
  reading.
- Publishes `proof.redhand` for the Red Hand's major.

**The finale.** It reads `proof.first_page`, `authors.*` and the pages during its race. "The Red Hand holds
the lab's pages" means `proof.pages` 5 from the reading on; `proof.redhand` (read out on air) is set only as
the stand begins.

**HC2 The Other Two** runs as built and is never re-run. Its flags set the authors list: named or
haven_names by name, numbers as numbers; with sold, one number is listed as "transferred".

**Other lines:**
- H3 reads `weeks.public`, which Act II now sets.
- B3's `pilot.flew_for` samples still adds one Halcyon guard at the stand.
- C1's `table.halcyon` puts the mother's kitchen notebook into the record before the reading (one sentence
  on the paper's last page, words only).

**Personal *(you)* choices:** Act I's copy, the kept pages, leverage, the sale, the debt, the swap, the
pages in the case, or a dead carrier. The director remembers each one by the usual memory rules, and
Halcyon's Talk checks need one tier more from that player this life.

**Publishes** `proof.day_one` (1–4), `proof.weeks` (0–3), `proof.pages` (0–5), `proof.first_page` (1–4),
`proof.redhand`, `proof.haven_gate`, `proof.debt_yours`, `proof.copy` and `proof.ending`, plus HC1's
`weeks.*` and HC3's `authors.*` flags (with the new `authors.terms`).

### Engine needs

- **Inner lines:** `ActLine(stage, key)` starts a registered questline inside the major's lane;
  `WaitLine(key)` waits for it (a fizzle counts as its default); `ActOnly()` stops `weeks` and `authors`
  starting on their own. Nothing is re-keyed.
- **Edits to built lines:**
  - HC1 stage 4 gains the choice to keep the director's pages.
  - HC3 drops `CURE_LATE`; its stand stage waits again after a lost stand; its ending waits for a won stand
    and reads `finale.how` and `finale.route` (its Take the Lab offer goes through the finale's controller,
    to the Red Hand route only); it publishes `authors.terms`; the CO's brief gets a text variant.
  - Stage versions of Patient Zero's Ledger and Patient Zero, and `NotWithQuest("proof")` on the ordinary
    contracts.
- **New stage missions on `RZ_QuestHalcyonScene`:**
  - the reading: stand-ins seated by flags, with the choices on the director's seat or on the duty officer;
  - Before the stand: HC2's records room at night, and the boss's man behind lookouts;
  - the paper: an escort with `m_bAfterCure` to `day2.checkpoint`'s table.
- **`TryPersuade` with modifiers:** a step easier or harder from flags, traits and `majors.pages_open` (used
  once), kept between 1 and 4.
- **The author list:** the stand's authors by the finale's bystander rule, and an "On the paper" list of up
  to 12 players with the first author marked, handed to the epilogue's season block.

---

## Last Rounds: the codas in the epilogue week

The epilogue lasts one week, 168 hours (decision 12), and [EPILOGUE.md](EPILOGUE.md) owns it: its
phases, its calendar of evenings and its closing work. The four codas are the first item of that
closing work, **Last Rounds**: each faction's closing arc, playable. The scenes themselves are owned
here, in each major's last act.

| Coda | Where | When | Standing version (until the wipe) |
|---|---|---|---|
| **The Gate** (the Haven, [Act VII](#act-vii-the-gate-coda)) | The bend, 150 m short of `day2.checkpoint` | The first free evening, 18:30 | The List pinned on every house board, and the chair's clerk who reads it on request |
| **The last call** (Bastion, [Act VI](#act-vi-the-last-call-and-the-last-roll-coda)) | The radio by the CO's seat | The same evening, 20:30. It's radio, so it doesn't take the evening's slot | Its transcript on the roster board, and an operator who plays it back |
| **The walk and the last roll** (Bastion) | From the nail to the halt, 150 m short | The next 05:30 | The box of tags back on the nail, and "AT THE BARRIER" with the names on the roster board |
| **The Paper** (Halcyon, [Act VI](#act-vi-the-paper-coda)) | From the lab to a table about 110 m short | From the next free change of shift (17:30) after the walk | The case on the table, and the lab gate's notice |
| **The last line** (the Red Hand, [the coda](#the-last-line-coda)) | Rudi's crossroads | The next free evening after the paper's sealing, at dusk | The lorry with the book nailed open at the last page, and a lad on a kitchen chair who reads you your line |

**The rules for all four**
- **Live first.** Each runs live on its first free evening with a player online who could take part. On
  an evening with nobody who could, it waits for the next one. Nothing is skipped or slept through.
- **Finished within the week.** A coda that still hasn't run live when the epilogue's last day begins
  (its processing phase) runs as data at its next hour, and its standing version goes up. So all four
  close before the wipe.
- **Order.** The Gate, the last call, the walk, the paper, the last line. One set piece an evening; the
  majors' codas go before the epilogue's own pieces on the calendar.
- **Airings.** The playable scene runs once. Its second airing, on the next free evening at least 8 real
  hours later, is words only on the faction's channel; the epilogue schedules it.
- **Bodies.** Escorts go only while `PerimeterSealed()` is false, never into the turn-back zone: at most 3
  escort groups, bodies only within 400 m of a player. Every walk halts at 150 m, and the paper's table
  stands just outside the 100 m kill zone (about 110 m), so nobody is ever turned back on the way.
- **Nothing carries over** but hall-of-fame lines (the closure rule). Codas can't change an ending's
  next-season rule; that's fixed at the cure. Pay inside the week is ordinary per-life standing, which
  the wipe clears anyway.
- **Hall lines.** The codas add the "At the barrier" names (Bastion's walk) and the "On the paper" names
  (Proof's author list and its first author). They go into the epilogue's season block as epithets.
- **No cure.** If an After-weeks wipe comes with no cure, the Haven's Gate runs at the first wipe warning,
  held at the stronghold safehouse; Bastion's last call runs in the last 24 real hours; the Red Hand's
  major ends at the wipe warning on its current branch; and Proof is Unsigned.

**The other closing work** in [EPILOGUE.md](EPILOGUE.md) meets the codas here:
- **Bring Them Home:** tags carried back to a post go on the nail, and into Sedlak's box if they arrive
  before the walk.
- **Settle Accounts:** a line paid off in the book is marked "Paid" aloud at the last line.
- **The Walk Out** uses the same north road; it never shares the hour of a coda at the barrier.
- **The Last Broadcast** can read what the majors put in the open: the List, the orders, the book, the
  paper's first page.
- **Loose Ends** never needs to close a major: every climax is settled by the gate. A Week On still open
  at the cure closes as data with its ending's line.

---

## The confirmed lore in the majors

The author confirmed the finale's new lore (decision 2) and Private Marek Fiala as Twice (decision 3).
The majors don't retell it; they carry only these clauses, so nothing contradicts the finale.

| Confirmed fact | Owned by | Where it shows here |
|---|---|---|
| The woman bitten on Day 4 who never turned, crossed off the List on Day 6 as a kindness ("did not present" on the lab's Day 6 ledger) | [FINALE.md](FINALE.md), the Haven's route | Every Name Act III: one cross that doesn't fit the pattern (*"Leave that one."*). Act V: page six's eleventh line. The Gate: her name read last, uncrossed, if the route found her |
| Lieutenant Sedlak led the Day 1 cordon detail (the chained lower store) | The finale, Halcyon's and Bastion's routes | Await Relief's coda: Sedlak, who chained the store on Day 1 and walked the column on Day 2, leads the walk |
| The captain's Day 2 tubes and his log (*"4 R / {count}. Ack."*) | The finale, Bastion's route | Only the last roll: the captain walks beside Sedlak with his empty box, and the four R numbers are read last. The majors never quote the log; Command never answers in the season |
| Private Marek Fiala is alive, as Twice | The finale, Bastion's route can bring him home | The last roll: if he's home, he answers *"Here"*. Struck Off never uses his name |
| The cook's line on Rudi's first page | The finale, the Red Hand's route | The book's canon; `book.first_page` keeps her line; the last line reads it with `cook.line_paid` |
| The eight who walked out of the Day 6 trial | The finale, the Independent route | The paper's author list names the Day 6 cohort |
| The radio ham who wrote down every number since Day 6 | The finale, the Independent route | The paper's coda: *"Got it."* |

And the repo's own new lore ([../docs/LORE.md](../docs/LORE.md)) holds throughout: Bastion's roster
("nobody is coming"; they're all they have; a deserter is struck off as KIA and someone is sent to bring
him home), the Red Hand's book and its lie about Rudi (said flatly, never as a joke), the Haven's bear,
and Halcyon's beaks.

---

## The other questlines

| Questline | Fate | Note |
|---|---|---|
| H1 The List | Absorbed | Its four investigation stages become Every Name's Act III, and its wait and decision become Act V. `RZ_Quest_TheList.c`'s stage classes and every built `list.*` flag are kept. Its own Define and Next are retired: it stays registered and is never offered. The new key `every_name` ships at a wipe |
| H2 The Price of the Door | Story | One house's door. Its flags table offers and tilt one head's vote at both sittings. `door.exile_home` counts as proof at the reading, and if H2 ran first, its house is Act IV's house |
| H3 The Edge of Town | Story | One house's walks. `walk.cut_dose` tables the arrangement and puts the cut dose on the book's Haven line; `walk.lab_road` seats the chair's clerk at Halcyon's reading; `walk.walks_stay` has the walker read his eleven at the Gate |
| Short Weight | Story | Stays the example questline: one storekeeper. Full Weight leans that head to the pot, Short Weight to the arrangement, and it starts twice as often during the Haven's Acts II–IV |
| B1 Await Relief | Absorbed | Bastion's major under its own key, `orders`. Its five built stages are Acts I–III, its four endings Act III's results through `ActEnd`, and its flags keep their names |
| B2 The Census | Story | Bastion counts the Haven, because everything it looks after is signed for. Counted and Forged feed the muster's count. It reads `orders.stance` back (the call's wording, the forged-count risk, and it can't start after Expended), and Counted counts as done under the Haven's cards. It shares one Requisition weight per house with the stance and `haven.rule` |
| B3 The Last Pilot | Story | One man's grief. `pilot.line` is the muster's strongest proof and his name is read in the last call; `pilot.redhand` opens Expended at Known; the stance makes talking him down harder after the orders are out |
| R1 The Book | Absorbed | The Red Hand's major under its own key, `book`. Its stages 1–2 are act 2 and stages 3–6 are act 3, its endings are act 3's results, and all `book.*` flags are kept |
| R2 Cargo | Story | While the major runs it picks the rival's crew when it can. `cargo.purge` and `cargo.cut` change settling day; `cargo.freed` counts as proof at the Haven's reading; `cargo.buyer` lets the jar money be read at settling day |
| R3 The Well Story | Story | Prefers the rival's crew while the major runs. `well.done` and `well.stood` make speaking against the rival harder or easier. `well.stood` also makes that house's head vote against the arrangement |
| HC1 The Three Weeks | Absorbed | Runs whole as Proof's Act II, an inner line under its own key, `weeks`. One new choice (keep the director's pages). It no longer starts on its own |
| HC2 The Other Two | Story | Halcyon's only smaller story: one family. It runs as built and is never re-run. Its flags put names or numbers on the paper, ease the director's persuade, and (with `haven_names`) put page six already in the open at the Haven's reading |
| HC3 Authors | Absorbed | Runs as Proof's Acts III and V under its own key, `authors`, and its stage 5 runs alongside Act IV. It loses its 90% cut-off, waits for a won stand and reads the finale for its ending. Its stage 3 terms are settled by Bastion's stance, and its stage 4 is read by the Red Hand's last page |
| C1 The House That Went Quiet | Story | The Long Table's story, in its own slot. Fed by Every Name (`list.found_table`, `list.read`, and the boy as proof) and by Bastion's stance (how fast an exposed house is burnt). Its `table.walked_out` is a proof at the Haven's second sitting |
| C2 A Place at the Table | Story | The Long Table's personal story, unchanged. It crosses only into C1, as built |

---

## Engine needs

Each major's own needs are under it. These are shared.

- **The majors' lane** (`Major()` in Define): one per faction, outside Max active and the
  one-line-per-faction rule; at most two majors' stage missions offered at once; a squeezed act takes a
  slot from one that isn't; a major starts with priority once its gate is met, runs once a season, and
  never fizzles for good.
- **Gates on stages:** `.WaitCure(pct)`, the outbreak stage, Aftermath, `.After(major, act)` for the
  climax order, the gate, the epilogue's Nth free evening or change of shift, and `WipeDueInHours()`.
  Each act also declares `.Anchor(beat)`, `.Need(sessions)` and, where it has one, `.Patience(...)` and
  `.LastCall(line)`. `.Default(outcome)` reads the stronghold leader's traits or the rule in force, and
  pays nobody.
- **`RZ_MajorClock`** (new): the dynamic fallbacks. It reads research pace, hours played a day, room left,
  the stage, the faction's state, offered time and touches, and the beat gap; it decides squeezes, leads,
  last calls, as-data runs and between beats; it saves its counters with the run; it writes the log lines
  above.
- **`ResolveAtSeasonEnd(run)`:** at the gate, or when the wipe's last 24 hours begin, a climax still open
  resolves by its branch at half world effect, with no player rewards and no next-season rule.
- **Results inside a run:** `ActEnd(run, outcome, line)` applies an act's consequences without Finish (B1
  and R1). `ActLine` / `WaitLine` / `ActOnly` run inner lines under their own keys (HC1, HC3).
  `NudgeMorale` and the other helpers called from Next() go through the budget. `<key>.ending` is set only
  at the major's real end.
- **Built in phase 1:** the set-piece calendar, the season budget with its four lanes, the seat ledger, the one
  outpost, research +3 per major and `SetContractWeight` ([DESIGN.md](../docs/DESIGN.md#the-season-budget)). Still to
  add: the budget's safehouse and standing limits, one Requisition weight per house, one curfew, at most three
  weights per major, and the majors booking the calendar.
- **Shared season flags and their readers:**
  - `haven.rule`, `haven.counted` and `haven.paying`: safehouses, supply lines, Requisition, racket and
    raids, exile leak, road posts;
  - `orders.stance`: voice, presence floor, curfew, Requisition, HC3, B2, B3, C1, No Reply;
  - `book.last` and `book.stand`, read by the finale's stand;
  - `proof.*` flags;
  - `majors.pages_open`, a counter any major's "read it" check uses once (and the finale's checks, once);
  - flag names reserved in QUESTLINES.md's Crossings table.
- **Inputs to the finale's stand** (the finale owns the stand; these are what the majors hand it): one
  group per faction at the lab, at most two of the majors' groups (the finisher's faction first) plus
  Halcyon's guard; Bastion's by stance; the Red Hand's lab-road group, which doesn't come if Bastion holds
  that road; the Haven's by `proof.haven_gate` behind a **Territory > Haven guard** setting; one decider for
  Take the Lab per stand (the Red Hand's last page overrides Authors' stage 4), through the controller;
  `OnFinalStandStart` and `OnFinalStandEnd` events for questlines (won, lost or a holder change).
- **Leaders and scenes:**
  - stronghold leader traits readable by questlines (read only);
  - `SpawnLeaderAt(gm, leaderId, pos)` stand-ins without moving the seat (only near players);
  - run claims on seats from summons to tally;
  - an "Ask about..." choice on other factions' stronghold leaders while a stage is open (a dialogue topic
    when dialogue is on, an action until then);
  - player-facing lines for the four stronghold leaders written without pronouns (names are seeded).
- **Channels:** `OpenLine(faction, text)`; `OpenChannel(faction, lines, gapSeconds)` for two-minute
  readings; `SecureLine(faction, text, hours)` for Radio Operators; a one-off line at a set time (Bastion's
  20:30 call); a one-night mute of a faction's open channel.
- **Canon:**
  - `day2.checkpoint`: the sealed road checkpoint the Day 2 column was turned back at, nearest the Haven
    stronghold and preferring the north. It replaces both `haven.gate` and `bastion.day2_checkpoint`. On a
    map with none, the stronghold safehouse's gate. The stand point is 150 m up the road and the paper's
    table just outside the kill zone (about 110 m up), both outside the 60 m checkpoint turn-back and the
    100 m kill zone, where the built line turns walkers back after the cure.
  - `bastion.orders` pinned to "expended".
  - `redhand.crossroads`, `haven.store`, `haven.driver`.
  - LORE, in the majors' commit: the Long Table dates a day earlier; the chair's fixed backstory; the CO
    burned a whole copy on Day 3 (only when the major is built, and in the "Expended" row, not the Day 3
    paragraph, so the Day 6 address still reads as news to the posts); the book's first page (the exact
    lines, with the cook's line under the boss's). Sedlak keeping the roster at the stronghold is in LORE
    already; her Day 1 cordon detail goes in with the finale's lore.
- **After the cure:** the codas run only while `IsEpilogue()` is true (or from the first wipe warning with
  no cure). The questline tick and these stages keep running after the cure (`m_bAfterCure`).
- **The season story record:**
  - `SeasonStory(faction, endingKey, line, deciderIds)` and `SetEpilogueLine(faction, text)`;
  - an epilogue line chooser: the line of the faction that controls the cure first, then one line from each
    other major;
  - `last_season.json` keeps each major's ending, branch, the cure's controller and deciders by identity,
    written even without a cure;
  - the hall lines ("The season's stories": one line per major, up to four names; "At the barrier"; "On the
    paper") go into the epilogue's season block (UI.md, UI/LAYOUTS.md).
- **Story rules next season:** `RZ_SeasonRuleSet.ForMajor(key, ending)` behind a new setting, **Season >
  Story rules** (on).
  - One small rule per major, at most half a winner's step, applied only after a cure and only for majors
    that really ended (not a fallback), read from the ending as it stood at the cure.
  - Merged after the controller's rules and the finale's one half rule set (the deal partner, else the Fame
    leader), through `RZ_SeasonMods.Add` and `Cap()`, never past the winner's own value for the same field,
    and at most ±2 strength per faction from story rules in total.
  - New fields: Long Table chance ×0.5–1.5 (never over 1); a first-day exile leak (at most 30%); one road
    post on a safehouse town's road; a contract weight (×0.9–1.1); a first notice-board note; one stolen
    dose at the start; a seeded stronghold trait; read-only legacy flags; a two-day rumour.
- **The journal:** the "The season" tab (each major's act, the rule or stance, the leans in words, the last
  calls).
- **Profiles:** a crew name for the life, a Bastion rank text, and a per-life "on the roster" flag.
- **Optional props:** **Questlines > Red Hand lorry** and **Bonfire prop** (WORKBENCH_SETUP.md entries and
  the VERIFY count).
- **`tools/pacing_sim.py`:** the 60, 70 and 80% milestones and the order check (conflict 0).

---

## Guardrails

- **Sealed Zone.** Command never answers in any ending. Bastion, Command is the stronghold answering
  itself, and the stance only changes what Bastion does. No outside aid arrives: Counted's food is
  Bastion's own, and the Haven's tins are the Red Hand's. No supply drop and no visible wall appear. Every
  coda stops short of the barrier: the readings and walks halt at 150 m, and the paper's table stands just
  outside the 100 m kill zone (about 110 m), outside the 60 m turn-back too. Nothing and nobody crosses, and the case stays on the
  Zone's side until the gates open with the wipe. The lines only uncover what the outside did before the
  gates closed ("Continue.", "expended"); the captain's Day 2 "Ack." is the finale's, and from before the
  line went quiet too.
- **No sleep and no time skip.** Every gate is a cure milestone, a stage, a game-clock hour or a played
  wait. The fallbacks count offered time and hours played, so nothing moves on an empty server. A set piece
  that can't run waits for the next free evening; nothing is skipped or slept through.
- **Neither waiting nor rushed** (decision 10). A busy server's climaxes come on their milestones, held at
  most one evening by players in the middle of something. A quiet server's acts stay open while there's
  room, every default gives a last call and a grace first, and a stalled server's climaxes may come up to
  5 points early, never more.
- **Strongholds and their leaders.** The four stronghold leaders are never replaced, never killed and stay
  Loyal. Their traits are only read: defaults, persuade steps, and the Red Hand's next-season seeding,
  which seeds only a fresh boss in a new season. Strongholds and the lab never fall. Settling day's coup
  always fails, at most one crew walks away, and the faction never splits.
- **Leaders and outposts.** The seat ledger stops two majors moving the same leader: each outpost or
  safehouse seat is moved at most once a season, two per major, three per faction, and never past one Rogue
  per faction. Only one outpost changes hands through majors in the whole season, by the usual territory
  rules. That covers settling day's raid, Bastion's file and Expended's feud alike.
- **Nothing overpowered.** Effects are capped per call and per major, and per faction across all majors
  (morale ±20, strength ±8), with separate lanes for contracts and errands, the epilogue and the finale.
  Everything recovers or drifts back. Season weights combine into one ×0.8–1.2 value per contract and one
  Requisition weight per house. A curfew from any source is one curfew. Rules that would contradict lapse
  in a fixed way: the cards end after Expended, Sent off loud ends the arrangement and the lab's line, Under
  Seal needs stance 1–3, and the Red Hand's road group stands down if Bastion holds that road.
- **No free loot or Fame.** Pay comes for play: a reward item only from the giver's shelf at the player's
  tier, within the existing items rule. Defaults and fallbacks pay nobody. Standing is capped per major
  (25 Fame, 40 Infamy, 60 Favor) and per player per faction across all majors (40, 40, 80). Fame from the
  majors counts toward season Fame like any other; season Fame no longer picks who has the cure (control
  does). No major adds research beyond +3 (Halcyon's inner lines included), changes the stand's rules or
  decides who controls the cure. Take the Lab is the Red Hand route's once a season. At most two of the
  majors' groups stand at the lab.
- **Light carry-over.** One small story rule per major, at most half a winner's step, only after a cure,
  only for majors that really ended, read at the cure, inside the existing caps (at most ±2 strength per
  faction from story rules). The codas carry nothing over but hall-of-fame lines.
- **Performance first.** The majors are data: votes, counts, musters, leans and the clocks are run flags.
  Commanders at the muster are text from their seats. Bodies spawn only near players: council stand-ins
  within 300 m; crew-boss stand-ins, holds and at most 3 escort groups within 400 m. At most two majors'
  stage missions are offered at once, and one set piece holds an evening. Props (lorry, bonfire, table)
  leave no persistent trace.
- **Any map.** Every place is canon, worked out from the map: `day2.checkpoint`, `redhand.crossroads`, the
  far house, the stronghold safehouse, `origin.site`. Each has a fallback for maps without checkpoints,
  junctions or a second house. Nothing uses fixed coordinates.
- **Most of the map stays abandoned.** No major founds towns. People move between existing safehouses
  within their caps, and the one possible outpost change follows the usual territory rules.
- **Tone.** Nobody is a villain in their own story: the chair, the CO, the boss and the director each have
  a reason, and the boss's isn't a good one. A stronghold leader never jokes about their own page. The
  Haven's dark side is said sideways until the reading. Rudi's end is never a joke; the Red Hand's lie about
  it is said flatly. The Long Table is never described.

---

## Docs in the same commit

When each part is built, in the same commit as its code:
- **QUESTLINES.md** splits into Majors and Stories, with the new crossing flags.
- **MISSIONS.md** gets the acts, with ✅ only when built. Our Crossroads, Coup Talk and Training Day are
  listed as questline-stage classes (out of the random pool).
- **DESIGN.md:**
  - Questlines: lanes, set pieces, the dynamic fallbacks (the signals, squeeze, patience, leads, last
    calls, hard stops), defaults, the budget and its lanes;
  - World facts: `day2.checkpoint`, `redhand.crossroads`, `bastion.orders`;
  - Season pacing: the majors' milestones and the order check;
  - Seasons: story rules and the season record.
- **LORE.md, REPUTATION.md, UI.md, UI/LAYOUTS.md, WORKBENCH_SETUP.md** (the settings, the two props, the
  VERIFY count, the log lines), **ROADMAP.md** and **README.md**.
- No new doc in `docs/` for the majors: they live in QUESTLINES.md (decision 16 adds only FINALE, INFECTED,
  DIALOGUE and WRITING, each as it's built).
