# Dialogue

**Status: designed, not built.** Approved by the author on 2026-11-11.

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
  [DISGUISE.md](DISGUISE.md#6-looks), shown before you commit. Dialogue's random challenge odds, its
  suspicion counter, its blown-for-15-minutes and its doubled caught-lie cost are gone. The word at the
  wire, a real name, a crew name and a notebook number are **cover details** (+1 at the look; a wrong word
  fails at once). Kit kinds follow DISGUISE.md: own, issued, in their kit, signed for, a dead man's, off
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
  The rate is a ceiling only ([Strangers](#strangers)).
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
  (conflict 39). See [FINALE.md](FINALE.md#4-the-lock).
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
- [16. Barks, radio and strangers](#16-barks-radio-and-strangers)
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

Talking becomes one of the main ways into the Zone's stories. Everyone a player can talk to gets a **hub**:
- a greeting in their voice;
- the **verbs** that speaker always has (Ask for work, Share some food...);
- up to four **topics**;
- Leave.

Each topic is a short tree of **nodes**. In a node the speaker says one to three lines and the player picks
a **choice**. A choice can carry a **check** (background, standing, persuade, threaten, lie, look, cost,
evidence) and **effects** (flags, regard, Favor, items, mission outcomes). The speaker **remembers** what
this character said and did.

Everything runs on the server, and clients get one rendered node at a time, owner-only. Conversations are
data: text files checked by a tool. Other systems add topics to a person's hub by setting the facts the
topic reads, so a writer adds a file instead of editing someone else's.

**Guardrails it keeps**
- **No companions.** Nothing in a conversation makes an NPC follow, fight for or travel with a player.
  Escorts stay mission or errand blocks that end at a place (at most 600 m for errands). The lint refuses
  follow effects and escorts without a destination.
- **No pause and no time skip.** The world runs while you talk. Danger ends the conversation.
- **Nothing overpowered.**
  - Talk never pays Fame. It pays small Favor, inside one daily budget per faction, and never items.
  - Bigger rewards come only through a mission, errand or questline that talk starts or resolves, with
    that piece's own pay and caps.
  - No stronghold leader is moved, killed, threatened or replaced by talk.
  - A disguise never unlocks a reward.
- **Performance first.** Conditions are read only when someone talks, plus a 1-second check for each open
  session (at most 64). Memory is capped, decays, and has its own save file. Nothing physical is left
  behind.
- **Any map, any server.** Speakers come from the systems that own them. Text uses world tokens, and places
  come from the world facts, never coordinates.
- **Off is today's game.** With **Dialogue > On** false (the default until the label test and a play-test
  pass), every built action and line runs as it does now. The built memory bits and `PersuadeScore` are
  untouched.

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
| Cover detail | Something real you can say at a disguise look: the word at the wire, a real name, a crew name, a notebook number, a mission's paper ([DISGUISE.md](DISGUISE.md#6-looks)) |

## 3. Who talks

| Kind (`for`) | Who | Memory key | Default hub | Replaces today |
|---|---|---|---|---|
| `stronghold:<faction>` | The four stronghold leaders | `leader:<seat>:<generation>` | Verbs: Ask for work, Ask for news, How do I stand? First contact as a greet | `RZ_LeaderAction` TALK, WORK, NEWS, STANDING |
| `seat:<faction>/<kind>` (`council_head`, `outpost_lead`, `crew_boss`, `field_lead`) | Council heads, outpost commanders, crew bosses, field leads | `leader:<seat>:<generation>`. Someone new in the chair starts with no regard or claims | The same verbs, plus the personal job (`offer:leader_job`) | Same |
| `quartermaster:<faction>` | Shopkeepers away from a seat (at a seat, the quartermaster *is* the leader) | the holding | What's short?; requisition talk. Shop and services stay their own actions | — |
| `wanderer:<role>` | Trader, Doctor, Scavenger, Deserter, Preacher, Courier, Kid, Mechanic | `wanderer:<id>` | Verbs: the role's special, Share some food, Rob (built rules); Give them your antidote while bitten | The wanderer's 4 choice slots |
| `occupant` | Safehouse occupants | `house:<idx>` (memory per place: face and claims only) | How's the house?; asks | `HandleOccupantTalk` |
| `named:<faction>` | The cook, the sentry, the toll man at holdings, plus errands' named people (4 per holding) | `named:<holding>:<idx>` | Shift talk; asks | `HandlePersonTalk` |
| `post:<faction>` | A road post's **speaker at the barrier**: territory's occupant prefab for that faction (it has the generic set). It stands there while a player is within 100 m; the crew stays an AI group | `post:<faction>:<town>` | Verbs: Pay the toll, Surrender (when they apply). Searches and disguise claims | Notices only |
| `quest:<key>.<role>` | People a stage or contract spawns (`SpawnPerson` with a conversation id) | `quest:<key>:<role>:<run>` | Whatever the stage writes | `SpawnPerson` choice labels |
| `table:<role>` | The Long Table's host, mother, quiet son, the boy | the built per-life memory (three words) | Verbs: Sit down and eat, Trade for a jar | `RZ_LongTable.HandleChoice` |
| `stranger:<kind>` | **New.** People who need help, at places where they'd be ([section 16](#strangers)) | `stranger:<id>` (per life; gone with them) | Their request | — |
| `radio:<channel>` | **New.** No body: "Call on the channel" at a house radio with power, or a carried radio | `radio:<channel>` | Call-ins, distress calls, haggling ([section 16](#radio)) | — |

**Built refusals come first**, before any hub. They are the existing rules, not new ones:
- leader attitude −2 or below: "Not you.";
- Haven exile at Haven speakers: the door stays shut;
- wanderer attitude −2 or below: they refuse and run;
- crossed with the Long Table: a shut door;
- Kill on Sight with the speaker's faction: no talk.

There are two exceptions. First, an `always` topic (only the finale's route topics) still shows, with Leave
beside it. Second, a refusing leader offers **Make it right** ([section 6](#make-it-right)).

**Claimed speakers.** A leader, wanderer or house that a running story holds (a world-facts claim) shows
that story's topics first. For that turn, other conversations can't `offer`, `begin` or `start` anything
that touches the claimed subject ("Not now. I've got enough on."). If a story claims a speaker
mid-session, the next pick reads `when` again, and may send the player back to the hub.

## 4. How a session runs

1. **Open.** A session opens in one of two ways:
   - **Talk** within 4 m ("Talk to {name}");
   - **a server-opened greet:** a post speaker at 35 m (for a player in that faction's kit below Known, so
     a claim is offered whether or not they press Talk), a stranger at 10 m, or the host on the road.

   The server builds the hub and opens a session. The session holds the speaker key, the node, a serial
   number, the choice ids it sent, the bindings and the time.
2. **Pick.** A pick is `RpcAsk_DialoguePick(serial, choiceId)`. The server checks all of these:
   - the serial is current, and the choice id was in the list it sent;
   - the player is alive, in range and not in danger;
   - the choice's `if`, the conversation's `when` and the scene's decided record still hold in the live
     world.

   A pick that fails a check is ignored, and the node is sent again ("They didn't catch that.").
3. **Fresh-node guard.** Commit and cost choices are refused within 1.5 s of a node change. So a slot label
   that hasn't refreshed can't trap anyone into paying or committing.
4. **Special targets:** `end` closes, `hub` goes back to the topics, `back` goes to the previous node.
5. **A session ends** when any of these happens:
   - the player chooses Leave;
   - the player is 6 m from the body (10 m from a moving wanderer);
   - 120 s pass idle (45 s while another player is waiting for that speaker);
   - the body is gone, or the player dies;
   - danger (below);
   - the player's kit changes ([section 12](#12-claims-in-someone-elses-kit)).
6. **Danger.** Checked every second for each open session:
   - the player took damage or fired a weapon;
   - an infected is within 6 m of the player or the speaker;
   - an infected within 30 m has closed in by 2 m or more since the last check.

   Crawlers lying up in a town's doorways end nothing unless they're within 6 m. The speaker says one
   "Not now!" line in their voice.
7. **Resume.** A session ended by danger or range keeps its node for 60 s. Talk again in that time and it
   resumes ("Where were we?"), if its conditions still hold. So ending a rival's session by luring an
   infected only delays them.
8. **Rates and caps:**
   - one pick per serial, at least 0.3 s apart, and one Talk every 2 s;
   - 64 sessions on the server, 6 per speaker;
   - public scene nodes allow up to 16 (a sitting, a route topic with a crowd);
   - at the cap, the session idle longest (over 30 s) is closed for the newcomer. Otherwise: "One at a
     time."
9. Sessions aren't saved. After a restart everyone starts from the hub.

## 5. The hub

- **Layout:** the greeting, the verbs, up to 4 topics, More... (the next page), Leave.
- **Verbs** never count toward the topic limit. Each is a `call:` to built behaviour, so Rob is the same
  Rob, at the same price, whichever way you reach it.

  | Speaker | Verbs |
  |---|---|
  | Leaders | Ask for work, Ask for news, How do I stand? (the tier cell and name, in voice) |
  | Wanderers | The role's special, Share some food, Rob (+8 Haven Infamy, as built); Give them your antidote while bitten |
  | Posts | Pay the toll; Surrender (Bastion, while Watched or Hunted) |
  | The Long Table | Sit down and eat; Trade for a jar |

- **Leaders never give errands.** Their hub has no errand asks. When a seat has work that is an errand, the
  leader names the person to see ("Ask the cook. She's short again.").
- **Selectors, not links.** A conversation says who says it (`for`) and when (`when`). At Talk the server
  collects every conversation for that speaker kind whose `when` holds. The lists are indexed by kind at
  start.
- **Priority:** `quest` > `ask` > `world` > `reaction` > `flavour`. Quest topics are never dropped: they
  page onto More.... Flavour rotates on a seed of the speaker and the day.
- **Once and daily** count per character for each (conversation, speaker key). An ask done at one house
  still shows at another. A topic you've heard is dimmed, not hidden, unless it's `once`.
- **Injected topics.** Other systems open topics only by setting the flags and facts `when` reads.
  Questlines, the majors, tribes, needs and morale, truces, lost outposts, the cure, the finale and the
  epilogue all work this way. One example: the majors' "Ask {chair_short} about Day 7" on a stronghold
  leader while that act is open. Nobody calls into the dialogue code.
- **Variety without volume.** One conversation serves every council head, through:
  - trait variants (`say? trait:greedy :: ...`);
  - tier, regard and memory variants;
  - world tokens.

  Lines never use pronouns for seeded speakers (`{npc_short}` or the role instead). The majors already
  write their stronghold leaders' lines this way.

## 6. Memory: what is kept, where, and for how long

**The built memory bits.** Leaders keep helped, crossed, saved and met; wanderers keep helped, wronged and
saved. They're set only by the built events, and a `call:` to a built service sets them exactly as it does
today. Dialogue's own effects never write a built bit. One change, owned by reputation: each entry is
**life-stamped**. Memory from this life counts as built; memory from an earlier life counts at half
(rounded toward zero), and is voiced once as the name, not the face: *"There was a Lukas who crossed me.
You're not him. Are you?"* Met doesn't carry. See
[REPUTATION_NAMES.md](REPUTATION_NAMES.md#12-face-and-name-one-scope-rule).

**What dialogue adds:**

| What | Held for | Scope | How long | Cleared by |
|---|---|---|---|---|
| **Regard** (the words below) | each speaker key | per player | −4 to +4; moves 1 toward 0 each real day | time; Make it right sets it to 0 |
| Face (they know you: you talked to them undisguised, or were caught by them) | each faction | per character | the life | death (a new life is a new face) |
| Claims | each speaker key, in the one lie store | per character | the life; 6 per speaker (the oldest true claim drops first) | death, or exposure |
| Promises (a held-errand kind) | each speaker key | per character | until kept, or broken (time up or death) | — |
| Seen nodes, picked choices, passed checks | each (speaker key, conversation, choice id) | per character | the life | death |
| Failed checks | each (speaker key, choice id) | per player (identity) | persuade: until your score for it rises; lies and threats: a real day | time |
| Liar count | each faction | per player | 1 off each real day | time |
| Knowledge: evidence | each key | per player | with its run, or the season | the run ending |
| Knowledge: passing facts (the word at the wire, truck times, the watch) | each key | per player | until it expires (the word: the next 05:30; chatter: 2 hours of play) | time |
| Season, run and scene records | the world | — | the season or run | — |
| The Long Table | as built: guest, family, crossed | per character | the life | as built. No regard, no liar count |

Catches, blown time and word gets round belong to the disguise system, not to dialogue memory
([DISGUISE.md](DISGUISE.md#8-caught)).

**Regard words:**

| Word | Change | When |
|---|---|---|
| thanks | +1 | Only on a choice with a real cost or deed (the lint checks) |
| kept | +1 | A promise kept |
| broke | −1 | A promise broken |
| threatened | −1 | Any threat, pass or fail |
| slander | −1 | A wrong call-out or point-out |
| vouched for a liar | −1 | [Section 14](#vouch) |
| lied | −3 (Merciful −2, Paranoid −4) | A lie caught, now or later. This covers what missions called "caught lying"; there's no separate leader bit for it |

**Who writes regard.** Dialogue, and errands on **named-person keys only**. Errands never write regard,
memory or loyalty on a leader.

**How regard is used:**
- **Attitude.** Attitude adds regard ÷ 2, rounded toward zero. So talk moves a leader's or wanderer's
  attitude by at most ±2, and the effect fades. A caught lie to a Paranoid leader can shut that door for a
  day or two, never for the season.
- **Persuade.** Persuade's "remembered help" means regard +2, or the built helped or saved bit (this life).
- **Prices** read only the built bits, as today. So talk can't farm discounts.

First contact uses the built met bit (this life). Lines addressed to the person ("Back again.", "You told
me...") read face, claims and seen nodes, which are per character. So a new life is a new face.

### Make it right

- A leader at −2 or below offers one line: "[Make it right] 'What would it take?'".
- It offers that faction's built amends contract, given by that leader.
- When it's done, regard goes to 0, and the built crossed bit is cleared, once a season per leader.
- The Infamy it works off goes through `Forgive()`, inside the 30-per-life budget
  ([REPUTATION_NAMES.md](../docs/REPUTATION.md#working-off-infamy)).
- This changes a built rule, so REPUTATION.md changes in the same commit.

## 7. Conditions

Terms are joined with `&`. `!` negates a term. Commas inside a term mean "any of". There is no OR across
terms: write two choices instead. Every term reads an existing system.

| Key | Example | True when |
|---|---|---|
| `bg` | `bg:doctor,nurse` | the character's background is one of these |
| `tier`, `standing` | `tier:haven>=trusted`, `standing:redhand=wildcard` | access tier (mixed tiers count one lower); the exact grid cell |
| `fame`, `infamy` | `infamy:bastion>=some` | the number or band (low, some, high, extreme) |
| `rep` | `rep:redhand=merciful_thug` | the reputation name for that cell. [REPUTATION_NAMES.md](REPUTATION_NAMES.md) owns the table |
| `hunted`, `exiled`, `kin`, `owed` | `hunted:bastion` | Hunted or Kill on Sight; a Haven exile; kin of the fallen; owed a parley |
| `flag`, `q`, `m` | `flag:list.read`, `q:every_name.act=1`, `m:act_house` | a season flag; the questline run's flag; the mission's `DialogueValue` |
| `cflag`, `nmem` | `nmem:sold_pharmacy` | the character's own flag; this speaker's memory of this character |
| `regard`, `face` | `regard>=2`, `face:bastion` | regard; this faction knows your face |
| `seen`, `claim` | `seen:haven.head.mouths/start`, `claim:expected` | you've seen that node; you told this speaker that |
| `knows`, `heard_lie` | `knows:bastion.word` | you learned it; you heard that lie |
| `promise` | `promise:tins@house=open` | the state of a promise to this speaker or place |
| `item`, `wearing`, `armed` | `item:food*3` | carries it (built keyword lists); wears it; a firearm in hand or slung |
| `kit` | `kit:bastion=in_kit` | what kind of that faction's kit you wear: `own`, `issued`, `in_kit`, `signed_other`, `dead_mans`, `off_turned` ([DISGUISE.md](DISGUISE.md#3-kit)) |
| `claimed`, `passing`, `caught` | `passing:bastion` | you've claimed to be one of them this visit; you're passing as one of them here; you're blown with them |
| `bite_shown` | | the HUD shows a bite: the golden window or a stage |
| `stage` | `stage:weakness+` | the HUD's infection stage |
| `symptoms` | | others can see it: Weakness or Final, with no suppressant |
| `declared` | | the bite was declared at a safehouse this life |
| Time and season | `night`, `dusk`, `fog`, `hour>=17`, `day>=5`, `outbreak:overrun`, `cure>=50`, `finalstand`, `epilogue` | the world facts. `day` is the season day |
| Speaker | `trait:greedy`, `loyalty:rogue`, `attitude>=1` | the speaker's record |
| Place | `house:short`, `house:tier>=settlement`, `town:held=redhand`, `town:infection>=60`, `town:thick=@target`, `tribe:within(2500)`, `tribe:hungry`, `alpha:down` | the speaker's house or town, or a binding. `town:thick` is true once the player is inside that town while its crawlers are thick (infection 60+) |
| Scene and vote | `decided`, `sitting:every_name.1`, `lean:workers`, `swayed` | the scene's record; the majors' vote (`RZ_QuestVote`) |
| Finale | `finale:open`, `finale:can_commit=bastion`, `route:none` | the finale engine's answers; the route this identity locked to this season (`RouteOf`), kept for lines after the cure |
| Outcome | `cure_held:bastion`, `cure_held:everyone`, `ending:every_name=every_name`, `was:every_name.reading` | who controls the cure (`finale.route`, or the handed-over faction); a major's ending; this player is on that scene's decider list |
| Others | `others>=1`, `group` | other players within 8 m; someone from your vanilla group within 8 m |

**Only what the player knows.** A lie's `truth`, and the `if` of any choice whose tag would show, may read
only what the player knows:
- their background, standing and reputation name;
- their items and what they wear;
- the HUD: `bite_shown`, `stage`, `symptoms`;
- their own claims, flags and knowledge.

No condition can read whether a cleaned bite took. The HUD hides that until the window would have closed,
and dialogue keeps it hidden.

**Quiet defaults** for keys from designs that aren't built yet:

| Keys | Owner | Until it's built |
|---|---|---|
| `rep` | [REPUTATION_NAMES.md](REPUTATION_NAMES.md) | false; the standing strip and tags show the tier |
| `tribe*`, `alpha*`, `{tribe:N}`, `{alpha:N}` | [INFECTED.md](INFECTED.md) (through the ecology adapter) | false; choices that need them hide |
| `lean`, `sitting`, `sway`, `{lean}` | [MAJORS.md](MAJORS.md) | false; sway choices hide |
| `finale:*`, `route`, `commit`, `cure_held` | [FINALE.md](FINALE.md) | `route:none` is true; `finale:open` is false; commits are refused |
| `ending`, `was` | the majors' season story | false |
| `kit`, `claimed`, `passing`, `caught` | [DISGUISE.md](DISGUISE.md) | `kit` reads only `own`; claim choices hide |
| `stranger:*` | [ERRANDS.md](ERRANDS.md) | no strangers are dealt |

## 8. Checks and tags (New Vegas style)

The packer writes tags from the mechanics. Writers never type them, so a tag can't promise something the
check doesn't test. **Tags show costs and requirements, never outcomes.** All checks go through the one
check helper, `RZ_Checks`, shared with missions, errands, the majors and the finale.

| Check | Tag (panel, menu) | Short tag (action menu) | Score |
|---|---|---|---|
| Background (`if bg:`) | [Doctor] | [Doctor] | Shown only to characters with it. Not a roll: the line is simply theirs |
| Standing (`if tier:`, `standing:`, `rep:`) | [Trusted · Haven], [Merciful Thug · Red Hand] | [Trusted] | Shown only when true |
| `persuade F D` | [Persuade · Haven · 1 of 2] | [Persuade] | **Trust**, below |
| `threaten F D` | [Threaten · +3 Bastion Infamy · Fear 2 of 2] | [Threaten] | **Fear** ([MISSION_STYLE.md](MISSION_STYLE.md#checks-persuade-and-fear)) |
| `lie F D` | [Lie], shown only when the claim is false | [Lie] | Access rank with F, +1 ★ (Ex-Con, Raider, or the line's own list), plus the dialogue bonus. Against D plus hidden modifiers (below) |
| `look F D [cover:<key>]` | [Full kit +2] [Watched −1] 1 of 2 | [Look] | **The look** ([DISGUISE.md](DISGUISE.md#6-looks)): fixed, shown before you commit |
| `cost` | [20 Haven Favor], [3 food] | [20 Favor] | You can pay. Spent when picked (`TryBribe` semantics) |
| Callout | [Evidence] | [Evidence] | You hold the truth for that lie |
| `if bite_shown` | [Bitten] | [Bitten] | — |
| Vouch | [Vouch] | [Vouch] | [Section 14](#vouch) |
| `commit` | [Point of no return] | [No return] | `finale:can_commit`; always followed by a confirm node |

**Trust: the one persuade score** (decision 11). It's its own wrapper, so built questline checks never
change. It adds up:
- the access rank with F: Stranger 0, Known 1, Trusted 2, Champion 3; mixed tiers one less (Uneasy 0;
  Tolerated and Wildcard 1; Respected 2); Watched −1. Hunted or worse: you can't persuade them, and the
  choice shows greyed;
- +1 for a ★ background in the check's list;
- **at most +1 in total from dialogue:** remembered help (regard +2, or the built helped or saved bit this
  life), or a vouch while the voucher stands within 8 m;
- −1 if that faction's liar count is 2 or more;
- −1 for crossing lines (QUESTLINES.md, "Crossing lines"). Finale stages never write that mark.

Trait modifiers in the check move the difficulty (`easier:merciful` −1, `harder:paranoid` +1). Difficulty
runs 1–4. A Known player passes a Trusted-level check only with help or a vouch, and nothing compresses the
ladder further. The built `PersuadeScore` is untouched.

**Results** (missions' rule, on top of the score):

| Your score | Result | Where it goes |
|---|---|---|
| at or over the difficulty | **Pass** | the pass node |
| one under | **Short**: a halfway deal at a price the choice names (Favor, food, a promise, a smaller result) | the short node |
| two or more under | **Refused**: that speaker won't hear that approach from you again this job | the refused node |

A choice can name two targets (`-> pass | fail`); then Short and Refused both go to the second. Scores are
fixed and shown, so picking a choice you're short on is a choice. One try per person per job per identity.
A failed check changes things only for the player who failed: nobody can fail on purpose to raise the price
for others.

**Lies** have hidden modifiers on top of D:
- the speaker's credulity: 0 or +1, seeded per speaker and never shown;
- +1 for a Paranoid speaker;
- +1 if this speaker caught you lying this life.

Pass or fail is never shown in advance. A tell catches the lie whatever the score
([section 10](#10-lying-and-being-lied-to)). So a spreadsheet can give a range, never a certainty.

**Threats** use the Fear score and its rules ([MISSION_STYLE.md](MISSION_STYLE.md#checks-persuade-and-fear)):
- +3 Infamy with F, pass or fail (a scene may name more, up to 8), and regard *threatened*;
- once per person, by anyone, for the season: after one threat by anyone, *"He's already scared. Of someone
  else."*;
- three per life per faction; after that it's always Refused (*"They're past scared."*) and still costs its
  Infamy;
- a failed threat is barred at that speaker for a real day, per player;
- never remembered as a pass;
- never on stronghold leaders (*"The boss doesn't scare."*), the Kid or the Long Table. A free-standing talk
  threat is never offered inside a safe zone (strongholds, the lab) or a safehouse. Lore: guns stay quiet
  in camp, and nobody fires near a house. A threat a mission writes into its own scene (Protection Racket's)
  stays.
- The tag carries the cost, the score against the difficulty, and the Hunted-cliff warning when the cost
  would make you Hunted: "[Threaten · +3 Bastion Infamy · this makes you Hunted]".

**Speakers with no faction** (strangers, factionless quest people, the Long Table):
- persuade reads your Haven rank, at most 2 (they've heard the Haven's channel); ★ and the +1 dialogue bonus
  apply. The Long Table is the exception: its rank counts 0;
- Fear reads your highest earned Infamy with anyone, at most 2; the +3 is charged to the Haven, which counts
  what's done to ordinary people. At Kill on Sight they won't listen;
- there's no liar count;
- the Long Table keeps no regard at all. It stays three words, so it never becomes a hidden fifth
  reputation.

**Repeats:**
- A passed persuade or standing check is remembered per character, keyed by (speaker key, conversation,
  choice id). The choice leads straight to its pass node again.
- Lies, threats, looks and anything whose truth can change over time are never remembered. Each telling is
  checked again and stores a new claim.
- A failed persuade answers "You've said that already." until your score for that check is higher than when
  it failed. The record is per player, so dying doesn't reset it.
- A failed lie or threat is barred at that speaker for a real day, per player.

**Failure is a branch.** The lint refuses:
- a check whose fail node is `end`;
- a fail node that offers only Leave.

A failed persuade still leads somewhere: the speaker names someone who might help, the price goes up, or
you get half the truth.

**What's shown:**
- The panel and the menu show every standing, persuade, threaten and look choice. One you'd fail is red,
  with what it needs: "[Persuade · Haven · 1 of 2] (needs Trusted)".
- Background lines show only to that background.
- The action menu hides checks you'd fail. The node's text adds one line instead: "There's more they'd say
  to someone they trusted."
- After a check, the reply opens with "[Persuade · Haven: passed]", or in the hint fallback with the title
  "Persuaded", "Short" or "Not persuaded".

**Groups.** In a decide scene ([section 14](#14-multiplayer)), the best talker present can answer for
everyone. That's intended ("send your Elder to the chair"), and decide-scene difficulties assume it. Each
member still has their own one try per job. In a tally scene, each player sways only their own voter.

## 9. Results (effects)

Effects are joined with `;`. They run in order, on node entry (`do` on its own line) or with a choice. On a
check choice, `do` runs on a pass, `short` on a Short, and `else` on a fail. Each effect goes through the
system that owns it, with that system's caps.

| Effect | Example | Does | Limit |
|---|---|---|---|
| `set`, `qset` | `set:ask.tins@house=1` | season and run flags | world effect: only in a scene. In a conversation that serves many speakers, a flag is scoped (`@house`, `@seat`, `@run`) |
| `mark` | `mark:other_two.sorry` | a season flag that only ever goes from 0 to 1 | allowed anywhere |
| `cset`, `nset` | `nset:honest` | a character flag; this speaker's memory of this character | personal |
| `regard` | `regard:thanks` | the regard words | `thanks` only with a real cost or deed on the same choice |
| `favor` | `favor:haven+5` | `AddFavor` | 10 per conversation; all talk shares **20 per faction, per player, per real day** (the small-pay ledger's talk line). Past it: "That's all I've got today." The choice still works, unpaid |
| `cost` (on the choice) | `cost favor:haven 20`, `cost item:food*3` | `SpendFavor` or `take` | tagged |
| `take`, `return` | `take:food*3`, `return:keepsake` | takes items by keyword; gives back only an item this character handed this speaker | never gives new items |
| `claim`, `promise`, `later`, `learn` | `promise:tins@house:food*2`, `later:6h:scav.came_back`, `learn:halcyon.back_room` | memory, promises, callbacks, knowledge ([sections 10–11](#10-lying-and-being-lied-to)) | 6 claims, 3 promises, 10 callbacks per player |
| `note`, `rumour` | `rumour:"..."@6` | a journal note on the run; gossip (`AddRumour`) | the existing caps |
| `reveal` | `reveal` | the exact spot of the speaker's contract | only to players the contract's own rule allows (Trusted or a Radio Operator, as built); never behind a lie or a disguise |
| `offer`, `begin` | `offer:leader_job`, `begin:other_two` | `StartIfCan` for this player and their vanilla group within 50 m; a questline's `TryBegin` | the mission's and the engine's own rules |
| `errand` | `errand:house_tins` | asks the errands dealer to deal that card or routine to this player | the dealer's rules ([ERRANDS.md](ERRANDS.md)); never on a leader |
| `start`, `event` | `start:walk_back` | the stage mission's block or event (`OnDialogueEvent`) | the physical half of an outcome |
| `resolve` | `resolve:all_in:"talked the head round"` | the stage's `Resolve`, with a **named** outcome | only from a conversation attached to that running stage's person; only outcomes talk finishes |
| `approach` | `approach:talk:"asked about the stores"` | `Remember`: this player took part this way | personal |
| `sway` | `sway:pot` | `RZ_QuestVote.Sway` | the vote's own rule (one sway per player per sitting) |
| `reward` | `reward:haven:5:20:"walked them home"` | `Run.Reward` | quest conversations only; the engine's caps |
| `call` | `call:work`, `call:rob`, `call:hand_in:food*2`, `call:toll:15`, `call:search`, `call:declare_bite`, `call:table_bite` | runs built behaviour unchanged | its own rules |
| `hostile`, `flee` | `hostile:3` | a timed grudge (`AddGrudge`); the wanderer runs | grudges restart, never add up |
| `pass`, `catch` | `pass:post`, `catch:bastion` | a passed look's result (one passage at this post while it stands, or the gate's 10 minutes); a caught look's result (held, kit taken, +3, one of three catches, word gets round) | the disguise system's own rules ([DISGUISE.md](DISGUISE.md#8-caught)) |
| `commit` | `commit:bastion` | the finale route lock | a confirm node, and the engine's `CanCommit` |

There's **no `fame`** (talk never pays Fame), **no `house`** (house numbers change through built hand-ins
and stage outcomes), and **no `infamy`** (Infamy comes only from lie, threaten and look checks and built
calls, so it always shows in a tag).

**Personal and world:**
- Memory, regard, character flags, standing, Favor, items, claims, promises, knowledge, sways and approaches
  belong to the player who picked.
- Season, run and scene flags, `resolve`, `start` and `commit` change the world. They're allowed only in a
  scene, so they run once ([section 14](#14-multiplayer)). `commit` is personal in effect, but it runs only
  from the finale's confirm node.
- These are allowed anywhere: `mark`, `call` to built per-player services (hand-ins, the toll, the search),
  `offer`, `begin` and `errand`.

**Talk resolves talk.** A choice resolves only what talk itself finishes: talked round, paid, refused.
Anything physical starts the stage's block with `start:`, and the stage resolves and pays when the block
ends:
- walking two people back to the road at dusk;
- an escort to Bastion;
- a delivery.

## 10. Lying and being lied to

There is **one lie store** for the whole game, per player per season. Every record carries an
**exposing condition**: what in the world would show it up. Dialogue's claims, missions' lies and errands'
lies all go in it. A lie pays at most 60% of what the truth would have, and never Fame (one rule for every
piece).

**The player lies.** A `[Lie]` choice has a `truth` condition about something the player knows.
- **True:** it isn't a lie. The tag doesn't show, the choice passes, and the claim is stored as true.
- **False:** a **tell** catches it first, whatever the score:
  - symptoms show, for a lie about a bite;
  - another faction's armband, ribbon or colours, for "I'm with X" or "I'm not with them";
  - your own kit with them, for "I'm not with them";
  - this speaker holds a different claim from you;
  - the writer's `tell` (`tell always` for a lie nobody could get away with: "Rudi sent me", because Rudi
    has been dead since Day 7).

  With no tell, the lie check runs. A claim to *be* one of them in their kit isn't a lie check: it's the
  look ([section 12](#12-claims-in-someone-elses-kit)).
- **Caught now:**
  - the fail node, and regard *lied*;
  - +2 Infamy with the faction (none with factionless speakers);
  - the faction's liar count +1. At 2, that faction's persuade and lie checks are −1, and the count drops 1
    each real day.
  - A lie about a bite at a safehouse while it shows is the built "caught hiding a bite" instead (+15 Haven
    Infamy, once per life). The two never stack.
- **Believed:** the pass node, and a false claim goes in the lie store with its exposing condition.

**Found out, events first.** A believed lie is exposed by the first of these:
- a visit by a player of that faction (who knows the truth, or whose presence tests it);
- road news that carries the truth;
- the owning system's tick (the family checks at 20:30; the post counts its roster; the book is read).

Only a claim with nothing in the world to test gets the **rumour tick**: the next half-hourly leader round,
and once more a real day later, 15% each, +10 for a Talker speaker, +5 for a Paranoid one.

**When it's found out:**
- regard *lied* and +2 Infamy;
- the speaker's next greeting says so ("You told me Bastion sent you. Bastion's never heard of you.");
- the choice's `found` effects run, but only on things still running (a passage revoked, a sway removed).
  Never on Fame already paid or a finished mission.

**Lies that move the world.** A lie picked in a scene (a sway, a decision) is recorded on the scene with the
liar's identity. A questline can read `scene_lied:<scene>` later: a callback, a harder stage, a name in a
headline (through the names policy). The world effect stays. Lying worked; it just isn't free.

**Claims the world tests.** Where the speaker could simply wait and see, the claim's exposing condition is a
later check instead of a roll. "They're expecting me back by six": the family checks at 20:30.

**The player is lied to.** A `say` line ending in `~lie:<key>` (bound if needed: `~lie:scav.pharmacy@target`)
marks a lie. Showing it sets `heard_lie:<key>` for the player and for anyone who overhears a public node. The
truth sets `knows:<key>`, from any of these:
- a clue;
- another speaker's `~truth:<key>` line;
- a background line ([Police] "He keeps looking at the door.");
- a cracked secure channel or a journal note;
- a **world predicate** checked when the player gets there. For example `town:thick=@target` is true the
  moment they walk into that town while its crawlers are thick.

Once a player has both, the conversation's `callout <key> -> node` choice appears, tagged [Evidence]. The
journal's "What you know" lists it ("Grigor said the cellar was empty. It wasn't."). Givers who lie are just
conversations and briefs with `~lie` lines; the mission owns the truth, in its truth deck.

**Hiding things.** Behind trust, evidence or a check. The default deflection is one line in voice ("Ask me
something else."). Beside it sits a Press choice (persuade, or threaten where allowed), with its fail branch.

## 11. Promises, callbacks and knowledge

**Promises** are a held-errand kind with their own clock. `promise:<kind>@<scope>[:<what>]` ties to a built
event:
- food handed in at that house;
- an item at a place;
- a visit within a time window;
- someone walked home;
- a message told to a named occupant.

How they run:
- **Clock:** 2 hours of the promiser's own online play, at most 3 real days. Logging off overnight doesn't
  break one.
- **Limit:** 3 open per character.
- **Where you see them:** "Promises" rows in the journal, with the time left. `/t` with no number lists them
  too.
- **Kept:** regard *kept* and a thank-you line next time.
- **Broken** (time up, or death): regard *broke* and a line ("You said you'd bring tins. I told the kids.").
- **No double pay.** A kept promise pays nothing beyond the built event's own pay: a food hand-in pays as
  built. A visit pays nothing.

**Callbacks.** Dialogue's `later:<time>:<key>`, missions' echoes and errands' follow-ups are **one callback
queue**:
- words only: a reaction topic, one bark, or one notice in the speaker's voice;
- checked on the 5-minute tick, on played time, near the subject;
- 10 per player (their own oldest go first), and a separate system pool of 100 for quests and systems, so
  nobody can push out someone else's;
- each expires after 3 season days;
- at death, a waiting one becomes an unsigned board note at its subject ("For the man who brought the
  Dvoraks in, if anyone sees him").

**Knowledge** is recorded on the server when it sends a player a line that carries a key:
- open-channel chatter;
- secure lines (including the word at the wire);
- clue reads and leaders' news;
- `~truth` lines and questline notes;
- world predicates.

It's per player, like the journal. Evidence keys (up to 96) last with their run or the season. Passing facts
(up to 32) expire.

## 12. Claims in someone else's kit

[DISGUISE.md](DISGUISE.md) owns kit, colours, the look, catches, word gets round and vouching at barriers.
Dialogue is where a claim is spoken. The built AI behaviour doesn't change.

1. **Kit has a kind**, read by the disguise system (`kit:` in [section 7](#7-conditions)): `own`, `issued`,
   `in_kit`, `signed_other`, `dead_mans` or `off_turned`. People react to kit without any claim: "Kit suits
   you." (own); "You're still in our kit. Take it off before someone asks why." (issued); "Where'd you get
   that jacket?" (in their kit). The true answer ("Off a dead man.") costs nothing.
2. **A disguise needs intent.** You're claiming only when you pick a claim line ("[Look · Bastion] 'I'm one
   of yours.'") or answer as one of them at a barrier. Nobody is ever "pretending" by accident. Your own kit,
   worn at Known or better, is never a disguise.
3. **The look decides it.** A claim runs DISGUISE.md's fixed look, with its tags on the choice before you
   pick: "[Full kit +2] [Name: Sgt Hruby +1] [Watched −1] 2 of 2". There are no random challenge odds and no
   suspicion meter. The Hunted can't claim at all, so the claim line doesn't show for them.
4. **Cover details are things you can say.** A choice that speaks one carries `cover:<key>` and adds +1 at
   the look:
   - **Bastion's word at the wire.** It changes at 05:30 and is read on Bastion's secure net (a Radio
     Operator hears it). Three words are offered, shuffled daily, and one is right. The word itself is the
     key: a friend can tell you over voice. The right word is +1; **a wrong word fails at once**. Saying you
     haven't got it runs the look without the detail.
   - **A real name you've heard:** a leader's, or a dead man's from the nightly call. A dead man's name at
     his own post fails at once.
   - **A crew name** at the Red Hand ("What do they call you?"): your own, or one you heard on the open
     channel.
   - **A notebook number** at Halcyon ("Notebook number?"): only from a dead researcher's notebook.
   - **A mission's paper,** where the stage provides one.
5. **No rewards through a claim.** It never changes your tier, shop, Favor, Fame, services, perks or the
   contracts offered. The lint refuses `reveal`, `offer`, `begin`, `learn`, `errand` and Favor on anything
   reachable only through a claim. All a claim can get you is lines, `pass:`, and a stage's own trick branch.
   While you're passing as a faction, your persuade score with it counts as 0 and you can't threaten its people.
6. **Passage is small**, as DISGUISE.md sets it: a passed look at a road post clears you there while it
   stands (no search at a Bastion checkpoint, no turning back at a Red Hand roadblock); a passed look at an
   outpost gate gives 10 minutes inside. Camp services and safe zones still go by your real standing.
7. **Leaders and home ground see through it.** Stronghold and seat leaders of that faction, everyone inside a
   stronghold's or the lab's wire, and the Red Hand camp at the dusk meal catch any claim at once. The one
   exception is a Rogue leader's price (DISGUISE.md, section 10).
8. **Caught** is DISGUISE.md's one cost: +3 Infamy (never taking you into Watched on its own), the kit taken,
   one of three catches per life per faction, and word gets round at that holding. A conversation does this
   with `catch:<faction>`.
9. **Signed for.** Kit a quartermaster sold is signed out to its buyer. Worn by anyone else, it fails the
   look at once: "That's Weber's jacket. Signed for."
10. **Point them out.** A player Known or better with a faction, within 8 m of a player who is claiming in
    that faction's kit, sees "[Point them out] 'That one's not yours.'". Each point-out is −1 at that look
    (two at most), shown in the tags. Wrong (it was their own kit): the pointer takes regard *slander* with
    that speaker. Once per pair per hour. The Coat Run's call-outs at the clean room door are the same
    mechanic ([FINALE.md](FINALE.md#the-coat-run-independent-once-a-season)).
11. **Changing clothes ends the session.** Kit is checked on every pick and every 5 s. Any change ends the
    session ("...Didn't you just—? Never mind. Off you go."). So nobody opens a hub in one outfit and picks in
    another.
12. There's no disguise with the Haven (no uniform; its mark is the bear on its doors, not on its people) or
    the Long Table.

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

- **Private by default.** Each player has their own session, checks, memory and tags. Two players at the same
  chair can see different choices.
- **Who hears what.** Only two distances:
  - **8 m: overheard.** A `public` node sends its lines and the chosen label, with tags stripped, to other
    players within 8 m: "Mara, to the chair: 'Read it out.'". It goes owner-only, at most once every 10 s per
    listener. A `~lie` line in a public node marks its listeners `heard_lie` too.
  - **30 m: the room.** Scene results and holds go to everyone within 30 m.
- **Who took part:** 60 s within 150 m, or a step done (the shared participation rule).

**Scenes.** A conversation can declare `scene <id> <scope> [who <conditions>]`. The scope is run, speaker,
house or season. There are two kinds.

**Decide (small shared moments).** Choices marked `decides` settle the scene for the world.
- Only players who meet `who` can pick one. `who` defaults to the stage's own requirement, and to having
  taken part. Others see the choice dimmed: "(only someone the house trusts can answer for it)".
- **The hold.** A decisive pick starts a **5-second hold**, announced to the room: "Mara is about to answer for
  the house: all four come in."
  - Any other eligible participant in the room can **Object**, once per job. The hold is cancelled, and the
    objector becomes the speaker for 60 s.
  - Anyone in the room can **Stand with** the decider during the hold.
  - When the hold ends unobjected, the pick writes the scene's record: who, which choice, when, and whether it
    was a lie.
- World effects run once. The result node's entry effects run once, for the decider. Personal effects go
  only to the picker.
- **With the decider:** the decider, their vanilla group within 150 m, and anyone who chose Stand with share
  the result's pay. A bad mark lands on the decider alone. Everyone else who was eligible and did the work
  gets half the Favor from the giver, with no Fame and no Infamy.
- Everyone else in the scene moves to the result: "{decider} has already answered for the house." Later
  visitors are routed by `jump decided`.
- Every node in the scene checks the record on each pick, so no sibling path can decide twice.

**Tally (anything that matters to the season).** `sway:` choices add one contribution each to a count the
owning system keeps. For the majors' sittings that's `RZ_QuestVote`, which allows one sway per player per
sitting. Nobody wins by being first: the owner counts at its own time (the vote at 20:30). Menu, panel and
chat players are equal. Errands and their memory never move a vote.

### Vouch

One vouch mechanic, shared with disguise's sign-ins at barriers and the errands' Vouch card.
- **Who, in talk:** a player Trusted or better with the speaker's faction, within 8 m of another player who is
  talking to that speaker. They see "[Vouch for {name}] 'They're with me.'". (At a barrier, DISGUISE.md's
  sign-in rule says who.)
- **What it gives:** for 5 minutes, while the voucher stays within 8 m, the other player's persuade and lie
  checks there get +1. This is inside the +1 dialogue cap.
- **What it costs:** if a lie told under the vouch is found out (at once, or later by events or the rumour
  tick), the voucher takes +5 Infamy with the faction (once per vouch), regard *vouched for a liar*, and
  can't vouch for 2 hours.
- **Limits:** once per pair per hour, and once per vouched player per speaker per life.
- It costs the vouched player nothing and works only while the voucher is standing there, so there's no
  consent step.

### Call it out

- **When:** for 20 s after **any** claim-carrying choice in a public node, true or false. Tags are stripped
  from overheard lines, so the call-out showing up tells nobody anything. Others within 8 m see "[Call it
  out] '{name}'s lying.'".
- **If it was a lie:** the liar is caught now, and the lie's effect is undone if it's still running (a sway
  removed, a passage revoked). The caller gets nothing: no regard, no Favor. So a throwaway character has
  nothing to launder.
- **If it was true:** the caller takes regard *slander*, and can't call out at that speaker again this life.
- Once per pair per hour.

**Point them out:** [section 12](#12-claims-in-someone-elses-kit).

- **No double pay.** Paying effects are once per character per choice. Mission and questline pay follow their
  own participant rules.
- **Late joiners and restarts.** Decisions are scene records and flags (saved), so a late player meets the
  result ("The council already voted. Workers first.").
- **AFK.** Idle sessions end at 120 s (45 s while someone waits). They hold no scene: decisions live in
  records, not sessions.

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

## 16. Barks, radio and strangers

### Barks

Barks are `bark` conversations: one line, conditions, no choices.
- They're said owner-only to a player passing within 6 m of the speaker's body, checked on the 1-second tick,
  and only for bodies near players.
- Limits: one per player per 60 s, and one per speaker per player per 10 minutes. Epilogue barks: once per
  speaker per player.
- They're the cheapest environmental storytelling there is. They flag an ask ("You're the one who brought the
  tins?"), read the hills ("The big one's calling tonight. Not past the chapel."), and carry most of the in-game
  ending, because most players won't press Talk on every occupant.

### Radio

`radio:<channel>`:
- **Where:** "Call on the open channel" at a house radio with power, or on a carried radio. There's no body, and
  it works on any map.
- **What:** answer a distress call (a stranger on the air), call the Haven's channel in with news, haggle a toll
  with a crew.
- **Who hears it:** a public radio node is heard by players within that channel's built reach, as one line with
  tags stripped. **No name goes on air** except a player's own broadcasts (their radio-station presets) and honour
  lines: the names policy.
- **Secure nets:** Bastion's net, the Red Hand boss's and Halcyon's lab can be called only by a Radio Operator
  who has cracked them, as built.
- It's where the finale's Independent route is committed ([section 18](#18-finale-and-the-last-week)).

### Strangers

Strangers are "help a random person" errands. [ERRANDS.md](ERRANDS.md) owns the catalogue, the dealer, the pay
and the budget; dialogue gives them a voice.

**Kinds to start:**
- **hiding:** someone shut in a house at a town's edge, with the town's crawlers between them and the road;
- **wreck:** a lone survivor by a crashed car;
- **grave:** someone digging, with a name;
- **roadside:** a broken car and two kids;
- **walk_out:** a man a safehouse walked out, still walking;
- **campfire:** someone at a fire who's been waiting for anyone at all.

**Where a stranger can find you** (decision 14). Only where it would make sense for one to be, and never in the
empty middle of nowhere:

| Place | Why someone would be there | Kinds that fit |
|---|---|---|
| A road or crossroads people use: a truck route, a road post's road, a road with recent traffic | People walk where the traffic is | roadside, walk_out, wreck |
| A wanderer route | They're following the same road the wanderers use | walk_out, roadside |
| Near a safehouse or outpost (outside its wire, 100–400 m) | Walking towards help, or turned away from it | walk_out, hiding |
| A campfire | Fires draw people at night | campfire |
| A stopped car: yours, stopped on a road for 20 s or more, or a car stopped on the verge | Someone walks up to a car that's stopped | roadside, wreck |
| Near a fresh event or a body (a lost truck, a fight, a death this season hour) | Someone saw it, or lost someone in it | wreck, grave |
| A town edge at dusk | People come out of hiding when the light goes | hiding, grave |

- **Never:** in open country away from all of these; on a timer; inside a holding; within 2 km of the Long
  Table's farm; inside the 300 m no man's land round a lived-in place for the kinds that hide from it.
- **Only a reason, never a clock.** A stranger is dealt when the player reaches such a place and the place has a
  stranger to give (each place has its own cooldown). Time passing on its own deals nothing.
- **The budget is a ceiling only:** never more than one stranger per player every 15 minutes, inside the
  unasked-contacts lane, and at most 6 stranger bodies on the server (the shared body budget).
- **Body:** only while a player is within 150 m. They're gone when they're done, or after 20 minutes out of
  range. Placed as data first, within 300 m of the player.
- **What they ask:** each has a conversation and at most one small block: an escort that ends at a place
  (at most 600 m), a message to a safehouse, a tin. They pay through the errand rules, never items.

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
- **Callbacks.** The asker's next greeting or bark reflects how it went, through the one callback queue.
- **Talk pay:** talk alone pays at most the talk line of the ledger (20 Favor per faction per real day), never
  items, never Fame. Errand pay is errands'.

## 18. Finale and the last week

### The finale's commit

[FINALE.md](FINALE.md#4-the-lock) owns the routes, the lock and its rules. Dialogue gives it the confirm node.

- **Where routes are taken.** Each faction route's topic is an `always` topic on its stronghold leader and on
  its Loyal seat leaders. So a player whose stronghold leader refuses them, or who is far away, can commit at
  another seat. The Independent route is committed on the open channel, at any powered house radio or a carried
  one. The Doctor wanderer gives its pitch; if the Doctor is gone, the Courier, else an unsigned sheet at the lab
  gate.
- **Before 90%.** From 75% the topic shows "Ask me again when it's nearly done." From 80% the pitches are on
  demand. Nothing locks before 90%.
- **Committing.** `commit:<route>` only runs through a confirm node tagged [Point of no return]. The engine
  refuses it unless `CanCommit` is true:
  - research is at or past the 90% gate;
  - the stand hasn't been won;
  - this identity hasn't locked this season;
  - `CanSideWith` says yes, by parley (you're owed), or by price (the route's price door). The Independent route
    is always open.

  No conversation text can override this. The lint refuses tier, standing or memory terms on a commit choice:
  the finale decides who may commit, not the writer.
- **The confirm node** shows the finale's three plain lines: the route's **admission**, **what you bring** (your
  assets and notes for this route), and **what you leave** (assets for other routes go dormant).
- **The lock** is per identity for the season, so dying isn't a way out. It ends at the cure.
- **Late joiners** and the undecided can commit until the stand is won.
- **After the lock** route lines are face to face only, cool on the first visit and back to normal over two
  real days. Shared channels never single out anyone's route. `route:none` players in the last 10% are treated
  as bystanders, and any route topic still offers them the commit.

### The last week

After the cure the epilogue lasts **one week** (168 hours; decision 12). [EPILOGUE.md](EPILOGUE.md) owns its
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
  ([MAJORS.md](MAJORS.md)).
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

Files go in `dialogue/<area>/<name>.rzd` at the repo root. The areas are base, haven, bastion, redhand,
halcyon, wanderers, residents, table, strangers, radio, barks, quests/<key>, majors/<key>, errands, finale and
epilogue. A file holds one or more conversations. `#` starts a comment, and indentation is free.

```
conv <id>                       unique, dotted, never renamed once shipped
for <speaker>[, <speaker>]      section 3 (seat:haven/council_head names one seat kind)
when <conditions>               read when the hub is built, and again on every pick
topic "<hub label>" | greet | bark
prio quest|ask|world|reaction|flavour
once | daily                    per character, per speaker key
always                          shown even when the speaker refuses (finale routes only)
opens approach <metres>         the server opens it when a matching player comes that close
voice haven|bastion|redhand|halcyon|table|plain
scene <id> run|speaker|house|season [who <conditions>]
bind <name> = <picker>          picked once a session: town:thick_within(2500), house:nearest_short
use <conv id>                   pull in shared nodes

node <id> [shared] [public]
  jump <conditions> -> <node>   checked in order on entry
  say <text> [~lie:<key>[@<bind>]] [~truth:<key>]
  say? <conditions> :: <text>   the first true variant replaces the next plain say
  say (<role>) <text>           someone else in the scene
  do <effects>                  on entry (in a scene: once, for the decider)
  > @<id> <label> -> <pass>[ | <short>][ | <refused>] [check <kind> <args>] [cost <what>] [truth <cond>]
      [tell <cond>|always] [if <cond>] [do <effects>] [short <effects>] [else <effects>] [found <effects>]
      [decides] [label "<40 characters>"]
  callout <lie key> -> <node>   appears with [Evidence] when the player holds the truth
```

- **The rules of a line:**
  - A label is everything before the first ` -> `. Speech goes in quotes; actions are plain words ("Put three
    tins in the stores.").
  - Limits: labels 60 characters (40 in the action menu, `label` if needed), says 240, 7 choices a node.
  - Faction keys are haven, bastion, redhand, halcyon, or `none` for factionless checks. Background ids are as
    in code.
- **Ids.**
  - Every choice has an `@id`, unique in its conversation. Memory is keyed by conversation, node and choice id,
    never by position, so moving or adding lines never scrambles saved records.
  - `dialogue/shipped.lock` lists every shipped id. The lint refuses removing one; it can only be marked
    `retired`.
- **Tokens:** the world tokens (`Fill`) and the per-player tokens (`FillFor`), plus:
  - `{player}`, `{npc}`, `{npc_short}`, `{house}`, `{resident}` (a seeded occupant name for this house);
  - `{tier:f}`, `{rep:f}`, `{word:f}`, `{claim:key}`, `{bind:name}` (as `@name` in keys and `{town:@name}` in
    text);
  - `{decider}` (who decided the scene, through the names policy);
  - `{lean}`, `{tribe:N}`, `{alpha:N}` (in the speaker's faction's own word), `{word:bastion.N}`,
    `{kit_owner}`, `{dead_name}`.

**Pipeline**
1. **Text source.**
   - **Plan A:** the server reads the `.rzd` files as addon resources at start (VERIFY: `FileIO` reads addon
     files on the dedicated server). Clients compile nothing.
   - **Plan B:** `tools/dialogue_pack.py` writes `scripts/Game/ReforgerZ/Dialogue/Pack/RZ_DialoguePack_<area>.c`,
     generated and committed. It's split into many small functions (at most 150 `Add()` lines each).
   - Either way, anything in the addon is on every client's disk. Only the optional
     `$profile:ReforgerZ/dialogue/*.rzd` overrides, matched by conversation id, stay server-only.
   - The voice bank (`RZ_VoiceDefaults.c`) holds single-line pools; `.rzd` holds conversations, greets and
     barks. One text pipeline, one lint.
2. `tools/compile_check.sh` runs the lint first and fails on lint errors.
3. The server parses everything once at start into compiled tables. Conditions and effects become small typed
   records, never parsed again.

**Lint** (one lint for all player text, with a baseline that only shrinks)
- **Errors, structure:**
  - unique ids and stable choice ids; a shipped id removed;
  - a missing target; an unreachable node; a path that can't end;
  - unknown keys, effects or tokens;
  - more than 7 choices; length limits.
- **Errors, checks:**
  - a check failing into `end` or a Leave-only node;
  - a lie without `truth` or `tell always`;
  - a `truth` that reads anything the player doesn't know ([section 7](#7-conditions));
  - `regard:lied` outside a lie check;
  - `regard:thanks` without a cost or deed on the same choice;
  - a free-standing `threaten` in a conversation whose speaker is in a safe zone or safehouse, or on a stronghold
    leader, the Kid or the Long Table.
- **Errors, effects:**
  - a world effect outside a scene;
  - `decides` outside a scene;
  - an unscoped season flag in a conversation that serves many speakers;
  - a flag outside its owner's namespace, unless QUESTLINES.md's Crossings lists it;
  - `fame`, `house` or `infamy` effects; Favor past the caps; `reward` outside quest conversations;
  - `resolve` with an outcome name the questline doesn't declare (`dialogue/quests/<key>/outcomes.txt`,
    cross-checked by the runtime test), or in a conversation not attached to a stage person;
  - `reveal`, `offer`, `begin`, `learn`, `errand` or Favor reachable only through a claim or behind a lie;
  - `errand` on a leader;
  - `commit` without a confirm node, with tier, standing or memory terms, or outside an `always` topic.
- **Errors, content:**
  - an ask with no world condition;
  - any follow or companion effect, or an escort without a destination;
  - a bark with choices or more than one line;
  - in `voice table` files, the banned-word list (no word for what they are, no kitchen or cellar contents);
  - a name from LORE.md's Long Table (Halas, Tomas, Moravec) in any non-table file;
  - "alpha", "tribe" or "crawler" in player text (each faction has its own words);
  - `{player}` or a name token in a radio node, except a player's own broadcasts and honour lines.
- **Warnings:**
  - voice hints: Bastion lines with exclamation marks, Halcyon without numbers in long trees;
  - more than 3 says;
  - he, she, him or her in lines about a seeded speaker.

**Simulator.**
- `tools/dialogue_sim.py <conv> --bg doctor --tier haven=trusted --flag every_name.act=1` walks a tree and prints
  what that character sees, tags included.
- `--chars "doctor:haven=trusted" "unemployed:haven=known" --race` runs several characters through a scene in a
  set order, to test decide holds, objections and tallies.

**Runtime test.** It opens every conversation's start node with a fake player, runs each lie, promise and
callback once, and logs anything that throws.

**Debug.**
- `/talkdebug` (admins): the nearest speaker's candidate conversations, and which `when` terms failed.
- `/talkreset <scene>` (admins): clears a stuck scene.
- Log lines:
  - `Dialogue: loaded N conversations, M nodes`;
  - `Dialogue: lint warnings N`;
  - `Dialogue: scene <id> decided by <name> (<choice>)`;
  - `Dialogue: scene <id> objection by <name>`;
  - `Dialogue: session cap at <speaker>`;
  - `Dialogue: budget reached <player> <faction>`.

**Writing.** The rules go in the writing guide (`docs/WRITING.md`, planned: it takes the writing guide out of
LORE.md and tone's rules from [TONE.md](TONE.md)):
- **Lines:** one to three short lines a node, in the faction's voice, coloured by the speaker's traits.
- **Choices:** no choice is labelled good or evil, and every option is something a person would say or do. Every
  option costs something real, and the kind one usually pays least.
- **The player's own voice by background:**
  - Ex-Con and Raider: wry, quick ("[Lie] 'It's a rash.'");
  - Unemployed and Drifter: plain, sometimes funny;
  - Soldier and Police: procedural;
  - Nurse, Doctor and Lab Technician: clinical and kind;
  - Hunter, Mechanic and Radio Operator: practical, few words;
  - Elder: slow, and knows everyone.
- **Dark humour:**
  - the Red Hand jokes about everything, and says its lies flatly;
  - the Haven is darkly funny, sideways;
  - Bastion is dry ("Bastion out.");
  - Halcyon's is accidental guilt.
- **Where the humour stops:** jokes go up, sideways or at oneself, never down at the bitten, volunteers,
  exiles, children, the never-arrived or a named dead person. Never at the Long Table, never about the Kid,
  never in a node about a walked-out person's family, never about how Rudi died. Mercy jobs carry no jokes. A
  wry option is never the only way through.
- **The Long Table is never described**, and no line elsewhere hints at it.
- **Names:** frozen at the deed and passed through the note filter; `/anonymous` and the admin strike are
  honoured; a name is never a joke's butt; never on air except a player's own broadcasts and honour lines.
- **QA checklist for writers:**
  - run the simulator for three backgrounds;
  - read it aloud in voice;
  - every fail goes somewhere;
  - asks read the world;
  - nothing free;
  - no pronouns for seeded people;
  - the Long Table rules;
  - the three humour tests.

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
| Errands | `errand:` asks the dealer. Strangers, askers and named people are speakers. Errands write regard only on named-person keys |
| Majors | `sway:` goes into `RZ_QuestVote` (a tally). `{lean}` reads it. "Ask about..." topics sit on other stronghold leaders while a stage is open. Set pieces and codas are public scenes. `was:` reads the decider lists |
| Tribes and crawlers | "Ask about the hills" at a safehouse. `{tribe:N}` and `{alpha:N}` in each faction's own word. Knowledge from kill sites, abandoned lairs and the calls. Asks and barks |
| Disguise | Kit kinds, claims as look choices, cover details, point-out as a −1, and the session ending on a change of clothes. AI behaviour unchanged |
| Finale | `always` route topics, the confirm node with its three lines, `CanCommit`, `commit`, `route:` |
| Epilogue | Leaders' topics, codas, closing stages, hand-ins, the barrier's "Name?", greets, reactions and barks reading `cure_held`, `ending`, `was`, `route` and memory |
| UI | `PAYLOAD_DIALOGUE`, the panel, the slot donors, the hint fallback, `/t`, the optional menu, and journal rows for promises and "What you know" |

## 21. Performance and saving

- **Cost:**
  - Talk builds a hub from the indexed list for that speaker kind: 20–60 conversations with compiled
    conditions, a fraction of a millisecond.
  - Picks are event-driven.
  - The 1-second session check (range, idle, danger, kit) runs at most 64 sessions against the spawner's tracked
    infected (at most 60).
  - Barks are checked only for bodies within 6 m of a player.
  - Strangers are placed as data; a body exists only within 150 m of a player, inside the body budget.
- **Network:** one owner-only record per node change, at most 2,000 characters. Overheard lines are small
  owner-only records.
- **Saving.** Dialogue memory has its own file, `$profile:ReforgerZ/dialogue.json`, written at the 2-minute save
  only when something changed:
  - regard, failures, liar counts and knowledge per player;
  - seen nodes, claims, promises and faces per character;
  - scene records.

  The lie store and the callback queue are the shared stores (saved with world memory). It's about 3–5 KB per
  active player, so a 100-player season is well under 1 MB (VERIFY on a busy server). Records decay on load, are
  dropped at death or by age, and are wiped with the season. `state.json` doesn't grow.

## 22. Phases and the first content

Dialogue lands across the plan's build order. Each step is playable on its own, and Dialogue off is today's game.

| Plan phase | Dialogue builds |
|---|---|
| 0. Fixes and spikes | The VERIFY spike (one evening): per-player donor labels, `PerformAction` on the client, replacing a hint, `FileIO` on addon files, moving with a menu open. The results pick the delivery and the text source. The owner-only payload for all personal text |
| 2. Talk | The engine: sessions, hubs and slot donors; the panel (layout optional), the replacing hint and `/t`; `.rzd` with the lint, simulator, `shipped.lock` and runtime test; regard, claims and knowledge; `dialogue.json`. **About 30 conversations**, with Dialogue **off** by default and the legacy talk unchanged: the leaders' hub and the standing readout in four voices; first contact; the 8 wanderer roles; occupants (fed, short, after a death); two named people; two post speakers (Bastion's search, the Red Hand's toll); the Long Table's host and table; three asks. **Turned on** only after a play-test proves the per-player labels; otherwise the numbered `/t` fallback is used |
| 3. Small things | Errand asks through `errand:`, promises, callbacks and barks |
| 4. The mission style | Lies, tells and the lie store; the multiplayer layer (overheard lines, decide holds and objections, tallies, vouch, call-outs); claims as look choices, cover details and point-out (with disguise's phase 1); strangers' voice |
| 5. The majors | Sitting scenes and sways into `RZ_QuestVote`; "Ask about..." topics; set pieces as public scenes |
| 6. The finale | The commit topics and confirm nodes; route lines |
| 7. The ending | Leaders' epilogue topics, the codas, closing stages, hand-ins, the barrier, epilogue greets and barks |
| 8. Growth | Strangers and radio speakers in full; the dialogue screen and panel layouts, once they're built in Workbench |

## 23. Docs in the same commits

- **A new `docs/DIALOGUE.md`**, added only when the engine is built (decision 16). It owns the topic: how it
  works, the format, conditions, effects, checks, scenes, delivery and the lint. DESIGN.md gets a short section
  that links to it. CLAUDE.md's Doc map and the README's doc list change in the same commit.
- **REPUTATION.md:** regard and its scope table; lies and the liar count; the Fear check; vouch, call-outs and
  point-out; claims and signed-for kit; reputation-name tags; Make it right.
- **WRITING.md** (the writing guide moved out of LORE.md): dialogue lines, options and tags, the player's voice,
  where the humour stops, the names policy, the word at the wire, crew names, notebook numbers.
- **UI.md and UI/LAYOUTS.md:** the panel, the optional menu and their names, the slot donors, `/t`, and the
  journal rows.
- **WORKBENCH_SETUP.md:** no prefab changes, the settings, the VERIFY count and list, the log lines.
- **MISSIONS.md and QUESTLINES.md:** talk approaches through dialogue, strangers and asks.
- **ROADMAP.md, README.md** and CLAUDE.md's Doc map.

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

### D. A Scavenger who lies about a pharmacy (a giver who lies)

With a binding and a world-predicate truth.

```
conv wanderer.scav.pharmacy
for wanderer:scavenger
when !nmem:sold_pharmacy
bind target = town:thick_within(2500)
topic "Ask if he knows anywhere worth looking."
prio ask
voice plain

node start
  say The pharmacy in {town:@target}. Nobody's touched it. Clean as a whistle. ~lie:scav.pharmacy@target
  say Bit of food and it's yours.
  > @buy Give him a tin. -> sold   cost item:food*1   do call:scav_rumour:@target; later:6h:scav.came_back
  > @hunter "Shop doors open and the dust not touched. They're lying up in there." -> caught_out   if bg:hunter
  > @leave Leave. -> end
  callout scav.pharmacy -> caught_out

node sold
  say Pleasure doing business. Mind how you go. I mean it.
  do nset:sold_pharmacy
  > @leave2 Leave. -> end

node caught_out
  say ...All right. They lie up in the shops by day. All of them. But there's antibiotics in the back, honest. I'd want a tin for that much honesty.
  do nset:honest
  > @buy2 Give him a tin. -> sold   cost item:food*1   do call:scav_rumour:@target
  > @leave3 Leave. -> end
```

Notes:
- The town is bound once (a town within 2.5 km whose crawlers are thick) and kept with the heard lie and the
  callback.
- The truth (`knows:scav.pharmacy@target`) is set the moment the player walks into that town while its crawlers
  are thick. Next time they meet him, '[Evidence] "You said it was clean."' appears.
- The Hunter's own line exposes it at once.
- The rumour is the built Scavenger rumour (food for a lead), pointed at that town if it can be. Nothing new is
  given.

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

### H. The point of no return

[FINALE.md](FINALE.md#4-the-lock) owns the routes. Bastion's route at any Loyal Bastion seat, and the Independent
route on the radio.

```
conv finale.route.bastion
for stronghold:bastion, seat:bastion/outpost_lead
when cure>=75 & route:none & loyalty:loyal
topic "Ask where Bastion stands when it's done."
prio quest
always
voice bastion

node start
  jump !finale:open -> early
  say The cure goes in a crate, and the crate goes where we can count it. Are you with us on the lab road?
  > @with "I'm with Bastion. To the end." -> confirm   if finale:can_commit=bastion
  > @why "Why should it be you?" -> why
  > @notyet "Not yet." -> end

node why
  say Because we count. Everyone else just hopes. Ask me again when you've decided.
  > @back Back. -> start

node early
  say Ask me again when it's nearly done. Until then you're a civilian, and civilians change their minds.
  > @leave Leave. -> end

node confirm
  say From now until the cure, you stand with Bastion on the lab road. Our cure reaches those who comply, and the register is a census.
  say {lock:bring}. {lock:leave}.
  say Other routes' work won't have you. Dying doesn't change it.
  > @yes "Write me down." -> sworn   do commit:bastion
  > @notyet2 "Not yet." -> end

node sworn
  say Noted. One civilian, attached. Name goes on the roster. Bastion out.
  > @leave2 Leave. -> end

conv finale.route.independent
for radio:open
when cure>=75 & route:none
topic "Speak on the open channel about the cure."
prio quest
always
voice plain

node start public
  jump !finale:open -> early
  say (static) ...open channel. Go ahead.
  > @all "This is nobody. The cure goes to everyone. No crates, no lists." -> confirm   if finale:can_commit=independent
  > @off Switch off. -> end

node early
  say (static) Nothing to say yet. The lab's still working.
  > @off2 Switch off. -> end

node confirm
  say (static) Nobody guards it and nobody counts it, and every faction remembers who took it from them.
  say {lock:bring}. {lock:leave}.
  say (static) Once it's on the air, every faction hears that somebody said it. Other routes' work won't have you. Dying doesn't change it.
  > @say Say it. -> said   do commit:independent
  > @off3 Switch off. -> end

node said
  say (static) ...Somebody keys the mike twice. Then a woman: 'Morton heard you.'
  > @leave Leave. -> end
```

- The packer tags each commit '[Point of no return]'. `{lock:bring}` and `{lock:leave}` are the finale's lock-card
  lines for this player ("You bring: one proof (the black box in the stove)." "You leave: the woman's house, which
  you know.").
- Before 90% nothing locks, and the engine refuses a commit whatever the text says. Refusals don't hide these
  topics, and a Stranger who joins at 95% can still choose, until the stand is won.
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

### J. A small ask at a short safehouse, with a promise that can't be double-paid

```
conv ask.occupant.tins
for occupant
when house:short & !promise:tins@house=open
topic "Ask how the house is doing."
prio ask
voice haven

node start
  say Half portions since Tuesday. The children eat first. Don't tell the head I said.
  > @promise "I'll bring tins by tomorrow." -> promised   do promise:tins@house:food*2
  > @give Put two tins in the stores. -> given   cost item:food*2   do call:hand_in:food*2; regard:thanks
  > @leave Leave. -> end

node promised
  say I'll hold you to that. So will they.
  > @leave2 Leave. -> end

node given
  say ...That's two suppers for somebody. I won't say who.
  > @leave3 Leave. -> end
```

The hand-in is the built one: 2 Haven Favor an item, plus Fame per 3 items as built. A kept promise adds only
regard 'kept' and a line next time ("You came back. The Dvorak girl asked who you were."). Broken: 'broke', and
"You said you'd bring tins. I told the kids." The clock is 2 hours of the promiser's own play, at most 3 real
days, and the journal shows the time left. The ask only appears while the house really is short.

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

### L. Barks: the world reacting without anyone pressing Talk

```
conv bark.occupant.tins_kept
for occupant
when promise:tins@house=kept
bark
voice haven

node start
  say You're the one who brought the tins? The Dvorak girl asked who you were.

conv bark.haven.big_one_calling
for occupant, named:haven
when tribe:within(1500) & dusk
bark
voice haven

node start
  say {alpha:near}'s calling tonight. Nobody past the chapel after eight.

conv bark.epilogue.bastion_sentry
for named:bastion, post:bastion
when epilogue & cure_held:haven
bark
voice bastion

node start
  say Processing starts at 05:30, they say. Nobody's told us who processes us.

conv bark.epilogue.redhand
for named:redhand
when epilogue & cure_held:redhand
bark
voice redhand

node start
  say Cure's on the market. Price on application. Don't apply.
```

`{alpha:near}` is the nearest tribe's leader in the Haven's own word: its name if the Haven knows who it was, else
"the big one". Each bark is one owner-only line to a player passing within 6 m: at most one every 60 s per player,
and once per speaker per player in the epilogue.

### M. A stranger at a town's edge at dusk

Helping a random person, always about the world as it is, and only where someone would be.

```
conv stranger.hiding.dusk
for stranger:hiding
when town:edge & dusk
greet
opens approach 10
prio ask
voice plain

node start
  say Shut the door. Shut it! ...They lie up in the shops by day, and at dusk they come out to the noise. I've been in here since Tuesday.
  > @go "Stay behind me. We go now, before they're all out." -> going   do start:escort_house
  > @dawn "Wait for the morning. I'll come back." -> waiting   do promise:visit@here:dawn
  > @tin Leave him a tin. -> fed   cost item:food*1   do regard:thanks
  > @leave Leave him. -> end

node going
  say My name's Jiri. From the bakery. In case.
  > @leave2 Leave. -> end

node waiting
  say Morning. You said morning.
  > @leave3 Leave. -> end

node fed
  say ...Thank you. I'll make it last. I've got good at that.
  > @leave4 Leave. -> end
```

Outcomes:
- The escort is a small errand block that ends at the nearest safehouse (at most 600 m). He becomes +1 person
  there if he arrives.
- A promised dawn visit is kept if you come back between 05:00 and 08:00.
- If nobody comes, he may not be there in the morning. A Haven bark at the next house says "Jiri from the bakery
  never came out." No joke goes near it.
- He was dealt because the player reached a town's edge at dusk, not because time passed.

### N. Your own kit, and someone else's

A Red Hand crewman at a holding reacts to a Bastion jacket.

```
conv redhand.kit.jacket
for named:redhand
when kit:bastion=own,issued,in_kit,dead_mans
greet
prio reaction
voice redhand

node start
  say Nice jacket. Whose was it?
  > @dead "A dead man's. He won't miss it." -> laugh   if kit:bastion=in_kit,dead_mans
  > @mine "Mine. Bastion signed it out to me." -> cold   if kit:bastion=own,issued
  > @fence "Found it on a fence." -> laugh | cold   check lie redhand 1   truth kit:bastion=in_kit,dead_mans
  > @walk Walk on. -> end

node laugh
  say Keep it. Bastion'll shoot you in it. Saves us the bullet.
  > @leave Leave. -> end

node cold
  say Then you're on the wrong road, soldier. Walk.
  do nset:bastion_kit
  > @leave2 Leave. -> end
```

Kit in their colours isn't a disguise until you claim one: 'A dead man's' is the truth, and it costs nothing.
'Found it on a fence' shows [Lie] only to someone whose kit is really their own. Bastion colours still carry
Bastion's enemies: this crewman talks, but the fence and the quartermaster won't deal with you while you wear it,
and at a Red Hand roadblock you're turned round at 150 m ([DISGUISE.md](DISGUISE.md#4-colours-wearing-someones-war)).

---

## Engine needs

- **A `Dialogue/` folder.** All server-side, except the client cache:
  - `RZ_DialogueDB`: loads text resources or packs once, and indexes conversations by speaker kind;
  - `RZ_DialogueParser`: Split, IndexOf, Substring, Trim and ToInt only;
  - `RZ_DialogueCond`: compiled condition records, with one evaluator over the existing systems;
  - `RZ_DialogueEffects`: the effect runner, calling owning systems with their caps;
  - `RZ_Dialogue`: sessions, hubs, serials, the fresh-node guard, tells, overheard lines, danger, range, resume;
  - `RZ_DialogueScenes`: decide records, the 5-second hold, objections and Stand with, plus tally sways into
    `RZ_QuestVote`;
  - `RZ_DialogueMemory`: regard, faces, claims (into the shared lie store), promises, failures, liar counts,
    knowledge;
  - `RZ_DialogueBarks`;
  - `RZ_DialogueSettings`;
  - `RZ_DialogueClient`: the client cache and the donor labels.
- **Shared pieces it uses** (built once in the plan's phase 1): `RZ_Checks` (Trust, Fear, the look, Pass / Short /
  Refused), `RZ_Ways` (keyed by identity), the lie store, the callback queue, the small-pay ledger (talk's 20 a
  faction a day), the notice lanes, the body budget, `RZ_Names`, `Fill` and `FillFor`.
- **Speaker adapters** (`RZ_DialogueSpeaker`) for: leader seat, quartermaster, wanderer, occupant, named person,
  road-post speaker, quest person, Long Table role, stranger and radio channel. Each gives the kind, faction (or
  none), traits, loyalty, seat, memory key, place (house, town, holding, nearest tribe) and its built refusal
  rule. They're routed from the existing `RZ_PuzzleObjectComponent` tags (`RZ_Wanderers.TAG`, `RZ_Leaders.TAG`,
  `RZ_Safehouses.TAG_OCCUPANT`, `RZ_Territory.TAG`, `RZ_LongTable.TAG`) and from `RZ_QuartermasterComponent`.
  New: `RZ_Checkpoints` puts a speaker at the barrier (territory's occupant prefab for that faction) while a
  player is within 100 m. The crew stays an AI group.
- **Slot donors**, so no prefab edits are needed:
  - `RZ_PuzzleChoiceAction`, `RZ_PuzzleUseAction`, `RZ_LeaderAction`, `RZ_StandingStatusAction` and the amends
    `RZ_FactionServiceAction` read `RZ_DialogueClient` first in `GetActionNameScript` and `CanBeShownScript`, in a
    fixed order: generic set 8 slots, quartermasters 6.
  - Every other ReforgerZ action on that entity (shops, services, safehouse actions) hides while the local
    player's session is on it.
  - On the client, `PerformAction` sends `RpcAsk_DialoguePick(serial, choiceId)`. On the server, a donor does
    nothing while that player has a session on that entity.
  - VERIFY: labels refresh per local player when the cache changes; `PerformAction` runs on the performing client
    (else `HasLocalEffectOnlyScript` answers from the cache).
- **UI pipe:**
  - `PAYLOAD_DIALOGUE` (kind 6; `PAYLOAD_KINDS` becomes 7), owner-only, at most 2,000 characters per record.
    Record types: node, overheard, knowledge, close, and the shared personal kinds (errand, look, standing
    notice, lock, epilogue line, detail).
  - `RpcAsk_DialoguePick(serial, choiceId)` on `RZ_PlayerUIComponent`, with its own 0.3 s gap and one pick per
    serial.
  - A dialogue hint that replaces the previous dialogue hint instead of going through `QueueHint` (VERIFY:
    `SCR_HintManagerComponent` can replace or clear a shown hint).
  - `/t`, `/t <n>` and `/t 0` in `RZ_ChatCommands`.
  - Journal rows for "Promises" (with time left) and "What you know".
  - Dialogue never calls `NotifyPlayer`.
- **Layouts and menus:**
  - `RZ_DialoguePanel.layout`, an info display drawn beside `RZ_HudDisplay`. Names: `DialoguePanel`,
    `DialogueAccent`, `DialogueEmblem`, `DialogueSpeaker`, `DialogueRole`, `DialogueStanding`, `DialogueLines`,
    `ChoiceList`, `DialogueOthers`, `DialogueOverheard`, `DialogueHint`.
  - `RZ_DialogueChoice.layout`: `ChoiceRow`, `ChoiceKey`, `ChoiceTag` (tinted: pass, short, would-fail, lie),
    `ChoiceText`, `ChoiceSeen`.
  - Optional `RZ_DialogueMenu.layout` (`ChimeraMenuBase`) with the same names plus `CloseButton`, a
    `chimeraMenus.conf` entry, input actions `RZ_DialogueOpen` and `RZ_DialoguePick1`–`7` in the menu context
    (VERIFY: whether the player can move with it open, and whether number keys clash).
  - Settings UI > Dialogue panel and UI > Dialogue menu (off until built).
  - A mockup `UI/Mockups/dialogue.html` and its preview.
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
- **Scores and memory:**
  - Trust (`RZ_Checks.Trust`): rank, plus ★, plus at most +1 dialogue bonus, minus the liar count, minus crossing
    lines; factionless speakers read the Haven rank (at most 2), the Long Table 0. The built `PersuadeScore` and
    `TryPersuade` are untouched.
  - Leader and wanderer `Attitude` add regard ÷ 2 only while Dialogue is on. Prices keep reading the built bits
    only. Earlier-life memory at half (reputation's change).
  - Make it right: a leader's amends contract clears that leader's built crossed bit once a season, through
    `Forgive()`.
  - `RZ_Reputation` gives a read-only name lookup (the tier until it exists).
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
- **Promises and callbacks:**
  - The built events report to `RZ_DialogueMemory`: food handed in at a house, a delivery at a drop, a visit within
    range and window, a rescue or escort arriving, a message told.
  - Promise clocks count the promiser's online play (2 hours), with a 3-real-day ceiling.
  - Callbacks go into the shared queue (10 per player, a system pool of 100, 3 season days).
  - The rumour tick for claims with no world test: at the half-hourly leader round, and a real day later.
- **Claims in kit** (through `RZ_Disguise`):
  - the kit kind per player and faction, read from the loadout slots;
  - the look's score and tags for a claim choice, with cover details;
  - the server opens post sessions on approach (35 m);
  - point-out as a −1 on the current look; the session ends when kit changes (checked every 5 s and on every
    pick);
  - AI behaviour unchanged.
- **Finale and epilogue**, supplied by those designs:
  - `RZ_Finale.CanCommit(identity, route)`: refused below the 90% gate, after the stand is won, if already locked,
    or when `CanSideWith` says no;
  - `Commit` and `RouteOf(identity)`, per identity for the season, ended at the cure;
  - the lock card's bring and leave lines;
  - `cure_held`, `ending:` and `was:` from `SeasonStory`'s decider lists by identity;
  - every key is quiet (false) until those designs are built.
- **Strangers and radio:**
  - strangers are dealt by the errands dealer at the places in [section 16](#strangers), with the
    one-per-15-minutes ceiling and 6 bodies on the server; a body within 150 m; gone when done or after 20 min out
    of range; never within 2 km of the Long Table's farm;
  - a "Call on the channel" action on house radios and the carried radio (`RZ_SecureRadio`'s channels; secure ones
    only for Radio Operators who've cracked them);
  - barks on the 1-second tick for bodies within 6 m of a player.
- **The danger check,** every second for each open session (at most 64): the player took damage or fired; any
  infected within 6 m of the player or the speaker; an infected within 30 m that has closed in by 2 m or more since
  the last check, read from the spawner's tracked list (at most 60). A 60-second resume record for each ended
  session.
- **Tools:**
  - `tools/dialogue_pack.py`: the lint, plus plan B's packs in small functions (at most 150 `Add()` lines each);
  - `dialogue/shipped.lock` and per-quest `outcomes.txt`;
  - `tools/dialogue_sim.py`, with `--chars` and `--race` for multi-player scenes;
  - `tools/compile_check.sh` runs the lint first;
  - `tools/runtime_test` opens every start node and runs each lie, promise and callback once.
- **Text source:** plan A reads `.rzd` files as addon resources on the server (VERIFY: `FileIO` on addon paths on
  the dedicated server; also measure parse time for the full set). Plan B is the generated packs. Optional
  server-only overrides come from `$profile:ReforgerZ/dialogue/*.rzd`, by conversation id, behind a setting.
- **Persistence:** a new `$profile:ReforgerZ/dialogue.json`, written at the 2-minute save only when something
  changed. It holds per player: regard, failures, liar counts, knowledge; per character: seen nodes, passes,
  claims, promises, faces; and scene records. It decays on load, drops records at death and by age, and is wiped
  and backed up with the season. `state.json` doesn't grow.
- **Debug:** `/talkdebug` and `/talkreset <scene>` (admins), and the log lines in
  [section 19](#19-authoring-format-rzd-pipeline-and-tools).

## Settings

`RZ_GameModeComponent > Dialogue`, a new settings class with no initialisers (defaults only in the attribute
strings), created with `RZ_Util.NewWithDefaults` when missing:
- On (off by default until the label test and a play-test pass);
- Talk range 4 m; Leave range 6 m;
- Idle seconds 120, and 45 while someone waits;
- Max sessions 64; Per speaker 6; Scene sessions 16;
- Overheard radius 8 m (0 off); Room radius 30 m;
- Decide hold 5 s; Objections on;
- Show failed checks on;
- Vouching on; Call-outs on; Point-out on;
- Rumour tick 15%;
- Talk Favor per day 20;
- Promise play hours 2; Promise real-day ceiling 3;
- Barks on;
- Make it right on;
- Text source: resources, packs, or profile overrides.

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
| Disguise passage | Whatever DISGUISE.md's look buys: one passage at a road post, or 10 minutes at a gate |
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
