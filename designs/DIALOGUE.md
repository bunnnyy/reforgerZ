# Dialogue

**Status: designed; most of it built.** Approved by the author on 2026-11-11. Built and living in
[docs/DIALOGUE.md](../docs/DIALOGUE.md): phase 2 (the trees on the talk screen behind **Dialogue > Trees**,
hubs and topics, checks shown before you choose, regard, claims and knowledge, `dialogue.json`, the `.rzd`
format with its lint, simulator and `shipped.lock`, a post's speaker at the barrier); phase 4's multiplayer
layer and the player's lies; and talk's rest from phases 3 to 8 (lies you're told, promises and callbacks,
claims in someone else's kit as lines, barks, strangers and the open channel as speakers, the majors'
sittings swayed by talk, the per-speaker cap); and the finale's commit nodes, `always` route topics and
pitches. This file keeps only what's still to build.

A server-side conversation system for everyone a player can talk to: stronghold and seat leaders,
quartermasters, the eight wanderer roles, safehouse occupants, named people, road-post crews (who get a
speaker at the barrier), quest people and the Long Table. It also gives a voice to three new kinds:
**strangers** (people who need help, dealt by [ERRANDS.md](ERRANDS.md)), **radio** calls with no body,
and one-line **barks**, which carry most of the in-game ending.

Dialogue is the delivery layer for almost everything in the New Vegas direction: mission scenes, errand
asks, reputation's parley and knock, disguise claims, the finale's commit and the epilogue's topics are
all conversations in this one engine. Nobody builds a second talk system.

## What changed in review

- **Off by default** (decision 13). **Dialogue > On** stays false until the phase 0 per-player label test
  and a play-test both pass. Until then every built action and line runs as it does now, and every
  sway, ask and persuade also works from the action menu.
- **One persuade score** (decision 11, the plan's section 5). Dialogue's score is the one Trust check for
  every piece (`RZ_Checks`): your access rank with the faction, mixed tiers one lower, +1 for a listed
  background, at most +1 from dialogue, −1 for the liar count, −1 for crossing lines. Missions' Pass /
  Short / Refused results sit on top of it. The built `PersuadeScore` is untouched.
- **One threat score.** Dialogue's own `threaten` score (Infamy band, +1 armed) is gone. Every threat
  uses the shared Fear score ([MISSION_STYLE.md](MISSION_STYLE.md#checks-persuade-and-fear) owns it;
  [REPUTATION_NAMES.md](REPUTATION_NAMES.md#the-fear-check-one-threat-score) supplies its numbers):
  +3 Infamy, once per person by anyone for the season, three per life per faction, never on stronghold
  leaders, never free-standing in safe zones or safehouses.
- **One look for disguises.** A claim to be one of them is judged by the fixed look in
  [REPUTATION.md](../docs/REPUTATION.md#borrowed-kit), shown before you commit. Dialogue's random challenge odds, its
  suspicion counter, its blown-for-15-minutes and its doubled caught-lie cost are gone. The word at the
  wire, a real name, a crew name and a notebook number are **cover details** (+1 at the look; a wrong word
  fails at once). Kit kinds follow [REPUTATION.md](../docs/REPUTATION.md#borrowed-kit): own, issued, in their kit, signed for, a dead man's, off
  the turned. Point-out is now a −1 at the look.
- **One vouch, one lie store, one callback queue.** The vouch has one liability everywhere. Lies are
  found out by events first; the two 15% discovery rolls become the **rumour tick**, only for claims with
  nothing in the world to test. Echoes, follow-ups and `later:` are one queue (words only, 3 season days).
- **Memory is life-stamped.** Earlier lives count at half and are voiced once as the name, as
  [REPUTATION_NAMES.md](REPUTATION_NAMES.md#12-face-and-name-one-scope-rule) sets out. Errands write regard only on
  named-person keys, never on leaders.
- **Deciding together** uses missions' 5-second hold, one objection per player per job, and Stand with.
  "First pick wins" and the 10-second window option are gone. Who took part: 60 s within 150 m, or a step
  done. Who hears: overheard lines at 8 m; scene results and holds to the room at 30 m.
- **Asks and strangers are errands.** Dialogue is how an ask is spoken; [ERRANDS.md](ERRANDS.md) owns the
  dealer, the catalogue, the pay and the interruption budget. The seeds table moved there. Leaders never
  give errands: a leader says who to see.
- **Strangers only where one would make sense** (decision 14, changed by the author). A stranger finds you
  only at a road people use, a wanderer route, near a safehouse or outpost, a campfire, a stopped car, near
  a fresh event or a body, or a town edge at dusk. Never in the empty middle of nowhere, never on a timer.
  The rate is a ceiling only ([docs/ERRANDS.md](../docs/ERRANDS.md#strangers)).
- **The commit nodes** (conflicts 27 and 28). Bastion's says "From now until the cure" and shows the
  finale's three lines (the admission, what you bring, what you leave). The Independent's is said on air
  without a name: *"This is nobody. The cure goes to everyone."* Every faction hears that somebody said it.
- **Old lore fixed.** The toll man's Rudi line follows LORE: Rudi was put out at dusk on Day 7, and the
  crewman tells the official lie flatly (conflict 29). The epilogue occupant's "Mrs Halas" is a seeded
  name now, because Halas was the Long Table's first meal (conflict 30). The stranger "Tomas" is renamed,
  because Tomas is the name in the Long Table's grace; "Alena" and "Ivo" are renamed, because they're
  Bastion's dead and its signaller in LORE.
- **Tribes replace Grounds** (conflicts 42 and 43). `ground:*` keys and `{ground:N}`, `{first:N}` become
  `tribe:*`, `{tribe:N}` and `{alpha:N}` (in each faction's own word). The scavenger's lie and the hiding
  stranger are re-angled to towns of crawlers; the bark reads a tribe's leader calling at dusk.
- **The finale's edges.** `CanCommit` reads `CanSideWith`; the lock is per identity for the season and ends
  at the cure; late joiners can commit until the stand is won; `RouteOf(identity)` everywhere
  (conflict 39). See [FINALE.md](../docs/FINALE.md#the-lock).
- **The epilogue lasts one week** (decision 12). Dialogue delivers the closure set's scenes and topics
  ([The last week](#18-finale-and-the-last-week)).
- **One owner-only payload for all personal text** and **one names policy** (the plan's section 5).
- **Docs** (decision 16): `docs/DIALOGUE.md` is added to the repo only when dialogue is built. The earlier
  revision's "how this answers the review" table is gone: its fixes are in the design below.

**Contents**
- [1. What it is](#1-what-it-is)
- [2. Words](#2-words)
- [3. Who talks](#3-who-talks)
- [4. How a session runs](#4-how-a-session-runs)
- [5. The hub](#5-the-hub)
- [6. Memory](#6-memory-what-is-kept-where-and-for-how-long)
- [7. Conditions](#7-conditions)
- [8. Checks and tags](#8-checks-and-tags-new-vegas-style)
- [9. Results (effects)](#9-results-effects)
- [10. Lying and being lied to](#10-lying-and-being-lied-to)
- [11. Promises, callbacks and knowledge](#11-promises-callbacks-and-knowledge)
- [12. Claims in someone else's kit](#12-claims-in-someone-elses-kit)
- [13. Infection, time and items](#13-infection-time-and-items)
- [14. Multiplayer](#14-multiplayer)
- [15. Delivery](#15-delivery)
- [16. Radio](#16-radio)
- [17. Asks: errands, spoken](#17-asks-errands-spoken)
- [18. Finale and the last week](#18-finale-and-the-last-week)
- [19. Authoring format, pipeline and tools](#19-authoring-format-rzd-pipeline-and-tools)
- [20. Fitting the existing systems](#20-fitting-the-existing-systems)
- [21. Performance and saving](#21-performance-and-saving)
- [22. Phases and the first content](#22-phases-and-the-first-content)
- [23. Docs in the same commits](#23-docs-in-the-same-commits)
- [Examples](#examples)
- [Engine needs](#engine-needs)
- [Settings](#settings)
- [Options decided](#options-decided)
- [Risks](#risks)

---

## 1. What it is

**Built:** the engine, hubs, nodes, checks and effects, memory, the guardrails on pay (no Fame, small Favor
inside one daily budget, no items), Trees off as today's game, and on it lies you're told, promises and
callbacks, claims in kit, the multiplayer layer, barks, strangers and the open channel
([docs/DIALOGUE.md](../docs/DIALOGUE.md)).

Still to build, on that engine: the rest of lies and of promises (sections 10 and 11), asks dealt as
errands (17), the finale's and the last week's conversations (18), and the pieces each section lists.

**Guardrails the rest keeps:** no companions (the lint refuses follow effects and escorts without a
destination; escorts stay mission or errand blocks ending at a place, at most 600 m for errands); no stronghold
leader moved, killed, threatened or replaced by talk; a disguise never unlocks a reward.

## 2. Words

| Word | Means |
|---|---|
| Conversation | One unit in a file: an id, a speaker selector, a `when`, a topic label (or greet or bark) and nodes. Ids are dotted (`haven.head.mouths`) and never renamed once shipped |
| Hub | What Talk opens: the greeting, the verbs, up to 4 topics a page, More..., Leave |
| Verb | A built behaviour every speaker of that kind offers (Ask for work, Rob). Verbs don't count toward the topic limit |
| Topic | A conversation's entry in a hub ("Ask about the family at the gate.") |
| Greet | A conversation that replaces the greeting while its `when` holds |
| Bark | One line said to a player passing by. No choices |
| Node | The speaker's lines (1–3), entry effects, and choices (up to 7, Leave included) |
| Choice | A player option with a stable id (`@all`), a label and a target |
| Check | What a choice tests before it goes to its pass, short or refused node |
| Tag | The bracketed label in front of a choice. The packer generates it from the check or condition. It shows costs and requirements, never outcomes |
| Scene | Shared nodes with one decision for the world ([section 14](#14-multiplayer)) |
| Speaker | Whoever is talked to, reached through an adapter that knows their faction, traits, seat, place and memory key |
| Regard | The small, decaying score a speaker keeps for a player ([section 6](#6-memory-what-is-kept-where-and-for-how-long)) |
| Claim | Something this character told this speaker, true or false, kept in the one lie store |
| Knowledge | What a player has learned (`knows:` keys) |
| Binding | A value picked once in a session (a town, a house) and kept with claims, promises and callbacks |
| Face | Recognition: this faction's people know you. ("Locals" are errands' remembered people, a different thing) |
| Cover detail | Something real you can say at a disguise look: the word at the wire, a real name, a crew name, a notebook number, a mission's paper ([REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)) |

## 3. Who talks

**Built:** leaders (stronghold and seat), wanderers, occupants, named people at holdings, a road post's
speaker at the barrier, and the Long Table's people, with their memory keys and the built refusals first
([docs/DIALOGUE.md](../docs/DIALOGUE.md#who-talks)).

Still to build:

| Kind (`for`) | Who | Memory key | Default hub |
|---|---|---|---|
| `quartermaster:<faction>` | Shopkeepers away from a seat | the holding | What's short?; requisition talk. Shop and services stay their own actions |
| `quest:<key>.<role>` | People a stage or contract spawns (`SpawnPerson` with a conversation id) | `quest:<key>:<role>:<run>` | Whatever the stage writes |
| `radio:<channel>` for the secure nets | No body: a cracked secure net, for a Radio Operator | `radio:<channel>` | ([section 16](#16-radio)) |

Also still to build: a
refusing leader offers **Make it right** ([section 6](#make-it-right)). **Claimed speakers:** a leader,
wanderer or house that a running story holds shows that story's topics first, and other conversations can't
`offer`, `begin` or `start` anything that touches the claimed subject that turn ("Not now. I've got enough on.").

## 4. How a session runs

**Built:** Talk within range, picks checked against the serial and the live world, the fresh-node guard on
priced choices, `end`, `hub` and `back`, the ways a session ends, danger, and the rates
([docs/DIALOGUE.md](../docs/DIALOGUE.md#the-talk-screen)).

Still to build:
- **Server-opened greets:** a post speaker at 35 m for a player in that faction's kit below Known (so a claim is
  offered whether or not they press Talk), a stranger at 10 m, the host on the road.

## 5. The hub

**Built:** the greeting, the verbs (the built replies), topics by priority with paging, once and daily,
selectors and trait variants, and leaders who name who to see instead of giving errands
([docs/DIALOGUE.md](../docs/DIALOGUE.md#trees-hubs-topics-and-nodes)).

Still to build: **injected topics** from the systems that own them (questlines, the majors, tribes, needs and
morale, truces, lost outposts, the cure, the finale, the epilogue) by setting the flags and facts `when` reads,
for example the majors' "Ask {chair_short} about Day 7" on a stronghold leader while that act is open; quest
topics that never drop but page onto More....

## 6. Memory: what is kept, where, and for how long

**Built:** regard (−4 to +4, decaying a step a real day), faces, claims, seen nodes, passed and failed
persuades, knowledge, attitude adding regard ÷ 2 with Trees on, persuade's +1 for remembered help, and
life-stamped leader and wanderer memory ([docs/DIALOGUE.md](../docs/DIALOGUE.md#memory),
[docs/REPUTATION.md](../docs/REPUTATION.md#face-and-name)).

Still to build:
- **The liar count** per faction per player, 1 off each real day.
- **Knowledge with expiry:** passing facts (the word at the wire until the next 05:30; chatter for 2 hours of
  play, 32 at most), and evidence kept with its run.
- **Who writes regard** beyond dialogue: errands, on named-person keys only, never on leaders.

### Make it right

- A leader at −2 or below offers one line: "[Make it right] 'What would it take?'".
- It offers that faction's built amends contract, given by that leader.
- When it's done, regard goes to 0, and the built crossed bit is cleared, once a season per leader.
- The Infamy it works off goes through `Forgive()`, inside the 30-per-life budget
  ([REPUTATION_NAMES.md](../docs/REPUTATION.md#working-off-infamy)).
- This changes a built rule, so REPUTATION.md changes in the same commit.

## 7. Conditions

**Built:** the condition syntax and its keys, including `kit`, `passing`, `caught`, `sitting`, `tabled`,
`route`, `promise`, `kept`, `broke`, `due`, `heard_lie`, `title`, `seasoned` and `owed`
([docs/DIALOGUE.md](../docs/DIALOGUE.md#conditions)).

Still to build, as their owners are built: `claimed` (disguise); `tribe*`, `alpha*` (the tribes, through the
infected adapter); `lean` (the majors); `cure_held`, `ending`, `was` (the season story; `finale:*` and `route`
are built); `q:` and `m:` (questline and mission values); `declared` and `symptoms`; `group`.

## 8. Checks and tags (New Vegas style)

**Built:** background, standing, `persuade` (Trust), `threaten` (Fear, with its rules and the Hunted-cliff tag),
`cost`, Pass / Short / Refused, trait modifiers, repeats, failure as a branch, and factionless speakers
([docs/DIALOGUE.md](../docs/DIALOGUE.md#checks-and-tags)).

Still to build:

- The lie's hidden modifiers (the speaker's seeded credulity, +1 Paranoid, +1 if this speaker caught you
  lying this life) and +1 for Ex-Con and Raider on every lie.
- **Groups.** In a decide scene the best talker present can answer for everyone; each member keeps their own
  one try per job. In a tally scene each player sways only their own voter.
- The design's non-modal deliveries (the panel, the action-menu donors that hide checks you'd fail with "There's
  more they'd say to someone they trusted.", the hint fallback) are dropped in favour of the talk screen, which
  shows every check with its score ([PLAN.md](PLAN.md#built-alongside-the-work-in-roadmap-next)).

## 9. Results (effects)

**Built:** `mark`, `cset`, `nset`, `regard`, `favor` (10 a conversation, 20 a faction a day), `cost`, `take`,
`claim`, `learn`, `rumour`, `call` (the person's own replies, the safehouse hand-in, the toll, the post's
passage, standing), `offer`, and no Fame, Infamy or items ever ([docs/DIALOGUE.md](../docs/DIALOGUE.md#what-a-choice-does)).

Still to build, with scenes and the systems that own them:

| Effect | Example | Does | Limit |
|---|---|---|---|
| `set`, `qset` | `set:ask.tins@house=1` | season and run flags | world effect: only in a scene; scoped (`@house`, `@seat`, `@run`) in a conversation that serves many speakers |
| `return` | `return:keepsake` | gives back only an item this character handed this speaker | never gives new items |
| `note` | `note:"..."` | a journal note on the run | the existing caps |
| `reveal` | `reveal` | the exact spot of the speaker's contract | only to players the contract's own rule allows; never behind a lie or a disguise |
| `begin` | `begin:other_two` | a questline's `TryBegin` | the engine's own rules |
| `errand` | `errand:house_tins` | asks the errands dealer to deal that card | the dealer's rules ([ERRANDS.md](ERRANDS.md)); never on a leader |
| `start`, `event` | `start:walk_back` | the stage mission's block or event (`OnDialogueEvent`) | the physical half of an outcome |
| `resolve` | `resolve:all_in:"talked the head round"` | the stage's `Resolve` with a named outcome | only from a conversation attached to that running stage's person |
| `approach` | `approach:talk:"asked about the stores"` | `Remember`: this player took part this way | personal |
| `sway` | `sway:pot` | `RZ_QuestVote.Sway` | one sway per player per sitting |
| `reward` | `reward:haven:5:20:"walked them home"` | `Run.Reward` | quest conversations only |
| `call` | `call:work`, `call:rob`, `call:search`, `call:declare_bite`, `call:table_bite` | named calls for the built behaviours (today's `slot:N` covers them) | its own rules |
| `hostile`, `flee` | `hostile:3` | a timed grudge; the wanderer runs | grudges restart |
| `pass`, `catch` | `pass:post`, `catch:bastion` | a passed or caught look's result | the borrowed-kit rules |

**Personal and world.** Season, run and scene flags, `resolve`, `start` and `commit` change the world: they're
allowed only in a scene, so they run once ([section 14](#14-multiplayer)). **Talk resolves talk:** anything
physical starts the stage's block with `start:`, and the stage resolves and pays when the block ends.

## 10. Lying and being lied to

**Built:** the player's `[Lie]`, the lie store found out by events first, lies you're told (`~lie`, `~truth`,
`heard_lie`, the [Evidence] call-out), a lie's `truth`, and the speaker's next greeting saying a lie was found out
([docs/DIALOGUE.md](../docs/DIALOGUE.md#lies-youre-told)).

Still to build:
- a lie about a bite at a safehouse being the built "caught hiding a bite" instead;
- **claims the world tests** by the owner's own check instead of the rumour tick ("They're expecting me back by
  six": the family checks at 20:30);
- a choice's `found` effects, run when the lie is found out, only on things still running (a passage revoked, a
  sway removed), never on Fame already paid or a finished mission;
- **lies that move the world:** a lie picked in a scene is recorded on the scene with the liar's identity, and a
  questline can read `scene_lied:<scene>` later (a callback, a harder stage, a name in a headline through the
  names policy); the world effect stays;
- **more ways to the truth:** a clue, a cracked secure channel or a journal note, and **world predicates**
  checked when the player gets there (`town:thick=@target` is true the moment they walk into that town while its
  crawlers are thick), with the binding that names the place; overheard public lines marking the lie for the
  people who hear them; the journal's "What you know" list ("Grigor said the cellar was empty. It wasn't.");
  givers' briefs with `~lie` lines, the mission owning the truth in its truth deck;
- **hiding things:** behind trust, evidence or a check, with a default deflection in voice ("Ask me something
  else.") and a Press choice beside it (persuade, or threaten where allowed), with its fail branch.

## 11. Promises, callbacks and knowledge

**Built:** promises (`promise:`, `keep:`, a `.visit` kept by coming back; 2 hours of the promiser's play, 3 real
days at most, 3 open; regard *kept* and *broke*; no pay of their own; the speaker lists what's open) and
callbacks (`later:`, `due:`; 10 per player, 3 days) ([docs/DIALOGUE.md](../docs/DIALOGUE.md#promises-and-callbacks)).

Still to build:
- **Promise kinds tied to more built events:** an item at a place, someone walked home, a message told to a named
  occupant.
- **Where you see them:** "Promises" rows in the journal, with the time left.
- **One callback queue** for dialogue's `later:`, missions' echoes and errands' follow-ups: checked on the 5-minute
  tick near the subject, and a separate system pool of 100 for quests and systems, so nobody can push out
  someone else's. At death, a waiting one becomes an unsigned board note at its subject ("For the man who
  brought the Dvoraks in, if anyone sees him").
- **Knowledge** recorded on the server when it sends a player a line that carries a key: open-channel chatter,
  secure lines (including the word at the wire), clue reads and leaders' news, questline notes, world
  predicates. Evidence keys (up to 96) last with their run or the season; passing facts (up to 32) expire.

## 12. Claims in someone else's kit

**Built:** kit greets (`kit:own`, `kit:in_kit`), claims as `look` choices with the look's tags, a pass waving you
through a post, passing counting 0 at persuade and no threats, leaders and home ground seeing through it, the lint
refusing rewards behind a claim, and a change of clothes ending the session
([docs/DIALOGUE.md](../docs/DIALOGUE.md#claims-in-someone-elses-kit)). The disguise system's kit kinds, signed-for
kit, a dead man's name and the Rogue's price are built too ([docs/REPUTATION.md](../docs/REPUTATION.md#borrowed-kit),
which owns kit, colours, the look, catches, word gets round and vouching at barriers).

Still to build:
- **The other kit kinds as `kit:` words** (`issued`, `signed_other`, `dead_mans`, `off_turned`) in conversations,
  with their reactions: "You're still in our kit. Take it off before someone asks why." (issued).
- **Cover details spoken as choices**, each `cover:<key>`, +1 at the look (Bastion's word at the wire is built
  as `wire:N`):
  - **a real name you've heard;**
  - **a crew name** at the Red Hand ("What do they call you?"), your own or one heard on the open channel;
  - **a notebook number** at Halcyon, only from a dead researcher's notebook;
  - **a mission's paper**, where the stage provides one.
- **A post speaker opening the claim** at 35 m (section 4), and an outpost gate's look giving 10 minutes inside.

## 13. Infection, time and items

- **Bitten** players (`bite_shown`) get [Bitten] topics:
  - "Declare it" at any safehouse (`call:declare_bite`, the built service);
  - "Ask about the trial dose" at Halcyon;
  - "Declare your bite" at the Long Table (`call:table_bite`, the family's own blessing, not the safehouse
    service).
- "I'm not bitten" is a lie whenever the HUD shows a bite. Its tell is visible symptoms. Nothing reads
  whether the bite took.
- ★ Nurse and Doctor read other people's symptoms: "[Doctor] 'That isn't a bite.'" (Every Name, Act I).
- **Time:** the routines are lines, not clocks of their own, converted to real minutes by `RZ_GameTime`:
  - shift change at 05:30 and 17:30 ("Shift change. Two minutes.");
  - supper at 18:30 ("Sit, it's grace.");
  - the night watch at 20:30, and Bastion's roll and nightly call.

  Dusk and night open and close topics.
- **Items** are read by the built keyword lists. Taking is real. Giving back is only ever your own item.

## 14. Multiplayer

**Built** ([docs/DIALOGUE.md](../docs/DIALOGUE.md#several-players)): overheard `public` nodes at 8 m, the
decide hold with Object and Stand with and the scene record, tallies (`sway:`), vouching, call-outs and
point-outs, a scene's `who`, the 45 s idle while someone waits, and the majors' sittings swayed through
`RZ_QuestVote`, one sway per player per sitting. Still to build:
- **Who may decide, by default:** the stage's own requirement and having taken part, when a mission's scene names
  no `who`.
- **With the decider:** the decider's vanilla group within 150 m and anyone who stood with them share the result's
  pay; everyone else eligible gets half the Favor from the giver (missions' rule, once missions take conversations).
- **No double pay** once missions pay through conversations.

## 15. Delivery

**All personal text travels owner-only** in `PAYLOAD_DIALOGUE` (kind 6, at most 2,000 characters a record).
The plan makes this the one personal payload for every piece: dialogue nodes, overheard lines, knowledge
lines and the close signal, and also errand offers and replies, disguise look scores, reputation's budget and
change notices, lock lines, epilogue personal lines and the journal's detail-on-open. Nothing personal goes
through `NotifyPlayer`, whose RPC is broadcast to every client (`Core/RZ_GameModeComponent.c`, about line
2329). So a modded client can't read anyone else's choices, tags, score or the day's word. Errands' "talk
lane" is this payload.

**A. The panel (the main view, non-modal).** `RZ_DialoguePanel` is drawn on the HUD beside `RZ_HudDisplay`
when its layout exists. It shows:
- the speaker, their role and place, the faction emblem and colour;
- a standing strip: your tier or reputation name with them, and their regard word ("remembers you helped");
- their lines, with a result tag after a check;
- numbered choices with full tags: seen ones dimmed, would-fail checks in red with what they need;
- "Also here: Mara", and overheard lines.

You keep moving and looking around. Picks go through the action menu (B) or `/t <n>`. The panel closes with
the session.

**B. The action-menu slots** (always, with or without layouts). No new actions go on any prefab. Dialogue
borrows the ReforgerZ actions already on the speaker, the **slot donors**, in a fixed order:

| Speaker body | Donors | Slots |
|---|---|---|
| The generic set (civilians, leaders' bodies, occupants, named people, quest people, wanderers, the post speaker) | 4 choices and 4 uses | 8: up to 6 choices, More..., Leave |
| Quartermasters | 4 `RZ_LeaderAction`, `RZ_StandingStatusAction`, the amends `RZ_FactionServiceAction` | 6: up to 4 choices, More..., Leave |

How the donors behave:
- While the local player has a session on that entity, each donor reads its label from the client's dialogue
  cache ("1 [Doctor] That isn't a bite."). Unused donors hide.
- Every other ReforgerZ action on the entity (shops, services) hides too.
- Labels are at most 40 characters, with short tags. The writer can give a `short "..."` label.
- On the client, a donor's `PerformAction` sends `RpcAsk_DialoguePick(serial, choiceId)` and nothing else.
- On the server, a donor does nothing while that player has a session on that entity.
- VERIFY: labels refresh per local player when the cache changes, and `PerformAction` runs on the performing
  client (the built `!Replication.IsServer()` guards suggest it does). Otherwise `HasLocalEffectOnlyScript`
  answers from the cache.

**C. The hint (no layout).** One vanilla hint per node, holding the lines and a numbered list. It **replaces**
dialogue's previous hint instead of joining the 8-deep, 9-seconds-each queue. VERIFY that the hint manager can
replace or clear a hint. `/t` shows it again.

**D. The last fallback.** If per-player labels don't refresh (VERIFY):
- the donors read "Answer 1" to "Answer 6", More..., Leave;
- `/t <n>` picks and `/t 0` leaves;
- `/t` repeats the node and lists open promises.

This works with no layout and no dynamic labels.

**E. The menu (optional).** `RZ_DialogueMenu` (`ChimeraMenuBase`): keys 1–7, mouse, Esc. It's opened from the
panel with an input action you bind, for reading a long scene somewhere safe. It closes on any damage or
danger. VERIFY whether players can move with it open; if they can't, it stays optional.

**Notice lanes.** Dialogue lines ride the **talk** lane of the shared interruption budget (owner-only, shown at
once). Barks ride the bark lane. Server-opened greets that nobody asked for (a stranger calling out, the host on
the road) count as **unasked contacts**: at most one every 4 minutes per player across radio offers, leader
pings, wanderers, the Long Table host and errands, with errands last.

**Off.** With **Dialogue > On** false, no donor relabels, and every built action and line runs exactly as built.

## 16. Radio

**Built:** the open channel as a speaker (`radio:open`, "Call on the open channel" on a radio item), public nodes
on air without names, and its first conversations ([docs/DIALOGUE.md](../docs/DIALOGUE.md#the-open-channel)).
Barks and strangers are built too ([docs/DIALOGUE.md](../docs/DIALOGUE.md#barks)).

Still to build:
- **A house radio with power:** the call from a house radio prop, with power read.
- **Distress calls:** a stranger on the air, dealt by errands, answered on the channel.
- **Secure nets:** Bastion's net, the Red Hand boss's and Halcyon's lab, callable only by a Radio Operator who
  has cracked them.
- The finale's Independent route is committed on the open channel ([section 18](#18-finale-and-the-last-week)).

## 17. Asks: errands, spoken

Asks are errands. [ERRANDS.md](ERRANDS.md) deals them, pays them (its small-pay ledger) and keeps the
interruption budget; dialogue is how they're spoken, through `errand:` and the asker's conversation.

- **Always relevant.** Every ask's `when` must read the world or the player's place in it (errands' Why):
  - the speaker's house, town or the nearest tribe;
  - the faction's need or morale;
  - a recent `Changed()` key, such as a lost truck or a dead leader;
  - a feud or truce;
  - the player's tier, reputation name, background, exile or bite.

  Items and time alone don't count. The lint refuses an ask without one of these. When nothing is true near
  you, nobody asks.
- **Leaders never ask.** A leader's line names the person who needs it.
- **Promises are an errand's accept** where there's no other: "I'll bring tins by tomorrow."
- **Callbacks.** The asker's next greeting or bark reflects how it went, through the one callback queue
  (section 11).
- **Talk pay:** talk alone pays at most the talk line of the ledger (20 Favor per faction per real day), never
  items, never Fame. Errand pay is errands'.

## 18. Finale and the last week

### The finale's commit

The commit node, the `always` route topics and the pitches are built ([docs/DIALOGUE.md](../docs/DIALOGUE.md#the-finales-commit)).
Still to build:

- **The Independent on the open channel**, at any powered house radio or a carried one, through the built open-channel speaker:
  said on air without a name, *"This is nobody. The cure goes to everyone."*, and every faction hears that
  somebody said it. Today it's committed at the Doctor or the Courier on the road.
- **After the lock** route lines are face to face only, cool on the first visit and back to normal over two
  real days. Shared channels never single out anyone's route. `route:none` players in the last 10% are treated
  as bystanders.
- **The confirm node's "you bring"** also lists the player's notes for the route, once notes are built.

### The last week

After the cure the epilogue lasts **one week** (168 hours; decision 12). [EPILOGUE.md](../docs/EPILOGUE.md) owns its
content, the closure set and the hall of fame; dialogue delivers most of it. Lines read four separate things and
never mix them up:

| What | Key | Example |
|---|---|---|
| The world's outcome | `cure_held:`, `ending:<major>` | "The cure came in a crate with a number on it." |
| Who was there | `was:<scene>`, from the majors' decider lists by identity | "You were at the barrier when the chair read it out." |
| This player's route | `route:` (the route at the lock, kept for lines) | "You stood with Bastion on the lab road. Don't expect soup." |
| This character's memory | claims, promises, regard | "You brought the tins. Before all this." |

What dialogue carries in the week:
- **Leaders' epilogue topics:** "The cure" and "Me", at every leader, in voice.
- **Last Rounds:** the majors' codas are public scenes (up to 16 sessions): the Haven reads the List at the north
  road, Bastion's last roll at the barrier, the Red Hand closes the book, Halcyon's paper goes out
  ([MAJORS.md](../docs/QUESTLINES.md#the-majors)).
- **Loose Ends:** each unfinished questline's closing stage is a short conversation scene at its own person.
- **Bring Them Home:** a hand-in topic at each post, house and station for tags, ID cards, notebooks and
  keepsakes of the named dead.
- **Settle Accounts:** the parley reopens with everyone ("It's over. Talk."); Make it right, settling up and
  the amends offer are topics as usual, inside the forgiveness budget.
- **The Walk Out:** the voice at the north road barrier is a `post` speaker. It asks "Name?".
- **The Last Broadcast:** a tally scene at the radio station: what the station tells the outside about the
  season.
- **The Last Entry:** a topic at the memorial board where a player writes their own line, through the note
  filter.
- **Greets, reactions and barks** with `when epilogue`, written per speaker kind and voice. Along with the
  boards, the radio and the marks, this is the world telling the season back to you.

Every scene and topic in the week can be finished within it, and none pays anything that carries into the next
season except hall-of-fame lines.

## 19. Authoring format (`.rzd`), pipeline and tools

**Built:** the format for conversations, topics, greets and barks, `~lie` and `~truth` lines, the choice options
`truth`, `callout` and `dry`, the tokens `{title:f}` and `{their:key}`, the packer with its lint, the simulator for
one character, `dialogue/shipped.lock`, the compile check running the lint first, `/talkdebug`, and the writing
rules ([docs/DIALOGUE.md](../docs/DIALOGUE.md#the-text-format-rzd), [docs/WRITING.md](../docs/WRITING.md#conversations)).
The text source is the generated packs ("plan B").

Still to build:
- **Keywords:** scene scopes `run` and `house`, `bind <name> = <picker>`, `use <conv id>`, `opens approach <metres>`,
  `always`; node option `shared`; `say (<role>)` for someone else in the scene; choice options `tell`, `found`,
  `decides`.
- **Tokens:** `{resident}`, `{tier:f}`, `{rep:f}`, `{word:f}` (another faction's), `{claim:key}`, `{bind:name}` and
  `{town:@name}`, `{decider}`, `{lean}`, `{tribe:N}`, `{alpha:N}`, `{word:bastion.N}`, `{kit_owner}`, `{dead_name}`.
- **Text source:** the server reading `.rzd` files from the addon (plan A, VERIFY `FileIO` on addon files on the
  dedicated server) and server-only `$profile:ReforgerZ/dialogue/*.rzd` overrides by conversation id.
- **Lint:** a `truth` that reads what the player can't know; a lie without `truth` or `tell always`; world effects
  and `decides` outside a scene; an unscoped season flag in a conversation that serves many speakers; a flag
  outside its owner's namespace; `resolve` with an outcome name the questline doesn't declare
  (`dialogue/quests/<key>/outcomes.txt`); `commit` without a confirm node; Halcyon long trees without numbers.
- **Simulator:** `--chars ... --race` for several characters in a scene (holds, objections, tallies).
- **Runtime test:** open every conversation's start node with a fake player and run each lie, promise and
  callback once.
- `/talkreset <scene>` (admins).

## 20. Fitting the existing systems

| System | Change |
|---|---|
| Leaders | Talk opens the hub; Work, News and Standing are verbs (`call:work`, `call:news`, `call:standing`). First contact is a greet that uses `GetFirstContactLine`. Personal jobs are `offer:leader_job`. Attitude reads regard ÷ 2 while Dialogue is on, and earlier-life memory at half. Make it right. Leaders never give errands |
| Wanderers | Verbs keep the built specials, Share and Rob unchanged (one path: with Dialogue on, the old slot labels hide). Road news, moods and asks are topics |
| Safehouses | Occupants' talk, Declare your bite and supper lines. Services and the board stay their own actions. Reputation's Knock is a topic at the door ([REPUTATION_NAMES.md](REPUTATION_NAMES.md#62-the-parley-for-the-owed)) |
| Territory and posts | Named people speak. A post gets a speaker at the barrier. Search, toll and surrender are `call:` to the built behaviour, in voice. Disguise claims at the barrier are look choices |
| The Long Table | Its dealings are verbs and topics, with factionless checks. The host on the road opens a greet. Memory stays its three words |
| Questlines | `SpawnPerson(gm, pos, objIdx, talkClue, label, choices, convId)`. Effects map to `Remember`, `Resolve` (named outcomes), `SetQFlag`, `QNote`, `TryBribe`, `SideWith` and `Run.Reward`, through `RZ_Ways` (keyed by identity). `OnLeaderTalk` and `OnWandererTalk` become injected topics. With Dialogue on, a stage person with a conversation hides its built choice slots: one path per scene |
| Contracts | Any puzzle object can take a conversation (`SetDialogue(convId)`). `event:` and `start:` call `RZ_Mission.OnDialogueEvent`; `m:` reads `DialogueValue` |
| Errands | `errand:` asks the dealer. Askers and named people are speakers (strangers are built). Errands write regard only on named-person keys |
| Majors | `sway:` goes into `RZ_QuestVote` (a tally). `{lean}` reads it. "Ask about..." topics sit on other stronghold leaders while a stage is open. Set pieces and codas are public scenes. `was:` reads the decider lists |
| Tribes and crawlers | "Ask about the hills" at a safehouse. `{tribe:N}` and `{alpha:N}` in each faction's own word. Knowledge from kill sites, abandoned lairs and the calls. Asks and barks |
| Disguise | The other kit kinds and cover details spoken as choices (the rest is built). AI behaviour unchanged |
| Finale | The Independent's commit on the open channel (the route topics, confirm nodes, `CanCommit`, `commit` and `route:` are built) |
| Epilogue | Leaders' topics, codas, closing stages, hand-ins, the barrier's "Name?", greets, reactions and barks reading `cure_held`, `ending`, `was`, `route` and memory |
| UI | `PAYLOAD_DIALOGUE`, the panel, the slot donors, the hint fallback, `/t`, the optional menu, and journal rows for promises and "What you know" |

## 21. Performance and saving

**Built:** hubs built only at Talk from indexed, compiled conversations; one owner-only record per node;
`dialogue.json` written at the 2-minute save when changed and dropped with the season
([docs/DIALOGUE.md](../docs/DIALOGUE.md#saving-and-performance)).

Still to build: overheard lines as small owner-only records; callbacks saved with world memory once missions'
echoes join them; seen nodes capped at 512 per character if a busy server needs it (measure `dialogue.json`
there).

## 22. Phases and the first content

Dialogue lands across the plan's build order. Each step is playable on its own, and Dialogue off is today's game.

| Plan phase | Dialogue builds |
|---|---|
| 0. Fixes and spikes | The VERIFY spike (one evening): per-player donor labels, `PerformAction` on the client, replacing a hint, `FileIO` on addon files, moving with a menu open. The results pick the delivery and the text source. The owner-only payload for all personal text |
| 2. Talk | **Built** ([docs/DIALOGUE.md](../docs/DIALOGUE.md)), on the talk screen. Still: the runtime test opening every start node. **Turned on** by default only after the per-player label test and a play-test pass |
| 3. Small things | Errand asks through `errand:`. Promises, callbacks and barks are built |
| 4. The mission style | Built, but for cover details spoken as choices and the other kit kinds (with disguise's later phases) |
| 5. The majors | "Ask about..." topics; set pieces as public scenes. Sways into `RZ_QuestVote` at the sittings are built |
| 6. The finale | **Built** (the commit topics and confirm nodes, the pitches: [docs/DIALOGUE.md](../docs/DIALOGUE.md#the-finales-commit)). Still: route lines, and the Independent's commit on the radio |
| 7. The ending | Leaders' epilogue topics, the codas, closing stages, hand-ins, the barrier, epilogue greets and barks |
| 8. Growth | Distress calls and the secure nets on the radio. Strangers' and the open channel's conversations are built |

## 23. Docs in the same commits

`docs/DIALOGUE.md` exists (phase 2) and owns the topic. As each later part is built it goes there, with
REPUTATION.md (lies and the liar count, vouch, call-outs and point-out, claims and signed-for kit, Make it
right), WRITING.md (the word at the wire, notebook numbers, lies' tells), UI.md (the journal's "Promises" and
"What you know" rows), WORKBENCH_SETUP.md (settings, VERIFY, log lines), MISSIONS.md and QUESTLINES.md
(talk approaches, strangers and asks), ROADMAP.md and README.md.

---

## Examples

### A. The family at the gate (Every Name, Act I)

A decide scene: one decision per run, any standing but exiles. Talk resolves only talk; walking people anywhere
starts the stage's block.

```
conv every_name.act1.head
for seat:haven/council_head
when q:every_name.act=1 & m:act_house & hour>=17
topic "Ask about the family at the gate."
prio quest
voice haven
scene act1 run who !exiled

node start shared
  jump decided -> done
  say? trait:merciful,idealist :: Four of them. I'd take all four if the pot would stretch. It won't.
  say We can take two. The two who can work. Not with that cough.
  > @all "Take all four. I'll answer for it." -> all_in | refused   check persuade haven 2 bg:elder,nurse easier:merciful   do resolve:all_in:"talked the head round"   decides
  > @cold "That isn't a bite. It's her chest." -> cold   if bg:doctor,nurse
  > @tins Put three tins in the stores. -> all_in   cost item:food*3   do resolve:all_in:"put tins in the stores"   decides
  > @favor Put twenty Favor in the stores. -> all_in   cost favor:haven 20   do resolve:all_in:"paid into the stores"   decides
  > @bastion "Bastion takes in the old. I'll walk them over." -> to_bastion   do start:escort_bastion   decides
  > @walk "Fine. I'll walk the other two back to the road." -> walk   do start:walk_back   decides
  > @leave Leave. -> end

node cold shared
  say (father) She's had it since the school trip. Tell her.
  say ...You'd say that in front of the house?
  > @front "In front of anyone." -> all_in   do resolve:all_in:"read the girl's chest"   decides
  > @other Ask something else. -> start

node refused shared
  say I said two. I meant two. You don't do the sums at supper. I do.
  > @tins2 Put three tins in the stores. -> all_in   cost item:food*3   do resolve:all_in:"put tins in the stores"   decides
  > @bastion2 "Then I'll walk them to Bastion myself." -> to_bastion   do start:escort_bastion   decides
  > @walk2 "All right. I'll walk them back." -> walk   do start:walk_back   decides
  > @leave2 Leave. -> end

node all_in shared
  say Then they eat what we eat. Somebody find the old man a corner.
  > @leave3 Leave. -> end

node to_bastion shared
  say Bastion. They'll put every name on a board. ...Go on, then. Before it's dark.
  > @leave4 Leave. -> end

node walk shared
  say Thank you. I'll write their names down. In pencil.
  > @leave5 Leave. -> end

node done
  say? q:every_name.family=1 :: All four are inside. The old man's already complaining about the soup.
  say It's settled. {decider} answered for the house.
  > @leave6 Leave. -> end
```

The stage applies +4 people and −2 food for all_in; the head pays 10 Favor only after the walk back at dusk ends;
Bastion's +5 Fame comes when the escort arrives. What each player sees:
- A Doctor sees '[Doctor] "That isn't a bite. It's her chest."'.
- A Known Nurse sees '[Persuade · Haven · 2 of 2]': nurse is listed, so it passes.
- A plain Known player sees '[Persuade · Haven · 1 of 2] "Take all four..."' in red, with '(needs Trusted)'. One
  under is Short, so the pick goes to `refused`, where the tins and the walk are still on offer.
- An exile never sees the topic: the Haven's door is shut.

A decisive pick starts the 5-second hold: the room hears "Mara is about to answer for the house." Tom, eligible
and in the room, can Object once; then he's the speaker for 60 s. Anyone can Stand with Mara instead. When the
hold ends, everyone else at the head moves to `done`: "Mara has already answered for the house."

### B. Bastion kit at a checkpoint: the look, and the word at the wire

The post speaker opens the session at 35 m for anyone in Bastion's kit below Known, so skipping Talk doesn't skip
the question. The word itself is a cover detail.

```
conv bastion.post.kit
for post:bastion
when kit:bastion=in_kit,signed_other,dead_mans,off_turned & !caught:bastion & !hunted:bastion
greet
opens approach 35
prio world
voice bastion

node start
  jump kit:bastion=signed_other -> signed
  jump kit:bastion=dead_mans -> dead_mans
  jump kit:bastion=off_turned -> off_turned
  say Not on the board. Word.
  > @w0 "{word:bastion.0}." -> pass | caught   check look bastion 2 cover:word.0
  > @w1 "{word:bastion.1}." -> pass | caught   check look bastion 2 cover:word.1
  > @w2 "{word:bastion.2}." -> pass | caught   check look bastion 2 cover:word.2
  > @noword "Haven't had it. On the road since the shift change." -> pass | caught   check look bastion 2
  > @truth "Took it off a man on the road. He'd no further use for it." -> honest
  > @away Walk away. -> end

node pass
  say Carry on. Sling that properly.
  do pass:post
  > @leave Leave. -> end

node honest
  say Then take it off before someone shoots you in it. Arms out. Search.
  do call:search
  > @leave3 Leave. -> end

node caught
  say Hands on the barrier. Name? Doesn't matter. You're on the board now.
  do catch:bastion
  > @leave4 Leave. -> end

node signed
  say That's {kit_owner}'s jacket. Signed for. So where's {kit_owner}?
  do catch:bastion
  > @leave6 Leave. -> end

node dead_mans
  say That's {dead_name}'s. He's on the nail behind the radio.
  do catch:bastion
  > @leave7 Leave. -> end

node off_turned
  say That's off one of ours. Where is he?
  do catch:bastion
  > @leave8 Leave. -> end
```

- The tags show the look before the pick: '[Full kit +2] [Watched −1] 1 of 2', and on the word choices
  '[Word: right +1, wrong fails]'. A player who heard the word on the secure net, or was told it by a friend
  over voice, picks it and scores 2 of 2. A guess is right 1 time in 3.
- A pass gets one passage at this post while it stands: no search. Nothing else.
- The truth gets a search and nothing worse.
- A wrong word, a caught look, signed-for kit, a dead man's kit or kit off the turned is a catch: held, the kit
  taken, +3 Bastion Infamy (never into Watched on its own), one of three catches this life, and word gets round
  at the nearest holding.
- Walking away is no claim: the post's built search or turn-back applies as you come in.
- None of it opens a shop, a job or the camp gate.

### C. The Red Hand toll man: dark humour, an honest tag on a joke lie, a threat that always costs

```
conv redhand.toll.man
for post:redhand
when tier:redhand=known
topic "Ask about the toll."
prio world
voice redhand

node start public
  say Red hand means stop. Stop means pay. Ten Favor, or walk round. It's only eleven kilometres. Mostly infected.
  > @pay Pay the toll. -> paid   do call:toll
  > @rudi "Rudi sent me." -> waved | rudi   check lie redhand 2   tell always
  > @ours "I'm one of yours." -> waved | caught   check look redhand 2   if kit:redhand=in_kit
  > @move "Move the lorry." -> moved | laughed   check threaten redhand 2
  > @leave Walk round. -> end

node paid
  say Pleasure. Mind the potholes. Some of them are people.
  > @leave2 Leave. -> end

node waved
  say Go on, then. Tell the boss Lukas says hello. Or don't. He hates that.
  do pass:post
  > @leave3 Leave. -> end

node rudi
  say Rudi? Rudi got bitten. Day 7. Everybody knows that. Nice try. It's fifteen now.
  > @pay15 Pay fifteen. -> paid   do call:toll:15
  > @round Walk round. -> end

node caught
  say Nobody calls you anything. That's the problem. Wallet or teeth?
  do catch:redhand
  > @leave5 Leave. -> end

node moved
  say ...Fine. Fine! Move it, lads. Somebody write this one down.
  do pass:post
  > @leave4 Leave. -> end

node laughed
  say He's got a stick! Look, lads, a stick. Fifteen, and you can keep the stick.
  > @pay15c Pay fifteen. -> paid   do call:toll:15
  > @round3 Walk round. -> end
```

Notes:
- 'Rudi sent me' shows [Lie] and is always caught. Its tell note: 'Rudi has been dead since Day 7.' The crewman
  tells the Red Hand's official lie flatly, and that's the dark edge, not a joke about Rudi.
- 'I'm one of yours' shows only to someone in Red Hand kit below Known, and it's the look, checked every time it's
  told: passing once never passes forever. Caught is the Red Hand's catch: the kit taken, plus the toll for liars
  (10 Red Hand Favor or a beating), +3 Infamy, one of three catches. The open channel laughs about it without a
  name.
- The threat shows '[Threaten · +3 Red Hand Infamy · Fear 1 of 2]' for someone with Feared band 1. It costs +3
  and 'threatened' regard, pass or fail, and it's never remembered as a pass. Once anyone has threatened this toll
  man: "He's already scared. Of someone else."
- `call:toll` charges the toll reputation sets: 20 for the Wildcard tier, and the higher of that and the lab-coat
  toll, never both.
- The node is public. Others within 8 m see the line without its tag, and for 20 s after the lie they can call it
  out.

### E. A Halcyon field lead who hides the other two

```
conv halcyon.lead.beds
for seat:halcyon/field_lead
when cure>=25 & !q:other_two.running & !flag:other_two.ending
topic "Ask about the volunteers."
prio world
voice halcyon

node start
  say Eight in ten walk out. We write down the other two. They're in the record.
  > @where "Where are they now?" -> deflect
  > @sheets "Your sheets say ten went in. Your beds say eight." -> back_room   if bg:labtech,doctor
  > @leave Leave. -> end
  callout halcyon.back_room -> back_room

node deflect
  say Somewhere quiet. They're comfortable. Ask me something else. ~lie:halcyon.back_room
  > @show "Show me." -> back_room | refused   check persuade halcyon 3 bg:labtech,doctor
  > @leave2 Leave. -> end

node refused
  say No. And if you're from the Haven, tell them we're sorry. We are.
  do mark:other_two.sorry
  > @yourself "Tell them yourself." -> start
  > @leave3 Leave. -> end

node back_room
  say There's a room at the back. Someone sits with them. With a clipboard. It's the protocol, and I hate it.
  do learn:halcyon.back_room; begin:other_two
  > @leave4 Leave. -> end
```

The failed persuade is a branch: it marks a one-way season flag (`other_two.sorry`) that The Other Two's Haven
side reads. The questline doesn't need to read anyone's dialogue memory. The Lab Technician sees '[Lab
Technician] "Your sheets say ten went in..."'. 'They're comfortable' is the lie the callout answers, once HC2's
clue or the Haven woman's brother gives the truth.

### F. The Long Table's host on the road

It is never described. The lie is a claim the family tests, not a roll.

```
conv table.host.road
for table:host
when !nmem:walked
greet
prio world
voice table

node start
  say Sit, sit, you're thin. Where've you come from, and does anyone know you're out this way?
  > @expected "Half of Morton. They're expecting me back by six." -> expected   do claim:expected; later:20:30:table.expected
  > @alone "Nobody. Just me." -> welcome   do claim:alone
  > @bite "I've a bite. Thought I should say." -> bitten   if bite_shown
  > @walk Walk on. -> end   do call:table_walk

node expected
  say Then they'll be expecting you. Best not keep them. Supper's at seven, if you're ever back this way.
  > @accept "I could eat first." -> end   do call:table_accept
  > @leave Leave. -> end

node welcome
  say Good. There's always a place set.
  > @accept2 "I could eat." -> end   do call:table_accept
  > @leave2 Leave. -> end

node bitten
  say Bless you for saying. We look after the bitten. Come and sit; mother will want to see you.
  > @accept3 "All right." -> end   do call:table_accept
  > @leave3 Leave. -> end
```

No faction standing decides anything here: the checks are factionless, and the family's memory stays its three
built words. The claim's exposing condition is the family's own test: at 20:30 the callback checks whether the
player is still within 300 m of the farm. If they are, the claim was false, and next time: "Still expected, are
you?" The guest rule runs exactly as built. Every road tip the host gives is true, as the lore requires.

### G. Several players at the first sitting (the majors)

Votes are a tally owned by `RZ_QuestVote`; dialogue only adds sways.

```
conv every_name.sitting1.head
for seat:haven/council_head
when q:every_name.act=2 & sitting:every_name.1 & !swayed
topic "Speak for a rule."
prio quest
voice haven

node start public
  say {lean}
  > @pot "Everyone eats. Even the ones who can't work." -> moved | not_moved   check persuade haven 2 bg:elder easier:merciful   do sway:pot
  > @workers "Workers first. We can't feed what can't work." -> moved   do sway:workers
  > @board "I've seen Bastion's board. The cards are only for the census." -> moved | not_moved   check lie haven 2   truth knows:bastion.cards   do sway:cards
  > @leave Leave. -> end

node moved
  say ...All right. I'll say it at the table.
  > @leave2 Leave. -> end

node not_moved
  say I've a house to feed, and you've a mouth. Talk to someone whose house is full.
  > @leave3 Leave. -> end
```

(The pot choice above is the version for a head who leans to workers first. A head already leaning to the pot
moves with no check.)

How it plays out:
- Mara is Trusted with the Haven. Tom is Known.
- Tom speaks for the pot at the far house's head, who leans to workers first. That's difficulty 2, and Tom scores
  1 (Short).
- Mara stands next to him and picks '[Vouch for Tom] "He's with me."'. For 5 minutes Tom's checks there get +1,
  so he passes.
- His one sway this sitting goes to the pot. Mara sways a different head herself.
- Players within 8 m see "Tom, to Mrs Dvorak: 'Everyone eats. Even the ones who can't work.'" with no tags.
- Then Petr tells another head '[Lie] "I've seen Bastion's board..."' (he never has) and is believed.
- For 20 seconds Mara sees '[Call it out] "Petr's lying."'. She heard the secure line ('Haven count puts every name
  on the board') and calls it.
- It was a lie, so Petr is caught now (regard lied, +2 Haven Infamy) and his sway is taken out of the tally. Mara
  gets nothing.
- If Petr had really seen the board, Mara would take 'slander' with that head, and lose call-outs there for the
  life.

The vote is counted at 20:30, and the chair breaks ties. Nobody won anything by picking first.

### H. The point of no return on the radio

The faction routes' commit topics are built ([docs/DIALOGUE.md](../docs/DIALOGUE.md#the-finales-commit)). The
Independent's, on the open channel, isn't:

```
conv finale.route.independent
for radio:open
when finale:open & route:none
topic "Speak on the open channel about the cure."
prio quest
always
voice plain

node start public
  say (static) ...open channel. Go ahead.
  > @all "This is nobody. The cure goes to everyone. No crates, no lists." -> card
  > @off Switch off. -> end

node card confirm
  say (static) Once it's on the air, every faction hears that somebody said it.
  > @say Say it. -> said   do commit:indep
  > @off3 Switch off. -> end

node said
  say (static) ...Somebody keys the mike twice. Then a woman: 'Morton heard you.'
  > @leave Leave. -> end
```

- Nothing on air names the player: shared channels never single out anyone's route.
- `CanCommit` decides who may commit, not the writer.

### I. The epilogue week: a safehouse occupant

The world's outcome, presence and the player's route are kept apart.

```
conv epilogue.occupant
for occupant
when epilogue
greet
prio reaction
voice haven

node start
  say? was:every_name.reading :: You were at the barrier when the chair read it out. I saw you. Sit, there's soup. Real soup.
  say? cure_held:bastion :: The cure came in a crate with a number on it. We're on a list for it. Everyone's on a list now.
  say? cure_held:everyone :: Somebody walked it out to every house. No crate, no list. Mrs {resident} asked who to thank, and nobody would say.
  say? ending:every_name=every_name :: They read every name at supper. Even the crossed ones. Mrs {resident} cried, and then she went to bed early.
  say The loudspeakers say processing at the checkpoints. Nobody's gone up there yet. Nobody quite believes it.
  > @route "I stood with Bastion on the lab road." -> bastion_route   if route:bastion
  > @leave Leave. -> end

node bastion_route
  say I know. Everyone knows. Don't expect soup.
  > @leave2 Leave. -> end
```

The first true variant replaces the next plain line. `{resident}` is a seeded name for this house, never a name
from LORE. Most players will hear the ending as barks instead (example L), over the week.

### K. One node in each delivery

The panel: a header 'JANA KRAL · COUNCIL HEAD · MORTON' in amber, and the strip 'You: Trusted · Haven. Remembers
you helped.' Then her line, and the rows:
- '1 [Persuade · Haven · 3 of 2] "Take all four..."' (bone: you'd pass);
- '2 [Doctor] "That isn't a bite..."';
- '3 [3 food] Put three tins in the stores.' (dim: seen);
- '4 "Fine. I'll walk the other two back."';
- '5 Leave'.

The footer reads 'Also here: Tom'. The player keeps walking; the panel isn't a menu.

The action menu on her body (the generic set's 8 donors) reads '1 [Persuade] Take all four...', '2 [Doctor] That
isn't a bite...', and so on, then More... and Leave. Her other actions hide. Tom, next to her, sees his own list.

No layout: one vanilla hint titled 'Jana Kral, the Haven' with her line and a numbered list. It replaces the last
dialogue hint, and `/t` shows it again.

If labels don't refresh: the donors read 'Answer 1'...'Answer 6', More..., Leave, and '/t 2' in chat picks the
Doctor's line.

## Engine needs

**Built** in phase 2 ([docs/DIALOGUE.md](../docs/DIALOGUE.md)): `RZ_DialogueDB` (the compiled packs, indexed by
speaker kind), `RZ_DialogueCond` (conditions and tokens), `RZ_DialogueTrees` (hubs, nodes, checks, tags,
effects), `RZ_DialogueMemory` (`dialogue.json`), the settings, speaker adapters for leaders, wanderers, occupants,
named people, the road post's speaker and the Long Table, Trust through `RZ_Checks` with the dialogue bonus,
regard in leader and wanderer attitude, the danger check (the talk screen's guard), `tools/dialogue_pack.py` and
`tools/dialogue_sim.py`. The talk screen replaced the slot donors, the HUD panel, the hint fallback and `/t`
(dropped: one modal screen with the pad, see [PLAN.md](PLAN.md#built-alongside-the-work-in-roadmap-next)).

Still needed:
- Speaker adapters for quartermasters, quest people and the secure nets.
- **Hooks in existing code:**
  - `RZ_Leaders`, `RZ_Wanderers`, `RZ_Safehouses`, `RZ_Territory`, `RZ_Checkpoints` and `RZ_LongTable` expose
    their built behaviours as `call:` targets: work, news, standing, first_contact, share_food, rob,
    give_antidote, ask_cars, role special, scav_rumour (with an optional town hint), stop_bells, courier_job,
    kid_way, declare_bite, meds, hand_in, toll (with an amount), search, surrender, knock, table_eat,
    table_trade, table_bite, table_accept, table_walk.
  - With Dialogue on, their old slot labels hide (one path).
  - `RZ_QuestMission.SpawnPerson` gains a conversation id. A bridge maps resolve (named outcomes per questline),
    approach, qset, note, bribe, side-with and reward to `Resolve`, `Remember`, `SetQFlag`, `QNote`, `TryBribe`,
    `SideWith` and `Run.Reward`, through `RZ_Ways`.
  - `RZ_Mission` gains `OnDialogueEvent(gm, playerId, key)` and `DialogueValue(key)`, plus `SetDialogue(convId)` on
    puzzle objects.
- **Knowledge recording:** the server sets `knows:` keys when it sends a player a line that carries one:
  `RZ_FactionLife` chatter, secure lines (including a new seeded word at the wire, changed at 05:30, three words
  with one right), clue reads in `RZ_PuzzleMission`, leaders' news, `~truth` lines, questline notes, and world
  predicates checked on entering a town. Evidence keys: 96 per player, with their run or the season. Passing
  facts: 32, which expire.
- **Tokens in `RZ_WorldFacts.Fill` and `FillFor`:** `{word:bastion.N}` (the day's shuffled words), `{decider}`,
  `{bind:name}` and `{town:@name}`, `{npc}`, `{npc_short}`, `{house}`, `{resident}`, `{tier:f}`, `{rep:f}`,
  `{word:f}`, `{claim:key}`, `{kit_owner}`, `{dead_name}`, `{lean}` (majors), `{tribe:N}` and `{alpha:N}` (tribes,
  in the speaker's faction word), `{lock:bring}` and `{lock:leave}` (the finale). Bindings come from pickers on
  the world facts: `town:thick_within(m)`, `house:nearest_short`, and so on.
- **Promises and callbacks:** the built events report to `RZ_DialogueMemory` (a delivery at a drop, a rescue or
  escort arriving, a message told); missions' echoes and errands' follow-ups join the callback queue, with a
  system pool of 100.
- **Claims in kit:** the other kit kinds per player and faction (through `RZ_Disguise`); the server opens post
  sessions on approach (35 m).
- **Finale and epilogue**, supplied by those designs:
  - `RZ_Finale.CanCommit(identity, route)`: refused below the 90% gate, after the stand is won, if already locked,
    or when `CanSideWith` says no;
  - `Commit` and `RouteOf(identity)`, per identity for the season, ended at the cure;
  - the lock card's bring and leave lines;
  - `cure_held`, `ending:` and `was:` from `SeasonStory`'s decider lists by identity;
  - every key is quiet (false) until those designs are built.
- **Radio:** the call on house radio props with power, and the secure nets (`RZ_SecureRadio`'s channels) for Radio
  Operators who've cracked them.

## Settings

**Built** on `RZ_GameModeComponent > Dialogue`: **Trees** (off), **Talk Favor a day** (20), with the talk screen's
range, idle seconds and danger ([docs/WORKBENCH_SETUP.md](../docs/WORKBENCH_SETUP.md#5-settings-objects)).
Still to add as settings (built today as fixed numbers in the code): the 45 s idle while someone waits; 6
sessions a person, 16 for scenes; the overheard and room radii; the decide hold; the rumour tick (15%); promise
play hours (2) and the real-day ceiling (3); the bark radius and limits. And, with their parts: Make it right; the
text source.

Strangers' settings (on, the 15-minute ceiling, the places) belong to Missions > Errands.

## Options decided

| Question | Decided |
|---|---|
| Default state | Off until the phase 0 label test and a play-test prove the per-player labels (decision 13) |
| Show checks you'd fail | Yes: the panel and menu show persuade, threaten and look choices with their score and what's needed, and background lines only to that background, as in New Vegas. The action menu hides failing ones with one line |
| Lie discovery later | Events first; the rumour tick (two 15% chances) only for claims with no world test. Lies that move a scene are recorded on it either way |
| Point-of-no-return scope | Per identity for the season, so dying isn't a way out. It ends at the cure |
| The Independent route's speaker | The open radio channel, which every player can reach whatever their standing |
| Disguise intent | A disguise only when you claim one; kit in their colours is noticed but never a disguise on its own |
| Disguise passage | Whatever borrowed kit's look buys: one passage at a road post, or 10 minutes at a gate |
| The word at the wire | The word itself is a cover detail, so a friend can tell you; a wrong word fails at once |
| Social layer | Vouch, call-outs and point-out all on; overheard lines at 8 m |
| Small shared moments | Missions' 5-second hold, one objection per player per job, Stand with. Anything that matters to the season is a tally |
| Main delivery | The non-modal HUD panel with action-menu picks, plus the optional modal menu |
| Text source | Server-read text resources if `FileIO` allows; otherwise generated packs; `$profile` overrides for server-only twists |
| Make it right | On, through `Forgive()`; it changes a built rule |
| The Long Table's "expected" claim | Flavour, tested by the family at 20:30. It never holds the guest rule off |
| Threaten | The one Fear check, never on stronghold leaders, the Kid, the Long Table, and never free-standing in safe zones or safehouses |
| Talk pay | No Fame from talk, and Favor inside 20 a faction a day |
| Strangers | Only where one would make sense; never on a timer; one per player every 15 minutes at most (decision 14) |
| Where the topic lives | A new `docs/DIALOGUE.md`, added when built, owning both how it works and the format |

## Risks

- **Per-player labels on borrowed actions** are the backbone of the no-UI fallback. The engine may cache action
  names, or run `PerformAction` only on the server. Mitigations: the phase 0 spike first; the "Answer N" labels;
  the `/t` chat command; the panel. Dialogue stays off by default until it's proven.
- **Replacing hints:** if the vanilla hint manager can't replace or clear a shown hint, the no-layout fallback
  still queues behind other notices. Mitigations: `/t` repeats the node, and the panel removes the problem once
  its layout exists.
- **`FileIO` on addon files** may not work on the dedicated server. Then the generated packs compile on every
  client, which adds client compile time and needs a Workshop update per text fix. Mitigations: small functions,
  areas split into files, and `$profile` overrides for live fixes. Measure early.
- **Content volume and drift:** many conversations to write in voice. Built stage choices and their `.rzd` scenes
  must reach the same named outcomes. Mitigations: selectors and variants instead of copies; the lint (outcome
  names, reachability, fail branches, caps, voice and Long Table rules); the simulator; the runtime test; the
  first content capped at about 30 conversations.
- **Shared words at posts:** a group with a Radio Operator shares the daily word and passes posts in kit. It's
  bounded: the word is only +1 at a look that still reads the kit, the face and the night; a wrong word fails;
  every pass is one post while it stands; catches count three a life; signed-for and dead men's kit always fail.
  Play-test whether posts feel toothless.
- **Regard decays by real days**, so on a quiet server a caught lie fades while nobody plays. That's acceptable
  (it never locks anyone out), but tune the decay if liars shrug it off.
- **Fairness in decide scenes:** panel and menu players pick faster than action-menu players. Mitigations: tallies
  for anything that matters to the season; `who` gates; the fresh-node guard; the 5-second hold and one objection
  per player.
- **Objections used to stall:** one per player per job, and the objector must be eligible and in the room.
- **Danger detection** relies on positions only (closing distance). Some zombie mods move in ways that look like
  closing in, or don't. Tune the radius and threshold in play-tests; resume within 60 s softens false ends.
- **Cross-design dependence:** names, routes, tribes, the majors' votes, the look and errands come from other
  designs. Every key is quiet until they're built (a condition is false, a token is left out, a choice hides), but
  writers must not ship lines that make sense only with them.
- **Consistency with built rules:** Make it right, the Long Table claim, regard in Attitude, life-stamped memory
  and claims on approach at posts each change or extend a built rule. Each needs its owning doc (REPUTATION.md,
  DESIGN.md) updated in the same commit, or the rules drift apart.
- **Save growth and memory:** `dialogue.json` is estimated at a few KB per active player. Measure it on a busy
  server and cap seen nodes per character (512) if needed.
- **Tone at scale:** many lines can break the voices, especially the Long Table's "never describe" rule, Bastion's
  clipped voice and the line where dark humour stops. The lint's word lists, names checks and voice warnings catch
  some of it; the writer QA checklist and review catch the rest.
- **Strangers in the wrong place:** the placement rule depends on knowing where roads, wanderer routes, campfires
  and fresh events are. Where the world facts can't say (an unusual map), no stranger is dealt there, which is the
  safe failure.
- **Spoilers:** anything shipped in the addon (text or packs) is on every client's disk. Twists in `.rzd` files can
  be read by anyone who unpacks the mod. Truths are server state, drawn per season; only `$profile` overrides keep
  twists fully server-only, at the cost of server setup.
