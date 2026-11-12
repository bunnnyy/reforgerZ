# Dialogue

How talking to people works: the talk screen and its engine, the conversation trees on top of it
(**Dialogue > Trees**, on in the repo's game mode since 2026-11-12; the attribute's default is off), what people remember, the checks shown before you choose,
lies you're told, promises and callbacks, claims in someone else's kit, Make it right, who opens a
conversation, barks, strangers, the radio (the open channel, the secure nets, distress calls), the
finale's commit, the last week, the `.rzd` text format, its lint and simulator, and the conversations
that ship. How the screen looks and is driven with a pad is in [UI.md](UI.md#the-talk-screen); how lines
are written is in [WRITING.md](WRITING.md#conversations).

Code: `Dialogue/RZ_Dialogue.c` (sessions), `Dialogue/RZ_TalkSpeakers.c` (who someone is),
`Dialogue/RZ_TalkStrangers.c` (strangers), `Dialogue/RZ_DialogueRadio.c` (the open channel and its
action), `Dialogue/RZ_DialogueTrees.c` (hubs, nodes, checks, effects), `Dialogue/RZ_DialogueDB.c` (the
compiled conversations), `Dialogue/RZ_DialogueCond.c` (conditions and tokens),
`Dialogue/RZ_DialogueMemory.c` (memory and `dialogue.json`), `Dialogue/RZ_DialogueScenes.c` (several
players), `Dialogue/RZ_DialogueVotes.c` (the majors' sittings), `Dialogue/RZ_DialogueBarks.c` (barks),
`Dialogue/RZ_DialogueWire.c` (Bastion's word at the wire), `Dialogue/RZ_DialogueWorld.c` (conditions and
tokens that read the season's story, the infected, the radio and kit marks), `Dialogue/RZ_DialogueMakeRight.c`
(Make it right), `Dialogue/RZ_DialogueOpens.c` (greets that open by themselves), `Dialogue/RZ_DialogueNets.c`
(the secure nets, distress calls, a house radio's power), `Dialogue/RZ_DialogueStory.c` (scene and
questline flags, notes, approaches, TryBegin, claims the world tests), `Dialogue/RZ_DialogueBinds.c`
(bindings, `{claim:...}`, world predicates), `Dialogue/RZ_DialogueJournal.c` (the notebook's Promises and
What you know rows), `Dialogue/Pack/` (generated). Server only, except the screen.

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
- [Make it right](#make-it-right)
- [Who opens a conversation](#who-opens-a-conversation)
- [Barks](#barks)
- [Strangers](#strangers)
- [The radio](#the-radio)
- [The finale's commit](#the-finales-commit)
- [The last week](#the-last-week)
- [The bitten, the clock and what you carry](#the-bitten-the-clock-and-what-you-carry)
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
  card), what they said stays. Then come their answer and the replies after it, New Vegas style.
- **Five at most on the screen.** The server sends a page of the list: up to four replies and the
  way out, or, when there are more, three, **Something else.** (the next page, round to the first)
  and the way out. A new list (another node, back to the hub) starts on its first page.
- **[Admin].** A server admin's replies (the admins' chat commands without typing, [Tools](#tools))
  sit under one reply, **[Admin]**, at the end of the list; it opens them as their own list, with
  **Back.** as its way out (B and Esc too). Only at the hub, never in a node.
- **Grey: nothing new.** A reply with nothing new behind it is grey and still works: a built reply
  you've picked in this conversation (forgotten when it ends); with Trees on, a topic or a choice
  whose nodes you've seen through ([below](#trees-hubs-topics-and-nodes)). The screen's own rows
  and the admins' are never grey.
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

✅ **Dialogue > Trees** (the attribute's default is off, the author's decision 13; the author turned it on in `RZ_GameMode.et` on 2026-11-12 for play-testing, before the per-player label test
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
  list after the verbs, a page at a time on the screen ([above](#the-talk-screen)). A topic you've
  opened that still has more in it is tagged "heard"; one with nothing new is grey. `once` topics
  go for this life at that speaker, `daily` ones for a real day.
- **I'll be going.**, always.

A topic opens its first **node**: the speaker says one to three lines, and you pick a **choice**
(at most 6, plus I'll be going., paged as above). A choice goes to another node, back to the hub
(`hub`), back a node (`back`), or ends the conversation (`end`). Nodes can jump on conditions.
Picking a choice reads its condition and the conversation's `when` again in the live world; if
either no longer holds, they "didn't catch that". A choice with a price is refused within 1.5 s of
a new node, so a list that hasn't refreshed can't take your Favor.

**Grey, from the seen nodes** (`RZ_DialogueTrees.Heard`, the memory's seen nodes, per character): a
choice is grey when the node it leads to on a pass (after that node's jumps, as entering it takes
them) is seen and every choice showing there is heard in turn, up to 6 nodes deep; a topic is grey
when its first node is heard that way. A choice to `hub`, `back` or `end` leads nowhere new: it is
never grey itself and doesn't keep its node from being heard. A check you haven't passed leads to a
node you haven't seen, so it stays bright. Seen nodes are shared by every speaker of that kind (one
wanderer's conversation heard is heard at the next).

**Leaders never give errands** in a tree; a leader's line names who to see.

## Who talks

Each speaker is described by its adapter (`RZ_TalkSpeaker.Describe`): its kinds (what a
conversation's `for` names), a memory key, its faction, and the leader, wanderer, house or post
behind it.

| Speaker | `for` | Memory key | Built verbs |
|---|---|---|---|
| Stronghold leaders | `stronghold:<faction>`, `leader:<faction>`, `leader` | `leader:<seat>:<generation>` (someone new in the chair starts fresh) | Ask for work, Heard anything?, What do you make of me?; in the epilogue week About the season, Where should I look? and the grey choice's two answers |
| Seat leaders | `seat:haven/council_head`, `seat:bastion/outpost_lead`, `seat:redhand/crew_boss`, `seat:halcyon/field_lead`, `leader:<faction>`, `leader` | as above | as above (no grey choice) |
| Wanderers | `wanderer:<role>` (trader, doctor, scavenger, deserter, preacher, courier, kid, mechanic), `wanderer` | `wanderer:<id>:<generation>` | the role's special, Share some food, the antidote while bitten (else Need a hand?, [errands](ERRANDS.md#the-wanderers-asks); the Mechanic: Ask about cars), Rob |
| Safehouse occupants | `occupant` | `house:<index>` | Ask how things are |
| A safehouse's named people (the first two occupants, [ERRANDS.md](ERRANDS.md#givers)) | none yet (no trees) | `house:<index>:<i>` (errands' regard) | Ask how things are, Anything need doing? |
| Named people at holdings | `named:<faction>/<role>` (`named:redhand/toll_man`), `named:<faction>`, `named` | `named:<holding>:<index>` | Ask how things are, Anything need doing? (errands) |
| A road post's speaker | `post:bastion`, `post:redhand`, `post` | `post:<faction>:<town>` | none |
| The Long Table | `table:host`, `table:mother`, `table:son`, `table:boy`, `table:road` (the host out on the road), `table` | `table:<role>` (no regard: its memory stays its three words) | eat, trade a jar, declare a bite; on the road, accept or decline |
| Strangers ([below](#strangers)) | `stranger:<routine>` (`stranger:tin`, `stranger:digger`...), `stranger` | `stranger:<visit>` | their ask's ways, as built ([ERRANDS.md](ERRANDS.md#strangers)) |
| The open channel ([below](#the-radio)) | `radio:open`, `radio` | `radio:open` | none |
| A cracked secure net ([below](#the-radio)) | `radio:bastion`, `radio:redhand`, `radio:halcyon`, `radio:secure` | `radio:open` | none |

| A contract's or questline stage's people | `quest:<questline>` (`quest:every_name`), `quest`; `mission` for a contract's | `mission:<id>:<index>` | their built choices, as built: a conversation picks one with `call:slot:N`, so the stage resolves (and starts anything physical) as built |

Factions are written `haven`, `bastion`, `redhand`, `halcyon`, and `none` for factionless checks.
Strangers and the open channel are factionless; a mission's people speak for its giver.

**The post speaker.** With Trees on, one of the crew stands at the barrier of a Bastion checkpoint
or a Red Hand roadblock once a player is within 100 m: the faction's territory occupant prefab
(else the puzzle civilian), entered in the one body budget as a speaker
([INFECTED.md](INFECTED.md#the-one-body-budget)), and gone once nobody is within 150 m or can
see them. Talk gives the post's
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
| `threaten <faction> <difficulty>` | [Fear 1 of 2 - +3 Infamy] (": Hunted" when it would make you Hunted) | **Fear**, with the threat's rules: +3 Infamy and regard *threatened* whatever happens, once per person by anyone for the season ("Already scared. Of someone else."), three a life per faction, a refused threat barred a real day; a threatened leader or wanderer remembers being crossed (this life). Never on stronghold leaders, the Kid, the Long Table, in safehouses (the lint refuses it, and the engine refuses it on a stronghold leader), or during a parley |
| `lie <faction> <difficulty>` | [Lie - 1 of 2], or [Lie - they'd see through it] | A **tell** catches it first, whatever the score: the bite showing, another faction's colours on you, or something else you already told this speaker. Without one, Trust against the difficulty (+1 under a vouch). The lie's difficulty is +1 at a Paranoid leader, +1 where this speaker has caught you lying this life, −1 for an Ex-Con or a Raider (all shown in the tag), and, never shown, ±1 for the speaker's own credulity, seeded by who they are and the season (a lie is the one check that's a gamble). Believed: a false claim in the [lie store](DESIGN.md#lies-and-witnesses), found out later by its rumour rounds (the speaker's regard *lied*, +2 Infamy). Caught now: regard *lied*, +2 Infamy and the liar count (−1 at Trust this life), and no more lies at that speaker for a real day ("they caught you lying today"). The reply opens with "They believe you. For now." or "They don't believe a word of it." A choice's `truth <conditions>` makes it true while they hold: then it's no lie, with no tag and no check. A caught or found-out lie is said at that speaker's next greeting (`nmem:lied`) |
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
| `learn` | `learn:haven.list`, `learn:bastion.word:6h` | Something you now know (`knows:` keys): for the season, or a **passing fact** for that many real hours (32 at most) |
| `rumour` | `rumour:"..."@6` | Gossip on the roads (the world's rumours) |
| `call` | `call:slot:2`, `call:hand_in:2`, `call:toll`, `call:post_wave`, `call:standing` | Built behaviour, unchanged: the person's own reply N; the N food the same choice's `cost item:food*N` took go into this safehouse's stores, paid as a delivery (the lint pairs the two, same N; nothing more is taken); the Red Hand toll at this roadblock (Known or better, as built); this post lets you through once while it stands; what they make of you, in their voice |
| `call` (more) | `call:season`, `call:look`, `call:declare_bite`, `call:caught_bite`, `call:distress`, `call:net_word`, `call:net_lost`, `call:brief_public` | A leader's epilogue carriers, as built (the season at this seat; where to look); the safehouse's bite service, as built; a bite lie caught at a safehouse counts as caught hiding that bite; a distress caller sent to a house's gate; the word at the wire from Bastion's net; codes changed on an intruder ([The radio](#the-radio)); a giver's lie taken public ([DESIGN.md](DESIGN.md#story-blocks)). The lint holds each to its own speaker |
| `decide` | `decide:all_in` | In a conversation with a `scene`: the pick waits 5 s for objections ([Several players](#several-players)), then writes the scene's record (who, which), once for the world |
| `sway` | `sway:council.workers` | In a conversation with a `scene`: one contribution per player to a count its owner reads (`tally.<name>`) |
| `offer` | `offer:leader_job` | A leader's personal job (the built Ask for work) |
| `commit` | `commit:bastion` | The finale's lock, from a confirm node only ([The finale's commit](#the-finales-commit)) |
| `promise` | `promise:house.tins@here:2h` | A promise to this speaker, in the words of the choice that made it ([Promises](#promises-and-callbacks)) |
| `keep` | `keep:house.tins@here` | Keeps that promise, if it's open |
| `later` | `later:6h:scav.came_back` | A callback: `due:scav.came_back` reads true once you've played 6 more hours |
| `wire` | `wire:2` | Says the second of today's three words at the wire to Bastion's people: the right one counts at your next look today; a wrong one is caught at once |
| `set` | `set:bed.stove@here=1` | A world flag of the scene's (`talk:<key>` reads it). Scene only, so it runs once for everyone |
| `qset` | `qset:every_name.family=1` | A running questline's own flag. Scene only |
| `begin` | `begin:other_two` | That questline's own TryBegin, with its rules. Scene only; never behind a claim |
| `note` | `note:every_name:"The head will take two."` | A line in that run's journal notes |
| `approach` | `approach:every_name:talk:"talked the head round"` | You took part in that run that way (the questlines' approaches) |
| `test` | `test:expected:20.5:300` | A claim the world tests: at the next 20:30, still within 300 m of where you said it, it was false, and this speaker remembers that (`nmem:expected.false`). Not saved |

There is **no Fame, no Infamy and no item** from talk, ever: the lint refuses `fame`, `infamy` and
`house` effects, and Infamy comes only from a tagged threat, a caught lie or a caught claim. Effects
that change the world for everyone go through a scene (`decide`, `sway`, `set`, `qset`, `begin`).
**Talk resolves only talk:** a stage's outcome, its pay and anything physical (an escort, a walk back)
are the stage's own built choices, which a conversation at its person picks with `call:slot:N`; an
errand is dealt by errands.

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
regard. Errands write regard only on named people's keys (a house's people, a holding's), never on
leaders ([ERRANDS.md](ERRANDS.md#givers)). The **liar count** is the life's record: a caught or
found-out lie with a faction is −1 at Trust with it for the rest of the life.

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
  Infamy with that faction and regard *vouched_liar*, and 2 hours without vouching. Once per pair per
  hour. At a road post's barrier or Halcyon's gate, Known with Bastion or Halcyon in your own colours is
  enough, and the vouch also signs them through ([REPUTATION.md](REPUTATION.md#borrowed-kit), "Vouching").
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

- **Overheard lies.** A lie said in a `public` node is a lie told to everyone who heard it (within 8 m,
  or on air): they can call it out later too.
- **Lies that move the world.** A lie believed in a scene is recorded on it (`scene_lied:<scene>`, with
  the liar's identity in the world memory), so a questline or a later conversation can read it.
- **Claims the world tests.** Some claims are checked by their owner, not left to rumour: the Long
  Table's host tests *"They're expecting me back by dark"* at 20:30 (`test:`); still near the road,
  it wasn't true, and next time: *"Still expected, are you?"*.
- **Hiding things.** A speaker keeps something back behind a deflection in their own voice (*"Book's
  the book. Ask me something else."*), with a Press beside it: a persuade, or a threat where threats
  are allowed, with a fail branch.
- **The notebook's "What you know"** row lists the lies you've seen through, as they were said
  (*"They told you: "The chemist's in Brod. Back room, top shelf. Nobody's touched it." It wasn't true."*).

Knowledge also comes from the secure nets and the word at the wire ([The radio](#the-radio)); clue reads and
the open channel's chatter are their own systems' notices.

## Promises and callbacks

**Promises.** `promise:<key>[:<hours>h]` on a choice is a promise to this speaker in the words of that
choice ("I'll bring two tins. Two hours."):
- **Clock:** the promiser's own play (2 hours by default), and 3 real days at most. Logging off
  doesn't break one; dying does.
- **At most 3 open** per player ("You've three promises open already. Keep one first.").
- **Not again so soon:** a promise kept or broken can't be made again for 3 real days ("You made
  them that promise lately. Give it a few days."); a promise choice hides with `!kept:` and `!broke:`.
- **Kept** by `keep:<key>` on whatever does it (the house's own hand-in keeps `house.tins@here`), or,
  for a key ending in `.visit`, by talking to the same speaker again once you've played 10 minutes
  since and been 300 m from where you made it (reopening the conversation keeps nothing). Regard
  *kept* (+1) and a notice.
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

- **Kept by the world:** a promise whose key starts `walk.` made to a wanderer is kept when that
  wanderer is settled at a house (the Kid: *"I'll get you home. I promise."*); a hand-in keeps a house's
  tins; a message to someone is kept by `keep:` at whoever it's for.
- **At death** an open promise to a house becomes an unsigned note on its board: *"For whoever
  promised "I'll bring two tins. Two hours.", if anyone sees them: we waited."*
- **In the notebook:** one **Promises** row in the Journal while you have any open, each with its play
  left (`Dialogue/RZ_DialogueJournal.c`).

Dialogue's `later:` is the talk-only half of the one callback queue: it sets a `due:` key for a
conversation to read. Missions' echoes and errands' follow-ups are lines that come back by themselves,
on the shared queue ([DESIGN.md](DESIGN.md#the-shared-engine)), with its system pool and its board notes
at death.

## Claims in someone else's kit

The look itself (kit, colours, cover details, catches, word gets round) is reputation's
([REPUTATION.md](REPUTATION.md#borrowed-kit)). Conversations are where a claim is spoken:

- **People react to kit without any claim** (`kit:own`, `kit:in_kit`): "Kit suits you."; "Where did
  you get that jacket? Don't answer. I'd have to write it down." The true answer costs nothing.
- **A claim needs intent.** You're claiming only when you pick a `look` choice ("One of yours. Let me
  through."), its score and tags shown before you pick. **Once per speaker per life:** a second claim at
  the same person gets *"You told me that already."* A road post asks "Which unit?" when you walk
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

- **Kit that remembers whose it was** ([REPUTATION.md](REPUTATION.md#borrowed-kit)) is a `kit:` word too:
  `kit:issued` (signed out to you, worn after your standing slipped: *"You're still in our kit. Take it
  off before someone asks why."*), `kit:signed_other` (*"That's {kit_owner}'s jacket. Signed for."*),
  `kit:dead_mans` (`{dead_name}`), `kit:off_turned`. A claim in marked kit is an instant catch at the
  look, as built.
- **Cover details spoken** are choices: `do cover:name|dead|crew|notebook|paper`, shown with
  `if cover:<key>` (you have it), the words as `{cover:<key>}` ("Kral", "Magpie", "31"). One you have
  counts at your next look with them; one you don't is a lie, caught at once, so a node after it
  checks `jump caught -> ...`. `claimed`: you've spoken a claim line to this speaker this life (one a
  life). `{cover}` alone says your best cover detail for the speaker's faction. What a detail is worth
  is reputation's ([REPUTATION.md](REPUTATION.md#borrowed-kit)).
- **Claims the look decides:** your own crew name at the Red Hand's post (*"They call me {crew}."*) and
  a notebook number at Halcyon (*"{cover}. I'm on the field team."*, `if cover:notebook`) are look
  choices, the detail's +1 counted as built.
- **A road post asks first:** its speaker opens the talk screen at 35 m on someone in their kit below
  Known ([Who opens a conversation](#who-opens-a-conversation)), reading the kit's marks first (*"Stop
  there. That's off one of ours. Where is he?"*). A named person at a holding passes you for 10
  minutes; a post, while it stands.

A catch taking your kit doesn't count as changing clothes.

## Make it right

**Dialogue > Make it right** (on), with Trees. A leader who won't deal with you on attitude (−2 or
below) still offers one reply, tagged **[Make it right]**: *"What would it take?"*. The Hunted don't
get it (they have the [parley](REPUTATION.md#the-parley-and-the-knock)).

- It reserves that faction's amends contract for you for an hour, the parley's own pick (the Haven's
  Medicine Run, Bastion's Burn the Bodies, the Red Hand's Fence the Goods, Halcyon's Field Samples), in
  the leader's voice (*"A job, done properly, signed for. Then the file gets a new page."*).
- When the job succeeds with you in it, that leader's regard for you goes to 0 and their crossed memory
  of you (this life) is cleared: *"That's done, then. We start again. Don't make me regret it."*
- **Once a season per leader** (*"We did this once. I don't do it twice in a season."*); a second ask
  while the job is open is dim.
- The Infamy the job works off goes through `Forgive()`, inside the one budget, as every amends
  contract does ([REPUTATION.md](REPUTATION.md#working-off-infamy)). Make it right adds nothing to it.
- `made_right` reads true at that leader afterwards, for a greet that remembers it.

## Who opens a conversation

**Dialogue > Opens** (on), with Trees. A greet written with `opens approach <metres>` (40 at most)
opens the talk screen by itself for a player who walks up within that distance while its `when`
holds, so the question is put whether or not they press Talk:

| Who | At | When |
|---|---|---|
| A road post's speaker (Bastion, the Red Hand) | 35 m | you're in their kit below Known, not caught, not passing |
| A stranger | 10 m | once a life per stranger: *"Hey. You. Sorry. Have you got a minute?"* |
| The Long Table's host on the road | 12 m | once a life, unless the family has crossed you |

Checked every 5 s for bodies already spawned near players. Never while you're talking, in a vehicle
or in danger (the screen guard refuses it), never twice in 10 minutes from the same person, and each
is an unasked contact inside the notice lanes' budget (one every 4 minutes, [UI.md](UI.md)). Leaving
is one button, as always; walking away from a post is no claim, and its search or toll applies as
built when you reach the barrier.

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

## The radio

A radio item with **Call on the open channel** (`RZ_RadioCallAction`, [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md))
opens the talk screen on the open channel, with Trees on: no body, no faction, `for radio:open`. You
stay within talking range of the radio. A house radio (the radio spot's third choice, **Call on the
open channel**) calls only with power (a running generator close by, `RZ_PowerGrid`): *"No power.
It'll hear the channel, but it won't carry your voice."* A radio you carry has its own batteries.

- Call in what you've seen (a rumour on the roads, said without a name), listen for anyone calling,
  ask the crews about the toll.
- **On air:** a `public` node goes to everyone within 1.5 km of the caller, as one line ("On the open
  channel, someone: ..."), with no name. The lint refuses `{player}`, `{name}`, `{word}` and the other
  personal tokens in a radio conversation, and threats and barks on air.

**The secure nets.** A Radio Operator listening to a cracked net (`RZ_SecureRadio`,
[DESIGN.md](DESIGN.md)) who calls on a radio can talk on that net too: Bastion's command net, the Red
Hand boss's channel, Halcyon's lab channel (`for radio:bastion`, `radio:redhand`, `radio:halcyon`,
`radio:secure`). An intruder's words go to the net, never to the air: the lint refuses `public` there.
Each net has a lie to try (*"Post four. Say again the word."*): believed, you learn something (the word
at the wire, `call:net_word`, +1 at Bastion's looks today; the toll; the lab's count); caught, it costs
the lie's Infamy as anywhere and they change codes (`call:net_lost`: the listening ends).

**Distress calls** (`distress`). An hour in two (the season's seed and the real hour, the same for
everyone listening) someone near the nearest Haven house within 2 km is on the open channel asking
which way (*"...anyone on this. Two of us, near {distress}. The car's done and the light's going."*).
Answering sends them to that house's gate (`call:distress`), where errands deal its gate card, Who's at
the Gate, under errands' own rules and pay ([ERRANDS.md](ERRANDS.md#the-cards)); one in four is
bitten. Only while nobody is at that gate already, once a real hour per player.

**Dialogue > Radio calls** (on) switches the secure nets and the distress calls together.

## The finale's commit

The cure finale's lock ([FINALE.md](FINALE.md#the-lock)) runs through the trees. With Trees off the
finale's desks take the names instead.

- **`always` topics.** A conversation marked `always` shows even when the speaker refuses you (a
  Hunted player, attitude −2): the finale's route topics, so a leader who won't talk to you still
  offers the route. The lint allows `always` only on a topic with a confirm node.
- **The confirm node** (`node <id> confirm`), tagged **[Point of no return]**: its lines, then the
  finale's three plain lines (the route's admission, what you bring, what you leave). It has exactly
  one commit choice and at least one way back.
- **`commit:<route>`** (`haven`, `bastion`, `redhand`, `halcyon`, `indep`) runs only from a choice in a
  confirm node, never on entry, never on a check's short or else. The choice has no `if`, check or
  cost: the finale decides who may lock. It's dimmed with `CanCommit`'s reason when you can't, refused
  within 1.5 s of the node opening, and checked again when picked.
- **Where:** each faction route at its stronghold leader and its Loyal seat leaders; the
  Independent at the Doctor on the road, else the Courier (`finale:voice`), or **on the open channel**
  from any radio that can call (`finale.route.independent`): said on air without a name, *"This is
  nobody. The cure goes to everyone. No crates, no lists."*, and every faction hears that somebody said
  it (a rumour on the roads), never who. From 75% every leader's
  "Ask about the cure." says to ask again later; from 80% the four stronghold leaders and the
  Independent's voice give the pitch (`dialogue/finale/routes.rzd`).

## The last week

In the epilogue week ([EPILOGUE.md](EPILOGUE.md)) lines read four separate things and never mix them
up: the world's outcome (`cure_held:`, `ending:`), who was there (`was:`), this player's route
(`route:`) and this character's memory.

- **Every leader:** "Ask about the cure." (their own read of who holds it, in their faction's voice,
  then *What happened here, this season?* and *Where should I look?*, which run the built carriers,
  `call:season` and `call:look`) and "Ask what they'll do now." (themselves, a week from the barrier;
  a line for a won or lost route, and for having decided their major). The built replies stay as
  verbs.
- **Occupants:** a greet for the week (*"The cure came in a crate with a number on it."*; *"Mrs
  {resident} asked who to thank"*), and a topic for having stood with Bastion.
- **Barks** in passing, at houses and holdings.

The closing work's own pieces (Loose Ends, Bring Them Home, the Walk Out, the Last Broadcast, the Last
Entry, Settle Accounts) are the epilogue's choices and actions, as built.

## The bitten, the clock and what you carry

- **[Bitten] topics** while the HUD shows a bite: tell a safehouse (`call:declare_bite`, the house's
  own service); a lie there (*"A scratch off a fence."*) has the symptoms as its tell, and caught it
  counts as caught hiding the bite (`call:caught_bite`) on top of the lie's cost; ask about Halcyon's
  trial dose; the Long Table's own (its blessing, as built).
- **Reading symptoms:** the Doctor on the road for a fever that isn't a bite; a Nurse or Doctor at a
  short house (*"That isn't a bite. It's her chest."*).
- **The day's routines** as lines, not clocks: shift change, supper and grace, the night watch.
  Dusk and night open and close topics as before.
- **What you carry** is read by the built keyword lists; taking is real, and talk never gives an
  item.

## Conditions

Terms are joined with ` & `; `!` negates one; commas in an argument mean "any of". There is no OR:
write two choices. Every key reads a system that's already there, and reads false while that system
is off.

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
| The finale | `finale:soon`, `finale:pitch`, `finale:open`, `finale:locked`, `finale:voice` | research 75–90 before the gate; 80 or more; the routes are open; you're locked to a route; this wanderer is the Independent's voice (the Doctor, else the Courier) |
| The lock | `route:none`, `any`, `in`, `out`, `independent`, `unsided`, `won`, `lost`, `route:bastion`... (`haven`, `bastion`, `redhand`, `halcyon`, `indep`), `lockdays<2` | your route at the 90% lock ([FINALE.md](FINALE.md#the-lock)): in is the speaker faction's route, out another faction's; in, out and independent until the cure; unsided while routes are open; won and lost after the cure; real days since you locked |
| The season record | `title:bastion=necessary_evil`, `seasoned:bastion` | the name your best life made with them this season; an earlier life stood out with them ([REPUTATION.md](REPUTATION.md#the-season-record)) |
| Reputation | `owed:bastion`, `setword:bastion` | Hunted and still owed a hearing (the parley); a word a major or questline set |
| Factions | `truce:redhand`, `feud:bastion` | the speaker's faction and that one have a truce or a feud now |
| Where | `radio` | on the open channel |
| This conversation | `bound`, `bound:target`, `scene_lied:<scene>`, `talk:<key>` | the topic's bindings were picked (that one); a lie was believed in that scene; a scene's own flag (`set:`) |
| The season's story | `cure_held:bastion` (`haven`, `redhand`, `halcyon`; the Independent: `indep` any terms, `everyone` its open terms, `indep_side` one side, `indep_sold` sold, `indep_kept` kept back; `nobody` handed over, `slow`, `any`; `none`: no cure by the wipe), `ending:every_name`, `ending:every_name=every_name`, `was:book`, `lean:every_name.first.pot`, `q:every_name.act=1` | after the cure, who holds it (the epilogue's record; `none` is the last day with no cure); a major ended (that way: its ending's name, lower case, without "the"); you were one of those who decided it; the speaker's own voter leans to that option; a running questline's own flag |
| The infected, you, the room | `tribe`, `alpha`, `declared`, `symptoms`, `group` | a pack's range takes in the speaker's place; that pack has its leader; you told a safehouse about your bite; visible symptoms; another of your group within 150 m |
| The radio | `secure`, `secure:bastion`, `power`, `distress` | you're listening to a cracked net (that one); the radio has power; a distress call is on air for you |
| Kit and cover | `kit:issued`, `kit:signed_other`, `kit:dead_mans`, `kit:off_turned`, `cover:name` (`dead`, `crew`, `notebook`, `paper`), `claimed` | what the kit itself says; you have that cover detail to give the speaker's faction; you've spoken a claim line to this speaker this life |
| The speaker | `busy`, `made_right` | a quest topic of theirs shows now (a story holds them: nothing else offers work through them, *"Not now. I've got enough on."*); you made it right with this leader this season |

**Only what the player knows** goes into a tag: background, standing, items, the HUD, their own
memory. Nothing reads whether a cleaned bite took.

## The text format (.rzd)

Conversations are text files in `dialogue/<area>/<name>.rzd` (areas today: `leaders`, `finale`,
`wanderers`, `residents`, `posts`, `table`, `strangers`, `radio`, `barks`). A file holds one or more conversations. A line whose
first character is `#` is a comment. Plain ASCII only (straight quotes).

```
conv <id>                       unique, dotted, lower case; never renamed once shipped
for <speaker>[, <speaker>]      who says it (Who talks)
when <conditions>               read when the hub is built and on every pick
topic "<hub label>" | greet | bark   a topic (40 characters at most), a greet (one node of lines), or a bark
opens approach <metres>         a greet that opens the talk screen by itself (Who opens a conversation)
prio quest|ask|world|reaction|flavour
once | daily                    per character, per speaker
always                          shows even when the speaker refuses (the finale's route topics)
voice haven|bastion|redhand|halcyon|table|plain

scene <id> speaker|season|house|run [who <conditions>]   deciding together (Several players): one record per speaker,
                                for the season, per safehouse, or per mission run; who may decide
bind <name> = <picker>          a place picked once when the topic opens: town:nearest, town:thick_within(<m>),
                                house:nearest, house:nearest_short
use <conv id>                   that conversation's nodes (but its start) are this one's too, copied at pack time
node <id> [public|shared|confirm]   the first node is "start"; public (or shared, a scene's node the room
                                sees): others within 8 m overhear it; confirm: [Point of no return]
  jump <conditions> -> <node>   checked in order on entry
  say [(<role>)] <text> [~lie:<key> | ~truth:<key>]   one to three lines; 200 characters each; a lie told or
                                the truth; (her father) is someone else in the scene, shown "Her father: ..."
  say? <conditions> :: <text>   the first true variant replaces the next plain say
  do <effects>                  on entry
  > @<id> <label> -> <pass>[ | <short>][ | <refused>] [check <kind> <faction> <difficulty> [easier:<trait>] [harder:<trait>] [bg:<ids>]]
      [cost favor:<faction> N | cost item:food*N] [if <conditions>] [do <effects>] [short <effects>] [else <effects>]
      [truth <conditions>] [tell always] [found <effects>] [callout <lie key>] [decides] [dry]
```

A lie's `tell always` makes it always caught (a joke lie: *"Rudi sent me."*); its `found` effects run if
it's found out later, only on what's still running (`nset`, `cset`, `mark`, `unsway`: a tally's sway
taken back). `decides` is `decide:<pass target>`. In a key, `@<name>` binds it to a binding as `@here`
binds it to the speaker, and `watch:<key>@<name>:<infection>` is a **world predicate**: the truth of a lie
about the bound town is learned the moment you walk into it while its infection is at least that.

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
stranger's `{their:<key>}`, `{wire:1}` to `{wire:3}` (today's offered words at Bastion's wire), and:
`{tier:<faction>}`, `{rep:<faction>}`, `{word:<faction>}` (your standing, name and word with another
faction), `{resident}` (a house's seeded surname, never one from the lore), `{decider:<major>}` (the
first who decided it, under the names policy), `{lean:<major>.<vote>}` (the speaker's own lean at a
sitting), `{kit_owner}` and `{dead_name}` (whose the kit was), `{cover}` (your best cover detail),
`{pack}` and `{pack_leader}` (the pack near the speaker and its leader, in the speaker's faction's
word), `{distress}` (where a distress caller is), `{bind:<name>}` and `{town:@<name>}` (a binding's
place), `{claim:<key>}` (what you told this speaker for that claim, in your own words).

**The pipeline.** `tools/dialogue_pack.py` lints every file and writes them as generated Enforce
Script, `scripts/Game/ReforgerZ/Dialogue/Pack/RZ_DialoguePack_<area>.c` (functions of at most 150
calls) and `RZ_DialoguePacks.c`. The server compiles them once at start into typed records indexed
by speaker kind. The packs are committed; never edit them by hand. They're the one text source, so a
text fix needs a mod update (see [Simplified](#simplified-and-not-built-yet) for reading `.rzd` files
from the addon). A lie's mark and a choice's `truth` and `callout` go into
the packs as `Mark` and `ChoiceExtra` calls, `tell always` and `found` as `ChoiceMore`, bindings as
`Bind`.

**Shipped ids.** `dialogue/shipped.lock` lists every conversation and choice id that has shipped.
The lint refuses removing one; a gone id is kept as `retired <id>`.

## Tools

| Tool | Does |
|---|---|
| `python3 tools/dialogue_pack.py` | Lint and write the packs. `--check` fails if the packs or the lock are out of date (the compile check runs it first); `--lock` adds new ids to the lock; `--list` lists the conversations |
| `python3 tools/dialogue_sim.py <conv> --bg nurse --tier haven=trusted` | Walks one conversation as one character sees it: the lines (variants picked), every choice with its tag, whether it shows, and where a check would send them. Also `--infamy`, `--fame`, `--favor`, `--flag`, `--knows`, `--fact night` (any other true term: `--fact heard_lie:scav.chemist`, `--fact kit:in_kit`), `--mem helped`, `--regard`, `--food`, `--house short`, `--trait`; `--all` shows every start node. A look's score needs the game, so it shows only where a pass and a catch lead. **Several characters:** `--chars "Mara:--tier haven=known" --chars "Tom:--bg nurse"` shows each one's start node, and `--race` how the scene settles (who holds, who may object or stand with, what lands once, each sway) |
| `python3 tools/voice_lint.py` | Also reads every `.rzd` line (says as talk-screen lines, labels as choices) against [WRITING.md](WRITING.md#checking-text) |
| **[Admin] Errands near here** (server admins, under **[Admin]** on any named person, wanderer or occupant: no typing) | The errands' live count, the nearest player's last deal and why each other errand wasn't dealt, and their errands ([ERRANDS.md](ERRANDS.md#telemetry-and-the-admin-view)) |
| `/talkdebug` (admins, in chat; or the **[Admin] Read this person's conversations** reply) | Whether Trees are on, the conversation and memory counts, your memory, and for the person you're talking to, every conversation for them and the first `when` term that fails |
| `/talkdebug test` (or **[Admin] Test every conversation, to the log**) | Every conversation's conditions, lines and labels run once for you through the real evaluator and tokens, and a lie, a promise and a callback on test keys; the counts back to you and the log (`Dialogue: Talk test: ...`). The runtime test runs it for its fake player (`RZT Talk test: ...`) |
| `/talkdebug reset <scene>` (or **[Admin] Undecide this person's scenes**) | A scene's record cleared, so whoever answers next decides it again |

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
a `dry` choice with no plain twin; a lie's `truth` that reads what the player can't know (another
system's flags, a house, a town); `tell always` or `found` off a [Lie], and a `found` effect on
something that may be over; a scene-only effect (`set`, `qset`, `begin`, `decide`, `sway`) outside a
scene; `set` without `@here` or a binding in a conversation for many speakers, or outside the
conversation's own namespace; Halcyon trees of four nodes or more that never say a number; a binding,
`watch` or token naming a binding the conversation doesn't have; `opens` on anything but a greet, over
40 m, or on the radio; `public` on a secure net. It **warns** for a [Lie] with neither `truth` nor `tell
always` (an ordinary lie, believable and never true).

## The conversations

✅ 157 conversations, 346 nodes (`python3 tools/dialogue_pack.py --list`), 17 of them barks.

| Speaker | Conversations |
|---|---|
| Haven leaders | The bear on the door; the names on the list (persuade: who crosses them out; [Elder], [Nurse] +1); who they read out at supper |
| Bastion leaders | The board on the wall (the roster: nobody is coming, they're all they have; [Soldier], [Police]; being signed on); whether Command ever answers |
| Red Hand leaders | The book (the first line, Rudi, persuade to see your own page, which reads it out in the book's voice); why the hand is red |
| Halcyon leaders | Why they're called the beaks ([Lab Technician], [Doctor], [Nurse]: the notebooks); how close the cure is (the other two); a field lead on the volunteers ("They're comfortable": a lie, with [Lab Technician], [Doctor], a persuade and an [Evidence] call-out to the room at the back) |
| Every leader | The name, not the face (a new life greeted by the season's title, once); a lie found out, said at the next greeting, once; what a brief left out, told or kept (a giver's lie you found, at any leader of that faction for a week: `leaders/briefs.rzd`, [DESIGN.md](DESIGN.md#story-blocks)) |
| Haven leaders at a sitting | Every Name's first and second sittings: a persuade to sway the speaker's vote to an option on the table (one sway a sitting) |
| The 8 wanderer roles | The Trader's prices; the Doctor's work ([Doctor], [Nurse]); where the Scavenger has been (persuade: is it true); why the Deserter left ([Soldier], [Police]); the Preacher's bells; the Courier's letters; what still runs ([Mechanic]); the Kid's name; the Scavenger's clean chemist's (a lie for a tin, [Hunter] or [Police] see through it, [Evidence] once the Doctor tells you, and a callback greet 6 hours on); the Doctor on the chemist's and on Halcyon's back room (the truths); what Bastion makes of you, as the Deserter reads it |
| Occupants | A fed house (leave a tin: 2 Haven Favor); the candle in the window after a death (a greet, and a topic: staying for the name is remembered 6 hours on); the bear on the door; who gets the bed by the stove (a short house: a public scene per house that someone Known with the Haven decides, with a sway for the room); a short house's promise of two tins (kept by the hand-in, with greets for kept and broken) |
| Named people | The Red Hand toll man (who painted the hand; Rudi, said flatly); the Bastion sentry (the roll; a promise to walk the road and come back, kept by coming back); kit greets at Bastion, Red Hand and Halcyon holdings (own colours, their kit on a stranger, issued kit worn after your standing slipped, and at Bastion's what the kit remembers: a dead man's, off the turned, signed out to someone else); Halcyon's "Say you're with the lab." (a look, a notebook number, or the truth) |
| Post speakers | Bastion's search (persuade with [Soldier], [Police]: the pole goes up); the Red Hand's toll (pay it, or threaten: Fear, +3 Red Hand Infamy); "Which unit?" and "Say you're crew." in their kit (a look, or the truth); "Word?" at Bastion's (three words, one right); the greet at 35 m that reads the kit's marks; a name who'll stand for you at Bastion's and what the crew calls you at the Red Hand's (a cover detail, then a look), your own crew name at the Red Hand's, and a notebook number to Halcyon's people (`posts/cover.rzd`, `posts/opens.rzd`) |
| The Long Table | The host on the road (speaking first at 12 m; where you're headed, and is anyone expecting you; "Put out, were you?" to an exile); the mother at the farm (is the boy hers; [Bitten]: declare it, as built) |
| Strangers | Calling out at 10 m; where any stranger's headed; and for 15 routines, who they are: the girl at the tin man's house, the wife who drove to market, the pram of pans, the one shut in since Tuesday, the kids in the back seat, who the grave is for, the sleeve rolled down ([Doctor], [Nurse]), the farm, the sister in a barn, the husband across the line, the debt in the book, the one who hid, the one who got there first, the scratch off a fence |
| The radio | Call in what you've seen (a rumour, on air); listen for anyone calling (by the night and the mood); ask the crews about the toll; answer a distress call; on the secure nets, Bastion's command net (the word), the boss's channel (the toll), the lab's channel (the count) |
| Make it right | The [Make it right] reply at any leader who won't deal with you (not a conversation: the engine's own) |
| The last week | Every faction's leaders: "Ask about the cure." and "Ask what they'll do now."; an occupant's greet for the week and a topic for Bastion's route; barks at houses and holdings |
| The bitten and the clock | Tell a safehouse about your bite (or lie about it); Halcyon's trial dose; the Doctor on a fever; a Nurse on a cough at a short house; shift change, supper and grace, the night watch (barks) |
| Kept back | The Red Hand crew boss on a crossed-out line (deflect, then persuade [Ex-Con] or threaten); an occupant on the pack up the hill; Bastion's and the Red Hand's leaders on the Haven's sitting while Every Name's runs |
| Lies the world tests | The Trader's quiet town (bound to the nearest thick one: walk in and see; then [Evidence]); "Rudi sent me." at the Red Hand's post (always caught); a bite called a fence scratch, remembered at the house when it's found out; the Long Table's "expected back by dark" |
| Barks | A short house (and its tins, promised or brought), a house after a death, a house at dusk, a Red Hand crew name and the toll man, Bastion's shift change and a set word, Halcyon's samples, the lost Kid, the Scavenger once the callback is due, the sentry once the road is walked |
| Asks | A short house (two tins into the stores: the built hand-in); the Kid, lost (show the way: the built escort); a bitten wanderer (your antidote: the built one) |
| The finale | "Ask about the cure.": ask me again (every leader, 75%); the pitches (the four stronghold leaders and the Doctor or Courier, 80%); the commit topics with their [Point of no return] nodes (stronghold and Loyal seat leaders of each faction; the Doctor or Courier, or the open channel, for the Independent) |

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
  `Dialogue: session cap at <speaker>`, `Dialogue: <player> committed to <route> at <speaker>`,
  `Dialogue: make it right <player> at <speaker>`, `Dialogue: made right <player> with <speaker>`,
  `Dialogue: <speaker> opened on <player> at N m`, `Dialogue: distress call answered by <player>, sent to
  house N`
  ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check)).

## Simplified, and not built yet

- *Simplified:* trees run only on the talk screen: with the talk screen off, there are no trees.
- *Simplified:* greets are lines only (a greet that opens by itself puts its question as its line, and
  the topics under it are the choices); "dimmed, not hidden" topics show grey, or a "heard" tag while
  there's more in them.
- *Simplified:* a built reply is grey only for the rest of the conversation you picked it in (the
  engine keeps no memory of built replies), whether or not its answer would change.
- *Simplified:* a stage's or a contract's people keep their built choices as verbs; a conversation at
  them picks one with `call:slot:N`.
- *Simplified:* a claim the world tests isn't saved: a restart forgets a test still waiting.
- *Simplified:* a binding lasts the session (it's picked again when the topic opens next), and a world
  predicate waits 3 real hours and isn't saved.
- *Simplified (waits for an engine check):* the generated packs are the one text source. Reading the
  `.rzd` files from the addon at start, with server-only `$profile:ReforgerZ/dialogue/*.rzd` overrides by
  conversation id (a text fix with no mod update, and twists kept off clients' disks), needs `FileIO` to
  open files inside the addon on the dedicated server (VERIFY, on the PC's list in `.claude/HANDOFF.md`).
  If it can, the reader is the packer's parser ported, with the overrides on top; if it can't, only the
  `$profile` overrides would be worth a runtime reader.
