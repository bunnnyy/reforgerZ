# Dialogue

How talking to people works: the talk screen and its engine, the conversation trees on top of it
(**Dialogue > Trees**, off by default), what people remember, the checks shown before you choose,
lies you're told, promises and callbacks, claims in someone else's kit, barks, strangers and the open
channel, the `.rzd` text format, its lint and simulator, and the conversations that ship. How the
screen looks and is driven with a pad is in [UI.md](UI.md#the-talk-screen); how lines are written is
in [WRITING.md](WRITING.md#conversations). What's still to build (the finale's commit, the epilogue's
topics, Make it right, the HUD panel and the other deliveries) is designed in
[designs/DIALOGUE.md](../designs/DIALOGUE.md).

Code: `Dialogue/RZ_Dialogue.c` (sessions), `Dialogue/RZ_TalkSpeakers.c` (who someone is),
`Dialogue/RZ_TalkStrangers.c` (strangers), `Dialogue/RZ_DialogueRadio.c` (the open channel and its
action), `Dialogue/RZ_DialogueTrees.c` (hubs, nodes, checks, effects), `Dialogue/RZ_DialogueDB.c` (the
compiled conversations), `Dialogue/RZ_DialogueCond.c` (conditions and tokens),
`Dialogue/RZ_DialogueMemory.c` (memory and `dialogue.json`), `Dialogue/RZ_DialogueScenes.c` (several
players), `Dialogue/RZ_DialogueVotes.c` (the majors' sittings), `Dialogue/RZ_DialogueBarks.c` (barks),
`Dialogue/RZ_DialogueWire.c` (Bastion's word at the wire),
`Dialogue/Pack/` (generated). Server only, except the screen.

## Contents

- [The talk screen](#the-talk-screen)
- [Trees: hubs, topics and nodes](#trees-hubs-topics-and-nodes)
- [Who talks](#who-talks)
- [Checks and tags](#checks-and-tags)
- [What a choice does](#what-a-choice-does)
- [Memory](#memory)
- [Several players](#several-players)
- [Lies you're told](#lies-youre-told)
- [Promises and callbacks](#promises-and-callbacks)
- [Claims in someone else's kit](#claims-in-someone-elses-kit)
- [Barks](#barks)
- [Strangers](#strangers)
- [The open channel](#the-open-channel)
- [Conditions](#conditions)
- [The text format (.rzd)](#the-text-format-rzd)
- [Tools](#tools)
- [The conversations](#the-conversations)
- [Saving and performance](#saving-and-performance)
- [Simplified, and not built yet](#simplified-and-not-built-yet)

## The talk screen

Look at a person and the action menu shows one line, **Talk to ...**. It opens the talk screen:
what they say, and your replies as a list. You stay in the conversation until you leave, walk
off (6 m), wait 120 s with no reply, or danger comes (the [screen guard](UI.md#how-the-scripts-fit):
then they say "Not now!" and it closes). You can't move while talking; the world keeps running.

**Who:** stronghold and seat leaders (and the quartermaster's "Talk to whoever's in charge"),
safehouse occupants, named people at holdings, the wanderers, the Long Table's people, strangers
(errands), the people missions and questlines spawn, and (with Trees on) a road post's speaker at the
barrier and the open channel on a radio. Any living character with the puzzle components. Bodies,
objects, spots and doors keep their actions.

**One engine, built on what's there.** Every one of these people already has a Talk handler and
up to 8 labelled slots. The engine doesn't rewrite them; it wraps them:

- **Greeting:** their Talk handler, run while the engine *captures* the player's notices and
  spoken lines whose title is the speaker's (a leader's is known; otherwise the first title the
  greeting uses). Those lines go into the screen. Anything else (a contract offer, a standing
  change) still comes as a card. A line from the speaker that arrives later (a treatment
  finishing) also goes into the screen while it's open.
- **Replies:** the person's visible choice and use slots (their labels are on the server), plus
  **I'll be going.** A leader's are Ask for work, Heard anything?, What do you make of me?
- **A pick** runs that slot's handler, capturing again. If it said nothing new (the answer was a
  card), what they said stays.
- **Shown before you choose, never rolled.** A reply you can't take stays in the list, dim, with
  what it needs: a price in Favor in its label ("Buy supplies", needs 20 Haven Favor, you have 12),
  "Give them your antidote" (you have none), "Share some food" (you have nothing to eat). There are
  no dice.

So every built rule (Rob's Infamy, the Trader's price, a leader's refusal at attitude −2, an
exile's shut door) works exactly as built.

**Safety.** At most 32 conversations (and readers) on the server, and 6 with one person at once (16
while someone there is in a public node; at the cap, the one idle longest, over 30 s, gives way; else
"One at a time."); a pick is checked for the
current serial, a reply id that was sent and enabled, a living player in range and not in danger,
and at most one every 0.3 s; Talk at most every 2 s. One node at a time goes to that player only
(the owner-only `PAYLOAD_TALK`). Sessions aren't saved. With Trees on, a conversation in a node
that danger or distance ended picks up where it was if you talk to the same person within 60 s
("Where were we?"), without running the node's effects again.

**The action list.** With the talk screen on (**Dialogue > Talk screen**, and **UI > Talk reply
layout** set), a person's choice and use actions hide, and so do the quartermaster's leader
actions for work, news and standing. Without it, or on a client where the screen can't open,
nothing hides and Talk and the slots work as actions, with notices.

## Trees: hubs, topics and nodes

✅ **Dialogue > Trees** (off by default, the author's decision 13, until the per-player label test
and a play-test pass). It needs the talk screen. **With Trees off, the talk screen is exactly the
one above**, and nothing on this page below this line runs, except the memory file being read.

With Trees on, Talk opens a **hub**:

- **The greeting:** the built one, as above. A **greet** conversation whose `when` holds puts its
  lines first ("There's a candle in the window, and people are talking more quietly than usual.").
  A leader's greeting line for someone they've met is their **opening** in reputation's voice
  ([REPUTATION.md](REPUTATION.md#what-people-call-you)).
- **Verbs:** the person's built replies, unchanged (Ask for work, Share some food, Rob...). A
  leader's **What do you make of me?** is said in their faction's voice.
- **Topics:** every conversation for that kind of speaker whose `when` holds, in priority order
  (quest, ask, world, reaction, flavour; flavour rotates on the speaker and the day). They fill the
  list after the verbs; when they don't fit, **Something else.** shows the next page. A topic
  you've heard is tagged "heard". `once` topics go for this life at that speaker, `daily` ones for
  a real day.
- **I'll be going.**, always.

A topic opens its first **node**: the speaker says one to three lines, and you pick a **choice**
(at most 6, plus I'll be going.). A choice goes to another node, back to the hub (`hub`), back a
node (`back`), or ends the conversation (`end`). Nodes can jump on conditions. Picking a choice
reads its condition and the conversation's `when` again in the live world; if either no longer
holds, they "didn't catch that". A choice with a price is refused within 1.5 s of a new node, so a
list that hasn't refreshed can't take your Favor.

**Leaders never give errands** in a tree; a leader's line names who to see.

## Who talks

Each speaker is described by its adapter (`RZ_TalkSpeaker.Describe`): its kinds (what a
conversation's `for` names), a memory key, its faction, and the leader, wanderer, house or post
behind it.

| Speaker | `for` | Memory key | Built verbs |
|---|---|---|---|
| Stronghold leaders | `stronghold:<faction>`, `leader:<faction>`, `leader` | `leader:<seat>:<generation>` (someone new in the chair starts fresh) | Ask for work, Heard anything?, What do you make of me? |
| Seat leaders | `seat:haven/council_head`, `seat:bastion/outpost_lead`, `seat:redhand/crew_boss`, `seat:halcyon/field_lead`, `leader:<faction>`, `leader` | as above | as above |
| Wanderers | `wanderer:<role>` (trader, doctor, scavenger, deserter, preacher, courier, kid, mechanic), `wanderer` | `wanderer:<id>:<generation>` | the role's special, Share some food, the antidote while bitten (else Need a hand?, [errands](ERRANDS.md#the-wanderers-asks); the Mechanic: Ask about cars), Rob |
| Safehouse occupants | `occupant` | `house:<index>` | Ask how things are |
| A safehouse's named people (the first two occupants, [ERRANDS.md](ERRANDS.md#givers)) | none yet (no trees) | `house:<index>:<i>` (errands' regard) | Ask how things are, Anything need doing? |
| Named people at holdings | `named:<faction>/<role>` (`named:redhand/toll_man`), `named:<faction>`, `named` | `named:<holding>:<index>` | Ask how things are, Anything need doing? (errands) |
| A road post's speaker | `post:bastion`, `post:redhand`, `post` | `post:<faction>:<town>` | none |
| The Long Table | `table:host`, `table:mother`, `table:son`, `table:boy`, `table:road` (the host out on the road), `table` | `table:<role>` (no regard: its memory stays its three words) | eat, trade a jar, declare a bite; on the road, accept or decline |
| Strangers ([below](#strangers)) | `stranger:<routine>` (`stranger:tin`, `stranger:digger`...), `stranger` | `stranger:<visit>` | their ask's ways, as built ([ERRANDS.md](ERRANDS.md#strangers)) |
| The open channel ([below](#the-open-channel)) | `radio:open`, `radio` | `radio:open` | none |

Factions are written `haven`, `bastion`, `redhand`, `halcyon`, and `none` for factionless checks.
Strangers and the open channel are factionless. Mission and questline people have no trees yet
(their built choices stay).

**The post speaker.** With Trees on, one of the crew stands at the barrier of a Bastion checkpoint
or a Red Hand roadblock while a player is within 100 m: the faction's territory occupant prefab
(else the puzzle civilian), entered in the one body budget as a speaker
([INFECTED.md](INFECTED.md#the-one-body-budget)), and gone when nobody is near. Talk gives the post's
rule for you in its voice; the search and the toll still happen as built at the barrier. The crew
stays an AI group.

**Built refusals come first.** A leader or wanderer at attitude −2 or below, an exile at a Haven
speaker, a Hunted player with a faction's people, someone the Long Table has crossed: the built
greeting refuses, and no topics show (`RZ_DialogueTrees.Refuses`).

## Checks and tags

Every check goes through `RZ_Checks` ([DESIGN.md](DESIGN.md#checks-and-ways)), and its tag is
written by the engine from the check, never by the writer. **Tags show what a choice needs and
costs, never what comes of it.**

| Check | Tag | Score |
|---|---|---|
| Background (`if bg:...`) | [Doctor] | Shown only to that background. Not a roll: the line is simply theirs |
| Standing (`if tier:...`) | [Trusted - Haven] | Shown only when true |
| `persuade <faction> <difficulty>` | [Persuade - Haven - 1 of 2] | **Trust**: your access rank with the faction (mixed tiers one lower), +1 for a listed background, +1 at most from dialogue (regard 2 or more, or the built helped or saved memory of this life), −1 for a found-out lie or crossed lines. Hunted or worse: dim, "they won't hear it from you". Factionless speakers read your Haven rank, at most 2; the Long Table's rank counts 0 |
| `threaten <faction> <difficulty>` | [Fear 1 of 2 - +3 Infamy] (": Hunted" when it would make you Hunted) | **Fear**, with the threat's rules: +3 Infamy and regard *threatened* whatever happens, once per person by anyone for the season ("Already scared. Of someone else."), three a life per faction, a refused threat barred a real day. Never on stronghold leaders, the Kid, the Long Table, or in safehouses (the lint refuses it, and the engine refuses it on a stronghold leader) |
| `lie <faction> <difficulty>` | [Lie - 1 of 2], or [Lie - they'd see through it] | A **tell** catches it first, whatever the score: the bite showing, another faction's colours on you, or something else you already told this speaker. Without one, Trust against the difficulty (+1 under a vouch). Believed: a false claim in the [lie store](DESIGN.md#lies-and-witnesses), found out later by its rumour rounds (the speaker's regard *lied*, +2 Infamy). Caught now: regard *lied*, +2 Infamy and the liar count (−1 at Trust this life), and no more lies at that speaker for a real day ("they caught you lying today"). The reply opens with "They believe you. For now." or "They don't believe a word of it." A choice's `truth <conditions>` makes it true while they hold: then it's no lie, with no tag and no check. A caught or found-out lie is said at that speaker's next greeting (`nmem:lied`) |
| `look <faction> <difficulty>` | [Full kit +2] [Watched -1] [Look - Bastion - 2 of 2], or [Look - Bastion - they'd see through it] | **The look** ([REPUTATION.md](REPUTATION.md#borrowed-kit)): a claim to be one of them, its score shown before you pick. Dim with the reason when you can't claim at all (Hunted, not in their kit, your own colours, too many tells, caught three times, blown). A leader of theirs or anyone inside their wire sees through it: caught. Passed: you pass (at a road post while it stands, elsewhere 10 minutes); caught: the built catch ([Claims](#claims-in-someone-elses-kit)) |
| `callout <lie key>` (a choice option) | [Evidence] | Shown only to a player who was told that lie and knows the truth ([Lies you're told](#lies-youre-told)) |
| `cost favor:<faction> N` | [20 Haven Favor] | Dim with what it needs when you can't pay. Paid when picked |
| `cost item:food*N` | [2 food] | Dim with "you have 1 food" when you can't. Taken when picked |

A trait on the speaker can move a check one step (`easier:merciful`, `harder:paranoid`).
Difficulty runs 1 to 4.

**Results.** At or over the difficulty: **Pass**. One under: **Short**, a halfway answer. Two or
more under: **Refused**. A choice names its targets in that order (`-> pass | short | refused`; a
missing one falls back to the one before). The reply opens with the result: [Persuaded], [Short],
[Not persuaded]; for a threat, [They back down], [Short], [They don't scare]. Since the scores are
shown, picking a check you're short on is a choice.

**Repeats.** A passed persuade is remembered for this character: the choice goes straight to its
pass next time. A failed one is dim ("you've tried that") until your score for it is higher than
when it failed; that record is per player, so dying doesn't reset it. Threats are checked every time.

**Failure is a branch.** The lint refuses a check whose fail target is `end`, or a fail node that
offers only leaving.

## What a choice does

Effects run on node entry (`do` in a node) or with a choice (`do` on a pass or a choice without a
check, `short` on a Short, `else` on a Refused), each through the system that owns it, with that
system's caps.

| Effect | Example | Does |
|---|---|---|
| `mark` | `mark:list.read` | A season flag that only goes from 0 to 1 |
| `cset` | `cset:asked_kid=1` | A flag on this character |
| `nset` | `nset:asked_rudi` | This speaker's note about this character |
| `regard` | `regard:thanks` | Regard ([Memory](#memory)). *thanks* only on a choice with a cost or a deed |
| `favor` | `favor:haven+2` | Favor, at most 10 a conversation, and all talk shares **Dialogue > Talk Favor a day** (20 a faction a player). Past it: "That's all I've got today." |
| `take` | `take:food*2` | Takes food by the built keywords |
| `claim` | `claim:alone` | Something you told this speaker (kept 6 a speaker, this life) |
| `learn` | `learn:haven.list` | Something you now know (`knows:` keys) |
| `rumour` | `rumour:"..."@6` | Gossip on the roads (the world's rumours) |
| `call` | `call:slot:2`, `call:hand_in:2`, `call:toll`, `call:post_wave`, `call:standing` | Built behaviour, unchanged: the person's own reply N; the safehouse food hand-in for N items (paid as built); the Red Hand toll at this roadblock (Known or better, as built); this post lets you through once while it stands; what they make of you, in their voice |
| `decide` | `decide:all_in` | In a conversation with a `scene`: the pick waits 5 s for objections ([Several players](#several-players)), then writes the scene's record (who, which), once for the world |
| `sway` | `sway:council.workers` | In a conversation with a `scene`: one contribution per player to a count its owner reads (`tally.<name>`) |
| `offer` | `offer:leader_job` | A leader's personal job (the built Ask for work) |
| `promise` | `promise:house.tins@here:2h` | A promise to this speaker, in the words of the choice that made it ([Promises](#promises-and-callbacks)) |
| `keep` | `keep:house.tins@here` | Keeps that promise, if it's open |
| `later` | `later:6h:scav.came_back` | A callback: `due:scav.came_back` reads true once you've played 6 more hours |
| `wire` | `wire:2` | Says the second of today's three words at the wire to Bastion's people: the right one counts at your next look today; a wrong one is caught at once |

There is **no Fame, no Infamy and no item** from talk, ever: the lint refuses `fame`, `infamy` and
`house` effects, and Infamy comes only from a tagged threat, a caught lie or a caught claim. Effects
that change the world for everyone go through a scene's `decide` or `sway`; mission outcomes from talk
wait for missions to take conversations.

**`@here`** in a key binds it to this speaker (`house.tins@here` is one key per house), in effects and
in conditions alike.

## Memory

`RZ_DialogueMemory`, saved in `$profile:ReforgerZ/dialogue.json`.

| What | Kept for | Scope | How long |
|---|---|---|---|
| **Regard** | each speaker key | player (identity) | −4 to +4; one step toward 0 each real day |
| Failed persuades (the score) | each speaker, conversation and choice | player | until your score is higher |
| Knowledge (`learn:`, `knows:`, `~truth`) | each key | player | the season (96 at most; the oldest goes) |
| Lies you were told (`heard_lie:`) | each key | player | the season (64 at most) |
| Promises (`promise:`, `kept:`, `broke:`) | each key | player | open: 3 at most; kept or broken: 3 real days |
| Callbacks (`later:`, `due:`) | each key | player | 10 at most (the oldest goes), each 3 real days |
| Talk Favor today | each faction | player | the real day |
| Face (they know you) | each faction | character | the life. Set when you talk to their people |
| Claims | each speaker | character | the life, 6 a speaker |
| Seen nodes, passed persuades, once and daily topics, character flags, speaker notes | per key | character | the life |
| Local deeds ("You're the one who...") | each seat | character | the life ([REPUTATION.md](REPUTATION.md#what-people-call-you)) |

A new life is a new face: everything kept per character is cleared when the profile's life count
changes.

**Regard words:** *thanks* +1 (only with a real cost or deed), *kept* +1, *broke* −1,
*threatened* −1, *slander* −1, *vouched_liar* −1, *lied* −3 (a Merciful speaker −2, a Paranoid one
−4). *kept* and *broke* come with promises (and errands' named people); the rest with a caught or
found-out lie, a false call-out or point-out, a vouch for a liar.

**What regard does.** With Trees on, a leader's and a wanderer's attitude adds regard ÷ 2 (rounded
toward zero), so talk moves it by at most ±2 and it fades. Regard 2 or more is persuade's +1.
Prices still read only the built memory, so talk can't farm discounts. The Long Table keeps no
regard.

## Several players

`RZ_DialogueScenes`, with **Dialogue > Trees**. Each player has their own session, checks and memory;
two players at one speaker can see different choices.

- **Overheard.** A `public` node's lines, and the label someone picked there (tags stripped), go to
  other players within **8 m**, owner-only, at most once every 10 s per listener.
- **Deciding together.** A conversation may name a `scene <id> speaker|season [who <conditions>]`. A choice with
  `decide:<value>` doesn't land at once: a **5 s hold** is told to the room (30 m). Anyone else
  talking to that speaker sees **Object** (once per scene) and **Stand with**. Unobjected, the pick
  lands for the decider and writes the scene's record (`scene.<key>`, a world record), once; later
  visitors meet the result through `jump decided -> <node>`. One hold per speaker at a time. With
  `who`, only those the conditions hold for may decide; everyone else sees the choice dimmed, *"only
  someone {name} trusts can answer for it"* (a sway is still theirs to add).
- **Waiting your turn.** A conversation left without a reply ends after 120 s (**Dialogue > Idle seconds**), or after
  45 s while another player stands within 8 m of the same person.
- **Tallies.** `sway:<name>` adds one contribution per player to `tally.<name>`. A major's vote
  reads its own: `sway:every_name.first.pot` (the major's key, the vote, the option's word: `pot`,
  `workers`, `cards`, `tins`) swings one voter to that option per contribution when the sitting is
  tallied ([DESIGN.md](DESIGN.md#the-majors)); the owning system
  counts at its own time.
- **Vouch.** Trusted or better with the speaker's faction, within 8 m of someone talking to them:
  "[Vouch] '{name}'s with me.'" For 5 minutes, while you stay within 8 m, their persuade and lie
  checks there get +1 (inside the dialogue +1). A lie told under the vouch, found out, costs you +5
  Infamy with that faction and regard *vouched_liar*. Once per pair per hour.
- **Call it out.** For 20 s after someone near you picks a choice that tells the speaker something (a
  claim or a lie): "[Call it out] '{name}'s lying.'" A lie: caught at once. True: regard *slander* for
  you. Once per pair per hour; the caller gets nothing either way.
- **Point them out.** Known or better with the speaker's faction, near someone in that faction's kit:
  "[Point them out] 'That one's not yours.'" It's −1 at their next look ([REPUTATION.md](REPUTATION.md#borrowed-kit)),
  two at most. Their own colours: regard *slander* for you. Once per pair per hour.

- **The majors' sittings.** While a major's vote is seated and not yet tallied
  (`sitting:every_name.first`), a conversation can sway it through the vote's own rules
  (`RZ_QuestVote`): `sway:every_name.first.pot` swings the speaker's own vote if they have one, else the
  first voter who doesn't lean that way, only for an option on the table (`tabled:every_name.first.pot`),
  and once per player per sitting (`swayed:every_name.first`). No scene is needed. The talk behind it
  is a persuade, so a failed one is a branch, and the sway lands only on a pass.

*Simplified:* the social verbs are on your own hub with that speaker (open Talk while they talk), and
"eligible to object" is anyone talking to the speaker. Who may decide a mission's scene by default,
and the decider's group sharing the pay, wait for missions to take conversations.

## Lies you're told

Givers and passers-by lie too. A `say` line ending in `~lie:<key>` is a lie: once the player has
heard it, `heard_lie:<key>` is true for them. A line ending in `~truth:<key>` is the truth: hearing it
records `knows:<key>`, the same as `learn:`. The truth comes from someone else (the Doctor wanderer
knows what the Scavenger's chemist's is really like), from a background line ([Hunter] "That door
was forced."), or from a check that gets it out of the liar.

Once a player has both, a choice with `callout <key>` shows, tagged [Evidence]
(*"Picked clean on Day 3. I asked."*). It opens the better answer, and that's the payoff: a caught
lie pays nothing of its own and sets no memory flag. Believing a lie costs nothing either; the
truth is a better door, not a trap. The lint refuses a `callout` for a lie no line tells.

*Simplified:* the truth comes only from lines; the design's world predicates (walking into the town),
clues and the notebook's "What you know" page aren't built. Lies told in public nodes aren't marked
for the people who overhear them.

## Promises and callbacks

**Promises.** `promise:<key>[:<hours>h]` on a choice is a promise to this speaker in the words of that
choice ("I'll bring two tins. Two hours."):
- **Clock:** the promiser's own play (2 hours by default), and 3 real days at most. Logging off
  doesn't break one; dying does.
- **At most 3 open** per player ("You've three promises open already. Keep one first.").
- **Kept** by `keep:<key>` on whatever does it (the house's own hand-in keeps `house.tins@here`), or,
  for a key ending in `.visit`, by talking to the same speaker again. Regard *kept* (+1) and a notice.
- **Broken** when the play runs out, three real days pass or you die: regard *broke* (−1) and a notice.
- **No double pay.** A promise pays nothing of its own: the hand-in pays as built, a visit pays
  nothing.
- **Where you see them:** the speaker you promised lists what's open in their greeting, with the play
  left ("(You promised "I'll bring two tins. Two hours." 1 h 20 min of play left.)").
- The speaker's next greeting can say how it went (`kept:<key>`, `broke:<key>`, written as `once`
  greets): *"You said you'd bring tins. I told the children."*

**Callbacks.** `later:<hours>h:<key>` makes `due:<key>` true once the player has played that much
more: words only (a greet, a reaction topic or a bark that reads it). 10 a player, the oldest going
first; each lasts 3 real days.

*Simplified:* missions' echoes and errands' follow-ups aren't on this queue yet, a callback waiting at
death doesn't become a board note, and the notebook has no "Promises" rows (the greeting is where you
see them).

## Claims in someone else's kit

The look itself (kit, colours, cover details, catches, word gets round) is reputation's
([REPUTATION.md](REPUTATION.md#borrowed-kit)). Conversations are where a claim is spoken:

- **People react to kit without any claim** (`kit:own`, `kit:in_kit`): "Kit suits you."; "Where did
  you get that jacket? Don't answer. I'd have to write it down." The true answer costs nothing.
- **A claim needs intent.** You're claiming only when you pick a `look` choice ("One of yours. Let me
  through."), its score and tags shown before you pick. A road post asks "Which unit?" when you walk
  up in their kit (`post.bastion.claim`, `post.redhand.claim`); a pass waves you through that post
  (`call:post_wave`), a catch is the built one (+3 Infamy, the kit taken, blown 60 minutes).
- **Cover details** you can say (the word at the wire heard today, a crew name, a real name) count at
  the look as built. **The word at the wire** is also a choice: Bastion's post asks "Word?" and offers
  three words, shuffled daily, one of them today's (`{wire:1}` to `{wire:3}`, `wire:N`). The right one
  is +1 at the look; a wrong one is caught at once; "Haven't had it today." runs the look without it.
  A friend can tell you the word over voice.
- **No rewards through a claim.** While you're passing as a faction, your persuade score with it
  counts 0 and you can't threaten its people. The lint refuses Favor, `offer`, `learn` and the like on
  a look choice or the node it passes into.
- **Leaders and home ground see through it**: a leader of that faction, or anyone inside its
  stronghold's or the lab's wire, catches the claim at once.
- **Changing clothes ends the conversation** ("Didn't you just change? Never mind. Off you go."):
  kit is read on every pick and on the server tick, so nobody opens a hub in one outfit and picks in
  another.
- No disguise with the Haven (no uniform) or the Long Table. Point-out is under [Several
  players](#several-players).

*Simplified:* kit kinds are own colours and their kit; signed-for kit, a dead man's and kit off the
turned come with disguise's later phases. A post's speaker doesn't call out to you at 35 m; you
press Talk. A catch taking your kit doesn't count as changing clothes.

## Barks

A **bark** is a conversation written with `bark` instead of a topic: one node of lines (variants pick
one), no choices, no effects. With Trees on, on the server tick, a player passing within **6 m** of
someone with a bark whose `when` holds hears it, owner-only, as that person's subtitle. Only bodies
already spawned near players are read (a 6 m query round each player).

- One bark a player a minute; one per speaker per player every 10 minutes; `once` barks once a life
  per speaker.
- None while you're talking to anyone, and none from someone who refuses you.
- A bark never names a player who is in someone else's kit, and says nothing the player couldn't see.

Barks read promises and callbacks too: *"Tins? No? Later, then. We're good at later."*

## Strangers

The errands' strangers ([ERRANDS.md](ERRANDS.md#strangers)) are speakers. Talk to one and their
call-out is said again as the greeting; their ask's ways are the hub's verbs, as built, paid by the
errand rules. With Trees on, `stranger:<routine>` conversations add who they are and why they're
there, in their own words: `{their:town}`, `{their:house}`, `{their:who}`... are the routine's own
texts (the lint allows `{their:...}` only for strangers). Nobody leans on a stranger through talk.

## The open channel

A radio item with **Call on the open channel** (`RZ_RadioCallAction`, [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md))
opens the talk screen on the open channel, with Trees on: no body, no faction, `for radio:open`. You
stay within talking range of the radio.

- Call in what you've seen (a rumour on the roads, said without a name), listen for anyone calling,
  ask the crews about the toll.
- **On air:** a `public` node goes to everyone within 1.5 km of the caller, as one line ("On the open
  channel, someone: ..."), with no name. The lint refuses `{player}`, `{name}`, `{word}` and the other
  personal tokens in a radio conversation, and threats and barks on air.

*Simplified:* any radio item with the action works, carried or on the ground; a house radio needs the
action on its prefab, and power isn't read. Distress calls and the secure nets aren't conversations.

## Conditions

Terms are joined with ` & `; `!` negates one; commas in an argument mean "any of". There is no OR:
write two choices. Every key reads a system that's already there; keys of designs not built yet
(`tribe`, `alpha`, `lean`, `finale`, `cure_held`, `ending`, `was`, `claimed`, `stranger`) read false.

| Key | Example | True when |
|---|---|---|
| `bg` | `bg:doctor,nurse` | your background is one of these (ids as in code: `radioham`, `labtech`, `excon`...) |
| `tier` | `tier:haven>=trusted`, `tier:bastion=wildcard` | your tier; `>=` and the like compare access rank (Hunted −2, Kill on Sight −3, Watched −1) |
| `fame`, `infamy` | `infamy:bastion>=15` | the number |
| `rep` | `rep:redhand=useful_trouble` | your reputation name with them |
| `hunted`, `exiled`, `kin` | `hunted:bastion` | Hunted or worse; a Haven exile; kin of a fallen Haven Champion |
| `flag`, `cflag`, `nmem` | `flag:list.read`, `cflag:asked=1`, `nmem:asked_rudi` | a season flag; a character flag; this speaker's note |
| `regard`, `face`, `seen`, `claim`, `knows` | `regard>=2`, `face:bastion`, `seen:haven.leader.list/told`, `claim:alone`, `knows:haven.list` | memory |
| `item`, `armed`, `bite_shown`, `stage` | `item:food*2`, `stage:weakness` | you carry food; a firearm; the HUD shows a bite; the infection stage |
| Time and season | `night`, `dusk`, `hour>=17`, `day>=5`, `outbreak:collapse`, `cure>=50`, `epilogue`, `mood:grim` | the world facts; the season mood ([WRITING.md](WRITING.md#the-season-mood)) |
| The speaker | `trait:greedy`, `loyalty:rogue`, `attitude>=1`, `stronghold`, `role:toll_man`, `met`, `helped`, `saved`, `crossed` | the leader's record; the built memory bits of this life |
| Place | `house:short`, `house:fed`, `house:died`, `house:medic`, `town:held=redhand`, `town:infection>=60`, `need:food` | the speaker's house, the nearest town, the faction's need |
| Wanderers | `wbitten`, `kidlost` | this wanderer is bitten; the Kid is lost and nobody is walking them |
| The Long Table | `table:guest`, `table:family`, `table:crossed` | its three words, as built |
| You | `crew`, `picture:mixed` | you have a crew name; the whole picture ([REPUTATION.md](REPUTATION.md#what-people-call-you)) |
| Others | `others>=1` | other players within 8 m |
| Scenes | `decided`, `decided:all_in`, `swayed:council.workers` | this conversation's scene is decided (any, or that value); you swayed that tally (a major's sitting: you've had your say) |
| Sittings | `sitting:every_name.first`, `tabled:every_name.first.cards` | a major's vote is seated and not tallied; that option is on its table |
| Lies, promises, callbacks | `heard_lie:scav.chemist`, `promise:house.tins@here`, `kept:...`, `broke:...`, `due:scav.came_back` | memory ([above](#memory)) |
| Kit | `kit:own`, `kit:in_kit`, `kit:full`, `kit:other`, `kit:any`, `passing`, `caught` | the speaker faction's kit as your own colours; their kit below Known; their full kit; another faction's colours; any; passing as them now; blown with them |
| The lock | `route:none`, `any`, `in`, `out`, `independent`, `unsided`, `won`, `lost`, `route:bastion`..., `lockdays<2` | your route at the 90% lock ([FINALE.md](FINALE.md#the-lock)): in is the speaker faction's route, out another faction's; in, out and independent until the cure; unsided while routes are open; won and lost after the cure; real days since you locked |
| The season record | `title:bastion=necessary_evil`, `seasoned:bastion` | the name your best life made with them this season; an earlier life stood out with them ([REPUTATION.md](REPUTATION.md#the-season-record)) |
| Reputation | `owed:bastion`, `setword:bastion` | Hunted and still owed a hearing (the parley); a word a major or questline set |
| Factions | `truce:redhand`, `feud:bastion` | the speaker's faction and that one have a truce or a feud now |
| Where | `radio` | on the open channel |

**Only what the player knows** goes into a tag: background, standing, items, the HUD, their own
memory. Nothing reads whether a cleaned bite took.

## The text format (.rzd)

Conversations are text files in `dialogue/<area>/<name>.rzd` (areas today: `leaders`,
`wanderers`, `residents`, `posts`, `table`, `strangers`, `radio`, `barks`). A file holds one or more conversations. A line whose
first character is `#` is a comment. Plain ASCII only (straight quotes).

```
conv <id>                       unique, dotted, lower case; never renamed once shipped
for <speaker>[, <speaker>]      who says it (Who talks)
when <conditions>               read when the hub is built and on every pick
topic "<hub label>" | greet | bark   a topic (40 characters at most), a greet (one node of lines), or a bark
prio quest|ask|world|reaction|flavour
once | daily                    per character, per speaker
voice haven|bastion|redhand|halcyon|table|plain

scene <id> speaker|season [who <conditions>]   deciding together (Several players): one record per speaker, or one for the season; who may decide
node <id> [public]              the first node is "start"; public: others within 8 m overhear it
  jump <conditions> -> <node>   checked in order on entry
  say <text> [~lie:<key> | ~truth:<key>]   one to three lines; 200 characters each; a lie told or the truth
  say? <conditions> :: <text>   the first true variant replaces the next plain say
  do <effects>                  on entry
  > @<id> <label> -> <pass>[ | <short>][ | <refused>] [check <kind> <faction> <difficulty> [easier:<trait>] [harder:<trait>] [bg:<ids>]]
      [cost favor:<faction> N | cost item:food*N] [if <conditions>] [do <effects>] [short <effects>] [else <effects>]
      [truth <conditions>] [callout <lie key>] [dry]
```

A `dry` choice is a node's wry way of saying what another choice in the same node says: the lint
holds it to the same targets, check, cost and effects ([WRITING.md](WRITING.md#conversations)).

A choice's label is everything between `@id` and ` -> `: speech in quotes, actions as plain words,
40 characters or fewer. Choice ids are unique in a conversation, and memory is keyed by
conversation, node and choice id, never by position.

**Tokens:** the world's (`{day}`, `{cure_percent}`, `{town:N}`...: [WRITING.md](WRITING.md#template-rules)),
plus `{npc}`, `{npc_short}`, `{role}`, `{here}` (the speaker's town or holding), `{house}`,
`{faction}`, `{player}`, `{boss}` (the speaker faction's stronghold leader), `{toll}` (your Red Hand
toll now), reputation's `{rep}`, `{word}`, `{best}`, `{worst}`, `{kills}`, `{jobs}`, `{picture}`,
`{name}`, `{crew}` for the speaker's faction, `{title:<faction>}` (the season record's title), a
stranger's `{their:<key>}`, and `{wire:1}` to `{wire:3}` (today's offered words at Bastion's wire).

**The pipeline.** `tools/dialogue_pack.py` lints every file and writes them as generated Enforce
Script, `scripts/Game/ReforgerZ/Dialogue/Pack/RZ_DialoguePack_<area>.c` (functions of at most 150
calls) and `RZ_DialoguePacks.c`. The server compiles them once at start into typed records indexed
by speaker kind. The packs are committed; never edit them by hand. *Simplified:* the design's
"plan A" (the server reading `.rzd` files from the addon) and server-only `$profile` overrides
aren't built: the packs are the one text source, so a text fix needs a mod update. A lie's mark and
a choice's `truth` and `callout` go into the packs as `Mark` and `ChoiceExtra` calls.

**Shipped ids.** `dialogue/shipped.lock` lists every conversation and choice id that has shipped.
The lint refuses removing one; a gone id is kept as `retired <id>`.

## Tools

| Tool | Does |
|---|---|
| `python3 tools/dialogue_pack.py` | Lint and write the packs. `--check` fails if the packs or the lock are out of date (the compile check runs it first); `--lock` adds new ids to the lock; `--list` lists the conversations |
| `python3 tools/dialogue_sim.py <conv> --bg nurse --tier haven=trusted` | Walks one conversation as one character sees it: the lines (variants picked), every choice with its tag, whether it shows, and where a check would send them. Also `--infamy`, `--fame`, `--favor`, `--flag`, `--knows`, `--fact night` (any other true term: `--fact heard_lie:scav.chemist`, `--fact kit:in_kit`), `--mem helped`, `--regard`, `--food`, `--house short`, `--trait`; `--all` shows every start node. A look's score needs the game, so it shows only where a pass and a catch lead |
| `python3 tools/voice_lint.py` | Also reads every `.rzd` line (says as talk-screen lines, labels as choices) against [WRITING.md](WRITING.md#checking-text) |
| **[Admin] Errands near here** (server admins, a reply on any named person, wanderer or occupant: no typing) | The errands' live count, the nearest player's last deal and why each other errand wasn't dealt, and their errands ([ERRANDS.md](ERRANDS.md#telemetry-and-the-admin-view)) |
| `/talkdebug` (admins, in chat; or the **[Admin] Read this person's conversations** reply) | Whether Trees are on, the conversation and memory counts, your memory, and for the person you're talking to, every conversation for them and the first `when` term that fails |

**The lint** (`dialogue_pack.py`): unique ids; known keywords, speakers, condition keys,
effects, checks, calls and tokens; targets that exist; nodes no path reaches; at most 6 choices;
labels 60 and topics 40 characters; says 200; a check fails into `end` or into a node with no
choices; a threat at a stronghold leader, the Kid, a safehouse or the Long Table; `regard:thanks`
without a cost or deed; Favor over 10 in one conversation; Fame, Infamy, house and world effects;
`offer` off a leader; an ask (`prio ask`) whose `when` doesn't read the world; a greet with choices;
the Long Table: its voice only for its people, its words never described (a banned list), its
names never outside its files, and nothing elsewhere hinting at it; "alpha", "tribe" or "crawler";
a pronoun after `{npc}`; `!` outside the Red Hand and Bastion (a warning for Bastion); a bark with
choices, effects or `public`, or on the radio; a `callout` for a lie no line tells; `truth` off a
[Lie]; a look for the Haven, or a reward behind a look (Favor, `offer`, `learn`, `errand`, `reveal`,
`begin`); a threat at a stranger or on air; personal tokens on air; `{their:...}` outside a stranger;
a `dry` choice with no plain twin.

## The conversations

✅ 91 conversations, 212 nodes (`python3 tools/dialogue_pack.py --list`), 11 of them barks.

| Speaker | Conversations |
|---|---|
| Haven leaders | The bear on the door; the names on the list (persuade: who crosses them out; [Elder], [Nurse] +1); who they read out at supper |
| Bastion leaders | The board on the wall (the roster: nobody is coming, they're all they have; [Soldier], [Police]; being signed on); whether Command ever answers |
| Red Hand leaders | The book (the first line, Rudi, persuade to see your own page, which reads it out in the book's voice); why the hand is red |
| Halcyon leaders | Why they're called the beaks ([Lab Technician], [Doctor], [Nurse]: the notebooks); how close the cure is (the other two); a field lead on the volunteers ("They're comfortable": a lie, with [Lab Technician], [Doctor], a persuade and an [Evidence] call-out to the room at the back) |
| Every leader | The name, not the face (a new life greeted by the season's title, once); a lie found out, said at the next greeting, once |
| Haven leaders at a sitting | Every Name's first and second sittings: a persuade to sway the speaker's vote to an option on the table (one sway a sitting) |
| The 8 wanderer roles | The Trader's prices; the Doctor's work ([Doctor], [Nurse]); where the Scavenger has been (persuade: is it true); why the Deserter left ([Soldier], [Police]); the Preacher's bells; the Courier's letters; what still runs ([Mechanic]); the Kid's name; the Scavenger's clean chemist's (a lie for a tin, [Hunter] or [Police] see through it, [Evidence] once the Doctor tells you, and a callback greet 6 hours on); the Doctor on the chemist's and on Halcyon's back room (the truths); what Bastion makes of you, as the Deserter reads it |
| Occupants | A fed house (leave a tin: 2 Haven Favor); the candle in the window after a death (a greet, and a topic: staying for the name is remembered 6 hours on); the bear on the door; who gets the bed by the stove (a short house: a public scene per house that someone Known with the Haven decides, with a sway for the room); a short house's promise of two tins (kept by the hand-in, with greets for kept and broken) |
| Named people | The Red Hand toll man (who painted the hand; Rudi, said flatly); the Bastion sentry (the roll; a promise to walk the road and come back, kept by coming back); kit greets at Bastion, Red Hand and Halcyon holdings (own colours, or their kit on a stranger) |
| Post speakers | Bastion's search (persuade with [Soldier], [Police]: the pole goes up); the Red Hand's toll (pay it, or threaten: Fear, +3 Red Hand Infamy); "Which unit?" and "Say you're crew." in their kit (a look, or the truth); "Word?" at Bastion's (three words, one right) |
| The Long Table | The host on the road (where you're headed, and is anyone expecting you; "Put out, were you?" to an exile); the mother at the farm (is the boy hers; [Bitten]: declare it, as built) |
| Strangers | Where any stranger's headed; and for 15 routines, who they are: the girl at the tin man's house, the wife who drove to market, the pram of pans, the one shut in since Tuesday, the kids in the back seat, who the grave is for, the sleeve rolled down ([Doctor], [Nurse]), the farm, the sister in a barn, the husband across the line, the debt in the book, the one who hid, the one who got there first, the scratch off a fence |
| The open channel | Call in what you've seen (a rumour, on air); listen for anyone calling (by the night and the mood); ask the crews about the toll |
| Barks | A short house (and its tins, promised or brought), a house after a death, a house at dusk, a Red Hand crew name and the toll man, Bastion's shift change and a set word, Halcyon's samples, the lost Kid, the Scavenger once the callback is due, the sentry once the road is walked |
| Asks | A short house (two tins into the stores: the built hand-in); the Kid, lost (show the way: the built escort); a bitten wanderer (your antidote: the built one) |

## Saving and performance

- **Cost.** A hub is built only when someone talks, from the conversations indexed by speaker
  kind; conditions are compiled once at start. Picks are event-driven. The post speaker is one
  body per post, only within 100 m of a player. Barks are a 6 m query round each player on the
  server tick, only while there are barks and Trees are on; promise and callback clocks run for
  players online with something open.
- **Network.** One owner-only record per node, as the talk screen sends today.
- **Saving.** `$profile:ReforgerZ/dialogue.json`, written at the 2-minute save only when something
  changed; dropped when the season number changes (a wipe). `state.json` doesn't grow.
- **Log lines:** `Dialogue: loaded N conversations, M nodes`; `Dialogue: trees on|off`;
  `Dialogue: memory for N players`; `Dialogue: memory from season N dropped (season M)`;
  `Dialogue: budget reached <player> <faction>`; `Dialogue: debug: ...`; `Dialogue: scene <key> decided
  <value> by <player>`, `Dialogue: scene <key> hold by <player> objected to by <player>`, `Dialogue: sway <name>
  by <player> (now N)`, `Dialogue: <player> vouched for <player>`, `Dialogue: sway <major>.<vote>.<word> by <player>
  (voter N)`, `Dialogue: promise <key> made|kept|broken by <player>`, `Dialogue: bark <conv> to <player>`,
  `Dialogue: session cap at <speaker>` ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check)).

## Simplified, and not built yet

- *Simplified:* trees run only on the talk screen. The design's other deliveries (the HUD panel
  with action-menu donors, the numbered `/t` fallback) aren't built: with the talk screen off, there
  are no trees.
- *Simplified:* greets are lines only (a greet with choices comes with scenes); "dimmed, not hidden"
  topics show a "heard" tag.
- *Simplified:* knowledge is written only by conversations (`learn:`); the radio, clues and leaders'
  news don't record `knows:` keys yet, and the notebook has no "What you know" page yet.
- *Simplified:* mission and questline people keep their built choices; `SpawnPerson` doesn't take a
  conversation yet.
- **Planned** ([designs/DIALOGUE.md](../designs/DIALOGUE.md)): Make it right; the finale's commit and
  the epilogue's topics; server-opened greets and resuming a conversation; injected topics; the
  other deliveries; the rest of the format (`bind`, `use`, `always`, scopes beyond speaker and season).
