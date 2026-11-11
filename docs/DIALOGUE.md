# Dialogue

How talking to people works: the talk screen and its engine, the conversation trees on top of it
(**Dialogue > Trees**, off by default), what people remember, the checks shown before you choose,
the `.rzd` text format, its lint and simulator, and the conversations that ship. How the screen
looks and is driven with a pad is in [UI.md](UI.md#the-talk-screen); how lines are written is in
[WRITING.md](WRITING.md#conversations). What's still to build (promises, strangers' and radio
speakers' conversations, barks, claim lines in someone else's kit, the finale's commit) is designed in
[designs/DIALOGUE.md](../designs/DIALOGUE.md).

Code: `Dialogue/RZ_Dialogue.c` (sessions), `Dialogue/RZ_TalkSpeakers.c` (who someone is),
`Dialogue/RZ_DialogueTrees.c` (hubs, nodes, checks, effects), `Dialogue/RZ_DialogueDB.c` (the
compiled conversations), `Dialogue/RZ_DialogueCond.c` (conditions and tokens),
`Dialogue/RZ_DialogueMemory.c` (memory and `dialogue.json`), `Dialogue/RZ_DialogueScenes.c` (several
players), `Dialogue/Pack/` (generated). Server only, except the screen.

## Contents

- [The talk screen](#the-talk-screen)
- [Trees: hubs, topics and nodes](#trees-hubs-topics-and-nodes)
- [Who talks](#who-talks)
- [Checks and tags](#checks-and-tags)
- [What a choice does](#what-a-choice-does)
- [Memory](#memory)
- [Several players](#several-players)
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
safehouse occupants, named people at holdings, the wanderers, the Long Table's people, the people
missions and questlines spawn, and (with Trees on) a road post's speaker at the barrier. Any living
character with the puzzle components. Bodies, objects, spots and doors keep their actions.

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

**Safety.** At most 32 conversations (and readers) on the server; a pick is checked for the
current serial, a reply id that was sent and enabled, a living player in range and not in danger,
and at most one every 0.3 s; Talk at most every 2 s. One node at a time goes to that player only
(the owner-only `PAYLOAD_TALK`). Sessions aren't saved.

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

Factions are written `haven`, `bastion`, `redhand`, `halcyon`, and `none` for factionless checks.
Mission and questline people have no trees yet (their built choices stay).

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
| `lie <faction> <difficulty>` | [Lie - 1 of 2], or [Lie - they'd see through it] | A **tell** catches it first, whatever the score: the bite showing, another faction's colours on you, or something else you already told this speaker. Without one, Trust against the difficulty (+1 under a vouch). Believed: a false claim in the [lie store](DESIGN.md#lies-and-witnesses), found out later by its rumour rounds (the speaker's regard *lied*, +2 Infamy). Caught now: regard *lied*, +2 Infamy and the liar count (−1 at Trust this life). The reply opens with "They believe you. For now." or "They don't believe a word of it." |
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

There is **no Fame, no Infamy and no item** from talk, ever: the lint refuses `fame`, `infamy` and
`house` effects, and Infamy comes only from a tagged threat or a caught lie. Effects that change the
world for everyone go through a scene's `decide` or `sway`; mission outcomes from talk wait for
missions to take conversations.

## Memory

`RZ_DialogueMemory`, saved in `$profile:ReforgerZ/dialogue.json`.

| What | Kept for | Scope | How long |
|---|---|---|---|
| **Regard** | each speaker key | player (identity) | −4 to +4; one step toward 0 each real day |
| Failed persuades (the score) | each speaker, conversation and choice | player | until your score is higher |
| Knowledge (`learn:`, `knows:`) | each key | player | the season (96 at most; the oldest goes) |
| Talk Favor today | each faction | player | the real day |
| Face (they know you) | each faction | character | the life. Set when you talk to their people |
| Claims | each speaker | character | the life, 6 a speaker |
| Seen nodes, passed persuades, once and daily topics, character flags, speaker notes | per key | character | the life |
| Local deeds ("You're the one who...") | each seat | character | the life ([REPUTATION.md](REPUTATION.md#what-people-call-you)) |

A new life is a new face: everything kept per character is cleared when the profile's life count
changes.

**Regard words:** *thanks* +1 (only with a real cost or deed), *kept* +1, *broke* −1,
*threatened* −1, *slander* −1, *vouched_liar* −1, *lied* −3 (a Merciful speaker −2, a Paranoid one
−4). *kept* and *broke* come with promises; the rest are written today (a caught or found-out lie,
a false call-out or point-out, a vouch for a liar).

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
- **Tallies.** `sway:<name>` adds one contribution per player to `tally.<name>`; the owning system
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

*Simplified:* the social verbs are on your own hub with that speaker (open Talk while they talk), and
"eligible to object" is anyone talking to the speaker.

## Conditions

Terms are joined with ` & `; `!` negates one; commas in an argument mean "any of". There is no OR:
write two choices. Every key reads a system that's already there; keys of designs not built yet
(tribes, the majors, the finale, disguise, errands, promises, lies) read false, and `route:none`
reads true.

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
| Scenes | `decided`, `decided:all_in`, `swayed:council.workers` | this conversation's scene is decided (any, or that value); you've swayed that tally |

**Only what the player knows** goes into a tag: background, standing, items, the HUD, their own
memory. Nothing reads whether a cleaned bite took.

## The text format (.rzd)

Conversations are text files in `dialogue/<area>/<name>.rzd` (areas today: `leaders`,
`wanderers`, `residents`, `posts`, `table`). A file holds one or more conversations. A line whose
first character is `#` is a comment. Plain ASCII only (straight quotes).

```
conv <id>                       unique, dotted, lower case; never renamed once shipped
for <speaker>[, <speaker>]      who says it (Who talks)
when <conditions>               read when the hub is built and on every pick
topic "<hub label>" | greet     a topic (40 characters at most), or a greet (one node of lines)
prio quest|ask|world|reaction|flavour
once | daily                    per character, per speaker
voice haven|bastion|redhand|halcyon|table|plain

scene <id> speaker|season [who <conditions>]   deciding together (Several players): one record per speaker, or one for the season; who may decide
node <id> [public]              the first node is "start"; public: others within 8 m overhear it
  jump <conditions> -> <node>   checked in order on entry
  say <text>                    one to three lines; 200 characters each
  say? <conditions> :: <text>   the first true variant replaces the next plain say
  do <effects>                  on entry
  > @<id> <label> -> <pass>[ | <short>][ | <refused>] [check <kind> <faction> <difficulty> [easier:<trait>] [harder:<trait>] [bg:<ids>]]
      [cost favor:<faction> N | cost item:food*N] [if <conditions>] [do <effects>] [short <effects>] [else <effects>]
```

A choice's label is everything between `@id` and ` -> `: speech in quotes, actions as plain words,
40 characters or fewer. Choice ids are unique in a conversation, and memory is keyed by
conversation, node and choice id, never by position.

**Tokens:** the world's (`{day}`, `{cure_percent}`, `{town:N}`...: [WRITING.md](WRITING.md#template-rules)),
plus `{npc}`, `{npc_short}`, `{role}`, `{here}` (the speaker's town or holding), `{house}`,
`{faction}`, `{player}`, `{boss}` (the speaker faction's stronghold leader), `{toll}` (your Red Hand
toll now), and reputation's `{rep}`, `{word}`, `{best}`, `{worst}`, `{kills}`, `{jobs}`, `{picture}`,
`{name}`, `{crew}` for the speaker's faction.

**The pipeline.** `tools/dialogue_pack.py` lints every file and writes them as generated Enforce
Script, `scripts/Game/ReforgerZ/Dialogue/Pack/RZ_DialoguePack_<area>.c` (functions of at most 150
calls) and `RZ_DialoguePacks.c`. The server compiles them once at start into typed records indexed
by speaker kind. The packs are committed; never edit them by hand. *Simplified:* the design's
"plan A" (the server reading `.rzd` files from the addon) and server-only `$profile` overrides
aren't built: the packs are the one text source, so a text fix needs a mod update.

**Shipped ids.** `dialogue/shipped.lock` lists every conversation and choice id that has shipped.
The lint refuses removing one; a gone id is kept as `retired <id>`.

## Tools

| Tool | Does |
|---|---|
| `python3 tools/dialogue_pack.py` | Lint and write the packs. `--check` fails if the packs or the lock are out of date (the compile check runs it first); `--lock` adds new ids to the lock; `--list` lists the conversations |
| `python3 tools/dialogue_sim.py <conv> --bg nurse --tier haven=trusted` | Walks one conversation as one character sees it: the lines (variants picked), every choice with its tag, whether it shows, and where a check would send them. Also `--infamy`, `--fame`, `--favor`, `--flag`, `--knows`, `--fact night`, `--mem helped`, `--regard`, `--food`, `--house short`, `--trait`; `--all` shows every start node |
| `python3 tools/voice_lint.py` | Also reads every `.rzd` line (says as talk-screen lines, labels as choices) against [WRITING.md](WRITING.md#checking-text) |
| **[Admin] Errands near here** (server admins, a reply on any named person, wanderer or occupant: no typing) | The errands' live count, the nearest player's last deal and why each other errand wasn't dealt, and their errands ([ERRANDS.md](ERRANDS.md#telemetry-and-the-admin-view)) |
| `/talkdebug` (admins, in chat) | Whether Trees are on, the conversation and memory counts, your memory, and for the person you're talking to, every conversation for them and the first `when` term that fails |

**The lint** (`dialogue_pack.py`): unique ids; known keywords, speakers, condition keys,
effects, checks, calls and tokens; targets that exist; nodes no path reaches; at most 6 choices;
labels 60 and topics 40 characters; says 200; a check fails into `end` or into a node with no
choices; a threat at a stronghold leader, the Kid, a safehouse or the Long Table; `regard:thanks`
without a cost or deed; Favor over 10 in one conversation; Fame, Infamy, house and world effects;
`offer` off a leader; an ask (`prio ask`) whose `when` doesn't read the world; a greet with choices;
the Long Table: its voice only for its people, its words never described (a banned list), its
names never outside its files, and nothing elsewhere hinting at it; "alpha", "tribe" or "crawler";
a pronoun after `{npc}`; `!` outside the Red Hand and Bastion (a warning for Bastion).

## The conversations

✅ 31 conversations, 100 nodes (`python3 tools/dialogue_pack.py --list`).

| Speaker | Conversations |
|---|---|
| Haven leaders | The bear on the door; the names on the list (persuade: who crosses them out; [Elder], [Nurse] +1); who they read out at supper |
| Bastion leaders | The board on the wall (the roster: nobody is coming, they're all they have; [Soldier], [Police]; being signed on); whether Command ever answers |
| Red Hand leaders | The book (the first line, Rudi, persuade to see your own page, which reads it out in the book's voice); why the hand is red |
| Halcyon leaders | Why they're called the beaks ([Lab Technician], [Doctor], [Nurse]: the notebooks); how close the cure is (the other two) |
| The 8 wanderer roles | The Trader's prices; the Doctor's work ([Doctor], [Nurse]); where the Scavenger has been (persuade: is it true); why the Deserter left ([Soldier], [Police]); the Preacher's bells; the Courier's letters; what still runs ([Mechanic]); the Kid's name |
| Occupants | A fed house (leave a tin: 2 Haven Favor); the candle in the window after a death (a greet, and a topic); the bear on the door; who gets the bed by the stove (a short house: a public scene per house that someone Known with the Haven decides, with a sway for the room) |
| Named people | The Red Hand toll man (who painted the hand; Rudi, said flatly); the Bastion sentry (the roll) |
| Post speakers | Bastion's search (persuade with [Soldier], [Police]: the pole goes up); the Red Hand's toll (pay it, or threaten: Fear, +3 Red Hand Infamy) |
| The Long Table | The host on the road (where you're headed, and is anyone expecting you); the mother at the farm (is the boy hers; [Bitten]: declare it, as built) |
| Asks | A short house (two tins into the stores: the built hand-in); the Kid, lost (show the way: the built escort); a bitten wanderer (your antidote: the built one) |

## Saving and performance

- **Cost.** A hub is built only when someone talks, from the conversations indexed by speaker
  kind; conditions are compiled once at start. Picks are event-driven. The post speaker is one
  body per post, only within 100 m of a player.
- **Network.** One owner-only record per node, as the talk screen sends today.
- **Saving.** `$profile:ReforgerZ/dialogue.json`, written at the 2-minute save only when something
  changed; dropped when the season number changes (a wipe). `state.json` doesn't grow.
- **Log lines:** `Dialogue: loaded N conversations, M nodes`; `Dialogue: trees on|off`;
  `Dialogue: memory for N players`; `Dialogue: memory from season N dropped (season M)`;
  `Dialogue: budget reached <player> <faction>`; `Dialogue: debug: ...`; `Dialogue: scene <key> decided
  <value> by <player>`, `Dialogue: scene <key> hold by <player> objected to by <player>`, `Dialogue: sway <name>
  by <player> (now N)`, `Dialogue: <player> vouched for <player>` ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check)).

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
- **Planned** ([designs/DIALOGUE.md](../designs/DIALOGUE.md)): lies the player is *told*
  (`~lie` lines, `heard_lie`, the `[Evidence]` call-out); promises and callbacks through `later:`;
  claims in someone else's kit as conversation lines (the look itself is built); Make it right;
  strangers', radio speakers' conversations and barks; the finale's commit and the epilogue's topics.
  One shipped conversation uses a scene (the bed by the stove); none uses lies yet.
