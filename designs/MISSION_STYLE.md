# The mission style

**Status: designed; waves 1 to 3 built.** Approved by the author on 2026-11-11. Wave 3 built the six
worked examples (simplified, see [Worked examples](#worked-examples)), the last two merges, fallout
errands and the Tier 0 audit's fallout lines. Built and described in
[docs/DESIGN.md](../docs/DESIGN.md#story-blocks): the story kit, who took part and who
decides (the hold, Object, Stand with), who benefits with `RZ_Mission_Choice` rebuilt on it (the
switch-pay fix), the truth deck, world records and their writers, the reason lane and the one weight
clamp, the journal's per-row detail. This file keeps the rest: route assets, talk scenes, tricks and
trade, lies, failure as a branch and fallout errands, the conversion of every mission
(waves 2 to 4), and the parts of wave 1 still listed below.

## What changed in review

Conflict numbers are the plan's, counted from 1 (as in [FINALE.md](FINALE.md) and
[INFECTED.md](INFECTED.md)). Decisions are numbered as in the plan's "Decisions for you". The
alternatives that were not chosen are gone.

- **One side-quest system: errands** (the New Vegas plan, section 5). Missions' 25 odd jobs, the found
  hooks and the fallout jobs move to [ERRANDS.md](ERRANDS.md). Odd jobs become routines; found hooks and
  fallout jobs become cards. Missions keep the **reasons**, which are errands' Why. The 7 missions that
  became odd jobs now become errand routines. Leaders never give errands, so the council-head odd jobs
  now come from named people.
- **The persuade score** (decision 11): your access rank with the listener's faction, with mixed tiers
  one lower (as built), plus the dialogue extras. The built `PersuadeScore` stays, so questline checks
  don't change for anyone. The earlier "Trust reads the Fame side of a mixed tier" is gone.
- **One threat score: Fear** (plan §5). Missions' Press, reputation's quiet threat and dialogue's
  threaten are one check. The cost is +3 Infamy (a scene may name up to 8), once per person by anyone,
  three a life per faction, barred at that speaker for a real day after a failure, and never on a
  stronghold leader.
- **Tricks are the disguise look** (plan §5). The trick score and wariness are gone. A trick is a claim
  judged by the fixed look in [DISGUISE.md](DISGUISE.md), and the job's paper is its cover detail.
  Missions' fixed +10 when found out is dropped: a catch costs +3 Infamy, the kit and one of three
  catches a life. Halcyon's full kit is a coat plus a hood or mask with a respirator (conflict 35).
- **Words** (plan §5): "leans" are **route assets**; missions' world marks are **world records** and
  life marks are **life records**; "marks" now means only physical things (chalk, props); echoes go
  into the one **callback queue**.
- **Lies:** one lie store, found out by events first. The new leader bit `MEM_CAUGHT_LYING` is dropped;
  dialogue's regard *lied* records it. A lie pays at most 60% and no Fame, everywhere.
- **Who hears what:** overheard lines at 8 m; scene results and holds to the room at 30 m. Votes that
  matter to the season are tallies in `RZ_QuestVote`.
- **Crash Site and Protection Racket lines** (conflict 36): the manifest is dated Day 9 or later; the
  fence's reveal is *"Signed correct, every one. Turns out they were signed to us."*; the storekeeper
  says *"We paid. Ask Janda at the pit."*
- **Our Crossroads, Coup Talk and Training Day aren't dropped** (conflict 37). Our Crossroads and Coup
  Talk become questline stages only (the Red Hand's major reuses them). Training Day stays a Bastion
  leader job, so LORE's line about signing a recruit onto the roster stays true. 12 missions are
  dropped now, not 15.
- **Tribes replace Grounds** (conflicts 42 and 43): reasons, contracts and lines use
  [INFECTED.md](INFECTED.md)'s rename table. Lay Them Down is Bring Them Home, Drive Them is Cull,
  Alpha Sighting becomes The Lone One, and the epilogue's grounds beats are tribe beats.
- **Struck Off follows the roster lore** (conflicts 3, 32 and 41): a deserter is struck off as KIA and
  his name stays on the board. Brought home, his KIA's rubbed out and he answers at the roll. Struck Off
  never uses the finale's canon name (Private Fiala).
- **Take the Lab** stays as the Red Hand route's once-a-season theft, startable by any Red Hand-locked
  player who is Trusted with the Red Hand (decision 6). [FINALE.md](FINALE.md) owns it.
- **After the lock** (decision 8): the finale's Acts I and III are open to anyone not locked elsewhere.
  An option whose route asset belongs to another route still refuses a locked player.
- **Reads the finale's keys as it defines them** (conflict 39): `finale.route` 0 is nobody (handed
  over), the lab alone is `finale.finisher` 0, and every lock is read with `RouteOf(identity)`.
- **The epilogue lasts one week** (decision 12). World records' epilogue lines are shown across that
  week, and [EPILOGUE.md](EPILOGUE.md) owns the closing work that lets every story finish in it.
- **Dialogue is off by default** (decision 13) until the per-player label test and a play-test pass.
  Talk scenes run in the action menu until then.
- **Research after the gate** goes into the lab's backlog like samples and never moves research past
  90% ([FINALE.md](FINALE.md), §3).
- **A name collision to settle:** the built Haven leader job "Bring Them Home" is retitled
  **Fetch the Family** when the tribes' Bring Them Home lands, so two jobs never share a name.
- **The build order** is the plan's phases: the framework in phase 3, the pilot in phase 4 (the stop
  point), the other story jobs in phase 5, and the one-choice and work rows in phase 8.

---

## In short

The author wants New Vegas missions: several ways through, a real choice of who gets the result,
givers who lie, failure as a branch, callbacks, and small side jobs that always fit what's happening.
The side jobs are errands ([ERRANDS.md](ERRANDS.md)). This design does the rest with four pieces:

1. **A story kit** that any mission can carry (`m_Story`, null by default). A mission without one runs
   exactly as today, so missions convert one at a time and nothing breaks in between. It's built from
   code that already exists: the questline approach helpers (moved out of `RZ_QuestMission` and keyed
   by identity), the puzzle framework's people and actions, `RZ_Mission_Choice`, leader and wanderer
   memory, and the world facts.
2. **Reasons.** A job is offered *because* something in the world changed. Errands use the same
   reasons as their Why.
3. **A smaller pool.** 34 missions leave the random pool or merge, so the jobs that stay get real work.
4. **Route assets** that tie mission outcomes to the finale's routes, with the 90% lock respected.

**What it gives players:**
- **Checks can fail, and the mixed picture works.** Persuade reads your access rank. Fear reads your
  Infamy this life, or who backs you, so a Wildcard can do both. Each check has three results: Pass,
  Short (a halfway deal) or Refused. Tags show what each option needs, and scores are fixed.
- **Tricks are very hard to abuse.** A trick is a claim judged by the disguise look, needs the job's
  paper as its cover detail where the mission names one, and a catch costs the kit.
- **Who decides is fair in multiplayer.** Only eligible participants decide. A scene decision waits 5
  seconds and anyone eligible can object once. Switched pay and blame go only to the decider and those
  who chose to stand with them.
- **Truths persist.** A truth is kept per subject for 3 season days. A lie that works for now is found
  out later by things that happen in the world, not by hidden dice.
- **In-game endings.** World records carry an epilogue line and a carrier (a board note, a voice, the
  radio, a prop, a headline or chalk).

**Effort.** About 9,900 lines added and 3,000 removed, in four waves. The odd-job code (about 1,900
added and 1,100 removed) is now counted in [ERRANDS.md](ERRANDS.md). The game is whole after the pilot
wave, so the later waves are optional growth.

## What the author decided, and what this design does with it

| Decision | What this design does |
|---|---|
| The finale is a fight over who controls the cure: one route per faction plus an Independent route. Locked after 90%, never before | A few outcomes add a **route asset** (an ally, proof or standing). An asset tilts and never closes a route. A player locked to a route can't decide an option whose asset belongs to another route |
| Endings shown in the world, not as slides | World records carry an **epilogue line and a carrier**: a board note, a voice line, a radio line, a prop, a headline or chalk. [EPILOGUE.md](EPILOGUE.md) shows them during its week |
| Dialogue trees | **Talk scenes**: 2–3 nodes per person in the action menu now. The dialogue screen ([DIALOGUE.md](DIALOGUE.md)) reads the same nodes once it's built and turned on |
| No companions | Escorts end with the mission. An errand can only "show someone the way" to a house within 600 m |
| Reputation as a mixed picture | Persuade reads your **access rank**. Fear reads your **Infamy this life, or who backs you**. A Wildcard can do both |
| Disguises and faction armour, very hard to abuse | A trick is a **claim** judged by the disguise look, shown before you commit. A catch costs +3 Infamy, the kit and one of three catches a life per faction |
| Grey choices and dark humour | Every option costs something, and the decent one usually pays least, often nothing. Each story job has one darkly funny line, spoken by the living |
| Several ways through, plus many small side quests | The story kit here; small side quests are errands |

## Scope: fewer, better

| Group | How many | What it gets | Code each |
|---|---|---|---|
| **Story jobs** (Tier A) | 28 | The full kit: 3+ ways (one needs no gun, one needs no standing), 2+ parties the result can go to, 2–4 truths, a fallout, world records, a callback, reasons | about 150 lines, plus 30 of text |
| **Already there** (Tier 0) | 27 | An audit: check tags, the decider rules, one world record written and one read, reasons, a fallout line | about 30 |
| **One real choice** (Tier B) | 16 | One hand-written choice where the job already has one waiting, a world record, reasons | about 70 |
| **Work** (Tier C) | 68 | A row in the reasons table: what starts it, the town tally it writes, a fallout line | one data row |
| **Conditions** | 9 | Reasons only | — |
| **Merged away** | 9 | — | removed |
| **Out of the pool** | 24 | 12 dropped, 7 become errand routines, 5 stay only as questline stages | removed, or kept only for the stage |
| **To the finale** | 1 | Take the Lab | — |

The count adds up: 27 + 28 + 16 + 68 + 9 + 9 + 24 + 1 = 182. After the rework the random pool holds
148 missions (leader jobs included), plus errands.

Every converted mission answers five questions in its MISSIONS.md block:
1. **What starts it?** The reasons it answers.
2. **How can I do it?** Its ways.
3. **Who gets it?** At least one option that isn't the giver.
4. **What aren't they telling me?** Its truths.
5. **What happens if it goes wrong, and who remembers?** Its fallout, and the records it writes and reads.

## Clocks and identity

| Clock | What runs on it |
|---|---|
| **Your play**: the life's played time (`m_fLifeSeconds`) | Callbacks, a giver's sulk |
| **Season days** (built: [DESIGN.md](../docs/DESIGN.md#clocks)) | How long truths are kept, pinned bad records, the contract budget, mission tricks, loyalty changes from contracts, reasons |
| **The mission's own clock**, as built | Time limits, holds, windows |
| **Real time**, for anti-abuse limits only | A failed threat's bar at that speaker (a real day) |

Nothing else counts real days. Weekend players still get callbacks, and fast seasons keep their shape.

**Identity, not session id.** Everything the kit keeps per player is keyed by identity, plus the life
where it's per life. That covers:
- tries at checks, and who won't talk to you;
- the speaker, objections and participation;
- the threat count, tricks and fallout priority;
- callbacks.

The shared helper (`RZ_Ways`, built: [DESIGN.md](../docs/DESIGN.md#checks-and-ways)) keys tries, threats and who
did what by identity, so a rejoin keeps everything.

**What survives a death:**

| Record | Belongs to | Ends |
|---|---|---|
| Reputation and Favor (as built) | the character | with the life |
| **Life records**: what this character did (pressed someone, covered a payment, hid a bite), in `RZ_Deeds`' life scope | the character | with the life. Dying to clear one costs the whole life's standing, so it isn't worth it |
| **Callbacks** waiting for this character | the character | with the life. A waiting one becomes an unsigned note on the board at its subject (*"For the man who brought the Dvoraks in, if anyone sees him"*) |
| Leader and wanderer memory (as built, life-stamped: earlier lives count at half and are voiced once as the name) and dialogue's regard, including *lied* | the player (identity) | as built |
| World records, truths, reasons | the season | with the wipe, or sooner as described below |

Rule: named people with a memory (leaders, wanderers) remember the player. Everyone else (houses,
people in a scene) remembers the character.

## Checks: persuade and Fear

**Built:** the one helper (`RZ_Checks`) with Trust (the persuade score, decision 11), Fear, a threat's cost and limits,
Pass / Short / Refused and the tags: [DESIGN.md](../docs/DESIGN.md#checks-and-ways). What this design adds on top, for
missions:

- People with no faction (the Distress Call pair, a trapped family, a stranger on a roof) read your persuade score
  with the Haven, at most 2 (built); at Kill on Sight they shoot you instead of listening.
- A threat a mission writes into its scene (Protection Racket's) may name more Infamy, up to 8, and stays even in a
  safe zone. Built questline stages that threaten keep recording FORCE.
- **One try per person per job per identity** (built, in `RZ_Ways`). A failed check changes things only for the
  player who failed; nobody can fail on purpose to raise the price for others.
- A check's extra (a better price, a bonus) goes only to the player who chose it. A squad's Champion can do the
  talking, but the pay doesn't multiply.
- **Short** is a halfway deal at a price the option names (Favor, food, a promise written as a life record, or a
  smaller result); a refused threat makes them call their people out, or bar the door.
- **Tags (New Vegas's "[Speech 50]")**, for example: "[Trusted · Haven] Say the Haven will vouch for them", "[Nurse]
  Look at his arm", "[Feared] Lean on her", "[4 food | 20 Favor] Pay it yourself", "[The receipt] Show him",
  "[Bastion kit + flight order] That's ours, lads." Action labels are the same for every viewer.

**Mixed reputation.** A Wildcard has high Fame and high Infamy, so persuade 1 and Fear 2: people trust
you a little and fear you more, New Vegas's "Merciful Thug". The mixed-reputation design supplies the
title, and a scene's opening line may use it. Checks never read the title.

## Ways

`RZ_EQuestApproach` has NONE plus eleven values; PRESS, TRADE, TRICK and LEAVE were appended once (built:
[DESIGN.md](../docs/DESIGN.md#checks-and-ways)). Missions, questlines, the majors, the finale's stages and errands use
them.

| Way | What it is | Check | Cost or risk |
|---|---|---|---|
| **Talk** | Ask, listen, get the story. An option can need something you're carrying ("[The receipt]"). That's proof, not a trick | None, or a line only some players get (a background read, a record) | Time |
| **Persuade** | Talk them round | Persuade | One try. Short or Refused is the branch |
| **Press** | Threaten | Fear | +3 Infamy with the listener's faction (a scene may name up to 8), plus a life record. The limits above |
| **Trade** | Give items or a service | You have it (the price table) | The items, worth the same as the matching bribe |
| **Bribe** | Pay Favor | You have it | Favor |
| **Trick** | A claim: you're one of theirs, or you're carrying their paper | The look ([DISGUISE.md](DISGUISE.md)) | Caught: the catch's costs, and the scene turns |
| **Sneak** | Do it unseen, within a window (before the first wave, or at night) | `RZ_QuestStealth`, as built, which reads built stealth | Seen: the loud version starts at once, and whatever was in the open is exposed. It pays the same as the plain way, with no extras |
| **Force** | Kill what the mission spawned | — | Infamy, noise, heat |
| **Betray** | Take the job, then hand the result to someone else | Through who benefits | Infamy with the giver. Their leader remembers |
| **Side** | Bring another faction in | Through who benefits | As Betray |
| **Leave** | Walk away | — | Only for you: you stop taking part, with no pay and no blame |

**Background reads.** Each background reads one kind of thing, the same in every mission, so players
learn it once. Police and Radio Operators no longer read everything.

| Background | Reads | Never reads |
|---|---|---|
| Nurse, Doctor | Bodies and bites: who's bitten, how long ago, what killed them | papers |
| Mechanic | Machines: engines, wrecks, black boxes, rigged locks | people |
| Hunter | The ground: tracks, how many, how old, which way | papers |
| Police | Whether someone in a talk scene is lying (that they are, not what about) | numbers |
| Ex-Con, Raider | Crews: marks, prices, who's really in charge | Bastion |
| Lab Tech | Numbers: ledgers, manifests, readings | people |
| Radio Op | Channels: live or a loop; at most one secure-net line per job | bodies |
| Elder | Families: names, who's kin to whom, a house's own story | crews |
| Soldier | Bastion: rosters, unit codes, orders. Bastion people will talk to a soldier | crews |
| Drifter | Road news: one extra hint from wanderers | — |
| Unemployed | Nothing new; keeps +50% Fame | — |

A read sends a line to that player (owner-only) and adds a note to their journal. A background read is
never the only way to find a truth. A mission never writes a read for a background it locks out.

## Talk scenes

- People in a scene come from the one scene-people spawner (`RZ_SceneProps`, shared with the epilogue's
  scenes, errand scenes and strangers): the puzzle framework's spawning and label code, moved into a
  helper the mission owns. `RZ_PuzzleMission` keeps its API. People exist only within 400 m of a player
  and are deleted at cleanup. `RZ_PuzzleActionHelper` sends choices and uses to
  `RZ_Mission.HandleChoice` and `HandleUse`, the same way reads already reach `HandleTalk`.
- **Nodes.** Each node has a line and up to four options (the choice slots), each with a tag and a
  result. "Talk to" (a use) repeats the current line. The last option is always "Leave it", which goes
  back to the first node.
- People in a story job have **two or three nodes**. That's honest for a button panel. The dialogue
  screen reads the same node table once it's built and turned on (decision 13: off by default until the
  per-player label test and a play-test pass). The action menu stays as the fallback.
- **Who hears it.** Every line rides the owner-only personal payload, sent to each player it's for,
  never a broadcast:
  - a node's line goes as a person card to the speaker and anyone within **8 m** (overheard);
  - scene results, reveals and holds go to everyone in the **room (30 m)**;
  - late joiners read it in the journal.
- **The speaker** holds the person: the first eligible participant to pick an option. They keep it for
  60 s after each pick, and for at most 3 minutes in all. Others in the room may pick options that only
  reveal something. Options that change the scene or end the job belong to the speaker.
- Someone you threatened, or caught lying, won't talk to you for the rest of the job.
- **Fallback if the replicated-labels play-test fails:** the node's line lists numbered options ("1 Ask
  about the tins · 2 [Trusted · Haven] Say the Haven will vouch..."), and the choices read "Option 1"
  to "Option 4".

## Tricks: the look

A trick is a lie that rests on something real. It's a **claim**, and the disguise design's look judges
it ([DISGUISE.md](DISGUISE.md), "Looks"): a fixed score, shown with its tags before you commit. The
mission sets the difficulty (1–3) through `RZ_QuestApproach.PassAs(gm, playerId, faction, difficulty,
coverKey)` and registers its people as watchers (`RZ_Disguise.AddWatcher`).

- **The job's paper is the cover detail** (+1): a flight order, a pass, a tag, a receipt. A mission
  names which paper counts.
- **Kit counts as the look counts it:** their colours +1, full kit +2. Halcyon's full kit is the coat
  plus a hood or mask with a respirator: the beaks (conflict 35; the respirator and gas-mask words go
  into the attribute's default keyword string, never an initialiser).
- **A paper without kit** scores only what the look gives it (the cover detail and any background), so
  it passes only the mission's difficulty-1 people.
- **A uniform trick is offered only where the mission names one.** Kit alone does nothing in a talk
  scene: nothing happens until you claim.
- **One mission trick per life per faction per season day.**
- **Always seen through**, on top of the look's own instant fails (any leader, inside a stronghold's or
  the lab's wire, the Red Hand at its dusk meal):
  - a wanderer who remembers you;
  - anyone holding a life record about you;
  - hunter squads (they're a fight, and fights are never fooled).
- While you're passing as a faction, your persuade score with it counts as 0, and you can't threaten its
  people.
- **Your own colours aren't a disguise.** At Known or better with a faction (mixed tiers by access),
  its kit is just your gear, and you're treated by your real standing ([DISGUISE.md](DISGUISE.md),
  "Kit").

**Caught** (one cost, owned by [DISGUISE.md](DISGUISE.md)):
- +3 Infamy with that faction, never taking you into Watched on its own;
- the kit is taken;
- one of your 3 catches with that faction this life;
- blown with them for 60 minutes of play.

The scene also turns, as the mission says: the carriers run with the box, or the person calls their
people. Being caught is never just a free retry of the fight you'd have had anyway.

## Trade and prices

One price table everywhere a way takes items (`RZ_Checks`):

| Item | Value |
|---|---|
| a tin of food, a bottle of water | 1 |
| jewellery | 1.5 |
| a medical item, a jerrycan | 2 |
| a car part | 3 |
| a background service on the spot (a Nurse dresses a wound, a Mechanic fixes a pump) | 2, plus the time |

- **One value is 5 Favor in a bribe.** A Trade option costs the same as the Bribe beside it: "[4 food |
  20 Favor]".
- Trades and bribes are costs. No option gives items or Favor back.
- Food delivered to a safehouse keeps its built rate (2 Haven Favor an item, plus some Fame). That's
  selling to the network, not paying a favour.
- **Mission goods** (anything a mission spawned, such as a crate of tins or a case) are tagged. They
  never count for Trade, and quartermasters, food delivery, the fence and errands refuse them for pay:
  *"These are Levie's tins. Take them back to Levie."*
- There's **no "keep it yourself"** option in contracts. World events keep their crates, as built.

## Who benefits

A story job lists 2–4 parties its result can go to:

| Field | What it says |
|---|---|
| Who | A faction, a house, a named person, or nobody |
| Where | A drop, a person in the scene, an action, or the radio station |
| Who pays | Fame and Favor, at most the base |
| Who's angry | Infamy and leader memory |
| What the world gets | The base result, sent here or there, plus small extras (see Limits) |
| Route asset | At most one: an ally, proof or standing toward a finale route |

**Rules:** the core is built (`RZ_Beneficiaries`, [docs/DESIGN.md](../docs/DESIGN.md#story-blocks)):
the side you chose pays anyone with you whom it doesn't hunt, the giver adds Infamy and its leader doesn't
count it as help, a switch pays at most the base and leaves season Fame alone, "nobody" pays nobody, and
`RZ_Mission_Choice` is built on it. Still to build:
- **The base result stays.** Choosing *where* it goes (which house gets the family) is part of the base
  result, not an extra.
- **Nobody** as an option in the converted jobs (sink the guns, burn the box, let him go), sometimes with an
  asset for the Independent route.
- **Decency doesn't pay the Haven by default.** A house's thanks is a record and a callback, not Haven
  Fame. Decency pays other factions where that fits the story: Bastion for civilians sheltered, Halcyon
  for honest data, the Red Hand for paying a debt. Usually it pays least, and often it pays nothing.
- **A lie without proof pays at most 60%, and no Fame**: Halcyon pays for a dog tag, not for the man.
  With proof (the receipt, his tag) it can pay in full, because it's no longer quite a lie. This is one
  rule with errands.
- **Research goes only to the lab.** Notes given to a Haven medic, or read on the open channel, don't
  move the cure. They give a small world effect (−3 infection in the medic's town) and maybe an asset.
  From the 90% gate, research a mission pays goes into the lab's backlog like samples and never moves
  research past 90 ([FINALE.md](FINALE.md), §3).

### Route assets and the 90% lock

The finale's side is built ([docs/FINALE.md](../docs/FINALE.md#before-the-gate-pitches-and-route-assets)):
`RZ_FinaleAssets.Add(route, kind, identity, key)`, the limits, the lock card, playing an asset once, and the
first story records that give one (`RZ_StoryKit.Record`). The missions' side still to build:

| Asset | For example |
|---|---|
| **Ally** | A scientist who walked out (Independent); a deserter the Haven hid (the Haven) |
| **Proof** | The manifest read on the open channel (Independent); the fence holding it (the Red Hand) |
| **Standing** | The black box burnt in the stove (Bastion) |

- Most jobs have no assets. A story job's result adds at most one. A player holds at most 3 for any one
  route in a season (`Asset(route, kind, identity, key)`).
- An asset does nothing until its owner locks to that route. Assets for other routes go dormant at the
  lock and lapse at the cure.
- The Independent route's assets come from the open channel, from "nobody", from letting people go, and
  from giving things to people rather than to factions.
- **Before 90%, nothing is ever locked:**
  - an asset tilts a route and never closes one;
  - a record makes a check at most one step harder;
  - a sulk lasts at most 2 hours of your play;
  - Infamy works off as usual.
- **After you lock a route**, an option whose asset belongs to another route refuses you: *"You've
  picked your side. This isn't it."* You can still do the work, and anyone not locked can still decide
  it. The label doesn't change (labels are shared); the refusal comes when you pick it.
- Everything without an asset is offered to locked players as usual. The finale's Acts I and III are open
  to anyone not locked elsewhere (decision 8); its sworn acts and route work belong to the route's
  locked players ([FINALE.md](FINALE.md), §5).
- The lock is read with `RZ_Finale.RouteOf(identity)`: no route, or one of five.

## Who decides

Built ([docs/DESIGN.md](../docs/DESIGN.md#story-blocks)): who took part, who may decide, the 5 s hold
announced to the room that anyone eligible may object to once, Stand with, with the decider, and half the
Favor to the others who did the work. Still to build, with the scene decisions of wave 2:
- the objector becomes the speaker for 60 s; *"It's not your call. You weren't hired for this."* when
  someone else picks an option that would end the job;
- **Leave** per player (you stop taking part, with no pay and no blame);
- **offline participants** (a step done, or 5 minutes in the area) paid on their saved profile at their
  next join, through the questlines' path;
- anything that matters to the season as a tally in `RZ_QuestVote`;
- the three distances for lines: overheard at 8 m (results and holds at 30 m are built);
- **amends need clean hands:** Press, Trick, a switch or hiding something voids that job's Infamy
  work-off for that player.

## Hidden truths

A story job has a deck of 2–4 truths. The brief is only the giver's word.

**The deck:**
- One truth is "it's as they said", never below 30%, so players can't assume every brief is a lie.
- Each truth has gates: a season flag, a free wanderer, a Rogue leader, a prefab, a place. A truth whose
  gate fails is never drawn.
- The giver's traits move the weights by at most ±50%:
  - Greedy and Paranoid givers hide more;
  - Merciful and Idealist givers hide less;
  - a Talker's secrets leak into road news;
  - a Pragmatist says it straight;
  - Ruthless and Veteran givers leave things out "by procedure".

  Radio contracts use the traits of the leader who voices them.
- The truth is server state, so everyone sees the same one.
- **Canon wins:** a truth about a fixed fact (`bastion.orders`, `origin.*`) reads canon and never rolls
  it again.

**It stays put:** the truth deck is built ([docs/DESIGN.md](../docs/DESIGN.md#world-memory)): seeded by the
story key, the subject and the season, kept 3 season days or until used up, "as they said" never under
30%. Still to build: the gates and the giver's traits moving the weights, canon truths, and the found
list kept across a restart.

**Fair play:**
- Every truth can be found on site before the decision it matters to, in at least two ways. At least
  one of them needs no background and no standing: reading something, a Talk option anyone has, or
  sneaking.
- A truth never turns a choice into a gotcha after you've been paid. Nobody is blamed for something they
  didn't know.

**Finding out.** A truth can come from:
- a clue;
- a background read;
- a talk check;
- a third party: a wanderer, or "Ask about..." at another faction's stronghold;
- a radio line.

The finder and the room get the line. The journal's "What you've found" adds it for everyone the job was
offered to.

## Lies, and how they're found out

There's one lie store (per player per season), shared with errands and dialogue. Each record holds its
exposing condition. **Lies are found out by events first**, never by hidden dice:
- **a visit:** a player Known with the faction you lied to comes within 30 m of the truth's subject, or
  a job or errand for that faction runs there;
- **road news:** a wanderer whose route passes the subject next talks at that faction's seat;
- **the owning system's tick:** the next upkeep, the next collection, the next shift.

Only a claim with nothing in the world to test it uses the **rumour tick** (dialogue's two 15% rolls,
renamed), and a Talker giver's leak runs on that tick. Each kind of discovery writes a log line, so the
author can tune them and the runtime test can trigger them.

**Caught lying:**
- The giver's regard records *lied* (dialogue's regard; the earlier `MEM_CAUGHT_LYING` leader bit is
  dropped).
- Next time you talk, they bring it up once, in their own voice.
- A Paranoid giver won't give you their in-person jobs for 2 hours of your play.
- Radio contracts are never blocked. There's no hush money.

**How each faction lies** (goes into [WRITING.md](../docs/WRITING.md) when it's built):

| Faction | Lies by | For example |
|---|---|---|
| Bastion | leaving things out | *"The patrol saw nothing."* (It saw a crew and turned back.) |
| The Red Hand | a joke | *"Late? Nobody's late. Some people are slow learners."* |
| Halcyon | a true number that isn't the whole number | *"Two of three volunteers recovered."* |
| The Haven | kindness | *"We didn't want to worry you."* |
| The Long Table | being too helpful | *"Take the north road, love. Nobody walks it."* |

## Failure is a branch

| Failure | What happens |
|---|---|
| Time up | The default happens without you (the giver's way): a line and a capped change |
| Seen, or caught | The loud version starts at once, with whatever was in the open exposed. The job doesn't fail; it gets worse |
| Someone died | The world names them, and a record goes on their house |
| The other side got it | Their result happens, and your offer ends as lost |

- **Fallout errands.** A story job's failure can open one errand at its subject: bring their names home,
  buy the box back, three tins for the house. It's dealt to the job's participants first for 20
  minutes, then to anyone. It pays like an errand, and at most 30% of the failed job's base, so failing
  on purpose and then doing the fallout never beats succeeding. [ERRANDS.md](ERRANDS.md) runs them.
- At most one fallout per job. A fallout doesn't make another, there's none after `Withdraw`, and losing
  a mirror gives a line only.
- **Grades.** A job done badly still says what happened (*"Two of the four made it."*) and pays for what
  got done, where the base mission already scales its pay (as Collapse Shelter does).

## Records, callbacks and the epilogue

**Records.** The stores are built ([DESIGN.md](../docs/DESIGN.md#deeds) for life records,
[DESIGN.md](../docs/DESIGN.md#world-memory) for world records: one per key and subject, 160, evicted by importance,
never while a reader holds one). What missions add:

- **Quotas:** a story job writes at most 2 world records, and a one-choice job 1. An errand writes 1 only when it's
  about a named person. Work jobs (Tier C) write only the town tally (one per town: who cleared it last, and how
  often, in town memory).
- **Each story job lists the records it reads**, and pins them while it runs.
- **Records are for people and places; season flags are for outcomes.** Questlines keep their flags.
- **Names** follow the epilogue's and tone's policy: frozen at the deed, passed through the note filter,
  `/anonymous` and the admin strike honoured, never a joke's butt, never on air except a player's own broadcasts and
  honour lines. Face to face, a speaker may use a player's name (filtered, unless they're anonymous).

**What reads records.** A record can:
- make a check one step easier or harder, never impossible;
- force a truth or rule one out;
- post a reason;
- fill a voice's first line or a board note;
- choose an epilogue line.

**Callbacks** go through the one callback queue (built: [DESIGN.md](../docs/DESIGN.md#callbacks)). What missions
add: a callback is a line, a person at the door (a stand-in within 300 m), a note on that house's board signed with a
name, or an errand (which pays like any errand), beyond the built "saved my life" wanderer gift nothing else.

**The epilogue (in-game endings).** Every world record with an epilogue line names its **carrier**:

| Carrier | How it's shown |
|---|---|
| Board | A note on the nearest safehouse board |
| Voice | A leader's or wanderer's first line after the cure |
| Radio | A line on a faction's open channel in the epilogue |
| Prop | An optional prop at the subject (tags on a nail by a post's radio; a radio with the batteries out on a school desk), through the one physical-marks spawner (`RZ_Marks`). It's spawned only within 300 m of a player and removed afterwards, at most 3 epilogue props in any 300 m. Without the prefab, it falls back to Board |
| Headline | The session news |
| Chalk | A decal on a wall (VERIFY: decals at runtime). Falls back to Board |

[EPILOGUE.md](EPILOGUE.md) picks which records show, by importance and carrier, across its one-week
epilogue. Nothing it shows is saved as an object.

## Reasons: why a job is offered now

Relevance comes from what starts a job, not from a line in front of the brief. Reasons live in
`RZ_WorldMemory` (the store is built: [DESIGN.md](../docs/DESIGN.md#world-memory)), and errands read them as their Why.

- **A reason** is a world change with a subject and a place: `house.hungry` at Levie, `post.pressure`
  at the crossroads post, `tribe.hungry` for the Quarry lot, `wanderer.bitten`. The systems that own the
  facts post them: safehouse upkeep, supply lines, territory, leaders, wanderers, cure research, the
  Director, the tribes and the majors.
- A reason lasts while its fact holds, and at most one season day.
- Every mission type and errand lists the reasons it answers (`Answers("house.hungry")`), with a place
  rule (near the subject).
- **Contracts:** the reason lane is built ([docs/DESIGN.md](../docs/DESIGN.md#story-blocks)), with the
  start of the table below. Still to post: `house.water`, `house.dark`, `house.bitten`, `house.feud`,
  `house.borrowed`, `post.pressure`, `horde.passed`, `leader.rogue`, `leader.bitten`, `wanderer.owes`,
  `wanderer.lost`, `crew.skimmed`, `racket.ran`, `lab.power`, `lab.short`, the tribes' and the majors'.
- **Errands** answer reasons through the people who give them, within their own caps.
- **Random picks** stay, for variety. A random contract gets a why-now line only when a fact fills one,
  never a filler line.
- **One clamp on weights:** built ([docs/DESIGN.md](../docs/DESIGN.md#story-blocks)).
- **Order per player.** The journal and the quartermaster's list put certain jobs first: those near you,
  those answering a reason where you have a record, and those from a giver you've helped. This changes
  only the order, not the odds.

**Reasons and what answers them (the start of the table):**

| Reason | Posted by | Answered by, for example |
|---|---|---|
| `house.hungry`, `house.water`, `house.dark`, `house.bitten` | safehouse upkeep | Harvest Guard, Protection Racket, Medicine Run; errands The Pot, Water for {house}, Lights, Sit With Him |
| `house.feud`, `house.borrowed` | safehouses | The Ledger; errand A Word to {house} |
| `post.pressure`, `post.truck_lost`, `post.shelled` | territory, supply lines, the Director | Checkpoint Duty, Convoy Escort; errands The Wire, The Count |
| `tribe.hungry`, `tribe.moving`, `alpha.down`, `tribe.splinter` | the tribes ([INFECTED.md](INFECTED.md)) | Lead Them Away, Blow the Bridge, Not Tonight, Bring Them Home, Cull, Clear the Trail; errands Up on the Roof, Not Past the Chapel |
| `horde.passed`, `town.infested` | the Director | Survivors Trapped, Retake the Town; errand Up on the Roof |
| `leader.died`, `leader.rogue`, `leader.bitten` | leaders | Rein In (built), Struck Off (*"the new commander wants his numbers right"*), The Bitten Friend |
| `wanderer.bitten`, `wanderer.owes`, `wanderer.lost` | wanderers | Trail of Clues; errand Before Dark |
| `crew.skimmed`, `racket.ran` | Red Hand content | errand Short Change |
| `lab.power`, `lab.short`, `cure.<percent>` | cure research | Cold Chain, Power the Lab; errand The Freezers |
| `major.<act>` | the majors ([MAJORS.md](MAJORS.md)) | the act's stage missions, and the errands' margin cards |
| `route.locked` | the finale | the route's own work ([FINALE.md](FINALE.md)) |

## Limits

**Pay:**
- More ways never mean more pay. An option can cost something (Favor, items, Infamy), or add a small
  extra of at most 5 Fame or 15 Favor, paid only to the player who chose it.
- A switch pays at most the base, and its Fame doesn't count toward season Fame.
- "Nobody" pays nobody.
- A lie without proof pays at most 60%, and no Fame.
- A fallout errand pays at most 30% of the failed job's base.
- No loot beyond what the base mission already gave (its reward crate, as built).

**World effects.** These caps apply to what the kit adds on top of the base result, per job:

| What | At most |
|---|---|
| Morale | ±3 per faction |
| Strength | ±1 |
| One house | ±2 food and ±1 person, never below 2 people |
| One town | ±10 infection |
| One leader's loyalty | ±5, and never a stronghold leader |
| Research | Nothing extra: research goes only to the lab, ×0.75 and through its capacity |

- The base result is unchanged. Retake the Town still takes 35 infection off, and Survivors Trapped
  still adds 3 or 4 people.
- Every effect goes through the shared capped helpers, in the **contracts and errands lane** of the
  majors' season budget (`RZ_MajorBudget`). Per faction per season day, contracts and errands together
  can move morale by ±10 and strength by ±4. Anything clipped becomes words only.

**Leaders.**
- Contracts never replace a leader.
- The kit's loyalty changes count in the majors' seat ledger as contract swings. Contracts can lower one
  leader's loyalty by at most 10 per season day.
- Contracts **never take a leader below 30**. So a contract can't make anyone Rogue, and can't start a
  Rein In that removes them.
- Stronghold leaders are never touched.

**Bodies.**
- Story groups the kit adds (crews, recovery teams, a keeper's people, carriers): at most 4 server-wide
  and 2 per job, inside the one AI-group budget shared with the tribes. These come on top of the base
  missions' own groups.
- Every option that needs a group names a fallback for when the cap is full or the prefab is missing:
  words and a fixed cost, as Protection Racket already does without the survivor group prefab.
- People in scenes plus askers: at most 16 (the one body budget). Escorts keep their limit of 3.
  Nothing physical stays in the world.

**Any map.** Truths, options and places are found relative to settlements, landmarks, leader seats and
canon places. An option or truth whose prefab or place is missing is never offered or drawn.

## Teaching it

New guidance tips (the contextual tips system), each shown once:
- The first tag you see: *"Tags say what an option needs. [Trusted · Haven] means your standing with the
  Haven."*
- The first truth you find: *"The brief is only what they told you."*
- The first background read: *"Your background notices things others don't."*
- The first hold: *"Someone is about to decide. You can object once."*

The first errand's tip is [ERRANDS.md](ERRANDS.md)'s.

## Fit with the tribes, the majors, the finale and the epilogue

**The tribes** ([INFECTED.md](INFECTED.md); approved, not built):
- The tribes post reasons (`tribe.hungry`, `tribe.moving`, `alpha.down`, `tribe.splinter`). The season
  day is built once, in the plan's phase 1, whichever comes first.
- Their contracts are built on the kit from the start:
  - **Bring Them Home** is a story job: find out who the named young was, put it down, then take the ID
    card or keepsake to the family, or let the fence buy it;
  - **Cull** (*"Not our sector"*) is a one-choice job: where the displaced tribe goes, shared with the
    "where the horde goes" choice block;
  - **Bastion's lost posts** (a military tribe's young, still on the roster) is a one-choice job: their
    tags to the nail, or left walking;
  - **Contact Report** (amends), **Not Tonight**, **Escort at Dawn**, **Clear the Trail**, **Burn the
    Lair**, **Break the Habit**, **Bait the Crew**, **Plant a Tag**, **Track Them Home**, **Capture the
    Index**, **The Lone One** and **Stock the Pit** are work, each with a reasons row.
- **Retake the Town** goes to towns (the crawlers). **Clear the Trail** and **Cull** go to tribes. A town
  claim stops two jobs running in one place at once.
- **Burn the Nest** stays the crawlers' nest in a town, with its new brief. Burning a tribe's lair is
  Burn the Lair. Burn the Nest's choice (burn it, or leave it for Halcyon) stays.
- **Alpha Sighting** becomes **The Lone One**.
- Player text never says "alpha", "tribe" or "crawler": each faction has its own word (`{tribe:N}`,
  `{alpha:N}`).

**The majors** ([MAJORS.md](MAJORS.md)):
- Their stage missions are questline stages on the kit: several ways through, failure as a branch.
  Await Relief's Act IV gets the disguise roster trick, within the questline cap.
- Contract weights from the majors go into the one clamp.
- The seat ledger takes contract swings, and the contracts and errands lane sits in the majors' budget.
- Votes stay tallies in `RZ_QuestVote`.
- Flags that drive voices: Protection Racket skips houses that pay by arrangement (`door.tins`,
  `haven.paying`); Crash Site reads `majors.pages_open`; `orders.stance` drives Crash Site's pay.
- Our Crossroads and Coup Talk are stage-only classes the Red Hand's acts 1 and 4 reuse. Training Day
  stays a Bastion leader job.
- **The Bastion record.** The nail list, the roster and "struck off" are one record. Jobs add names to
  it; only the majors and the built nightly call read names on the air.

**The finale** ([FINALE.md](FINALE.md)):
- Take the Lab leaves the contract tables. It's the Red Hand route's once-a-season theft, startable by
  any Red Hand-locked player who is Trusted with the Red Hand, and every Red Hand-locked player counts
  for the bay hold.
- Route assets and the lock work as described above. The interface is `RZ_Finale.RouteOf(identity)` (no
  route, or one of five) and `Asset(route, kind, identity, key)`.
- The finale's stages and Last Mile contracts use this kit.

**Dialogue, reputation, disguise and the epilogue** need only four things from this design's partners:
- the node table ([DIALOGUE.md](DIALOGUE.md));
- the mixed-reputation title;
- the look and the own-colours rule ([DISGUISE.md](DISGUISE.md));
- the carrier list above ([EPILOGUE.md](EPILOGUE.md)).

## The conversion

**Tier 0, already there (27):**
- **The Haven:** The Bitten Friend, The List, Walk Him Out, The Ledger, A Quiet Arrangement.
- **Bastion:** Requisition, Secure the Shipment, No Reply.
- **The Red Hand's hard edges:** Learning the Drones, The Pit's Debtor, Tied Down.
- **Halcyon:** Serum Trial.
- **World events:** Rival Survivors.
- **Puzzles:** The Rat, The Silent Village, The Witness.
- **Leader jobs:** Shut the Door, The Real Orders, Local Law, Off-Book Requisition, Desertion Route,
  Honour Among Thieves, Debt Collection, Patients Not Samples, The Notebook, Private Collection; and
  Rein In.

**The audit is built** for all 27 ([MISSIONS.md](../docs/MISSIONS.md#who-gets-offered-what)): a fallout
line, the `t0.<key>` record written at their town and read by the next one there, the decider rule, and
Walk Him Out's `walk.out` and `house.bitten`. What it still leaves: check tags on the choices that have
none, and the tribes reading `walk.out` (a man walked on feeding the nearest tribe as a named young).

The audit, as designed, for example on **Walk Him Out**:
- tags on its two choices (the ★ Nurse bite read is already there);
- the decider rules;
- a world record `walked.<name>` (ended, or walked on), which H3 and the tribes read (a man walked on
  feeds the nearest tribe within 2 km as a named young);
- the reason `house.bitten`;
- a fallout line: nobody came, he turns, and the house says his name at supper.

The same pass gives the decider rules to the work-tier leader jobs that have a choice.

**Tier A, story jobs (28; the five worked examples marked \*):**
- **The Haven (6):** The Cache (Supply Cache + Raid the Cache), Medicine Run (+ Pharmacy Lure), Missing
  Scout, Refugee Column, Trader Caravan, The Informant.
- **Bastion (6):** Crash Site\*, Struck Off (+ Hunt the Deserter)\*, Missing Patrol, Rescue the Pilot,
  Raid the Red Hand, The Armory (Weapons Cache + Loot the Armory).
- **The Red Hand (5):** Protection Racket\*, Dead Drop, Smuggler's Run, Bust Out, Double Agent.
- **Halcyon (4):** Lost Colleague\*, Research Data, Cold Chain, Escort the Scientist.
- **World events (4):** Distress Call (+ Fake Distress Call)\*, Wandering Trader, Prison Break, Collapse
  Shelter.
- **Unarmed, house and perimeter (3):** Trail of Clues, Letters (+ Silent Courier), The Crossing.

**Tier B, one real choice (16):**

| Mission | The choice |
|---|---|
| Survivors Trapped | Which house, or Bastion's post (worked example 6) |
| Radio Relay | Whose net it carries: the Haven's alone, or Bastion's too, in return for a patrol past the house |
| Harvest Guard | Which house gets the food: the farm's own, or the hungriest |
| Burn the Nest | Burn the crawlers' nest (as built), or leave it for Halcyon: take a sample first, and the research goes through the lab. The house that asked remembers |
| Rescue the Kids | Which house, or the kids' aunt at Bastion's post (Bastion pays the base, and civilians sheltered goes up on that night's call) |
| Checkpoint Duty (+ Quarantine Line) | People come to the barrier at the infested town's edge. Let them through (+2 people at a house; the town's spread pauses for less time) or turn them back (Bastion's way) |
| Convoy Escort | The driver wants to drop two crates at a hungry house, "lost to raiders". Let him (+2 food at the house; if it's found out by events, +8 Bastion Infamy), or don't |
| Blow the Bridge, Zombie Bait, Lead Them Away | Where the horde goes: an empty town, the edge of a Bastion post (Zombie Bait's pay) or a Red Hand camp. One shared choice block (Cull uses it too), and every answer is grey |
| Fight Pit | Go down in round 3, so a debtor's bet clears his debt (you get half pay, and he walks free of the book), or fight it out |
| Hijack the Convoy | The driver: let him walk, hand him to the crew (+5 Bastion Infamy and a record), or leave him tied for Bastion to find |
| Kill the Officer | He offers his post's stores if you walk away. Take them (Bastion Favor from him, at most the base; the Red Hand pays nothing, and the boss remembers), or do the job |
| Steal the Antidotes | The two doses go to the black market (as built), or to a Haven medic (no Red Hand pay; a house's sick get them; an asset for the Haven) |
| Lab Defence | A raider surrenders. Send him to Bastion's stockade, give him back to the crew for Favor, or hand him to Halcyon "for the trial" |
| Water Survey | Who hears about the town at risk: Halcyon only, or the Haven too (people move out: −1 person at that town's house, +1 at another) |

**Tier C, work (68).** Each is one row in `RZ_MissionReasons.c`: the reasons it answers, the town tally
it writes (or none), and a fallout line.
- **The Haven (2):** Recover the Bus, Memorial.
- **Bastion (5):** Retake the Town (+ Take Back the Town), Last Helicopter, Burn the Bodies (+
  Decontamination), Recon, Fire Mission.
- **The Red Hand (6):** Fence the Goods, Car Theft, Pharmacy Heist, Arson, Sabotage the Relay, Collect
  the Bounty.
- **Halcyon (11):** The Lone One, Field Samples, Power the Lab, Install Sensors, Blood Drive, Specimen
  from the Nest, Hunt the Screamer, Medical Drone, Patient Zero, Live Capture, Mobile Lab.
- **World events (8):** Abandoned Convoy, The Sniper, Stash Note, Mass Grave, Locked Bunker, Tunnel Rats,
  Military Remnant Camp, Wedding Party.
- **Puzzles (12):** The Numbers Station, Two Bearings, Patient Zero's Ledger, The Poisoned Well, The
  Smuggler's Ledger, The Clockwork Horde, Time-Lock Safe, The Bell Order, Lights in the Window, Junction
  Boxes, The Weighing Vault, The Last Experiment.
- **Unarmed (2):** Distraction, Quiet Count.
- **Perimeter (2):** The Blind Spot, Shot at the Line.
- **Leader jobs (17):** Our Own First, Fetch the Family (the built "Bring Them Home", retitled), Over the
  Fence, Ration Run, Names for Bastion, Training Day, Hidden Stores, Hearts and Minds, Rifles for Sale,
  Burn the Ledger, Skim the Toll, The Soft Touch, Sell Out a Rival Crew, Second Opinion, Unsanctioned
  Trial, Leak the Data, Sell the Formula.
- **Story arcs (3):** The Deserter's Map, Fever Town: the water, Fever Town: get them out.

For example, **Retake the Town**:
- it answers `town.infested` near a lived-in town;
- the giver is Bastion while it's around, and the Haven after (Take Back the Town merged in);
- it writes the town tally (Haven chalk or a Bastion stencil, which the epilogue can show);
- its fallout line is *"The town stays theirs."*;
- pay is unchanged.

**Conditions (9):** Horde Migration (The Tribe Moves), Night of the Dead (Every Ridge Calls), Fog Bank,
Power Surge, Siren Test, Gas Station Fire, Bastion Shelling, Storm, Turned Player. They post reasons (a
horde passed, a town was shelled) and answer none.

The 9 merges, the 12 dropped, the 7 that became errands (Rooftop Rescue is the stranger Up on the
Roof) and the 5 stage-only missions are done ([MISSIONS.md](../docs/MISSIONS.md)).

## Worked examples

**Built** (wave 3 and phase 4's second pass, [MISSIONS.md](../docs/MISSIONS.md)): Protection Racket, Lost Colleague, Crash Site,
Distress Call (Fake Distress Call merged in), Struck Off (Hunt the Deserter merged in) and Survivors
Trapped, each with its truths, the main ways, a world record, its lies and its fallout, as each row
there says, simplified. What the six worked examples still add:

| Job | Still to build |
|---|---|
| Protection Racket | Off the book (a Rogue boss; Rein In's drop pays the stronghold); the council head's regard; R3's season weight on time-up |
| Lost Colleague | Sold (his sister, the fence); the step chain and shortcuts to him; the escort (or ★ Nurse patching him); the Haven medic for the notebook; the Doctor wanderer slot and the filter's water need; His Notebook (Notebook 31) as the hiding truth's fallout; the arc's ×3 |
| Crash Site | The recovery team at minute 15; the sneak lift; the carriers' ways other than force (persuade, bribe, the trick with the flight order); the quiet commander's pay and loyalty; the nail (tags, 70%); the open channel by hand at the radio station; the 90% lock |
| Distress Call | The Sneak out the back; the Nurse's dress-and-hide; "My sister's in {town}" and Bastion's post as destinations; the named young in a tribe's range |
| Struck Off | Press and Trade at a Haven door; the commander's loyalty for the stores book; ★ Hunter and ★ Soldier finding shortcuts; let him go as a Deserter wanderer; the epilogue roll on the majors' coda |
| Survivors Trapped | The Nurse's dress-and-hide and the "hid a bite" life record; reasons (`horde.passed`); civilians sheltered on Bastion's call; the Watched amends rule's exception for Bastion is built |

### 7. After the lock

(A contract in the last 10%.)

Research is at 93%. Anna has locked herself to Bastion's route. Her friend Petr hasn't locked to any
route.

**The job.** A Crash Site is offered to both of them, and the truth is Inventory.

**Anna:**
- The stove (asset: Bastion) and the nail (no asset) work for her as usual.
- If she picks "read it on the open channel" (asset: Independent) or carries the box to the fence (asset:
  the Red Hand), she's refused: *"You've picked your side. This isn't it."*
- The labels are the same for everyone. The refusal only comes when she picks.
- She can still fight the horde, read the manifest and do the work.

**Petr** can carry the box to the radio station and air it.
- He decides. It's a hand-in, so it resolves at once.
- Anna isn't in his vanilla group, so she isn't with him. She gets half the Favor from Bastion for
  holding the wreck, with no Fame and no Infamy.
- Petr gets a proof asset for the Independent route, one of his three for the season. It does nothing
  unless he locks to the Independent route.

**Errands:** Anna still gets house and post errands (they're not route work), and her own route's
errands ([FINALE.md](FINALE.md)). Petr, unsided, can also do any route's open-act errands.

**Before 90%:** neither of them could have been refused anything. Every asset only tilts a route, and
nothing closes one.

Errands' own worked example (one evening in Levie) is in [ERRANDS.md](ERRANDS.md).

## The MISSIONS.md block format

Each story job gets one compact block of at most eight rows under its table. Each one-choice job gets
one line.

```
**Protection Racket** · `racket` · Red Hand, Trusted, not Elder · (status)
| | |
|---|---|
| Starts | A hungry house that doesn't pay by arrangement |
| Truths | Just late 45 · Paid twice 25 (another crew within 6 km) · The storekeeper's cut 20 · Off the book 10 (a Rogue boss) |
| Ways | Talk · Persuade [Trusted · Haven] 2 · Press [Feared] 2 · Cover [4 food or 20 Favor] · [The receipt] · Force · Side with the house · Leave |
| Goes to | The boss · the house · the storekeeper · the stronghold (Rein In's drop) |
| If it fails | Time up: the crew collects (−3 food). Fallout: The Pot |
| Reads / writes | Reads `ledger.<house>`, `well.done` · writes `racket.<house>`; life records: pressed, covered |
```

A **Records** table in MISSIONS.md lists each world record, who writes it and who reads it. The errands
have their own table ([ERRANDS.md](ERRANDS.md)).

## Docs, in the same commit as each wave

| Doc | Change |
|---|---|
| DESIGN.md | "Mission building blocks" gains **Story blocks**: the kit, ways, talk scenes, who benefits, who decides, truths, lies, fallout, records and callbacks, reasons and limits. "Questlines: Approaches" links there. "World facts" gains truths, world records and reasons. "Town memory" gains the town tally. The clocks table gains the season day |
| REPUTATION.md | A section, "Talking, threatening, lying": the persuade score's use in missions, Fear and its limits, results, tricks (pointing to Disguises) and the price table. Also *lied* in regard |
| MISSIONS.md | "Who gets offered what" gains reasons, mirror offers and who decides. A block per story job, one line per one-choice job, the Records table. Merged and retired missions are removed; the Counts table is updated |
| QUESTLINES.md | The approaches table grows to eleven and points to DESIGN.md. The stage "Uses" in the references table are fixed |
| WRITING.md | Gains how each faction lies, and the dark-humour rule |
| LORE.md | The rows in the references table are fixed |
| UI.md, UI/LAYOUTS.md | Check tags; the journal's per-job detail ("What they told you", "What you've found", "Who it went to"); talk lines as person cards |
| WORKBENCH_SETUP.md | Settings (Missions > Story, Mirrors). The "used by" rows. VERIFY entries: choice routing, label refresh, how to mark an item as mission goods, the journal detail RPC, decals for chalk. The log lines and play-test checklist |
| ROADMAP.md | The waves go under Next. The 12 dropped missions go on the Dropped list, one line each. Take the Lab moves to the finale |
| README.md | The feature line ("missions with several ways through, givers who lie, and a world that remembers") and the counts |

The dark-humour rule for the writing guide: each story job has at least one darkly funny line, spoken by
a living person in their faction's voice, about the situation. A dead person can be funny only in their
own words, found on them. The narrator never jokes about a victim. Examples: the CO's *"Weather. Write it
down."* The boss's *"Light this week."*

## Waves, effort and where they sit in the plan

| Wave | Plan phase | What | Code added | Removed | Player text |
|---|---|---|---|---|---|
| 1 **Framework** | 3 | **Built** ([docs/DESIGN.md](../docs/DESIGN.md#story-blocks)). Still from it: the look (with disguise), scene choice routing (`HandleChoice`/`HandleUse` for missions that aren't puzzles) and talk nodes, the four guidance tips | ~2,400 | — | ~50 |
| 2 **Pilot** | 4 (the stop point) | The six worked examples; the 9 merges; the 12 dropped and 5 stage-only missions; the Tier 0 audit. Then a play-test. **After this wave the game is whole**, and what follows is growth | ~2,800 | ~3,000 | ~500 |
| 3 **The other 22 story jobs** | 5, in parallel with the majors | A faction at a time, in two halves of 11, each followed by a play-test | ~3,300 | — | ~700 |
| 4 **One-choice jobs and work** | 8 (growth) | The 16 Tier B choices and the 68 Tier C rows; the tribes' contracts on the kit as they're built | ~1,400 | — | ~300 |
| **Total** | | | **~9,900** | **~3,000** | **~1,550** |

The 7 missions that become errand routines leave the pool in phase 3, as their routines land. Every wave
runs `tools/compile_check.sh` and `tools/runtime_test/run.sh`. The runtime test must first start and end
every questline and every reused mission once (the finale's precondition), because this kit moves the
approach helpers all 15 questlines use.

## Engine needs

Built ([docs/DESIGN.md](../docs/DESIGN.md#story-blocks)): `RZ_MissionStory` (`m_Story`), the decider rules,
`RZ_Beneficiaries`, the truth deck, `RZ_MissionReasons.c` with the start of the table, the one clamp and the
journal's detail payload. Still to build:

- **Built in phase 1** ([DESIGN.md](../docs/DESIGN.md#the-shared-engine)): `RZ_Ways` (keyed by identity, the
  approach enum appended), `RZ_Checks` (Trust, Fear, threats, `Result`, the price table, tags), the stores for
  life and world records, the callback queue, and reasons in `RZ_WorldMemory`. Still to add to them: `PassAs` and
  the look ([DISGUISE.md](DISGUISE.md)); `Resolve` for jobs that aren't questline stages; the liar count's and
  crossing lines' writers; the notice board accepting system notes signed with a person's name (unsigned notes are
  built).
- **`RZ_SceneProps`** inside the one scene-people spawner: the spawning and label helpers move out of
  `RZ_PuzzleMission`, which keeps its API. `RZ_PuzzleActionHelper` routes choices and uses to new
  `RZ_Mission.HandleChoice` and `HandleUse` methods for missions that aren't puzzles.
- **Talk nodes:** per person at most 3 nodes, each with up to 4 options carrying fixed tags and results;
  the speaker's 60 s hold, at most 3 minutes in all; lines overheard at 8 m, results and holds to the room
  at 30 m, all through the owner-only personal payload; the numbered fallback; the dialogue screen reads
  the same table later.
- **`RZ_Trade`:** take items by the price table through `RZ_Safehouses.TakeFood`'s keywords, plus
  background services as timed stand-bys. Mission goods are tagged and refused for pay (VERIFY: how to
  tag an item).
- **Disguise hooks** (owned by [DISGUISE.md](DISGUISE.md)): `PassAs`, `IsPassing`, `AddWatcher`; the
  own-colours rule; the mission's extra see-through list (a wanderer who remembers you, a life record
  about you, hunter squads); one mission trick per life per faction per season day.
- **The lie store** (shared): each record with its exposing condition; events first (a visit, road news,
  the owning system's tick), the rumour tick only for claims with no world test; a log line for each.
- **Records:** the quotas per tier, each story job's pinned reads, and the epilogue line and carrier filled in.
- **Reasons:** their posters in safehouse upkeep, supply lines, territory, leaders, wanderers, cure research, the
  Director, the tribes and the majors; the reason lane (one reason-started contract every 30 minutes server-wide,
  inside `MAX_CONTRACTS` and `CanRunNow`); the why-now line from the reason; jobs ordered per player.
- **Fallout errands** handed to the errand dealer at the subject: participants first for 20 minutes, at
  most 30% of the base. None from a mirror, a fallout or `Withdraw`.
- **Limits:** kit extras through the shared capped helpers in the contracts and errands lane of the
  majors' season budget (per faction per season day: morale ±10, strength ±4); loyalty changes of at most
  ±5 per job and −10 per leader per season day, never below 30, counted in the seat ledger; story groups
  at most 4 server-wide and 2 per job, each option with a written fallback; people in scenes plus askers,
  at most 16.
- **Finale interface:** `RZ_Finale.RouteOf(identity)`; `Asset(route, kind, identity, key)`, at most one
  per outcome and 3 per player per route per season; options whose asset belongs to another route refuse
  a locked player when picked, with a line.
- **The season-day helper**, built once in the plan's phase 1.
- **A hand-in at the radio station:** broadcasting while carrying a mission's document tells the mission
  (Crash Site's open-channel option).
- **Four new contextual guidance tips:** tags, truths, background reads, holds.
- **Amends need clean hands:** Press, Trick, a switch or hiding something voids that job's Infamy
  work-off for that player. Every amends goes through `Forgive()`.
- **Registry and data** for the merges (9), the drops (12), the missions that become errand routines (7)
  and the stage-only missions (5): rename Supply Cache to The Cache and Weapons Cache to The Armory;
  retitle the Haven leader job Fetch the Family; take Take the Lab out of the pool; fix every reference in
  the references table.
- **Settings** under Missions > Story (on/off, the mirror chance, the reason lane's gap), every default
  in the attribute's default string, created with `RZ_Util.NewWithDefaults`.

## Log lines and tests

**Log lines** (for WORKBENCH_SETUP's list):
- `Story: <key> <way> <pass|short|refused|caught> by <identity>`
- `Story: <key> resolved by <way> to <party> (Fame, Favor, extras; with N)`
- `Story: truth <key>.<subject> = <truth> (kept|new); found by <how>`
- `Story: lie <key> found out by <visit|road news|tick|rumour>`
- `Callback: <key> fired|expired|left on the board`
- An hourly summary: Fame and Favor per player-hour by way; how often switches and "nobody" are picked
  per job; food spent on trades; fallout starts and their pay.

**Runtime test additions** (with the Wait scale):
- every check result: Persuade pass, short and refused; Press; Trade; a trick caught; Sneak seen;
- each truth revealed, and a truth kept after a simulated restart;
- each beneficiary, with the with-the-decider pay, and an objection;
- a fallout errand;
- a world record written and read;
- a callback firing, and one left on a board at death;
- a reason starting a contract;
- a lie found out by each kind of event;
- a locked player refused another route's asset option.

**Multiplayer play-test cases:**
- two groups trying to end one job opposite ways at once;
- a stranger trying to decide;
- a relog in the middle of a check;
- the decider dying with the item;
- a restart in the middle of an investigation (the truth and the found list must be kept);
- a draw at The Cache;
- a griefer holding the floor;
- whether players find truths, and whether Police and Radio Op feel too strong;
- Fame per player-hour before and after the pilot.

## Risks

- **Action labels replicate the same to everyone.** If the replicated-labels play-test fails, talk
  scenes fall back to numbered options in the line. That works, but it's clumsy.
- **Talk scenes in the vanilla action menu stay shallow** (2–3 nodes, four slots) until the dialogue
  screen is built and turned on.
- **Multiplayer friction:** the speaker floor, the 5 s hold and objections may annoy players or be gamed
  (holding the floor, objecting to everything). Bounded by the 3-minute floor and one objection per
  player per job, but this needs play-testing.
- **Reasons may flood a busy server** with offers, or starve a quiet one. The one-per-30-minutes lane
  needs tuning from the logs; random picks remain.
- **Writing load:** 28 story jobs in five voices, and the epilogue lines. Dark humour can miss. Nothing
  ships until its text pool is full.
- **Lies found out by events may be too rare** (lies always pay) or too common. The log lines exist so the
  author can tune them.
- **Save size:** 160 world records, 32 life records per profile, plus truths, reasons and callbacks.
  Budget about 40 KB in world facts and measure it after wave 1.
- **Balance:** more ways add Fame and Favor per hour, which moves cure pacing and the season. Watch the
  hourly summary after waves 1 and 2.
- **Dependencies on designs not built yet:** route assets and the lock, dialogue nodes, the
  mixed-reputation title, the disguise look, epilogue carriers, the tribes' reasons, and the majors'
  ledger and budget. Each has a fallback here:
  - no assets;
  - the action menu;
  - no title;
  - the built `RZ_Disguise` check (standing of Known or more counts as own colours);
  - board notes;
  - the season-day helper built in phase 1;
  - the kit's own caps.
- **Dropping 12 missions** removes jobs players know. The Dropped list says why, and the classes stay in
  git history.
- **Moving the ways out of `RZ_QuestMission`** touches all 15 questlines. The runtime test has to start
  and end each one first, then pass again after the move.
- **Performance:** stand-ins and story groups near players. The caps (4 groups, 16 people, 3 escorts, 3
  epilogue props per area) must hold under load.
- **Mirror offers can still turn into PvP near safehouses** (house rules still apply). They're off on PvE
  servers, and the second side gets its hint late and rough.
- **A truth kept for 3 season days can feel stale** to someone who meets the same subject twice. Dropping
  it when the job used it up helps; play-test it.

## Tuning the author can do later

These are settings or numbers, not open decisions:
- the mirror chance (10% default; 0 turns mirrors off);
- the reason lane's gap (30 minutes);
- the weight clamp (×0.75–1.3);
- the lie-without-proof cap (60%);
- the threat count (three per life per faction);
- the asset cap (3 per player per route), and whether the journal shows players their own assets.
