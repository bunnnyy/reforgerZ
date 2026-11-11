# The mission style

**Status: designed, not built.** Approved by the author on 2026-11-11.

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

**Rules:**
- This replaces `RZ_Mission_Choice`, which swaps `m_eGiver` and sets the rival. It puts the hard edges'
  rule in one place: the side you chose pays anyone with you whom it doesn't hunt, the giver adds
  Infamy, and the giver's leader doesn't count it as help (`m_bNoLeaderCredit`). Every built choice
  mission gets the switch-pay fix as soon as `RZ_Mission_Choice` is rebuilt on top of it.
- **The base result stays.** Choosing *where* it goes (which house gets the family) is part of the base
  result, not an extra.
- **No profit from switching sides.** A switch pays at most the base. Its Fame doesn't count toward
  season Fame (a flag on `AddFame`), so a group can't steer the season's Fame by switching.
- **Nobody** is a real option: sink the guns, burn the box, let him go. It pays nobody. It leaves a world
  record, and sometimes an asset for the Independent route.
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

[FINALE.md](FINALE.md), §2, owns what an asset is worth in its route. The missions' side:

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

The participation and deciding rules are shared by missions, dialogue scenes, errands, the epilogue's
grey choices and the finale.

- **Who may decide:** someone the contract was offered to (eligible for the giver, or for one side of a
  mirror) who has taken part: 60 s within 150 m, or a step done. For world events, anyone who has taken
  part. Everyone else can talk, listen and read. If they pick an option that would end the job, they're
  told: *"It's not your call. You weren't hired for this."*
- **Scene decisions** (a choice at a person):
  - Picking one starts a 5-second hold, announced to the room: *"Mara is about to hand them to the
    crew."*
  - Any other eligible participant in the room can **Object**, once per job (by identity). The hold is
    cancelled, and the objector becomes the speaker for 60 s.
  - Anyone in the room can **Stand with** the decider during the hold.
- **Hand-ins** (carrying the item to a drop or the radio station) resolve at once, as built. Getting it
  there is the deciding step, and anyone can try to take it off you on the way.
- **With the decider:** the decider, their vanilla group within 150 m, and anyone who chose Stand with.
  Only they get a switched side's pay, its Infamy and the records. They share good or neutral records;
  a bad one (took the cut) lands on the decider alone.
- **Everyone else who was eligible:**
  - if the result went to the giver, they're paid as usual;
  - if it went another way, the giver pays those who did the work half the Favor, with no Fame and no
    Infamy: *"You held it. Someone else sold it."*
- **Leave** works per player. You stop taking part, with no pay and no blame. The job carries on for
  anyone else until it's done or time runs out.
- **Offline players:** a participant who did a step, or spent 5 minutes in the area, and is offline when
  the job ends is paid on their saved profile when they next join, through the questlines' path. That
  applies if the result went to the giver, or if they stood with the decider.
- **Anything that matters to the season** is a tally in `RZ_QuestVote`, never a single pick.
- **Only three distances:**
  - **150 m:** you took part (as built).
  - **30 m:** you're in the room. You get results, reveals and holds, and can object or stand with
    someone.
  - **8 m:** you overhear the lines.

  The last draft's 10 m, 50 m and Distress Call's own 150 m are gone.
- **Amends need clean hands.** If a Watched player in an amends job threatens, tricks, switches sides or
  hides something, that job works off no Infamy for them. The option's own cost still applies. Every
  amends goes through `Forgive()`, inside reputation's 30 per faction per life.

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

**It stays put** (the truth deck in `RZ_WorldMemory`):
- The draw is seeded by the story key, the subject (the house, the man, the wreck) and the season.
- It's stored in world facts (`truth.<key>.<subject>`), together with what's been found.
- A job offered again at the same subject, after a restart or a time-out, gets the same truth and the
  same "What you've found".
- It's kept for 3 season days, or until the job says it's used up. It's dropped if the subject is gone.
- Leaving, or failing on purpose, never draws a new truth.

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

**How each faction lies** (goes into the writing guide: LORE.md's today, WRITING.md once it's built):

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
- **Contracts:** when a reason is posted, the manager looks for one contract that answers it near its
  subject.
  - At most one reason-started contract every 30 minutes, server-wide.
  - It stays inside `MAX_CONTRACTS` and every existing check (`CanRunNow`, eligibility, claims).
  - Its brief opens with the reason's line (*"Levie's on half portions."*). That's the why-now line, and
    it's true by construction.
- **Errands** answer reasons through the people who give them, within their own caps.
- **Random picks** stay, for variety. A random contract gets a why-now line only when a fact fills one,
  never a filler line.
- **One clamp on weights.** Three multipliers stack: leader traits (±20%), faction needs (×0.85–1.2) and
  the majors' season weights (which the majors first combine and clamp to ×0.8–1.2). Their product is
  clamped once, to ×0.75–1.3. Reasons don't change weights.
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

## Mirror offers

- A mirror offer is one mission class with two givers. There are two:
  - **The Cache:** the Haven, and the Red Hand for players the Haven Watches;
  - **The Armory:** Bastion (Trusted) and the Red Hand (Known).

  Convoy Escort and Hijack the Convoy already pair up through Bastion's real truck.
- The last draft's other pairs (the relay, the lab doses, the pharmacy, the bus) were two classes
  claiming one place, and claims block that, so they're left out.
- A setting controls the chance: **10% by default, and off on PvE servers.**
- The second side hears about it 5 minutes later, with a rough area twice as wide, so it can't camp the
  first side's drop.
- If both sides hold at once, the side with more eligible holders inside wins. If that's equal, whoever
  entered first.
- "[Talk] Half each", when both sides are there: each side pays 50%, and neither giver's leader counts it
  as help.
- The losing side gets a line, not a fallout errand.

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

The audit, for example on **Walk Him Out**:
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

**Merges (9):**

| Merged away | Into | Why | How |
|---|---|---|---|
| Fake Distress Call | Distress Call | The same announcement. A lie works better as a truth than as a second event | Worked example 4 |
| Hunt the Deserter | Struck Off | The same man and the same hard edge, and they already never run together | Worked example 5 |
| Raid the Cache | Supply Cache, renamed **The Cache** | The same cache, seen from both sides | One class with two givers (a mirror). Hold it for 60 s; whoever holds it last decides |
| Take Back the Town | Retake the Town | The same job; only the giver differs | The giver depends on Bastion's presence. The town tally says whose it is |
| Decontamination | Burn the Bodies | The same corpse piles (its class already extends Burn the Bodies) | The giver depends on the place: Halcyon in the lab's town, Bastion elsewhere. Authors' stage 5 (`RZ_QM_AuthorsDecon`) extends Burn the Bodies, with the lab as giver |
| Loot the Armory | Weapons Cache, renamed **The Armory** | The same gun cage and the same case | A mirror. "Nobody": drop the case down a well (both sides are angry and nobody pays; a world record). ★ Police knows the layout |
| Pharmacy Lure | Medicine Run | The lure is a way of doing a medicine run | Medicine Run gets [Sneak] draw them out with noise. It stays open to anyone, unarmed |
| Silent Courier | Letters | Both are the Courier's letters | Going quietly through the infested town becomes a way (heat under 35: pay ×1.25). New truth: who the letter is really for |
| Quarantine Line | Checkpoint Duty | The same ten-minute hold at an infested town's edge | Checkpoint Duty moves to an infested town's edge. Success pauses the town's spread for 30 minutes (Quarantine Line's result). Its choice is about the people at the barrier |

**Out of the pool (24).**

*Dropped (12). Each goes on the roadmap's Dropped list with this line:*
- **Clear the Road:** kills along a straight line. The tribes' Clear the Trail clears roads for a reason.
- **Signal Fires:** needs three players online at once, and has no story in it.
- **Restore Comms:** a parts fetch and a hold at a mast. Radio Relay is the same job.
- **Clear the Airfield:** exists only to make the helicopter run more often, on a field that isn't an
  airfield.
- **Toll Road:** the Red Hand roadblocks already built are the toll roads. Skim the Toll keeps the grey
  part.
- **Quarantine Breach:** ten minutes of killing in a safe town, with no choice in it.
- **Crashed Helicopter:** the same wreck as Crash Site. B3's ending starts Crash Site instead.
- **Burning Town:** the second fire event. Gas Station Fire keeps the set piece.
- **Paces from the Tower:** a third stash hunt beside Stash Note and Two Bearings.
- **Hot and Cold:** a radio stash hunt. The Numbers Station keeps the radio puzzle.
- **The Old Clinic:** a generic hold.
- **Field Station:** a three-minute hold for one research point.

*Become errand routines (7):*

| Mission | Errand ([ERRANDS.md](ERRANDS.md)) |
|---|---|
| Fuel for the Lights | Lights |
| Water Purifier | Filter Parts |
| Water Run | Water for {house} |
| Medicine Before Dark | Before Dark |
| Rooftop Rescue | Up on the Roof |
| Barter Chain | Barter Chain |
| Car Keys | Ask About Cars |

*Questline stages only (5):* Fortify the Safehouse (H2 stage 5), House Rules (H2 stage 5), Curfew
Patrol (B2 stage 3), Our Crossroads (the Red Hand's major, acts 1 and 4: its hold and stand-ins) and
Coup Talk (the Red Hand's major, act 4: "laughed off or purged"). They leave the random pool, and each
class stays where its stage uses it.

**References to fix:**

| Where | Fix |
|---|---|
| `RZ_MissionRegistry.c` | Take out of the random pool the 9 merged, 12 dropped, 7 errand and 5 stage-only types. Rename Supply Cache and Weapons Cache. Take the Lab leaves the pool |
| `RZ_LeaderJobsHaven.c` | "Bring Them Home" is retitled Fetch the Family. Our Crossroads and Coup Talk leave the leader-job pools |
| `RZ_UIFeed.c` (the unarmed icon list) | Remove Pharmacy Lure, Silent Courier, Water Run and Car Keys |
| `RZ_BastionLoreMissions.c` | Remove Struck Off's "never with Hunt the Deserter" check |
| `RZ_Quest_Authors.c` | Stage 5's Decontamination extends Burn the Bodies, with the lab as giver. Stage 4's Take the Lab follows [FINALE.md](FINALE.md) and [MAJORS.md](MAJORS.md) |
| QUESTLINES.md | **B1** stages 2 and 5 and **R1** stage 2: "Hunt the Deserter's trail" becomes Struck Off's trail (the step chain). **H3** stage 3 and **HC2** stage 1: Silent Courier becomes Letters' quiet way. **HC3** stage 5: Decontamination becomes Burn the Bodies. **B3**: starts after Last Helicopter only (`pilot.lz_kept`), "as Clear the Airfield" is reworded, and "He flew for the line" starts Crash Site instead of Crashed Helicopter. **H2** stage 5 and **B2** stage 3 keep their stage classes |
| LORE.md | "Struck off" row: Struck Off and Desertion Route. "Containment": Checkpoint Duty and Bastion Shelling. "The pit": Fight Pit and The Pit's Debtor. "Crews eat crews" and "He was bitten": Coup Talk is named as the Red Hand major's settling-day stage |
| REPUTATION.md | "What makes AI shoot you": the Fake Distress Call ambush becomes Distress Call's crew (the A loop and Bait truths) |
| DESIGN.md | Escort and step chain "Used by": Struck Off instead of Hunt the Deserter. The Mechanic wanderer's Car Keys job becomes an errand. The safehouse food line "The Water Run brings 2" becomes Water for {house}. Town memory: a town counts as Barricaded through Fortify the Safehouse only by H2's stage. Fire: Burning Town goes. Loose items: the Crashed Helicopter's wreck becomes Crash Site's |
| WORKBENCH_SETUP.md | The prefab "used by" rows: the escort soldier (remove Hunt the Deserter); the corpse pile (Decontamination becomes Burn the Bodies); the puzzle object and the Red Hand groups (Fake Distress Call becomes Distress Call); and the rows for Restore Comms, Water Purifier, Burning Town, Car Keys and Crashed Helicopter |
| MISSIONS.md | The Counts table and every mission table |

## Worked examples

### 1. Protection Racket

(The Red Hand, Trusted, not Elder; a story job; story key `racket`.)

**Today:** the keeper pays 55% of the time. If refused, you lean on them: either a fight with the
keeper's people, or +5 Haven Infamy. The house loses 3 food. There's one way through, and it's a coin
flip.

**What starts it:**
- The reason `house.hungry`, at a house that doesn't pay by arrangement (`door.tins` and the majors'
  `haven.paying` are skipped).
- It's given by the crew boss whose seat is nearest the house.
- Why now: *"Levie's down to {food} days. Everyone's late this week."*

**Truths.** These weights apply before traits. The truth is kept per house for 3 season days.
- **Just late (45%):** the house really is short. Nothing is hidden but hunger.
- **Paid twice (25%; needs another crew boss alive within 6 km):** the house paid the next crew last
  week for the same protection. The boss knows, and wants a fight between crews that he can win. Found
  out by:
  - [Talk] with the storekeeper: *"We paid. Ask Janda at the pit."*;
  - reading the other crew's hand painted on the door (anyone; ★ Ex-Con also says whose hand it is);
  - Sneak: the receipt pinned up in the stores.
- **The storekeeper's cut (20%; not while The Ledger runs at that house):** the house did pay. The
  storekeeper kept tins back to trade for her sick daughter's antibiotics, and the council doesn't know.
  Found out by:
  - [Talk] with the daughter in the doorway (anyone): *"Mum says the tins are for my medicine."*;
  - Sneak: the stores ledger, after dark;
  - ★ Police: she's lying;
  - ★ Nurse: the antibiotic boxes.
- **Off the book (10%; needs a Rogue Red Hand leader):** the boss sending you is working for himself,
  not the crew. Found out by:
  - "Ask about..." at the Red Hand stronghold's quartermaster, if you're Known: *"Who's collecting in
    {town}?" "Nobody we sent."*;
  - [Talk] with the boss's runner at the house: *"Keep it off the book."*;
  - ★ Ex-Con;
  - ★ Radio Op hears the secure line: *"Who sent a collector to {town}?"*

**Ways at the storekeeper** (*"I know who sent you. We've nothing to spare."*):
- **[Talk]** Ask. Each truth answers differently.
- **[Trusted · Haven] Persuade**, difficulty 2 (★ Nurse or Police +1). She pays what she can: −2 food,
  75% pay, no Infamy.
  - **Short:** she pays the same if you take the life record "you'll speak for us at the next
    collection".
  - **Refused:** she won't talk to you again this job.
- **[Feared] Press**, Fear 2. Your Red Hand Fame counts as backing (at most 2), or your Haven Infamy this
  life; ★ Ex-Con or Raider +1. She pays the full −3 food, and you get +5 Haven Infamy (the scene names
  it) and the life record "pressed at {house}". This is a threat the mission writes, so it stays,
  safehouse or not.
  - **Short:** she still pays the full −3, but it's +8 Infamy.
  - **Refused:** the keeper's people come out (with the survivor group prefab and room under the AI cap).
    Otherwise the door shuts and she doesn't pay.
- **[4 food | 20 Red Hand Favor] Cover it yourself.** The house loses nothing, and the boss pays you 50%
  (*"Light this week."*). Tell the storekeeper, or don't. It's a lie in the store, found out only by
  events: the next collection at this house (another racket, or Off-Book Requisition). Found out: +5 Red
  Hand Infamy and a jibe.
- **[The receipt] Show the boss** (Paid twice; carry it to his seat). This is proof, not a lie, so it
  pays in full. The boss goes after the other crew (in words; Red Hand morale −2), and the house pays
  nothing.
- **The storekeeper's cut, once found:**
  - **[Keep her secret]:** she pays out of her hidden tins (−1 food), and you get the life record "kept
    her secret";
  - **[Tell the council]:** the council head pays from the stores (−3 food) and puts her out (−1 person,
    as Short Weight does). The council head remembers you helped, and there's no Haven Fame. A girl
    loses her mother's place.
- **[Force]** As built, with +10 Haven Infamy. Without the prefab or room under the AI cap: +5 Infamy,
  and she pays.
- **[Side with the house].** Only the eligible contractor can choose this, after a 5 s hold. You warn the
  council head, and they hide the stores. Nobody pays you. The council head remembers you helped, you
  get +10 Red Hand Infamy, and the boss remembers you crossed him.
- **[Leave]**, for you only.

**Who decides.** The speaker at the storekeeper, who must be an eligible Red Hand contractor. A Haven
player standing there can talk, read the door and pin a board note, but can't decide.

**Who gets the result:** the boss; the house; the storekeeper; or the stronghold (Off the book: Rein
In's drop pays the stronghold's base). No route assets.

**If it fails:**
- **Time up** (40 min): the crew collects without you (−3 food), and "the well story" is told about the
  house. That's R3's season weight ×1.1, inside the one clamp.
- **Fallout errand:** The Pot at the house, dealt to the participants first.

**Records and callbacks:**
- **Writes:** the world record `racket.<house>` (how it was paid: pressed, covered, persuaded, warned, or
  the receipt); the life records pressed, covered and kept her secret.
- **Reads:** The Ledger's record at that house (*"You again."*); `well.done` (the story's been told, so
  Press is difficulty 1).
- **Callback (covered, and you told her):** 2–6 hours of your play later, a note on the board: *"They came
  back. Told them a friend of the crew had paid. Thank you. Don't do it again."*
- **Epilogue** (prop or board): *"Levie's stores still have tally marks for every tin the Red Hand took.
  Mara's are in a different pencil."*

**Caps:** pay at most the built base; extras go only to the player who chose them; the house loses at
most 3 food (the base); at most 10 Infamy per way.

**Spawned near players only:** the storekeeper, the daughter, the runner (Off the book only), the door
mark, and the keeper's people (optional, under the AI cap).

**Code:** a puzzle mission plus the kit, about 180 lines.

### 2. Lost Colleague

(Halcyon, Known; also The Missing Courier arc; a story job; story key `colleague`.)

**Today:** walk within 20 m. Half the time he's turned and you put him down. Research +2.

**What starts it:**
- The reason `lab.short` (a field station has gone silent), the arc, or a random pick.
- It's given by the field lead of his station, or else the director by radio.
- Why now: *"{n} days since his last reading."*

**The search:** a step chain from his field station (his cold box, then his pack). Shortcuts:
- ★ Radio Op hears his set off the net (a grid within 300 m);
- a Scavenger wanderer sells the last sighting for one food (Trade);
- the Haven trader saw "a man with a lab badge" (Talk, anyone).

**Truths** (kept per name for 3 season days):
- **Hiding, hurt (35%):** he's alive in a house with his notes and a broken ankle. He asks you not to say
  he went out alone, against orders: [Say nothing] or [Tell the field lead]. If you tell, he loses his
  field post. Halcyon pays the same either way.
- **Turned (30%):** as built. His notebook records his own turning, hour by hour. Anyone who reads it
  learns he was bitten a day before he left, and ★ Nurse or Doctor sees the bite on the body. On the last
  page: *"Don't give them this part."*
- **Walked out (25%; needs a safehouse within 3 km):** he's alive and well at a Haven house under
  another name, working on the water. Found out by:
  - [Talk] with the house's people: *"We've a new man. Good with pumps."*;
  - his badge in the house's bin (a search);
  - ★ Lab Tech recognises his handwriting on the filter log.
- **Sold (10%; needs a Red Hand seat within 5 km; ×3 in the arc):** he traded his notes to a crew for
  doses for his sister. Found out by:
  - [Talk] with his sister at the nearest house: *"He got them from the crew. Don't tell anyone."*;
  - Sneak: the fence's table;
  - ★ Ex-Con.

**Ways:**
- **Hiding:** escort him (20 min), or ★ Nurse or Doctor patches him and he walks himself.
- **Turned:** put him down, and take the notebook.
- **Walked out:**
  - **[Trusted · Halcyon] Persuade**, difficulty 2 (★ Lab Tech +1): *"The lab will take you back as a
    medic, not a sampler."* That's a promise, written as a world record that the field lead must keep.
  - **[Feared] Press** him back: +4 Haven Infamy (the house saw), and a life record.
  - **[4 food] Trade** with the house for his notes: *"Tell them what you like."* Ask after that, and he
    gives you his tag: *"Tell them I turned."*
- **Sold:**
  - [25 Red Hand Favor] bribe the fence;
  - Sneak the notes off the table at night;
  - [Known · Red Hand][5 food] they sell them back.

**Who gets the result:**
- **The lab:** research +2 for bringing him back, +1 for his notes alone (×0.75, through the lab's
  capacity; from the gate, into the backlog).
- **Turned:**
  - the whole notebook to the lab (+2);
  - [Tear out the last pages] (+1; his wish kept; Halcyon pays 60%). If a later Halcyon act reads the
    notebooks, the world says *"Someone tore pages out of Novak's."* Nobody is named;
  - or the notebook goes to a Haven medic (Second Opinion's drop). No cure research: −3 infection in the
    medic's town instead. Nobody pays. Asset: the Haven, proof.
- **Walked out:**
  - **[Say he turned]** needs his tag in hand. Halcyon pays 60% and no Fame: it pays for the tag, not
    the man. It's a lie in the store, found out only by events: a Halcyon job or errand later runs at his
    town; a Halcyon-Known player talks to him (in the house, or as a Doctor wanderer). Found out: +10
    Halcyon Infamy, the director's line once (*"You told us he turned. He signed a Haven ration card in
    {town} last week."*), and the field lead's regard records *lied*.
  - **[Leave him be]:** no pay. He takes a Doctor wanderer slot if one is free. The house's water need
    drops, because the filter works. Asset: Independent, ally (he'll check a batch for whoever takes the
    cure for everyone).
- **Sold:** the notes stay with the Red Hand, and the arc's next step has its proof. Asset: the Red
  Hand, proof.

**If it fails:**
- **Time up while he's hiding:** he dies overnight. Fallout errand: Notebook 31's His Notebook variant
  (carry the notebook from the body to the station; Known with Halcyon).
- **Walked out:** he moves on, and no record is written.

**Records and callbacks:**
- **Writes:** the world record `colleague.<name>` (lab, turned, walked, sold or torn), which Proof's
  author list and his house read; the life record "lied to Halcyon".
- **Reads:** `weeks.public` (Walked out ×1.5); HC2's names; the arc.
- **Callback (left him be):** a board note at the house: *"The filter's working. Tell no one who fixed
  it."* Words only.
- **Epilogue** (voice or board): *"Dr Novak is on the paper as an author, deceased. In Levie he's the man
  who fixed the filter."*

**Caps:** research at most +2, ×0.75, through the lab's capacity.

**Code:** becomes a puzzle mission with a step chain and an escort; `m_bCourier` keeps the arc's title
and weights. About 200 lines.

### 3. Crash Site

(Bastion, Known, ★ Mechanic; a story job; replaces its `RZ_Mission_Choice`; story key `crash`.)

**Today:** secure the wreck for 60 s against a horde, take the black box, and deliver it to Bastion's
drop or the Red Hand's.

**What starts it:**
- A random pick, or B3's "He flew for the line" ending, which now starts Crash Site at the pilot's wreck
  (Crashed Helicopter is dropped).
- It's given by the outpost commander nearest the wreck.
- Why now: *"The helicopter flew over it yesterday. The box light's still blinking."*

**Truths** (kept per wreck for 3 season days):
- **Weather (45%):** icing and a stall. The box holds the last minute: ice warnings, the pilot calm, then
  nothing.
- **Fell off the inventory (35%; needs a Bastion outpost or interim post):** a manifest in the cockpit
  lists 12 rifles signed out by {leader}'s post on Day {n} (always Day 9 or later), routed via a Red Hand
  crossroads, "for diesel". The box says the engines were fine: he was landing there. Found out by:
  - reading the manifest (anyone);
  - [Talk] with the recovery team's sergeant if you're Trusted: *"Not our flight plan."*;
  - ★ Mechanic reads the box;
  - ★ Soldier or Lab Tech reads the unit codes.
- **Already taken (20%; needs a Red Hand seat within 5 km):** a crew got there in the night. Bastion's
  patrol saw them and turned back, and the brief doesn't say so (Bastion lies by leaving things out).
  Found out by:
  - reading the wreck (anyone): cut straps, and drag marks heading east;
  - [Talk] with the post's sentry, if Known: the patrol log;
  - ★ Hunter: *"Two men, heavy load, east."*

**Ways at the wreck:**
- **Force:** secure it for 60 s, as built (★ Mechanic twice as fast). Inside a tribe's range, the horde
  is its hunters (by day, its day party).
- **[Mechanic]** read the box. **[Radio Op]** play it.
- **Sneak:** lift the box before the horde finds you, in the 90 s after the first player comes within
  150 m. If you're seen, the horde comes at once and you hold with the box in your hands.
- **[Talk]** Bastion's recovery team arrives at minute 15 (with the squad prefab and room under the AI
  cap):
  - Trusted: you keep the box;
  - Known: they take it, and you're paid half;
  - Watched: they fire.

  Without the squad, a truck stops at the road and shouts for the box. Watched players get +5 Bastion
  Infamy instead.

**Ways when it's already taken:**
- **Force:** the two carriers (Red Hand group prefab, under the AI cap).
- **[Known · Red Hand] Persuade**, difficulty 1: *"Bastion pays twice what the fence does."*
- **[25 Red Hand Favor | 5 food]** Bribe or Trade.
- **Sneak:** take it off them at their fire, at night.
- **[Bastion kit + flight order] Trick:** *"That's ours, lads."* A claim at the carriers (difficulty 1,
  away from home), with the crew's flight order from the cockpit as the cover detail. Bastion's full kit
  and the order score 3; colours and the order score 2.
  - Caught: the catch (+3 Red Hand Infamy, the kit taken, one of three catches), and the carriers run for
    the fence. If they get there, it's the fence's result.
  - One mission trick per life per faction per season day.

**Who gets the result:**
- **Bastion's drop:** the base Fame and Favor. If the truth is Inventory, the box goes in the stove: the
  commander's loyalty +5, and the CO's line next time is *"Weather. Write it down."* Asset: Bastion,
  standing.
- **The commander, quietly** (Inventory): Bastion Favor from his own stores, at most the base. It's
  found out only by events (the manifest turning up another way, or a Talker CO's leak on the rumour
  tick): then +10 Bastion Infamy, and his loyalty −5 (in the ledger, never below 30).
- **The nail** (Weather only): take the crew's tags to the post. 70% pay, Bastion morale +2, and the
  names are read on that night's call.
- **The Red Hand fence:** Red Hand pay, at most Bastion's base, and +20 Bastion Infamy (as built). If
  the truth is Inventory, the manifest is read on the open channel: *"Signed correct, every one. Turns
  out they were signed to us."* Bastion morale −3, a headline, and the commander's loyalty −5. Asset: the
  Red Hand, proof.
- **The open channel** (Inventory): read it out at the radio station yourself. No pay. Bastion morale −3,
  +10 Bastion Infamy, and the commander remembers you crossed him. Asset: Independent, proof.

**Who decides.** Whoever carries the box to a drop or to the radio station; a hand-in resolves at once.
Only that carrier's vanilla group within 150 m counts as being with them. The other eligible players
who held the wreck get half the Favor from Bastion, with no Fame and no Infamy: *"You held it. Someone
else sold it."*

**If it fails:**
- **Time up with the box still on the ground:** it becomes Already taken, after the fact.
- **Fallout errand "Buy It Back"** (Bastion, Known): buy it from the fence for 25 Red Hand Favor or 5
  food, then hand it in. Bastion pays 25% of the base. If nobody buys it back within a season day and
  the truth was Inventory, the Red Hand reads it on the air.

**Records and callbacks:**
- **Writes:** the world record `crash.box` (where it went, and what it was); the commander's memory; the
  life record "aired it".
- **Reads:** B3's `pilot.flew_for` (the pilot asks to hear the box, as a callback); No Reply's operator
  record (a ★ Radio Op line); `majors.pages_open` (if a page is already out, the stove pays 80%);
  `orders.stance` (Crash Site's pay under the muster's stance, [MAJORS.md](MAJORS.md)).
- **Callback (the nail):** 2–6 hours of your play later, the operator reads the names on the nightly call
  and adds *"civilian, recovered"*.
- **Epilogue** (prop: tags on a nail by the post's radio; otherwise voice): *"Behind the radio in {post},
  three more tags on the nail. Under them, in grease pencil, the weather."*

**Caps:** one leader's loyalty ±5 per job (never below 30); morale extras ±3; never a replacement.

**Engine needs:** a hand-in at the radio station; the recovery team and carriers count under the
story-group cap.

**Code:** about 180 lines.

### 4. Distress Call

(World event; Fake Distress Call merged in; a story job; story key `distress`.)

**Today:** two events with the same announcement. Radio Operators hear which is which.
- The real one: two people are cornered, the infected come, and you talk them out with nothing within
  20 m. They join a safehouse and leave a crate.
- The fake one: a tape on a crate, and two Red Hand groups in the trees.

**After:** one event, one announcement and a deck of truths (kept per site for one season day).
- **Real (50%):** as built.
- **A loop (30%; needs the Red Hand group prefab):** as the fake call works today.
- **Bait (20%; needs the Red Hand group prefab):** real people, tied up in the house. A crew in the trees
  waits for whoever comes to cut them loose (the "cargo" hard edge).

**Finding out.** Each truth has a way anyone with a radio can use, plus background and standing ways:
- **Use your radio and answer the call:**
  - Real: they answer you;
  - A loop: the voice repeats and doesn't answer;
  - Bait: she starts to answer, and a man's voice says *"Say it again."*
- **At 30 m**, the tied people say *"They're in the trees!"*
- ★ Radio Op hears "live", "a loop" or "live, but someone off-mic is telling her what to say".
- ★ Hunter at 150 m: *"Boots in the trees. Six, maybe eight."*
- A player Trusted with the Red Hand gets a secure-net line: *"Leave the house on {road} alone. It's
  spoken for."*

**Ways:**
- **Real:**
  - clear the infected and Talk them out (as built);
  - **[Sneak]** lead them out the back while the infected are at the front, in a 2-minute window. They
    follow you only to the edge of town, then go to the house as data. If you're seen, the infected turn
    on all of you in the open;
  - **[Nurse]** one of them is bitten. The house checks everyone at the door, so anyone else just sees
    the house's rule applied. Only a Nurse who knew can [Dress it and say nothing]. Then he turns at the
    house within a season day, and the house names her: +8 Haven Infamy, and only to her.
- **A loop:** leave; Force (as built); or **[Known · Red Hand]** walk up laughing (they let friends go,
  but give no kit).
- **Bait:**
  - Force (each death among the tied people is named);
  - **[Known · Red Hand] Persuade**, difficulty 2: *"Not these two. They're mine."*;
  - **[30 Red Hand Favor | 6 food]** buy them;
  - **[Sneak]** cut them loose from the back, at night only;
  - **[Side with the crew]:** the speaker only, after a 5 s hold that others in the room can object to.
    You take 10 Red Hand Favor and walk away. If a Haven body or wanderer witnesses it (the one witness
    rule, [ERRANDS.md](ERRANDS.md)) or a Haven-Known player is within 150 m at that moment, +12 Haven
    Infamy at once. Otherwise it's found out later by events, if Tied Down runs and frees them: they name
    you, for the same +12. The two go on the next Red Hand truck, and Tied Down's brief mentions them.

**Who gets the result:**
- the nearest safehouse (the base);
- the house they ask for (*"My sister's in {town}"*): farther away; no extra Fame, but a record and a
  callback;
- Bastion's nearest post ("the soldiers"): Bastion pays the base instead, and civilians sheltered goes
  up by 2 on that night's call;
- the crew (Bait, if you take the cut).

No route assets.

**Who decides:** anyone who took part (60 s within 150 m), through the speaker and the hold.

**If it fails:**
- Time up (25 min), or they die.
- **Fallout errand:** The Name on the Tag, taking their IDs to the nearest house.
- With tribes built, the dead who fell inside a range run with that tribe as named young, and Bring Them
  Home can find them.

**Records and callbacks:**
- **Writes:** the world record `distress.<surname>` (saved at a house, to Bastion, dead, or taken); the
  life record "took the cut", which Haven voices remember for the life.
- **Reads:** Tied Down; R2's `cargo.*` (if the Cargo driver is out, Bait ×0.5).
- **Callback (saved):** 2–6 hours of your play later, the woman asks for you by name at her house (face
  to face, filtered, never if you're anonymous): a stand-in at the door within 300 m, or else a board
  note.
- **Epilogue** (prop: a radio with the batteries out; otherwise board): *"In {town}'s school, a radio with
  the batteries taken out, and a note: 'In case we need to call again. We won't.'"*

**Code:** the two classes become one, on the shared base. About 150 lines added and 250 removed.

### 5. Struck Off

(Bastion, Trusted; Hunt the Deserter merged in; a story job; story key `struck`.)

**Today:** two jobs about the same man, which never run together.
- **Hunt the Deserter:** a trail (his pack, his fire, his hideout). Arrest him, or kill him for 60%.
- **Struck Off:** a soldier written up as killed in action is alive. The Haven trader, or the Deserter
  wanderer, knows where he is. Choose the roster, arrest, the Haven or the nail.

**After:** one contract. Under the roster lore ([../docs/LORE.md](../docs/LORE.md)) every deserter is
struck off as "KIA. Body not recovered", his name stays on the board, and nobody answers for him at the
roll until someone brings him home. Struck Off never uses the finale's canon name (Private Fiala).

**What starts it:**
- The reason `leader.died` at a post (*"the new commander wants his numbers right"*), B1's `orders.read`,
  a struck-off man the majors' Effective Strength reports alive, the posts the majors weight it up at
  for a season day, or a random pick.
- The commander's brief is Hunt the Deserter's: *"Private {name} walked off post with a week's rations.
  He's on the board as killed in action. Bring him back, or bring back his tag."*

**Truths** (kept per name for 3 season days):
- **He stole and ran (35%):** the rations are at his hideout. He's armed and scared.
- **Written off (30%):** he never took a tin. He walked away, and the rations story covers a gap in the
  stores. Found out by:
  - [Talk] with him (anyone);
  - Sneak: the post's stores book, at night;
  - ★ Soldier reads the unit codes.
- **The quartermaster's gap (20%; needs an outpost commander):** the rations were sold (they fell off the
  inventory), and his walking off was convenient. Found out by:
  - Sneak: the stores ledger, at night (anyone);
  - ★ Police or Lab Tech.

  If the record `count.cooked` is there (someone wrote the number they wanted in the errand The Count),
  this truth's weight doubles, and the ledger shows it.
- **A Haven door (15%; needs a safehouse within 3 km):** he took the rations to a house where his
  girlfriend's two kids are, and the house hides him. Found out by:
  - [Talk] at the house: *"We've no soldiers here."* The kids say otherwise;
  - ★ Hunter: tracks to the door.

**Finding him:**
- the old step chain (★ Hunter: half the time at each step);
- the Haven trader;
- the Deserter wanderer, if he'll talk to you;
- ★ Soldier: he comes to the door for a soldier.

**Ways with him:**
- **[Talk]:** he tells his side, which reveals the truth for everyone in the room.
- **[Trusted · Bastion] Persuade** him back, difficulty 2 (★ Soldier +1).
- **[Feared] Press:** +3 Haven Infamy at a Haven door. A threat the mission writes, so it stays.
- **[4 food] Trade, for the kids:** the rations stay with them.
- **Force:** his tag.

**Who gets the result:**
- **The roster:** walk him to the post. His KIA's rubbed out, and he answers at the roll. 80% pay;
  morale +3; strength +1.
- **The stockade:** arrest him. The base Fame.
- **The nail:** his tag. 60% pay.
- **The Haven:** walk him to a safehouse (+1 person). Bastion pays nothing, and the council head
  remembers you helped. It's found out by events only: a later Bastion job or errand at that house brings
  +15 Bastion Infamy. At a Haven door the rations stay: +2 food (the base result, sent there).
- **Let him go:** no pay. He takes a Deserter wanderer slot if one is free, or else walks off as data.
- **The stronghold** (the quartermaster's gap, or written off): carry the ledger to the duty officer.
  +5 Bastion Fame (an extra, for the chooser only), the commander's loyalty −5 (in the ledger, never below
  30), and he remembers you crossed him.

No route assets.

**If it fails:**
- **Time up:** a hunter squad finds him (as data), his tag goes on the nail, and a headline: *"Struck off
  twice."*
- **At a Haven door:** Requisition's weight on that house ×1.2 for a season day, through the
  season-weight helper and inside the one clamp.

**Records and callbacks:**
- **Writes:** the world record `struck.<name>` (roster, stockade, nail, Haven or gone); the commander's
  memory.
- **Reads:** B1's `orders.read` and `orders.mocked` (Written off ×1.5: more men walk once the orders are
  out); Desertion Route's record (the Haven door picks that commander's house); the Deserter wanderer's
  memory; `count.cooked`.
- **Callback (let go, and he became a wanderer):** he remembers you (the built one-off "saved my life"
  gift).
- **Epilogue** (radio: Bastion's last call in the majors' coda): the nail's tags are read out. At the
  last roll, a struck-off man walked home this season answers *"Here"*; for one still out, nobody
  answers.

**Code:** Struck Off gains the step chain and the truths. Removed: Hunt the Deserter's class, its
registry entry, the "never with Hunt the Deserter" check in `RZ_BastionLoreMissions.c`, and its rows in
the docs (see the references table). About 150 lines added and 300 removed.

### 6. Survivors Trapped

(The Haven, Any, ★ Nurse; Tier B, the one-choice recipe; story key `trapped`.)

**Today:** reach a barricaded family and survive 3 waves. They join a safehouse (+3 people, +4 with ★).

**Tier B gives it** reasons, one hand-written choice, a world record and a fallout line. It gets no new
ways, truths or callback.

**What starts it:** the reason `horde.passed` (or a tribe's splinters) at a town; otherwise a random
pick. Why now, when a reason starts it: *"The noise came from {town} last night. Somebody's still in the
chemist's."*

**The choice.** After wave 3 the family asks where they're going. The speaker chooses, after a 5 s hold:
- **the nearest safehouse** (the base: +3 people, +4 with ★);
- **the hungriest house** ("they can work"): the base +3 goes there, plus Haven morale +1 (an extra);
- **Bastion's nearest post** ("the mother wants the soldiers"): Bastion pays the base instead, civilians
  sheltered goes up by 3 on that night's call, and the Haven pays nothing.

**The bite: a fixed fact about this family, never a gotcha.** One time in four the father is bitten.
- ★ Nurse or Doctor sees it on arrival.
- Anyone else learns at the door, because the house checks everyone (the house rules). The house then
  applies its rule: the bite is slowed, or he's walked out at dusk, and Walk Him Out may follow as a
  reason. Nobody who didn't know takes any Infamy.
- A Nurse who knew may [Dress it and say nothing]. Then the house doesn't see it at the door. He turns
  within a season day, and the house names her: +8 Haven Infamy, to her only.

**Amends:** a Watched player who sends the family to Bastion, or hides the bite, works off nothing this
time.

**If it fails:**
- **All dead:** fallout errand The Name on the Tag (the same one Distress Call uses).
- **Some dead:** the rest still come, and the house's gossip names who didn't.

**Records:**
- **Writes:** the world record `trapped.<surname>` (which house, with an epilogue line); the life record
  "hid a bite", which that house's people read (+1 difficulty for you there, for the life).
- **Epilogue** (chalk, VERIFY; otherwise board): *"The {surname}s' boy chalks the days on {town}'s gym
  wall. He's up to {day}."*

**Caps:** the base result is unchanged; the extras are morale +1 at most.

**Code:** stays `RZ_Mission_Hold`, plus a choice, a record and a reasons row. About 70 lines.

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
| LORE.md (WRITING.md once it exists) | The writing guide gains how each faction lies, and the dark-humour rule. The rows in the references table are fixed |
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
| 1 **Framework** | 3 | The kit; ways moved out of `RZ_QuestMission` into `RZ_Ways`, keyed by identity; the shared checks (persuade, Fear, the look, results); scenes and choice routing; the decider rules; who benefits (with `RZ_Mission_Choice` rebuilt on it, so every built choice mission gets the switch-pay fix at once); truths with persistence; world records; callbacks; reasons, the reason lane and the one clamp; the journal's detail payload; log lines. No content changes. The runtime test passes unchanged | ~2,400 | — | ~50 |
| 2 **Pilot** | 4 (the stop point) | The six worked examples; the 9 merges; the 12 dropped and 5 stage-only missions; the Tier 0 audit. Then a play-test. **After this wave the game is whole**, and what follows is growth | ~2,800 | ~3,000 | ~500 |
| 3 **The other 22 story jobs** | 5, in parallel with the majors | A faction at a time, in two halves of 11, each followed by a play-test | ~3,300 | — | ~700 |
| 4 **One-choice jobs and work** | 8 (growth) | The 16 Tier B choices and the 68 Tier C rows; the tribes' contracts on the kit as they're built | ~1,400 | — | ~300 |
| **Total** | | | **~9,900** | **~3,000** | **~1,550** |

The 7 missions that become errand routines leave the pool in phase 3, as their routines land. Every wave
runs `tools/compile_check.sh` and `tools/runtime_test/run.sh`. The runtime test must first start and end
every questline and every reused mission once (the finale's precondition), because this kit moves the
approach helpers all 15 questlines use.

## Engine needs

- **`RZ_MissionStory`:** an optional `m_Story` on `RZ_Mission` (null means the mission runs exactly as
  today). When it's set, `Update`, `OnPlayerShot`, `OnSuccess`/`OnFailed`, `OnRewardPlayer`, `GetBrief`
  and `Cleanup` forward to it. Each converted mission gets a story key that is never renamed once
  shipped.
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
- **Decider rules** (shared): only eligible participants can end a job; the 5 s hold, announced to the
  room, that anyone eligible can Object to once per job (by identity); Stand with; with the decider = the
  decider, their vanilla group within 150 m, and those who chose Stand with; hand-ins resolve at once;
  half the Favor for the work when the result went elsewhere; Leave per player; offline participants paid
  through the questlines' path; tallies through `RZ_QuestVote`.
- **`RZ_Beneficiaries`**, which replaces the core of `RZ_Mission_Choice`: 2–4 parties, each with a
  hand-in kind, a payer, rival Infamy, `m_bNoLeaderCredit`, where the base result goes, and at most one
  route asset; pay at most the base, with extras only to the chooser; "nobody" pays nobody; no "keep it
  yourself"; switch Fame left out of season Fame (a flag on `AddFame`); research only at the lab.
- **`RZ_Trade`:** take items by the price table through `RZ_Safehouses.TakeFood`'s keywords, plus
  background services as timed stand-bys. Mission goods are tagged and refused for pay (VERIFY: how to
  tag an item).
- **Disguise hooks** (owned by [DISGUISE.md](DISGUISE.md)): `PassAs`, `IsPassing`, `AddWatcher`; the
  own-colours rule; the mission's extra see-through list (a wanderer who remembers you, a life record
  about you, hunter squads); one mission trick per life per faction per season day.
- **The truth deck** in `RZ_WorldMemory`: a gated, weighted draw seeded by story key, subject and season;
  stored in world facts (`truth.<key>.<subject>`) with what's been found, kept 3 season days or until
  used up, dropped if the subject goes; `Reveal(gm, playerId, how)` tells the room and the journal.
- **The lie store** (shared): each record with its exposing condition; events first (a visit, road news,
  the owning system's tick), the rumour tick only for claims with no world test; a log line for each.
- **Records:** the quotas per tier, each story job's pinned reads, and the epilogue line and carrier filled in.
- **Reasons:** their posters in safehouse upkeep, supply lines, territory, leaders, wanderers, cure research, the
  Director, the tribes and the majors; the reason lane (one reason-started contract every 30 minutes server-wide,
  inside `MAX_CONTRACTS` and `CanRunNow`); the why-now line from the reason; jobs ordered per player.
- **One clamp** on contract weights in `RZ_MissionManager.CreateRandom`: leader traits × faction-life
  needs × the majors' season weights, clamped together to ×0.75–1.3.
- **`RZ_MissionReasons.c`:** one data row per mission type (the reasons it answers, the town tally it
  writes, a fallout line). This is all a work-tier mission needs.
- **Fallout errands** handed to the errand dealer at the subject: participants first for 20 minutes, at
  most 30% of the base. None from a mirror, a fallout or `Withdraw`.
- **Limits:** kit extras through the shared capped helpers in the contracts and errands lane of the
  majors' season budget (per faction per season day: morale ±10, strength ±4); loyalty changes of at most
  ±5 per job and −10 per leader per season day, never below 30, counted in the seat ledger; story groups
  at most 4 server-wide and 2 per job, each option with a written fallback; people in scenes plus askers,
  at most 16.
- **Mirror offers:** `AlsoOfferTo(faction, tier, rules)` for one class (The Cache, The Armory). A
  setting: 10% by default, off on PvE. The second side gets a hint 5 minutes later with a rough area twice
  as wide; the more-holders tiebreak; "Half each" pays 50/50.
- **Finale interface:** `RZ_Finale.RouteOf(identity)`; `Asset(route, kind, identity, key)`, at most one
  per outcome and 3 per player per route per season; options whose asset belongs to another route refuse
  a locked player when picked, with a line.
- **The season-day helper**, built once in the plan's phase 1.
- **Journal:** a per-job detail payload sent when its row is opened, at most 1,500 characters ("What they
  told you", "What you've found", "Who it went to"), so the 6,000-character list isn't cut; talk lines
  as person cards (`RZ_NoticeStyle`). Documented in UI.md and UI/LAYOUTS.md.
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
- `Mirror: <key> won by <side>`
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
