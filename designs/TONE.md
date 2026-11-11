# Grey choices and dark humour

**Status: designed; the standard, the lint, the season mood, the voice bank, route lines face to face, look
lines, dry lines and saying a set word built.** Approved by the author on 2026-11-11.

This piece decides how the existing voices speak, and sets the standard every choice is written to. It
adds only small, data-only helpers.

**Built so far:** the text standard (the tone, the voices, by channel, the grey test, phrasing, the humour line
and the template rules) is in [WRITING.md](../docs/WRITING.md), and the inventory and the lint are in
[WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#text-lint). Phase 2 built the season mood (with the broadcast on the
same ladder), temper, grief face to face, the voice bank with its first pools and the players' joke memory,
`tools/voice_sim.py`, and the rules for conversation lines and for saying the word and crew names
([WRITING.md](../docs/WRITING.md#the-season-mood)). Sections 2, 3.1, 3.6, 3.11, 4.1, 4.7 and 5 are kept
below only as pointers, so the numbers other designs use still work. Everything else here is designed, not built.

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
  ends at the cure ([FINALE.md](../docs/FINALE.md#the-lock)), so the option on lock scope is gone.
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
  at a place [docs/ERRANDS.md](../docs/ERRANDS.md#strangers) allows, never by a timer (4.6b).
- **The finale's lore is confirmed** (decision 2). Lines may use it, under its owners' rules: the captain's log
  is never quoted on air, and Command never answers.
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

[WRITING.md](../docs/WRITING.md) holds the rules in force. What's left lands as it's built:

| Doc | Gets |
|---|---|
| `docs/WRITING.md` | The season mood, temper, grief and joke memory (phase 3); the rules for dry lines (3.7), saying the word (3.8), disguise lines (3.9), running jokes (3.10), repeats (3.12), givers who lie (4.3) and small asks (4.6b), each with its system; the infected's words for each speaker (from INFECTED.md) |
| `docs/DESIGN.md`, Missions | "Grey choices": the payout spread, lies, failure as a branch, callbacks (4.2–4.5) and the shared-server rules (4.8), as the mission style builds them. The by-kind content becomes two columns of the mission-style table, not a second table |
| `docs/DESIGN.md`, Faction life | The voice bank, the season mood and temper, grief, joke memory |
| `docs/MISSIONS.md` | Each Result text corrected where options or outcomes change |
| `docs/WORKBENCH_SETUP.md` | Voice > Lines, Voice > Disabled, the debug settings, the voice log line |

## 2. The grey core: everyone is right about something

Built: [WRITING.md](../docs/WRITING.md#everyone-is-right-about-something).

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

Built: [WRITING.md](../docs/WRITING.md#by-channel).

### 3.2 By the season's mood

**Built:** `SeasonMood()` with its ladder, the gate's step lighter, the holding rules, temper by morale, the joke
share by mood face to face, and the broadcast on the same table with its two new lines
([WRITING.md](../docs/WRITING.md#the-season-mood)).

Still to build:
- **Channel and mood combine by the lower:** a radio line's joke share is the lower of the channel cap (3.1) and
  the speaker faction's mood share after temper (Bastion's open channel in Grim: 1 in 5), when the radio's pools
  move into the bank.
- **The Red Hand's exception:** low temper changes its register to taunts, not its share; its open channel never
  drops below 1 in 5, except in grief; its secure channel goes quiet in Bleak like everyone's.

### 3.3 Grief and other local overrides

**Built:** grief face to face for leaders' and wanderers' deaths (1.5 km, 10 minutes) and a death in a safehouse
(30 minutes), read by the voice bank ([WRITING.md](../docs/WRITING.md#the-season-mood)).

Still to build:
- **Grief's other sources:** named people, a Haven Champion's Remembered, a truck crew's funeral, a Turned Player.
- **Radio grief:** when a faction loses a leader or a truck crew, or the Haven loses someone in a house, that
  faction's open channel goes plain for 20 minutes, for everyone (the guard keeps the time; the radio's pools
  read it once they're in the bank). The Red Hand's own send-off lines are exempt.
- **A short house** jokes about the soup, the potatoes and the chores *more*, never about who eats, and not at
  all in the 30 minutes after a death there. The narrator never jokes about a shortage.
- **Feud or truce** on the channels (face to face is built: [WRITING.md](../docs/WRITING.md#the-voice-bank)).
- **The infected.** [INFECTED.md](INFECTED.md#people) owns the marks and each faction's words for them. Only the
  Red Hand jokes about the infected, in its own words ("the furniture", a crew nickname for a pack, "the boss"
  for its leader); the Haven names a pack's leader fondly, by who it was ("the big one" when nobody knew);
  Halcyon never says "him" or "her" about them; near a pack's range at dusk (or in fog) the Haven's people go
  quiet.
- **A player they won't talk to:** wanderers who were robbed don't joke with the robber (a refusing leader's seat
  is built: [WRITING.md](../docs/WRITING.md#the-voice-bank)).
- **Stronghold pages** (majors): a stronghold leader never jokes about their own page; once a page is in the
  open (`majors.pages_open`) rivals may gossip about it plainly, never as a joke in the first real day.

### 3.4 The race and the point of no return (90%)

**Scope.** [FINALE.md](../docs/FINALE.md#the-lock) owns the lock. The voice reads `RouteOf(identity)`: none, the Haven,
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

**After the lock: face to face only.** Built: the route lines for the in-group, out-group, Independent, unsided
and a lost route, with coldness fading after two real days ([WRITING.md](../docs/WRITING.md#the-race-and-the-lock)).
The Independent's commit on air is said without a name: *"This is nobody. The cure goes to everyone."* Still to
build: on a new life, *"Somebody said you're with us."*; at most one route line a visit after the first.

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

Built: [WRITING.md](../docs/WRITING.md#the-voices).

### 3.7 Player lines in dialogue trees

Built, with dry lines: [WRITING.md](../docs/WRITING.md#conversations).

### 3.8 The faction's word for you, and crew names

Built, with a set word said as each faction says rank and the crew name at Champion:
[WRITING.md](../docs/WRITING.md#conversations), [REPUTATION.md](../docs/REPUTATION.md#what-people-call-you).

### 3.9 Faction kit and disguises

**Built:** the voice following the look's result in conversations ([WRITING.md](../docs/WRITING.md#faction-kit-and-disguises)).

Still to build: **earned clues and bought kit** (the amber ribbon, the red armband, faction armour bought at
Trusted or Champion) say whose you are. A guard who sees a rival's clue makes one plain remark, at most once a
visit, saying only what the hostility state already does: never a consequence that doesn't happen, never safety
where there's danger. The red armband means the crew has something on you. And the look lines at barriers outside
a conversation (disguise's): a failed look at a hostile post can go straight to the guns, and a noticing line plays
only while the guard turns to watch you.

### 3.10 Running jokes and the majors' states

The people's running jokes on the majors' flags are built in the voice bank
([WRITING.md](../docs/WRITING.md#the-voice-bank)). Still to build, on the channels and signs:

| Running joke | Runs while | Then |
|---|---|---|
| The Red Hand's *"Still waiting for relief? We'll keep the kettle on."* on the open channel (built as a guidance line, `Guidance/RZ_GuidanceText.c`) | `orders.stance` unset or 1: nightly at 1 | Retired at stances 2–4. In the epilogue it also stops if the CO closes the net: *"Nobody left to laugh at."* |
| Bastion's *"No reply."* on the call | Always: it's a fact | The sergeant's dry tag goes plain from stance 2 |
| The toll sign's handwriting | The book stands | After `book.burnt` or `book.debts_dead`, the handwriting changes, and the book jokes stop for a real day |
| Rivals' gossip about a page | Plain, from the second real day after it's in the open | Never a joke from the page's owner |

### 3.11 Information comes first

Built: [WRITING.md](../docs/WRITING.md#information-comes-first), and the lint's `info-token` check.

### 3.12 Repeats

**Built:** face to face, the last 24 jokes each player heard with the real time, on the profile across lives; a
joke heard in the last 24 real hours isn't said again, and a pool out of jokes falls back to plain lines; the
minimum pool size checked by `tools/voice_sim.py` ([WRITING.md](../docs/WRITING.md#the-voice-bank)).

Still to build:
- **Shared channels:** the last 16 jokes each channel aired, with the time, saved with the season; no joke airs
  twice within 6 real hours.
- **The epilogue's week:** each stronghold and seat leader gets at least **8 plain lines per ending**, and the
  closing work's people get their own lines, owned by EPILOGUE.md.

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

Built: [WRITING.md](../docs/WRITING.md#grey-choices). The sections below are the mechanics behind tests 3, 6 and 7.

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
- **Labels show costs and requirements, and hide rewards and consequences** (built:
  [WRITING.md](../docs/WRITING.md#grey-choices)).
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

Built: [WRITING.md](../docs/WRITING.md#phrasing-choices-and-outcomes). Built labels over the limit are in the lint baseline,
and get shorter tags in Phase 4.

### 4.8 The shared server

- **Who can choose.** Only players the job was offered to and who took part: 60 s within 150 m, or a step done (the
  plan's one participation rule).
  - Deciding uses missions' 5-second hold, with one objection per player per job and Stand with.
  - Everyone else who took part gets a line saying someone else made the call: *"Someone got to him first. His
    KIA's rubbed out."*
- **The clever way closes the honest one.** Once anyone in the run tries a trick or a betrayal, the honest branch is
  closed for that run. If they're found out, the run ends in the found-out outcome for everyone in it.
- **Standing is personal; world consequences are shared.**
- **Names in shared text** are built ([WRITING.md](../docs/WRITING.md#names-in-shared-text)). The Turned Player brief loses
  its quip in Phase 1. Epilogue honour lines follow 3.5.
- **Late joiners read it in the world:**
  - headlines (the newest 10) and road news;
  - **boards of record**: a few stable boards whose text is worked out from season flags each time, the same for
    everyone, with nothing new saved. The Haven council's pinned notes, Bastion's roster board, the toll sign's
    tally, the lab door's sheet.

  The epilogue owns those props (one set for the season and the epilogue). This piece asks for their in-season mode
  and writes it.

## 5. The line: dark humour, not edginess

Built: [WRITING.md](../docs/WRITING.md#dark-humour-not-edginess) (5.1–5.5) and [its template rules](../docs/WRITING.md#template-rules)
(5.6). What joins them later:
- **5.2:** a joke about the walk out, once the Walk Out is built ([EPILOGUE.md](EPILOGUE.md)).
- **5.4:** the infected's row (each speaker's word, always with the place: the Haven's name or "the big one",
  Bastion's contact and "the heavy", the Red Hand's crew nickname and "the boss", Halcyon's pack number and "the
  index"), with the tribes ([INFECTED.md](INFECTED.md#people)).
- **5.6:** the planned tokens, as each is built: reputation's `FillFor` tokens, the infected's `{tribe:N}`,
  `{alpha:N}` and `{young:N}` (each in the speaker's own word), and the voice bank's context tokens (7), with
  `{count}` only once the majors' `orders.count` exists (until then Bastion's call says *"Strength as last
  reported"*). There's no home town: players don't have one.

## 6. The writing pass

| Phase | What | Files | Done when |
|---|---|---|---|
| **0. The guide** | Built: [WRITING.md](../docs/WRITING.md), with every link moved. DESIGN.md's "Grey choices" waits for the mission style's mechanics (section 1) | Docs, `CLAUDE.md`, `README.md`, code comments | Done |
| **1. Built contradictions** (text only) | The Red Hand's meal aside (5.5). `HintAfterCure`'s Halcyon line on Bastion: only when Bastion didn't control the cure, plus a Bastion-controlled line, and the Haven's "The scientists did it" only when Halcyon did. The Turned Player brief's quip. Halcyon's "Habit." | `Factions/RZ_FactionVoice.c`, `Factions/RZ_LeaderVoice.c`, `Missions/Events/RZ_WorldEvents.c`, and any doc that quotes them | `tools/compile_check.sh` exits 0; the docs match |
| **2. Inventory and lint** | Built: `tools/strings_inventory.py`, `tools/voice_lint.py` and its baseline ([WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#text-lint)). Left for later phases: the lint learns reputation's `FillFor` tokens and the voice bank's context tokens (7) when they're built | `tools/` | Done |
| **3. The voice bank** | Bank, `SeasonMood()`, temper, grief, joke memory, picker. Move the top-ranked pools (3.13) and tag their registers. The broadcast reads the mood and gains its two lines. Loudspeakers, the wipe line and guidance tips get the lint and a review, but stay out of the bank (few lines, or their own attribute class). `tools/voice_sim.py` | `Factions/RZ_FactionVoice.c`, `Factions/RZ_LeaderVoice.c`, `Factions/RZ_FactionLife.c` (chatter, secure channels), `Guidance/RZ_GuidanceText.c` (newcomer open channels), `World/RZ_Wanderers.c` (road news), `World/RZ_Safehouses.c`, `Director/RZ_Broadcast.c`; review only: `World/RZ_Perimeter.c`, `Core/RZ_Seasons.c`, `Guidance/RZ_GuidanceTips.c` | The simulator shows each mood's share inside 3.2 and no repeats inside the windows at the fastest cadence; the compile check passes. WRITING.md and DESIGN.md gain the mood, temper, grief and joke memory |
| **4. Choice missions** | The grey test on every choice. Each cost line gets a world change behind it: a flag plus at least one reader. The three-beat outcome and system line. Who can choose; the clever way closes the honest one. Labels shortened. Honour Among Thieves' "let him keep it" becomes a branch. Then: Local Law, Shut the Door, The Ledger, Requisition, Protection Racket, Struck Off, No Reply, the three hard edges, Patients Not Samples, The Notebook, Private Collection, The Silent Village, The Witness, Rein In | `Missions/`, `Puzzles/` | Every choice passes 4.1; MISSIONS.md's Result text matches |
| **5. Givers who lie** | About 12 briefs, each with 2–3 omissions plus an honest version, picked once a season: Learning the Drones, A Quiet Arrangement, Requisition, Names for Bastion, Off-Book Requisition, Rifles for Sale, The Real Orders, Unsanctioned Trial, Private Collection, Sell the Formula, The Pit's Debtor, Debt Collection. For each: the truth-finding approach open to everyone, the checks, the outcome lines, and the morale cap | Missions | No gotchas (4.3); no tag gives a lie away |
| **6. State variants** | Majors' running jokes on the channels (3.10). Race lines and pitches. Epilogue lines with [EPILOGUE.md](EPILOGUE.md), for every ending including the slow cure, handed over and no cure, and the plain lines for a week. Every after-cure line made ending-aware. (Built: route lines face to face, lost-route and unsided lines, saying the word and crew names, disguise look lines) | Voice bank | Every variant meets the minimum pool size (3.12) |
| **7. Check** | `voice_sim.py` over three scripted seasons: season speed 1, season speed 2, and a gate that runs to the slow finish. The runtime test with the voice log on and Voice > Force mood cycling the moods, to catch script exceptions and confirm the in-game picker matches the simulator. The author reads one player's radio log for an hour | `tools/`, `tools/runtime_test/` | No exceptions; the shares and repeats in the simulator are inside the targets |

**Order.**
- Phases 0 and 2 are built.
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

- **Built in phase 2:** the voice bank (`RZ_VoiceLine`, `RZ_VoiceDefaults.c`, **Voice > Lines** and **Disabled**) for
  face-to-face pools, `SeasonMood()` and temper with the broadcast reading it, `RZ_ToneGuard` face to face, the
  players' joke memory, `tools/voice_sim.py`, the voice log and **Voice > Force mood**; the route relation through
  the bank's conditions (`route:in`, `route:out`...). Still needed below: the bank's ending field and the radio's
  pools, the shared channels' memory, grief's other feeds.
- **Voice bank (the design in full):** an `RZ_VoiceLine` attribute class made with `RZ_Util.NewWithDefaults`, with no field initialisers.
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
