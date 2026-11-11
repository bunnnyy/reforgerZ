# Grey choices and dark humour

**Status: designed, not built.** Approved by the author on 2026-11-11.

This piece decides how the existing voices speak, and sets the standard every choice is written to. It
adds only small, data-only helpers. In the repo it becomes `docs/WRITING.md` (section 1).

## What changed in review

Conflict numbers are the plan's, counted from 1, as in [PLAN.md](PLAN.md). Decisions are numbered as in
the plan's "Decisions for you". Tone's own options are taken as recommended; the alternatives are gone.

- **Crew names are leverage, not honours** (conflict 31). A crew name is what the crew holds on you,
  picked after something you'd rather they forgot. The names and their triggers are reputation's
  ([REPUTATION_NAMES.md](REPUTATION_NAMES.md#9-the-factions-word-for-you)); "Sunday" and "Postman" are gone
  ("the Postmen" is a tribe's nickname now).
- **One word table** (the plan's section 5). Reputation owns what each faction calls you. This piece's own
  table of words is dropped; it keeps only the rules for saying them (3.8).
- **Old lore fixed** (conflict 32). The Red Hand is right that nobody was coming and nothing taken ever has to
  be given back. Bastion is right that they're all they have. Rudi's end is never a joke, and the Red Hand's
  lie about it is said flatly. A name never leaves the roster: *"Someone got to him first. His KIA's rubbed
  out."*
- **"Struck off" is only Bastion's word for a deserter**, struck off as KIA with his name still on the board.
  The book's word is "paid" (conflict 7, as [FINALE.md](FINALE.md) applies it).
- **The finale's keys** (conflict 39). `RouteOf(identity)` everywhere. `finale.route` 0 is nobody (handed
  over); the lab alone is `finale.finisher` 0. The lock is per identity for the season, survives death and
  ends at the cure ([FINALE.md](FINALE.md#how-long)), so the option on lock scope is gone.
- **Tribes replace Grounds** (conflicts 42 and 43). The infected's words come from
  [INFECTED.md](INFECTED.md#people). Player text never says "alpha", "tribe" or "crawler", and "strays" means
  only the people the Haven turned out.
- **Mood is `SeasonMood()`** (the plan's renames). "Mood" means only the season mood now.
- **The epilogue lasts one week** (decision 12). The epilogue's people are revisited for a week, not two
  days, so their pools are sized for it (3.12). The Walk Out lets people leave at the north road in
  processing, so "nobody has gone home" holds only until then, and the outside's few words after the cure are
  listed in one place (3.5).
- **Dynamic, never fixed days** (decision 10). Running jokes switch on the majors' flags, never on a day.
- **Dialogue is off by default** (decision 13). The vanilla fallback is what ships first, so its rules for dry
  lines (3.7) come first.
- **Strangers only where one would make sense** (decision 14). Small side quests are spoken by a seeded person
  at a place [ERRANDS.md](ERRANDS.md#where-a-stranger-can-find-you) allows, never by a timer (4.6b).
- **The finale's lore is confirmed** (decision 2). Lines may use it, under its owners' rules: the captain's log
  is never quoted on air, and Command never answers.
- **WRITING.md is added only as it's built** (decision 16): when the writing pass's phase 0 lands, holding only
  the rules in force then (section 1).
- **The PC_SETUP.md gap is gone.** The repo's Doc map already lists it.
- **Room left in a joke is never a timer.** Pitches, route lines and coldness fade by real days and visits, as
  before; nothing new runs on fixed season days.

---

Where it meets the other designs, it owns the **voice**, and they own the mechanics. One owner per thing:

| Design | Owns | This piece adds |
|---|---|---|
| Finale ([FINALE.md](FINALE.md)): routes, the 90% lock | The routes, the lock and its scope, who can choose, the fight | Wording rules for the pitches. Lines for players on the route (in-group), on another route (out-group), not yet sided (unsided), on a route that lost, and on the Independent route, all face to face (3.4) |
| Epilogue ([EPILOGUE.md](EPILOGUE.md)) | Which places react, the props, the boards of record, the closing work of the week | Phrasing, variety and honour-line rules (3.5) |
| Majors ([MAJORS.md](MAJORS.md)) | The four codas (the Gate, the last call and the last roll, the last line, the paper), their lines, the page states | Running jokes switched by their flags (3.10). Nothing at their scenes |
| Infected ([INFECTED.md](INFECTED.md)) | The marks at a range's edge, "Read the mark", each faction's words for the infected | Its word list joins the lint (6), and its rules for writing the infected move into WRITING.md |
| Mission style ([MISSION_STYLE.md](MISSION_STYLE.md)) | Approaches by mission kind: one table in DESIGN.md | Two columns for that table: who pays, and what to watch for (4.6) |
| Dialogue ([DIALOGUE.md](DIALOGUE.md)) | Nodes, checks, the vanilla fallback | Dry lines and their limits (3.7) |
| Disguise ([DISGUISE.md](DISGUISE.md)) | The look, being caught | Lines that follow the look's results (3.9) |
| Reputation ([REPUTATION_NAMES.md](REPUTATION_NAMES.md)) | The names, the faction's word for you, crew names | The rules for saying them (3.8) |
| Errands ([ERRANDS.md](ERRANDS.md)) | The dealer, the spots where strangers can be, the pay | The rules for how a small ask sounds (4.6b) |

## 1. Where it lives in the docs

| Doc | Gets |
|---|---|
| **New `docs/WRITING.md`** | Moved from LORE.md: Tone, the writing guide and the broadcast table. New: the tone map (3), the humour line (5), phrasing for choices and outcomes (4.7), the template rules (5.6), the infected's words (from INFECTED.md), the before-and-after rewrites |
| `docs/LORE.md` | Keeps the setting, the factions as people, the hard edges, the Long Table and the endings. Its broadcast paragraph stays and links to WRITING.md for the table. Each faction keeps a one-line "Voice:" sample with a link |
| `docs/DESIGN.md`, Missions | "Grey choices": 4.1–4.5 and 4.8. The by-kind content becomes two columns of the mission-style table, not a second table |
| `docs/DESIGN.md`, Faction life | The voice bank, the season mood and temper, grief, joke memory (once built) |
| `docs/MISSIONS.md` | Each Result text corrected where options or outcomes change |
| `docs/ROADMAP.md` | "Writing pass" under Next, with the phases (section 6) |
| `docs/WORKBENCH_SETUP.md` | Voice > Lines, Voice > Disabled, the debug settings, the voice log line |

**When WRITING.md is added** (decision 16). It's added in the commit that lands phase 0 of the writing pass:
the guide moved out of LORE.md, the grey test, the humour line and the phrasing and template rules, which are
rules for writing and are in force the day they land. The season mood, temper, grief, joke memory and the voice
bank go into it (and into DESIGN.md) only when phase 3 builds them. Until then WRITING.md doesn't describe them,
and the roadmap lists them as planned.

**Every link that changes with the move** goes in the same commit:
- **`CLAUDE.md`:**
  - in the Doc map, the LORE.md row loses "tone, writing guide", and WRITING.md gets a row;
  - the change list's "Change to setting, tone or factions" line points tone to `docs/WRITING.md`;
  - the writing-style line changes from `docs/LORE.md` to `docs/WRITING.md`.
- **`README.md`:** the LORE.md row of the doc list, plus a new WRITING.md row.
- **`docs/DESIGN.md`:** three links: the mission text rule (`LORE.md#writing-guide`), things found in drawers,
  and the guidance voices.
- **Code comments** that point at LORE.md's writing guide or broadcast table:
  - `Factions/RZ_FactionVoice.c` and `Factions/RZ_LeaderVoice.c` (the writing guide);
  - `Director/RZ_Broadcast.c` ("The table in docs/LORE.md", and its file header);
  - `Guidance/RZ_GuidanceTips.c` and `Guidance/RZ_GuidanceText.c` ("written to the voices in docs/LORE.md").
- Then run CLAUDE.md's link check, and check the Doc map lists exactly the files in `docs/`.

## 2. The grey core: everyone is right about something

A grey choice puts one side's truth against another's. If one side of a choice has nothing it's right about,
the choice is a virtue test, not a grey choice. Fix it, or make it a plain job.

| Faction | Right about | Wrong about (its hard edges, LORE.md) | What it jokes about instead |
|---|---|---|---|
| The Haven | Nobody is useless. People are the point | The List, the mouths, walking people out, quiet arrangements, "borrowing" | Food, chores, the house rules, itself |
| Bastion | They're all they have: feed your own, look for your own, bring your own home. And someone has to count, queue and hold, or people get trampled | Containment, "contribution", "signed correct", the stove | Forms, Command, the roster, rank |
| The Red Hand | Nobody was coming, and nothing taken ever has to be given back. Everyone would sell you, so keep something on everyone | Rudi, Cargo, the pit, the cut dose, the well, learning the drones | Everyone, Bastion most; prices, the book, the toll |
| Halcyon | The cure is the only way out, and the numbers matter | The three weeks, the other two, samples first | Rarely on purpose: exact words that land wrong |
| The Long Table | Nothing that can be said aloud | Everything | Nothing, ever |

## 3. Where the tone shifts

### 3.0 Who hears what (the shared-server rule)

- **Shared text is picked once, on the server, for everyone.** That covers the broadcast, open and secure
  channels, headlines, rumours on the roads, notices and boards.
  - Two players standing together always hear the same words.
  - Shared text never depends on one listener: not their route, standing, disguise, deeds, or which jokes
    they've heard.
- **Face-to-face text is picked per player.** That covers leader openers and closers, a guard's remark, a
  wanderer's line, dialogue nodes, and a mission's outcome to the players in it. It rides the one owner-only
  payload, so nobody else's client receives it.
- So grief (3.3), route (3.4) and joke memory (3.12) each work in two ways, one per kind of channel.

### 3.1 By channel

| Channel | Who speaks | Register | Joke cap | Never |
|---|---|---|---|---|
| Government broadcast | The outside, through a machine | Official and deadpan, with one wrong word | No intended jokes | Names a cause, apologises, acts, answers anyone |
| Open channels | People on shift | The faction's voice | 1 in 4 | A joke instead of the information (3.11). A joke about the bitten on air |
| Secure channels | The faction when nobody's listening | Blunt, with numbers | 1 in 10, bleak register only | A joke at its own people |
| Rumours, road news | The teller | Gossip, biased, worst about rivals | 1 in 3, aimed at rivals | Saying which are true |
| Headlines | The roads | One plain sentence and one detail | The detail may be dry | A verdict. A player's name (except as listed in 4.8) |
| Notices and boards | The printed text, then whoever wrote on it | The printed line, then handwriting | The handwriting may answer back | Reusing a questline clue's text. Vanishing in front of someone |
| Leader openers and closers | A named person | Faction voice × traits (3.6) | Set by trait | A speech (the route pitch is the one exception) |
| Mission briefs | The giver | The giver's voice, with what they leave out | Haven and Red Hand givers may open with one. Bastion is procedural, Halcyon clinical. **Mercy jobs never** | Telling the player what's right |
| Choice labels | The player | Verb + object, then the cost or check in brackets | None | Judgement words. Rewards or consequences |
| Outcomes | What happened, then one reaction | Three beats (4.7) | Only in the reaction | A verdict. Numbers inside the voiced beats |
| Guidance | A voiced speaker, or nobody | Voiced tips: the speaker's register. Plain hints: plain | Voiced tips: the speaker's. Plain hints, veteran short lines, standing and pay notices: none | Burying a critical instruction: it comes first and can't be misread |
| Epilogue | The world (3.5) | Looking back | One wry line per place | Contradicting the season |
| The Long Table | The family | Homely | None, ever | Describing anything |

**A voiced tip may be callous in the speaker's way.** The Red Hand's built tips stay as they are:
- *"Pour the vodka on it, not in you. Quick, before it takes."*
- *"Bitten? Shame. Our cut dose works half the time..."*

The target is the price, the dose and the drinking, not the bitten person's fear or pain (5.1).

**Swearing:** mild and rare, only in Red Hand and Bastion voices, and never the punchline.

### 3.2 By the season's mood

The season mood is one server-wide value, `RZ_WorldFacts.SeasonMood()`.
- It is the broadcast's own table, moved into world facts, so the broadcast and every voice read the same
  thing.
- It adds no simulation.
- It runs on stages and cure percent, not days, so it works at any season speed.

**The ladder (the first match wins):**

| # | Mood | When (all built facts) | Joke share |
|---|---|---|---|
| 1 | **Epilogue** | `IsEpilogue()` | One wry line per place; people 1 in 3 |
| 2 | **Hope** | The broadcast's breakthrough window is open: 90 online minutes from when players first heard the news (`RZ_Broadcast`, as built) | 1 in 3 |
| 3 | **Bleak** | The broadcast's long stall (3 h with no research), or Overrun or Aftermath with the cure under 50% | 1 in 10 |
| 4 | **Grim** | Collapse or later, or average infection 45+, or the 45-minute stall | 1 in 5 |
| 5 | **Early** | Anything else | 1 in 3 |

- **No gaps.** Early is the default. Aftermath at 50% or more is Grim. Early Outbreak at 25% or more is Early
  unless infection is high.
- **Hope has one source.** It is the built window. Faction voices celebrate for exactly as long as the broadcast
  "evaluates reports". The window is runtime-only as built, so a restart ends it; that's fine for a mood.
- **The gate.** While `AtFinaleGate()` is true, research stops by design.
  - The stall timers don't count there, so the gate never reads as a stall.
  - The mood is one step lighter (Grim reads as Early), because everyone can see the end.
  - The gate adds race lines (3.4). It isn't a mood of its own.
- **Holding a mood.** Hope and Epilogue start at once. Early, Grim and Bleak change only when the new condition
  has held for two mood checks in a row (every 10 minutes, on the existing tick). A mood, once entered, holds for
  at least 60 online minutes unless Hope or Epilogue takes over.

**Temper: morale moves one faction one step.**
- Each faction has a temper read from its own morale, with a gap so it doesn't flicker:
  - one step bleaker below 30, back once morale is above 38;
  - one step lighter above 75, back once it's below 67.
- Steps run Hope/Early → Grim → Bleak. The Epilogue isn't shifted.
- Morale isn't in the ladder, so nothing counts twice. A truck lost (−10) and the slow recovery toward 60 no
  longer flip the server's mood within the hour; they nudge one faction's temper.
- **The Red Hand is the exception.**
  - Low temper changes its register to taunts; it doesn't change its share.
  - Its open channel never drops below 1 in 5, except in grief.
  - Its secure channel goes quiet in Bleak like everyone's.

**Channel and mood combine by the lower.** A line's joke share is the lower of:
- the channel cap (3.1);
- the speaker faction's mood share after temper.

For example, Bastion's open channel in Grim: the lower of 1 in 4 and 1 in 5 is 1 in 5.

**The broadcast reads the same ladder.** Every built line stays, and two are new. The order is the built one,
with the gate inserted after Hope:

| State | Line | |
|---|---|---|
| Epilogue | The new order: *"...Processing will begin at the road checkpoints."* | built |
| Hope | *"Reports of research inside the zone are being evaluated."* | built |
| At the gate | *"The status of this area is under review. Remain where you are."* | **new** |
| Bleak: long stall (never at the gate) | *"The perimeter will be maintained indefinitely."* | built |
| Bleak: Overrun under 50% | *"The perimeter will be maintained."* | built |
| Bleak: Aftermath under 50% | *"Residents are thanked for remaining indoors. Residents are thanked for remaining."* | **new** |
| Grim | *"Residents are reminded that the perimeter is for your protection."* | built |
| Early | *"Remain indoors. Aid is on its way."* | built |
| Season over (any end) | *"The perimeter will be maintained."* plus the wipe note (`Core/RZ_Seasons.c`) | built |

The table moves from LORE.md to WRITING.md and is rewritten there. The guidance radio tip that quotes
*"...the perimeter will be maintained"* stays true.

### 3.3 Grief and other local overrides

**Grief has two scopes.**
- **Face to face, by place.** After a death the server knows about, wry and joke lines are skipped for listeners
  within 1.5 km for 10 minutes. These deaths count:
  - a leader, a wanderer or a named person;
  - a Haven Champion's Remembered;
  - a truck crew's funeral;
  - a Turned Player;
  - a death in a safehouse (`House().m_iDied` going up). This one quiets that safehouse's people for 30
    minutes.
- **Radio, for the whole faction.** When a faction loses a leader or a truck crew, or the Haven loses someone in
  a house, that faction's open channel goes plain for 20 minutes, for everyone.
- **The Red Hand's own send-off lines are exempt.** Its grief is loud, as the lore says.
- **PvP deaths aren't tracked.** The low joke shares in Grim and Bleak cover them.

**A short house** jokes about the soup, the potatoes and the chores *more*, because that's how the Haven copes. It
never jokes about who eats (the mouths), and not at all in the 30 minutes after a death in that house. The
narrator never jokes about a shortage.

**Feud or truce.** A feud sharpens the Red Hand's taunts and Bastion's lists. A truce allows a joke across the
line: *"Bastion and the Red Hand shared a fire last night. Both counted the matches."*

**The infected.** [INFECTED.md](INFECTED.md#people) owns the marks and each faction's words for them.
- Only the Red Hand jokes about the infected, in its own words: "the furniture" for a town's slow ones, a crew
  nickname for a pack ("the Postmen", "the Choir"), "the boss" for its leader.
- The Haven names a pack's leader fondly, by who it was, and gives it a job, never a joke. When nobody knew who
  it was, it's "the big one".
- Halcyon never says "him" or "her" about them.
- Near a pack's range at dusk (or in fog, which counts as dusk), the Haven's people go quiet.
- Player text never says "alpha", "tribe" or "crawler". "Strays" is never used for the infected: it's the crews'
  word for the people the Haven turned out.

**A player they won't talk to.**
- A leader whose attitude to a player is −2 or below has no line for them but the refusal, and nobody at that seat
  jokes with them.
- Wanderers who were robbed don't joke with the robber.

**Stronghold pages** (majors). A stronghold leader never jokes about their own page: the book's back page, the Day
3 orders, the List, the site-log pages. Once a page is in the open (`majors.pages_open`), rivals may gossip about
it, plainly, and never as a joke in the first real day.

### 3.4 The race and the point of no return (90%)

**Scope.** [FINALE.md](FINALE.md#4-the-lock) owns the lock. The voice reads `RouteOf(identity)`: none, the Haven,
Bastion, the Red Hand, Halcyon or Independent. The lock is per identity for the season: it survives death, so
dying doesn't undo it, and it ends at the cure.

**Before 90%, nothing is locked, and everyone jokes about the race.** These lines go on shared channels, so they
never use numbers or players:
- The Red Hand opens a book that never names a favourite, so it can't work as a scoreboard: *"Evens on everyone.
  We like to lose money slowly."*
- Bastion logs "civilian interest".
- The Haven prays out loud for "whoever's quickest".
- Halcyon declines to comment, then comments.

**The pitches.** Each route has one best argument and one admission. The admission is always made **before** the
lock, never held back for after it. Nobody locks blind:
- **From 75%,** route topics answer *"Ask me again when it's nearly done."*
- **Face to face, on demand:** "Ask about the cure" at each pitch-giver, from 80% until the player locks.
- **On air, once:** at 85%, on each faction's open channel. The same for everyone, and no player is named.
- **The lock prompt** repeats the chosen route's admission in one plain line. A late joiner who locks without
  asking anyone still reads it.
- **Pitch-givers:** the chair, the commanding officer, the boss and the director.
- **The Independent route has no seat.** Its pitch comes from a wanderer with no side: the Doctor, or the Courier
  if the Doctor is gone. If neither is alive, it's an unsigned sheet at the lab gate.
- **After the lock,** a leader on your route may tell you one *personal* thing, about themselves, not the route.
  It changes nothing you chose on.

| Route | Best argument | The admission |
|---|---|---|
| The Haven | The cure goes hand to hand, to the people the others would leave till last | The houses decide who goes first, and the houses kept the List |
| Bastion | Order: a queue, a register, nobody trampled at the gate | It reaches those who comply, and the register is a census |
| The Red Hand | It reaches anyone who can pay, including people nobody else would serve | The price is the queue. The cut dose was sold the same way |
| Halcyon | Published, fast, checked, with every author named | The paper starts on Day 1. The three weeks are on page one, or not at all |
| Independent | For everyone, on nobody's terms | Nobody guards it and nobody counts it, and every faction remembers who took it from them |

**After the lock: face to face only.** Shared channels never single out a player's route; their race lines are
about factions, never about who backs them. That way the radio can't be used to find a route's backers in the
fight. The Independent's commit on air is said without a name: *"This is nobody. The cure goes to everyone."*

| The listener is | Lines said to them | Shared channels |
|---|---|---|
| **In-group** (locked into this faction's route) | Their name if this leader remembers them (helped or saved); otherwise the faction's word for them, with the lock's overlay ([REPUTATION_NAMES.md](REPUTATION_NAMES.md#9-the-factions-word-for-you)). Gallows humour *with* them, as one of their own. On a new life: *"Somebody said you're with us."* | Unchanged |
| **Out-group** (locked into another route) | Cool: no in-jokes with them, and their word, not their name. The Haven stays kind and may make one joke at its own expense (it shuts its door over exile, never over a route). The Red Hand jokes *at* them, as at everyone. Halcyon is exact and disappointed. Bastion says "Civilian." | Unchanged |
| **Independent** | Everyone is a little afraid of them. Jokes are *about* them, said to their face, never on air | Unchanged |
| **Unsided** (late joiners, the undecided) | Courted once a visit: the Red Hand has an offer, Bastion a form, the Haven a bowl and a question, Halcyon a pen. Otherwise normal | Unchanged |

**Coldness fades.**
- It covers the opener only, not every line.
- The first visit after the lock is cool. Later visits that real day are civil.
- After two real days the leader is back to their normal register, with at most one route line a visit.
- Ambient lines and radio never change.

So a player on a minority route or the Independent route hears a normal Zone for the 4–17 days the gate may stay
open.

**When a route loses** (another route controls the cure):
- **The loser's own leaders** give one plain line, with no blame on the player, and are back to their usual
  register the next day.
- **The winners, to a loser:**
  - The Haven is gracious.
  - Bastion files them with everyone else.
  - The Red Hand: *"No hard feelings. Some soft ones."*
  - Halcyon lists objections on page two.
- **Never on air, never with a name.**

**After the lock, the grey moves inside the route:** who goes first in Bastion's queue, which house the Haven serves
first, what the Red Hand charges the Haven, whose name heads Halcyon's paper, and who the Independent route leaves to
carry it.

### 3.5 The epilogue, in the world

**Who owns what.**
- [EPILOGUE.md](EPILOGUE.md) decides which places react, builds the props and owns the closing work of the week.
- [MAJORS.md](MAJORS.md) owns the four codas and their lines.
- This piece owns only the phrasing rules, for all of them:

- **One line per place**, in its owner's voice, pointing at a flag from this season, not just at the winner.
- **At most one wry line per place.** The funniest faction is still the Red Hand. If the Long Table lasted the
  season unfound, its farm gets no line: it's just quiet.
- **Never contradict the facts.**
  - Until processing (the epilogue's last day), the barriers stay closed, nobody has gone home, and processing is
    always "about to begin".
  - In processing, people walk out at the north road one at a time (the Walk Out). Lines may say that someone
    went; nothing past the barrier is ever shown or said, and nobody comes back.
  - Elsewhere the barriers stay closed until the wipe.
- **What the outside says after the cure** is only this, the same on every route, and never a reply to anyone:
  - the loudspeaker's new order and the built processing warnings;
  - at the paper's table: *"Leave it on the table. Step back. It will be processed with you."* (MAJORS.md);
  - at the north road barrier in processing: *"Name?"*. Nothing comes after it.
- **Variety.**
  - A prop's text is picked from flags and is the same every time, for the whole epilogue.
  - People's lines (leaders, guards, wanderers) come from pools with joke memory, because players revisit the
    same seats for a week. The size rule is in 3.12.
- **Players are named only in honour lines**, and only from the hall of fame's own records or the epilogue's frozen
  record, in the name as it was recorded at the deed.
  - If the note filter matches any word in the name, the line says "a stranger", never asterisks.
  - `/anonymous` and the admin strike are honoured.
  - No new slots to compete for.
- **Every ending has lines:**
  - each route that controlled the cure;
  - **the slow cure**: the built fallback, where no route finished and the lab finished on its own work
    (`cure.slow`). Every faction claims a share and nobody fought for it; the voice is relief with an edge;
  - **handed over**: nobody held the room (`finale.route` 0), so the cure went to the faction with the most season
    Fame. The slow cure's lines, with that faction's name;
  - **a wipe with no cure** (the majors' no-cure codas at the first wipe warning). No wry lines anywhere but the Red
    Hand's last line. The Haven reads names.
- **The closing work speaks in the same voices.** Each closing stage, hand-in and scene of the week
  ([EPILOGUE.md](EPILOGUE.md#finishing-everything)) uses its owner's register: a rushed or bitter ending is said
  plainly, never as a punchline, and a walk-out is a goodbye, never a joke.
- **Every existing after-cure line must match the ending.** Today `HintAfterCure` (`Factions/RZ_LeaderVoice.c`)
  has Halcyon say Bastion's cure *"went out over every frequency instead"*, whoever won. That's false whenever
  Bastion controlled the cure. It is fixed in Phase 1.

### 3.6 Leader traits and humour

| Trait | Humour | Example of the register |
|---|---|---|
| Greedy | About money and the price of the job | *"Pay first, then the job."* |
| Paranoid | None. Asks who saw you | *"Close the door. Who saw you come in?"* |
| Merciful | Gentle, about food and rest | *"The soup doesn't mind either way."* |
| Ruthless | Dry and short, aimed at the job or the institution, never the victim | *"The form was short."* |
| Idealist | None, earnest. May laugh at themselves once | |
| Pragmatist | Wry about the deal | *"I like starving less."* (built) |
| Veteran | Gallows humour about their own dead, fondly | *"They itch."* |
| Talker | The most jokes, and they leak | |

**Stronghold leaders:**
- Bastion's commanding officer almost never jokes.
- The chair jokes once in a while, about food.
- The boss always jokes.
- The director never jokes on purpose.

### 3.7 Player lines in dialogue trees

Dialogue is off by default (decision 13), so the vanilla fallback ships first. The rules are the same either way.
- **A node may have one dry line** among its plain ones. It leads to the same result as the plain line it shadows;
  only the reply changes.
- **Only checks are tagged** (`[Nurse]`, `[Trusted with Bastion]`). Tone never is.
- **A dry line is never the only kind option**, and never the only way to say no.
- **No player line jokes** at the bitten, the volunteers, the exiled or the Kid.
- **No reward for tone, ever.** A good dry line earns nothing, so there's nothing to farm.
- **The vanilla fallback** (a person or puzzle object offers at most four choices): a dry line is dropped first, and
  only shows when the real approaches leave a slot free. As a label it is 40 characters or fewer:
  `Say: "Same as yesterday."`

### 3.8 The faction's word for you, and crew names

[REPUTATION_NAMES.md](REPUTATION_NAMES.md#9-the-factions-word-for-you) owns the words: what each faction calls you
to your face, per life and per cell, the lock's overlay, set words and crew names. This piece owns only how they're
said. The word reaches lines as `{player_word}`.

**Rules:**
- It is always the faction's own word, and never a moral word.
- It's said face to face only. The narrator never says it, and it never goes on air.
- A set word ("Sergeant {name}") is said as the faction says rank: Bastion plainly, the Haven with a smile, the Red
  Hand as a reminder.
- A disguise never changes the word anyone uses for you once they know who you are.

**Red Hand crew names** (conflict 31). LORE: status in the Red Hand is a name, and the crew picks it, usually after
something you'd rather they forgot.
- A crew name is **leverage, not an honour.** It means the crews have something on you. Say it the way the crews
  would: cheerfully, in front of people, as a reminder of what they know.
- The names and what earns them (Lucky, Snack, Legs, Skint, Pockets, Bailiff, Sorry, Smoke) are reputation's.
  Until you have one, you're "new face".
- A crew name mocks the player's luck, nerve or skill. **It never mocks a kindness or a victim** (so "Nanny", for
  walking the Kid home, is cut), and "Stray" is never one.
- At Champion the boss says it; it still isn't respect. It's the book being polite.

### 3.9 Faction kit and disguises

**Earned clues and bought kit** (the amber ribbon, the red armband, faction armour bought at Trusted or Champion)
say whose you are. The red armband means the crew has something on you.
- A guard who sees a rival's clue makes one plain remark, at most once a visit.
- The remark says only what the hostility state already does.
- It never hints at a consequence that doesn't happen, and never at safety where there's danger.

**Disguises.** [DISGUISE.md](DISGUISE.md) owns the look: one fixed check, shown before you commit, with no suspicion
meter. The voice follows its results and has **no fixed count**:
- A pass can come with no line at all, at least one time in three, chosen by the server.
- A failed look at a hostile post can go straight to the guns.
- A noticing line plays only when the look is actually being made: the guard turns to watch you. Walking away and
  back never gives a free early warning.
- The first noticing line may be dry (*"Nice uniform. Whose was it?"*), but only while the guard turns to watch
  you. Never while nothing changes.

### 3.10 Running jokes and the majors' states

Running jokes read the majors' flags through the voice bank's flag conditions. They never switch on a day.

| Running joke | Runs while | Then |
|---|---|---|
| The Red Hand's *"Still waiting for relief? We'll keep the kettle on."* (built, `Guidance/RZ_GuidanceText.c`) and its jokes about Bastion's dead radio | `orders.stance` unset or 1 (The Call Stands). The majors make the kettle line nightly at 1 | Retired at stance 2 (All Stations: the orders read aloud, "they've lost their joke"), and at stance 3 (Bastion, Command) or 4 (Expended). In the epilogue it also stops if the CO closes the net: *"Nobody left to laugh at."* |
| Bastion's *"No reply."* | Always: it's a fact | The sergeant's dry tag goes plain from stance 2 |
| Haven gossip about the ration cards | `haven.rule` 3 (Counted) | Each rule has its own gossip: the shared pot (1), workers first (2), the arrangement (4, always said sideways) |
| The toll sign's handwriting and the Red Hand's book jokes | The book stands | After `book.burnt` or `book.debts_dead`, the handwriting changes and the book jokes stop for a real day |
| Rivals' gossip about a page | Plain, from the second real day after it's in the open | Never a joke from the page's owner |

### 3.11 Information comes first

Some kinds carry information: chatter with a truck time, toll hours, a watch rotation, the lab's count, an outlet's
next dose.
- Their information clause is fixed. A joke is an optional tail from the bank, riding on top, never in its place.
- The lint checks that these kinds keep their information tokens.
- That way players never learn that joke lines are filler they can skip.

### 3.12 Repeats

- **Face to face:** the last 24 joke lines each player heard, with the real time, saved on the profile (across
  lives). A joke heard in the last 24 real hours is swapped for a plain line from the same pool. Reconnects and the
  daily restart don't wipe it.
- **Shared channels:** the last 16 jokes each channel aired, with the time, saved with the season. No joke airs
  twice within 6 real hours.
- **A pool that runs out falls back to plain lines.** Never repeat a joke to fill a gap.
- **Minimum pool size.** A variant (a mood, a route, an ending) ships only when its joke lines number at least
  1.5 × (how many times it's heard in its window × its joke share), at the fastest cadence. Otherwise the base pool
  is used.
  - The fastest cadence is a brand-new player: the radio every 12 minutes, and chatter every 10 minutes rotating
    through the factions.
  - A Merciful leader's joke pool, for a veteran who visits that seat about 10 times a day at 1 in 3:
    10 × 1/3 × 1.5 = 5 joke lines at least.
- **The epilogue's week.** The window is a real day, so a seat's epilogue pool needs the same 5 joke lines per
  ending whether the epilogue is two days or seven. What a week adds is plain lines: a seat with too few is heard
  word for word on day 5. So each stronghold and seat leader gets at least **8 plain lines per ending** for the
  epilogue, and the closing work's people get their own lines, owned by EPILOGUE.md.

### 3.13 Content budget

Pools are ranked by how often a player hears them, and only the top of the list grows:

| Pool | Heard | Today | New |
|---|---|---|---|
| Leader openers and closers (by faction and trait register) | Every visit | ~65 | ~80 |
| Meal, night and shift asides | Every visit at those times | 9 | ~60 |
| People at outposts and safehouses | Every visit | ~45 | ~15 |
| Open channels: newcomer lines, chatter tails, Hope and Bleak variants | Every 12–42 min | ~40 | ~90 |
| Road news and rumours | Each wanderer meeting | ~47 | ~30 |
| Race lines and the five pitches | 75% to the lock | 0 | ~30 |
| Route lines, face to face (in, out, Independent, unsided, lost) | Each visit after the lock | 0 | ~60 |
| Saying the word and crew names (the words themselves are reputation's) | Each opener | 0 | ~30 |
| Epilogue people (4 factions × 7 endings: 5 jokes and 8 plain each) | Each visit, for a week | few | ~360 |
| Epilogue props (about 8 places × 7 endings) | Once per place | few | ~56 |
| **Total** | | ~250 | **about 810** |

The seven endings are the five routes, the slow cure (with handed over) and no cure. The closing work's own lines
(its stages, hand-ins and scenes) are counted in [EPILOGUE.md](EPILOGUE.md#writing-the-epilogue), not here.

That is one author's work, spread over phases 3 and 6. The biggest share waits for the epilogue, and its plain
lines are built from parts (a route clause, a place line, a "you" line) to keep the count down.

## 4. Grey choices

### 4.1 The grey test (every choice passes all seven)

1. **Two options someone would defend.** Someone in the world would argue for each.
2. **Every option changes the world, and the text says how.** A cost is real only if something changes that the
   world or a later line reads:
   - food at a house, or people at a safehouse;
   - a person's fate, or a town's infection;
   - a leader's memory flag;
   - a season flag that a later line or job reads;
   - the player's Favor or antidote.

   A sentence with nothing behind it fails the test.
3. **No option is best on every axis** (4.2).
4. **Labelled as actions, not judgements** (4.7).
5. **The giver's framing is one side.** At least one option needs something the giver didn't say.
6. **Something comes back later, at least sometimes** (4.5).
7. **Doing nothing and failing are outcomes too** (4.4).

### 4.2 The payout spread

- **The giver's way** pays the giver's Fame and Favor in full.
- **The option kindest to the person in front of you** pays the least Favor, often none. In return it brings a world
  change (they live, they join a house, they keep their food) and a deed (4.5).
  - Leader memory is a flag (helped, saved, crossed), per player and life-stamped, as reputation sets it. It is set
    once and then adds nothing.
  - So memory can never be the kind option's whole reward, and it can't be farmed.
- **The option that hurts a third party** pays more Fame or Favor with one faction, and costs Infamy or a world loss
  somewhere else.
- **The clever way** (trick, betray, keep both) pays best on paper.
  - It has a discovery chance in the mod's 15–35% band, and a worse result when it's found out.
  - Being found out ends the run for the whole group (4.8).
- **No new loot.** A grey choice never adds loot beyond the mission's built reward.
  - Where a built crate exists (Supply Cache, Survivors Trapped, Radio Relay, world events), the grey is who gets it,
    not more of it.
  - Side quests only move items and food that already exist.
- **Background and standing change which option is cheapest**, so different characters choose differently.
- **Labels show costs and requirements, and hide rewards and consequences.** *"Pay their dues yourself (20 Red Hand
  Favor)"* is right; *"(+10 Haven Fame)"* is not. New Vegas does the same.
- **Small side quests pay mostly in deeds and world change**, under errands' small-pay ledger.

### 4.3 Givers who lie or hide things

- **At most one omission per brief.** It must be plausible and in the giver's voice (*"He volunteered."*).
- **Not every time.** Each lying brief has two or three possible omissions and one honest version, and `CanonPick`
  picks one per season.
  - So last season's wiki is wrong half the time.
  - Check-tagged options appear on the honest version too, so a passed check can say *"He really did volunteer."*
    A tag never gives a lie away.
- **No gotchas.** Before the choice, at least one approach can find the truth, and at least one of those is open to
  everyone:
  - asking the person themselves, or a clue at the scene;
  - checks find it faster or more fully: a background, standing, a Radio Operator on the secure channel. The talk
    checks are dialogue's one persuade score (your access rank, mixed tiers one lower).
- **The outcome names it**, either way: *"He hadn't volunteered. He was in the book."*
- **Lying follows traits:**
  - Paranoid and Ruthless leaders leave things out.
  - Talkers leak.
  - Idealists don't lie, but leave things out to protect someone.
  - Stronghold leaders hide their page.
- **A caught lie has no reward of its own.** The truth opens or changes an option, and that's the payoff. It sets no
  memory flag.
- **Making a lie public is capped.** If a player finishes a branch and takes the lie public ("Tell the crew"):
  - the giver's faction loses 1 morale, at most once per faction per real day and once per lie per season, inside
    the existing morale caps;
  - an outpost leader caught out loses 5 loyalty, within the hourly limit.

### 4.4 Failure as a branch

- **Every failure has its own outcome text and world effect:** running out of time, losing the fight, or nobody
  choosing.
  - It's usually the worst result for the person in front of you, but not always: *"Nobody chose. He went in alone
    and walked out, which annoyed the crew more than anything."*
- **A mission the world made pointless** is still called off with no blame, as built.

### 4.5 Callbacks and deeds

- **What records a deed.** A choice that involves a person, a house or a leader records a deed for that life, in the
  one deeds store (`RZ_Deeds`, life scope).
- **Deeds are weighted:**
  - heavy: a person's life, a page, a betrayal;
  - medium: a house's food, a lie told;
  - light: a small side quest.
- **Twelve callback slots per life.** When they're full, the lightest, oldest deed goes. Light deeds hold at most four
  slots and never push out a heavy one, so a day of small kindnesses can't wash the record clean.
- **People bring one up plainly, at most once a visit:** *"You're the one who walked the debtor to the line."*
- **Memory outlives deeds.** A leader's memory lasts across lives (earlier lives at half); deeds end with the life.
  When a leader remembers the player but the deed died with their last life, the leader gives a generic line in their
  faction's voice instead.
- **A callback can open a small side quest**, through the one callback queue. The man you let go asks a favour. The
  house you short-changed is short again.

### 4.6 What each kind of mission should offer

These become two columns, "who pays" and "watch for", in the mission-style design's table of approaches by kind. One
table, not two. Offer at least two grey options in each row.

| Kind (examples) | Grey options | Who pays | Watch for |
|---|---|---|---|
| **Delivery** (Medicine Run, Fence the Goods, Letters, Cold Chain, A Quiet Arrangement) | Deliver; deliver where it's needed more; open it first; skim; sell to the rival | The receiver, or the place that went without | Opening it tells you something worth knowing |
| **Collection** (Requisition, Protection Racket, Off-Book Requisition, Debt Collection) | Collect it all; collect half and cover the rest; collect from someone who can afford it; warn and refuse; give it back | The house (food), you (Favor), the giver (memory) | No option where everyone is happy |
| **Rescue and escort** (Survivors Trapped, Refugee Column, Struck Off, Bust Out, The Pit's Debtor) | Where the giver wants; where *they* want; where they're safest; hand them over; leave the one who can't make it | The giver's numbers, a house's food, the person's freedom | Always ask the person what they want |
| **Hold and defend** (Checkpoint Duty, Lab Defence, Our Crossroads, Fortify) | Hold; hold for a higher price, raised mid-fight; get the people out instead; let the rival in | The place, the people inside, your standing | The grey comes before the fight or at a beat in it |
| **Hunt a person** (Struck Off, Kill the Officer, The Rat) | Kill; bring back alive; let go and claim the kill (proof needed, so it can be found out); hear them out; hand them to someone else | The target, the giver's trust, a later callback | Every target gets a line. For Bastion, bringing a man home is the faction's own way |
| **Retrieve and search** (Research Data, Crash Site, The Real Orders, The Notebook) | Return; read first; copy; burn; sell; give it to the person it's about | Whoever the information hurts | Reading it adds a journal note |
| **Research** (Live Capture, Serum Trial, Unsanctioned Trial, Patients Not Samples) | The lab's way; the person's way; the Red Hand's price; wait for consent and lose the sample | The volunteer, research, the Haven's trust | The numbers never stand in for the person |
| **Mercy** (The Bitten Friend, Walk Him Out, a bitten wanderer) | End it; let them walk; to the lab; your antidote; sit with them | You, the town, the person | **No jokes, ever.** The bitten person chooses if they can |
| **Perimeter** (The Blind Spot, Learning the Drones, The Crossing) | Do it; send someone else (who?); take the package and leave the person; get them away from the line | Whoever walks toward the line | The line never opens. The grey is who gets sent |
| **Investigation and puzzles** (The List, The Witness, The Silent Village, Patient Zero) | Tell the giver; the family; the rival; keep it quiet; tell everyone | Whoever the truth hurts | The answer is a reason, not a villain |
| **Trade** (Over the Fence, Ration Run, Rifles for Sale, Sell the Formula) | Deal; cheat (can be found out); take a cut; switch sides; turn the leader in | The other side, the leader, the stores | Both sides have a good reason to trade |
| **World events** (Wandering Trader, Wedding Party, Rival Survivors, Distress Call) | Who gets the built loot; protect or rob; help the strangers or not | Other players, the stranger | Say so when the cost falls on another player |
| **Leader personal jobs** | Do it; do it and turn them in; do it badly on purpose; tell their people | The leader, the stronghold, whoever the rules are bent for | The leader's reason is human, and they say it if asked |
| **Small side quests** (errands) | The favour; the favour at someone else's cost; your way; say no kindly or unkindly | Small and local: a tin, a lie, a rule bent | See 4.6b |
| **Questline and major stages** | All of the above, plus siding | As above | The choice echoes the faction's hard edge |
| **The finale** | Before 90%: any route, freely. After: the grey moves inside the route | Everyone | No route reads as the good ending |
| **The epilogue's closing work** | Finish it properly; finish it rushed; leave it to close on its own; do it someone else's way | Mostly the people it's about | A rushed or bitter ending is still an ending, said plainly |

### 4.6b Small side quests

[ERRANDS.md](ERRANDS.md) owns the dealer, the catalogue and the pay. This is how they sound.
- **Always caused, and the brief says so in voice.** The cause is something the player can check:
  - a world fact: *"Since the truck didn't come..."*;
  - the player's standing: *"You're the one Bastion waves through..."*;
  - or a deed.
- **Spoken by a seeded person** with one personal detail (a name, an old job, a habit), not by the template. Side
  quests are the text players hear most, so the humour lives in that person's detail and changes with the person.
- **Only where that person would be.** A stranger speaks only at a spot where a stranger makes sense (a road people
  use, a wanderer's route, a holding's approach, a campfire, a stopped car, a fresh event or a body, a town edge at
  dusk), never in the empty middle of nowhere and never because a timer ran out. So the first line says why they're
  there: *"Saw your fire."*, *"That's my wife's car."*
- **Small means small.** The grey is a pinch, not a tragedy.
- **Gates against farming** are errands': no new items, light deeds, the small-pay ledger, cooldowns per place and
  per giver.
- **Ideas, each tied to the moment** (errands owns the full list):
  - a Bastion clerk's missing form when the post is short;
  - a Haven house that needs someone to count bowls honestly during a shortage;
  - the Red Hand's reminder to a debtor, for a player the crews know;
  - Halcyon's mislabelled case, for a player with initials in the log;
  - the outpost's generator, watch or post;
  - a stranger at a holding's approach with something to sell.

### 4.7 Phrasing choices and outcomes

- **Choice labels:**
  - Verb + object, from the player's point of view, 40 characters or fewer, with no judgement words.
  - Then the check or cost in short brackets: `[Police or Known]`, `(20 Favor)`.
  - The whole label is 64 characters or fewer, so the action menu doesn't cut it.
  - Good: "Walk him to the line", "Get him away", "Pay what he owes". Not: "Save him", "Do the right thing", "[Evil]".
  - Built labels over the limit go in the lint baseline, and get shorter tags in Phase 4.
- **Outcomes come in three beats**, then a separate system line:
  1. What happened, in one concrete sentence.
  2. What it cost, and to whom: one detail in the world.
  3. One reaction, from a voice that would have one. If there's a joke, it goes here.

  Then **the system line**, shown after: standing and pay as numbers, like New Vegas's reputation notice. Built
  outcomes that put *"(+%1 Red Hand Infamy)"* inside the voiced text have it moved out as each one is touched.
- **No narrator verdicts.** Never *"You did the right thing"* or *"Monster."*
- **Use the faction's own word:** "contribution", "borrowed", "walked out", "struck off" (Bastion's deserter only),
  "paid" (the book), "expended", "the other two". The euphemism is the grey.

### 4.8 The shared server

- **Who can choose.** Only players the job was offered to and who took part: 60 s within 150 m, or a step done (the
  plan's one participation rule).
  - Deciding uses missions' 5-second hold, with one objection per player per job and Stand with.
  - Everyone else who took part gets a line saying someone else made the call: *"Someone got to him first. His
    KIA's rubbed out."*
- **The clever way closes the honest one.** Once anyone in the run tries a trick or a betrayal, the honest branch is
  closed for that run. If they're found out, the run ends in the found-out outcome for everyone in it.
- **Standing is personal; world consequences are shared.**
- **Names in shared text.** Headlines name no players. These exceptions stay as built:
  - the Turned Player event's title (its point is whose gear and whose friend);
  - the Long Table's headlines that name online family members (REPUTATION.md);
  - players' own radio station presets (they sign their own).

  The Turned Player brief loses its quip in Phase 1. Epilogue honour lines follow 3.5.
- **Late joiners read it in the world:**
  - headlines (the newest 10) and road news;
  - **boards of record**: a few stable boards whose text is worked out from season flags each time, the same for
    everyone, with nothing new saved. The Haven council's pinned notes, Bastion's roster board, the toll sign's
    tally, the lab door's sheet.

  The epilogue owns those props (one set for the season and the epilogue). This piece asks for their in-season mode
  and writes it.

## 5. The line: dark humour, not edginess

### 5.1 Three tests

1. **Who's laughing?** Someone in the world who would. Never the narrator, the system or a plain hint.
2. **At what?**
   - Up: the broadcast, Command, the forms, the book, prices, the lab's euphemisms.
   - Sideways: rivals, in gossip.
   - At themselves.
   - Never down at someone with less power in the scene: the bitten, the volunteers, exiles, the people who never
     arrived, children, or a named dead person. The one exception is the dead person's own people, fondly.
3. **What's underneath?** A true detail: a number the world holds, a name, a habit, a shortage. Take the joke out
   and something real is still there; usually the next line says it.

**Callous isn't mocking.** A cruel speaker may be callous about price, odds or count, because that's who they are (the
Red Hand's tips, a crew with a stopwatch). But:
- the target is never the victim's fear, pain or body;
- the line is never the narrator's;
- the next beat shows the cost, in the same outcome.

### 5.2 Never, in any voice

- Sexual violence, in any form. It isn't mentioned at all.
- Torture or suffering as the punchline; gore described for effect.
- A child hurt, or a joke at a child's expense (the Kid, the class trip).
- Suicide as a joke.
- **Rudi's end as a joke.** Rudi was put out of his own garage at dusk on Day 7. The Red Hand's lie about it (*"He was
  bitten"*) is said flatly, and nobody laughs after it. The boss's line stays as LORE has it: *"He let people pay
  later. You can't run a road like that."*
- Slurs, or jokes about real-world groups, ethnicities, faiths as such, disability, or illness as identity. The Preacher
  is a person and the church is a building.
- Real countries, governments, parties, real pandemics, or their politics. The broadcast mocks official language in
  general.
- **The Long Table.**
  - Describing it, or the family or the narrator joking about it.
  - "Pork" belongs to the Red Hand, only at the moments already written (when the house is found out, in Cargo and in
    C1), and silence always follows it.
  - **No ambient pool ever hints at it**: no meat of unknown origin, no "don't ask what it was", no "a place set". The
    built Red Hand meal aside does exactly that every evening, in every season, and is fixed in Phase 1.
- A player's name as the butt of a joke.
- Memes, internet talk, pop culture, fourth-wall jokes, modern slang.
- Praising cruelty. Only a faction that would laugh does, and the next beat shows the cost.
- A joke that replaces information (3.11).
- A joke about the walk out, or about anyone who went through the barrier.

### 5.3 Signs a line has turned edgy

- The joke would work in any setting: there's no detail in it.
- It's louder than the thing it's about.
- It needs a swear word to land.
- The victim has no name and no line, and is the punchline.
- The player is made to feel stupid for choosing kindness.
- It repeats: the same joke to one player within a real day, or on one channel within 6 hours (3.12).

### 5.4 Each faction's humour

| Voice | Jokes about | How | Goes quiet about |
|---|---|---|---|
| The Haven | Food, chores, house rules, itself, other houses' cooking | Warm, practical, sideways | The List, the mouths, the walks, the Long Table; anyone's dead but its own; the big ones it knew by name |
| Bastion | Forms, Command, the roster, rank, rations | Dry, procedural, deadpan, often in the passive voice | Day 2 at the north road, the pilot, "expended", its deserters; its orders once read |
| The Red Hand | Everyone, Bastion most; prices, the book, the toll; itself; the infected, in its own words | Mocking and casual. The joke is how it avoids saying something | Rudi; Cargo; the mothers and sisters in safehouses; the drones' count (the joke dies on air) |
| Halcyon | Rarely on purpose. Tired understatement; words that are exact and terrible | Precise, short | The other two by name, the three weeks, the volunteers |
| The broadcast | Never on purpose | One wrong word | Everything |
| Wanderers | By role: the Trader prices, the Mechanic cars, the Deserter Bastion (bitterly), the Doctor gallows humour, the Scavenger his own rumours, the Courier the post | Their own way | The Preacher almost never jokes. The Kid never does, and nobody jokes about him |
| The infected's words ([INFECTED.md](INFECTED.md#people)) | Only the Red Hand jokes about them | Each speaker's word, always with the place: the Haven's name or "the big one", Bastion's contact and "the heavy", the Red Hand's crew nickname and "the boss", Halcyon's pack number and "the index" | Never "alpha", "tribe" or "crawler" in player text; never "strays" for the infected |

### 5.5 Before and after

| Edgy | Fixed | Why |
|---|---|---|
| Red Hand, about a Haven death: *"Another do-gooder down. More tins for us."* | *"Heard {subject_town} buried someone. We'll keep the noise down on that road tonight. Don't tell anyone we're nice."* | The joke is now at the Red Hand's own expense, and the death stays real |
| Halcyon, after a trial: *"Volunteer seven popped. Science!"* | *"Volunteer seven didn't walk out. The notes are thorough. The director asks that someone from the Haven come for his coat."* | Exact and guilty, and the coat makes him a person |
| Walk Him Out brief: *"He's going to turn. Bring popcorn."* | *"He declared it himself, at supper. He'd like it done before dark, by someone who isn't family."* | Mercy jobs carry no joke |
| Bastion: *"Shoot first. The paperwork's easier if they're dead."* | *"The form for a civilian casualty is four pages. For an infected it's one. The sergeant checks every one-pager twice."* | It shows the pressure, and the man resisting it |
| Red Hand meal aside (built): *"Dinner's on. Don't ask what it was."* | *"Dinner's on. Tins. Someone's written prices on them, so don't eat the expensive ones."* | The old line hinted at the Long Table every evening. The new joke is about prices, which is the Red Hand |
| Halcyon (built): *"It's done. I keep checking the numbers anyway. Habit."* | *"It's done. I keep checking the numbers anyway. Nobody has asked me to stop."* | The Haven also ends on "Habit." One punchline, one owner |
| A crewman about Rudi: *"Rudi? Bit, wasn't he. Bit thick, too."* | *"Rudi was bitten."* Nothing after it. The crewman looks at the shutter, then at you | The lie is said flatly, and the silence carries it |

### 5.6 Template rules

- **Only real tokens.** Use the `Fill` table's tokens (`{day}`, `{days_since_command_words}`, `{cure_percent}`,
  `{leader:id}`, `{town:N}`, `{canon_town:key}`...) plus reputation's `FillFor` tokens, the infected's
  `{tribe:N}`, `{alpha:N}` and `{young:N}` (each in the speaker's own word), and the voice bank's context tokens:
  - `{place}`: the speaker's settlement;
  - `{subject_town}`: the town a line is about;
  - `{player_name}`: face to face only, through the note filter, otherwise "stranger";
  - `{player_word}` (3.8);
  - `{count}`: only once the majors' `orders.count` exists.

  There's no home town: players don't have one.
- **No invented numbers.** If the world doesn't hold a count, use a token or say it without the number. Bastion's call
  says *"Strength as last reported"* until the majors give it `{count}`.
- **No pronouns for seeded people** (leaders, the infected's named leaders, seeded named people). The seed stores no
  gender, so use the name or the title.
- **Ambient notices never reuse a questline clue's text.** The Census's curfew sheet stays a clue.
- **Seeded names never collide with LORE's named people.** Halas (the Long Table's first meal), Tomas (the Long Table's
  grace), Alena and Ivo (Bastion's dead and its signaller) and the like are kept out of the seed lists.

## 6. The writing pass

| Phase | What | Files | Done when |
|---|---|---|---|
| **0. The guide** | WRITING.md (section 1); DESIGN.md's "Grey choices"; the roadmap; every link in section 1 | Docs, `CLAUDE.md`, `README.md`, code comments | The link check passes; the Doc map lists exactly `docs/` |
| **1. Built contradictions** (text only) | The Red Hand's meal aside (5.5). `HintAfterCure`'s Halcyon line on Bastion: only when Bastion didn't control the cure, plus a Bastion-controlled line, and the Haven's "The scientists did it" only when Halcyon did. The Turned Player brief's quip. Halcyon's "Habit." | `Factions/RZ_FactionVoice.c`, `Factions/RZ_LeaderVoice.c`, `Missions/Events/RZ_WorldEvents.c`, and any doc that quotes them | `tools/compile_check.sh` exits 0; the docs match |
| **2. Inventory and lint** | Built: `tools/strings_inventory.py`, `tools/voice_lint.py` and its baseline ([WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#text-lint)). Left for later phases: the lint learns reputation's `FillFor` tokens and the voice bank's context tokens (7) when they're built | `tools/` | Done |
| **3. The voice bank** | Bank, `SeasonMood()`, temper, grief, joke memory, picker. Move the top-ranked pools (3.13) and tag their registers. The broadcast reads the mood and gains its two lines. Loudspeakers, the wipe line and guidance tips get the lint and a review, but stay out of the bank (few lines, or their own attribute class). `tools/voice_sim.py` | `Factions/RZ_FactionVoice.c`, `Factions/RZ_LeaderVoice.c`, `Factions/RZ_FactionLife.c` (chatter, secure channels), `Guidance/RZ_GuidanceText.c` (newcomer open channels), `World/RZ_Wanderers.c` (road news), `World/RZ_Safehouses.c`, `Director/RZ_Broadcast.c`; review only: `World/RZ_Perimeter.c`, `Core/RZ_Seasons.c`, `Guidance/RZ_GuidanceTips.c` | The simulator shows each mood's share inside 3.2 and no repeats inside the windows at the fastest cadence; the compile check passes. WRITING.md and DESIGN.md gain the mood, temper, grief and joke memory |
| **4. Choice missions** | The grey test on every choice. Each cost line gets a world change behind it: a flag plus at least one reader. The three-beat outcome and system line. Who can choose; the clever way closes the honest one. Labels shortened. Honour Among Thieves' "let him keep it" becomes a branch. Then: Local Law, Shut the Door, The Ledger, Requisition, Protection Racket, Struck Off, No Reply, the three hard edges, Patients Not Samples, The Notebook, Private Collection, The Silent Village, The Witness, Rein In | `Missions/`, `Puzzles/` | Every choice passes 4.1; MISSIONS.md's Result text matches |
| **5. Givers who lie** | About 12 briefs, each with 2–3 omissions plus an honest version, picked once a season: Learning the Drones, A Quiet Arrangement, Requisition, Names for Bastion, Off-Book Requisition, Rifles for Sale, The Real Orders, Unsanctioned Trial, Private Collection, Sell the Formula, The Pit's Debtor, Debt Collection. For each: the truth-finding approach open to everyone, the checks, the outcome lines, and the morale cap | Missions | No gotchas (4.3); no tag gives a lie away |
| **6. State variants** | Majors' running jokes (3.10). Race lines and pitches. Route lines face to face. Lost-route and unsided lines. Saying the word and crew names. Disguise look lines. Epilogue lines with [EPILOGUE.md](EPILOGUE.md), for every ending including the slow cure, handed over and no cure, and the plain lines for a week. Every after-cure line made ending-aware | Voice bank | Every variant meets the minimum pool size (3.12) |
| **7. Check** | `voice_sim.py` over three scripted seasons: season speed 1, season speed 2, and a gate that runs to the slow finish. The runtime test with the voice log on and Voice > Force mood cycling the moods, to catch script exceptions and confirm the in-game picker matches the simulator. The author reads one player's radio log for an hour | `tools/`, `tools/runtime_test/` | No exceptions; the shares and repeats in the simulator are inside the targets |

**Order.**
- Phase 0 can land now; phase 2 is built.
- Phase 3 needs code.
- In Phase 4, the label, outcome and eligibility fixes can start after Phase 2. New options wait for the mission-style
  and dialogue designs.
- Phase 5 waits for the dialogue design's checks.
- Phase 6 waits for the finale, the epilogue, the majors, disguise and the reputation names.
- The questlines get the lint and the grey test last, not a rewrite.

**Code rules apply to every phase.**
- Attribute classes have no initialisers and are made with `RZ_Util.NewWithDefaults`.
- Run `tools/compile_check.sh` before every script commit.
- After cross-system changes, run the runtime test.
- Docs say "planned" until something is built.

## 7. Engine needs

- **Voice bank:** an `RZ_VoiceLine` attribute class made with `RZ_Util.NewWithDefaults`, with no field initialisers.
  Fields: a stable id; channel; faction; kind (aside, chatter tail, rumour, opener, closer, reaction, notice hand,
  epilogue, race, pitch, route, word); register (PLAIN, WRY, JOKE, TAUNT, BLEAK, SILENCE); a mood mask; a trait mask;
  stage and cure ranges; up to two flag conditions (key, operator, value, so it can read `orders.stance` 1,
  `haven.rule` 3 or `book.burnt`); a route relation (any, in-group, out-group, Independent, unsided, lost); an ending
  (any, a route, slow, handed over, none); a weight; and the text. Code defaults live in `Factions/RZ_VoiceDefaults.c`,
  one `Line(...)` call per line, so the offline tools can parse them. Two attributes on `RZ_GameModeComponent`:
  **Voice > Lines** (adds a line, or replaces the default with the same id) and **Voice > Disabled** (ids to switch
  off). This is not GuidanceTips' copy-everything model. Today's code stays as the fallback whenever no bank line
  fits.
- **`RZ_WorldFacts.SeasonMood()`:** the ladder (Epilogue, Hope, Bleak, Grim, Early) with its holding rules: two checks
  before a change, 60 online minutes minimum. `Temper(faction)` from morale, with gaps at 30/38 and 75/67.
  `RZ_Broadcast` exposes `InBreakthroughWindow()` and its stall timers, ignores the stall at `AtFinaleGate()`, and
  reads `SeasonMood()` for its line (one table). It gains two lines: one at the gate, one for Aftermath under 50%.
- **`RZ_ToneGuard`** (runtime only): a ring of 8 deaths (position, time, faction) for face-to-face lines, 1.5 km for 10
  minutes; a quiet-until time per faction for its radio, 20 minutes. It is fed by the existing death headlines
  (leaders, wanderers, Remembered, truck crew funerals, Turned Player) and by `RZ_Safehouses` when `House().m_iDied`
  goes up (that safehouse for 30 minutes, and the Haven's radio). `AllowsJoke(position, faction, channel)`. The Red
  Hand's send-off lines are exempt.
- **Joke memory:** per player, on `RZ_PlayerProfile` (by identity, across lives, saved), the last 24 joke ids with real
  unix time, a 24-hour window. Per shared channel, the last 16 joke ids aired with time, saved with the season, a
  6-hour window. When a pool's jokes are used up, a PLAIN line from the same pool.
- **Deeds:** tone's 12 weighted callback slots live in the one `RZ_Deeds` store, life scope (the plan's shared engine),
  beside reputation's and missions' life records. `Record(identity, key, weight)` on the current life. When full, the
  lightest, oldest goes. Light deeds hold at most 4 slots and never push out a heavy one. Built once, in the plan's
  phase 1.
- **Context tokens** filled by the voice picker: `{place}`, `{subject_town}`, `{player_name}` (face to face only,
  through the note filter, "stranger" on any match), `{player_word}`, and `{count}` (only once the majors'
  `orders.count` exists). The lint knows them alongside the Fill and `FillFor` tables.
- **From the finale:** `RouteOf(identity)`; whether the player's route lost; `finale.route` (0 nobody, handed over) and
  `finale.finisher` (0 the lab alone) as the finale defines them; the pitch triggers at 75%, 80% and 85%; the lock
  prompt carrying the admission line; and `cure.slow` when the lab finishes on its own work.
- **From reputation:** the word, the lock's overlay and the crew name, through `{player_word}` and its `FillFor`
  tokens. The voice never computes a word of its own.
- **Ambient notice prop:** one generic puzzle-object prop (`RZ_ClueComponent`) at Bastion checkpoints, Red Hand
  roadblocks and safehouse gates, spawned through `RZ_Marks` (the one prop spawner). None at a pack's range (the
  infected's marks) and no Halcyon flyers (built). It spawns within 150 m of a player and is removed when no player
  has been within 250 m for 2 minutes, or when its anchor goes. Its text is picked at spawn from a hash of the anchor
  id, the season seed, the season mood and the relevant flags, so it's the same for every reader. Never within 1 km of
  an active questline clue of the same faction. Nothing is saved. An optional prefab attribute; empty turns it off.
- **Boards of record for late joiners:** the epilogue's props in an in-season mode. Text worked out from season flags
  each time it's read, the same for everyone, nothing new saved.
- **`RZ_Outcome.Tell(gm, identities, what, cost, reactionFaction, reactionPool, systemLine)`:** the three voiced beats,
  with the reaction drawn from the bank (grief and repeat checks apply), the standing and pay as a separate system
  line, and a "someone else made the call" line for the others who took part.
- **Choice rules in the mission base:** who can choose (the plan's one participation rule); the decide-scene hold; a
  clever attempt closes the honest branch for that run; being found out ends the run for everyone in it with the
  found-out outcome.
- **Lie variants:** each lying brief has a set of 2 to 3 omissions plus an honest version, picked once a season with
  `CanonPick`, kept in the one lie store. Making a caught lie public costs 1 morale, at most once per faction per real
  day and once per lie per season; an outpost leader caught loses 5 loyalty. No memory flag for catching a lie.
- **Offline tools**, none run in the game (the inventory and the lint are built): `tools/voice_sim.py` (parses
  `RZ_VoiceDefaults.c` and replays scripted seasons at each guidance cadence, in the manner of `tools/pacing_sim.py`, reporting joke share by mood, repeats per window and
  grief skips).
- **Debug settings**, off by default: the voice log (`[RZ] voice: <channel> <faction> <register> <id> mood=<mood>
  listener=<identity or all>`) and **Voice > Force mood**, so the runtime test can cycle moods it wouldn't reach in
  60 to 90 minutes.

## 8. Examples

1. **Broadcast, Bleak** (Aftermath, cure under 50%), in the built frame: *"Day 23. This area is under federal
   quarantine. Do not approach the perimeter. Residents are thanked for remaining indoors. Residents are thanked for
   remaining."*
2. **Broadcast at the finale gate** (research has stopped by design, so the stall lines never play here): *"Day 39.
   This area is under federal quarantine. Do not approach the perimeter. The status of this area is under review.
   Remain where you are."*
3. **Haven open channel, a house on half portions** (a joke about the soup, never about who eats): *"If you can hear
   this, {place} is open. We're on half portions, so bring water if you can, and bring yourself either way. The
   soup's thin, but it's honest."*
4. **Bastion nightly call, Grim,** holding no number the world doesn't have: *"Command, Bastion. Nightly call.
   Strength as last reported. Civilians sheltered, as last reported. No reply for {days_since_command_words} days.
   The sergeant asks me to log that he isn't angry. Over."* A long hiss. *"Bastion out."* (Once the majors' muster
   sets `orders.count`, the first line becomes *"Effective strength {count}."*)
5. **Red Hand open channel after a Bastion truck was robbed** (aimed up, at Bastion's own habit): *"To the Bastion
   driver on the {subject_town} road: you left your truck. Don't worry. We signed for it."*
6. **Halcyon open channel** (exact, and only numbers the world holds): *"Lab, all stations. The cure is at
   {cure_percent} percent. The director asks that nobody round it up."*
7. **Rumour from a wanderer** (biased, about rivals, maybe half true): *"Heard Bastion's buying its own rifles back
   off the Red Hand. Paying in diesel. Says it's cheaper than the paperwork."*
8. **Ambient notice at a Bastion checkpoint** (a printed form unlike the Census clue; the handwriting is picked when the
   sheet appears and is the same for everyone): *"TASK FORCE BASTION. NOTICE TO RESIDENTS. Travel on this road is by
   permit. Permits are not being issued at this time."* Under it, in pencil: *"When?"* Under that, in grease pencil:
   *"See above."*
9. **Red Hand meal aside,** replacing the built Long Table hint (Phase 1): *"Dinner's on. Tins. Someone's written
   prices on them, so don't eat the expensive ones."*
10. **Leader, a Merciful Haven council head, to a player the Haven is watching:** *"Sit down. Eat. You can tell me what
    you did after, or not. The soup doesn't mind either way."*
11. **Leader, a Ruthless Halcyon field lead, after an unsanctioned trial** (aimed at the institution, never the
    volunteer): *"The volunteer consented to everything on the form. The form was short."*
12. **A lying brief with an honest season** (Learning the Drones). Brief: *"Walk a lad out toward the line and count how
    long the drones take. He knows the way. He volunteered."* Anyone can ask him why he's going. In a season where it's
    a lie: *"I owe. It's this or the pit."* (An Ex-Con reading the crew also learns who he owes.) In an honest season:
    *"I did volunteer. If the count's good, my sister's debt is done."* The outcome when it was a lie: *"He hadn't
    volunteered. He was in the book."*
13. **Haven giver who leaves something out** (a stores job; the rats are the house's own people "borrowing"): *"The
    stores came up short again. Rats, probably. Big ones, with keys. Find out which rat, and don't tell the chair."*
14. **Outcome, Learning the Drones, "Walk him to the line", when he lived** (the callous joke never lands; the cost beat
    follows): *"He walked back past the crew without looking at them. The crew chalks forty-one seconds on the lorry,
    under the others. Nobody makes the joke."* Then a separate system line with the Fame and Favor.
15. **Outcome after Requisition was delivered to Bastion** (no narrator joke; the faction's own word carries it): *"The
    quartermaster signs for four sacks of the Haven's flour and writes 'contribution' in the box marked 'source'. In
    {subject_town}, they're counting bowls twice."*
16. **Side job from a Rogue outpost commander, to a player the Haven has exiled** (the ask comes from standing): *"You're
    the one the Haven put out? Good. I need somebody they won't miss, and somebody who won't ask why."*
17. **Small side quest at a short safehouse's approach** (cause named, a seeded person with one detail, at a place a
    stranger would be, no new value): *"A woman on the road into {place} used to fix bicycles. Since the truck didn't
    come, she's trading her husband's watch for a tin of anything. 'It keeps good time. He'd want it to go to someone
    who's going somewhere.'"* Choices: "Give her a tin" / "Trade a tin for the watch" / "Send her to the Haven house"
    (that house is already short) / "Walk on". Once per house per shortage.
18. **The Red Hand fence, if offered the watch** (dark humour that also stops tins being turned into Favor): *"Watches
    don't sell. Time's the one thing nobody's short of."*
19. **Dialogue at a Bastion checkpoint** (one dry line, same result as the plain one, dropped first in the four-slot
    fallback): Sergeant: *"Name, town, business."* Player: *"{player_name}. Nowhere now. Passing through."* / *"Same as
    yesterday. You wrote it down."* / [Nurse] *"Medic. Your post asked for me."* To the dry line: *"I did. I'll write it
    again. It's the one thing round here that still works."*
20. **Race line on the Red Hand open channel before 90%** (a book that is never a scoreboard): *"The book on the cure is
    open. Evens on everyone. We like to lose money slowly."*
21. **The Independent pitch, from the Doctor wanderer** (best argument, then the admission, before any lock): *"Everyone
    wants to hand it out. Nobody wants to hand it over. Take it, give it to everyone, and let them be angry at you
    instead of each other. Mind: nobody will guard it, nobody will count it, and every one of them will remember who
    took it."*
22. **The lock prompt,** repeating the route's admission (plain, from the system): *"You're siding with Bastion for the
    rest of the cure. Their cure reaches those who comply, and the register is a census. This can't be undone this
    season."*
23. **After the lock: the Haven chair, face to face, to a player locked into another route** (out-group: kind, the one
    joke at the Haven's own expense): *"You've picked. Fine. You'll still be fed here. We've fed people who picked
    worse."*
24. **After the lock: the Red Hand boss, face to face, to an Independent player** (a joke about them, never on air): *"So
    you're taking it for everyone, for nothing. I've been robbed before, but never on behalf of the whole Zone."*
25. **A route that lost: Bastion's commanding officer, to a player locked with Bastion,** after the Haven took the cure
    (no blame, one plain line): *"We held the line we were given. Log it. Then eat something."*
26. **A crew name, said as leverage** (a toll man who knows exactly what you have): *"Skint! Toll's one tin. You've got
    exactly one, haven't you."*
27. **The word, at a Bastion gate** (a mixed cell reads as a box on a form): *"Flagged, useful. Go through. Don't make me
    open the file."*
28. **A disguise, while the guard turns to watch you** (the look is being made; another time the same pass may come with
    no line at all): *"Nice uniform. Whose was it?"*
29. **An earned rival clue at a gate** (matches the hostility state: Bastion trusts this player): *"Red armband at a
    Bastion gate. Bold. Go on, you're on the list. So is the armband."*
30. **Grief on the Haven's open channel after a death in a house** (plain for 20 minutes, the same for everyone):
    *"{place}. We buried one of ours this afternoon. The soup's on anyway. Come and eat."*
31. **Callback from a Haven house head** (a deed from this life), then the generic line for a leader who remembers a past
    life's deed: *"You're the one who brought the lad from the crossroads in. He does the water now. Badly."* / *"I know
    your face. I don't know from where. Round here, that's usually good."*
32. **Epilogue, a Bastion roster board at an outpost** (the Haven controlled the cure; a place the epilogue owns):
    *"Someone who isn't a soldier has chalked a new column on the roster board: 'Cured'. Nobody has rubbed it out, which
    on that board counts as a medal."*
33. **Epilogue, the lab door** (Halcyon published; no pronoun for the seeded director): *"A sheet is taped to the lab
    door: the cure's method, in the director's hand. Someone has corrected the spelling in red. Underneath, in pencil:
    'thank you'."*
34. **Epilogue, the council's pinned note on a safehouse board** (the Independent route controlled the cure, open):
    *"Cure: everyone. Price: nothing. Taken by people none of us voted for. Supper as usual."*
35. **Epilogue, the slow cure** (the lab finished on its own work), the toll sign's handwriting: *"CURE: ARRIVED. NOBODY
    FOUGHT FOR IT. TOLL: STILL ONE TIN."*
36. **The fixed after-cure hint** (Phase 1), Halcyon on Bastion when Bastion controlled the cure: *"Bastion has the cure
    in a locked crate. They're issuing it in alphabetical order. We are under H."*
37. **The Turned Player brief without its quip** (Phase 1): *"%1 has turned, still carrying everything they had. Whoever
    reaches them first decides what happens to it."*
38. **Epilogue, processing, the north road** (a goodbye, never a joke; the outside's one word): A resident hands her bag
    to the man behind her and walks the last stretch alone. The barrier arm doesn't lift, and then it does. *"Name?"* She
    says it. The queue shuffles up one place. Somebody behind says, to nobody, *"She'll want her bag."*
39. **A crewman asked about Rudi** (the lie, flatly; the shutter is the detail): *"Rudi was bitten."* He doesn't look at
    the shutter. Then he does.

## 9. Risks

- **Repetition still kills humour if the budget isn't met.** The fallback to PLAIN lines and the minimum pool size stop
  repeats, but the cost is fewer jokes. If the budget slips, the Zone gets drier rather than repetitive, which is the
  right failure. In the epilogue's week, a seat short of plain lines is the likeliest place to show it.
- **The Python simulator can drift from the Enforce picker.** Keep the picker rules small and mirrored, keep defaults in
  a strict one-call-per-line format, and use the runtime test with forced moods to confirm the in-game picks fall inside
  the simulator's numbers.
- **Tonal whiplash after deaths the server doesn't know about** (PvP). Grief covers known deaths only. Keep Grim and
  Bleak shares low, and never let a joke stand in for information.
- **Edginess creeps in as content grows.** The lint catches words, not tone. The three tests, "callous isn't mocking"
  and the signs in 5.3 must be a checklist in every text commit, and the baseline must only ever shrink.
- **Players will still optimise grey choices if payouts are lopsided.** Log which options are chosen in play-tests and
  the runtime test, and retune when one option takes most of the picks. Memory flags saturate on purpose and can't be
  the reward.
- **Lies that rotate per season need the "found out" and "it was true" text for every variant.** A missing variant is a
  gotcha. Phase 5 is done only when every variant has an approach open to everyone and both outcomes written.
- **Flag-conditioned lines grow combinatorially** (moods × routes × majors' flags × endings). Allow at most two flag
  conditions per line. Make variants fall back to the base pool, so a missing combination is plain, never wrong.
- **Contradicting the world:** a joke about Bastion's checkpoints after Bastion has pulled out, about food when the house
  is fed, about "going home" before processing, or about the kettle after the orders were read. Gate every
  state-flavoured line on facts and flags; the 3.10 table lists the known running jokes.
- **Player names in world text can offend or be used to grief.** Honour lines take only recorded names, fall back to "a
  stranger" on any filter match, and admins can strike one. Names are never used in jokes and never on air.
- **Ambient notices could pass for questline clues or crowd the infected's marks.** Keep the 1 km rule around active
  clues, the lint check against clue text, and no notices at a pack's range.
- **Real-world sensitivity:** a sealed quarantine echoes real pandemics. Keep the satire on official language in general,
  never on real governments, real measures or their politics. The Walk Out's barrier is the most exposed moment: it
  says one word and shows nothing.
- **Too much darkness numbs.** The tone is "bleak but human". In Bleak, silence lines and warmth (the Haven's soup, the
  coat, the "thank you") must outnumber the bitter jokes.
- **Long Table creep:** every extra hint turns horror into a gag. The ambient word list in the lint, the Phase 1 fix of
  the meal aside, and "silence always follows" must hold.
- **Long lines get cut** in the vanilla hint fallback and in action-menu labels. The per-kind limits help; test with
  styled notices off.
- **Wordplay doesn't translate.** Prefer jokes that come from the situation over puns, in case the mod is ever
  localised.
- **Several parts depend on designs not yet built:** the finale's lock and routes, the epilogue and its boards of record,
  dialogue, mission style, disguise, the reputation names and the majors' flags. Until they land, those lines stay
  planned. Phases 0 to 2, and Phase 4's label, outcome and eligibility fixes, don't need them.
