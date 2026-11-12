# Missions

Every mission ReforgerZ has or plans, with its requirements and status. How the systems
behind them work is in [DESIGN.md](DESIGN.md); standing, Fame and Infamy are in
[REPUTATION.md](REPUTATION.md); the prefabs each mission needs are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md), "The prefab manifest". The long questlines that reuse
these missions as stages are in [QUESTLINES.md](QUESTLINES.md). The small, personal jobs named
people ask in person are errands, not missions: [ERRANDS.md](ERRANDS.md). Every mission's place is
checked as it starts (dry, off the line, near the town its brief names; a warning in the log if
not): [PLACES.md](PLACES.md#the-live-check).

## Counts

| Kind | In code | Designed, not built |
|---|---|---|
| [Faction contracts](#faction-contracts) (20 designed per faction, plus three hard edges; merged, dropped, errand and stage-only numbers are left out) | 66 | — |
| [The Red Hand's hard edges](#the-red-hands-hard-edges) | 3 | — |
| [The packs' contracts](#the-packs-contracts) | 13 | — |
| [World events](#world-events) | 25, plus the packs' own four | — (3 more are covered by other systems) |
| [Puzzle missions](#puzzle-missions) | 15 | — |
| [Unarmed missions](#unarmed-missions) (Water Run and Medicine Before Dark are errands now; Rooftop Rescue is the stranger Up on the Roof) | 6 | — |
| [Perimeter missions](#perimeter-missions) | 3 | — |
| [Haven house contracts](#haven-house-contracts) | 3, plus the Courier's letters | — |
| [Leader missions](#leader-missions) | 27, plus Rein In | — |
| [Story arc missions](#story-arc-missions) | 3 | — |
| **Total** | **164** | **—** |
| [Errands](ERRANDS.md) (not missions: asked in person) | 34 routines, 2 wanderers' asks, 18 strangers, 70 cards (the margin cards of all four majors, a Champion card per faction and the lock card among them), plus the epilogue deck's 20 | — |
| [Questlines](#questlines) | 15 questlines and the 4 majors, about 90 stage missions plus the majors' 15 scenes (not counted above) | — (the majors' remaining approaches are scene choices: [QUESTLINES.md](QUESTLINES.md#the-majors)) |
| [The cure finale](#the-cure-finale) | 5 routes (20 scenes and 5 releases), 5 desks, 2 tissue hunts, Take the Lab's finale mode (not counted above) | — (simplified: [FINALE.md](FINALE.md#simplified-and-not-built-yet)) |
| [The epilogue week](#the-epilogue-week) | the closure set's 11 items, 8 closing stages, the dose outlets, the four grey choices, the epilogue deck's 20 cards ([ERRANDS.md](ERRANDS.md#the-epilogue-deck)) (not counted above) | — (simplified: [EPILOGUE.md](EPILOGUE.md#simplified-and-not-built-yet)) |

## Who gets offered what

**World events** are news: the radio tells everyone, they're never on the Quests tab and never
tracked, and anyone who takes part shares the reward. **Contracts** are offered, in person, only
to players who meet all of their requirements:

1. **Tier with the giver.** Known, Trusted or Champion. Big jobs need trust.
2. **Standing with others.** For example *not Trusted by the Red Hand*, or *at least Watched by
   the Haven*. Some open *because* someone hates you.
3. **Background.** Some contracts are locked to or from backgrounds (🔒), and some backgrounds
   get a better outcome (★).
4. **World state.** Outbreak stage, cure progress, whether Bastion is still around, night. Once
   the cure is out, Halcyon's contracts and events stop (its work is research), and so does any
   mission at the line; one running when the cure comes out is called off (no pay, no penalty). A
   mission can also wait on a season flag or stay away while a questline tells its story
   ([DESIGN.md](DESIGN.md#world-facts)).

How the code does it:

- **Jobs only by interaction.** Every job starts from a person you talk to or a thing you use,
  never from the radio, a timer, a spawn or walking past. The one exception is the openings'
  steps, and only their "Talk to ..." objective while someone's outside (the steps are handed when
  you talk to them; with nobody outside, a step starts by itself;
  [ERRANDS.md](ERRANDS.md#handed-errands-the-openings-chain)). How each kind starts:

  | Kind | Starts from |
  |---|---|
  | Faction contracts, the hard edges, the packs' contracts, puzzle and unarmed contracts, Perimeter and Haven house contracts, Rein In, the arcs' contracts, questline and majors' stage missions | Its faction, in person: a leader's or a quartermaster's first replies list every open contract of that faction, one reply each tagged [Job] or [Quest], those you can't take yet locked with what they need; picked, it's explained, and *I'll do it.* takes it ([DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies)). A safehouse radio (**Listen to the safehouse radio**) names the Haven's within 3 km, to take from the quartermaster there. Without the talk screen, a leader's **Ask for work** offers every one you qualify for at once |
  | A leader's own job (an Independent or Rogue leader's, or a questline's in-person stage such as The operator's tape) | That leader, at their seat: first among their job replies (or their quartermaster's while they're in the chair) |
  | The car keys job, the Courier's letters, Barter Chain | The wanderer you ask |
  | A parley's amends job | The parley itself (reserved for you for the hour) |
  | A puzzle or arc no faction gives (The Deserter's Map) | Its thing: reading its clue makes it yours |
  | A questline's later stages | Already yours once you did something at an earlier stage |
  | World events | Nothing: they're news, not jobs |
  | Errands, strangers, the cure finale's routes, the epilogue's closing work | The person you ask, or the desk you lock at ([ERRANDS.md](ERRANDS.md), [FINALE.md](FINALE.md), [EPILOGUE.md](EPILOGUE.md)) |

  **The radio only says who to ask.** An eligible player hears of an open contract they haven't
  been offered, once per contract and at most once a minute, as a Radio-tab entry under Missions
  that never makes a card (`RZ_ENoticeTag.HEARD`): *"Bastion wants people for Recover the Bus,
  east of Levie, a few km north-west. Ask Sergeant Hale at Fort Kell, or at any Bastion
  quartermaster."* The spawn's *Radio* entry lists the world events and the work going, each with
  who to ask. Walking up to a quartermaster with work you could be offered gets one line
  (*"There are 2 jobs going here. Talk to the quartermaster."*), every 5 minutes at most. A
  leader's own job is only heard of from the leader (*"A word, somewhere quieter..."*) or, for a questline's
  in-person stage, by word of mouth naming who to see.

  **A taken job is yours at once:** one conversation is enough, it's on your Quests tab (*Offered*
  until you get there, then taken) and its start card shows ([UI.md](UI.md#notices)); a questline's
  next stage sends the same card to everyone part of it. **Who's paid:** only players it was offered to, and anyone in their vanilla group, who
  meet the requirements when it ends. Anyone else who turns up hears *"Nobody asked you to. Bastion
  pays the people it hired."*
- **Built from a few blocks:** hold an area, kill what the mission spawned, visit points, destroy
  a prop, carry an item to a drop point, or choose which drop point gets it; escort people,
  capture a special alive, drive a vehicle into a zone, pay in health, or follow a chain of
  steps ([DESIGN.md](DESIGN.md#mission-building-blocks)). Puzzle-style missions add clues and
  answer actions.
- **Rough areas.** A contract names a circle, not a spot: "Somewhere around Morton: within 500 m
  of a point 1.2 km NE". The circle holds the real spot but is centred off it (30–70% of the
  radius). Within 500 m of the spot a player learns it, and is told if it was offered to them.
  **Radio Operators** and players **Trusted** (or more) with the giver get the exact spot from the
  start. The radio's who-to-ask lines, the safehouse radio's list, leaders, wanderers, the Haven
  neighbours perk and the notebook all use the same rule (**Contracts > Rough radius**, 0 = always
  exact); the notebook and the quest tracker say it in words ("East of Morton, a few
  km north-west"; within a kilometre the tracker points an arrow at the circle's centre with the
  metres to its edge: [UI.md](UI.md#the-quest-tracker)), and the map shows the tracked job's circle as a search area until the spot is known, then a diamond on it ([UI.md](UI.md#the-map)). Contracts that give their own positions keep them: puzzles (their clues and grids),
  points to visit (Install Sensors, Recon, Burn the Bodies, Water Survey, leaders'
  visits), the Memorial, the car keys job and Medical Drone (its brief already
  gives a ping, give or take 250 m). World events stay exact. *Simplified:* the extra lines some
  contracts add (drop points, grids) also wait until you're close; a contract's own progress
  messages may still give directions.
- **Arriving.** A step done by getting somewhere finishes within 15 m of its spot, or inside a
  town or building or within 25 m of it, in a vehicle too (the rule and its two settings:
  [arriving](DESIGN.md#mission-building-blocks)). It covers points to visit (Install Sensors, Water
  Survey, Recon, the curfew, Coup Talk), step chains (Patient Zero, Struck Off, The Three Weeks, Tied
  Down, The Book, Await Relief), vehicle zones, the puzzles' "get there" checks, Stash Note,
  Abandoned Convoy's cache, The Blind Spot's runner, the Census's watch point and patrol points, and
  the Long Table's meal. Working at a thing you can see keeps its short reach: a grave or a
  clue prop to stand at. Work with your hands (Burn the Bodies' piles, Blow the Bridge's charges,
  the sensors, the wells' samples, the convoy's engine, the hive's tissue, the Armory's well, Bust
  Out's chain, the Red Hand van's padlock) is a hold
  at a [work site](DESIGN.md#work-site) when work sites are on; off, it's the old stay there.
- **Where they happen.** A contract picks its town by who holds it ([DESIGN.md](DESIGN.md#world-facts)):
  work in a ruin (a nest, a cache, a town to retake, a pharmacy, a lost patrol) goes to a town
  nobody lives in; a faction's open work never goes to its rivals' holdings; a drop, a fence, a
  stockade or a fuel depot goes where its faction really is when it holds a town in reach. Bastion's
  contracts come as often as Bastion is still out on the roads (×1 early, ×0.37 in Aftermath).
  A contract that tells part of a questline's story waits while that questline runs
  ([QUESTLINES.md](QUESTLINES.md#contracts-that-wait-for-a-questline)).
- **No Vehicles** (a season rule set, [DESIGN.md](DESIGN.md#seasons)) takes out Recover the Bus,
  Car Theft, Mobile Lab, Tied Down, the car keys job and the Cargo questline.
- **Missions that need an optional prefab** never appear until it's set.
- **Needs and morale** make matching contracts more likely ([REPUTATION.md](REPUTATION.md),
  "Morale, needs, truces and feuds").
- **Supply trucks aren't contracts.** Escorting, driving in or stealing one pays directly
  ([DESIGN.md](DESIGN.md), "Supply trucks"). Convoy Escort and Hijack the Convoy ask for
  Bastion's next real truck; while a contract watches it, only the contract pays (the truck's
  escort, drive-in and steal Fame aren't paid on top; stealing it still costs Bastion Infamy).
  Convoy Escort pays nobody who shot near the truck while its crew lived, nor anyone in a shooter's
  group. If the post it's going to changes hands first, the truck turns back and both are called
  off (no pay, no penalty).
- **Rescues** add population at the nearest Haven safehouse ([DESIGN.md](DESIGN.md), "Haven safehouses").
- **Reasons start some contracts** ([DESIGN.md](DESIGN.md#story-blocks)): when the world posts a
  reason (a hungry house, a post's lost truck, a town gone bad, a dead leader, a bitten wanderer,
  the cure's milestones, a shelled town), one contract that answers it may start within 3 km of it,
  at most one every 30 minutes server-wide, inside the contract cap. Its brief opens with the
  reason's line (*"Levie's on half portions."*). Random picks stay.
- **Who decides, and who benefits** ([DESIGN.md](DESIGN.md#story-blocks)): in the choice missions
  (Crash Site, Research Data, Requisition) the one who hands the item in decides; their group within
  150 m is paid by the side it went to (if that side doesn't hunt them). Taken to the other side,
  it pays at most the base and its Fame doesn't count toward the season's; everyone else who did
  the work is paid half the Favor by the giver.
- **Mirror offers** ([DESIGN.md](DESIGN.md#story-blocks)): The Cache and The Armory are one class
  with two givers. One time in ten (**Missions > Story > Mirror chance**; off on a PvE server) the
  second side wants it too. Its eligible players hear 5 minutes later, with a rough area twice as
  wide and who to ask: it's theirs once their own side offers it (its job reply at a leader or quartermaster). The side with more holders at the end wins; a draw is half each, and neither leader counts
  it. The losing side gets a line, never a fallout errand.
- **Story jobs** ([DESIGN.md](DESIGN.md#story-blocks)): Protection Racket, Lost Colleague, Crash
  Site, Distress Call, Struck Off, Survivors Trapped, Missing Patrol, Missing Scout and Smuggler's Run,
  and the twenty on the story job's kit (The Cache, Medicine Run, Refugee Column, Trader Caravan, The
  Informant, Rescue the Pilot, Raid the Red Hand, The Armory, Dead Drop, Bust Out, Double Agent,
  Research Data, Cold Chain, Escort the Scientist, Wandering Trader, Prison Break, Collapse Shelter,
  Trail of Clues, Letters and The Crossing: each row's Status says how) have a hidden truth (kept per
  subject, the same for everyone), several ways through (one needs no gun, one no standing: a talk
  scene's person usually holds those two), parties the result can go to beyond the giver, a world
  record of how it ended, a callback and, failed, a fallout errand or line. Options that decide the job belong to someone it was offered to who took part:
  *"It's not your call. You weren't hired for this."*
- **The Tier 0 audit** (the 27 jobs that already had ways through: The Bitten Friend, The List, Walk
  Him Out, The Ledger, A Quiet Arrangement, Requisition, Secure the Shipment, No Reply, Learning the
  Drones, The Pit's Debtor, Tied Down, Serum Trial, Rival Survivors, The Rat, The Silent Village, The
  Witness, the leader jobs Shut the Door, The Real Orders, Local Law, Off-Book Requisition, Desertion
  Route, Honour Among Thieves, Debt Collection, Patients Not Samples, The Notebook, Private Collection,
  and Rein In):
  - a **fallout line** when they fail, after the fail text (*"Nobody came. He turned in the end, and
    the house says his name at supper."*);
  - a **world record** `t0.<key>` at their town when they end (done or failed, with who was there),
    which the next one offered in that town reads into its brief (*"The last time this came up here,
    nobody did."*); Walk Him Out also writes `walk.out` (his name: ended, walked on, turned), which the
    stranger Still Walking reads, and posts the reason `house.bitten`;
  - the **decider rule**: a contract's choices belong to someone it was offered to (*"It's not your
    call. You weren't hired for this."*). Tags on their choices are as built.
- **One-choice jobs and work rows**: see [below](#one-choice-jobs-and-work).
- **Out of the random pool, kept only as questline stages:** Fortify the Safehouse ([The Price of
  the Door](#the-price-of-the-door-the-haven-or-the-red-hand)), Curfew Patrol (The Census), Take the
  Lab ([Authors](#authors-halcyon); the finale's Red Hand route will own it), and the Red Hand
  leader jobs Our Crossroads, House Rules and Coup Talk (the Red Hand's major has its own scenes;
  House Rules is The Price of the Door's pit). Training Day stays a Bastion leader job.
- **Bodies on approach.** A contract's guards, AI people and raiders are spawned when a player
  comes within about 300 m of them; until then the job is data. A spawned AI group that is gone
  (groups delete themselves when the last member dies) counts as dead.
- **Drops are the mission's own.** An item is only taken at a drop point that mission spawned,
  never at another mission's drop of the same faction.
- **Lying low helps** ([DESIGN.md](DESIGN.md#stealth-in-missions)). With Stealth on, the distances
  in these tables at which someone sees you are for normal visibility and shrink for a player who
  is hard to see, and an infected close by is slower to notice them. With it off, everything is
  exactly as written.

### Objectives

Every job reports its **objectives** for the quest tracker and the notebook's Quests tab
([UI.md](UI.md#the-quest-tracker)), each **current**, **next** (known), **done**, **optional** or
**failed**. A step shows only once the player has learned of it: no placeholders, nothing hidden.

| Job | Where its objectives come from | How a step is revealed |
|---|---|---|
| A mission or contract (the default) | `RZ_Mission.GetObjectives`: until you're taking part, "Get to *place*." (or "Search around *town*." while you only know the rough area), then its ask as next; once you're there, the ask as current. The ask is the mission's `m_sObjective`, set beside its brief: one line naming who, what and where (*"Hold the cellar cache in Levie for a minute."*, *"Find Emil Novak's trial number at the lab in Morton."*), with the world's tokens (`{lab_town}`, `{leader:id}`) filled when shown. Every mission, contract, event, leader job, questline stage mission and finale desk sets one (the text lint checks it); one that didn't would show its title | Being counted on the job makes the ask current |
| A mission with steps of its own | `ObjSteps(...)` (and `ObjOptional`) in its setup, `ObjStepDone()` as each is done, `ObjReveal(n)` when the players learn of the steps ahead (a clue read, a person talked to): the done ones, the current one, the next only once revealed. *No mission declares steps yet;* they use the default or a step chain | `ObjStepDone`, `ObjReveal` |
| A step chain (Patient Zero, Struck Off's trail and the rest on `RZ_StepChain`) | Each step reached is "Went to *town*." (done); the current one "Go to *town*.", or "Find what was left in *town*." once you're there and its clue is unread. The steps after it never show | Reaching a step; its clue points onward |
| A questline, a major's act, a finale route | `RZ_Questline.GetObjectives`: the stages you took part in, done (by their titles, which name people: "{brother}'s number", never "His number"); the current stage's mission's own objectives, or for a wait stage "Wait for word: *stage*." The next stages never show. A major's scene and a finale route's stage set the scene's `m_sObjective` (a route's `StageObjective`; the release: "Set the release's terms with the duty officer in the clean room at the lab in *town*.") | Each new stage, as it comes |
| An errand | The step you agreed to, current (its `m_sStep`, with the giver and the place filled in), and the way back next (they told you both): "Go back to *giver* at the safehouse in *town*." for a safehouse's person, "Go back to *giver* at *place*." for a holding's, "Find *giver* on the road again." for a wanderer. Once the step is done, it's done and going back is current. An opening's last step ends where it's done | Agreeing to it |

Objective lines follow [WRITING.md](WRITING.md#objective-lines): imperative, one line, and no
pronoun without a named person. Briefs aren't rewritten: the Quests tab shows a job's own brief cut
to about three sentences at a sentence's end.

### One-choice jobs and work

**One-choice jobs** (16): Survivors Trapped, Radio Relay, Harvest Guard, Burn the Nest, Rescue the Kids,
Checkpoint Duty, Convoy Escort, Blow the Bridge, Zombie Bait, Lead Them Away, Fight Pit, Hijack the
Convoy, Kill the Officer, Steal the Antidotes, Lab Defence and Water Survey. When the work is done
(Fight Pit in round 3, Kill the Officer when someone it was offered to is within 40 m of him), a
stand-in person asks one question ([DESIGN.md](DESIGN.md#story-blocks)): its options are choice slots,
the decider rule and the 5-second hold apply, and nobody in 90 s (or no **civilian puzzle prefab**)
takes the default. Each writes one world record, answers reasons, and has a fallout line. The choice
is in each row's Status above. Never as a questline's stage (the stage's story decides), and never in
the questlines' raids that reuse Lab Defence or the arc that reuses Water Survey.

**Route assets.** Some options give the decider an ally for one of the finale's routes
([FINALE.md](FINALE.md#before-the-gate-pitches-and-route-assets)): Bastion for Radio Relay's shared net,
Rescue the Kids' aunt at the post, Lab Defence's stockade and Kill the Officer's stores; the Haven for
Harvest Guard's hungriest house, Water Survey's "tell the Haven too" and Steal the Antidotes' medic; the
Red Hand for Lab Defence's and Hijack the Convoy's "back to the crew"; Halcyon for Burn the Nest's sample
and Lab Defence's trial; the Independent for letting people go or giving to people rather than to a
faction (Fight Pit's going down, Hijack the Convoy's "let him walk", Convoy Escort's crates, Checkpoint
Duty's "let them through"). After a player locks to a route, an option whose asset is another route's
refuses them. The horde's drift gives none.

**Where the horde drifts** (Blow the Bridge, Zombie Bait, Lead Them Away): an empty town (default
for two of them: +5 infection in an empty town 0.4–3 km off), past Bastion's nearest post, or past the
Red Hand's nearest camp within 3 km (that side's morale −2, +4 Infamy with it to those with the
decider). A side with no holding near is an empty town instead. Every answer is grey.

### Givers who lie

✅ Twelve briefs may leave one thing out, the version picked once a season (one of them honest);
how it's found and taken public is [DESIGN.md](DESIGN.md#story-blocks)'s. Backgrounds find it faster:

| Job | What the brief may say (each a version) | Faster for |
|---|---|---|
| Learning the Drones | he volunteered; the drones only go for runners; he's done this before | Ex-Con, Raider |
| A Quiet Arrangement | it was his idea; it's once a month | Elder, Ex-Con |
| Requisition | the house signed for it; it's surplus | Soldier, Elder |
| Names for Bastion | only names of people who've left; Bastion only wants to count them | Elder, Soldier |
| Off-Book Requisition | they've plenty; the stronghold knows | Soldier, Lab Tech |
| Rifles for Sale | the buyer hunts; the stronghold signed off | Soldier, Mechanic |
| The Real Orders | the courier was ambushed; Command still answers | Soldier, Nurse, Doctor |
| Unsanctioned Trial | his family agreed; the serum's been through animal trials | Doctor, Nurse, Lab Tech |
| Private Collection | the director knows; it's for the cure | Lab Tech, Hunter |
| Sell the Formula | the director approved it; it's the whole formula | Lab Tech, Doctor |
| The Pit's Debtor | sixty tins, all his own; he agreed to the fight | Ex-Con, Raider |
| Debt Collection | they borrowed it; they can afford it | Ex-Con, Raider, Elder |

**Work rows** (68, `Missions/Story/RZ_WorkRows.c`): plain work keeps its play, and gets a row: the
reasons it answers, whether it writes the **town tally**, and a fallout line said when it fails.

| Group | Jobs, with the reasons they answer · ✎ writes the tally |
|---|---|
| The Haven | Recover the Bus (`town.infested`) ✎, Memorial (`house.lost_to`) ✎ |
| Bastion | Retake the Town (`town.infested`, `horde.passed`) ✎, Last Helicopter, Burn the Bodies (`town.shelled`) ✎, Recon (`town.infested`), Fire Mission (`town.infested`) ✎ |
| The Red Hand | Fence the Goods (`post.truck_lost`), Car Theft, Pharmacy Heist (`wanderer.bitten`) ✎, Arson (`town.shelled`), Sabotage the Relay, Collect the Bounty |
| Halcyon | The Lone One (`cure.*`), Field Samples (`cure.*`, `wanderer.bitten`), Power the Lab (`cure.*`), Install Sensors (`town.infested`) ✎, Blood Drive (`house.bitten`, `cure.*`), Specimen from the Nest (`town.infested`) ✎, Hunt the Screamer (`town.infested`) ✎, Medical Drone, Patient Zero (`cure.*`), Live Capture (`cure.*`), Mobile Lab (`house.bitten`) |
| World events | Abandoned Convoy, The Sniper, Stash Note, Mass Grave ✎, Locked Bunker, Tunnel Rats ✎, Military Remnant Camp, Wedding Party |
| Puzzles | The Numbers Station, Two Bearings, Patient Zero's Ledger, The Poisoned Well ✎, The Smuggler's Ledger, The Clockwork Horde (`town.infested`) ✎, Time-Lock Safe, The Bell Order, Lights in the Window, Junction Boxes ✎, The Weighing Vault, The Last Experiment |
| Unarmed and the perimeter | Distraction (`town.infested`), Quiet Count (`town.infested`), The Blind Spot, Shot at the Line |
| Leader jobs (in person: no reasons) | Our Own First, Fetch the Family, Over the Fence, Ration Run, Names for Bastion, Training Day, Hidden Stores, Hearts and Minds ✎, Rifles for Sale, Burn the Ledger, Skim the Toll, The Soft Touch, Sell Out a Rival Crew, Second Opinion, Unsanctioned Trial, Leak the Data, Sell the Formula |
| Story arcs | The Deserter's Map, Fever Town: the water, Fever Town: get them out ✎ |

**The town tally**: a work job marked ✎, done in a town, adds one to that town's count (the season
flags `tally.<town>.n` and `.by`) and leaves its giver's mark: Haven chalk on the church door, a
Bastion stencil, a painted red hand, Halcyon's numbered tape. The next work job offered there says so
in its brief (*"Haven chalk on the church door says it's been done here 3 times."*). Only the reason
lane starts a job from a reason; world events and leader jobs answer none in practice (events aren't
contracts, leader jobs are in person). *Simplified:* the tally is words in a brief, not a decal.

**Reading the tables:**
- **Needs:** tier with the giver first, then other requirements. *Any* = Stranger or better and not Hunted.
- **Bg:** 🔒 background lock, ★ background bonus.
- **Result:** what you gain, and in brackets what it costs with other factions. Every contract also pays Favor.
- **Status:** ✅ in code (with any simplification in *italics*), *planned*, or *covered* by another system.

Backgrounds: Nurse, Mechanic, Hunter, Police, Ex-Con, Lab Tech, Radio Op, Unemployed; unlockable
Soldier, Doctor, Raider, Elder, Drifter.

---

## Faction contracts

### The Haven

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | The Cache | Any | | Hold a cache in a town for 60 s | Haven Fame; a reward crate | ✅ *Raid the Cache merged in: a mirror offer ([who gets offered what](#who-gets-offered-what)). One time in ten the Red Hand wants it too, for players the Haven Watches (Red Hand Known); whoever has more holders there when the hold ends gets it, a draw is half each. **Story job** (key `cache`, per town): a cache as they said; a family's larder (a house within 4 km; ★ Elder reads the jars); a crew's stash (a Red Hand holding within 5 km; ★ Ex-Con/Raider reads the thumbprint). The scavenger on the crates: [Known · Haven] talk him off or 2 tins, and the hold is done. Then: the Haven; leave it for the family (nobody pays, the house +3 food, a callback at the door, an Independent asset); leave the crew's stash alone (8 Red Hand Favor, a Red Hand asset), emptied anyway +4 Red Hand Infamy. Record `cache.went_to`* |
| 2 | Survivors Trapped | Any | ★ Nurse: healthier survivors, bonus Fame; ★ Nurse or Doctor sees a bite | Reach a barricaded family and survive 3 waves. Then the mother asks where they're going (a 5 s hold; nobody in 90 s: the nearest house) | Fame, a reward crate. **Choice:** the nearest house (+3, +4 with ★); the hungriest house (the same, Haven morale +1); or Bastion's post (Bastion pays the base, the Haven nothing). One time in four the father is bitten: the house checks him at the door and walks him out (one fewer) | ✅ *story key `trapped`; record `trapped.family` with an epilogue line. The family is real AI only with the survivor group prefab; all dead: fallout The Name on the Tag. A Nurse who saw the bite can dress it and say nothing (a fourth slot on the mother): the house doesn't see it at the door, and finds it within a season day (a lie, +8 Haven Infamy to her, the life record "hid a bite"). To Bastion's post: civilians sheltered goes up on that night's call (the season count `bastion.sheltered`, read to those with the job). Answers `horde.passed`* |
| 3 | Radio Relay | Any | ★ Radio Op: half the hold time | Keep a loud relay on high ground running for 3 min | Fame; a reward crate | ✅ *a horde comes after the first minute* · **One choice** (`relay.net`): the Haven's net alone (default), or Bastion's too for a patrol past the house (Haven morale +1, 5 Bastion Favor to those with the decider) |
| 4 | Medicine Run | Any | ★ Nurse/Doctor: the pharmacy is pre-marked, bonus Fame | Bring medical supplies from a pharmacy town to a Haven safehouse. Fight the infected in the aisles, or **[Sneak]** make noise well away from the door to draw them out (Pharmacy Lure merged in) | Fame. Noticed taking it on the quiet way: they come for you | ✅ *the supplies lie loose in the town's pharmacy (the building nearest a point in the middle if it has none), put out once someone is within 60 m. The quiet way pays the same, with no extras. **Story job** (key `medicine`, per town): stocked as they said; a crew took half (a Red Hand holding within 5 km); the chemist lives upstairs. The woman in the doorway: [Known · Haven] or 3 tins and she brings the box down (the quiet way); split it with the chemist (60%, a callback note, an Independent asset); sell it to the crew's runner (12 Red Hand Favor, +5 Haven Infamy, the Haven pays nothing). Record `medicine.went_to`* |
| 5 | The Bitten Friend | Known | ★ Nurse: slows his infection | A Haven member is bitten. Bring an antidote before he turns, or end it | Fame; mercy = half the Fame. Killed by anything else: failed | ✅ *mercy counts only if a player fired within 30 m of him in his last 10 s; there's no one-person slot, so the survivor group stands in for him (all of it has to die)* |
| 8 | Harvest Guard | Known | ★ Hunter: farmers stay hidden longer | Protect farmers in the fields for 10 min | Fame; +3 food at the nearest safehouse | ✅ *farmers are real only with the survivor group prefab; ★ delays the first wave by 2 min* · **One choice** (`harvest.went`): the farm's own house (default) or the hungriest house |
| 9 | Missing Scout | Known | | Find a scout's body and bring his notes home. The truth (per name): he died on the road, as the house says; or his notes show he'd been selling the house's water runs to a crew (a Red Hand holding within 5 km). The clerk reads them at the house and asks | Fame; the notes mark a stash (a reward crate in a random town) for whoever brought them. **Died:** keep the stash for the house, or read it out on the open channel (nobody pays, no crate; an Independent route asset). **Selling:** tell the council (the base; +3 Red Hand Infamy to those with the decider), burn the page (60%, and it isn't clean hands), or sell the page back to the crew (10 Red Hand Favor, nothing from the Haven, a lie a Haven-Known visitor to the house finds out, +5 Infamy; a Red Hand route asset) | ✅ *story key `scout`; record `scout.fate`. Home is a Haven safehouse (the safest town when there's none); the clerk's question is asked there* |
| 11 | Burn the Nest | Trusted | | Destroy a hive in an infested town | Fame, infection −30, a sample. Fails: infection +15 there | ✅ · **One choice** (`nest.fate`): burn it all (default), or leave a piece for Halcyon (two samples, Haven morale −1; a Halcyon route asset) |
| 12 | Refugee Column | Trusted | | Escort a group walking a road to a safe town | Big Fame; three join the safehouse | ✅ *to a safehouse 0.8–3.5 km away, in a straight line through a midpoint. **Story job** (key `column`, per town): a family as they said; one bitten (★ Nurse/Doctor sees the sleeve); one who owes a crew within 6 km. The grandmother: [Known · Haven] a nearer house (when there is one), or 2 tins for a carter (they ride: there in 2 minutes). At the door: all in (a signed thank-you and a callback at the door), or Bastion's post (sheltered, 5 Bastion Favor, the Haven pays nothing); bitten: tell them (two join) or say nothing (three; a lie a visit finds out, +8 Infamy, 60%); owed: hand him to the crew (two join, 10 Red Hand Favor, +4 Haven Infamy, half pay). Record `column.fate`. A pack that hunts their road at its hour (a road habit of 50+ within 300 m of the way): they ask for the walk past it, and it pays ×1.15* |
| 16 | Trader Caravan | Trusted, not Trusted by the Red Hand | | Defend a Haven trader at a crossroads from a Red Hand raid | Fame, a reward crate (Red Hand Infamy) | ✅ *the raid comes 90 s after someone reaches him. **Story job** (key `caravan`, per town): a raid as they said; a debt he owes a crew within 6 km; Bastion's stores under new paint (★ Soldier, ★ Lab Tech). His boy: [Known · Haven] pack up and go before they come (half pay, no crate), or pay the debt (4 tins or 20 Red Hand Favor: the collectors take it and go). Then: tell the Haven; keep his secret (nobody pays, a callback); send Bastion's goods back (8 Bastion Favor). Record `caravan.fate`* |
| 17 | Rescue the Kids | Trusted | ★ Hunter: quieter approach | Hold a school for 90 s without making noise. Too loud and a horde comes | Big Fame; the kids join a safehouse (+3) | ✅ · **One choice** (`kids.went`): the nearest house (default), or their aunt at Bastion's post (Bastion pays the base, the Haven nothing) |
| 18 | The Informant | Trusted, Watched+ by the Red Hand | 🔒 not Police/Soldier | Pose as a buyer at a Red Hand dead drop and report who shows up | Fame; the Red Hand loses a little strength (+10 Red Hand Infamy for the buyer the moment he's spotted, even if the names never arrive) | ✅ *wait 60 s for the seller, buy, then reach any safehouse; spotted 20%, +30% carrying a gun. **Story job** (key `informant`, per town): the names as they said; bait (wrong names; ★ Ex-Con/Raider sees the price); he sells to Bastion too (★ Radio Op hears his handset). The seller is a talk scene: buy (as before), pay in tins (never spotted), or [Red Hand kit] "Mira sent me" (a trick: never spotted). At the house: the names to the Haven (a callback); bait: burn the list (half pay); both: sell Bastion a copy (10 Bastion Favor, a lie a visit finds out). Record `informant.names`* |
| 19 | Recover the Bus | Champion | ★ Mechanic: tunes it to half a tank | Get a bus or truck running and drive it to a safehouse | Fame | ✅ *it starts with a splash of fuel; any bus, truck or van counts, also after the depot's bus is wrecked; the Haven keeps it (it leaves the world)* |
| 20 | Memorial | Champion | | Collect ID tags of the dead from an overrun city | Fame | ✅ *five bodies, 8 s each, then any safehouse; the bodies are the corpse pile prop (else the puzzle object), and it never appears without either* |
| 21 | The List | Known, a safehouse | ★ Police/Elder: more Fame; the list says when the line was drawn | A name is crossed off a safehouse's list. Read the list and talk to a council member, and they point somewhere: a cairn at the edge of town (walked out after a bite), a bedroll on the road out (put out by the council), a packed bag at a landmark (left on their own), or the person, alive in another town. Then **tell the family**, **tell the council** or **keep it quiet** | Family: Fame (+3 Haven Infamy if the council put them out; alive: the family goes after them, the safehouse is one fewer). Council: 75%. Quiet: 40% | ✅ *one of the four truths at random; the living one is a Haven wanderer when one is free (they remember a council told, or a secret kept), else a generated person. Only a player the job is open to can choose. Done, it can open the questline [The List](#the-list-the-haven) (who crosses names out, and why). Code: `Missions/Haven/RZ_HavenLoreMissions.c`* |

**Amends:** Medicine Run and Survivors Trapped work off Haven Infamy while you're Watched.

#6 Fuel for the Lights and #14 Water Purifier are the errands Lights and Filter Parts now
([ERRANDS.md](ERRANDS.md#the-routines)).

### Task Force Bastion

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Retake the Town | Any, Bastion present; else the Haven's, Known | ★ Police/Soldier: +25% Fame | Kill 8 × size infected in a town | Fame, infection −35. Fails: *"The town stays theirs."* | ✅ *Take Back the Town merged in: once Bastion has pulled out (Overrun+) the Haven gives it, as Take Back the Town* |
| 2 | Last Helicopter | Any, Bastion present, no storm | | Bastion's one helicopter lands at an LZ on its run between its own bases. Keep the LZ clear and be there when it lifts off | Fame, Favor | ✅ *no runs while the helicopter is grounded or lost, or while [The Last Pilot](#the-last-pilot-task-force-bastion) has it; the first run after a grounding says who flies it now. Finishing it can start that questline* |
| 3 | Burn the Bodies | Any (Bastion); Trusted (Halcyon, at the lab) | ★ Nurse: faster | Burn 5 corpse piles in a town: hold **Burn the pile** at each (10 s; ash stays) | Fame, infection −10 there | ✅ *the piles are the corpse pile prop (else the puzzle object); it never appears without either. Decontamination merged in: one time in three while the lab stands it's Halcyon's, four piles round the lab, the lab's town −15* |
| 4 | Recon | Any | ★ Hunter/Radio Op: double Fame | Visit 3 observation points on high ground | Fame; Bastion shares where the next horde is forming | ✅ *with the packs on: the hungriest pack, in Bastion's word with its place, and it goes on the contact board* |
| 5 | Checkpoint Duty | Known | ★ Police: holds faster | Hold a checkpoint at an infested town's edge for 10 min against waves | Fame (Red Hand Infamy); the town stops spreading for 30 min | ✅ *Quarantine Line merged in* · **One choice** (`barrier.barrier`): turn them back (default), or let them through (+2 at the nearest house, the town stops spreading for 15 min only, no leader credit) |
| 6 | Missing Patrol | Known | ★ Soldier reads the unit codes; ★ Ex-Con or Raider the crew's work; ★ Hunter the tracks | Find a squad's last position and deal with the turned soldiers. The truth (per patrol): they turned, as the post says; a crew took their rifles first (a Red Hand holding within 5 km; a toll chit in the sergeant's pocket); or they were sent to the line (the log's last order, "weather permitting"). When it's done, a private from the post asks | Fame, a crate of military loot. Their tags to the nail (the base; a Bastion route asset), or bury them there, tags and all (nobody pays; an Independent asset). **Ambushed:** tell the post a crew did it (Bastion morale −1, +4 Red Hand Infamy to those with the decider), or sell the crew a quiet word (10 Red Hand Favor, Bastion pays nothing, a lie road news finds out; a Red Hand asset). **Orders:** read the last order on the open channel (nobody pays, +5 Bastion Infamy, Bastion morale −2; an Independent asset) | ✅ *story key `patrol`; record `patrol.fate` with an epilogue line. With a military pack within 3 km, the four soldiers are its young, in its look, and the brief puts them with it in Bastion's word. Answers `post.pressure`* |
| 7 | Crash Site | Known | ★ Mechanic: black box twice as fast, and reads it | Secure a helicopter wreck for 60 s and take its black box. The truth (per wreck): weather, fell off the inventory (the cockpit's manifest), or already taken (two Red Hand carriers have it at their fire). The Last Pilot's "He flew for the line" starts it at his wreck | **Choice:** Bastion Fame (inventory: the box goes in the stove), or sell it to the Red Hand (inventory: the manifest on the open channel, Bastion morale −3) | ✅ *story key `crash`; record `crash.box`. Failed with the box not handed in: fallout Buy It Back. Time up with the box still out there: the next Crash Site at that wreck finds it already taken (with the Red Hand group prefab). **More ways:** the sneak lift (hidden within 8 m of the wreck in the 90 s after someone first comes within 150 m: the box comes out at once); Bastion's recovery team at minute 15 (a real squad walking up from the road, as a story group: **Contracts > Bastion squad**, else the hunter squad or a truck crew; with none, or no room, words only. Trusted with Bastion, they leave it to you and the box comes out; only Known, they take it and the job's paid at half; Watched or worse within 150 m, +5 Bastion Infamy); at the carriers' fire (Already taken): [Known · Red Hand] persuade (difficulty 1), pay 25 Red Hand Favor or 5 food, or [Bastion kit] "That's ours, lads" (the disguise look at difficulty 1, the cockpit's flight order as the cover detail if you read it; caught, they walk off to the fence and the job fails); Inventory to Bastion (the stove): the commander's loyalty +5 and a Bastion route asset, and a silence the rumour round can find (+10 Bastion Infamy), paid 80% once a stronghold's page is out (`majors.pages_open`); Bastion pays 75% for the box after the muster ended Expended (`orders.stance` 4); Inventory to the fence: his loyalty −5, a Red Hand route asset; Weather to Bastion (the nail): morale +2 and the crew's names read on that night's call (a callback); Inventory carried to the radio station and held 10 s (the open channel): nobody pays, Bastion morale −3, +10 Bastion Infamy, the commander remembers, an Independent route asset. After the lock, a locked player can't hand an Inventory box to another route's side* |
| 10 | The Armory | Trusted | ★ Police: knows the layout (the exact spot) | Retrieve weapons from an armory in an overrun base | Fame; whoever gets the case gets stronger | ✅ *a barracks or police station in an overrun town if the map has one (else the building nearest a point in the middle); the case lies loose inside, put out once someone is within 60 m; carry it to a Bastion drop. Loot the Armory merged in: a mirror offer, one time in ten the Red Hand (Known) wants it too and its drop goes up when it hears; whoever hands the case in decides (a choice). **Nobody**: whoever carries the case holds **Drop the rifles down the well** by the town's well (8 s; with the well prop) and it goes down: +5 Infamy with both, nobody pays, a world record. **Story job** (its key `weaponscache`, per town): rifles as they said; emptied (one case left, 70%; ★ Police sees the caretaker lie); sold by the post's own sergeant to a crew within 6 km (★ Soldier reads the rack log, ★ Ex-Con/Raider the chalk). The caretaker: [Known · Bastion] or a tin and he shows you the case; sold: name the sergeant to the post (Bastion morale −1, the nearest Bastion leader's loyalty −3, +3 Red Hand Infamy, a callback on the nightly call). Record `weaponscache.sold`* |
| 11 | Convoy Escort | Trusted | ★ Mechanic: fixes a broken-down truck in a third of the time | Escort supply trucks between two towns | Big Fame (Red Hand Infamy if raiders came) | ✅ *Bastion's own supply truck from its stronghold to an outpost; it breaks down once halfway: hold **Fix the engine** at the bonnet (15 s; a Mechanic 5 s; a toolbox and parts there); raiders only with the Red Hand group prefab; paid for staying with it 40% of the way. Needs supply trucks* · **One choice** (`convoy.crates`): when it's in, the driver wants two crates "lost to raiders": let him (+2 food at the hungriest house; a lie, found by rumour, +8 Bastion Infamy) or don't (default) |
| 12 | Rescue the Pilot | Trusted, Bastion present | ★ Nurse/Doctor: patches him up, no clock | The last helicopter put down hard. Find its pilot and bring him out | Fame | ✅ *he follows you to a Bastion seat; 20 min before he bleeds out. Not while the helicopter is grounded or lost, or while [The Last Pilot](#the-last-pilot-task-force-bastion) runs. If he dies, the helicopter is grounded for 2 real hours until a new pilot is found, and if The Last Pilot hasn't run yet, it never will: he was its pilot. **Story job** (key `pilot`, per helicopter): a crash as they said; he put her down on purpose (★ Mechanic, ★ Soldier read it); a crew's cargo (a Red Hand holding within 6 km). The farmer whose cabbages he landed in: [Known · Bastion] or 2 tins for the cart (he rides in, no bleeding, in 2.5 minutes). At the post: the engine; ditched: report it (Bastion morale −1) or say it was the engine (a lie the rumour round can find, 60%, an Independent asset); cargo: tell the crew he's home (10 Red Hand Favor, Bastion pays nothing); a crash: ask the post to pay the farmer (half pay, a callback at his gate). Record `pilot.fate`* |
| 15 | Raid the Red Hand | Trusted, not Trusted by the Red Hand | 🔒 not Ex-Con/Raider unless Champion with Bastion | Assault a Red Hand camp | Big Fame, a reward crate (big Red Hand Infamy) | ✅ *two Red Hand groups; every member has to die. **Story job** (key `raid`, per town): a camp as they said; people held there (a house within 5 km); Bastion's deserters in red paint. The lookout: [Threaten] tell them to clear out (Fear 2: gone without a shot, no crate), or 3 tins and one group fewer. Done: burn the camp, or leave its stores to scavengers (no crate); held: Bastion's truck (sheltered) or walk them to the house (10 Haven Favor, the house +2, a callback); deserters: their tags to the post (Bastion morale −1) or burn the paint and say nothing (Bastion pays nothing, an Independent asset). Record `raid.fate`* |
| 17 | Requisition | Trusted | 🔒 not Elder | "Collect" supplies from a Haven safehouse. Deliver them to Bastion, or give them back | To Bastion: Fame (Haven Infamy), the safehouse loses 4 food. Back: Bastion Infamy | ✅ *giving them back means carrying them to another safehouse 0.8–5 km away (a quiet town when there's none). The town: a house that refused [The Census](#the-census-task-force-bastion) first (and ×1.2 as often after a refusal), else the safest free town whose nearest safehouse isn't spared (the chair's after [Await Relief](#await-relief-task-force-bastion)'s Haven ending; houses The Census read low)* |
| 18 | Secure the Shipment | Champion | | Escort a Halcyon sample shipment | **Choice:** Bastion Fame (Halcyon Infamy) or let it reach the lab (Halcyon Fame, research) | ✅ *a courier on foot; divert her by talking to her* |
| 19 | Blow the Bridge | Champion | ★ Mechanic/Soldier: two charges, not three | Plant charges to stop a horde heading for a safe town | Fame; the horde turns back. Killed to the last instead: paid all the same. It arrives: +10 infection there | ✅ *with the packs on, first a pack with a real bridge (the land grid's, not a ford) between its lair and a safe town 0.4–3 km beyond it: the horde is its hunters and young from its lair, the charges go on the bridge, and the pack is held while it runs. Otherwise no real bridge: a choke point on its way. The horde is data until a player is within 600 m; each charge spot is a puzzle object with a hold beside it, **Wire the charge** (10 s; off: stand by it 10 s), listed by grid; the clock covers the horde's whole walk* · **One choice** (`bridge.horde`): where the turned horde drifts, [the shared block](#one-choice-jobs-and-work) |
| 20 | Fire Mission | Champion | | Mark three hordes with smoke for artillery | Fame | ✅ *10 s within 80 m marks one; 30 s later the strike kills it outright. No target within 300 m of a safehouse, a holding or the lab* |
| 21 | No Reply | Known | ★ Radio Op: takes the second bearing on their own set, anywhere 400 m+ from the post; 1.25× Fame | A post's radio operator heard an answer on the Command net after the nightly call, and it comes back every hour. He gives a bearing from the post; take a second at a relay mast, draw both lines, find the source where they cross, then tell him what goes in the log | **Choice:** *"No reply"*: paid. What you found: 1.25× Fame, Bastion morale −4. *"Command answered"*: morale +6, 60% pay; 35% the stronghold asks for the transcript: no pay, +10 Bastion Infamy for whoever chose it | ✅ *the post is an outpost commander's seat (else a territory outpost, else the stronghold); never runs without one. The source is one of three, at random: a tape of Day 2 traffic on a timer, a Bastion set someone inside the Zone was keying (gone when you arrive), or nothing at all. It is never Command and never anyone outside; who did it is never said. The operator is the puzzle civilian, the mast the radio mast, the source the puzzle object (the table radio from guidance stands by the operator if set). The tape at an Independent commander's post leads into the [Await Relief](#await-relief-task-force-bastion) questline when it can start: the reel also carries a broken burst after the voice, and the operator says so as the log is written. Never at the post of a running Await Relief* |
| 22 | Struck Off | Trusted | ★ Soldier: +1 to talk him back | A soldier walked off post "with a week's rations" and is on the board as killed in action. His pack by the post, a Haven trader in a nearby town, or the Deserter wanderer (asked about Bastion) leads to where he sleeps. The truth (per name): he stole and ran, written off, the quartermaster's gap, or a Haven door (he sleeps by a house). Talking to him tells it to the room; the post's stores book, read after dark, shows the last two | **Choice:** talk him back (Trust with Bastion 2): the KIA is rubbed out (walk him to the post): 80%, Bastion morale +3, strength +1. Arrest him: Fame. The Haven (walk him to a safehouse, +1 there): the Haven pays instead (12 Fame, 15 Favor), and it's a lie to Bastion that a Bastion-Known visitor to that house finds out (+15 Infamy). Bastion's way: his tag on the empty nail: 60%. Let him go: no pay. The stores book reported from the board: +5 Bastion Fame, the chooser only | ✅ *story key `struck` (Hunt the Deserter merged in); record `struck.fate`. He is an escort (Contracts > Escort soldier); dead any other way, his tag drops where he fell. Time up: "struck off twice", his tag on the nail. Needs a safehouse and a Bastion post. From broadcast Day 13; his KIA is dated from Day 11 to two days ago. Offered a little more often (×1.2 each, inside the weight clamp) after a post's count was cooked (The Count's record) and after Bastion's orders were read on the air (`orders.read`, which the majors will set). **More ways:** at a Haven door, [Feared] press him in front of the kids (+3 Haven Infamy; he comes under arrest) or leave the kids four tins (+2 food at the house; he goes back to the roster). The stores book reported, when it shows the book was cooked: the post's commander remembers you crossed him and his loyalty −5. ★ Hunter within 600 m of the post reads his tracks to where he sleeps; ★ Soldier within 200 m of it, he shows himself. Let go: he takes to the roads as the Deserter if a place is free. The last roll in Bastion's coda reads the season's count: the struck-off men walked home answer "Here", and for those still out nobody does* |

**Amends:** Burn the Bodies and Recon work off Bastion Infamy while you're Watched. So do
surrendering at a Bastion checkpoint and paying Favor ([REPUTATION.md](REPUTATION.md#working-off-infamy)).

### The Red Hand

The Red Hand doesn't care who you were, but it does care who you've crossed.

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Dead Drop | Any | ★ Ex-Con: 25% better pay | Sell infected samples for loot and Favor | Fame (Halcyon Infamy; the cure slows) | ✅ ***Story job** (key `deaddrop`, per town): sold on as they said; cut into fake doses for the houses; bought back off the books by Halcyon's own (a Halcyon station on the map; ★ Lab Tech reads the box). The buyer: [Known · Red Hand] be paid in person (the Ex-Con's rate); cut: warn the houses (it ends: +5 Red Hand Infamy, Haven morale +1, a callback on the house radio, a Haven asset); Halcyon: take them to the lab instead (10 Halcyon Favor, a Halcyon asset). Record `deaddrop.went_to`* |
| 2 | Fence the Goods | Any | ★ Ex-Con: +50% Fame, extra Favor | Carry stolen goods to a fence in another town | Fame, mostly Favor | ✅ |
| 3 | Car Theft | Any | ★ Mechanic: +25% Fame | Deliver a specific vehicle to the chop shop | Fame | ✅ *a car from Cars > Car prefabs parked in a town, driven to a Red Hand drop and left there (it's stripped: it leaves the world); spawned cars aren't locked, so no hotwiring* |
| 4 | Pharmacy Heist | Any | | Hold a pharmacy for 90 s before the Haven gets there | Fame (Haven Infamy) | ✅ *a real pharmacy or clinic found by prefab name; a town with none: the middle of town, and the brief names no building* |
| 5 | Fight Pit | Known | | Survive 4 minutes of waves in a ring while the Red Hand bets | Fame; the bets pay Favor | ✅ *the ring is the season's one pit, the canon spot behind the boss's seat ([DESIGN.md](DESIGN.md#world-facts)), held while it runs. The bets are a fixed, larger Favor reward. After [Stock the Pit](#the-packs-contracts), the next one's last round lets out the two caged young ("Fresh hires, from the Postmen!")* · **One choice** (`pit.debtor`), in round 3: a debtor asks you to go down (half pay, the job ends, he walks free of the book) or fight it out (default) |
| 7 | Smuggler's Run | Known | ★ Hunter: fewer location reports | Carry a package across the map. Everyone is told a courier is out. The truth (per town): cigarettes and tins, as they said; a Haven house's antidotes (a house within 4 km); or pages off Bastion's roster. Whoever picks it up reads the tape, and the room hears. The crew's runner at the drop asks for it | Big Favor. Hand it over; or take it back where it came from: the doses to their house (10 Haven Favor to those with the decider; a Haven route asset) or the pages to Bastion's post (10 Bastion Favor; a Bastion asset), +5 Red Hand Infamy and the crew pays nothing | ✅ *story key `smuggle`; record `smuggle.package`* |
| 10 | Protection Racket | Trusted | 🔒 not Elder; ★ Nurse or Police +1 to persuade, ★ Ex-Con or Raider to press | "Collect" from a Haven safehouse. The truth (per house): just late, paid twice (another crew's receipt in the stores), or the storekeeper's cut (her daughter in the doorway). At the storekeeper: Ask, Persuade, Press, Cover it yourself, the receipt, her cut, Side with the house | Persuade: 2 food, 75%. Press (Fear 2): 3 food, +5 Haven Infamy (+8 short; refused: her people come out, or the door shuts). Cover it (4 food or 20 Red Hand Favor): the house loses nothing, 50%, a lie the next collection there finds out (+5 Red Hand Infamy). The receipt: full pay, the house pays nothing, Red Hand morale −2. Her cut: keep her secret (1 food) or tell the council (3 food, she's put out). Side with the house (5 s hold): +10 Red Hand Infamy, nobody pays | ✅ *story key `racket`; record `racket.<house>`. Not while [The Well Story](#the-well-story-the-red-hand) runs. Time up: the crew collects anyway (−3 food) and the house's cook has The Pot as fallout. Never a house in a town where A Quiet Arrangement was seen through (it pays by arrangement); paid twice only with another Red Hand leader within 6 km. A Short persuade writes `racket.speak_for`: the next collection at that house says so in its brief, and the one who promised persuades one step easier. Collected: a callback note at the house some hours of the collector's play later. **Off the book** (10%, a Rogue Red Hand boss within 6 km): the boss's runner at the fence (*"Keep it off the book."*), ★ Ex-Con or ★ Radio Op on Ask; found, *Collect for the stronghold*: the full collection goes to the stronghold's drop (paid the base, no leader credit) and the Rogue boss remembers you crossed him. Telling the council, or siding with the house, the house's council head remembers you helped (and the boss that you crossed him). Time up: the crews' collections weigh ×1.1 for the season* |
| 11 | Hijack the Convoy | Trusted, not Trusted by Bastion | | Ambush a Bastion convoy | Big Fame (big Bastion Infamy) | ✅ *Bastion's own supply truck to an outpost: robbed, burnt or driven off counts, for anyone within 300 m when it happens. Needs supply trucks* · **One choice** (`hijack.driver`): let the driver walk (default), hand him to the crew (+5 Bastion Infamy) or leave him tied for Bastion |
| 12 | Zombie Bait | Trusted | ★ Radio Op: twice the noise | Make enough noise right outside a Bastion outpost that a horde comes for it | Fame (Bastion Infamy) | ✅ *a random Bastion outpost (never the stronghold); with territory off or no outposts, the edge of a town Bastion passes through. Within 2 km of a pack's ground, the horde is that pack's hunters and young at night, its walkers by day; otherwise the town's own* · **One choice** (`bait.horde`): where the horde drifts, [the shared block](#one-choice-jobs-and-work); Bastion's post is the default and the pay, an empty town pays half, a Red Hand camp nothing |
| 13 | Bust Out | Trusted | ★ Ex-Con: cuts the lock in 8 s, not 20 (with work sites, the hold **Cut the chain** at the gate, for whoever chose Cut the lock; the guard's second look comes 15 s into the server's count) | Free Red Hand prisoners from a Bastion stockade | Fame (Bastion Infamy) | ✅ *a gate 45–70 m from a Bastion outpost's seat (never the stronghold); its guards (Contracts > Bastion squad) are the mission's watchers. In Bastion's kit, Cut the lock is a claim ([borrowed kit](REPUTATION.md#borrowed-kit)): "Who sent you?", difficulty 2, the commander's name as the cover detail if you've met him; a 20 s cut gets a second look at 15 s. Fail either: the alarm, a fight. Without the kit, the guards see you. The prisoners walk with you to a Red Hand drop. **Story job** (key `bustout`, per town): crew as they said; a Haven house's boy among them (a house within 5 km); one who talked to get his cell. The prisoner at the wire: 4 tins for the night key (the gate opens without a cut). At the drop: all to the crew; the boy home (8 Haven Favor, the house +1, 80%, a callback at the door, a Haven asset); the one who talked named (10 Red Hand Favor) or let slip away (80%, an Independent asset); crew: walk them past the drop (the crew pays nothing). Record `bustout.fate`* |
| 14 | Arson | Trusted | | Burn a Bastion fuel depot | Fame (Bastion Infamy) | ✅ |
| 15 | Sabotage the Relay | Trusted, not Trusted by the Haven | | Destroy a Haven radio relay | Fame (Haven Infamy) | ✅ |
| 16 | Steal the Antidotes | Trusted, the lab has doses in stock | ★ Lab Tech: twice as fast | Hold the lab's store room for 90 s while the crew loads up | Big Fame, Favor; up to two of the lab's doses move to the Red Hand's black market (Halcyon Infamy) | ✅ · **One choice** (`antidotes.*`): the black market (default) or a Haven medic (no pay at all; Haven morale +1; a Haven route asset) |
| 17 | Double Agent | Trusted, Known by Bastion | ★ Police: Bastion believes you (caught 15%, not 35%) | Feed fake intel to Bastion | Fame with both (Bastion +5); caught = +20 Bastion Infamy | ✅ *a dossier from a Red Hand seat to a Bastion drop; believed, Bastion loses a little strength. Handed in wearing Red Hand colours, Bastion sees through it at once. **Story job** (key `double`, per town): fake as they said; real (grid references round a Haven house within 5 km: believed, Bastion raids it, the house −1, Haven morale −2); a test (★ Police sees the hair across the seal; the crew's leader remembers the carrier). The clerk at the drop: [Known · Bastion] or 2 tins and it goes on top of the pile (never caught); real: warn the house instead (it ends: 10 Haven Favor, +6 Red Hand Infamy, a callback). Record `double.dossier`* |
| 18 | Collect the Bounty | Champion | | Kill a player or AI with a bounty on them | Fame, Favor | ✅ *a player Kill on Sight with another faction (not a Red Hand friend), paid to the player who kills them (not for a death to the infected, a fall or suicide); else an armed loner outside a town (Contracts > Bounty target, a group on SURV: he's made hostile as SURV), who shoots back* |
| 19 | Kill the Officer | Champion | | Assassinate a Bastion outpost commander (the stronghold's commanding officer never dies) | Huge Fame (+75 Bastion Infamy: Kill on Sight) | ✅ *needs a killable commander body (Leaders > Bastion outpost commander or Bastion leader); a new one takes the chair after 2 hours. The officer is held while it runs; the brief names whoever sits in the chair when it's read* · **One choice** (`officer.fate`): within 40 m of him his aide offers the post's stores; take them and walk (the job ends paid by Bastion at half Favor, no Fame, no Bastion Infamy, the boss remembers) or do the job (default) |

**Amends:** Fence the Goods works off Red Hand Infamy; so does paying Favor ([REPUTATION.md](REPUTATION.md#working-off-infamy)).

### The Red Hand's hard edges

What the crews don't say out loud ([LORE.md](LORE.md), "The Red Hand"), as jobs with a choice in
them. In the Red Hand's two, choosing against it makes the Haven the payer: anyone the Haven
doesn't hunt is paid, the Red Hand adds Infamy, and the crew boss who gave the job doesn't count
it as help. Tied Down is the Haven's, and taking a cut makes the Red Hand the payer the same way.
Code: `Missions/RedHand/RZ_RedHandLoreMissions.c`.

| # | Contract | From | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|---|
| 1 | Learning the Drones | Red Hand | Known, Perimeter on and sealed (not after the cure) | ★ Radio Op: counts twice as fast, so he's out there half as long (shot 35%, not 55%) | A crew waits with a debtor 120–200 m outside the warning zone. **Walk him to the line**: he walks with you to a marker 20 m short of the kill zone, then on alone; stay within 50 m of the marker for 90 s and count. A warning shot, then maybe the second one. **Get him away**: he follows you to the nearest Haven safehouse (the crew fights you) | Walk: Fame, Favor, whether he lives or not (shot 55% of the time). Away: Haven Fame, Favor, he joins the safehouse (+15 Red Hand Infamy) | ✅ *spots from the Perimeter helper, so it works on land and sea edges (on a sea edge he wades out from the shore); never where the coast lies past the drones' reach. He is data out past the marker: the shots are effects, not a body. The crew is real only with the Red Hand group prefab* |
| 2 | The Pit's Debtor | Red Hand | Known | ★ Ex-Con: caught rigging 10% (not 25%), half the chance of being seen walking him out | Nobody volunteered for the pit (the season's one pit, Fight Pit's), so a debtor goes in, and you're his second. **Pay what he owes** (25 Red Hand Favor), **rig the pens**, or **get him out** | Pay: half pay. Rig: caught, +10 Red Hand Infamy and failed; else keep him standing in the ring for 150 s (two rounds of three infected): full pay. Out: walk him to the Haven; seen near the pit (30% a check by day, 12% by night, every 5 s within 60 m): +8 Red Hand Infamy at once and the pit crew fights. Arrives: Haven Fame, Favor, he joins the safehouse (+15 Red Hand Infamy). Nobody chooses in 40 min: he goes in alone, failed | ✅ *only a player the Red Hand hired can pay, checked before the Favor is taken. The pit crew is real only with the Red Hand group prefab. The pit is held while it runs* |
| 3 | Tied Down | Haven | Known | ★ Hunter: the crew sees you within 15 m, not 30 | A Red Hand truck carries people. Three steps: a lookout's note outside one town names the next; there the truck is parked (watch it from within 80 m for 20 s; seen, its crew fights and it leaves at once); then it drives on to a shed outside a third town. At the shed: **cut the lock** (Cut the lock, then hold **Cut the padlock** at the door, 12 s; the guards fight) and drive them to the Haven in the truck, **radio the Haven**, or ask the driver for **a cut** | Free: Fame, Favor, +3 at the safehouse (+15 Red Hand Infamy). Radio: 60% pay, +2 at the safehouse (+8 Red Hand Infamy). Cut: the Red Hand pays anyone it doesn't hunt, 10 Fame and 40 Favor (+15 Haven Infamy) | ✅ *the truck is Supply > Red Hand truck (needed); it's data while it moves and only parked trucks are seen, with no fuel until the lock is cut. It waits at the stop until someone has watched it. Its crew is Supply > Red Hand crew, the shed's guards the Red Hand group (each falls back to the other; neither = none). Freeing them needs a vehicle, so No Vehicles takes it out. Reaching the shed sets `cargo.truck_seen`, the Haven's way into the [Cargo questline](#cargo-the-red-hand-or-the-haven), which tells the long version with the same van and padlock; while it runs, Tied Down isn't offered* |

### Halcyon

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | The Lone One (was Alpha Sighting) | Any | | Kill a lone one (a pack's leader walking alone) when there is one, else a special in a bad town, and bring its samples | Fame, samples | ✅ *a lone one the Haven knew is the Haven's job instead: lay it down by name (Haven Fame and Favor). Its brief never says what it is, only "a big one, on its own"* |
| 2 | Field Samples | Any | ★ Lab Tech/Nurse: each kill counts double | Kill infected in an infested town while Halcyon takes readings | Fame, research | ✅ |
| 3 | Water Survey | Any | ★ Lab Tech: +50% Fame | Sample wells in 3 towns: hold **Take a water sample** at each (10 s) | Fame; reveals the town most at risk | ✅ *the most infected town still under 50, anywhere on the map* · **One choice** (`survey.warned`): Halcyon only (default), or the Haven too (one person moves from a house in the town at risk to another house) |
| 4 | Power the Lab | Any | ★ Mechanic: +50% Fame | Bring fuel and a generator part to the lab | Fame; antidote doses come twice as fast for 2 hours | ✅ *the part is the mission item at a factory, petrol station or power station in a nearby town* |
| 5 | Lost Colleague | Known | ★ Lab Tech +1 to persuade; ★ Nurse or Doctor reads the bite | Find a missing researcher. The truth (per name): hiding hurt, turned, or walked out to a Haven house within 3 km (his camp's note points there) | Research +2. Hiding: say nothing, or tell the field lead. Turned: put him down; his notebook whole (+2), or its last pages torn out (+1, 60%). Walked out: persuade him back, press him (+4 Haven Infamy), trade the house 4 food for his notes (+1), take his tag and say he turned (60%, a lie a Halcyon-Known visitor finds out: +10 Infamy), or leave him be (no pay) | ✅ *story key `colleague`; record `colleague.fate`. **Sold** (10%, ×3 in The Missing Courier; a Red Hand holding within 5 km): a crew's chit in his camp, his sister at the nearest house, his notes on the fence's table: bribe the fence (25 Red Hand Favor), lift them at night unseen, or [Known · Red Hand] buy them back for 5 food (+1 research each); or leave them with the crew (no pay, a Red Hand route asset). Hiding: [Nurse or Doctor] patch him up and he walks back himself (Halcyon morale +1). Turned: the notebook can go to a Haven medic instead (no research, nobody pays, infection −3 in her town, a Haven route asset). Walked out and left be: the house's water fault is fixed, he takes to the roads as a Doctor if a place is free, an Independent route asset. **The search:** a step chain from his field station (the nearest Halcyon station or the lab): his cold box, then his pack with his grid; the job's spot is the cold box until then. Shortcuts: ★ Radio Op within 1.5 km hears his set and his grid; a scavenger by the cold box sells the last sighting for a tin; a Haven trader in the nearest lived-in town saw "a man with a lab badge" (and, walked out or sold, where he went or who with). Hiding, say nothing or tell: you walk him in (an escort to his station or the lab within 3 km, the job's clock at least 20 minutes; Contracts > Escort scientist); no escort free, or nowhere near, Halcyon sends a stretcher. Dead on the way, or time up while he hid: fallout Notebook 31* |
| 6 | Research Data | Known | | Recover a hard drive from an overrun clinic or pharmacy | **Choice:** Halcyon research, or sell it to Bastion (Halcyon Infamy) | ✅ *the building is found by prefab name; the drive lies loose inside (on its furniture or floor, like [loot in houses](DESIGN.md#loot-in-houses)), put out once someone is within 60 m; a town with none: on the ground where a courier fell near the middle. **Story job** (its key `researchdata`, per town): research as they said; a trial log with Haven names (a house within 5 km); already copied (the lab gets +1, Bastion pays half). The orderly sweeping the glass: [Known · Halcyon] or a tin and he brings the drive; trials: give the names to the house (it ends: 8 Haven Favor, +5 Halcyon Infamy, a callback at the door, a Haven asset). Record `researchdata.names` (and who benefits' `went_to`)* |
| 7 | Install Sensors | Known | ★ Radio Op/Lab Tech: twice as fast | Place 3 sensors in infested towns | Fame; twice the specials there for 2 hours | ✅ *hold **Set up the sensor** at each spot (15 s; Radio Op/Lab Tech 8 s): the parts become a set on a crate* |
| 8 | Blood Drive | Known | 🔒 not infected | Donate blood at the lab (costs health) | Fame, small research | ✅ *a quarter of your health, refused under 70%, once per life; amends* |
| 9 | Cold Chain | Known, cure 50%+ | ★ Nurse/Doctor: 25 min, not 15 | Get an antidote batch to a Haven clinic before it spoils | Fame with Halcyon and the Haven | ✅ *the clock starts when it's picked up; the clinic is a Haven drop at a safehouse. **Story job** (key `coldchain`, per house): a batch as they said; two doses short; an unnumbered trial batch (★ Lab Tech, ★ Nurse/Doctor read it). The porter: [Known · Halcyon] or 2 tins and he packs it properly (25 minutes for anyone). At the clinic: hand it over; short: tell them (+3 Halcyon Infamy, 5 Haven Favor, a callback, a Haven asset); trial: tell the nurse (she refuses it: Halcyon and the Haven's Fame pay nothing, 5 Haven Favor, an Independent asset). Record `coldchain.batch`* |
| 10 | Escort the Scientist | Trusted | | Escort a scientist to take readings at 3 sites | Fame, research | ✅ *she follows whoever leads her; 20 s within 15 m of each site. **Story job** (key `scientist`, per town): readings as they said; she looks for her brother (a house within 5 km of the last site; ★ Elder); she times the drones (★ Radio Op). The lab's driver: [Known · Halcyon] or 2 tins and he takes the last site (two to walk). Done: hand them in; brother: leave her with him (half pay, no research, the house +1, a callback, an Independent asset); the line: burn the drone times (research +1, nobody pays) or copy them for the crew (10 Red Hand Favor, +5 Halcyon Infamy); readings: the water readings to the house too (5 Haven Favor, research +1). Record `scientist.fate`* |
| 11 | Specimen from the Nest | Trusted | | Get a sample from inside a hive without destroying it | Big research | ✅ *hold **Cut the tissue** at the hive's edge (15 s; it hears you the whole time), then carry it to the lab. Needs the nest prefab* |
| 12 | Hunt the Screamer | Trusted | ★ Hunter: +50% Fame | Kill a Screamer before it alerts the town (4 min) | Fame, rare sample | ✅ *by day, with the packs on, a pack's posted scout when there is one (its own rule decides when it shrieks; [INFECTED.md](INFECTED.md#scouts)); else a town's screamer* |
| 15 | Lab Defence | Trusted | | Hold the lab for 4 minutes against waves (and Red Hand raiders if their prefab is set) | Fame (Red Hand Infamy if raiders) | ✅ · **One choice** (`labraid.raider`): a raider surrenders: Bastion's stockade (default, 4 Bastion Favor), back to the crew (6 Red Hand Favor) or to Halcyon for the trial (+4 Red Hand Infamy) |
| 16 | Medical Drone | Trusted | ★ Mechanic/Radio Op: the exact grid within 600 m | Recover a crashed prototype drone | Fame, a little research | ✅ *somewhere within 250 m of its last ping; carry its core to the lab* |
| 17 | Patient Zero | Champion | ★ Lab Tech: clues also give the grid | Follow clues through 4 towns to the outbreak's origin | Huge research; solved, it opens [The Three Weeks](#the-three-weeks-halcyon) | ✅ *a body in each town names the next; the last is the season's canon clinic (the same all season), and the records name the first patient, the town below the site and the day the Institute was told. 45 s searching the clinic while infected come. Not while [The Three Weeks](#the-three-weeks-halcyon) runs* |
| 18 | Live Capture | Champion | | Lure a special into a cage trap alive | Huge research | ✅ *or wound it and tie it; hold it 30 s, then carry the case to the lab within 12 min. With the packs on it's one of a free pack's guard (its young if it has under 2 guard), in its look, at the edge of its lair: the pack is held while it runs, and the one caught leaves it* |
| 19 | Serum Trial | Champion | 🔒 infected only | Take an experimental serum while bitten: it cures you or speeds up the turn | Huge Fame; research either way | ✅ *50/50; the turn comes twice as fast. Separate from the everyday trial dose at Halcyon's quartermaster ([DESIGN.md](DESIGN.md#treatments))* |
| 20 | Mobile Lab | Champion | | Escort the lab truck to a safer town | Fame; a field annex there takes samples for 3 hours | ✅ *the lab never moves (it's the stronghold); players drive the truck; the annex is a second lab prefab* |

**Amends:** Field Samples and Blood Drive work off Halcyon Infamy while you're Watched.

---

## The packs' contracts

Jobs about the packs in the wild ([INFECTED.md](INFECTED.md#people)). Code:
`Missions/Tribes/RZ_TribeContracts.c`. Each holds its pack still while it runs (the claim
`tribe:N`: no lair move, split, merge or absorption), counts its research ×0.75 through the lab's
backlog, and is offered only within an hour of play before its window, so it can't outlive its
night. Each giver names the pack in its own word, always with the place. They need **Zombies >
Tribes > People** on and a pack on the map. Halcyon's stop with the cure; Bring Them Home, Cull
and Clear the Trail run on after it (with no samples, as nothing drops them).

| # | Contract | From | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|---|
| 1 | Not Tonight | Haven | Any (unarmed is fine); offered 17:00–18:00 | ★ Radio Op: more Fame | A pack will come to a house's wire tonight (a back-door habit, or a hungry pack testing the wire). Before 21:00, make a noise at a marked spot 600 m off its way, then get clear (nobody within 300 m for 20 s) | Fame, Favor; no wire trail tonight, the back-door habit −30. It reaches the wire (21:00 with no noise): no pay, two people lost from the house | ✅ *the noise is any remembered noise within 50 m of the spot in the last minute with someone within 60 m. If it's already night the pack's hunters and young turn for it at once* |
| 2 | Bring Them Home | Haven | Known | ★ Elder, ★ Nurse | Someone named runs with a pack's young (a walk-out, a wanderer, a player's character). Put that one down (it's always the one the job wants when an eligible player is within 100 m), then carry what's in their pockets (the mission item) to any house | Fame, Favor; the name is read at every house (a player's character as the server's name setting allows) | ✅ *never for someone in your group, someone who died within 300 m of you or under a season day ago; one delivery per player a season day. Withdrawn if someone else lays them down first. Needs the mission item prefab. **Story job** (key `bringhome`, per name): who they were, as the board says, or someone else (the card in the pockets names another); the pockets tell it to the room (★ Elder reads the family, ★ Nurse how long ago). At the house the clerk asks: the card to the family (a callback at the door), or sell the keepsake to the fence (6 Red Hand Favor, the Haven pays nothing, +3 Haven Infamy, a Red Hand asset). Record `bringhome.fate`* |
| 3 | Escort at Dawn | Haven | Known; offered 03:30–04:30 | ★ Hunter | Walk 3–5 people from a house, across a pack's ground, to another house 0.8–3.5 km away. They set off from 05:00, when you're with them | People at the house they reach; Fame, Favor. One is lost for each of the pack's groups that comes within 150 m of them; none left: failed | ✅ *the Escort block; the clock ends at 06:30* |
| 4 | Contact Report | Bastion | Any; amends | ★ Hunter, ★ Radio Op, ★ Soldier | At dusk, listen 30 s at each of two points 350–600 m from a pack's lair, without its scouts or a pack coming for you | The pack is Mapped for you and goes on Bastion's contact board (Read for Bastion's Known) | ✅ *offered 17:30–18:30, a pack not yet on the board first; dusk is 18:30–20:30* |
| 5 | Clear the Trail | Bastion | Trusted; presence 0.2+; offered 19:30–20:30 | | Kill 6 of a pack in one night within 300 m of a trail that passes within 3 km of a Bastion holding | Fame; the trail is dropped for 3 counted nights, its crossing and road habits −50, disturbance +30 | ✅ *a kill counts with a player within 200 m. Also on a truck road: a trail where it crosses a road (tonight's crossings), wherever it runs. A military pack's trail (Bastion's own dead, a lost post): when it's done a corporal asks (one choice): their tags to the nail (Bastion morale +1, a Bastion asset, record `trail.lost_post`) or leave them walking (nobody pays, an Independent asset)* |
| 6 | Cull | Bastion | Trusted, Champion for a leader in its prime or older; presence 0.2+ | | Kill the leader, or 3 of its guard, of a pack of 12 or more within 3 km of a Bastion holding. Bastion's smoke is allowed | Fame; disturbance 100, so the pack is pushed off away from Bastion (within the displacement cap); *"Not our sector"* to Bastion's Known, and the Haven hears where it went | ✅ *When it's done, where the pushed pack goes is the shared "where the horde drifts" choice (an empty town by default, past Bastion's post, past a Red Hand camp)* |
| 7 | Burn the Lair | Haven or Bastion | Trusted; fire on; offered 09:00–13:00; you must have the lair Mapped | | Between 11:00 and 14:00, set a fire within 30 m of the lair, then get out (nobody within 150 m) | Fame; the life cycle's fire moves the lair and burns the land ([INFECTED.md](INFECTED.md#burned-land)) | ✅ *offered only while someone online has it Mapped. With 4 or more of the pack killed near the lair in the half hour before the fire, 3–6 of the rest break off as a splinter ([INFECTED.md](INFECTED.md#lair-moves-and-displacement))* |
| 8 | Break the Habit | Bastion or the Red Hand | Known; offered the hour before the habit's hour | | A pack hunts a road at its hour (a road habit). Be there and kill 4 of its hunters within 200 m of the place | The habit −60 (gone under 10, so usually two contracts); Fame, Favor | ✅ |
| 9 | Bait the Crew | Red Hand | Trusted; offered 18:30–19:30, only with nobody within 500 m of the border | ★ Radio Op: twice the noise | Two packs share a border. At night, make heat 40+ at the border point (within 150 m) | One side's hunters cross: a Border Night with you in it ([INFECTED.md](INFECTED.md#borders)); when it's settled, Fame and Favor | ✅ *Zombie Bait's rule: when it's settled, a holding or a house within 600 m of the border point is +4 Infamy with its faction for those who baited it (the Red Hand's own excepted)* |
| 10 | Plant a Tag | Halcyon | Known; offered 06:00–12:00; needs the tag item | ★ Lab Tech | Pick up the tag at the lab. Use it (**Tag it**, held 5 s) within 8 m of a pack's leader in its prime or older, by day or in its sleep after a feed, while the pack isn't after you | Research +1 (×0.75); the pack is tagged: Lab Techs and Halcyon's Trusted read its leader's rough area in the journal, Halcyon's range map shows it, and Halcyon reports on it at dusk | ✅ *killing a tagged leader in its prime or older costs +2 Halcyon Infamy ([REPUTATION.md](REPUTATION.md))* |
| 11 | Track Them Home | Halcyon or Bastion | Any (unarmed is fine); offered 03:30–04:30 | ★ Hunter | Between 05:00 and 07:00 follow a group of a pack walking home within 200 m of it until it's all home and you're within 300 m of the lair, without it coming for you | The lair is Mapped for you; Bastion puts it on its board, Halcyon gets a little research | ✅ *lost for 90 s: failed; 10 s closer than 80 m (warned once): failed; being seen: failed* |
| 12 | Capture the Index | Halcyon | Champion | | A pack's leader in its prime or older, alive, with Live Capture's rules: wound it to a third and get within 3 m to tie it, hold it 30 s while its guard comes, then carry the case to the lab before it wakes (12 min) | Live Capture's research (×0.75); the pack holds its vigil, takes a new leader, and goes **Furious** | ✅ *the leader's own body is the special (it carries the mission while the job runs). While its case is carried, its pack is drawn after it every minute (`RZ_Tribes.DrawTo`: by night its hunters and young, by day its day party). Needs the mission item, Halcyon's drop and an Alpha prefab. The finale's live tissue builds on it ([FINALE.md](FINALE.md#live-tissue))* |
| 13 | Stock the Pit | Red Hand | Known; a Red Hand camp within 6 km of the lair | | Two of a pack's young alive, one after the other, with Live Capture's rules: lure it into the cage at the lair's edge and drop the door, or put it down without killing it and tie it; hold it 30 s, then the case to the Red Hand's drop by the camp before it wakes | Fame, Favor; the next Fight Pit's last round lets them out: *"Fresh hires, from the Postmen."* Each one caught leaves the pack | ✅ *the second capture starts when the first case is handed in; if the pack has no young left by then, half pay. House Rules is a questline stage now, so only the Fight Pit takes them* |

The Lone One is Halcyon's #1. **Fire Mission** and **Bastion Shelling** on a lair disturb it +50
([INFECTED.md](INFECTED.md#lair-moves-and-displacement)), never a pack a job holds.

---

## World events

News on the radio for everyone, open to everyone; never a job on the Quests tab, never tracked,
never in the Journal's past jobs. Most pay in loot found on the spot rather than Fame.
Code: `Missions/Events/`; numbers and optional prefabs in **Missions > Events**
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)). Nothing comes in from outside: every reward was
already in the Zone, and every crate is earned on the spot.

| # | Event | What happens | Why it's interesting | Status |
|---|---|---|---|---|
| 2 | The Tribe Moves (was Horde Migration; players see "On the Move") | A pack walks across the map to its new lair, as a herd shown as two groups within 600 m of a player | Get out of the way (lying low as it passes works: its senses drop with [Stealth](DESIGN.md#the-infected-near-you)), or thin it out: 10+ kills leave its old lair's finds (a crate there) | ✅ *started by the life cycle when a lair moves out of its range, a pack is pushed off, or a nomad moves on ([INFECTED.md](INFECTED.md#lair-moves-and-displacement)); never picked at random while the packs are on. With them off, the old migration between two towns, from Collapse* |
| 3 | Every Ridge Calls (was Night of the Dead) | A frenzy night: announced at dusk, every pack hunts three trails with its range doubled until dawn, and for 20 minutes the towns hold twice the infected and specials | Many hunts, not one endless one: the commitment rule still holds | ✅ *one frenzy at a time, none in the first 2 season days, one every 3 at most; only at dusk while there are packs. A frenzy ignores fear places and borders ([INFECTED.md](INFECTED.md#contracts-and-events)); the great one's clan gathers. With no packs: the 20 minutes only, any hour* |
| 4 | Fog Bank | A region drops to low visibility for 20 min | Hiding is easy. So are ambushes | ✅ *the engine's fog is global and never changed: within 700 m of a town, heat is ×0.6 and infected spawn twice as close; an optional mist prop is the look* |
| 5 | Distress Call | A survivor asks for help on the radio | Could be real, a loop, or bait (Fake Distress Call merged in) | ✅ *story key `distress`; one announcement, the truth per town for a season day. Real (50%): two people cornered at a town's edge, infected round the house once someone is within 300 m; talk them out with nothing within 20 m: they join the nearest safehouse and leave a crate. A loop (30%): a tape on a crate and two Red Hand groups in the trees (they appear within 400 m, close in at 60 m, spare Red Hand friends; all dead, their kit). Bait (20%): two people tied up in the house and the crew waiting. Cut them loose (quietly at night, else the crew comes), "They're mine" (Known with the Red Hand, Trust 2), buy them (30 Red Hand Favor), or Side with the crew after the hold (10 Red Hand Favor; seen by the Haven, +12 Haven Infamy at once, otherwise a lie the next Tied Down finds out). Answering on your radio (within 800 m) tells them apart; so do a Radio Operator, a Hunter at 150 m and, for Trusted Red Hand players, the secure net. People dead: fallout The Name on the Tag. 25 min. Record `distress.saved` / `distress.dead` / `distress.taken`. Saved: a callback at the house they went to, some hours of each rescuer's play later. **Real, more ways:** [Sneak] out the back while hidden, whatever's at the door; one time in four the woman's "fall" is a bite (★ Nurse or Doctor sees it): the house finds it at the door and one walks in, unless a Nurse dressed it and said nothing (both walk in; a lie the house finds within a season day, +8 Haven Infamy to her, the life record "hid a bite"). Out, they ask where (one choice): the nearest house, *"My sister's in {town}"* (a house 1.2–4 km off, with its own callback) or Bastion's post (civilians sheltered on Bastion's call, 5 Bastion Favor to those with the decider). Dead inside a pack's range: the caller runs with that pack as a named young* |
| 6 | Abandoned Convoy | Bastion trucks left on a highway | Loot, but it may be booby-trapped or watched | ✅ *Bastion's own trucks, inside the Zone since the first week, halfway between two towns (not snapped to a road); two crates and a few infected. 40% a tripwire: a Mechanic, Soldier or Police within 25 m cuts it, else the first to the crates sets it off (an explosion if the prefab is set, 80 heat) and one crate is lost. 35% a Red Hand crew watches it and fights all but its friends. The wrecks are optional props* |
| 7 | Power Surge | A town's power comes back for 10 min | Alarms go off (heat); electronics loot | ✅ *a dark town outside the grid's region is Powered through the power grid and town memory; three alarms make heat for 90 s; the grid's electronics loot bonus applies to buildings rolled while it lasts* |
| 8 | Siren Test | Air-raid sirens pull every infected into one town for 5 minutes | The towns around it are briefly safer to loot | ✅ *packs come only if they've learned the bells (a bells habit, within 3 km)* |
| 9 | Wandering Trader | A neutral merchant parks somewhere for 20 min | Rare items; protect him or rob him | ✅ ***Story job** (key `trader`, per town): honest as they said; a dead house's stores (a house within 6 km; ★ Elder, ★ Lab Tech); he sells the roads to a crew within 6 km. His wife on the wagon: stolen: send the crates home (the house +3 food, 8 Haven Favor, he leaves, a callback, a Haven asset); roads: tell the house (Haven morale +1, +4 Red Hand Infamy, he leaves) or 2 tins for your page of his notebook. Record `trader.fate`* |
| 10 | Gas Station Fire | A station explodes and burns | Infection drops; loot is destroyed | ✅ *a real petrol station (by name) in a town nobody lives in; it goes up 3 min after the warning: infected within 30 m die, the station and up to two buildings beside it burn (the fire system), town infection −8. Needs fire on* |
| 11 | Prison Break | Escaped convicts roam the area | Hostile; the Red Hand recruits them | ✅ *two groups from a town's police station walk to the nearest Red Hand camp (data until a player is within 600 m). They fight anyone near them except Red Hand friends (Known+). All dead: a crate where the last fell. Any reach the camp, or 30 min pass: the Red Hand gains strength, a headline, and its friends who walked in with them +4 Fame. **Story job** (key `prison`, per town): convicts as they said; half held for nothing (Bastion's curfew sweep; ★ Police reads the sheets); a crew cut the lock from outside (★ Mechanic). The jailer: [Known · Bastion] or 2 tins and he rings the town's bell (heat, and a horde where the convicts are: no gun); swept: tell the curfew lot to go home (one group fewer, the nearest house +2, a callback, a Haven asset); opened: tell the post (5 Bastion Favor, +4 Red Hand Infamy, a Bastion asset). Record `prison.fate`* |
| 12 | Bastion Shelling | Bastion's own mortars shell an infested town after a 5-minute warning | Get out, or loot fast. Infection there drops sharply | ✅ *never within 300 m of where people live (a safehouse, a holding, the lab)* |
| 13 | Bell Tower | Someone rings church bells nonstop | A horde converges | *covered* by The Preacher's Flock arc: he rings bells in every town (twice the heat) and small hordes come for him |
| 14 | The Sniper | A crazed survivor shoots from a tower | Take him down; his stash is up there | ✅ *on the high ground above a town nobody lives in, not up a tower: one armed AI (Events > Sniper, a BAND group; without it the SURV bounty target, made hostile as SURV), there only while a player is within 900 m, who fights everyone (Red Hand friends too); his shots make heat. Dead: his stash. 30 min, then he moves on* |
| 15 | Stash Note | A note with coordinates turns up | Treasure hunt | ✅ |
| 16 | Mass Grave | A pit full of infected bodies | Lots of samples, and a horde underneath | ✅ *the nearest pack within 3 km has eaten there: its hunger −20* |
| 17 | Lost Child | A child wanders alone | Escort them to the Haven | *covered* by the Kid wanderer: show them the way and they follow you to a safehouse (+1 person, +5 Haven Fame) |
| 20 | Storm | For 15 minutes, noise carries less and heat fades three times as fast | A good time to move | ✅ |
| 21 | Last Broadcast | A station is still on the air | Reach it and broadcast | *covered* by the radio station: always on the map rather than an event; take it over and broadcast a preset (cosmetic, no Fame) |
| 22 | Rival Survivors | An unaligned AI group | They trade or fight depending on your reputation | ✅ *a group on `SURV` camps outside a town nobody lives in for 30 min. Hunted by anyone, or Watched by two factions: they fight you (only you). Trusted by someone and Watched by nobody: a crate of their kit for 2 food; anyone else 3 food; two trades in all. Gunning them down when they'd fought nobody: +6 Haven Infamy; if they had: their kit* |
| 23 | Locked Bunker | A sealed bunker, code split over 3 notes | Teamwork or a race between strangers | ✅ *a real locked building in a town nobody lives in: its door lock becomes a keypad (no key or crowbar while it runs), two crates inside, six digits on three bodies around town. The right code unlocks it for everyone; three wrong ones lock the keypad for 2 min (noise). No lockable building, or locks off: a steel hatch and the crates come up when it opens. 40 min* |
| 24 | Turned Player | A player who turned roams with their gear (started when someone turns) | Their loot is up for grabs | ✅ |
| 25 | Tunnel Rats | Infected flood out of tunnels | A town's infection spikes | ✅ *three culverts at the town's edge (the generic puzzle object); each open one adds 1 infection a minute (at most +20) and sends two infected into town while a player is within 700 m. Collapse all three (no infected within 12 m) for a crate and −5 infection* |
| 26 | Military Remnant Camp | An abandoned Bastion camp with gear | Bastion Infamy if they're still around | ✅ *outside an infested town nobody lives in, never near a Bastion holding (an eighth of the map's span, 400 m to 1.5 km): two crates, turned soldiers and a special. With a military pack within 3 km, the soldiers are its young, in its look. While Bastion is still around (military presence 0.1+), coming within 4 m of a crate is taking: +10 Bastion Infamy, once per player. The camp is an optional prop* |
| 27 | Wedding Party | A horde in formal wear by a church | Dark humour, jewellery for trading | ✅ *six guests and the bride by the church of a town nobody lives in, there once a player is within 500 m. About half drop a piece of jewellery, the bride always (at most 6); a Trader wanderer takes one instead of Favor. No jewellery prefab: a crate once they're all down* |
| 28 | Collapse Shelter | A town's shelter is overrun and survivors scatter | Save them before they turn (Haven Fame) | ✅ *four people hide round the school or church of a town nobody lives in, each with infected close by; they turn after 15–22 min. Tell each the way (nothing within 15 m) and they join the nearest safehouse; the Haven's 12 Fame and 8 Favor go to the rescuers, scaled by how many were saved. **Story job** (key `shelter`, per town): overrun as they said; it started inside (★ Nurse/Doctor); Bastion chained the doors (★ Soldier). The warden: [Known · Haven] "Where would they hide?" (5 more minutes each, the grids) or 2 tins for his whistle (the nearest still hiding comes to him: saved); inside: tell the house (Haven morale −1); chained: take the chain to the house (6 Haven Favor, +3 Bastion Infamy, a callback, a Haven asset). Record `shelter.fate`* |
| 29 | Red Dusk | At a clear dusk, the packs within 3 km of one point frenzy: range ×1.75, three trails, the nearest wire first | *"The sun's going down red over the hills, and they're all calling."* | ✅ *from Collapse, at most once every 7 season days, counted with the frenzies above (and like them it ignores fear places and borders). Simplified: the sky isn't changed; it runs only at a clear dusk (no rain)* |
| 30 | The Gathering | Every 4 season days, and on a frenzy night, while there's a great one: each pack of its clan sends a group along its way to the nearest lived-in place's wire, in waves, until first light | *"Every ridge answered the call from the quarry by Morton tonight, and now they're all coming down toward the wire at Morton together."* Hold the wire: 10 or more of them put down within 300 m leave what they carried there at dawn | ✅ *started by the great one ([INFECTED.md](INFECTED.md#the-great-one)), never at random* |
| 31 | The Night of Many Calls | The dusk after the great one dies: every border on the map reopens for 2 counted nights | *"Every ridge is calling at once tonight, and none of them is answering the same voice."* Border Nights at the bridges | ✅ *started by the great one's fall, never at random; told until dawn* |

The packs' own events are the life cycle's, not missions, and aren't counted above
([INFECTED.md](INFECTED.md#the-life-cycle)):

| Event | When | What | Status |
|---|---|---|---|
| Border Night | A border fight with a player within 800 m (or Bait the Crew) | Both sides' hunters at the border or the bridge for up to 20 minutes, with the calls; heard, then found | ✅ |
| The Clash | A challenge at a Declining leader's lair with a player within 1 km | The ring at dusk, the two meet; step in and choose who leads | ✅ |
| The Vigil | A leader dies with a player within 1.5 km | The pack gathers where it fell until dawn or 30 real minutes | ✅ |
| Furious | Its leader caught alive (Capture the Index) | For 2 counted nights every trail runs along the capture road toward the lab, stopping at the lab's no man's land plus P, whatever its hunger. The Haven: *"Whatever they took from …, they want it back."* If it's still Furious at the final stand, it comes with the last wave (the most recent one only) | ✅ *simplified: its walkers don't walk that road by day, and couriers aren't warned* |

---

## Puzzle missions

Contracts you solve by reading, deducing and sneaking rather than shooting; most low-tier ones
can be done unarmed. Each works on any map (built from settlements and real landmark buildings
found by prefab name, with a fallback prop) and deletes everything it spawned when it ends.
Code: `Puzzles/`. ✅ all 17.

| # | Puzzle | From | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|---|
| 1 | The Numbers Station | Haven | Any | ★ Radio Op: hears it without the note or mast | A radio log gives a frequency. Of three radio masts, one carries a voice reading a map grid, every third number a decoy | Fame; stash at the grid. *It sits in the middle of the grid square and is found within 45 m* |
| 2 | Two Bearings | Bastion | Any | ★ Hunter/Soldier: more Fame | Two wreck pieces have bearings scratched on them. Reach where the lines cross | Fame; the pilot's kit |
| 5 | The Rat | Red Hand | Known | ★ Police/Ex-Con: more Fame | Three suspects, exactly one lying. Point at the rat at the boss | Fame. Wrong: fails, Red Hand Infamy |
| 6 | Patient Zero's Ledger | Halcyon | Known, a lab | ★ Lab Tech/Doctor: the method spelled out | A ledger plus hospital bracelets on three bodies. Name who was first, at the lab | Fame, research; solved, it opens [The Three Weeks](#the-three-weeks-halcyon). Wrong: locked 5 min; twice: fails |
| 7 | The Poisoned Well | Halcyon | Any | ★ Lab Tech/Nurse: can test the water | Three wells, two notes ruling one out each. Seal the poisoned one | Fame, research, that village −10 infection. Wrong: infection up; twice: fails |
| 8 | The Smuggler's Ledger | Red Hand | Known | ★ Ex-Con: more Fame | A Caesar-shifted ledger line; the key is on a memorial plaque by the church | Fame; stash |
| 9 | The Clockwork Horde | Haven | Any | ★ Hunter: harder to notice | A stuck siren sends a horde back and forth through a square. Take the strongbox unnoticed | Fame; the strongbox. Noticed: the horde turns on you |
| 10 | Time-Lock Safe | Red Hand | Known | ★ Ex-Con: window twice as long | A bank safe opens only at 03:00 game time, for 20 min. Opening it is loud | Fame; the safe. Offered only when 03:00 is at most 3 real hours away |
| 11 | The Bell Order | Haven | Known | ★ Elder: more Fame | Ring three village bells in the order on a hymn sheet, at most 15 min apart | Fame; heat and infection drop at the Haven safehouse in the least infected town (with no safehouses: the safest town). Wrong: a horde, start over |
| 12 | Lights in the Window | Bastion | Known, night | ★ Radio Op: Morse on the radio too | A lamp blinks a code word in Morse. Decode it and reach the right meeting place (the middle of a grid square, within 45 m) | Fame; a pack. A wrong meeting place first: fails |
| 13 | Junction Boxes | Bastion | Any | ★ Mechanic: one more switch named | Four fuse boxes, two switches each, lights-out wiring. Get all eight green | Fame; the substation's contents |
| 14 | The Weighing Vault | Haven | Known | ★ Mechanic: reads to 0.1 kg | Fill a scale crate to the weight in a riddle (±0.2 kg) | Fame; the vault. *Simplified option:* count items instead of weight (a setting) |
| 15 | The Last Experiment | Halcyon | Trusted | ★ Lab Tech: first letter given | Each missing researcher's belongings carry one letter of a password (no password is an anagram of another word). Type it on the keypad | Big research. Three wrong: wiped |
| 16 | The Silent Village | Haven | Trusted | ★ Elder: more Fame | A village with no infected. Find what's under the church, then **burn it**, **seal it**, or **let Halcyon study it** (until the cure is out) | Depends on the choice |
| 17 | The Witness | Haven | Known, night | ★ Police: talks by day too | A wanderer saw a Haven courier killed. His story points to a camp. **Report it to the Haven** or **sell it to the Red Hand** | Depends on the choice |

How they work:

- **Clues** are notes, bodies, plaques, wreck pieces or people with `RZ_ClueComponent`; reading
  one sends its text to that player only.
- **Answers** go through actions on one generic puzzle object (keypad, choices, uses); the
  mission labels them.
- **Wrong answers** cost something: heat, a horde, a lock-out, infection, Infamy or failure.
- **Night missions** appear only at or near nightfall and last until dawn. Time is never skipped.

## Unarmed missions

Low-level work for new characters, unarmed backgrounds or anyone who'd rather not shoot. All
are open to anyone not Hunted by the giver, pay small Fame, and are built on **noise that
leads** and **being noticed** ([DESIGN.md](DESIGN.md), "Heat and noise"; lying low helps,
[Stealth](DESIGN.md#stealth)). Code: `Missions/Unarmed/`.

| # | Mission | From | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 3 | Lead Them Away | Haven | ★ Radio Op: more Fame | A horde drifts toward a safehouse. Lead it with noise to a quarry 550–750 m away | Fame, Favor. It arrives: fails, two people die. Killed instead: half pay | ✅ *the quarry is a grid reference. With a pack's ground within 3 km of the house, it's that pack's hunting party, in its look, coming from its side (in the Haven's word). With the life cycle on, a free pack of 8 or more (a nomad always, else half the time) is the pack on the move itself, held for the job: the bodies killed come off it, and it lies up past the quarry (or near the house if it gets there) ([INFECTED.md](INFECTED.md#contracts-and-events))* · **One choice** (`lead.horde`): where the settled horde drifts by morning, [the shared block](#one-choice-jobs-and-work) |
| 5 | Trail of Clues | Haven (for a wanderer) | ★ Police: grid references | Follow a note, then a bag, then a third clue to a missing person | Alive (half the time): they join a safehouse. Dead: their last letter, 70% pay | ✅ ***Story job** (key `trail`, per first town): lost as they said (alive half the time); found, with their bitten mother, and staying (alive); taken in by a crew within 5 km of the last town (alive). The neighbour at the first house: [Known · Haven] or a tin and she saw which way (straight to the last town). Found: leave them together (nobody pays, a callback, an Independent asset); crew: the lookout lets the kid go ([Known · Red Hand] or 3 tins) or leave the kid with the crew (5 Red Hand Favor, the Haven pays nothing). Brought home: a callback at the house. Record `trail.fate`* |
| 7 | Barter Chain | Haven, through three wanderers (a wanderer's ask: "Need a hand?") | | Each wants what the next has; the last wants food | 2 Fame, 8 Favor (errands' pay); all three remember you | ✅ *only the food changes hands; started from one of them when nothing else is asked, offered to that player alone ([ERRANDS.md](ERRANDS.md#the-wanderers-asks))* |
| 8 | Distraction | Any faction | | Someone makes noise across the street while someone else searches a guarded cellar for 3 minutes | Fame, Favor, a reward crate. Doable alone with alarm clocks | ✅ |
| 10 | Quiet Count | Halcyon | ★ Lab Tech: more Fame | Count the specials in a town from outside and write it in a notebook | Fame, Favor, +1 research. Too close (50 m; less while you're hard to see): half pay. Wrong: fails | ✅ *the town's own infected are cleared and kept out while it runs, so the only specials there are the ones to count* |
| 11 | Car Keys | Haven, from a Mechanic wanderer ("Ask about cars") | | A car the Mechanic fixed stands in a town 0.5–2.5 km away; its keys lie in one of the 5 houses nearest it that have no lock of their own. Go through them quietly and drive it back to where you met the Mechanic | Fame, Favor, for the one who asked and whoever went with them. Heat 45+ at the house when the keys are picked up: a horde, ×0.7. Hotwired: half | ✅ *a fixable car that already runs (no parts); the keys always lie loose in the right house (on its furniture or floor; just outside its wall if it has no spot), and picking them up is finding them; they go when the job ends. The Mechanic keeps the car (it leaves the world once nobody sits in it). One at a time, one per player per 30 min* |

#6 Water Run and #9 Medicine Before Dark are the errands Water for {house} and Before Dark now
([ERRANDS.md](ERRANDS.md)). Four more designed ideas are covered by existing missions: The Safe Code (The Last Experiment,
Time-Lock Safe), Breaker Sequence (Junction Boxes), Scout the Town (Recon) and Night Lantern
(Lights in the Window).

## Perimeter missions

Jobs at the quarantine line ([DESIGN.md](DESIGN.md), "The Perimeter"). Nothing crosses the line in
either direction; these happen at it. Rare and dangerous on purpose; nothing is built for them. They never appear while the Perimeter
is off or after the cure (the line has stood down), and one that finds nowhere to happen on a map is never offered there ("Missions at the line" in [DESIGN.md](DESIGN.md), "The Perimeter"). Code: `Missions/Perimeter/`.

| # | Mission | From | Needs | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | The Blind Spot | Red Hand | Known, night | Ten minutes after the offer, a 250 m stretch of the line goes uncovered for ten minutes. A Red Hand runner waits with a package 40 m inland of where the land starts: in the kill zone, at the cache it hides there, where the edge runs over land; on the shore where it runs out at sea. Take it to a Red Hand drop 500–700 m further inland before the drones come back | Fame, Favor. The window closing: fails, and anyone still in the kill zone is shot | ✅ *window in real minutes; offered only if it ends before dawn. On a sea edge the stretch is centred on the shore, and where the coast lies further in than the warning zone the runner is past the drones' reach* |
| 2 | The Crossing | Haven | Any, night | A Haven runner dropped a medicine package just outside the kill zone when a drone found her: at the treeline where the edge runs over land, at the waterline where it runs out at sea. Take it out of the warning zone before the drone locks on (45 s with the package), and deliver it to the Haven safehouse in the least infected town | Fame, Favor. Lock-on: the package is shot, fails | ✅ *with no safehouses, a Haven drop in the safest town. Spots in the warning zone come first; where the coast lies further in, the package is at the waterline there, out of the drones' reach, so nothing locks on. **Story job** (key `crossing`, per grid): medicine as they said; forged Bastion line passes (★ Soldier); the runner's drawings of the drones' turns (★ Radio Op). A woodcutter (a fisherman at the waterline) inland of it: [Known · Haven] or 2 tins and he tells you when the drone turns (90 s before it locks on). At the house the runner asks: the house; medicine: let her take it home (nobody pays, a callback, an Independent asset); passes: Bastion's post (8 Bastion Favor); the turns: sell them to the crew (10 Red Hand Favor, +4 Haven Infamy) or burn them (80%). Record `crossing.package`* |
| 3 | Shot at the Line | Any faction | Any | A body lies in the warning zone, just short of the kill zone. Where the edge runs out at sea, they tried it by water: the body has washed up on the shore. Search it without going further; its papers name a stash | Fame, Favor; the stash (a reward crate) | ✅ *the stash sits in the middle of the grid square the papers give and is found within 45 m. Where the coast lies further in than the warning zone, the body is on the shore there* |

## Haven house contracts

Morally grey work around the safehouses and their rules ([LORE.md](LORE.md), "The Haven";
[REPUTATION.md](REPUTATION.md), "Haven house rules and exile"). Nobody in them is a villain.
They need at least one safehouse. Code: `Missions/Haven/RZ_HavenHouseMissions.c`.

| # | Contract | From | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|---|
| 1 | Walk Him Out | Haven | Known, dusk | ★ Nurse: reads the bite; ★ Nurse/Elder: more Fame | A bitten member declared it himself. At dusk he waits at the edge of town. **End it here** or **let him walk on** | End it: Fame. Walk on: 60% Fame; the nearest pack within 2 km takes him (a named young, a walk-out), else town infection +5. Nobody comes in 40 min: he turns | ✅ *he waits rather than walking with you; only a player the job is open to can choose. Done, it can open the questline [The Edge of Town](#the-edge-of-town-the-haven), whose first and fifth stages are this contract; on its own it isn't offered while that runs* |
| 2 | The Ledger | Haven | Known | ★ Elder: more Fame | A member has been taking extra from the stores. **Threaten** them, **pay their debt** (20 Haven Favor), or **tell them to disappear** | Threaten or pay: Fame. Warn off: no pay, +3 Haven Infamy. They remember you | ✅ *only a player the job is open to can choose (checked before the Favor is spent)* |
| 3 | A Quiet Arrangement | Red Hand | Known | ★ Ex-Con: lookouts spot you at 25 m | A safehouse keeper pays the Red Hand to be left alone. Carry the envelope to a Red Hand drop without being seen by two Haven lookouts or a wanderer | Small Red Hand Fame, Favor. Seen: +12 Haven Infamy at once. Never delivered: the safehouse loses 2 people | ✅ *the lookouts are props; the first stage of [The Price of the Door](#the-price-of-the-door-the-haven-or-the-red-hand) is this walk, so it isn't offered on its own while that runs. The drop is outside a Red Hand town when one is in reach* |
| – | Letters (the Courier's job) | Haven, via a Courier wanderer | Any | ★ Hunter: heat limit 45 | Carry letters to a safehouse or quiet town 0.8–4 km away. The quiet way (Silent Courier merged in): through an infested town on the way with the heat where you stand under 35 | Small Haven Fame, Favor: only the one who asked, and anyone with them at the drop (taking the letters off them pays nobody). The quiet way ×1.25 | ✅ *a truth: three times in ten the letter is to the man from himself, said at the door. **Story job** (key `letters`, per destination): letters as they said; to the man from himself (a callback at his door); a crew's red-sealed toll list among them (a Red Hand holding within 6 km). Its ways are the road, the quiet way and the Hunter's quieter one. The woman at the door: who they're for; the crew's: burn the red one (4 Haven Favor, +3 Red Hand Infamy, a Haven asset). Record `letters.post`. A pack that hunts the road at its hour: the courier asks you to see them past it, ×1.15* |

## Leader missions

Personal jobs that Independent and Rogue leaders give **in person**, off their stronghold's books
([REPUTATION.md](REPUTATION.md), "Faction leaders"). Code: `Missions/Leaders/`.

- **Who gets offered what.** The leader's attitude decides who may take it, not your tier, so a
  Rogue leader can hire someone their stronghold hunts. Traits pick the job's tone.
- **Pay.** Independent jobs pay a little Fame and Favor; Rogue jobs pay Favor only, plus any Fame
  with the other faction involved. Rewards never include loot.
- **Rogue jobs can be discovered**, and most rogue deliveries have a second drop at the stronghold
  where you can **turn the leader in** instead.
- **The same world rules as any contract:** a job's requirements, one of each job per leader at a
  time, and Halcyon's jobs stop once the cure is out ([DESIGN.md](DESIGN.md#world-facts)).

| # | Job | From | What you do | Result | Status |
|---|---|---|---|---|---|
| 1 | Our Own First | Haven, Independent | Supplies meant for another safehouse wait in a town. Bring them here, or where they were meant to go | Here: +4 food. There: that one +4 food, half pay | ✅ |
| 2 | Fetch the Family | Haven, Independent | Find a local's family in another town. One of them is bitten | All three: +3 people, infection +5. The two who are well: +2 people, less pay | ✅ |
| 3 | Over the Fence | Haven, Independent | Take bread to a Bastion post for medicine | Bastion Fame +3; infection −3 | ✅ |
| 5 | Ration Run | Haven, Rogue | Three days of the stores go to the Red Hand for antibiotics | Stores −3 food; Red Hand Fame +3; infection −3 | ✅ |
| 6 | Names for Bastion | Haven, Rogue | Deliver a "troublemakers" list to Bastion for protection | Bastion Fame +3; players the Haven exiled take +5 Bastion Infamy | ✅ |
| 7 | Shut the Door | Haven, Rogue | A family at the edge of town asks to come in. Turn them away | Turned away: paid. Let in: +3 people, no pay | ✅ |
| 8 | Hidden Stores | Haven, Rogue | Move the council head's hidden stash before the count (20 min) | Paid. Fail it: the leader is discovered. Turn it in: +5 food | ✅ |
| 9 | Hearts and Minds | Bastion, Independent | Rations to a Haven safehouse, against orders | +4 food; Haven Fame +4 | ✅ |
| 10 | The Real Orders | Bastion, Independent | Recover command's last orders from a dead courier (guarded) | To the commander or to the stronghold, with different loyalty effects | ✅ *the orders say what the season's canon says ("expended", or relief "not forgotten"), the same every time. Not while the [Await Relief](#await-relief-task-force-bastion) questline runs, nor once it has ended* |
| 11 | Training Day | Bastion, Independent | Stand with raw recruits at a town's edge for 4 min (3 waves) | Paid | ✅ *recruits are real only with the survivor group prefab; they turn up when a player is within 300 m. All of them killed: fails* |
| 12 | Local Law | Bastion, Independent | Two families fight over a well. Hear both, then decide | Split: full pay. One side: 80%, the other leaves | ✅ |
| 13 | Rifles for Sale | Bastion, Rogue | Army rifles to a Red Hand buyer | Red Hand Fame +4 | ✅ |
| 14 | Off-Book Requisition | Bastion, Rogue | Collect a "contribution" from a village | All: +6 Haven Infamy, −4 food. Half: 60% pay. Pay it yourself: 15 Bastion Favor | ✅ |
| 15 | Burn the Ledger | Bastion, Rogue | Burn the supply ledger before the audit (15 min) | Burn it: paid. Keep it: the commander is turned in | ✅ |
| 16 | Desertion Route | Bastion, Rogue | Walk two deserters to a safehouse, or march them to the stronghold instead | Paid; they join the safehouse. Marched in: the commander is turned in | ✅ *a safehouse only, not the Blind Spot* |
| 18 | Honour Among Thieves | Red Hand, Independent | Recover what a crew member stole | Take it: paid, loud. Talk it back: 60%. Let him keep it: fails for the crew, a medium callback deed, and he says so later on the road (the callback queue) | ✅ |
| 20 | Debt Collection | Red Hand, Independent | Collect from a debtor (a real wanderer if free): talk, threaten or pay | Talk: 60%. Threaten: full, +3 Haven Infamy. Pay it: 15 Red Hand Favor | ✅ |
| 21 | The Soft Touch | Red Hand, Rogue | Secret medicine to a Haven safehouse | Haven Fame +5; infection −5 | ✅ |
| 23 | Sell Out a Rival Crew | Red Hand, Rogue | A map of another crew's camp, to Bastion | Bastion Fame +4; another crew boss dies (never one a mission or story is using; if a story takes them up before the raid, the camp is found empty) | ✅ |
| 24 | Skim the Toll | Red Hand, Rogue | Take the boss's toll on the road for 5 min | Paid; +3 Haven Infamy | ✅ |
| 26 | Second Opinion | Halcyon, Independent | Notes to a Haven medic | Research +1; Haven Fame +3 | ✅ |
| 27 | Patients Not Samples | Halcyon, Independent | A dying man: sit with him, or take the sample | Sit: Haven Fame +3. Sample: research +1, half pay | ✅ |
| 28 | The Notebook | Halcyon, Independent | A dead researcher's notebook (guarded) | To the field lead: research +1. To the director: research +2, +10 Fame | ✅ |
| 29 | Unsanctioned Trial | Halcyon, Rogue | A willing, dying volunteer and an untested serum | Half the time he lives (research +2), half he turns (research +1, a special) | ✅ |
| 30 | Leak the Data | Halcyon, Rogue | Research data to a Haven safehouse for nothing | Haven Fame +5; infection −3 | ✅ |
| 31 | Private Collection | Halcyon, Rogue | Capture a special alive for the field lead | Paid. Taken to the lab instead: the field lead is turned in, research +3 | ✅ |
| 32 | Sell the Formula | Halcyon, Rogue | The antidote formula to the Red Hand | Red Hand Fame +5 | ✅ |
| – | Rein In | The stronghold (its leaders, in person; Known) | A leader turned Rogue. Read them the terms, or take their counter-offer | Terms: loyalty +25. Counter-offer: Favor, 35% the stronghold finds out (+10 Infamy) | ✅ |

*Simplified:* "in person" means at the leader's seat. Families are one stand-in figure; people
join or leave as numbers. The other side of a trade is a drop point at their nearest seat (within
5 km), else outside a town. Rein In goes up a few minutes after a leader turns Rogue, then at
most hourly, and is offered like any contract.

## Story arc missions

Story arcs ([DESIGN.md](DESIGN.md), "Story arcs and headlines") mostly reuse the missions above:
Lost Colleague (as "The missing courier"), Lab Defence, the supply trucks and the Preacher. Three
missions exist only inside arcs and are never picked at random:

| Mission | Arc | Kind | What you do | Result | Status |
|---|---|---|---|---|---|
| The Deserter's Map | The Deserter's Map | World event; yours (on the Quests tab) once you search the deserter's jacket | Take a map piece from a dead deserter; get the other two from the wanderers it names; reach the bunker (four infected) | The bunker's stores (a reward crate). A holder dies first: fails | ✅ |
| Fever Town: the water | Fever Town | Halcyon contract, Any, ★ Lab Tech | The Water Survey, starting at the sick safehouse's well (not research: it still runs after the cure) | Fame, Favor; the arc goes on | ✅ |
| Fever Town: get them out | Fever Town | Haven contract, Any, ★ Nurse/Doctor: faster | Hold the safehouse gate for 2 minutes (2 waves) | Fame, Favor; two people move, the added infection goes. Fails: two die | ✅ |

## Questlines

Long storylines over many sessions, in stages: each stage offers one mission or waits for time
and the world, and how players did a stage (talk, persuade, sneak, force, bribe, betray, side
with another faction) decides what comes next and how it ends. How the engine works and how to
write one: [DESIGN.md](DESIGN.md#questlines). Code: `Missions/Questlines/<Faction>/`.

- **Who gets offered what.** Each stage mission is a normal contract from the questline's
  faction, offered in person like any other (the radio says who to ask). Anyone who did something
  at a stage is part of the questline from then on: it's on their Quests tab, every later stage is
  theirs without asking again, and they hear when it opens. When it ends, it's a past job in their
  Journal.
- **Pay.** A stage mission pays its Fame and Favor as usual; approaches can add personal standing
  (siding with another faction) or a small reward. The ending's world consequences are shared,
  and its standing goes to everyone who took part.
- **At most 2 at once, one per faction.** A stage that can't happen waits, then fizzles with a
  line of text. The four majors run beside them, outside that limit, and never fizzle
  ([The majors' acts](#the-majors-acts)).

Every questline's stage missions are counted in [Counts](#counts).

### The majors' acts

The four majors' stories, acts, defaults and endings are in [QUESTLINES.md](QUESTLINES.md#the-majors);
how they run in [DESIGN.md](DESIGN.md#the-majors). Their absorbed stages are the built questlines'
own (The Book, Await Relief, The List, The Three Weeks, Authors, below). Every new act is one scene
(`RZ_QM_MajorScene`, `Missions/Questlines/RZ_MajorScene.c`): a contract from the major's faction,
never picked at random, with people spawned only when a player is within 300 m, and a window after
which its vote is taken or its timeout outcome stands. Set pieces hold the evening on the set-piece
calendar. Needs Puzzles > Civilian (or the generic object).

| Major | Scene (act) | Contract | Window | Status |
|---|---|---|---|---|
| The Book Has to Balance | Send Rudi off (1), at Rudi's crossroads | the Red Hand, any standing; 5 Fame, 10 Favor | 1 hour, 16:00–21:00; a 4-minute hold | ✅ |
| | Settling day (4), at the crossroads and Bastion's duty officer | the Red Hand, Known; 8 Fame, 15 Favor | 1 hour, 16:00–21:00; a 4-minute hold | ✅ |
| | The last page (5), at the boss's seat, the boss's table and the lab | the Red Hand, Known; 10 Fame, 20 Favor | 1 hour, 16:00–21:00 | ✅ |
| | Sent off loud (5), at the crossroads | the Red Hand, any standing; 5 Fame, 10 Favor | 1 hour; a 3-minute hold | ✅ |
| | The last line (7), at the crossroads | the Red Hand, any standing, after the cure; no pay | 1 hour, 17:00–22:00 | ✅ |
| Await Relief | Effective Strength (IV), at the CO's radio and the three nearest posts | Bastion, Known; 6 Fame, 15 Favor | 90 minutes | ✅ |
| | The muster (V), at the CO's radio, the quartermaster and the relay | Bastion, Known; 10 Fame, 20 Favor | 90 minutes, 19:00–22:30; a 3-minute hold at the gate or the relay | ✅ |
| | The last call (VI), at the radio and the halt on the north road | Bastion, any standing, after the cure; no pay | 2 hours | ✅ |
| Every Name | The ones who can't work (I), at the edge of the house nearest the stronghold | the Haven, any standing; 5 Fame, 10 Favor | 1 hour, 17:00–21:00 | ✅ |
| | The first sitting (II), at the chair's safehouse | the Haven, any standing; 5 Fame, 10 Favor | 90 minutes at supper; the vote at the end | ✅ |
| | The hungry week (IV), at the hungry house | the Haven, Known; 6 Fame, 15 Favor | 90 minutes; a 4-minute hold | ✅ |
| | The second sitting (V), at the chair's safehouse | the Haven, any standing; 5 Fame, 10 Favor | 90 minutes at supper; a 3-minute hold for the reading; the vote at the end | ✅ |
| | A week on (VI), at the house the rule costs | the Haven, Known; 5 Fame, 10 Favor | 90 minutes | ✅ |
| | The Gate (VII), at the bend on the north road | the Haven, any standing, after the cure; no pay | 90 minutes, 17:00–22:00; a hold | ✅ |
| Proof | Day One (I), at the clinic, the lab, Bastion's and the Haven's strongholds | Halcyon, Known; 10 Fame, 15 Favor | 90 minutes | ✅ |
| | The reading (IV), at the lab | Halcyon, Known; 10 Fame, 20 Favor | 90 minutes, 17:30–20:30 | ✅ |
| | Before the stand (V), at the boss's seat and the lab's records room | Halcyon, any standing; no pay | 90 minutes, offered again until the stand begins | ✅ |
| | The Paper (VI), from the lab gate to the table on the north road | Halcyon, any standing, after the cure; no pay | 90 minutes, from 17:30 | ✅ |

Our Crossroads and Coup Talk stay out of the random pool and Training Day stays a Bastion leader job;
the majors' scenes don't reuse them (simplified: their holds are the scene's own).

### Short Weight (the Haven)

The example questline: four stages, two branches, two endings. Needs a safehouse with at least
3 people, the note prefab and a person to talk to (Puzzles > Note and Civilian, or the generic
object). Text keys: the safehouse's town, the storekeeper, the Red Hand runner and a sick girl,
all picked at the start. Code: `Missions/Questlines/Haven/RZ_Quest_ShortWeight.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Where the food goes** | Haven contract, Any, ★ Police; 40 min; 8 Fame, 10 Favor | At the safehouse: *talk* to the storekeeper (a hint), *persuade* him (difficulty 2; Police, Ex-Con, Elder +1), *bribe*: cover the shortfall (15 Haven Favor, +2 food), or *sneak*: read his ledger nearby (seen if he's within 12 m or you've been noticed: he'll be harder to talk round later). Any of them: you learn he trades tins to the Red Hand for antibiotics for a sick girl | Learned: stage 2. Time up: stage 1 again, once; then it fizzles | ✅ |
| 2 | **The next handover** | Wait | 30 minutes of play or 12 real hours, whichever comes first. The safehouse's council head has a word if you talk to them | Stage 3 | ✅ |
| 3 | **The handover** | Haven contract, Any, ★ Nurse; 45 min; 12 Fame, 12 Favor | 350–600 m outside town: the storekeeper, the runner and the crate. The Haven's branch: *talk*: turn him in; *persuade* him (difficulty 2, 3 if he caught someone reading his ledger; Nurse, Doctor, Elder +1; +3 Haven Fame and 5 Favor on top); *force*: run the runner off (you must be armed; +5 Red Hand Infamy), or kill his lookouts (only with the Red Hand group prefab set); *bribe* the runner (15 Red Hand Favor); *sneak*: take the crate back unseen (no shots within 150 m, not noticed; seen: +4 Red Hand Infamy). The Red Hand's branch: *side with the Red Hand*: warn the runner (+6 Red Hand Fame, +8 Haven Infamy; the contract pays nothing); or nobody comes in time | Stage 4, on its branch | ✅ |
| 4 | **Word gets round** | Wait | 15 minutes of play or 6 real hours | The ending | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Full Weight** | The Haven's | Haven morale +5, Red Hand morale −3, the safehouse +4 food; Red Hand strength −2 if the runner was run off by force; turned in: the storekeeper is put out (the safehouse −1 person). Everyone who took part: +4 Haven Fame. A headline and a rumour (6 hours) | ✅ |
| **Short Weight** | The Red Hand's | Red Hand morale +4, Haven morale −3, the safehouse −3 food; whoever warned the runner: +3 Red Hand Fame. A headline and a rumour (6 hours) | ✅ |

*Simplified:* the storekeeper, the runner and the crate are standing props with talk, choice and
use actions; only the optional lookouts are AI. The sick girl is in the text, not in the world.

### The List (the Haven)

Who crosses names off the Haven's lists, and why ([QUESTLINES.md](QUESTLINES.md#h1-the-list)). One
crossed-out name is the contract The List (Haven #21); this questline opens once that has been done
this season (its council head asks, if they're free to), or once a safehouse has gone hungry or lost
people lately. Needs two safehouses, an Independent or Rogue council head, a farm past the last
village (a small place far from every safehouse, held by nobody) and the note, the object and the
civilian. Text keys: the council head who asks and their house, the chair (the Haven's stronghold
leader), the wife and her husband, the farm. Code:
`Missions/Questlines/Haven/RZ_Quest_TheList.c`; pieces shared by the Haven's questlines in
`RZ_HavenQuestHelp.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Find the husband** | Haven contract, Known, ★ Police (grids on the trail), ★ Radio Op; 60 min; 10 Fame, 10 Favor | His wife at the safehouse; his note in the town he went to, his pack in a second town, and in a third him (half the time: *bring him home*, +1 person) or his last letter. Each place shows its clue once the one before was read. *Bribe*: talk to a Scavenger wanderer with a food item on you, and he sells the last sighting (the end of the trail). ★ Radio Op: talking to the wife gives the same grid (his last call) | Stage 2 (him home is proof at stage 6). Time up: once more, then stage 2 without him | ✅ |
| 2 | **Three copies of the List** | Haven contract, Known, ★ Elder; 90 min; 12 Fame, 12 Favor | The asking house's list is the first copy. At each other safehouse: *talk* to its council head (Merciful: yes; Loyal: only if you're Trusted, or a Known ★ Elder; Paranoid: never; the rest: Known; the chair: no, and now knows someone's asking); *sneak*: read the list in its stores after dark, past two lookouts (within 18 m or noticed: +8 Haven Infamy and "stolen", the copy still counts); *bribe*: leave 3 food for its stores. *Side with the Red Hand*: the nearest crew's tally man sells their list of strays for 15 Red Hand Favor (+3 Red Hand Fame) | Three copies (fewer on a map without the houses): stage 3. Copies are kept, so a retry keeps them. Time up: once more, then stage 3 with what you have | ✅ |
| 3 | **Read the pattern** | Haven contract, Known, ★ Elder/Police (the method spelled out, more Fame); 60 min; 12 Fame, 10 Favor | At the asking house, the copies side by side and its ration notes: the day crossed out in every house, and the portions that went that day, name the farm. Choose one of three places; wrong is 5 minutes' wait; twice wrong, or fewer than three copies, is half the truth ("a farm out that way"). *Talk* to the chair while Trusted: half the story (the farm itself) and a request to stop | Stage 4. Time up: once more, then half the truth | ✅ |
| 4 | **Go and look** | Haven contract, Known; 60 min; 12 Fame, 10 Favor | An empty farm, 3 to 5 infected about. *Look round*: stand 10 s at each of three places (nine graves, ten names). *Leave and report* from the gate. With half the truth the farm is a rough area, not a spot. In a season with the [Long Table](REPUTATION.md#the-long-table) its house is the farm (unless it moved before this began): within 200 m, somebody lives there (`list.found_table`: [The House That Went Quiet](#the-house-that-went-quiet-the-long-table) begins at its guest stage, and the family has noticed you); if the family has gone, 60 m finds the long table left laid, or burnt | Stage 5, either way (after two tries without anyone going) | ✅ *at the Long Table's farm there's nothing to look round: getting there is the finding, and the meal is the house's own* |
| 5 | **Someone's been asking** | Wait | 2 hours of play; a rumour and a quiet headline. The chair has a word for anyone who took part | Stage 6 | ✅ |
| 6 | **The chair's decision** | Haven contract, Known, ★ Elder; 60 min; 10 Fame, 10 Favor; in the chair's voice | The copies on a table at the chair's seat. *Read it out* (Trusted, a ★ Elder who is Known, or Known with the husband home; Known once B2's `census.refused` is set), then hold the gate on the road in for 3 minutes while the bells bring two waves (the reader: +10 Haven Fame, 30 Favor). *Give the copies to the chair* (+5 Haven Fame, 20 Favor). *Side with Bastion*: carry them to the nearest Bastion seat within 6 km (+10 Bastion Fame and +15 Haven Infamy, then +5 Bastion Fame and 30 Favor; exiles online take +5 Bastion Infamy, as Names for Bastion). *Side with the Red Hand*: sell them to the nearest crew (+10 Red Hand Fame, 40 Favor; one time in five the houses find out: +15 Haven Infamy); with stage 2's Red Hand copy, a runner meets you just outside town. Either road passes two of the house's lookouts (one with the Red Hand copy): seen within 35 m, or by a Haven wanderer, and the copies are taken back (+8 Haven Infamy) | The ending | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **The names read** | Read out | Haven morale −8; everyone who took part +10 Haven Fame, and the council head who asked remembers them as having saved them; the council head whose house crossed out the most names leaves the chair (if it's still the same person); `list.read` (the Long Table's suspicion +30). A headline and a rumour (8 hours) | ✅ |
| **Buried** | The copies to the chair, taken back by the lookouts, or nobody chose | Everyone who took part +5 Haven Fame, and the chair remembers them as having helped; `list.buried`. A quiet headline and a rumour | ✅ |
| **The census** | To Bastion | Haven morale −10; `list.to_bastion` (B2 starts at its stage 3). A headline and a rumour | ✅ |
| **Sold** | To the Red Hand | `list.to_redhand` (R2); found out: `list.sold_known` and a headline, otherwise a quiet one. A rumour | ✅ *the exile-leak chance isn't raised: `list.to_redhand` is published for it* |

*Simplified:* the council heads and the chair are the real leaders, talked to as usual; the wife,
the husband, the stores, the lookouts, the tally man and the copies are props, and the farm's
people are only in the text (at the Long Table's farm, the family are the house's own people).
Choices the design marks *(you)* are kept as season-wide flags and in who took part, not per life.

### The Price of the Door (the Haven, or the Red Hand)

A safehouse pays the Red Hand to be left alone, and this month the price is a man
([QUESTLINES.md](QUESTLINES.md#h2-the-price-of-the-door)). Stage 1 is A Quiet Arrangement's walk
(Haven house contract 3), with its lookouts. Needs a safehouse under pressure (hungry, or it has
lost people lately; not the chair's house, not one at the Preacher's prayers) with a Red Hand crew
within 6 km, and the note, the object and the civilian. Half the time it's the Haven's version (its
council head, if Independent or Rogue, asks), else the Red Hand's (the crew boss asks): they differ
in who gives stages 1, 2 and the tins. Text keys: the house, its council head, the crew boss, the
exile and the town by his barn. Code: `Missions/Questlines/Haven/RZ_Quest_PriceOfTheDoor.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Carry the envelope** | The version's contract (Haven or Red Hand), Known, ★ Ex-Con; 45 min; 8 Fame, 15 Favor | Under the loose board by the door: *take it sealed*, *sneak*: open it first (the exile's name and grid, `door.knows`), or *leave it* (refused, no pay). Two of the house's lookouts on the way to the crew's drop (150–250 m outside the crew boss's seat): within 35 m (25 m ★ Ex-Con), or a Haven wanderer near, +12 Haven Infamy. Only the carrier hands it over | Delivered (`door.paid`): stage 2. Refused or time up: stage 3 | ✅ |
| 2 | **The man at the grid** | The version's contract, Known; 60 min; 12 Fame, 12 Favor | The exile in a barn outside a town 0.8–3 km away, an escort (a figure, or the survivor group). *Talk*: warn him (he takes to the roads as a new wanderer if the roster has room; `door.exile_warned`). *Talk*: bring him back to plead (he follows you to the house; `door.exile_home`). *Betray*: walk him to the crew's drop past two lookouts (seen within 35 m, or a Haven wanderer near him: +15 Haven Infamy; `door.exile_sold`). The Haven pays for warning him or bringing him home, the Red Hand for walking him over; the other side's choices pay nothing (warning him for the Red Hand: +3 Haven Fame, +5 Red Hand Infamy; walking him over for the Haven: +5 Red Hand Fame). Time up: the crew found him | The talk (stage 4) | ✅ |
| 3 | **The crew comes to collect** | Haven contract, Known, ★ Ex-Con; 50 min; 15 Fame, 15 Favor | The crew boss at the gate on the road in (70–90 m out). *Bribe*: 40 Haven Favor or 25 Red Hand Favor. *Persuade* the Red Hand (difficulty 2, Ex-Con +1): the debt is yours (`door.debt_yours`). *Force* (only with the Red Hand group prefab): tell him no, and a raid comes a minute later; hold the gate 3 minutes or kill the raiders. *Side with Bastion*: carry the house's bread (−2 food) to the nearest Bastion seat within 6 km for a patrol (+4 Bastion Fame). Time up: the crew takes two people (never below two) | Stage 4 | ✅ |
| 4 | **A day of talk**, then **the vote** | Wait, 2 hours of play; then Haven contract, Known, ★ Elder; 45 min; 8 Fame, 8 Favor | At the house: *talk*: speak for the council head (Known), or *persuade*: speak against them (the Haven, difficulty 2, Elder +1): the cook takes the chair. With the exile home, speak for him first (the same check): he's let back in, +1 person. Skipped if the council head left the chair during the wait. Time up: they keep the chair | Stage 5 (or the ending) | ✅ |
| 5 | **The house stands, or pays** | By how the month went | Held, or Bastion's patrol: the contract Fortify the Safehouse at this house (Known, 15 Fame, 15 Favor; the town is Barricaded); held with R3's `well.stood`, no barricades: straight to the ending. Paid, bought off, collected or sold: the envelope walk again, with tins. The debt: the first instalment near the crew boss's seat (Red Hand contract, anyone not Hunted; 6 Fame, 10 Favor): fight in the pit (hold the ring 200 s through 4 waves, as House Rules) or pay 20 Red Hand Favor | The ending. Time up: once more, then Paid in tins | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Stands alone** | The gate held | Everyone who took part +10 Haven Fame, +10 Red Hand Infamy, and the crew boss remembers them as having crossed him; the council head leaves the chair if the vote went that way; `door.shut`, `door.house`. A headline and a rumour | ✅ |
| **Under Bastion** | Bastion's patrol | Everyone +10 Bastion Fame, +8 Haven Fame; the council head as above; `door.bastion`, `door.shut`, `door.house` (which house). A headline and a rumour | ✅ *Bastion's road posts aren't more likely near the town: the flags are published instead* |
| **Paid in tins** | Paid, bought off, collected, sold; every fallback | Everyone +5 Red Hand Fame, +5 Haven Fame (whoever carried the tins: +5 Red Hand Fame, 15 Favor more); the house −4 food; `door.tins`. A quiet headline and a rumour | ✅ *a one-off −4 food stands in for "1 food every second upkeep, never raided by that crew"* |
| **The debt is yours** | The debt | Everyone +10 Red Hand Fame, +10 Haven Fame (whoever paid the instalment: +5 Haven Fame more); `door.debt_yours` (R1). A quiet headline and a rumour | ✅ |

*Simplified:* the crew boss at the gate is a stand-in figure (the real one stays at his seat); the
envelope, the tins and the bread are carried by a player, not as items. Bastion's checkpoint "for
the night" is a line of text. "The cook takes the chair" is in words: the seat is filled by the
leaders' usual successor. Walked to the crew in a season with a Long Table, the exile is one of the
people who never arrive (`door.exile_sold`, read by the Long Table and R2).

### The Edge of Town (the Haven)

The walks at the edge of town against the lab's trial dose
([QUESTLINES.md](QUESTLINES.md#h3-the-edge-of-town)). Stages 1 and 5 ("the walks stay") are the
contract Walk Him Out at the walker's house; the lab road is the contract Cold Chain. Starts after
anyone has finished Walk Him Out this season, with the lab there and the cure not out. Needs a
safehouse of four or more (the walker's), another for the nurse (one with a medic if there is one;
else the same house), and the note, the object and the civilian. Text keys: the walker and his
house, the nurse and hers, the bitten man, the research director. Code:
`Missions/Questlines/Haven/RZ_Quest_EdgeOfTown.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **One more walk** | Haven contract (Walk Him Out), Known, ★ Nurse/Elder; dusk only; 40 min; 15 Fame, 15 Favor | *End it* or *let him walk on*, as in the contract. Or take him to the lab for the trial dose: he follows you (an escort); at the lab, 8 in 10 he walks out and goes home (+1 person, `walk.lab_saved`), else Halcyon writes him down (half pay) | Stage 2. Nobody came: another bite, another dusk, once | ✅ |
| 2 | **The walker's tally** | Haven contract, Known, ★ Elder; 60 min; 10 Fame, 10 Favor | At the edge of the walker's town: three graves (5 s at each) and the tally. *Talk*: ask him for the names. *Sneak*: read his notebook by the graves while he's on watch (more than 15 m from him, not noticed; seen: he takes it back and won't talk to you). ★ Elder: he asks you to walk the next one with him | Stage 3. Time up: once more, then on | ✅ |
| 3 | **The lab's numbers** | Haven contract, Known, ★ Lab Tech/Doctor; 60 min; 12 Fame, 10 Favor | At the lab: *persuade* a night-shift field lead (Halcyon, difficulty 2; Lab Tech, Doctor +1), *sneak*: the trial records after dark (noticed, or shots near: +8 Halcyon Infamy), or *bribe* a courier (20 Halcyon Favor). Any: `walk.knows_other_two` (HC2 opens at its stage 2) | Stage 4. Time up: once more, then on without | ✅ |
| 4 | **Two offers** | Wait, 2 hours of play; then Haven contract, Known, ★ Nurse/Doctor; 60 min; 10 Fame, 10 Favor | Radio Operators who took part hear the director's offer on the secure channel; everyone hears both from the nurse. *Persuade*: broker the lab road (Known with Halcyon, and the Haven at difficulty 2, or 1 after a man was saved at stage 1, 2 again with HC1's `weeks.public`; Nurse, Doctor +1; only with the mission item and the Haven drop point). *Talk*: refuse both. *Side with the Red Hand*: the cut dose (Trusted with the Red Hand; +5 Red Hand Fame; the Haven pays nothing). Time up: the walks stay | Stage 5 on that branch | ✅ |
| 5 | **Make it real** | By the branch | The lab road: Cold Chain from the lab to the nurse's house (Halcyon, Known; 12 Fame, 15 Favor, and +10 Haven Fame; no cure threshold). The walks stay: Walk Him Out again, the walker's last walk, at dusk. The cut dose: carry a crate from the nearest Red Hand seat to the nurse's house (Red Hand contract, Known, ★ Ex-Con; 6 Fame, 10 Favor, and +6 Red Hand Fame, 20 Favor for the carrier), past Halcyon's courier (within 35 m, 25 m ★ Ex-Con: +8 Halcyon Infamy) | The ending; the cut dose: stage 6. Failed twice: The walks stay | ✅ |
| 6 | **A session later** | Wait (the cut dose only) | 2 hours of play | The cut dose | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **The lab road** | Lab road | Everyone who took part +5 Haven Fame, +5 Halcyon Fame; research +2; Haven morale −4; a headline names the first of the other two (numbers them with HC2's `other_two.numbers`); a Haven–Halcyon truce if the usual rules allow (`walk.truce`); `walk.lab_road`. A rumour | ✅ |
| **The walks stay** | Both refused; every fallback | Everyone +10 Haven Fame; Halcyon morale −3; `walk.walks_stay`. A quiet headline names the walker, and a rumour | ✅ *the walker and the nurse are in the text, not people the world keeps* |
| **The cut dose** | The cut dose | Everyone who sided with the Red Hand +10 Haven Infamy; the walker's house −2 people (never below two); `walk.cut_dose` (R1). A headline and a rumour | ✅ |

*Simplified:* the walker, the nurse, the field lead, the courier and the man at the edge of town are
standing figures; the man taken to the lab is an escort (data, a figure, or the survivor group). The
director's offer reaches Radio Operators as a message. Dusk is 16:30 to 21:00 game time, as for Walk
Him Out (any time if the clock isn't known).

### Await Relief (Task Force Bastion)

Five stages, four branches, four endings ([QUESTLINES.md](QUESTLINES.md#b1-await-relief), B1). Needs
an Independent Bastion outpost commander, Bastion's stronghold leader (the commanding officer), the
note, a person to talk to (Puzzles > Civilian, or the generic object) and the mission item (the
page, the crate). Not after the cure. It starts on the usual schedule with any Independent
commander (the one R1's `book.bastion` names, if he's still in the chair), or straight from No
Reply's tape at that contract's post. Text keys: the commander, his post, his operator, the runner,
the commanding officer and the stronghold. Code: `Missions/Questlines/Bastion/RZ_Quest_AwaitRelief.c`,
which also holds what the three Bastion lines share.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **The operator's tape** | Bastion contract, Known, in person from the commander (word reaches Bastion's Known players that he wants a word, not on the radio, as a Radio-tab line; the job is theirs when they take it from his job replies), only after the nightly call (at night); ★ Radio Op; 50 min; 12 Fame, 15 Favor | At the post: *talk* to the operator and take his tape, or *sneak*: copy it onto your own set while he isn't looking (a radio on you; no shots within 60 m and not noticed; seen, you take it openly). On the hill 0.7–1.6 km away (the radio mast, else the generic object): play it into the relay and stay within 30 m for 3 min (heat; a horde after the first minute; ★ Radio Op: twice as fast). The relay's log names the runner | Stage 2. Time up: once more; then, or with the commander gone, it fizzles | ✅ |
| 2 | **The runner** | Bastion contract, Known, in the commander's voice (nobody's once he's gone); 60 min; 15 Fame, 15 Favor | A step chain through three towns 0.7–2.5 km apart: his satchel, his fire (2 infected), his body (5 infected, one a special). Reading the page on him ends it | Stage 3. Time up: once more | ✅ |
| 3 | **Who reads it** | Bastion contract, Known; 60 min; 10 Fame, 10 Favor | The page (the mission item) is on the body. *Talk:* to the duty officer at the stronghold (Trusted, or ★ Soldier): the stove, +8 Bastion Fame and 30 Favor on top; or to the commander, held for the stage and standing in at his post: he reads it to his men. *Side with the Red Hand:* a crew boss's radio man at the nearest Red Hand seat within 6 km (else a Red Hand meeting point; not if the Red Hand is hostile to you): +5 Red Hand Fame, 40 Red Hand Favor, +8 Bastion Infamy, no pay. *Side with the Haven:* the chair's clerk at the Haven stronghold: +5 Haven Fame, no pay | Stage 4 on that branch (`orders.stove`, `.read`, `.mocked`, `.haven`). Lost or time up: once more | ✅ |
| 4 | **Word gets round** | Wait | 2 hours of play or 12 real hours. Read: a headline, two men walked off the post. Open channel: a headline, "expended" on the open channel every hour | Stage 5. The commander gone (read), or no copy of the tape (stove): the ending at once | ✅ |
| 5 | **The reckoning** | Bastion contract, Known, from the commanding officer (the Haven's branch: from the commander) | *Read* (★ Soldier): the commander is held and stands at his post as an escort, the squad's sergeant beside him; the Bastion squad prefab is the squad, if set. *Force:* kill the squad (+15 Bastion Infamy each; no pay). *Talk:* take him in yourself: he walks with you to the stronghold (killed on the way, the commander dies). *Persuade:* talk the commanding officer down on the sergeant's radio (difficulty 3: Champion, or Trusted and ★ Soldier). Stand aside (half pay), or nobody comes: they take him. *Open channel:* the operator hides outside a town 1–3 km away: bring him in (paid), or *side with the Haven*: walk him to a safehouse (+1 person, +5 Haven Fame, no pay). *Stove*, only if someone copied the tape: whoever did can give the copy to the commander, a crew or the chair (no pay), or the stove too. *The Haven:* a ration crate from the commander's post (else the stronghold) to the chair's clerk: +4 food at her house | The ending: the branch's, or the copy's | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Into the stove** | Stove | Everyone who took part: +10 Bastion Fame, and the commanding officer remembers them as having helped. Bastion morale +5; the commander who sent you is relieved and replaced (a headline); a rumour (8 hours) | ✅ |
| **The men were told** | Read, or the copy to the commander | Everyone: +8 Bastion Fame; the two who walked off join the nearest safehouse (+2 people; everyone +8 Haven Fame). Bastion morale −8. Fought for: the commander turns Rogue (Independent if another Bastion leader already is) and keeps his post, the usual Rein In follows, and whoever fought takes +5 more Bastion Infamy. Talked down: loyalty +25. Arrested, taken or the copy read: relieved and replaced | ✅ |
| **On the open channel** | Open channel, or the copy to a crew | Whoever sided with the Red Hand: +10 Red Hand Fame, +12 Bastion Infamy. Bastion morale −10; a Bastion–Red Hand feud, under the usual rules; a rumour (12 hours) | ✅ |
| **The Haven's leverage** | The Haven, or the copy to the chair | Everyone: +10 Haven Fame, +5 Bastion Fame. Haven morale +5; Requisition never comes to the chair's safehouse this season, nor The Census's contribution (`orders.haven`); a rumour (10 hours) | ✅ |

*Simplified:* the tape is the operator's word, not an inventory item. The duty officer, the
commander, the radio man, the clerk and the operator are standing props; only the squad and the
escorts are AI. A relieved commander's successor gets a loyalty like any replacement, not always
Loyal. The two who walk off join a safehouse rather than becoming Deserter wanderers. Bastion's
open channel isn't silenced: that's a headline and a rumour. While it runs, The Real Orders isn't
offered and No Reply picks another post than its commander's.

### The Census (Task Force Bastion)

The six designed stages (six contracts and a wait), a start that can skip ahead, and four endings
([QUESTLINES.md](QUESTLINES.md#b2-the-census), B2). The commanding officer's contracts, starting
while Bastion needs supplies or security (any time with faction life off). Needs Bastion's
stronghold leader, two safehouses, a person to talk to and the mission item (the crates, the sack).
Not after the cure. Two houses carry the line: the first counted (a random one with 2+ people, or
the one H2 put on Bastion's census: `door.bastion`, with `door.house` naming it) and the nearest other.
Text keys: the two towns, the commanding officer, the stronghold, the store. Code:
`Missions/Questlines/Bastion/RZ_Quest_Census.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 0 | **The commanding officer's count** | No wait | — | Stage 1; with H1's `list.to_bastion`, stage 3 (both houses counted already) | ✅ |
| 1 | **Count a safehouse** | Bastion contract, Known, ★ Police; 40 min; 10 Fame, 10 Favor | At the first house: *talk*: ask the council head for an honest count (Known with the Haven; a house on Bastion's census hands over its own list). *Sneak*: 45 s at a watch point 70–110 m out, then write it down; noticed (or a shot within 140 m): +3 Haven Infamy and no count. ★ Police: exact; otherwise a person or two out | Stage 2 (`census.counted`). Time up: once more | ✅ |
| 2 | **Ration cards** | Bastion contract, Known; 60 min; 12 Fame, 12 Favor | Two crates (the mission item) at the Bastion seat nearest the first house, one to each house's storekeeper: +2 food each. *Betray*: skim one at the store (15 Bastion Favor; 25% the quartermaster notices, +10 Bastion Infamy; one house goes without). *Talk*: give 1–2 food items from your own pack at a house (+1 food there, +3 Haven Fame, once per player) | Stage 3. Twice lost: stage 3 without the rations | ✅ |
| 3 | **The curfew** | Bastion contract, Known, at night (from 10 min before), until dawn; ★ Police: 1.5× faster | Four patrol points in the first house's town (the second's, if the first has emptied), 15 s each. At the notice: *force*: enforce it (+3 Haven Infamy each), or *talk*: patrol gently (★ Police, or Known with the Haven; half pay). At the edge of town: *betray*: let a Red Hand toll in for the night (Known with the Red Hand; +6 Red Hand Fame; no pay): the crew (with the Red Hand group prefab) holds the road until dawn and fights strangers within 80 m | Stage 4 (the second house if the first is the chair's after `orders.haven`, or empty; neither: stage 5). Unwalked: once more the next night | ✅ |
| 4 | **The contribution** | Bastion contract, Known, 🔒 not Elder; 50 min; 12 Fame, 15 Favor | At the house's storekeeper: collect four food (the house −4 at once) and carry the sack to Bastion's clerk at the nearest Bastion seat (paid), or *side with the Haven*: to the other house's storekeeper (+4 food there, +4 Haven Fame, +5 Bastion Infamy, no pay). *Bribe*: pay it yourself (15 Bastion Favor). *Betray*: falsify the ledger (Trusted, or ★ Police) | Stage 5 | ✅ |
| 5 | **The Red Hand hears**, then **the raid** | Wait (90 min of play or 12 real hours), then a Bastion contract, Known; 30 min; 15 Fame, 15 Favor | Only with the Red Hand group prefab, at the counted house with the fuller stores. The crew appears 250–350 m out when someone comes within 400 m, and walks in after 45 s (it spares Red Hand friends). *Force*: kill it (+5 Red Hand Infamy). *Side with the Red Hand*: hold the stores for them, 90 s within 20 m (Known with the Red Hand; +5 Red Hand Fame, 30 Red Hand Favor, +5 Bastion Infamy; no pay). Nobody stops them: 90 s of loading. Raided: the house −3 food | Sided: the Red Hand's count. Otherwise stage 6; no prefab: straight to stage 6 | ✅ |
| 6 | **The nightly call** | Bastion contract, Known, offered from 19:00, until 22:30 | At the radio by the commanding officer's seat (the generic object, with the table radio from guidance if set), 20:30 to 22:30: read the count *as it is* (+5 Bastion Fame and 30 Favor on top), *high* or *low* (betray: 25% found out, and then no pay), or *side with the Haven*: tear it up (+5 Haven Fame, no pay). A falsified ledger is found out here 25% of the time: the house pays its 4 food after all, and the falsifier (if online) takes +10 Bastion Infamy. Command doesn't answer | The ending. Nobody at the radio: once more | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Counted** | As it is | Everyone who took part: +10 Bastion Fame, +5 Haven Infamy. Both houses +2 food; Haven morale −5. Headline: "Bastion read [the number] names to Command last night. No reply." A rumour (8 hours) | ✅ |
| **Forged** | High or low | Everyone: +10 Haven Fame, and +10 Bastion Fame (found out: +15 Bastion Infamy instead). High: both houses +2 food. Low: Requisition never comes to them this season (`census.spared`) | ✅ |
| **Refused** | Torn up | Everyone: +10 Haven Fame, +10 Bastion Infamy. Requisition comes to those houses first, and ×1.2 as often (`census.pressed`); `census.refused` | ✅ |
| **The Red Hand's count** | Sided with the raid | Whoever sided: +10 Red Hand Fame, +15 Bastion Infamy, +10 Haven Infamy. Red Hand strength +3; a headline | ✅ |

*Simplified:* the people (council heads, storekeepers, the clerk, the toll-man) are standing props;
only the toll crew and the raiders are AI. Counted houses get their food at once, not off a truck.
"Civilians sheltered" is the people in the two counted houses.

### The Last Pilot (Task Force Bastion)

Five stages, four branches, four endings ([QUESTLINES.md](QUESTLINES.md#b3-the-last-pilot), B3). It
starts when someone finishes Last Helicopter (at once if Bastion's questline
slot is free). Needs Bastion still flying (stage presence 0.2 or more), its stronghold leader, the
Perimeter with a spot for the wreck, a person to talk to and the mission item (the black box, the
Red Hand's fuel). Not after the cure, not while a Last Helicopter is out or Rescue the Pilot runs,
and never once its pilot has died. The pad is 120–350 m from Bastion's stronghold seat. Text keys:
the pilot, his Day 3 friend, the commanding officer, the stronghold, the far LZ. Code:
`Missions/Questlines/Bastion/RZ_Quest_LastPilot.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Fuel** | Bastion contract, Known, ★ Mechanic; 50 min; 10 Fame, 10 Favor | At the pad, the pilot and the fuel point: pour in two jerrycans (a Mechanic finds the leak: one). At a Red Hand depot by their nearest seat within 5 km (the fuel depot prefab as its look, if set): *sneak*: siphon it (unseen: no shots within 80 m, not noticed; seen: +8 Red Hand Infamy) and carry it back to the pad; *bribe*: buy it from the fence (25 Red Hand Favor; the crew delivers it, and the boss hears Bastion is buying) | Stage 2. Time up: once more | ✅ |
| 2 | **Whose flight** | Bastion contract, Known, no storm; 12 min; 15 Fame, 15 Favor | Lift-off in 12 minutes; it lands 3 minutes before (a horde, then heat). *Talk*: choose the load at the pilot (the first choice stands): Halcyon's samples (+6 Halcyon Fame for you, research +1), the Haven's wounded (+6 Haven Fame, +2 people at the nearest safehouse) or Bastion's ammunition (+6 Bastion Fame, Bastion strength +2). Nobody chose: ammunition. Be within 40 m at lift-off. Fuel bought from the Red Hand: a Red Hand crew comes up the road as it lands (with the group prefab) | Stage 3 (`pilot.flew_for`: the faction it flew for). Nobody at the LZ: once more | ✅ |
| 3 | **{friend}'s wreck** | Bastion contract, Known, ★ Mechanic, ★ Radio Op; 60 min; 15 Fame, 15 Favor | The Day 3 wreck in the warning zone, 30–60 m short of the kill zone and 500 m from any road checkpoint (the helicopter wreck prefab, else the generic object; 3 infected). Stay within 25 m for 60 s (★ Mechanic: twice as fast) and the black box (the mission item) comes out; a ★ Radio Operator there plays it. Carry it to the pilot: if it was played, he hears it | Stage 4. Time up or lost: once more | ✅ |
| 4 | **The pilot decides** | Bastion contract, Known, ★ Soldier; 60 min; 10 Fame, 10 Favor | At the pad: *persuade*: talk him down (difficulty 2, 1 if he heard the box; ★ Soldier +1): +10 Bastion Fame and 30 Favor on top; or *betray*: help him, two jerrycans (★ Mechanic: one) to a far LZ past the warning zone, at least 1.2 km from the pad (only if the map has one; no pay). At the stronghold: *talk*: report him to the duty officer. At the nearest Red Hand seat: *side with the Red Hand*: sell the plan to a fixer (Trusted with the Red Hand; +5 Red Hand Fame, 40 Red Hand Favor, +10 Bastion Infamy; no pay) | Stage 5 on that branch. Nobody decides: once more, then Grounded | ✅ |
| 5 | **The last of it** | By branch | *Stays*: one more 12-minute run from the pad, kept clear (paid). *Grounded*: walk him from the pad to the stronghold (an escort; paid). *The line*: keep the far LZ clear for its 12 minutes while he lifts off (no pay). *The Red Hand*: a Red Hand contract (Known with the Red Hand; 10 Fame, 10 Favor): keep watch within 25 m of the pad for 90 s while a crew (the group prefab, if set; it spares friends) does the rest; a dead crew can't burn it | The ending. Stays, unkept twice: He stays anyway. The line, unkept: Grounded. The Red Hand, failed twice: it fizzles | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **He stays** | Stays | Everyone who took part: +10 Bastion Fame. Bastion morale +5; after the extra run, Last Helicopter runs twice as often for 2 hours; a rumour (8 hours) | ✅ |
| **Grounded** | Grounded, or the line unkept | Everyone: +10 Bastion Fame. No Last Helicopter or Rescue the Pilot for 2 real hours (`pilot.down_until`); the next Last Helicopter names the new pilot | ✅ |
| **He flew for the line** | The line | Nothing. No helicopter for 2 real days, until Bastion's mechanics patch up a grounded airframe; Bastion morale −10; Bastion's Crash Site starts at his wreck (with the wreck prefab) in the warning zone nearest the far LZ: the line's truth, his name in the brief. Headline: "Bastion's last pilot flew for the line [at dawn]. The line shot him down short of it, like the one before." | ✅ |
| **Burnt for the Red Hand** | The Red Hand | Whoever sided: +10 Red Hand Fame, +15 Bastion Infamy. No helicopter for 2 real days; Bastion morale −10, Red Hand morale +5; a Bastion–Red Hand feud; a headline | ✅ |

*Simplified:* there's no helicopter prop, as in Last Helicopter; the pilot, the duty officer, the
fence and the fixer are standing props. The groundings count real time times **Wait scale**, and
"twice as often" isn't saved across a restart. Last Helicopter waits while the story has the
helicopter (fuel, its flight, the last of it); Rescue the Pilot waits while the questline runs.

### The Three Weeks (Halcyon)

HC1 in [QUESTLINES.md](QUESTLINES.md#hc1-the-three-weeks): what the lab did with the three weeks
before the gates closed. It replaces the old contract of the same name (its remote site and keypad
store are stage 2 now). It starts once Patient Zero or Patient Zero's Ledger has been solved this
season, with an Independent or Rogue field lead (she voices and pays its contracts while she's
still in her chair), a living Scavenger wanderer (the driver) and the season's closed site, the
world fact `origin.site` that Patient Zero's records point to (a remote spot on high ground between
two towns, well clear of every settlement and the lab, at a mast, factory or barracks there if there
is one; on a small map like Arland, its fallback outside the town farthest from the lab;
[DESIGN.md](DESIGN.md#world-facts)). Needs a lab, the note, a civilian and the puzzle object. Code: `Missions/Questlines/Halcyon/RZ_Quest_ThreeWeeks.c`; the Halcyon lines share a scene base,
`RZ_QuestHalcyonScene.c` (people at several places, each put down only when a player comes near).

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **The driver** | Halcyon contract, Known, ★ Radio Op; 60 min; 10 Fame, 10 Favor | The driver (held by the mission) sits at the edge of the town nearest the site. *Talk*: needs Known with the Haven and below Trusted with Halcyon, or a Radio Operator (he heard you on the net on Day 3); share food twice (one food item each time, anyone's; he remembers who); *force*: threaten him, armed (+3 Haven Infamy, he remembers). He names the site: "She stayed because she wanted to." | Stage 2. Time up: stage 1 again, once | ✅ |
| 2 | **The site** | Halcyon contract, Known, ★ Lab Tech; 60 min; 15 Fame, 15 Favor | Three of her team lie 30–110 m around the site, each with two digits of the store's six; an infected by each, three by the store. The store's terminal (or keypad) takes the code; wrong, a flat buzz, and the third wrong, dead for 2 minutes and loud (a horde). Without either prefab it's a combination lock that opens for whoever has read all three pages. Lab Tech: the door's text gives the first digit | Stage 3. Time up: again, once | ✅ |
| 3 | **The team lead** | Halcyon contract, Known; 60 min; 15 Fame, 15 Favor | Dr Wolff is a special in the store. *Force*: kill her (mercy: the notebook is in her coat), or catch her alive (`RZ_LiveCapture`: the cage outside, then the case to the lab drop before it wakes; research +2). *Sneak*: take the notebook unseen (no shots within 60 m, not noticed) and leave her. Caught, then lost on the way: she's loose, the notebook is safe. No room for a capture (or no mission item or lab drop): no cage | Stage 4. Time up: again, once | ✅ |
| 4 | **Who reads it** | Halcyon contract, Known, ★ Lab Tech/Doctor; 90 min; 12 Fame, 20 Favor | People at the seats that exist: *persuade* the duty officer at the lab for the director (difficulty 2: Trusted, or Known as a Lab Tech or Doctor; +8 Fame and 10 Favor on top); *talk*: the field lead at her station (held while it runs; +3 Halcyon Fame, +10 Halcyon Infamy); *side with* Bastion (its adjutant: +20 Bastion Fame, +15 Halcyon Infamy), the Haven (its council clerk: +15 Haven Fame, +10 Halcyon Infamy) or the Red Hand (the boss's man: +15 Red Hand Fame, 40 Favor, +10 Halcyon Infamy). Siding: the contract pays nothing | The director: in the record. The field lead, the Haven, the Red Hand: public. Bastion: Bastion's file. Time up: again, once | ✅ |
| 5 | **The lab answers** | Wait | 2 hours of play or a real day, and not during a final stand | Public: stage 6 (after the cure, the ending). In the record: stage 7 if the field lead is still in her chair, else the ending. Bastion: the ending | ✅ |
| 6 | **The raid** | Halcyon contract, Known; 30 min; 20 Fame, 20 Favor | Lab Defence: hold the lab 4 minutes against waves (Red Hand raiders too, with their prefab). Not during a final stand | The ending, held or not | ✅ |
| 7 | **The field lead walks out** | Halcyon contract, Known; 60 min; 10 Fame, 10 Favor | She (held) follows whoever answers her (an escort) to the nearest safehouse or back to the lab. With no room for another escort she goes on her own. Her escort body killed: she's dead | The ending | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **In the record** | The director's | Halcyon morale +3, research +1; everyone who took part +4 Halcyon Fame; the field lead leaves her station (replaced; walked to a safehouse: +1 person there); a rumour (8 hours); `weeks.record`, `weeks.lead_left` | ✅ |
| **Out on the open channel** | Public | Halcyon morale −10; a headline by who read it (and a quiet one if the raid wasn't held); the rumour "Halcyon made this" for 48 hours (the engine's longest); `weeks.public` (H3's lab road is harder) | ✅ |
| **Bastion's file** | Bastion's | Bastion strength +3; a headline and a rumour (12 hours); `weeks.bastion` (Authors starts with Bastion's terms on the table) | ✅ |
| *(any)* **She's at the lab** | caught at stage 3 | Research +2 at stage 3; a headline: the director spoke about her for two minutes on the open channel; `weeks.captured` | ✅ |

*Simplified:* the notebook is carried as data, not an item: anyone who was part of the story can
hand it over at stage 4. The people at the seats are stand-ins (the duty officer, the adjutant,
the council clerk, the boss's man), never the stronghold leaders' own bodies; the field lead's
stand-in replaces her body while a stage needs her. "Fame +15 with whoever you gave it to" and the
like are paid at once to whoever chose, within the engine's caps; the endings' shared standing is
small. The pages about the director going in the stove is text.

### The Other Two (Halcyon)

HC2 in [QUESTLINES.md](QUESTLINES.md#hc2-the-other-two): what happens to the two in ten who don't
walk out of the trial dose. It replaces The Author List contract (its deduction is stage 5 now).
The sister and her brother are generated, at a safehouse with at least 3 people within 3.5 km of
a Halcyon field station when one is that close (the back room is at the station). Its stages are
the sister's (Haven) contracts, offered to anyone Known with the Haven or with Halcyon and Hunted
by neither. It doesn't start after the cure; with H3's `knows_other_two` it starts at stage 2.
Needs a lab, the note, a civilian and the puzzle object. Code:
`Missions/Questlines/Halcyon/RZ_Quest_OtherTwo.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **{brother}'s number** | Haven contract, ★ Lab Tech; 60 min; 8 Fame, 10 Favor | At the lab: *talk*: the trial clerk (Known with Halcyon; Known with the Haven once H3 opened the lab road); *sneak*: the volunteers' ledger in the records room, only at night and unseen (seen: +8 Halcyon Infamy and no number; a Lab Tech also learns where F2 is and which door it uses); *bribe*: a courier, 20 Halcyon Favor | Stage 2 (the field lead gone: stage 5). Time up: again, once | ✅ |
| 2 | **The back room** | Haven contract, ★ Doctor; 60 min; 10 Fame, 10 Favor | At the field station, the field lead held: *talk*: walk in with the lab's courier and his cold case (he follows you from the lab; with no room for an escort, carry his case to the side door yourself); *persuade* the field lead (difficulty 2: Trusted, or Known as a Doctor); *sneak*: the side door at night (a Lab Tech who read the codes: any time), unseen (seen: +5 Halcyon Infamy). Inside: three beds in the Final stage, a chair with a clipboard by each | Stage 3. Time up: again, once | ✅ |
| 3 | **{brother} in the back room** | Haven contract, ★ Nurse; 90 min; 10 Fame, 15 Favor | From the first player's arrival he has long enough to be walked home at a steady pace (10–60 minutes). *Talk*: sit with him (90 s beside him), or walk him home (an escort to the sister's safehouse; a Nurse buys half as long again); *betray*: wait with the sample kit (90 s beside him: research +1, and he gets up as an infected); *force*: end it (armed). Out of time, in the bed or on the road: he turns (an infected where he is) | Stage 4 (the field lead gone: stage 5) | ✅ |
| 4 | **The field lead** | Haven contract, ★ Doctor; 60 min; 8 Fame, 10 Favor | The field lead at her station (held) and the duty officer at the lab: *persuade* her to close the room (Trusted, or Known as a Doctor); *betray*: report her (she remembers it); *bribe*: say nothing (25 Halcyon Favor; the contract pays nothing); *side with* the Red Hand: sell one of the other two (Trusted with them: +15 Red Hand Fame, 40 Favor; the contract pays nothing). She's gone mid-stage: the room went with her | Sold: the ending. Else stage 5. Time up: again, once | ✅ |
| 5 | **Names** | Haven contract, ★ Elder; 70 min; 12 Fame, 15 Favor | At the sister's safehouse: the lab's paper has another number from the house; the trial list by the door (who went up which day) and the man who walked them up (who came back) give the name (an Elder sees the council's pencil). Tell the sister: wrong, 5 minutes; twice, numbers. Then *talk*: the names to the director through the duty officer (Trusted with Halcyon; or Known with it after he came home, with the sister beside you); *side with* the Haven: the council clerk at its seat (+10 Haven Fame, +5 Halcyon Infamy); or *betray*: let them stay numbers (+10 Halcyon Fame, 25 Favor; the contract pays nothing) | The ending | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Named** | the names to the director | Everyone who took part +8 Haven and +8 Halcyon Fame; Haven morale +5; the headline "The lab put fourteen names on its paper last night. The Haven read them at supper." (the number is generated); a Haven–Halcyon truce if faction life allows one now (only then a rumour says so); `other_two.named` | ✅ |
| **Brought home** | the names to the Haven | Everyone +10 Haven Fame; Halcyon morale −3; a headline that says what became of the room; `other_two.haven_names` | ✅ |
| **The work goes on** | numbers (also when nobody names them in time) | Everyone +5 Halcyon Fame; research +2; Haven morale −3; a quiet headline; `other_two.numbers` (H3's headline gives numbers) | ✅ |
| **Sold** | stage 4 | A headline about something new in the Red Hand's pit; 35% the Haven finds out: +15 Haven Infamy for whoever sold (the engine's cap; the design says 25); `other_two.sold` | ✅ |

Every ending also sets `other_two.ward` (1 closed, 2 moved to the lab, 3 kept, 4 sold from,
5 gone with the field lead) and spreads a rumour (8 hours); reported, the field lead is replaced.

*Simplified:* the people are stand-ins, and the brother is a standing figure until he's walked
home (an escort). The other twelve or so names come from the other houses' lists once one is
named, in text. Naming a single volunteer, once a repeatable contract, is now this stage. "A few
minutes left" became time enough to walk him home.

### Authors (Halcyon)

HC3 in [QUESTLINES.md](QUESTLINES.md#hc3-authors): whose names go on the paper, and who stands at
the lab when the cure is made. It starts between 50% and 90% research, never during or after the
final stand, with Halcyon's director and Bastion's commanding officer in their seats, a field lead
and a Haven safehouse for the clinic (one with a medic if there is). Needs a lab, the note, a
civilian and the puzzle object. Its stages are Halcyon
contracts for players Trusted with it (stage 2 from the field lead). It adds no research and never
changes the stand: the stand keeps its own rules, who controls the cure is the finale's
([FINALE.md](FINALE.md)), and the endings are paid after the stand. Its stages wait out a stand and never
run after the cure. It holds Halcyon's questline slot until the stand. Code:
`Missions/Questlines/Halcyon/RZ_Quest_Authors.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **The director's letter** | Halcyon contract, Trusted, cure 50%+, ★ Soldier (+25% Fame); 60 min; 15 Fame, 20 Favor | The duty officer at the lab hands over the sealed letter; *sneak*: take it and read it on the way; *talk*: whoever carries it hands it to Bastion's adjutant. Read, it can be copied onto the field lead's desk at her station, unseen (seen: no copy, +5 Halcyon Infamy) | Stage 2 (skipped with HC1's Bastion's file; the field lead gone: stage 3). Time up: again, once | ✅ |
| 2 | **The field lead's plan** | Halcyon contract from the field lead (held), Trusted, ★ Nurse/Doctor; 60 min; 15 Fame, 15 Favor | Take her cold case: from then 20 minutes (Nurse or Doctor 30) to the medic at the clinic (*talk*: +5 Haven Fame); *talk*: report her to the duty officer at the lab (she remembers it); *side with* the Red Hand: sell it to a buyer near a Red Hand seat (+10 Red Hand Fame, 30 Favor, +15 Halcyon Infamy; the contract pays nothing). It goes warm: missed | Stage 3 | ✅ |
| 3 | **The CO's answer** | Wait, then a Halcyon contract, Trusted, ★ Soldier; 60 min; 10 Fame, 15 Favor | 2 hours of play or a real day; then at Bastion's adjutant: *talk*: agree for the director; *persuade*: bargain for two guards at the gate and no key (Trusted with Halcyon and with Bastion); refuse (+5 Bastion Infamy). Nobody answers: Bastion takes it as no | Stage 4 (no Red Hand seat: stage 5) | ✅ |
| 4 | **The Red Hand's offer** | Halcyon contract, Trusted; 60 min; 10 Fame, 10 Favor | The boss's man at a meeting place near a Red Hand seat: *talk*: turn him down (the director refuses); *betray*: take the money (40 Red Hand Favor, the engine's cap; the contract pays nothing): Take the Lab is offered during the stand; *side with* Bastion once he's been heard: tell the adjutant (+6 Bastion Fame): a Bastion group on the lab's road during the stand | Stage 5 | ✅ |
| 5 | **Ready the lab** | From 75% research: one of Power the Lab, Decontamination or Lab Defence (each with its own rules, as a Halcyon contract for the Trusted) | Picked from the flags: Bastion's terms, power; the Haven's copy, decontamination; else defence. One that can't run, or already runs as an ordinary contract, gives way to the next | Stage 6 | ✅ |
| 6 | **The final stand** | Wait for the stand, then a Halcyon contract (Trusted; 5 Fame, 10 Favor; under the cure finale it pays nothing) that lasts as long as it; everyone within 150 m of the lab takes part | With players near: a Bastion group at the gate (Bastion's terms; Territory > Bastion guard), one on the road toward the Red Hand (Bastion was told; same prefab), and one more Halcyon guard (B3's pilot flew for the samples; Territory > Halcyon guard). While the cure finale runs, the stage places none of these: Bastion's terms or road post send the finale's Bastion group inside its cap, never under Expended, and no Halcyon group goes ([FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room)). The boss was paid: Take the Lab is offered to anyone Known with the Red Hand, not only Champions, with its usual result; while the cure finale runs, the controller offers it to the Red Hand route instead | The ending: won, lost or stolen | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **The Red Hand's batch** | took the money | Whoever took it: +10 Red Hand Fame, +15 Halcyon Infamy (the engine's caps); Halcyon morale −10; a headline that says what happened (stolen, held, or lost to the infected); `authors.batch_sold` | ✅ |
| **Under Bastion's guard** | agreed or bargained | Everyone who took part +10 Bastion and +10 Halcyon Fame; Bastion strength +3; Haven morale −3; a headline (a key in the CO's pocket, or two guards and no key; the squad at the gate only if one stood there); `authors.bastion_key` (agreed), `authors.guard` (a squad held the gate) | ✅ |
| **Open doors** | the method reached the clinic, and nobody reported her | Everyone +10 Halcyon and +10 Haven Fame; infection −3 in every safehouse town (as Leak the Data); a headline: the field lead's name is first on the paper; `authors.open`, `authors.lead_first` | ✅ |
| **The lab alone** | anything else | Everyone +10 Halcyon Fame; a headline on how the stand went | ✅ |

The endings are taken in that order. Each tells those who took part who stood at the lab (up to
four names). Sold at stage 2: `authors.formula_sold`.

*Simplified:* the letter and the cold case are carried as data. "A Bastion checkpoint goes up on
the lab's road" is a Bastion group on that road during the stand, not a road post. Fame +25 and
Infamy +30 are within the engine's caps. A stand that comes early is waited out; after a cure the
line fizzles, with a journal line that says so.

### The Book (the Red Hand)

R1 in [QUESTLINES.md](QUESTLINES.md#r1-the-book): somebody tore a page out of the Red Hand's book,
the one with who outside the Red Hand owes it. Five stages and a wait, four endings. Needs the
boss (the Red Hand's stronghold leader), leaders of at least two other factions for the page (one
each: a Bastion outpost commander, a Halcyon field lead, a Haven council head; Rogue ones first,
then Independent, then any), and the note and puzzle object prefabs (the civilian falls back to the
object). From the second real day of the season. Text keys: the boss, the names, the thief, his
sister and the two other suspects, the trail's towns. Code:
`Missions/Questlines/RedHand/RZ_Quest_TheBook.c`; the three Red Hand lines share
`RZ_RedHandQuestHelp.c` (leaders kept by seat and generation, places put down on approach).

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Who tore it** | Red Hand contract, Known, ★ Police/Ex-Con; 40 min; 12 Fame, 15 Favor | At the boss's camp, the book-keeper and three of the crew, exactly one lying (as The Rat); the liar is the thief. ★: the night book clears one of the other two. Only someone Known with the Red Hand may point. *Talk*: the right one, or the wrong one (+5 Red Hand Infamy, no pay, `book.wrong_man`) | Stage 2 either way: the thief runs. Time up: stage 1 again, once | ✅ |
| 2 | **Find {thief_short}** | Red Hand contract, Known, ★ Ex-Con; 60 min; 15 Fame, 20 Favor | His trail: a jacket in one town, a cold fire in the next (two infected), a barn outside it. *Persuade*: talk him out, unarmed (difficulty 1: Known with the Red Hand, or ★); *force*: take him (his two friends fight, with the Red Hand group prefab; without it, at gunpoint); *bribe*: pay him to run (20 Red Hand Favor, half pay, `book.thief_ran`). He says why: his sister's house is on the page | Stage 3; every name already off the page: stage 4 | ✅ |
| 3 | **Read the page** | Red Hand contract, Known; 90 min; 12 Fame, 30 Favor, times the share collected | Each name at their own seat (held while a stand-in speaks for them). *Talk*: ask (60%, once a name); *force*: lean on them (+3 Infamy with their faction); *bribe*: pay it yourself (15 Red Hand Favor; they remember you helped); let it lie; *side with their faction*: take one line (one in all) to their own stronghold: that leader is replaced (if the leaders' rules allow), their faction's morale −3, +10 Fame with it, +15 Red Hand Infamy. A name whose leader left the chair drops off the page | Stage 4 once every name is seen to, or at time up if any was (the rest are let lie); nothing done: stage 3 again, once | ✅ |
| 4 | **The page** | Red Hand contract, Known; 60 min; 10 Fame, 20 Favor | *Talk*: give it back to the book-keeper (Trusted with the Red Hand; Known with B1's `orders.mocked`): +20 Red Hand Favor. At the barn's fire, *betray*: keep it (each faction collected from pays the keeper 20 Favor, once) or burn it (+15 Red Hand Infamy). *Side with Bastion*: the whole page to its duty officer (+20 Bastion Fame, +15 Red Hand Infamy). Only giving it back pays | Stage 5 on that branch; if he ran, the ending (kept: stage 6) | ✅ |
| 5 | **The thief** | Red Hand contract, Known, ★ Ex-Con; 40 min; 10 Fame, 20 Favor | At the boss's own pit: let it happen; *force*: take his place (4 min in the ring, four rounds of three infected); *bribe*: pay his debt (30 Red Hand Favor, half pay); *sneak*: get him out (he follows you to a safehouse: +1 there, +5 Haven Fame and 10 Favor, no Red Hand pay; near the pit the crowd may see, 30% a check by day, 12% by night, halved for ★: +10 Red Hand Infamy and the pit crew fights). Nobody comes: he goes in | The ending (`book.thief`); kept: stage 6 | ✅ |
| 6 | **The boss hears** | Wait | Only after the page was kept: 2 hours of play | Held over them | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **The book balances** | Given back | Red Hand morale +5; everyone who took part +10 Red Hand Fame; each leader collected from remembers who collected (crossed). A headline with the crew name the returner is given, and a rumour (8 hours) | ✅ |
| **Held over them** | Kept | +15 Red Hand Infamy for the keeper, once the boss hears; the leader leaned on (else the first collected from) swings 25 toward Rogue. A quiet headline and a rumour | ✅ |
| **Bastion's file** | To Bastion | The Bastion commander on the page, if still in the chair, +25 loyalty; Red Hand strength −3; +10 Red Hand Infamy for whoever sided against it; Bust Out ×1.2 for the season (it reads `book.bastion`). A headline and a rumour | ✅ |
| **Burnt** | Burnt | Red Hand morale −5; +10 Red Hand Infamy more for the burner; +5 Haven Fame for everyone if the thief lived; every leader still on the page remembers everyone as having saved them. A headline and a rumour | ✅ |

*Simplified:* the people are standing props with talk and choice actions; only the thief's friends
and the pit crew are AI. The crew name is in the headline and the rumour, and isn't kept for the
rest of the life. H2's `door.debt_yours` only adds a fourth line to the page's text: the world flag
doesn't say whose debt it is. H3's `walk.cut_dose` adds the cut dose to the Haven name's debt (text). B1's `orders.mocked` lets anyone Known give the page back.

### Cargo (the Red Hand, or the Haven)

R2 in [QUESTLINES.md](QUESTLINES.md#r2-cargo), the long version of [Tied Down](#the-red-hands-hard-edges):
the driver of a Red Hand van that carries people wants out. Five stages and up to three waits,
four endings. Needs Supply > Red Hand truck (the van; vehicles allowed, so No Vehicles takes it
out), a Red Hand crew boss who isn't the boss, and the note and puzzle object prefabs. Two
versions: the Red Hand's (its contracts are the Red Hand's, Known, voiced by no leader: it's the
driver's own business), and, half the time once Tied Down has reached its shed this season
(`cargo.truck_seen`), the Haven's (Haven contracts, Known). The loads go to the Long Table's farm
while the house publishes where it is (`table.town`), else to the crew boss's own pit, 250–400 m
from the camp. Tied Down isn't offered while Cargo runs. Code:
`Missions/Questlines/RedHand/RZ_Quest_Cargo.c`; the van and its padlock are Tied Down's
(`RZ_CargoVan`, `Missions/RedHand/RZ_RedHandLoreMissions.c`).

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Meet the driver** | Contract, Known, ★ Ex-Con; offered only 16:00–21:00 game time; 45 min; 6 Fame, 10 Favor | At a crossroads between the camp and the Haven. *Talk*: hear him out (stay within 8 m for 40 s; ★ 10 s); *force*: threaten him (at once; he tells his crew). He tells you where the loads go (with H1's `list.to_redhand`: half came off a Haven list) | Stage 2 (threatened: the camp is on guard). Time up: once more | ✅ |
| 2 | **The load** | Contract, Known, ★ Mechanic; 60 min; 14 Fame, 20 Favor | The van at the camp, no fuel, three people padlocked in the back, its guards (the Red Hand group, else the truck crew; fighting at once if he was threatened). *Sneak*: cut the lock (12 s at the door; living guards see it) and walk them to a safehouse: +3 people, +10 Red Hand Infamy (the Red Hand's version: no pay, +5 Haven Fame and 10 Favor instead). Or get the engine going (30 s at the bonnet, ★ at once; the guards hear it) and drive it, still locked, to the Haven (the same as freeing them); *betray*: to the drop (`cargo.delivered`; the Haven's version: no pay, +8 Red Hand Fame, +10 Haven Infamy); *side with Bastion*: to a Bastion seat (`cargo.bastion`: three civilians sheltered, the driver arrested; +8 Bastion Fame, +10 Red Hand Infamy, no pay) | Freed or Bastion: stage 3. Delivered: the buyer is known, stage 4. Nothing: once more | ✅ |
| 3 | **Who buys** | Contract, Known, ★ Police; 90 min; 10 Fame, 15 Favor | *Sneak*: after dark, watch the drop from 60–150 m for 2 minutes, unseen (★ 1 minute; closer than 60 m spoils it); *talk*: the fence at the Red Hand stronghold (persuade 2 with the Red Hand: Trusted); *bribe*: the driver at the crossroads, 10 Red Hand Favor (not while he's in a Bastion cell). At the farm: `cargo.buyer_known` (C1's proof); with H2's `door.exile_sold`, the exile was one of the ones who never arrived | Stage 4 | ✅ |
| 4 | **Tell someone** | Contract, Known; 70 min; 10 Fame, 15 Favor | *Talk*: the boss's second (Trusted with the Red Hand; +15 Red Hand Favor): a purge (`cargo.purge`). *Betray*: keep quiet for a cut, at the crew boss (held while a stand-in talks; `cargo.cut`). *Side with Bastion*: its duty officer gets the camp, and the farm's grid (`cargo.bastion_raid`, `table.bastion_knows`; +10 Bastion Fame, +15 Red Hand Infamy). *Side with the Haven*: the chair's table (`cargo.told_haven`; +5 Haven Fame, +10 Red Hand Infamy). The Red Hand pays only for the purge; the Haven for all but the cut (Bastion at half) | Stage 5 on that branch; while the driver is in a Bastion cell, or for the raid without the Red Hand group prefab: the ending | ✅ |
| 5 | **The driver** | Contract, Known; 50 min; 8 Fame, 10 Favor | At the crossroads: *sneak*: get him out (he follows you to a safehouse: +1 there), or tell him to keep driving. After a purge, at the boss's pit: get him out, let it be, or *force*: take his place (4 min in the ring, four rounds). After Bastion's raid: clear the crew's camp (the Red Hand group, hostile) before Bastion gets there | The ending (`cargo.driver`: his fate); the cut: stage 6 | ✅ |
| 6–8 | **The cut** | Wait ×3 | Only after the cut: 2 hours of play each, and each pays the cut-taker 20 Red Hand Favor if they're on (else that session's cut goes unpaid). At the farm, once the house is exposed (`table.exposed`): +15 Haven Infamy for whoever took the cut or delivered a load, and the cut stops | The cut | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Purged** | The boss | The crew boss is replaced (if the leaders' rules allow; else a headline that the boss had words); Red Hand morale −3; everyone +10 Red Hand Fame; at the farm, `table.supplier_cut`. A rumour | ✅ |
| **The cut** | The crew boss | The three sessions' cut; another *"never arrived"* headline | ✅ |
| **Bastion's raid** | Bastion | The crew's outpost goes to Bastion if Bastion's strength beats the Red Hand's by 10 and the territory's rules allow it, else Red Hand strength −3; +10 Bastion Fame and +10 Red Hand Infamy for whoever sided with Bastion. A headline | ✅ |
| **Told** (Freed and told) | The Haven | Everyone +10 Haven Fame; whoever was freed at stage 2 and the driver are already at the safehouse; with the farm, C1 can start at stage 4 with the Haven (`cargo.told_haven`). A headline and a rumour | ✅ |

*Simplified:* the three in the van are data until the lock is cut (then an escort); the driver,
the fence and the stand-ins are standing props. Nobody from the farm is spawned here: what's seen at
the drop is told, not shown. Without the Long Table's `table.town` the loads go to the crew boss's
pit, so `cargo.buyer_known`, `table.supplier_cut` and `table.bastion_knows` aren't set and C1 isn't
started (and `cargo.told_haven` doesn't find the house out). Bastion's raid has no Bastion soldiers on the ground. "The Red Hand drops it" (Bastion too
weak) is Red Hand strength −3 rather than an abandoned outpost.

### The Well Story (the Red Hand)

R3 in [QUESTLINES.md](QUESTLINES.md#r3-the-well-story): a crew boss's village has stopped paying.
Five stages and a wait, four endings. Needs a Red Hand crew boss who isn't the boss (Ruthless and
Greedy ones twice as likely), the boss, and the note and puzzle object prefabs. The village: H2's
house when `door.shut` or `door.bastion` is set (`door.house` names it), else the house B2
counted first (`census.counted`), else the Haven safehouse nearest the crew boss (within 6 km), else a quiet village under 30 infection 0.6–4 km
away; its well is a real one if the town has one, else Puzzles > Well (or the puzzle object). It
fizzles if the village reaches 75 infection, or the crew's camp is lost, before the order. A new
crew boss in the chair keeps the grudge. Code: `Missions/Questlines/RedHand/RZ_Quest_WellStory.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **Collect** | Red Hand contract, Known, 🔒 not Elder, voiced by the crew boss; 40 min; 8 Fame, 10 Favor | The keeper at the house won't pay. *Talk*: ask; *force*: lean on them (+5 Haven Infamy; with the survivor group prefab their people fight, and still say no) | Stage 2. Time up: once more | ✅ |
| 2 | **Why {village} stopped** | Red Hand contract, Known, 🔒 not Elder; 40 min; 8 Fame, 10 Favor | The council head (held while a stand-in speaks; the keeper without one). *Talk*: ask why (Known with the Haven): they're short, or they pay Bastion now (`door.bastion`, `census.counted`, or a Bastion seat within 1.5 km); *bribe*: pay their dues yourself (20 Red Hand Favor, `well.paid`) | Asked, or nobody did: stage 3. Paid: The story worked | ✅ |
| 3 | **Tell the story** | Red Hand contract, Known, 🔒 not Elder, ★ Ex-Con; offered only 16:00–21:00 game time; 45 min; 10 Fame, 15 Favor | At the well, the woman drawing water. *Talk*: tell the story: 60% they pay (40% after someone leaned on them; ★ always; +15 Red Hand Favor, `well.paid`). *Side with the Haven*: warn them instead (+5 Haven Fame, no pay, `well.warned`: two people on the well) | Paid: The story worked. Otherwise stage 4 | ✅ |
| 4 | **The crew boss's order** | Red Hand contract, Known, 🔒 not Elder, ★ Lab Tech/Nurse; 90 min; 10 Fame, 15 Favor | At the well. *Sneak*: put something in it, after dark only (`well.done`; seen while they're warned, once noticed, or with shots within 80 m: +15 Haven Infamy at once). *Betray*: fake it (`well.faked`; ★ knows what's harmless, anyone else gets it wrong 35% of the time: +5 infection there). *Side with Halcyon*: a water sample (hold **Fill a sample bottle** by the well, 8 s; ★ at once) to the nearest Halcyon seat (`well.tested`; +8 Halcyon Fame, +5 Haven Fame, no pay). *Force*: stand guard (with the Red Hand group prefab: the crew comes a minute later and has to be put down; +5 Haven Fame and +10 Red Hand Infamy for the one standing guard, no pay). Sealed or held: `well.stood` | Stage 5. Time up: the crew does it themselves (the well was done; nobody's standing changes) | ✅ |
| 5 | **A session later** | Wait | 2 hours of play. After a fake, someone who took part can talk to the village's council head and tell them the truth (+8 Haven Fame, once) | Stage 6 | ✅ |
| 6 | **The boss hears** | Red Hand contract, Trusted; 60 min; 8 Fame, 15 Favor | At the boss's camp, the boss's second; only someone Trusted may answer. *Talk*: own it; *persuade*: blame the crew boss (he goes in the pit: replaced); say nothing | The ending | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **The story worked** | Paid (stage 2 or 3) | Everyone +10 Red Hand Fame; the boss remembers everyone as having helped; the house −2 food (not when a player paid the dues). A rumour (8 hours) | ✅ |
| **The well was done** | Done, or nobody stood in the way | Town infection +15; the safehouse −3 people (never below 2); the crew boss is replaced for a bad deal. Whoever did it: +5 Red Hand Fame, +10 Red Hand Infamy (none after blaming the crew boss) and, if seen, +15 Haven Infamy more. Headline: *"Nobody in [the village] is drinking from the well."* A rumour (12 hours) | ✅ |
| **Faked** | Faked | The house −1 food; the faker +10 Red Hand Fame, and 25% the crew boss finds out (+10 Red Hand Infamy; never after owning it or blaming him); blamed: the crew boss is replaced. A rumour | ✅ |
| **Stood up** | Tested, or held | Everyone +10 Haven Fame; +10 Red Hand Infamy for whoever sided with the Haven or Halcyon (warned them, took the sample); tested: infection −10 there; the crew boss swings 25 toward Rogue (blamed: replaced instead). A headline and a rumour | ✅ |

*Simplified:* the keeper, the council head's stand-in and the villagers are standing props; only the
keeper's people and the crew are AI. The two people on a warned well see everything, so a warned well
can't be done unseen. The poisoned well doesn't start Fever Town there (the infection is real; the arc
picks its own safehouse). Paying the dues yourself ends at The story worked without a refund of the
Favor.

### The House That Went Quiet (the Long Table)

C1 in [QUESTLINES.md](QUESTLINES.md#c1-the-house-that-went-quiet): seven stages, five endings. Only
in a season with a Long Table ([DESIGN.md](DESIGN.md#the-long-table)), and never with A Place at
the Table running. Needs Puzzles > Note and Object (Civilian too, else the object stands in). Starts when someone
hears a wanderer talk about the missing at suspicion 30 or more, or from another line's find.
Nobody gives its first stages: they're offered in person to whoever noticed the house (heard the
rumour, found a tell, ate there, met the host) and to everyone who took part. Code:
`Missions/Questlines/LongTable/RZ_Quest_HouseWentQuiet.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 0 | **Somebody noticed** | Wait (none) | Routes it: H1's `list.found_table` to stage 2; R2's `cargo.buyer_known`, C2's `place.undercover` or `place.proof`, or the house already found out, to stage 4 | 1, 2 or 4 | ✅ |
| 1 | **Never arrived** | In person, Any, ★ Hunter, Police; 60 min; no pay | The last road of the last one who never arrived: a boot on the verge, a satchel at a fence, the farm gate (a step chain). ★ Hunter or Police: each gives the next one's grid | The gate: 2. Time up: 1 again, once | ✅ |
| 2 | **Guest** | In person, Any, ★ Nurse, Elder; 60 min | Whoever keeps the road now, on the lane 160 m out. *Talk*: accept, walk up and eat at the table (`table.guest`, yours; the guest rule watches the rest of the visit). *Sneak*: decline, then lie up a minute on the rise behind the farm unseen (seen: they're wary at stage 3). *Declare a bite* (infected): blessed and sent on, past the boots. Shooting the host ends it | 3. Time up: 2 again, once | ✅ |
| 3 | **Proof** | In person, Any, ★ Nurse, Elder; 60 min | Three of the guestbook (its last small cross is the last one taken), the boots, the letters in the drawer, the cellar (the mother's key, once the boy has said where it hangs; or a crowbar, which they hear) and the boy's one thing. *Sneak*: at supper (18:30–20:30) nobody sees you; otherwise any of them within 20 m (30 m if wary), shots, heat or the infected's notice and you're caught. *Talk*: the boy at the stream, 160–220 m out, unarmed, ★ Nurse or Elder only. *Force*: caught, they set on you; shoot at the house and you start it (+6 Haven Infamy a death until you've found something); win and the house is open | Three: 4. Caught and shut out, or time up: 3 again (three tries). The house finished in the fight: 6 | ✅ |
| 4 | **Who to tell** | In person, Any; 2 h | *Side*: talk to any leader of the Haven, Bastion, Halcyon (needs the lab) or the Red Hand, having taken part: the house is found out; +10 Fame with that faction (the Red Hand +4; +5 more for C2's undercover guest); `table.haven`, `table.bastion`, `table.halcyon` or `table.redhand`. *Force*: "Deal with it yourself" at the farm gate | 5, that faction's way or yours. Time up: 4 again, once | ✅ |
| 5 | **The end of the house** | The told faction's contract, offered in person by its leaders (the radio says who to ask), Any | *The Haven* (10 Fame, 30 Favor; 16:00–21:30): unarmed, "It's time" with the mother once the quiet son is talked round (persuade with the Haven, difficulty 2; ★ Elder or Nurse +1) or let go; the family waits at the edge of town; "Let them walk on", and the boy stays. *Bastion* (10 Fame, 30 Favor): a warning once someone is within 600 m (or after 15 min), the guns 5 min later (Missions > Explosion); get the boy out first (no infected within 25 m of him). *Halcyon* (10 Fame, 20 Favor): the mother, or one of them alive, follows you to the lab (an escort). *The Red Hand* (11 Fame, 40 Favor): 90 s within 50 m of the house, two waves of three infected. *Yourself*: fight them (the fight prefab), or tell the mother you know (persuade, as above) and she walks out, then tell the rest to go; a failed word and they set on you; get the boy out first | The ending; yours: 6. Failed: 5 again, once | ✅ |
| 6 | **What you know** | Wait | Two hours of play or 12 real hours. A faction's leader hearing the proof from someone who took part: half that faction's telling Fame, once each | By your own hand | ✅ |

| Ending | Branch | Consequences | Status |
|---|---|---|---|
| **Walked out** | The Haven | The house is broken up; the boy goes to a safehouse (+1) or becomes the Kid wanderer; Haven morale +5 (after −8 when it was found out); +4 Haven Fame for everyone who took part; the headline *"Every Haven house lit a candle last night. Nobody said who for."* and a rumour (12 hours); `table.walked_out` | ✅ |
| **Burnt** | Bastion | Broken up; its town counts as Burned (town memory), infection −15; Bastion morale +3; +4 Bastion Fame for everyone who took part; a headline. The boy lives only if he was got out (else `table.boy_lost`) | ✅ |
| **Studied** | Halcyon | Research +2 (scaled as any mission's); broken up; the boy to a safehouse or the Kid; the host becomes a Scavenger wanderer; +4 Halcyon Fame for everyone who took part; a headline; `table.studied`, `table.host_wanders` | ✅ |
| **Paid to go** | The Red Hand | The house moves (quiet for a real day, then 2–5 km away) and takes people half as often after; C1 can start again, once; +4 Red Hand Fame for everyone who took part; a rumour; `table.paid` | ✅ |
| **By your own hand** | Yourself | Broken up as stage 5 ends (the boy lives if you got him out); the ending, with a rumour, after the last wait; `table.by_hand` (and `table.boy_lost`) | ✅ |

*Simplified:* the family are standing props with talk and choice actions; only the fights (the
Missions > Events > Rival group, else Survivor group) are AI. The family doesn't walk to the edge of
town with you: they wait there. The jars are in the text, not in the cellar.

### A Place at the Table (the Long Table)

C2 in [QUESTLINES.md](QUESTLINES.md#c2-a-place-at-the-table): four stages, four endings. Personal:
the host makes his offer at the gate on the third meal of a life, to anyone not Trusted with the
Haven, and only that character can do its stages, in that life. Needs Puzzles > Note and Civilian
(or the generic object). No stage pays. Code: `Missions/Questlines/LongTable/RZ_Quest_PlaceAtTable.c`.

| # | Stage | Kind | Approaches | Leads to | Status |
|---|---|---|---|---|---|
| 1 | **The dead** | In person, the guest only; 60 min | A death to the infected in the last hour within 4 km (a wanderer, a supply truck's crew), or else a traveller in a ditch 0.9–2 km out with two infected about. Stand there 10 s (or look), then "Tell him where the dead are" at the farm gate: a bowl, true road news (where a truck is going), `place.dead` (yours), and the house is fed a little. "Walk away": Walked away | 2. Time up: 1 again, once | ✅ |
| 2 | **The dying** | In person, the guest; 40 min; after 20 min of play or 6 real hours | A wanderer from the roster (not the Kid), held by the stage, bitten, 1–2 km from the farm (a stranger if nobody is free). *Betray*: "Tell them where the farm is": they never arrive (the news, a boot), `place.dying` (yours). *Talk*: "Send them the wrong way": the usual bitten wanderer (an antidote within 20 min saves them), the host cooler. "Give them your antidote": +8 Haven Fame, and they remember you saved them | 3 (time up too: they walk on, bitten) | ✅ |
| 3 | **A stray** | In person, the guest; 60 min; after a wait | An exile sleeping rough 1.5–3 km out, away from the safehouses, who follows you once asked (an escort). *Betray*: within 40 m of the farm, he never arrives, `place.stray` (yours). *Side*: within 60 m of a safehouse (it takes him in: +1) or 80 m of a Bastion holding or seat (sheltered), Undercover. *Refuse*: "Tell him no" to the host at the gate: crossed, and the guest rule holds in daylight too | 4, Undercover or Walked away. Time up: 3 again, once | ✅ |
| 4 | **Grace** | In person, the guest; 60 min; after a wait, 17:00–22:00 | The mother at the head of the table. "Sit, and say the name" (the stray's, or the last one taken): Family. "Get up and go": Walked away, with the proof. "Turn the table over" (the fight prefab): a fight; won, the house is finished | An ending. Failed: 4 again, once | ✅ |

| Ending | Consequences | Status |
|---|---|---|
| **Family** | No Fame anywhere. Eating there gives a jar to take away (one food item from the loot pool, once a real day); the guest rule never applies; found out while you're family: Haven Infamy +40, Bastion Infamy +20, your name in the headline; `place.family` | ✅ |
| **Undercover** | The proof: C1 begins at stage 4, and you get +5 Fame on top when you tell; the exile lives with the Haven or Bastion; `place.undercover` | ✅ |
| **Walked away** | Nothing; the host stops meeting you on the road. With the proof (Grace's "get up and go", or a ★ Police guest at stage 1): `place.proof`, and C1 begins at stage 4 | ✅ |
| **By your own hand** (at the table) | The house is finished there (the boy lives); `place.by_hand`, `table.by_hand` | ✅ |

*Simplified:* the people are standing props with talk and choice actions (the stray, an escort,
moves as data while nobody watches); the fight is the only AI.

## The cure finale

The last tenth of the cure ([FINALE.md](FINALE.md)): five routes open at the 90% gate, each a
questline of four acts whose stages are scenes. Open acts are offered in person by the route's
faction, like any contract (anyone not locked elsewhere; the radio says who to ask); sworn acts are
told to the route's locked players and are theirs without asking.
Each step pays its decider at most 10 Fame and 20 Favor (the Independent: 20 Haven Favor). Code:
`Missions/Questlines/Finale/`.

| Route | I (open, +2) | II, the door (sworn, +1) | II, the piece (sworn, +2) | IV (sworn, last, +3) | Status |
|---|---|---|---|---|---|
| **The One Who Didn't Turn** (the Haven) | The Crossed-Out Name: the chair, the List, a house at dusk, 3 food, the Day 6 ledger | And my boy: her yes (persuade 3), then pay her boy's line, walk him out, or she gives without it | Three Times: two bags drawn at dusk, stored cold at the lab | Up the Hill: at dawn, walk her up or run the last draw; or the lab's 2 days if she was spared | ✅ *the subject's branches aren't built* |
| **Held for Collection** (Bastion) | Request Instructions: Sedlak, the quartermaster, the bus driver, the diesel line | Names for the tubes: the driver's slips, the clipboard at night, the census; or the CO's manifest | Collection: the CO's word, a signal form, the box while he sleeps, or cut the door; bring Twice home | Signed For: the box in at the lab; a short box waits 2 days a missing tube | ✅ *no fuel clock, drain or stage road posts* |
| **The Cook** (the Red Hand) | The Kitchen Book: ask her, win her round, follow her, her sister | A Tray: the director, a page of her tally, a field lead, the night store; or four stolen doses | A Night on the Still: an 8-minute hold, or run it cold | The Lab's Word: the proof (60%, the third pan always holds), the case to the lab; mark her line paid | ✅ *Who Came Back and the rival crew boss fold into the odds* |
| **The Lower Store** (Halcyon) | Reference: None: the duty officer, the vial map, the sergeant, the records room | Item 114: the adjutant, the quartermaster, the key board; or the sergeant's key, or cut the chain (3 min) | Something Cold: the vet's tank, then a hold at the dewar | Samples First: the comparison (28 or 56 real hours), then a 4-minute hold | ✅ *no canister riddle or family at dusk* |
| **One Each** (Independent) | Eight Walked Out: the ham's books, the Day 6 ledger, the Doctor | Asking: three of the eight, each with their keeper's leave; or donors | Three bags stored "for everyone" | For Everyone: a tray off the books, then the batch sheet | ✅ *no sit-in or on-air carry* |

| Also | What | Status |
|---|---|---|
| **The desks** | One a route, from the gate until the stand is won: lock (two steps), the finale page, a price door or the Coat Run, pacts, give back what you found | ✅ |
| **Live tissue** (Act III, open) | Who It Was, Contact Captured, Fresh Stock, The Index, Nobody's Index: a leader taken alive (Capture the Index's rules, +2 at once) or one of its guard or young or a special (Live Capture's rules, +2 after 12 real hours); never picked at random | ✅ |
| **The release** | The controller's route: its locked players choose the terms in 3 minutes at the lab | ✅ |
| **Take the Lab** (the Red Hand route, at the stand, once a season) | The loading bay, 120 s, started by a Red Hand-locked player Trusted with the Red Hand; the room changes hands. With the finale off: as before, the batch is stolen | ✅ |

## The epilogue week

After the cure ([EPILOGUE.md](EPILOGUE.md)): the closing work, so that every story can end inside the
week. None of it is offered on the radio; each is a choice on a person or a table at its place, a
leader's reply, or a world change. Code: `Director/Epilogue/`.

| Item | What you do | Status |
|---|---|---|
| **Last Rounds** | The majors' codas on the set-piece calendar ([QUESTLINES.md](QUESTLINES.md#the-majors)) | ✅ *built with the majors; the epilogue lists them and closes them* |
| **Loose Ends** | One closing stage for each questline open at the cure, at its place, two or three ways each on a built ending ([QUESTLINES.md](QUESTLINES.md#loose-ends)) | ✅ *half pay is a fixed Fame 3 and Favor 5; endings' world effects as built* |
| **Bring Them Home** | Take a named dead person's keepsake where it lies; hand it in at the nail, the book, the lab gate or a house; the Names board fills | ✅ *wanderers and notable players' characters only* |
| **Quiet the Ridges** | The packs run down ([INFECTED.md](INFECTED.md#the-last-week)); the hall line names the last to call | ✅ *the factions' jobs (Cull, the last reading, the bet) aren't built* |
| **Take Back a Town** | Clear a town that had 6 or more crawlers at the cure: a candle, the town log, a hall line | ✅ *no givers: the town itself is the job* |
| **The Long Table's last week** | Find it out (as built), or the family packs at processing and leaves for the queue | ✅ *the family as text at the queue* |
| **The Walk Out** | In processing: give your name at the north road's barrier, unarmed; put your name down in phase B | ✅ *no escorts; the queue walks as text* |
| **Settle Accounts** | The parley; pay into the book at the crossroads; wanted posters; square with all four | ✅ |
| **The Last Broadcast** | Say what the station tells the outside; the tally closes at processing and airs | ✅ *no lobbying lines, no airtime for sale* |
| **Personal Goals** | Each built background's last goal | ✅ *on the epilogue's deeds* |
| **The Last Entry** | Your line on the Names board | ✅ |
| **The dose outlets** | Ask for the dose where the route gives it ([EPILOGUE.md](EPILOGUE.md#the-dose-outlets)) | ✅ |
| **The grey choices** | The chair, the CO, the boss and the director each ask once a season, with the decide hold | ✅ |

## Backgrounds at a glance

Every background has contracts it shines at, and some it's shut out of.

| Background | Shines at | Shut out of |
|---|---|---|
| Nurse | Medicine Run, Bitten Friend, Walk Him Out, Rescue the Pilot, Cold Chain | — |
| Mechanic | Crash Site, Junction Boxes, Recover the Bus, Convoy Escort, Blow the Bridge, Car Theft | — |
| Hunter | Rescue the Kids, Smuggler's Run, Hunt the Screamer, Letters (the quiet way), Harvest Guard, Tied Down | — |
| Police | Checkpoint Duty, Trail of Clues, The Rat, The Armory, Double Agent | The Informant |
| Ex-Con | Dead Drop, Fence the Goods, A Quiet Arrangement, Time-Lock Safe, Bust Out, The Pit's Debtor | Raid the Red Hand (until Champion with Bastion) |
| Lab Tech | Field Samples, Water Survey, Quiet Count, Steal the Antidotes, Install Sensors, Patient Zero | — |
| Radio Op | Radio Relay, Recon, Zombie Bait, Lead Them Away, No Reply, Install Sensors, Medical Drone, Learning the Drones | — |
| Unemployed | +50% Fame from every contract (and Infamy worked off by amends) | — |
| Elder (unlock) | Walk Him Out, The Ledger, The Bell Order, The Silent Village | Requisition, Protection Racket |
