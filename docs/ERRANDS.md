# Errands

The small, personal jobs one named person asks of one player, each true because something in the
world is true right now, and over the moment it stops being true; and the [duties](#duties), a
place's own everyday work, there to be asked for from the first day. Contracts are the factions' work,
offered in person by their leaders ([MISSIONS.md](MISSIONS.md#who-gets-offered-what)); questlines are the long stories
([QUESTLINES.md](QUESTLINES.md)); errands are everything in between. Their design is built in full (simplified as the last section
says). In the epilogue week a few of the epilogue
deck's asks stand at the week's places instead of on the dealer: *Bring a cup* at the queue, *Read
the names at supper* at a house, taking a name home ([EPILOGUE.md](EPILOGUE.md#places-and-scenes)).

Code: `Missions/Errands/` (`RZ_Errands.c` the dealer and the small-pay ledger, `RZ_Errand.c` the
parts, `RZ_ErrandRoutines.c`, `RZ_ErrandRoutinesPeople.c` and `RZ_ErrandRoutinesLate.c` the routines, `RZ_ErrandDuties.c` the duties, `RZ_ErrandRegistry.c`
the list and the cards' base, `Cards/<Family>/` one class per card, `RZ_Strangers.c` the stranger
spots and visits, `RZ_StrangerKinds.c` the stranger routines), `World/RZ_NamedPeople.c` (the givers),
`World/RZ_Locals.c` (the locals), `World/RZ_Faults.c` (the faults table). Settings: **Missions > Errands** ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)).
Server only, except the journal row.

## Contents

- [The idea](#the-idea)
- [Givers](#givers)
- [Being asked](#being-asked)
- [The dealer](#the-dealer)
- [Keeping them true](#keeping-them-true)
- [Ways and steps](#ways-and-steps)
- [Work sites](#work-sites) ([carried for real](#carried-for-real))
- [Pay: nothing for free](#pay-nothing-for-free)
- [The faults table](#the-faults-table)
- [The routines](#the-routines)
- [Duties](#duties)
- [The wanderers' asks](#the-wanderers-asks)
- [The cards](#the-cards)
- [Fallout](#fallout)
- [Strangers](#strangers)
- [Locals](#locals)
- [Threads and callbacks](#threads-and-callbacks)
- [Twists](#twists)
- [Board and radio asks](#board-and-radio-asks)
- [Handed errands: the openings' chain](#handed-errands-the-openings-chain)
- [Telemetry and the admin view](#telemetry-and-the-admin-view)
- [Saving and performance](#saving-and-performance)
- [Simplified](#simplified)

## The idea

| | Contract | Errand |
|---|---|---|
| Offered by | the faction's leaders, in person (the radio only says who to ask) | one named person, in person (never a leader) |
| Offered to | everyone who qualifies | one player, and their helpers |
| Size | 10–40 min | a few minutes, one step, within 1.5 km |
| Exists because | weights, needs, reasons | one live fact |
| Pays | Fame, Favor, crates | a little Favor; Fame up to a ceiling; an effect in kind |

Two layers:

- **Routines** repeat, on cooldowns: per player per place a real day, per player anywhere 3 real
  hours, per place server-wide 20 minutes. 34 at the four holding kinds, the wanderers' asks, and
  18 stranger routines.
- **Cards** are hand-written, **once per player per season** (keyed to the identity, not the
  character), at most **2 a real day**. 70, every one true only while its reason is (a world fact, a
  place, your standing or background, the season's stage, a major's wait or the finale's lock).

Errands are never stages, and on the map only while tracked (a diamond on the step's place, then the giver's: [UI.md](UI.md#the-map)). Every one has a way without a gun. They spawn no
AI, no loot and no items. A few steps show a prop at their place while they're open (**Missions >
Errands > Props**: a dead man's effects, a crate, papers, a part; and the car of Shift Change, the
openings' **Guidance > Car wreck prefab**; up only while a player is within 150 m, gone when the
step is done; empty slots: words only, and a `Place check:` warning): His Effects, The Name on the
Tape, Notebook 31 and its fallout, Into the Record, The Truck Didn't Come, Clean Glasses, Pack for
the Queue, Last Orders, Buy It Back, Filter Parts and Shift Change. Nobody in the Zone calls them errands: the Haven asks you to "do us a
turn", Bastion has "a detail", the Red Hand "a little something", Halcyon "a small request".

## Givers

A giver is a **named person**: a name, a role and a place. **Leaders never give errands**, and no
errand reads or writes a leader's memory, regard, loyalty, attitude or vote, so no errand can close
a stronghold leader (or a finale route) to anyone. (A leader may say a thread back, and with **Majors
read errand memory** on a sitting's head may count it: [Threads](#threads-and-callbacks).)

| Where | Who | Shown |
|---|---|---|
| A faction's outpost or stronghold | territory's three named people ([Territory](DESIGN.md#territory)): the cook, the sentry, the toll man... saved with the holding | the first two, on territory's named occupants, while a player is within 300 m |
| A Haven safehouse | four of its own: a cook, a lookout, a storekeeper and a clerk, made from the season's seed, saved with the errands | the first two, dressed onto the house's first two occupants (they stop being plain occupants) |
| The roads | the wanderers ([Wanderers](DESIGN.md#wanderers)), for their own asks | as built |

A routine names the roles it prefers (the Pot the cook, the Set the radio operator); anyone shown
at the place can speak for them. What a giver remembers of you is dialogue's regard on their own
key (`named:<holding>:<i>`, `house:<safehouse>:<i>`; [DIALOGUE.md](DIALOGUE.md#memory)): a done
errand adds "thanks". A wanderer remembers you as helped (their built memory).

## Being asked

- **"Anything need doing?"** is the second reply on a named person (beside **Ask how things are**),
  in the action menu or on the [talk screen](DIALOGUE.md#the-talk-screen). On the talk screen it names
  what they have for you, first, tagged: *[Errand] Water for Morton* (yours already: *[Errand] About
  Water for Morton.*, or *[Errand] It's done: ...* to hand it in), looked at without dealing it
  (`RZ_Errands.TalkErrand`, `Deal`'s look-ahead; the one named is the one dealt when you pick it, while
  it's still true), or shows locked with why (the refusal below, or
  [the reason there's nothing](#when-theres-nothing-the-reason-in-words)) ([DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies)). A wanderer's is **Need a
  hand?**, in their free third slot (not on the Kid, the Mechanic, whose slot is Ask about cars, or
  a bitten wanderer, whose slot is the antidote); with no errand it stays open while Barter Chain
  could start from them. With **Dialogue > Trees** the person's own topic (the cook's supper, a
  sentry's shift, the Courier's late letters) shows beside it and its *Put me to work* runs the same
  ask; the errand reply never hides ([DIALOGUE.md](DIALOGUE.md#hubs-work-comes-out-of-talk)).
- **Waved over.** A safehouse's or holding's shown named person may call a passing player within
  30 m: *"You there. Have you got a minute? Something wants doing, when you're free."* Nothing more: the errand waits as their open offer (10 minutes),
  and they say what it is only when you ask **Anything need doing?** (jobs only by interaction,
  [MISSIONS.md](MISSIONS.md#who-gets-offered-what)). That's an unasked contact: it asks the notice lanes'
  [interruption budget](DESIGN.md#notice-lanes) (`TryContact`, errands last), and every rule of the
  dealer must hold.
- **The offer.** The giver says what they need, in their own voice, and their replies become the
  errand's ways (three at most) and **Not now**, each with what it needs as a tag:
  *"Hand over two tins [2 food]"*, *"Wire in a car battery [Mechanic] [1 battery]"*,
  *"Soften it on the way [Trusted - Haven]"*, *"Add a threat of your own [Feared | +3 Infamy]"*.
  Tags show costs and requirements, never outcomes. The replies are that player's for 90 seconds,
  or until they pick or walk 20 m off; anyone else at the same person hears *"Give us a minute. I'm in the middle of something with someone else."*
- **Not now** is always free: nothing is remembered, and the offer stands 10 minutes.
- **On the talk screen** the ways are said as what you agree to (`RZ_TalkWords.Agree`): "Hand over
  two tins" is *[Errand] I'll hand over two tins.*, with [Errand] and its tags in front, and "Not now"
  is *Not now. Ask me again another time.* ([WRITING.md](WRITING.md#talk)). A way you can't take shows
  locked with what it needs (`RZ_Errands.WayLock`: *Needs Mechanic*, *Needs Trusted with Haven*,
  *Needs 2 food*, *Needs 8 Haven Favor*), things handed over at the place too (*Needs 1 battery*:
  you take the way with them on you, and the work there is locked without them); a Trust check or a threat isn't a lock, its tag shows the
  score. While the ways show, the giver's topics wait (they're waiting on your answer). The labels
  above stay the world prompt's.
- **Errands and jobs are separate** (the author's rule). Errands and duties count only against
  errands and duties: the errands you may hold at once (**Errands one player may hold**, 2), one at a
  time from a giver, one chore of yours at a place. A contract, a leader's job, a questline stage
  and the opening's chain never count against them, and an errand never counts against a job: a
  holding's leader offers its [Job]s and its named people their [Errand]s and [Duty]s side by side,
  whatever you already hold from the place (`RZ_Errands.CountHeld`, `HasOpenOffer` and `ChoresAt`
  leave the opening's handed steps out).
- **The reason in their voice.** Asked when nothing's true, or when it isn't safe, the giver says
  why: *"Not while that lot's at the fence."*, *"You've your hands full already."*, *"Nothing on the
  board for you. Check back after the shift change."*
- **Every line is personal**: the giver's lines are spoken (subtitles, or the talk screen under the
  giver's name), the step and the pay are notices to that player only.
- **Its cards:** taken, *Errand started: <title>* with the giver and the step and its direction; ended,
  *Errand done: <title>* with the pay, *Errand failed: ...* or *Errand abandoned: ...* with why
  ([UI.md](UI.md#notices)).
- **When it ends**, a held errand is a past job in the holder's Journal
  ([UI.md](UI.md#notebook-journal)): done (the giver's thanks as its line), failed (late, lapsed,
  what you carried gone, you died) or abandoned (the world moved on, the fence instead, an opening's
  step dropped). An offer that ran out isn't a job, so it isn't there.

## The dealer

`RZ_Errands` runs on the game mode's tick beside the mission manager, outside the random picks and
the contract cap. Steps run on the 5 s tick; checks, faults and call-outs every 30 s.

**Asked** errands skip the gaps, but not safety: no shot within 60 m in the last minute, no infected
within 30 m, not in a vehicle, not Hunted by the giver's faction, fewer than 2 held.

**Unasked** (a call-out) also needs: alive 3 minutes in this life (a first life finding the Haven:
10 minutes after it gets there), not in a bite's Final stage, no
open offer, 8 minutes since the last unasked offer, not within 300 m of a running mission's spot,
600 m of the set piece holding the evening ([the calendar](DESIGN.md#the-set-piece-calendar)), 1 km
of the lab during the final stand, or 2 km of the Long Table's farm; and room in the budget. Three
players at most each 30 s, round-robin.

**Candidates** are the routines, [duties](#duties) and cards anchored to the giver's place (a
pressing one deals before any duty; the duty is dealt from its own reply beside it) (a safehouse, a Bastion
post, a Red Hand camp, a Halcyon station or the lab, any holding, a wanderer and their role),
checked cheapest first: the card done this season or two cards today; the routine's cooldowns; a
running clock for timed ones; for whom (standing, background); **Why** (the live facts, which fill
the places and the text); **Still**; the subject free; a way that fits here. Each scores
weight × need × fit × novelty × story: ×1.2 if it answers the giver faction's need, ×1.2 if the
giver has the role, ×1.2 if they remember you well (regard 2+), ×0.8 if uneasy, ×0.3 if its family
was one of your last three, ×1.5 for a card; **nearness**, ×1.2 when its own place (its subject, else
the giver) is within 600 m of you, ×0.85 past 1.2 km; **freshness**, ×1.2 when something fresh happened at
its subject (a death, a bite, a fire, shells in the last 2 played hours); and ×0.8 for a real day from a
faction a dead life of yours lied to (below). The pick is weighted among the top three. At most 12
errands live on the server.

**Errand reach.** Most players run: an errand whose subject (the person, lair, house or body it's
about) lies further than **Errand reach** from its giver (`RZ_ErrandSettings`, 1.5 km) is dealt only
when no nearer one is true at that giver (the deal's reason says *a long walk*), and then the giver
adds *It's a long walk: about 3 km north-east.* An errand can't move to another giver (it's about
that person's own place), so it stays theirs. The ways' own places are picked near the giver already
(on Everon all within 1.3 km, [PLACES.md](PLACES.md#offer-reach)).

**The context cache** (`Missions/Errands/RZ_ErrandsDealer.c`, `RZ_ErrandContext`): per online player,
remade every 30 s: the settlement you're in and who holds it, a holding within 600 m, a pack's range
within 1.5 km, a road people use, in a car, a fight (a shot within 60 m in a minute), near a running job
or set piece, minutes into the life, your tiers and your finale route. The score and locals read it;
the admin view prints it.

**"The last one who said that lied."** A lie found out on a dead life (the lie store's events): that
faction's next errand for your identity opens with the line, and its givers' errands score ×0.8 for a
real day.

**Chore slots.** A holding or a house has 1 routine live at once, plus 1 per 5 players online (3 at
most), and one of them per player there; cards don't count, and duties have their own three.

**The suspect variant** (the arsonist rule's other side): The Smoke Was Ours may still be dealt to
whoever called the shells, scored half, the giver opening with *"Where were you Tuesday? Never mind.
Don't answer that."* Every other card keeps the rule: never to whoever made its fact.

## Keeping them true

- **Why and Still.** An errand is dealt only while its facts hold, and re-checked every 30 s and at
  its step. When they stop holding it's **overtaken**: it ends at the next tick, nobody is paid or
  blamed, and the giver says so (*"Never mind. The truck came in after all."*).
- **Soft holds** (`RZ_WorldFacts.SoftHold`, separate from claims): an errand holds its subject (a
  house, a holding, a wreck) so a second errand can't use it. A soft hold never makes a town busy
  for anyone. A **mission's claim** on the subject overtakes any errand on it at the next tick (a
  stage is happening there); a **questline's or an arc's** long hold overtakes only a card whose
  premise lists the subject, and routines go on.
- **Lapse.** A held errand lapses 45 real minutes after it's taken, or 20 minutes after its player
  went offline; the giver *"stopped waiting"*. No penalty.
- **Death.** Held errands lapse quietly. Nothing is lost but the errand.
- **Before the hour.** A way that must be done before a game hour (Before Dark, by half past eight)
  says so in game time and real minutes, and ends if the hour passes.

## Ways and steps

| Way | What it is |
|---|---|
| Give | Hand items over, on the spot or at the step's place: food, a fuel can, a battery, something medical (a bandage, field dressing, gauze, aspirin, medkit, morphine, tourniquet, saline, pills, antibiotics), an antidote (by the prefab's name; mission items never count) |
| Do | Go to a place and stay there long enough: stand the gate, watch, sit with someone, look around. Work with your hands there (dig, search, fill, fix, paint, board up) is a hold at a thing you can see: [Work sites](#work-sites) |
| Sneak | The same, only while hidden (built [stealth](DESIGN.md#stealth)): seen, and the clock stops |
| Talk | A reply that resolves on the spot, sometimes with a Trust check ([RZ_Checks](DESIGN.md#checks-and-ways)), fixed, no roll |
| Pay | Favor with the giver's faction, paid when picked |
| Press | A threat, through `RZ_Checks.Threaten` (Fear against 2; +3 Infamy whatever happens) |
| Report, leave | Hand the matter to someone else, or let it go the giver's way |

**Places**, all within 1.5 km of the offer: the giver's place, its gate (15–25 m), 200–400 m out,
300–500 m out, the nearest empty village within 1.2 km, the nearest other town, or the card's own
place (a wreck, an impact, another house, the warning point at the line). Each is true to the step's
words: the way's place need (`m_sNeed`: the well, the pump house, the farmhouse, a road for a car,
the ridge, houses for a town by name...; [PLACES.md](PLACES.md#the-needs)) is met, or the next of
the four nearest towns is tried. A way whose place doesn't exist here isn't offered; a routine whose
ask names `{next_town}` isn't dealt when no town near has the place. You're at a place within the way's own
radius or 15 m, whichever is wider, in a vehicle too ([arriving](DESIGN.md#mission-building-blocks)).

**A first place.** A way done at two places has a first one of its own (`RZ_ErrandWay.First`: its
place, its line, its seconds), so each place is one objective ([MISSION_RULES.md](MISSION_RULES.md),
rule 23): The Market's ring (search the grass where the wedding party was, 20 s, then the groom's
mother's door) and Petr's key (from under his step, 5 s, then Petr in the queue). Standing at the first
place its seconds tells you the step; the step itself counts only after. A way whose words named a
second place nothing checked now names the one place it's done: the sales "at the fence" (Notebook
31, The Name on the Tape, His Effects, The Patrol That Didn't Come Back) take the thing *for* the
fence (its Favor comes with the hand-in), and The Boy's ways talk him round where he is.

**Done** is coming back and handing it in, in their talk: the giver's *[Errand] It's done: <title>.*
reply, first on their list ([DIALOGUE.md](DIALOGUE.md#jobs-on-the-first-replies)); something you
carried back has to be on you, or on a helper beside you (*"Where is it, then? Bring it here with
you."*), and is taken off whoever has it (`RZ_Errands.HandIn`). Only without the talk screen is
arriving back (as above; a wanderer: where they stand now) enough. A handed errand (an opening's step)
never waits to be handed in: each of its steps ends where it's done (`RZ_Errands.Hand` makes sure,
logging `Errands: handed <key> didn't end where it's done; it does now`), and its one hand-over (The
Cut) counts only by its reply ([below](#handed-errands-the-openings-chain)). The tracker, the map and the Quests tab point at the step's place (within the
way's radius) until it's done, then at the giver where they stand now (15 m), the same spot the
errand counts you at (`RZ_Errands.GiverAt`; [MISSIONS.md](MISSIONS.md#where-each-objective-points)). **Helpers** are the asker's vanilla group within 50 m for 2 minutes or when the
step was done, three at most.

A way's **step** is the line the tracker shows: it names who and where with the card's tokens
(`{giver}`, `{place}`, `{here}`...), and the way back names the giver's place
([MISSIONS.md](MISSIONS.md#objectives); how it's written: [WRITING.md](WRITING.md#objective-lines)).

## Work sites

A way whose step is work with your hands is a [work site](DESIGN.md#work-site) at its place: the
thing to work at stands there (its before look, or the way's own prop), with one prompt in the step's
words. Hold it for its time (vanilla's bar); the step is done, and the result stands there until the
errand is over and nobody is near. Only the asker can do it (the prompt doesn't show for anyone
else); at the giver's place or its gate it stands 3–6 m off, at water 1.5–3 m off. Work the words
put indoors (a bench, a sick man, a search of a house, a shop, the police station, the school or the
hall) is on the ground floor of the nearest such building within 60 m of the place, else in the open
as before; where the words name someone (the newcomer, the bitten man, the survivor before dark) that
person stands at it while you work (Props > Duty person). The table says which. An item the step
needs at its place (a battery) is handed over after the hold, there. The step's line drops its time
there. With work sites off, these steps are done by staying there as before (the times in the
tables below).

| Errand | The prompt | Hold | Before → after |
|---|---|---|---|
| The Pot | **Search the gardens and sheds** | 9 s | a crate of odds and ends → rummaged clutter |
| Water for {house} | **Fill the bottles** | 8 s | empty cans and buckets → nothing (carried off) |
| Filter Parts | **Take the filter housing off** | 8 s | a toolbox and parts → nothing (fitted, packed away) |
| Filter Parts | **Take the part off** | 6 s | a toolbox and parts → nothing (fitted, packed away) |
| The Burying | **Dig the grave** | 15 s | a marked plot (a stake, a wrapped bundle, planks) → a grave: a mound and a post |
| The Round | **See to the sick** | 20 s | a mattress, a blanket, a bucket → the same (it stays) |
| The Newcomer | **Search the shops for a coat** | 8 s | a crate of odds and ends → rummaged clutter (in a shop) |
| The Newcomer | **Check the newcomer over** | 9 s | a mattress, a blanket, a bucket → the same (it stays) (indoors, with him) |
| Make It Right | **Fill the water cans** | 7 s | empty cans and buckets → nothing (carried off) |
| The Count | **Count the stores** | 9 s | a table with a lamp → the same (it stays) (indoors) |
| The Latrine | **Dig the latrine** | 15 s | a marked pit (stakes, planks, a sack of lime) → the latrine screen |
| The Pit Count | **Count the bets into the book** | 20 s | a table with a lamp → the same (it stays) (indoors) |
| Night Readings | **Fit the battery** | 6 s | a toolbox and parts → nothing (fitted, packed away) |
| Night Readings | **Reset the sensor** | 7 s | a toolbox and parts → nothing (fitted, packed away) |
| Labels | **Relabel the sample trays** | 20 s | a table with a lamp → the same (it stays) (indoors) |
| Labels | **Relabel the sample trays** | 12 s | a table with a lamp → the same (it stays) (indoors) |
| Before Dark | **Clean and dress the bite** | 11 s | a mattress, a blanket, a bucket → the same (it stays) (in a house, with the survivor) |
| The Market | **Search the grass** | 8 s | the effects (a kit bag, a jacket) → nothing (taken) |
| The Receiver | **Fit the battery** | 6 s | a toolbox and parts → nothing (fitted, packed away) |
| The Receiver | **Re-seat the cable** | 7 s | a toolbox and parts → nothing (fitted, packed away) |
| Sit With Him | **Dress the bite** | 12 s | a mattress, a blanket, a bucket → the same (it stays) (indoors, with him) |
| Sandbags | **Fill and stack the sandbags** | 18 s | a pile of empty sacks → a sandbag barricade |
| The New Number | **Paint the new number** | 10 s | a bare board on a post, a paint tin → the stencilled board |
| Behind the Red Door | **Search what's left** | 7 s | a crate of odds and ends → rummaged clutter (in a house) |
| Behind the Red Door | **Search the boy's room** | 8 s | a crate of odds and ends → rummaged clutter (in a house) |
| Clean Rifles | **Search for cleaning kits** | 8 s | a crate of odds and ends → rummaged clutter (in the police station) |
| Clean Rifles | **Strip and clean the rifle** | 15 s | a table with a lamp → the same (it stays) (indoors) |
| Clean Paper | **Search for clean paper** | 8 s | a crate of odds and ends → rummaged clutter (in the school) |
| Clean Paper | **Rule the margins** | 9 s | a table with a lamp → the same (it stays) (indoors) |
| Chairs for the Reading | **Pick up a chair** · **Put the chair down** | 2 s each | [carried for real](#carried-for-real): three old chairs inside the hall → a row of them by its door |
| What the Cure Costs Here | **Put the name on the roster** | 8 s | a board on a post → the notice wall |
| Into the Record | **Take the notebook** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| Take a Name to the Board | **Write the name up** | 8 s | a board on a post → the notice wall |
| Pack for the Queue | **Search the house** | 9 s | a crate of odds and ends → rummaged clutter (in a house) |
| The Tags | **Take the tags** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| The Tags | **Take the photo frame** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| The Wreck's Name | **Search the cab for papers** | 8 s | a crate of odds and ends → rummaged clutter |
| The Wreck's Name | **Search the cab** | 8 s | a crate of odds and ends → rummaged clutter |
| Clean Glasses | **Search for whole glasses** | 9 s | a crate of odds and ends → rummaged clutter (in a house) |
| Name Day | **Take the clock down** | 8 s | a toolbox and parts → nothing (fitted, packed away) |
| The Truck Didn't Come | **Search the truck** | 9 s | a crate of odds and ends → rummaged clutter |
| Clean Slate | **Dig at the burial pit** | 15 s | a marked plot (a stake, a wrapped bundle, planks) → a grave: a mound and a post |
| Civvies | **Search for plain clothes** | 8 s | a crate of odds and ends → rummaged clutter (in a house) |
| The Name on the Tape | **Take the tag and jacket** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| The Name on the Tape | **Take the jacket** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| Notebook 31 | **Take the coat and notebook** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| Notebook 31 | **Take the notebook** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| Notebook 31 | **Pocket the notebook** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| Paint | **Paint the hand** | 12 s | a bare board on a post, a paint tin → the stencilled board |
| Dress Uniform | **Fold the uniform** | 12 s | a table with a lamp → the same (it stays) (indoors) |
| Names on the Call | **Pin the list up** | 5 s | a board on a post → the notice wall |
| The Last Apples | **Pick what's left** | 9 s | a crate of odds and ends → rummaged clutter |
| You're the One | **Write the name in the book** | 6 s | a table with a lamp → the same (it stays) (indoors) |
| The Visitors' Log | **Sign the log** | 5 s | a table with a lamp → the same (it stays) (indoors) |
| Not Past the Chapel | **Board up the doors** | 15 s | planks and a bucket of nails → the boarded doorway, in the farmhouse's real outside door within 40 m (no real door: the boards are left out; no door is ever spawned in the open) |
| False Hope at the Line | **Buy a set of papers** | 8 s | the forger himself, at the gate (Props > Duty person) → nothing |
| False Hope at the Line | **Send him on his way** | 8 s | the forger himself, at the gate → nothing |
| His Effects | **Gather their things** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| His Effects | **Take their things** | 7 s | the effects (a kit bag, a jacket) → nothing (taken) |
| Last Orders | **Take the order book** | 6 s | the effects (a kit bag, a jacket) → nothing (taken) |
| The Name on the Tag | **Raise the cairn** | 12 s | a marked plot (a stake, a wrapped bundle, planks) → a grave: a mound and a post |
| The Name on the Tag | **Take the ID** | 6 s | the effects (a kit bag, a jacket) → nothing (taken) |
| The openings: Water, Water for the Fever | **Fill what you can carry** | 8 s | empty cans and buckets → nothing |
| The openings: The Fever, The Clock | **Treat his fever**, **Give him the antidote**, **Clean and bind the bite** | 8 s | a mattress, a blanket, a bucket, with the son or the bitten friend lying on it (the prompt at his side) → the same |
| Strangers: The Funeral for the Car, Two in the Back (push-start) | **Push the car** | 10 s | the car itself |
| Strangers: Two in the Back ([Mechanic]) | **Fix the car** | 15 s | a toolbox and parts → nothing |
| Strangers: The Digger | **Dig the grave** | 15 s | a marked plot → a grave |
| Strangers: Seed Potatoes | **Dig along the field edge** | 10 s | a stake and a sack → turned earth |
| Strangers: The Face on the Notice | **Take the book** (unseen) / **Burn the books** | 10 s | papers → nothing / ash |

The rest stay as they are: standing a post or the wire, watching, counting from cover, sitting with
someone, talking, walking someone somewhere, handing something over, and leaving something at a
door (a moment's work, not a job of work).

**The thing its words name stands there.** A hands task names a thing hands work has no look for,
so each gives its own (`RZ_ErrandWay.Look`, and plainer ones of the same kind by `Fallback`, tried
where it doesn't fit: never a box of another kind, [DESIGN.md](DESIGN.md#mission-building-blocks)).
The kitchen's three tasks stand **at the place's own kitchen** when it has one: the camp's cook pot
(or a hand-built site's cook spot) within 60 m, with nothing more put out. Else at a small kitchen of
their own:

| Task | Its look | Plainer, in order |
|---|---|---|
| **Peel and chop** | the kitchen table: the pot, two bowls, a sack of potatoes at its end | the pot on its own |
| **Stir the pot** | the pot on a canteen stove | the kitchen table; the pot on its own |
| **Ladle supper out** | the pot on a table, bowls, a mug and a tin set out | the pot on its own |
| **Split the wood** | the woodpile and chopping block | the chopping block alone |
| **Wash and hang the bedding** | the washing line | a laundry basket |
| **Sluice the yard** | a bucket | — |
| **Scrub the tables** (bench) | the meal's table | an old table |

The other kinds' plainest things: filling's a bucket, sandbags' one sack, a "Take ..." prompt's and a
search's the base game's cardboard box. Before this, *Stir the pot* at Morton's hall stood at a
cardboard box against the wall (its fire-and-pot look didn't fit, and every look fell back to the box). The forger's two prompts have the forger
standing there. A hands task that opens with neither a look nor a person is logged as a fault
(`Errands: <errand> '<prompt>' has nothing standing at its work site`). Before this, *Carry and stack
the crates* at Morton's hall was a prompt on bare grass: hands work had no look, and the task gave
none.

### Carried for real

A task whose words say carry, stack or bring in is carried for real ([DESIGN.md](DESIGN.md#carrying)):
the things stand in a pile, you pick each up (2 s), walk it where the words say, slowed, and put it
down there (2 s), stacked; the tracker counts them. Raise your weapon and you put it down where you
stand.

| Task | The things | From → to |
|---|---|---|
| ✅ Wood and water (every faction's): **Carry the crates in** | 3 small wooden crates (the base game's `BoxWooden_01`) | outside, 6–12 m from the place → indoors on the nearest building's ground floor, three high |
| ✅ Chairs for the Reading | 3 old chairs (`ChairOld_01`) | inside the village hall, ground floor → outside by its door, in a row |

Left as one hold, with why:

| Task | Why it stays a hold |
|---|---|
| Sandbags: **Fill and stack the sandbags** | built where it stands: the sacks are filled and laid in place, nothing is carried away |
| Wood and water: **Fill the water cans**; the openings' **Fill what you can carry** | filled at the water, then carried off in the pack, not a load to set down anywhere |
| The duties' scene options: Haven's **Carry the sack to the kitchen**, **Carry them back in**, **Carry it in to the storekeeper**, **Carry it from the stream instead**; Halcyon's **Carry water up from the yard tank**; the Red Hand's **Load the handcart**, **Load the mash**; Bastion's **Move the nest to the hedge** | a choice at a scene's beat ([Things that happen](#things-that-happen)): the option is what you decide there, its line says what was done; a carry between two beats' places is not built yet |
| The Cold Chain's **Take the cold box** / **Carry it back** | already carried by its own rules (no running, no drops, the clock: the scene's carry), one box in the arms |
| The couriers' errands (*Carry it to ...*, *Carry the list to the house*, *Carry the tins to {house}* ...) | a letter, a list or a parcel in your pack, walked there: an item, not a load |
| The Last Pilot's **Load ...** (a questline's choice of cargo) | one choice at the helicopter, not a carry |

## Pay: nothing for free

Through the one small-pay ledger, per identity for the season (`RZ_ErrandLedger`):

- **Favor** 3–10 an errand (× **Pay scale**), with diminishing returns per player per real hour
  (×1, ×1, ×0.75, ×0.5, ×0.25) inside **40 an hour**. A quiet way pays ×1.3 if no shot was heard
  near it.
- **Fame** 0–2 an errand, as small pay (`RZ_RepFlag.SMALL`: left out of season Fame, the hall of
  fame's Fame lines and the best deed): never above **39** with that faction (errands can make you
  Known, never Trusted), and at most **10** errand Fame per faction per season day. No rival shares.
- **Infamy decides the kind.** At 15+ Infamy with the paying faction, an errand pays **amends**
  instead of Fame: 2 Infamy off through `Forgive()`, at most **8 a life per faction** from errands,
  never below 15 (inside reputation's 30; [REPUTATION.md](REPUTATION.md#working-off-infamy)). The
  amends family (Make It Right, the Latrine, Clean Slate) pays only that while you're Watched.
  Hunted or worse: nothing.
- **Helpers** get half the Favor and at most 1 Fame. In-kind effects go to the asker only.
- **In kind is an effect, not loot**: +2 food at a house, a fault fixed, morale +1 through the
  contracts and errands lane of the [season budget](DESIGN.md#the-season-budget). Errands never hand
  out doses, crates, kit or keys.
- **Bought food counts for nothing.** Food a quartermaster sold or handed out is refused by every
  errand's food hand-in, as at a house and the fence ([REPUTATION.md](REPUTATION.md#haven-house-rules-and-exile)
  has the house's pay). A safehouse's food Fame also goes through this ledger's ceiling and day's cap.
- **"Most people helped"**: each errand done counts on the hall of fame's line of that name, until
  the cure. It pays nothing.

**Other kinds of pay** (`Missions/Errands/RZ_ErrandsPay.c`; each a setting on **Missions > Errands**):

| Kind | When | What |
|---|---|---|
| **In kind** | a Bastion post (a Red Hand camp), one errand in three paying 4+ Favor, never while that faction needs food or supplies | a ration (a magazine) off its own shelf in place of 3 Favor, in your pack, **tagged** as the giver's goods: food delivery, the fence and every errand refuse it for pay (*"These are Levie's. Take them back to Levie."*, [mission goods](DESIGN.md#story-blocks)). Needs **Props > Pay ration** (**Pay magazine**); empty, Favor only |
| **A meal** | a house errand paying 4 Favor or less, at a house fed above one upkeep | supper: the house −1 food, 15% of your health back, 2 Haven Favor, in place of the Favor |
| **When the truck comes** | a post whose faction needs supplies | the Favor waits for its next truck there (a real day at most) and goes to the life that earned it, if it's online when the truck comes in |
| **Carried items** | a way marked carried (His Effects' effects, The Name on the Tag's ID) | the step puts an item in your pack (a card's own where it has one: **Props > Soldier's ID**, **Forged papers**; else **Props > Carried**, else the mission item: it can't be equipped, no mission's drop takes it). Only you or a helper can hand it in; taken by anyone else, the errand is spoiled and the thief takes +3 Infamy with the giver's people; lost, it's over |

**Headlines**: errands put out at most **2** a real day (**Headlines a day**).

## The faults table

Some routines answer a small real fault at a place (`RZ_Faults`), rolled at most once a shift
(05:30 and 17:30) per place, only while a player is within 2 km, each kind at **Fault chance**
(0.25), and cleared at the next shift or when an errand fixes it. **A fault never drains stock.**

| Fault | Where | What it shows |
|---|---|---|
| Relief missing | Bastion posts | one guard fewer at the gate; a line on Bastion's secure channel |
| Set low | Bastion posts | a line on Bastion's secure channel (*"you're all hiss"*) |
| Fuel low | Halcyon stations; safehouses | Halcyon's secure channel; the house's news (*"The generator's on its last cupful"*) |
| Sensor dark | Halcyon stations | a line on Halcyon's secure channel |
| Water low | safehouses | the house's news |

## The routines

✅ all built. Pay is before the diminishing returns; "+2 food" is at the house.

| Routine | Where, giver | True when | Ways and pay |
|---|---|---|---|
| **The Pot** | Haven safehouse, the cook | food under one upkeep, and no Haven truck on its way to it | Hand over two tins [2 food]: 4 Favor, 1 Fame, +2 food. Search the gardens at the nearest empty village (a crate: hold **Search the gardens and sheds**): 5 Favor, +2 food. Writes `errand.fed` |
| **Lights** | safehouse, the storekeeper | the house's fuel fault, dusk within 20 real min (or dusk) | A fuel can, or [Mechanic] a car battery: 5 Favor, the fault fixed. Writes `errand.lit` |
| **Water for {house}** | safehouse, the lookout | the house's water fault | Fetch water from the next town (20 s): 5 Favor (×1.3 quiet), +2 food, fixed |
| **Filter Parts** | safehouse, the storekeeper | the water fault, 8+ people, Known | The pump house part at the next town (25 s; [Mechanic or Lab Tech] 12 s): 8 Favor, fixed |
| **The Gate at Dusk** | safehouse, the lookout | dusk near, one guard or none, a town at 60+ infection within 1.5 km, not Every Ridge Calls | Stand at the gate 10 real min: 5 Favor |
| **The Burying** | safehouse, the clerk | someone died at the house lately | Dig the grave at the edge of town (hold, 15 s; a grave stands there after): 3 Favor; a world record |
| **The Round** | safehouse, the clerk | [Nurse or Doctor], no medic, 4+ people | Go round the sick at their beds ([below](#the-round-at-the-beds)): 4 Favor, less for the wrong calls (no Fame unless every call was right), 2 more for every one right |
| **The Newcomer** | safehouse, the clerk | the house took someone in this season day | A coat from the next town (20 s), or [Nurse or Doctor] check him over (30 s): 4 Favor |
| **The Children's Hour** | safehouse | 5+ people, 08:00–17:00 | Watch the yard 5 real min: 4 Favor |
| **Make It Right** (amends) | safehouse, the lookout | the Haven Watches you | Keep the gate (3 min) or carry the night water (15 s at the next town): amends |
| **Shift Change** | Bastion post | the relief fault | Wake the relief in a car at a road's verge 200–400 m out (10 s): 4 Favor, fixed. Report him: 1 Fame. Stand the post (3 min): 6 Favor, fixed |
| **The Set** | Bastion post, the radio operator | the set fault, 17:30–20:30 | A battery: 5 Favor, fixed |
| **The Wire** | Bastion post | a Bastion truck lost within 1.5 km in a season day, or a town at 50+ within 1.5 km; not Watched | Man the wire 5 min: 6 Favor |
| **The Count** | Bastion post | a Bastion truck lost near, or the post short; Known | Count three stacks at the table (hold, 9 s), then write what's there (6 Favor) or what they want (10 Favor; a world record) |
| **The Bag** | Bastion post | night, Bastion morale under 40 | Put his letter in the mail bag: no pay; Bastion morale +1 |
| **Fuel for the Post** | Bastion post | the post short after a lost truck | A fuel can: 6 Favor |
| **The Latrine** (amends) | Bastion post | Watched or Stranger with Bastion | Dig it: hold **Dig the latrine** (15 s) at the marked pit behind the post; the latrine screen stands there after: Watched, amends; Stranger, 3 Favor |
| **Fuel for the Lorry** | Red Hand camp, the toll man | the Red Hand needs supplies | A fuel can: 6 Favor. Or say there's no fuel left |
| **Medicine** | Red Hand camp (not the stronghold), the cook | always | Two tins for the still: 6 Favor |
| **Short Change** | Red Hand camp, the lookout | Red Hand morale under 55; Known | Watch the collectors (a minute) and name him, or [Police or Ex-Con] pick him out (20 s): 8 Favor. Or watch and warn him: nothing |
| **The Pit Count** | Red Hand camp | a Fight Pit running | Count the bets into the book (2 min): 5 Favor |
| **The Freezers** | Halcyon station or the lab | the fuel fault, evening or night | A fuel can or a battery: 6 Favor, 1 Fame, fixed |
| **Night Readings** | Halcyon station or the lab | the sensor fault | A battery at the sensor 300–500 m out, or reset it unseen (15 s hidden): 5 Favor, fixed |
| **Labels** | Halcyon station or the lab | night | Relabel the trays (2 min; [Lab Tech] 1 min): 4 Favor |
| **Who's at the Gate** | safehouse, the lookout | someone waits at the gate (a found hook's bitten stranger was sent here) | Let them in (+1 person); [Nurse, Doctor or Police] look them over first; or walk them to Bastion's post within 1.5 km. No pay. The house checks everyone at the door: a bite is found there, they're walked out, and nobody blames you |
| **Sit With Him** | safehouse, the clerk | a bite declared at the house today (the reason `house.bitten`) | Sit with him 2 min ([Nurse or Doctor] 1 min, seeing to the bite). He tells you where his tins are buried (+1 food), or one true thing from the world. No pay. The record `errand.sat_with` with an epilogue line |
| **Fresh Chalk** | safehouse, the lookout | by day, the edge of the infected's ground within 1.5 km, the Haven's reading of it 2+ season days old | Watch the edge 30 s unseen and chalk the date: 6 Favor, the reading day written (the only thing it writes). [Hunter or Radio Op] at dusk: listen from 300 m out (20 s), the same. Chalk without looking: 3 Favor, nothing written, and a lie that the next real reading by anyone finds |
| **Fell Off the Inventory** (Spare Magazine merged in) | Bastion post | the shift change within 20 real min, a Red Hand holding within 1.5 km | [Known: Red Hand] buy the rifle back there (8 Red Hand Favor): 5 Bastion Favor. Tell the sergeant: 1 Fame, and the fence hears who sent Bastion asking. Known with Bastion, its morale under 40: sell his spare magazine to the crews for him: 6 Favor; a Bastion body seeing it, +5 Bastion Infamy |
| **Sandbags** | Bastion post | a death or shells within 400 m in 3 played hours | Fill and stack sandbags at the gap (2 min at the gate): 5 Favor |
| **Rent's Due** | Red Hand camp | the infected's ground within 1.2 km, dusk within 20 real min | Count them past from 200–400 m out, unseen (2 min): 6 Favor. Make a number up: 3 Favor, and a lie found by rumour (+3 Red Hand Infamy) |
| **The Number** | Halcyon station or the lab | night, a trial dose that didn't take today | Sit with the night shift (90 s). No pay |
| **His Lighter** (the dead) | Red Hand camp, a crewman or the cook | a death within 1 km in the last season day (never to whoever caused it) | Sell it to the fence: 6 Favor. Take it to his father at a house within 1.5 km (5 s): 1 Haven Fame, a local (what he owed is in the book against him now). [Known: Bastion] sell it to Bastion's post as proof of one less: 5 Bastion Favor and a lie the rumour round finds (+4 Red Hand Infamy) |
| **The Market** | Red Hand camp, the fence | a Wedding Party cleared within 3 km in the last season day (its signal `wedding`) | Find the ring (20 s) and sell it to him: 8 Favor. Give it to the groom's father at a house (5 s there): 2 Haven Fame, and the fence calls you soft. Sell it to him for two of the house's tins: 4 Favor, the house −2 food |
| **The Receiver** | Halcyon station or the lab, the technician | a tagged pack within 6 km, and the receiver down today (half the days, by the season's seed) | A battery at the mast 200–400 m out (10 s), or [Lab Tech] re-seat the cable (15 s): 5 Favor. Never near the pack |

*Simplified:* His Lighter's "crewman" is anyone who died near the camp, and The Market's ring is found
at the party's spot as a step, not as an item.

### The Round at the beds

✅ Played as a scene (`Missions/Errands/RZ_DutyRound.c`, the duties' scene engine), laid out when it's
taken. Two to four of the sick, each with a name and one of ten conditions (a fever going round the
town, a cut gone bad, a bite hidden under a sleeve, hunger from giving their food to the children, a
broken wrist, a child waiting for their mother, an old man who's stopped trying, a chest that's
worse lying flat, a burn, and a wound too far gone), at most one of each.

**Where they lie**, in this order, with the place and every piece's fit or drop in the log
(`Errands: The Round: ...`):

| Where | When | How it fits |
|---|---|---|
| Indoors | the house's ground floor has room for two cots or more | Against the walls, on the loose loot's floor points of the lowest storey, a cot (**Props > Round bed**) each: its long side to the wall, else its foot to the room, a little further out each try. Its measured box needs a floor under every corner (a downward trace) and nothing in the way at knee or chest height, and keeps a person's width from the other cots |
| Under a tent | not indoors | A canvas shelter (**Props > Round tent**) 6–14 m from the house, its open front away from it, else sideways: its box clear at knee and chest, level within 0.6 m, nothing over it. Up to 8 spots tried. The cots under it are laid out as JTF Bronze Props' casualty point lays them (three side by side, a fourth across the back); one whose box isn't clear is left out |
| In the yard | neither | Cots 4–10 m from the house, their long side to the wall, each box clear, level within 0.3 m, nothing over it and apart from the others; a cot that won't fit is left out |

Nothing is squeezed in: what doesn't fit is dropped. With no Round bed (or one that was never
measured) there's nothing indoors and they lie on the ground under the tent or in the yard. If
nowhere takes even one, the clerk says so and the errand pays nothing. The measured boxes are the
cot's and the shelter's in `tools/camp_kit/footprints.json` ([SITES.md](SITES.md#real-footprints)),
copied into `RZ_DutyRound.Box`. The cots and the tent go when the errand ends.

**At each bed.** The patient lies on the cot (vanilla's unconscious state, so they lie still) and
is a duty's person on the talk screen: only the bed you're at answers, the others are waiting their
turn. Their conversation (`dialogue/duties/round.rzd`) is what they say, what happened to them and
what they need. The bed's work sites stand along its open side, not in the wall:

- **Five looks, time for three:** feel their brow, look them over for a wound, listen to their
  breathing, look in their eyes, and ask what happened (also asked on the talk screen). Each tells
  you something; together they tell you what it is.
- **Three things to do, one right:** water and a cool cloth, clean and dress it [1 med], fetch
  the clerk quietly (a bite: counts as reporting it), something to eat [1 food], splint and sling it
  [1 med], sit with them and talk (morale), sit them up and get them talking, sit them up and keep
  them up, cool water then a loose dressing [1 med], send for the medic from the next house. The
  wrong one uses what it uses and does nothing for them, and they let you see that.
- **A bite missed:** if the hidden bite was treated as something else, before the round's done the
  patient sits up turned. The house deals with it: morale down, the town 2 more infected, and the
  clerk tells you two of theirs are being stitched.

**Pay**, scored as a duty's scene is: 4 Favor and what the choices earned; less for the wrong
calls and no Fame then; every call right, 2 Favor more and the clerk says so.

*Simplified:* the patient's lying pose is vanilla's unconscious state; the turned patient is told,
not spawned as an infected.

Four missions left the random pool for these and are gone from the code: Fuel for the Lights
(Lights), Water Purifier (Filter Parts), Water Run (Water for {house}) and Medicine Before Dark
(Before Dark).

## Duties

A **duty** is a place's own everyday work, taken over for a shift: the kitchen, the wood and the
water, washing down, then the line at the gate, the dead, a patrol or a hunt with their people,
the sample bench. It is an errand like any other (the same givers, ways, steps, work sites, pay
and ledger), with three differences that put work in reach from the first day:

- **It's always there to be asked for.** A routine or a card needs something to be true right now.
  A duty needs only the place to stand. So a named person asked *Anything need doing?* has work for
  a newcomer every time, unless one of the stated reasons below applies.
- **Always on offer, alongside.** A named person with a duty to give always shows one. With
  something pressing to ask (a routine or a card that's true now), the talk screen lists that
  first, as *[Errand] ...*, and their duty after it, as ***[Duty] Kitchen Duty***
  (`RZ_Errands.TalkDuty`); you take one of the two. With nothing pressing, the errand reply itself
  is the duty. Which duty: the giver's own work first (×2 for their role: the cook's kitchen, the
  nurse's washing down), and a rung you've just reached before the ones below it.
- **The pressing one overrides** wherever only one can be offered: the action menu's single
  *Anything need doing?* without the talk screen, and anything unasked. A duty is never called out,
  waved over, put on a board or sent as a radio ask: it's asked for.
- **A short gap, not the routines' cooldowns.** One kind of duty (the kitchen, in whatever faction's
  words) isn't given to the same player again for **Duty gap** minutes (20) after it's done. No
  per-place day, no 3 hours anywhere. Three duties may be live at one place at once, one per player
  there, and they take none of the place's chore slots.

Code: `Missions/Errands/RZ_ErrandDuties.c` (`RZ_Duties` the ladder, `RZ_ErrandDuty` the duties: one
object per kind and faction), `RZ_DutyGame.c` ([the sorting duties](#the-sorting-duties)), `RZ_DutySquads.c`
([their people](#with-their-people)), `RZ_DutyScenes.c` and `RZ_DutyFinds.c`
([things that happen](#things-that-happen)). Settings: **Missions > Errands > Duties** (on) and **Duty
gap**; the sorting duties' three optional props under **Missions > Errands > Props**.

### The trust ladder

Each duty has a rung with the giver's faction (`RZ_Duties.Rung`). Higher rungs are more sensitive
work, and pay more. Nothing new is counted: the rungs are the tiers that exist, plus one count.

| Rung | Who | The work | Pays |
|---|---|---|---|
| 0 | Watched or worse | No duties: the amends work only (Make It Right, The Latrine, Clean Slate) | — |
| 1 | Anyone else, from the first minute | Chores, watched: the kitchen, wood and water, washing down | 3–4 Favor |
| 2 | **They know your face**: three errands or duties done for that faction this season (any of them: a routine counts), or Known | The line at the gate, the dead, a watch, a patrol's short leg with their people | 5–6 Favor, 1 Fame |
| 3 | Known | Armed work with their people and the stores: a patrol's long round, a hunt, the sample bench, stock-taking | 7–8 Favor, 1 Fame |
| 4 | Trusted | [The sensitive ones](#the-trusted-rung): the lock-up, the names (the dead, the debt book, whose work it was), the trial list, who gets a dose, a cull, the animal house. Each once a place a season day | 10 Favor, 2 Fame |

The third piece of work for a faction says so (*"That's three pieces of work for the Haven. They
know your face now..."*). Pay goes through the small-pay ledger above, so a duty can make you Known
and never Trusted: that takes contracts, which a giver now makes on asking
([MISSIONS.md](MISSIONS.md#who-gets-offered-what)).

### Each run is drawn from the world

- **The task.** A duty has a few tasks and offers one a run, drawn when it's dealt (the kitchen:
  peel and chop, mind the pot, or serve the line), plus any way that's always there (put two tins
  in the pot).
- **The reason.** The ask says why this one, now, from what's so (`RZ_ErrandDuty.Reason`): the
  house's last meal ran short, someone died there this week, it's raining, it's dusk or night, the
  outbreak is past its early stage, the town is sick (it's named), the faction is short of food,
  the cure is past halfway (the bench). With none of those, one of two or three everyday reasons.
- **What's laid out, and what's out there.** A sorting duty's subjects and how many are bad, and
  what a patrol finds at each point, come from the world too (below).

### The duties

✅ built. A faction has each duty in its own words; the work is the same. A task is work with your
hands at a [work site](#work-sites) at the place (or behind it), then back to the giver.

| Duty | Rung | Their own work (roles) | Haven | Bastion | Red Hand | Halcyon | Tasks (one a run) and ways | Pay | Status |
|---|---|---|---|---|---|---|---|---|---|
<<<<<<< HEAD
| Kitchen | 1 | cook, storekeeper, farmer | Kitchen Duty | Mess Duty | Feeding the Crew | Canteen | Peel and chop (18 s) · Stir the pot (20 s) · Ladle supper out (16 s), both at the cook pot (**Stir the pot** is [minded up close](#minding-the-pot)). Or **put two tins in the pot**, or **a leg of raw meat** (a deer's, [DESIGN.md](DESIGN.md#wildlife)) | 3 Favor; the tins 4 Favor and +1 food at a house, the meat 4 Favor and +2 (a house is fed by duties twice a season day at most) | ✅ |
=======
| Kitchen | 1 | cook, storekeeper, farmer | Kitchen Duty | Mess Duty | Feeding the Crew | Canteen | Peel and chop (18 s) · Stir the pot (20 s) · Ladle supper out (16 s), all three at the camp's own cook pot, else at a kitchen table or stove with the pot ([Work sites](#work-sites)). Or **put two tins in the pot**, or **a leg of raw meat** (a deer's, [DESIGN.md](DESIGN.md#wildlife)) | 3 Favor; the tins 4 Favor and +1 food at a house, the meat 4 Favor and +2 (a house is fed by duties twice a season day at most) | ✅ |
>>>>>>> origin/claude/sleepy-volta-nwm433
| Wood and water | 1 | storekeeper, farmer, mechanic, technician | Wood and Water | Fatigues | Earn Your Keep | The Boiler | Split the wood (18 s, at the woodpile) · Fill the water cans (15 s) · **Carry the crates in**: three crates from outside, one by one, stacked indoors ([carried for real](#carried-for-real)) | 3 Favor | ✅ |
| Washing down | 1 | nurse, medic, lab assistant, clerk | Washing Down | Hygiene Detail | The Scrub | Decontamination, Minor | Scrub the tables (18 s) · Wash and hang the bedding (16 s, at the washing line) · Sluice the yard (18 s, a bucket) | 3 Favor | ✅ |
| The line | 2 | nurse, medic, lookout, sentry, guard, toll man | The Line at the Door | The Wash-Down | The Look-Over | Intake | [A sorting duty](#the-sorting-duties): the people waiting at the gate. Take each by the arm to **the holding room** (a bite, clouded eyes), **the wash** (a fever) or **through the gate** | Up to 6 Favor by how many you got right; 1 Fame if every one | ✅ |
| The dead | 2 | nurse, medic, clerk, cook, lab assistant | Laying Out | Burial Detail | The Pit's Leavings | The Cold Room | A sorting duty: the dead laid out. Carry each to **the pyre** (bitten), **the holding table** (papers that don't match the tag) or **a grave you dig** | Up to 6 Favor; 1 Fame if every one | ✅ |
| A patrol | 2; the long round 3 (*Needs Known with ...* on its reply) | lookout, sentry, guard, toll man | The Walk Round | Patrol Leg | The Round | The Sample Walk | **The short leg**: three points 70–140 m out and back, [with their people](#with-their-people). **The long round, armed**: four points 180–300 m out | 5 Favor, 1 Fame; the long round 8 Favor, 1 Fame | ✅ |
| A hunt | 3 | farmer, cook, lookout, toll man | For the Pot | — | The Meat Run | — | **Walk the snare line with the hunter**: 200–400 m out, the hunter following you, *Check the snares* (15 s) with them beside you. Or **hand over a leg of raw meat** | 7 Favor, 1 Fame, +1 food at a house; the meat 8 Favor, 1 Fame, +2 food | ✅ |
| The bench | 3 | lab assistant, technician | — | — | — | Bench Work | A sorting duty: the sample trays set out. Carry each to **the bin** (a broken seal, a turned colour), **the bath** (a label that doesn't match) or **the rack** | Up to 8 Favor; 1 Fame if every one | ✅ |

| A watch | 2 | lookout, sentry, guard, toll man | The Roof at Dusk | Stag | The Quiet Hour | The Gate Log | At dusk or night. [A scene](#things-that-happen): take the post, then three things come by, 25 to 45 s apart. Each is a threat, someone, or nothing, and its line holds the tell (*it stops at the marker and stands there* against *sits down on its pack*). **Stand the gate to**, **send someone out**, or **let it pass** | Up to 6 Favor by the calls got right; 1 Fame if all three. A false alarm is spoken of; a threat let pass is the town 2 more infected; someone sent out to a threat is morale 1 down; someone met on the road becomes a local. **Bastion's stag** starts with a sector (the road, the wood line, the gap in the wire) and four come by from that sector's own; **the Red Hand's quiet hour at night** is kept by ear, three sounds and the same three calls | ✅ |
| Stock-taking | 3 | storekeeper, clerk, mechanic, technician, toll man | The Larder Book | The Armoury Count | Counted Twice | The Cold Store | [A sorting duty](#the-sorting-duties), the fourth skin: crates against the book. *Count what's in it*, *Check the seal*, *Read the chit against the book*; carry each to **the counted stack**, **written off** (only the chit wrong) or **the query table** (short, or the seal cut: someone answers for it) | Up to 8 Favor; 1 Fame if every one. A theft passed as sound: 2 Infamy, morale 1 down | ✅ |
| The lock-up | 4 | sentry, guard, lookout, toll man, medic | The One at the Gate | The Stockade Watch | The Debtor in the Shed | The Isolation Room | [Below](#the-trusted-rung) | 10 Favor, 2 Fame | ✅ |
| The names | 4 | clerk, cook, nurse; Bastion: radio operator, medic, sentry; the Red Hand: toll man, mechanic, cook; Halcyon: lab assistant, technician | The Book of Names | The Casualty Return | The Debt Book | Whose Work It Was | [Below](#the-trusted-rung) | 10 Favor, 2 Fame | ✅ |
| The list | 4 | doctor, lab assistant, technician | — | — | — | The Trial List | [Below](#the-trusted-rung) | 10 Favor, 2 Fame | ✅ |
| A dose | 4 | nurse, clerk; Bastion: medic, sentry; the Red Hand: toll man, cook | The House's One Dose | The Sergeant's List | Who the Crew Pays For | — | [Below](#the-trusted-rung) | 10 Favor, 2 Fame | ✅ |
| A cull | 4 | lookout, sentry, guard, farmer | The Strays | Clearing the Wire | — | — | [Below](#the-trusted-rung) | 10 Favor, 2 Fame | ✅ |
| The animal house | 4 | lab assistant, technician | — | — | — | The Animal House | [Below](#the-trusted-rung) | 10 Favor, 2 Fame | ✅ |


### Minding the pot

The kitchen duty's **Stir the pot** is played at the real cook pot in the
[hands-on view](UI.md#the-hands-on-view) (`RZ_StoveHandsOn`): a short press on its prompt opens it.

| | |
|---|---|
| The fire | Yours: **More wood** and **Rake the fire down** (the panel's buttons, 0 to 4). The pot follows it slowly: cold, warming, simmering, boiling hard, boiling over |
| Cooking | Twenty seconds at a simmer, stirred lately and nothing stuck, and it's done: the step is done, as the hold was. The status line counts it |
| Stir it | A place on the pot: resets the clock. Hot and not stirred for 9 s, it catches (no cooking till it's scraped off with a stir); 16 s, it burns through and is started again (nothing lost but the time) |
| Taste it | A place on the pot: says what it wants (more heat, keep it so, calm the fire) |
| Boiling over | Too much fire: it spills, and the third time the fire is half drowned |
| Pay | The chore's own 3 Favor, nothing more |

Every turn is said in the panel's **What you found**. A client without the view holds the prompt as before.

### The sorting duties

The line, the dead, the bench and stock-taking are one game in four skins, **played at the place
with your hands**, never on a screen (`RZ_DutyGames`, `RZ_DutyGame`; the rule is
[UI.md](UI.md#default-first-then-menus), "Games are played in the world"). Every prompt in it is a
[work site](#work-sites): held, animated, your feet locked while it runs, and the world showing
the result. The server holds the truth and judges.

**How it plays, in three lines.** Three or four subjects wait in a row at the place (bodies, people,
trays). Each has **one prompt, its name** (*A courier*, *Tag 114*): it opens it up close in the
[hands-on view](UI.md#the-hands-on-view) (`RZ_DutyHandsOn`): the camera on the person, body, tray or
crate, its three places glowing where they are on it (a look is a short hold there, and one seen
shows again for nothing), everything said about it in the panel's **What you found**, and **Take
them by the arm** as its button (the prompt then waits for the 3 s hold). A client that can't show
the view gets the [object menu](UI.md#the-object-menu) with its three places to look, a 2 s hold each once picked
(*Look at the arms*, *Look at the neck*, *Check the papers against the tag*), and taking it up
(*Take them by the arm*, 3 s), under its tell, what you've seen of it, the looks left and the
rules. You have fewer looks than there are places (subjects + 2), so the tells say where to spend
them. Three **named places** stand within 10 m (*the gate*, *the wash*, *the holding room*), each a
thing that reads as what it is; carry one there and leave it (*Leave them here: the wash*): where
you leave it is the verdict. Right pays in full, too careful pays half, too careless pays nothing,
and a bad one let through is the dangerous miss.

| | |
|---|---|
| Subjects | 3, or 4 from the Collapse stage on. Each is **clean**, **odd** (the third place: papers that don't match, a fever, a label that doesn't match) or **bad** (one of the first two places: a bite; the line's clouded eyes; the bench's broken seal or turned colour) |
| How many are bad | Each is bad at 15% + the infection of the sickest town within 2.5 km ÷ 300 + 4% a stage (50% at most), odd at 22%. Never a row with nothing to find |
| The tell | One short line, said as you come within 4 m: calm (*Died in bed, family there*), odd (*The tag is written twice*) or risky (*Found in the street at Morton*: the sick town, by name). It points the right way three times in four |
| Looking | A subject's one prompt lists its places to look while it has looks left, then only taking it up. A look picked is held at the subject (2 s, the hands animation) and said as a line (*A bite on the arm, wrapped in cloth. 3 looks left.*); it's gone from the list for good. At the line in the rain: one look fewer. Without the object menu (UI > Object row layout empty) the three places stand as their own prompts beside it while you're within 5 m, as before |
| The rules, where you are | Never a note to go back to: the rules (*A bite or clouded eyes goes to the holding room. Only a fever goes to the wash. All clear goes through the gate.*) are at the foot of every subject's menu, in the line when you take one up, and in the tracker under the step with the looks left |
| The places | Within 10 m of the row, each with its thing standing, a vanilla prop measured in `tools/camp_kit/footprints.json`, set where its box is clear, level within 0.5 m and apart from the others (10 tries; one that won't fit is dropped and logged, and its place is still there): the line's gate a road barrier, the wash a shower stand, the holding room a white tent; the dead's graves a grave, holding table a medical table, pyre a woodpile; the bench's rack a long table, bath an old sink, bin a rusty drum; the stores' counted stack a pallet pile, write-off pile a crate, query table a long table. Gone when the sorting ends |
| Taking and leaving | One at a time. While you lead one, each place has its prompt in front of its thing (*Leave them here: the wash*; *Bury them here: the graves*, 8 s with the spade if you carry one; *Set it down here: the bin*), the tracker's line names the three with their distance and way (*The gate: 8 m NE.*) and its map point sits between them. Each one left stays where it was put (the next grave beside the last), by the work sites' own looks |
| The line at a gate | The people waiting stand at the place's real way in: the faction place's own gate within 120 m (its road entry), else the road where it comes into the town (the road point 25 to 120 m from the giver farthest from the town's middle), queued along it outside, facing in. The barrier is beside the road at the gate, the wash and the holding room 6 m inside, either side. With neither, the giver's own door, and the log says so (`Duty sorting: the line for errand N stands at ...`) |
| Who waits | People in the line each wear their own clothes: **Props > Duty person looks** (optional) by the errand and their place in it, else three of vanilla's civilians and the Duty person, so no two side by side are the same |
| The clipboard | *Read the clipboard*, by the row: the goal in three lines, the looks left, and each subject with its tell, what you've seen and where you put it, on a sheet in [the reader](UI.md#subtitles-and-the-reader). There if you want it; the menus and the tracker say all of it |
| The verdicts | In order of care: pass (the grave, the gate, the rack), check (the holding table, the wash, the bath), stop (the pyre, the holding room, the bin). The one that matches is **right**; a more careful one is **too careful**; a less careful one **missed**, or **let through** if the subject was bad |
| The score | Judged when the last one is put down. Right 1, too careful ½, the others 0, over the number of subjects. Favor is the way's × the score (1 at least); Fame only for a perfect row, which is also a record in the world's memory (`duty.<kind>`, the giver's name) and the giver says so |
| The dangerous miss | No pay. 2 Infamy with the giver's faction, and the place pays a little: the town is 2 more infected (the dead, the line), or the faction's morale is 1 down (the bench, the stores). It comes out when you report, or when the errand ends any other way |
| Leaving it | Nothing is lost: what you looked at and put down stays until the errand lapses. Walk back and go on |
| What stands there | **Props > Duty body**, **Duty person** and **Duty tray** (optional): what waits in the row. Empty: Props > Effects, Puzzles > Civilian and Props > Crate. Each is a work site's look, up only while a player is near |
| What's on show | **Things** (optional): display copies of Suspicious Item Props' things, a mesh with nothing to pick up (`Prefabs/ReforgerZ/Work/Things/RZ_Show_*.et`), on the ground in front of an open beat and gone when it closes. Only what's in plain sight: the swap table's cash, pearls, travel papers and aspirin; the count's gold bar, pearls, bank cards, banknotes and medicine; the toll's second set of papers; gate duty's passports and soldier's identity cards; the medic's open kit at kit inspection; the dead one's card on the roll call's blanket; a blindfold at the lock-up's door. A toll's pack or a kit's bag isn't shown before it's looked in, and the incinerator's bags are sealed. The lock-up's night watch also stands JTF Bronze Props' holding pen beside the door (the Haven, Bastion) or its interrogation room's table and lamp (the Red Hand), from the first beat until the errand ends. Empty slots: words only (`Missions/Errands/RZ_DutyThings.c`) |
| No work sites | With **Shared > Work sites** off there is nothing to handle: standing at the place sorts every one the careful way, for half pay and nothing dangerous |

### With their people

A patrol and a hunt go out with the place's own people (`RZ_DutySquads`): one
[escort block](DESIGN.md#mission-building-blocks) each, so data until a player is near, a body
only then, and inside **Contracts > Max escorts** (with no room on the server the way isn't
offered; with none left when it's taken, or no ground for a round, you're told to do it alone).
Who goes: the faction's holding guard (**Factions > Territory**), else Bastion's squad or the Red
Hand group, else the survivor group; with none of those, a figure that moves as data.

- **A patrol** sets off from the gate when you take it, walks its points in order round the
  compass and comes home. They walk only while a player is within the escort leash (150 m), so you
  walk it with them. At a point there is either something off the path to go and see to
  ([below](#things-that-happen)), or one line about what's there, from the world as it is at that
  spot (`RZ_DutySquads.Found`): the infected within 80 m, the noise off a sick town or the quiet of
  a clean one (by name), tracks in the rain, a light at night, smoke at dusk, what the late stages
  leave on the road. The step is done when they're home and you're within 40 m of them.
- **A hunter** follows you out to the snare line, where something is found first
  ([below](#things-that-happen)). The hold there counts only with the hunter within 40 m, so you
  wait for them. Then home.
- **If they're killed**, you're told to go back and say so: no pay, the faction's morale 1 down,
  and the giver's line. Nothing else is taken from you.

### The Trusted rung

For the Trusted. Each is done **once at a place on a season day**, pays 10 Favor and 2 Fame through
the ledger, and decides something about people who have names: drawn from the place and the day,
so the same watch has the same prisoner all night, and what you decide is written in the world's
memory, said as a rumour, or remembered by a local. All are [scenes](#things-that-happen) played in
person, alone: nobody of the place is watching.

| Duty | True when | How it plays | What it decides |
|---|---|---|---|
| **The lock-up** (all four) | Dusk or night | Take the watch at the door by the gate (5 s). After half a minute the one inside speaks through it: the deserter who wouldn't fire on a road, the debtor with a girl at the Haven, the one who's frightened and not bitten, Halcyon's subject whose readings are normal and nobody reads them. Then three things you can do at the door | **Keep the watch**: paid. **Write down what was said**: reported, recorded, 1 more Fame, and it changes what happens to them. **Open the door**: no pay, 3 Infamy, morale 1 down, and the giver knows whose watch it was; the one you let out becomes a local who remembers |
| **The Book of Names** (the Haven) | The house has dead (`m_iDied`) | A door for each of the house's dead, three at most, by the gate: who's inside doesn't know yet. Then the book by the stove (*Write the names in the book*, 6 s) | At each door: **tell them all of it** (they ask for you at the burying), **tell them gently** (they hear the rest from someone else), or **leave it for the clerk** (2 Favor less, the giver's regard down). The book: recorded, morale +1, a rumour that the names were read |
| **The Casualty Return** (Bastion) | From the Collapse on | Three bunks. What was left at each says what became of the man: kit squared away and a letter half written; kit here but rifle and water gone; the locker empty to the paint, or his tags on the pillow. Return each **killed**, **missing** or **gone of his own will**, then sign | Scored: pays by the calls got right. A deserter returned killed is seen at a toll after the letter was written; a dead man posted absent is known to his section (morale 1 down); a deserter posted right is a rumour at every gate |
| **The Trial List** (Halcyon) | The cure 40% made, and not out | Read the chart at each of three beds (the one most likely to live, the one the trial learns most from, the night technician's brother), then write one name on the list by the door | No right answer. The name is recorded and goes round as a rumour naming all three. The one who'd live: morale +1, and the director calls it sentiment. The useful one: 1 more Favor, and the ward goes quiet. The brother: the technician becomes a local who remembers. It never touches a dose or the cure's count |
| **The Debt Book** (the Red Hand) | By day | Three names in the boss's book, a door each near the camp. What's there says the truth: hiding it (tins under a board, a new padlock), can't pay (an empty larder, the last chair in the grate), or paid and the book never heard (a chit with the boss's thumbprint, the red thread's paid knot). Mark each **owed** (the collectors call), **written off** or **paid**, then press your thumb under the lines | Scored, like the return. Collectors sent to a house that can't pay take the stove, and the lane knows whose book it was; a debtor rightly written off becomes a local who remembers; a paid one rightly closed is a rumour that the crew was collecting twice; a wrong "paid" is twelve tins out of the camp's pot |
| **Whose Work It Was** (Halcyon) | The cure 20% made, and not out | The week's bench report names who did each run. Three desks: notebooks in one hand from the first line to the last (the work), a neat hand on the labels and nothing else (help), or a clean desk with the name signed under someone else's pages (credit taken). Place each **first**, **thanked below the line** or **left off**, then initial the report | Scored. The one who did the work placed first becomes a local who remembers; the one who took credit left off is a rumour; anyone else left off costs morale. It never touches the cure's count or HC3's paper ([QUESTLINES.md](QUESTLINES.md#hc3-authors)) |
| **A dose**: The House's One Dose (the Haven), The Sergeant's List (Bastion), Who the Crew Pays For (the Red Hand) | The lab has a dose in stock (the Red Hand: the black market has one), and the cure not out | Three bitten in beds, all of whom asked for it (consent, as [FINALE.md](FINALE.md) has it for every dose): the one the place needs most, the one with most life ahead, and a third (the old one who asked to be passed over; the boy not on the strength; the boss's sister's boy). Read each bed, then write one name on the card in the case, or **send it back** | No right answer. The dose comes off the stock when the card is written (the lab's, so Halcyon's quartermaster has one fewer to sell; the Red Hand's from the fence's shelf) and goes to the one named: **never to the player**. Recorded and a rumour naming who had it. The one needed most: 1 more Favor. The most life: morale +1. The Haven's old one: the giver's regard down; the others' third: a local who remembers. If the stock emptied first, the case is empty at the bed. Sent back, the stock keeps it |
| **The Animal House** (Halcyon) | The cure not out | Three cages, drawn from six animals (a rhesus monkey, a rabbit, rats, a beagle, a goat, mice). At each, two of three looks (the card, watch it, hold out food), then **put it down**, **keep it for the trial** or **back to the clean room**: turning, the dose working in it, or the card on the wrong cage. Then sign the log | Scored. A turning one kept gets out: the town's infection up and heat at the lab |
| **A cull** (the Haven, Bastion) | The town 30+ infected, and room for an escort | Out armed with their people to one point 70 to 120 m off. There the strays come, up to five, as the spawner has room, 40 to 60 m out. They stand with you until none is within 90 m (three minutes at most), then home | The town is 2 less infected. If their people are killed: no pay, morale 1 down |

### The factions' own duties

Past the duties every place has, each faction has five of its own. About three in five are
**worked out and led**: you gather what you can see and hear, in person, and the call is yours,
right or wrong, with a consequence either way. The rest are simple, physical and quick. All are
[scenes](#things-that-happen): beats played with your hands, drawn from pools so a run doesn't
repeat soon. Two things the beats can do that a patrol's finds don't:

- **Places to look, and fewer looks than places.** A beat can have several things to do, each at
  its own spot a few metres out, with only so many allowed before it moves on (the roll call: five
  places, three looks).
- **Up close.** A beat whose things to do stand at one spot (an arrival at the decon line, a bag at
  the incinerator, a bed on the ward, a kit, a thing off the tarp) opens in the
  [hands-on view](UI.md#the-hands-on-view) (`RZ_BeatHandsOn`) on what stands there: its looks are
  glowing places set out across it, its calls the panel's buttons (places too, when there are more
  than three), and every line said out here in **What you found**. Choosing up close is the same as
  the prompt's hold; the next beat at the same spot opens up close by itself. A beat with someone to
  talk to stays the talk.
- **A call that's scored.** The beat that follows asks what you make of it; the right answer is in
  what you saw. The scene pays by the share of calls got right, and Fame only when every one was.

#### The Haven's duties

| Duty | Rung | Kind | True when | How it plays | What it changes |
|---|---|---|---|---|---|
| **The School Room** | 1 | Simple | By day | Count the heads (eleven, and the slate says twelve). Four places a child hides, each 9 m off: look until you find them. Then why they won't come out, and what you do: sit with them, carry them in, or leave them and tell the teacher | The giver's regard; the child asks for you after |
| **The Garden** | 1 | Simple, and worked out when a row is dying | By day, at a house | The house's **four beds are kept between visits** (below). This visit does what the neediest bed needs: dig it over, plant it from the seed tin (your choice of three), carry water to it from the well, or lift what's ripe and carry it to the kitchen. Then **one thing happens**, from eight. With **Things > Bed, dug / planted / ripe** slotted, the bed stands as what it is in front of you while you work it | A harvest feeds the house (potatoes +2, beans +1, onions +1 and morale); what happens can give meat, a local, a rumour, regard either way |
| **The Door Round** | 2 | Worked out | Dusk or night | Four doors, each 18 m off: knock and listen at every one. One has someone living behind it who hasn't said so (a kettle just lifted off the heat, a curtain an inch further across, small wet footprints going in). One has the dead behind it. Then chalk one door for the house | Scored. The right door: reported, and a face at the glass asks you not yet (leave the chalk: a local; rub it out, shady: a local who'll come in when they can). The dead's door chalked: morale 1 down, and you're told |
| **The Roll Call** | 2 | Worked out | Always | The list has twenty names and the kitchen fed nineteen. Five places to look (the coat pegs, the gate book, the beds, the bowls, the children) and time for three. Then write against the name: out and not back, dead in the night, or gone with their things. Written out and not back, two go along the wood road to look, and you can **go with them**: in a ditch with a turned ankle, in a barn loft too frightened to come down, or at the wood's edge, bitten (or nothing at all, if the name wasn't out) | Scored, recorded. If it was a death, a dead man's identity card is on the blanket: hand it to the clerk, or keep it (shady). The search: someone brought home is 2 more Favor and a local (morale up, or regard up); the dead brought back is recorded and morale 1 down; nothing found, where you wrote them out, is a wrong call |
| **The Swap Table** | 3 | Worked out | By day | Four come to the table one at a time, drawn from sixteen, each with something laid down and something they want (a wedding ring for two tins, a child with a dead man's watch, a man selling his coat in the cold, antibiotics that are aspirin, a spoon its owner recognises across the room, a man cornering the candles, a boy trading for his grandad, a Red Hand face buying for the camp, banknotes for a loaf, pearls for a place on a list, forged travel papers...). You may do two of three things first (turn it over in your hands, ask where it came from, ask who it's for), then call it: take the trade, refuse it, or find another way. Each trader stands at the table to be talked to, with a conversation of their own (all sixteen, `dialogue/duties/swap.rzd`): asking is a look, and the call is made in the talk | Scored: the right call is in what you learned. A trade taken feeds the house; the other way, when it's right, makes a local who speaks of it after |

**The garden's beds.** Four a house, saved with the errands' counters: a bed is fallow, dug,
planted or watered, with its crop and the day it went in. Potatoes and onions are ripe two season
days after planting, beans one; a bed must have been watered. What happens, one a visit: crows (run
at them, or hang a tin), rabbits (a snare: a leg of meat in your pack; or mend the fence), **a row
dying** (three looks: the leaf, the root, the soil; then say whether it's blight, wireworm or slugs,
and do what that needs), something dug up (a tin box, bones laid straight, a greased rifle, a child's
tin of things: hand it in, or not), frost coming at dusk (cover the beds, or lose the beans), a
stranger in the carrots (let go, bring in, or give a tin as well: a local each way), a quarrel over
whose bed it is, the well low.

#### Bastion's duties

| Duty | Rung | Kind | True when | How it plays | What it changes |
|---|---|---|---|---|---|
| **The Loudspeaker Tape** | 1 | Simple | Always | Out to the speaker on its pole, 60 to 140 m off: open the housing, change the tape, and run the test (it's loud: noise where you stand). One of five things about the old tape or the pole: a soldier's voice saying names under the warning, a wren's nest in the horn, the battery stolen, one of the infected standing under it listening, an answer chalked on the pole by people in a mill | A tape handed in or names spliced into the loop (shady: a rumour); a found hook where the chalk says; the town 2 less infected |
| **Gate Duty** | 2 | Worked out | By day | Four come to the barrier, from fourteen. For each you may do two of three things (hold the stamp to the light, read the dates and the name, ask their business), then **lift the barrier**, **turn them back** or **hold them for the sergeant**. Some papers are forged (an eagle with one wing longer, today's date in the stamp's own ink, a potato stamp); some true papers are in the wrong hands (a missing man's identity card, a pass for one held by five, a warrant from a Command that doesn't answer); some false ones should pass (a nurse a week out of date, a priest with a dead bishop's letter). Each stands at the barrier to be talked to: asking their business is a look, and the call is made in the talk (one conversation for all fourteen, and six with a word of their own besides, `dialogue/duties/gate.rzd`) | Scored. Someone wrongly passed is 1 Infamy; someone held is reported; the giver says what came of the ones you got right |
| **Stag at the Wire** | 2 | Worked out | Dusk or night | [The watch](#the-duties), Bastion's: four things come by where the others have three | As the watch |
| **Kit Inspection** | 2 | Worked out, with a follow-on | Always | Three kits, from fourteen. Two of three looks at each (count it against the list, look down the barrel, go through the pack), then **pass the kit**, **write him up** or **deal with it quietly**. What's wrong varies: a magazine, the radio's spare battery, the section map or a morphine syrette missing; a private's letters and tags packed to go; a fouled barrel; a round in the chamber; a serial on the lost list; a bottle, a brick of cigarettes, a forged pass, a Red Hand armband. A **missing** thing, written up or dealt with, opens the search: go and find it, or leave it at a line in the book. Where it truly went, this time, is one of five: under the end bunk, in the grass where the section went to ground, sold over the wire to a toll man, traded to a man down the lane for bread, or gone with the man who had it, who is sitting on a milestone. Then clean the spare rifle. Sold to a toll man, you can **go and buy it back yourself** at a real Red Hand pole within 2 km (else one far out): two tins from your pack, or leave it there | Scored. A clean kit written up is morale 1 down. The search found: 2 more Favor. Bought back: 3 more Favor, reported, regard up. What you cover for makes a local, or a line later; what you report is spoken of |
| **The Field Telephone** | 3 | Worked out | Always | Test the line at the post, then out to the break, one of ten (a fallen pine, chewed, cut clean, thirty yards taken for snares, a junction box under water, a dead man wound in it, one of the infected still walking against it, a second wire twisted on, a cottage where someone sits with an earpiece, frost). Two of three looks (the wire's ends, the ground, along the wire), then say it **broke**, was **cut** or was **tapped**. Splice it and ring through: the far post, nobody, a voice that isn't ours (hang up and report, or keep them talking), or breathing. Then, if you like, run new wire to the further post | Scored on the reading of the break. Cut or tapped is reported. The further post wired: 2 more Favor, morale +1 |

#### The Red Hand's duties

| Duty | Rung | Kind | True when | How it plays | What it changes |
|---|---|---|---|---|---|
| **Stripping a Wreck** | 1 | Simple | By day | Out to a car in the ditch, 60 to 140 m off: wheels, battery, seats, each by hand. Something slides out from under the driver's seat, one of six (a pistol in parts, a child's shoe, a bag of banknotes, a tin of photographs, something knocking in the boot, a road map with our tolls priced on it). Then load the handcart | What was under the seat: handed to the camp (reported), kept (shady), or let be |
| **The Still** | 2 | Simple, by hand | Always | A potato, plum or beet mash: load it, light and feed the fire, then **the needle**: it's climbing past the band, or falling out of it, and you damp the fire or feed it (the wrong one boils it over, or lets it go cold). Then **the cut**: pour the first runnings away, or keep it all. One of eight things happens round it (a hand feeling along the shelf for a bottle, smoke going straight up, the boss wanting it stronger, a Haven man at the fence wanting spirit for wounds, the coil cracked, another crew asking the method, the woodpile wet, one of the lads wanting a taste before the gate). Then bottle it | Scored on the needle and the cut. The cut skipped: morale 1 down, and two of the lads can't see straight. **A bottle is yours** (Props > Duty vodka: BanditGear's bottle; empty: no bottle). What you do for the Haven man or the boy makes a local |
| **The Count** | 2 | Worked out | Always | Four things off the tarp, from fourteen (tins, boots, a gold bar, pearls, bank cards in six names, banknotes, medicine, a rifle with the boss's wax thumbprint, a Bastion radio, a station master's watch, a child's coat, spanners, blank stamped passes, a signet ring). Two of three looks (turn it over, weigh it, check it against the tally), then **the crew's pile**, **the fence** or **the boss's pile**. The tally's own hand names who entered each line, and one of three has been rubbing lines out. Then the tally is one short: name him | Scored on the piles and on the name. The right name is reported; the wrong one takes a beating for it (morale 1 down) and knows whose word it was |
| **The Toll** | 2 | Worked out | By day | **Five come up the road, drawn from fifty-one** by what's so: the outbreak's stage, the hour, rain, how sick the town is, and who's at odds and who holds the road (a feud with the Haven or Halcyon keeps their courier or technician off it; a Bastion post within 1.5 km or a truce brings Bastion's soldiers, papers and crier up it, a feud keeps its soldiers off and brings its patrol to the bar) (a family with a handcart, a Haven courier with medicine, a deserter with his badges cut off, a Halcyon man with a case chained to his wrist, a child alone, an old man with nothing but news, a man with his hand in his coat, another crew's runner, a keeper with a shotgun, a cart of the dead, forged Bastion papers, a man who'll work it off, one who says the boss knows him, a coffin that is heavy for its size...). Two of three looks (what they carry, their hands and boots, where they're bound), then **take the toll**, **wave them through** or **turn them back**. Each stands at the pole to be talked to: asking where they're bound is a look, and the call is made in the talk (one conversation for all fifty-one, and twelve with a word of their own besides, `dialogue/duties/toll.rzd`). Then one chance to do more, from four: walk a merchant through to the far marker (he walks beside you, a body with his mule's worth of nerves, until the marker), run down one who bolts before he reaches the trees (50 s, alive), follow one who turns off into the wood, or hold the bar when a Bastion patrol comes up. Then the boss counts the box | Scored on the five. A toll rightly taken: 1 more Favor. The extension: 2 more Favor and 1 Fame. The box handed over one thing short is shady |
| **Spotter** | 3 | Worked out | By day | Up a hill 180 to 350 m off with the glass. One of five things to watch (the road, a Bastion post's gate, the Haven's truck road, Halcyon's people at a well, another crew's camp). Three watches, 20 s apart, each saying one thing seen. One of five things happens up there (your glass flashes: **lie still** for 20 s, or run and be sure of; cloud comes down; another crew's spotter climbs your hill; you see one of ours hand a package to one of theirs; someone who gave you soup walks toward what's planned). Then the slate: one line, of three, that the crew can use | Scored on the line. The right line is recorded, goes round as a rumour, and is kept as the camp's board for that target (a saved counter with the day). The infected's ground at dusk isn't one of the targets: that's the routine Rent's Due. **The crew acts on it**: at the camp's next toll within two season days the boss says what the crew did on the mark (the carts taken, a cart walked past Bastion's barrier, a third off the Haven's truck, Halcyon's handcart, the other crew a hut short), morale up and a rumour, and the mark is used. Simplified: what the crew did is said, not played; no convoy or truck moves |

**The toll's numbers.** Fifty-one passers-by are written; twenty-eight can come any day, and
the rest only when the world says so (six from the Collapse on, three before it, six while the town
is 40+ infected, three in the dark, three by day, two in rain). A shift is five of those, one
extension and the boss: about ten minutes.

#### Halcyon's duties

All five are worked out and led.

| Duty | Rung | True when | How it plays | What it changes |
|---|---|---|---|---|
| **The Sample Round** | 2 | By day | Four marked spots, each 14 m out (the pump handle, the market trough, the church hall door, the mill race): a swab at every one, and the strip stays white, goes pink slowly, or goes red before the tube is capped. Then one mark on the station's map: where it's coming from. Then one of three things (a child with a cup at the spot you marked, a face at a window asking what the suit has found, a tear in your own suit) | Scored. The right mark: reported, and the town 2 less infected. What you tell the man at the window is a rumour either way |
| **The Decon Line** | 2 | Always | **Five arrivals, drawn from fifteen, with five faults, drawn from twenty-five, shared among them** (one may carry two, one none). For each, two of three checks (the lamp over the suit, the seals and the visor, the case and the log), then **pass them through**, **run them through again**, **send them to the room**, or **report it to the director**. A fault shows only at its own check, and the worst one decides the right call: a splash on a boot is a re-run, a cracked visor or a bite pressed into the forearm is the room, a tube missing from a case or a kitten asleep in one is a report. **The tank and the drum run short by count**: the tank holds four rinses and the drum six sprays; passing someone uses one of each, running them through again two of each, the room one rinse. When either is empty the line stops after that arrival: carry water up (or mix a new drum from the store, 1 more Favor), or go on without (the town more infected) | Scored. A fault passed: the town 2 more infected and 1 Infamy. A clean one held or reported: morale 1 down. Carrying the water: 1 more Favor |
| **The Incinerator** | 2 | Always | Four sealed bags, from twelve. Two of three checks (the label against the log, the weight, the feel through the bag), then **the fire**, **set aside and reported**, or **cut open**. The label, the weight and the feel don't always agree: clinical waste that's a man's weight with a wristband under the plastic; expired cultures with one tube warm; a bag that was at the other end of the trolley a minute ago; sharps that are a pound of wedding rings; a canvas bag of rifle rounds; sealed, in-date morphine written off as expired; the director's files. Then the log is one short, and a porter has one more, off the book | Scored. What's set aside is reported. The porter's bag: burn it (shady), refuse it (reported), or open it: every letter the wards have written since the road closed, to burn or to carry to the Haven's board (shady: a rumour, and the porter remembers) |
| **The Observation Ward** | 3 | Until the cure is out | Three beds, from eight conditions (a plain fever; the infection, early; the infection, turning; the trial dose doing it; nothing wrong and wants out; nothing wrong and wants the dose; fear; hunger). At each, **six signs taken by hand and time for four** (the temperature, the pulse, the torch in the eyes, the skin and any bite, how they speak, the chart at the bed's foot). Then what it is (the true one among three), then what's to be done (fluids and cooling, isolation and the doctor, restraints, the doctor, the truth, a meal). Then the researcher at the door, who'd like the fevers written as responses | Scored on each diagnosis and each treatment. **A turning missed** (written as anything else, or not restrained) sits up behind you: morale 1 down, the town 2 more infected, and you're told. Charts written true: recorded, regard up. Written his way: shady, 1 more Favor. **A round done well counts**: every diagnosis and treatment right and the charts written true adds 1 research to the cure (as a mission's research, inside the backlog's room), once a place a season day |
| **The Cold Chain** | 3 | Always | Take the cold box (a case stands at the prompt: Props > Duty tray, else Crate, else the base game's cardboard box). One of six cargoes, each with one thing it can't bear: trial vials (no waiting), blood samples (not opened), a culture in a flask (no rough ground), four doses (they arrive counted), sealed results (not opened), the heavy cold box (no rough ground, never set down in the wet). Two things happen on the road, from eight (a corporal who wants it opened, a toll bar, a man on his knees for one dose, the cold pack split, the bridge out, something keeping pace in the hedge, rain like a tap, a man in a good coat who'd pay to look), and each offers three ways through: one opens it, one costs time, one is rough. Then the far door, one of three (the power out, nobody there, the count one short), and the manifest | Scored: a way the cargo can bear is right. With the doses, giving the man one of the four is wrong, and he remembers it; telling him which door is right. Nothing in the box is ever the player's. **The rules are felt on the way**: a cargo that can't wait warms after six minutes on the road, and one that can't bear rough ground rings at a sprint, a jump or a fall (a ride counts too); each is a wrong call, two jolts at most, and is said once |

**The decon line's numbers.** Fifteen arrivals and twenty-five faults are written. Nine faults
show under the lamp, eight at the seals and the visor, eight in the case and the log; ten are a
re-run, nine the room, six a report. One run is five arrivals with five faults among them, two
checks of three on each: about ten minutes.

### Things that happen

The author's rule: *"something interesting should happen or see something or whatever. Doesn't
just have to be shooting."* A duty has things on the way to see, find and decide, in person
(`RZ_DutyScenes`, the beats in `RZ_DutyFinds`). A **beat** is a moment at a spot: one line for what you see as
you come within 9 m (the spot's menu text when its things are one prompt; a card only when there's
no menu and nobody to say it), and one to three things you can do about it, each a [work site](#work-sites),
a hold with your hands. **Several at the one spot are one prompt** (the rule:
[UI.md](UI.md#default-first-then-menus), "One spot, one prompt"): its name (*The far door*, a
patient's name), opening the object menu with each thing to do as a row; the row picked is then
that prompt's hold, so it's still done with your hands. A thing set out at a place of its own to go
and look at (more than 3 m off, a patrol's finds) keeps its own prompt there. What you choose is
applied at once and remembered by whoever it concerns.

| | |
|---|---|
| The spot's name | The beat's own name if it has one, else who stands there to talk to (*Someone at the barrier*), else what its line names first when that's a thing (*"The far door. No lamp over it."* gives *The far door*), else the duty's title |
| A row you can't do | One needing something from your pack you haven't got is greyed, with why (*You haven't got that on you: a tin of food.*) |
| Changing your mind | Walk off a few steps, or leave it a minute, and the prompt is the list again. With one thing left at the spot, it's that thing's plain hold |
| At the way in | What's said at a gate stands at the place's real way in (`AT_ENTRY`): the decon line, gate duty's barrier, the toll's bar and what comes to it, a watch's post. The faction place's own gate within 120 m, else the road where it comes into the town, else the giver's door, and the log says which (`Duty scene: errand N, <beat> at the way in: ...`) |
| Without the object menu | UI > Object row layout empty, or a player whose client couldn't open it: each thing stands as its own prompt side by side, as before |
| What a choice can change | The giver's regard (up or down); a **local** made at the nearest house who remembers you; a line the giver says to you some hours later (a callback); a record in the world's memory; a rumour for 6 hours; the place's food or its faction's morale; the nearest town's infection (by 2); noise where you stand; a found hook for the [strangers](#strangers) (someone hiding there); a thing in your pack (a leg of meat, a post's ration, Props > Watch); the job's Favor; small Fame; Infamy |
| A find reported | Each find put in the report is the giver's regard up, and 1 more Favor for the job (twice at most). What you reported can come back as what the giver says when you hand in (*"They brought that truck's load in this morning. That was yours."*) |
| A shady choice | Pocketing, or keeping something from the people you work for, costs only **if it's seen**: their people within 20 m of you, or the place itself within 25 m. Seen: it's undone, 1 or 2 Infamy with the faction, the giver's regard down, and they say so when you report. Unseen: it's yours, and nobody knows |
| The pay | The choices' Favor is added to the way's before the ledger's caps, 3 at most. A scored scene (each beat has a right call) pays by the share right, with Fame only when every call was |
| Where a beat stands | Beside the giver's place, at its gate, out on the road, or near the last beat, in the open. A beat whose words name a door stands on the doorstep of a real house's outside door (the door finder the locks use): the Cold Chain's far door 180–350 m off, a debt's door 12–60 m from the last; with no house's door near, any building's real outside door (up to 900 m and 300 m); with none at all, in the open, logged (`Duties: no real door within reach for beat ...`). No door is ever spawned for a beat. A thing to take (the Cold Chain's box) always stands at its prompt: Props > Duty tray, else Crate, else the base game's cardboard box |
| Finding the next beat | The errand's place moves to each beat as it opens, so the journal and the tracker point at it (no card saying which way: the marker does). A patrol's finds are called by its lead, and the squad's own steps point at them |
| No work sites | The first thing you could do at a beat is what's done, by standing there |

**A patrol's finds.** The patrol has a lead with a name (*Lance Corporal Petr Novak* in Bastion;
a man's name for the others), and **he says it all**: what's at a stop, what he wants you to look
at, his answer to what you chose, "Right. On we go." (a spoken line over him, with his name and
arrow: [WRITING.md](WRITING.md#spoken-lines-and-notices)). Out of earshot (60 m) or dead, it's a
card. At two points in three (and always at one), the patrol stops for a find **whose thing really
stands there** (the rule: [MISSION_RULES.md](MISSION_RULES.md), rules 2 and 3): at most 120 m from
the stop, its one prompt on the thing. A find further than 35 m off, they walk over and wait 15 m
short of it, in sight, then go on round from there. They wait until you've seen to it, or 150 s. A
stop where no find's thing can stand is left alone (`Duty finds: no ... left out`). One find a stop,
drawn from those true there:

| Find | True when | Its thing, where | What you can do |
|---|---|---|---|
| A door with a mark | A door mark near, in [Traces](TRACES.md#errands-old-mark)' grammar (its line read at your level), twice as likely | the mark's own real door, on its doorstep | Copy it into the patrol book (reported, recorded) · leave it |
| A fresh grave | The town 40+ infected, or a Traces kill-site scene within 400 m | a grave mound (`Grave_USSR_02`) in the open, 10–45 m from the stop | Read the name for the report · set the cross straight (the family asks who, later) |
| A radio still talking | A rumour is going round | a small radio set on a house's ground floor | Listen and write it down (a real rumour, reported) · switch it off |
| A house with a trodden path | Always | a real house's doorstep | Knock, and then a voice behind the door (said from the door): **report them** · **say nothing** (shady: a local who remembers) · **leave a tin** (1 food: a local). Or mark the house (reported; the strangers may meet whoever hides there) |
| Remains in the grass | Always | **Duty body**, else the base game's remains in clothes, in the open | Take the papers (reported; someone knew the name) · take the watch (shady) |
| A box under a loose step | Always | a red tool box on the doorstep of a real building's outside door | Hand it in unopened (reported) · open it and take what's in it (shady) |

*Taken out:* tracks, a dog, an infected in a pit, a truck, smoke from a chimney, another patrol and a
child's drawing: none had a thing that could stand there, so a stop for them was empty ground.

**The tracker on a patrol.** While you walk the round, the step is *"Walk the round with Lance
Corporal Petr Novak: point 2 of 3"* and the marker rides on him as he walks; stopped for a find, it's
the find's own step (*"Read the name on the fresh grave"*) with the marker on the thing, then *"Walk
on with ..."*; at the end, *"Come back in with ..."*, then back to the giver. Any escort that walks
a route has its marker on its lead the same way (`RZ_Escort.NowAt`).

**A hunt's finds**, at the snare line before you check the snares: a cat in a snare (cut it loose,
and it comes to the kitchen door after; or it's meat), a deer path crossing the line (set a snare
across it, +1 food; or leave it), or two snares robbed with a small boot print: wait in the hedge,
and a boy comes with a rabbit (let him keep it: a local; take it back: +1 food; bring him in: a
local, reported).

**A chore's small turn.** Four times in ten, once a chore's work is done, something comes up where
you worked, and the step waits on what you do:

| Chore | The turn | What you can do |
|---|---|---|
| Kitchen | The pot comes up a ladle short | Water it down · put in a tin of your own (1 food: +1 Favor, +1 food, regard up) · give up your own bowl (regard up) |
| Wood and water | The well rope has frayed, or the axe head is loose | Splice or wedge it (a longer hold: +1 Favor, regard up) · knot it and say so, or leave it (the axe comes off the day after, and you hear of it) |
| Washing down | The water has gone grey half way | Fetch clean and do it again (+1 Favor, regard up) · call it done (two are sick later, regard down) |

### When there's nothing: the reason, in words

With duties on, *Anything need doing?* is locked only for a stated reason (`RZ_Errands.NothingWhy`,
and the refusals in [Being asked](#being-asked)):

| Shown | When |
|---|---|
| *You've done your shift. Ask again in a while (20 min at most)* | Every duty you could have is on its gap |
| *You've a chore of theirs already. Finish that one* | You hold an errand or a duty at this place (a job, a quest or the opening's chain never counts) |
| *Every chore here is taken. Ask again when one is done* | Three duties are live at the place |
| *Needs Known with Bastion* (on a way's reply, not the ask) | The patrol's long round, below rung 3 |
| *Everyone has work in hand. Ask again shortly* | The server's 12 live errands are all out |
| *Nothing while you're Watched with Bastion, except to make amends* | Watched, and no amends work is true |
| *Nothing today. Ask again tomorrow* | A wanderer; or duties are off |

## The wanderers' asks

| Ask | Wanderer | True when | Ways and pay |
|---|---|---|---|
| **Before Dark** ✅ | the Doctor | 15:00–20:00, a town at 30+ infection within 1.5 km | Take something medical to the bitten survivor there before 20:30, or [Nurse or Doctor] dress the bite (45 s): 6 Haven Favor |
| **Wanted Poster** ✅ | any wanderer | you're Kill on Sight with Bastion or the Red Hand | Pay him (8 Favor), lean on him (a threat), or let him sell where you are (a hunter squad comes now, if one hunts you) |
| **Barter Chain** ✅ | three wanderers | as built ([MISSIONS.md](MISSIONS.md#unarmed-missions)) | Started from one of them when nothing else is asked; offered to that player alone, paid 2 Fame and 8 Favor; it left the random pool |
| **Ask About Cars** ✅ | the Mechanic | as built | His own slot, Ask about cars, and the Car Keys job |
| **Whoever Sits There Now** ✅ | the Courier | as built | His own slot, Ask for work: the letters |

## The cards

✅ all built. Once per player per season (taken), at most 2 offered a real day.

| Card | Family, giver | True when | Ways |
|---|---|---|---|
| **Sleeve Down** | house, a Haven named person | a house of 5+, a bite on the roads within 1.5 km in 3 played hours | Something for the fever [1 medical]; [Nurse or Doctor] ask to see their arm; tell the clerk. The truth deck draws flu or a bite (6 in 10) for that house, kept 3 season days; the same replies show either way. A bite kept quiet: the house loses someone in the night. Seen: declared, 2 Haven Fame. Told: walked out at dusk (a bite), or shamed (flu: their regard −1) |
| **A Word to {house}** | house, the clerk | this house short, another lived-in house within 1.5 km fed | Carry the word to the other house: as it was said [Known], softened [Trusted - Haven], or with your own threat [Feared]: 5 Favor; which you chose is a world record |
| **I'd Rather Be Wrong** | house | a house with a medic, the lab, before the cure; you're a Nurse, Doctor or Lab Tech | Go through the numbers [Doctor or Nurse], read them the labels [Lab Tech]: 3 Favor. Or agree |
| **A Word for Me** | house | Champion with the Haven; the house's head alive | Speak to the head (10 s beside them): a world record, no vote moved. Or say you're no one to ask |
| **The Truck Didn't Come** | aftermath, the storekeeper | this house's truck lost within 1.5 km in 2 played hours (never to whoever robbed it, or their group) | Salvage the wreck (search it: hold, 9 s): 4 Favor, +2 food. [Known: Red Hand] ask a toll lad who: 3 Favor, a rumour naming no one |
| **The Smoke Was Ours** | aftermath, a Bastion named person | shells within 1.5 km in 2 played hours (never to whoever called them) | Count at the impact (10 s): 5 Favor. Two rations to the nearest house: 1 Bastion Fame, 1 Haven Fame, +2 food there. Tell the Haven whose shells: a world record |
| **Is It True?** | aftermath, a Haven named person | a rumour going round names a town within 1.5 km | Go and look (15 s): 4 Favor. Tell them it's true: the rumour runs 6 hours longer, no pay |
| **The Letter Nobody Can Take** | lost, the Courier | a house that lost people lately, another lived-in house within 1.5 km | Carry it there: 4 Haven Favor. Or give it back |
| **The Trader's Detour** | lost, a Trader | his destination's town at 60+ infection, or a mission there | Scout his road ahead (20 s): 4 Haven Favor; he remembers you helped |
| **The Deserter's Way Round** | line, the Deserter | he likes you, a Bastion road post within 1.5 km | Meet him past the post (10 s): 4 Haven Favor; helped. Turn him in at the post: 2 Bastion Fame, a rumour, a world record |
| **Nothing Goes Out** | line, a Bastion named person | the perimeter sealed, a sealed checkpoint within 1.65 km, Bastion morale under 50 | Leave his letter under a stone at the warning point, 150 m short (5 s), read or not: 3 Favor; the world record `errand.letter_at_line` with its epilogue line (carrier: a prop) |
| **His Effects** | dead, any faction's named person (Bastion: The Locker; others: The Wreck's Name) | that faction's truck lost within 1.5 km in a season day (never to whoever robbed it), else Traces' truck wreck (L5, a driver's papers) or fallen post (V7, a soldier's tags) within 1.5 km; once per dead man, 3 times a life (not once a season) | The effects are a **carried item**: gathered at the wreck (15 s), only you or a helper can hand them in. Bring his things back: 5 Favor, 1 Fame; Bastion's boxes "for when the gates open" as an epilogue line. [Known: Red Hand] sell them at the fence: the jacket 3 and the photo frame 10 Red Hand Favor, a record "sold"; seen by the giver's people (the witness rule), +5 Infamy with them. A kit piece off a man you killed is refused (*"You'd know. You were there."*, +3 Infamy with his faction) |
| **The Name on the Tag** | dead, the clerk | a death within 1.5 km in a season day (the background and how long they lasted, never the name), or the fallout of a Distress Call or Survivors Trapped whose people died | Leave the satchel on a cairn where they fell (10 s): 4 Favor. Or pin the line to the board. A fallout's **named body** at the job's site: take the ID (10 s, a carried item) and carry it in yourself: 4 Favor from the faction it names (a Red Hand crewman's: thanks only, crews don't bury their dead). A world record with an epilogue line (carrier: the board) |
| **Not for the Crew** | crews, a Red Hand named person | a bite within 1 km in the last played hour, a Halcyon station within 1.5 km, before the cure | Walk him to the station: 4 Halcyon Favor, a world record. Your own antidote: 6 Red Hand Favor. Tell the book-keeper: 2 Red Hand Fame |
| **Wildcard Money** | crews, a Red Hand named person | Wildcard or Tolerated with the Red Hand; another Red Hand holding within 1.5 km | Carry the cut there: 6 Favor; skim a little: 8 Favor; give it to a hungry house within 1.5 km: +2 food there, 2 Haven Fame, +8 Red Hand Infamy |
| **Clean Slate** (amends) | any faction's named person | that faction Watches you | The worst shift going: dig at the burial pit (hold, 15 s): amends |
| **Dress Rehearsal** (disguises) | crews, a Red Hand named person; Known with the Red Hand, below Known with Bastion; disguises on | a Bastion road post within 2 km | Walk up to it in Bastion kit and pass the look, 10 s at the barrier, and walk back: 10 Favor, 2 Fame. At the barrier and not passing: it's over, and the open channel laughs at someone in borrowed green (no name) |
| **The Officer's Coat** (house; disguises) | a Haven named person; you're disguised in Bastion kit; disguises on | a Bastion holding or post within 1.5 km | Tell his you're not Bastion (regard). Take the tin "for the paperwork": the house −1 food, a lie found by rumour (+3 Haven Infamy). [Known: Bastion] ask at the post yourself (10 s): 4 Favor, and his answer: *"On the roster as killed, or struck off? Which?"* |
| **The Name on the Tape** (the dead; disguises) | a Bastion named person; Known with Bastion; disguises on | one of Bastion's people died within 1.5 km this season day and is still out there (the nail list, [REPUTATION.md](REPUTATION.md#whose-kit-it-was)); never to whoever killed them | Bring the tag and jacket home (15 s where they fell): 4 Favor, 1 Fame; the name is marked home and read on the nightly call. [Known: Red Hand] sell the jacket to the fence: 10 Red Hand Favor, and the board lists "kit lost". Once per dead name |
| **The Patrol That Didn't Come Back** ✅ (the props work) | a Bastion named person | the nearest town to the post, within 1.5 km, is 50+ infected: the patrol sent in didn't come out | Go into the town to where they stopped (a spot 40 to 120 m from its centre, the effects there) and take an identity card (10 s, a carried item: **Props > Soldier's ID**, `RZ_SoldiersID_Carried.et`), and carry it home yourself: 5 Favor, 1 Fame, recorded. [Known: Red Hand] sell the card to the fence (a dead man's name is a door): 10 Red Hand Favor, and the section stays missing |
| **False Hope at the Line** ✅ (the props work) | a Haven house's named person | a house of five or more whose last meal ran short or that lost people lately: a forger at its gate sells papers for the sealed checkpoint | Buy a set off him for proof at the gate (8 s, a carried item: **Props > Forged papers**, `RZ_Passport_Forged.et`) and carry it to the giver: 5 Favor, recorded. Or see him off the line (8 s): 3 Favor. Or leave him be: a family of the house walks to the checkpoint with his papers, and the house is one fewer (nothing goes through the perimeter) |
| **Spot the Tells** (Bastion; disguises) | a Bastion named person; Known with Bastion; disguises on | two or more different players caught by Bastion within 2.5 km in the last 2 season days | Find the scout by the tells (20 s among the three on duty): right (always with a Soldier or Police background, else two times in three) 5 Favor, 2 Fame; wrong, Private Hollis spends the night in the stockade and nobody pays. [Known: Red Hand] take the scout's money and point at Hollis: 10 Red Hand Favor, 35% it comes out (+5 Bastion Infamy) |
| **Paint** (the crews; disguises; with Name Day's Brush) | a Red Hand named person; Known with the Red Hand; disguises on | night, a Bastion road post within 2.5 km | Paint a red hand on the barrier (hold, 12 s): in Bastion kit asking for a light (you must be passing as Bastion at the barrier, else it's over), from the back unseen, or in plain sight (Bastion's timed grudge and +3 Bastion Infamy). 2 Fame, the crew name Brush if you have none, and "a civilian with a paint tin" on Bastion's board |
| **Wrong Way Round** (the crews) | a Red Hand named person; Trusted with the Red Hand | a lived-in house within 2 km with no Red Hand holding within 1.5 km of it | Look at the house's backwards red hands (10 s there), then: tell the boss (2 Fame; the house loses a person; +5 Haven Infamy); take a cut (6 Haven Favor; 35% the house finds out, +5 Haven Infamy); warn them off (2 Haven Fame); recruit them (2 Fame; the house loses a person) |
| **Laundry Day** (help across lines) | a Haven named person; Known with the Haven | a Haven and Bastion truce, a Bastion post and a Red Hand roadblock both within 1.5 km | Carry the uniforms' sack to the post (10 s there) the long way, never within 150 m of the roadblock (else the crew keeps it and it's over), or pay the toll twice (20 Red Hand Favor): 2 Haven Fame, 1 Bastion Fame, +2 food at the house. [Known: Red Hand] sell the sack at the roadblock: 15 Red Hand Favor, the house −2 food, 35% found out (+3 Haven Infamy) |
| **Civvies** (help across lines) | a Haven named person | a Bastion holding within 3 km: a deserter struck off as KIA shelters at the house (a seeded name, never the finale's canon deserter) | Bring civilian clothes from the next town (20 s there): 2 Fame, +1 person. [Known: Red Hand] sell where the deserter sleeps (10 s with a runner): 10 Red Hand Favor, a rumour, and the back room is empty |
| **Masks Off** (help across lines) | a Halcyon named person; Known with Halcyon | a house within 2 km took a bite declaration (its reason is up) | Find the beak at the house (15 s there), a Haven orderly selling samples: turn them in (2 Fame, coat 17 back in the record), let them go (2 Haven Fame), or take the coat (no good at Halcyon's looks; 35% the lab finds out, +3 Halcyon Infamy) |
| **Notebook 31** (the dead; merged with His Notebook and Into the Record) | a Halcyon named person; disguises on | one of Halcyon's people died within 2 km this season and the coat and notebook are still out there; never to whoever killed them | Bring both back (15 s there): 2 Fame, the name on the authors' list at the change of shift. [Known: Red Hand] sell the notebook: 15 Red Hand Favor, 35% found out (+5 Halcyon Infamy). Keep it: its number is a cover detail at one Halcyon look |
| **Vouch** (house; the two Vouch cards merged) | a Haven named person (Known with the Haven) or a Red Hand one (Trusted) | always: a newcomer, seeded | Speak for them: the record `vouched.<name>`. The Haven: +1 person; with the card's twist they steal (−1 food, +5 Haven Infamy on you). The Red Hand: one time in three he's selling to Bastion, a liability in the lie store (+5 Red Hand Infamy when found). [Ex-Con] ask about inside first: the truth, before you decide. Or don't vouch. No pay |
| **You're the One** (champions) | a Haven named person; Champion with the Haven | always | Write his daughter's name in the council book (hold, 6 s): a line on the board. Or say you're no one: *"That's what the last one said."* No pay |
| **Pay Packet** (backgrounds: Unemployed) | the clerk | a house of 6+, not short | Hand out the payroll (3 min): 4 Favor, 1 Fame |
| **Behind the Red Door** (the lost) | a Haven named person; the Kid lives at this house, and you're Known with the Haven or he remembers you | the next town within 1.5 km | Find the red door (15–25 s there), then tell him the truth (4 Favor), bring him a thing from his room (4 Favor), or tell him she's coming: he waits at the gate at dusk from then on (a pinned local) |
| **Somebody Should Tell Them** (the lost) | the clerk | the house lost people lately, the infected's ground within 1.5 km | Read the marks at their ground's edge unseen (15 s), [Hunter] tell them, or check the book of names (20 s): the reason `house.lost_to`; regard, no pay |
| **Both Hands** (the crews) | any holding's named person; Mixed Bag, Useful Trouble or Favourite Rascal with them | a rival's holding within 1.5 km (a crew; the Red Hand's: a Bastion post) | Talk him out (Trust 2, 20 s there): 6 Favor, 1 Fame; his lot seeing you, +2 Infamy with them. Or leave him |
| **Name Day** (the crews) | a Red Hand named person; Known with the Red Hand, no crew name yet | a town the infected hold, or a Fight Pit running, within 1.5 km | Belfry: fetch the church clock (20 s there): crew name Legs. Ringside: one round in the pit (a minute): crew name Sorry. 5 Favor, 1 Fame each |
| **Professional Courtesy** (backgrounds: Raider) | a Red Hand named person | always | Give it straight: 4 Favor, 1 Fame. Steer him wrong: 2 Favor and a lie (+3 Red Hand Infamy when found). Tell Bastion at a post within 1.5 km: 2 Bastion Fame; a crew body seeing it, +6 Red Hand Infamy |
| **A Debt Gone Quiet** (the crews) | a Red Hand named person; Feared band 2+ with anyone | a lived-in house within 1.5 km | Find him and lean on him (15 s there): 6 Favor, +3 Haven Infamy (he was paying for medicine). Pay it yourself (10 Favor): 1 Fame. Say he's dead: 3 Favor and a lie (+3 Red Hand Infamy when found) |
| **Off the Report** (Bastion) | a Bastion named person; Known+ with Bastion, some Bastion Infamy | always | Lose the report: 4 Favor and a lie (+3 Bastion Infamy when found). Tell him to own it. [Soldier] tell him how to answer for it: 3 Favor, 1 Fame |
| **Dress Uniform** (backgrounds: Soldier) | a Bastion named person; Soldier, Known with Bastion | a death within 2 km this season day | Fold it the regulation way at the table (hold, 12 s): 4 Favor |
| **Borrowed Green** (help across lines; disguises) | a Haven named person | a bite declared at the house today, the lab, before the cure, a Bastion road post between the house and the lab within 1.5 km | Walk them round the post (180 m off it, 10 s); ask the sergeant straight (persuade 1 with Bastion); walk through in Bastion kit (you must pass their look at the barrier; not passing there ends it), or with disguises off pay their way (15 Bastion Favor). 8 Haven Favor, 2 Haven Fame; half the time the dose takes (+1 person); the record `errand.borrowed_green` |
| **Both Doors** (backgrounds: Drifter) | a Bastion named person; Drifter, Known with all four | a Red Hand holding within 1.5 km | Carry true shift times both ways (5 s there): 5 Favor and 1 Fame from each. Give the crews a false one: 3 Favor and a lie (+3 Red Hand Infamy when found). Carry nothing, and say so |
| **Character Reference** (help across lines) | any holding's named person; Known with them and another faction | always | Say what you've seen (a rumour from the other side's real morale): 3 Favor. Speak well or ill of them whatever the truth: a rumour, and a callback later |
| **Unknown Face** (help across lines) | any holding's named person; a Stranger with them | a rival's holding within 1.5 km | Deliver the letter (5 s there): 5 Favor. Swap it for your own: 3 Favor and a lie. Sell it to the camp: 6 of their Favor; the giver's people seeing you, +3 Infamy with them |
| **Nobody Looks at a Stray** (bad standing) | a Red Hand named person; a Haven exile | a Red Hand road post or holding within 1.5 km | Carry the parcel there (5 s): 6 Favor |
| **Who's Asking** (bad standing) | a Bastion, Red Hand or Halcyon named person; a Haven exile | a Rogue leader of theirs within 2 km, a house within 1.5 km | Leave a crate of tins at the house's back door (10 s): +2 food there, 6 Favor; the Haven seeing you, +4 Haven Infamy. The exile stays an exile |
| **The Jar Run** (the Long Table) | a Red Hand named person; not family, not crossed | the farm unexposed within 1.5 km, none of its questlines or Cargo running | Fetch a jar (10 s at the door): 6 Favor, and nobody says anything. Say nobody was home: 3 Favor and a lie. Ask where the farmer gets his meat: *"Don't."* (it counts as something noticed). Never described |
| **Shoes in a Row** (the infected) | the clerk | by day, a lair within 1.5 km whose leader the Haven knew by name | Count the shoes from about 110 m unseen (30 s; [Hunter] from 150 m): 5 Favor, the count in the book (`errand.shoes`). Make a number up: 3 Favor, read as "about", and a lie |
| **Not Past the Chapel** (the infected) | a Haven named person | dusk within 20 real min, a hungry lot's ground within 1 km | Bring the old man in before half past eight (+1 person): 5 Favor. Board his doors: 4 Favor. Or leave him. He's rude whichever |
| **Buy It Back** (fallout) | aftermath, a Bastion named person; Known with Bastion | Crash Site's fallout: the box not handed in, a post within 3 km of the wreck | A fence's runner sits on the box at the wreck: five tins there, or 25 Red Hand Favor (only offered if you have it), then hand it in: 6 Favor (at most the fallout's cap); the world record `crash.box` = bought back |
| **Notebook 31, after Lost Colleague** (the dead; fallout) | a Halcyon named person | Lost Colleague's fallout: he died where he hid | Bring back the coat and the notebook (15 s where he fell): 5 Favor, 2 Fame, the record `errand.notebook31` with an epilogue line (the authors' list). [Known: Red Hand] sell the notebook to the fence: 6 Red Hand Favor and a lie (+5 Halcyon Infamy when found). Keep it (a life record: its number is something to say at Halcyon's gate) |
| **The Other Half** (help across lines) | any holding's named person; Known with their rival (the Red Hand's: Bastion; everyone else's: the Red Hand) | a holding of that rival within 1.5 km | Carry the threat word for word (5 s at their gate): 4 Favor, +2 Infamy with the rival. Soften it (Trust 2 with the rival): 4 Favor |
| **The Drawing** (the Long Table) | the clerk; Known with the Haven | the Long Table broken up and the boy not lost (`table.broken`, not `table.boy_lost`) | Put it in the drawer, keep it, or burn it on the stove. No pay; the flag `errand.drawing` (1, 2, 3) and a record |
| **Is He Gone?** (the infected) | a Haven named person | a pack within 1.5 km standing its vigil for a leader the house knew by name (the watching place at Traces' kill-site candle, L1, when one stands within 600 m of the lair) | Watch from the lair's edge, unseen (15 s): 4 Favor. [Elder] sit at supper while they say his name (a minute): 3 Favor, a record with an epilogue line |
| **Boarded Up** (players' traces) | a Haven named person | a town within 1.5 km that nobody holds, at 30+ infection, with a player's barricade standing; or Traces' held-out house (V2) within 1.5 km, first | Bring the family out (10 s): +1 at the house, 4 Favor. Leave a tin there: 3 Favor. *"Their father sent word back with the lookout: whoever boarded that house, tell them it held."* (never the builder's name) |
| **Old Habits** (backgrounds: Ex-Con) | a Bastion named person | a Red Hand camp within 1.5 km | Walk the wire and say where you'd come in (a minute at the gate): 5 Favor, 1 Fame. Tell him the wrong place: 3 Favor and a lie (+3 Bastion Infamy when found) |
| **Names on the Call** (Bastion) | the radio operator | civilians sheltered at Bastion's posts this season (`bastion.sheltered`), a house within 1.5 km | Pin the list on the house's board (hold, 5 s; the notices stay up): 4 Favor |
| **The Chalk on the Door** (the house) | the clerk | a town within 1.5 km with a work tally ([MISSIONS.md](MISSIONS.md#one-choice-jobs-and-work)) | Go and read the wall (15 s): 3 Favor. Say it's still true: 2 Favor and a lie |
| **After the Noise** (aftermath) | the lookout | a horde passed a town within 1.5 km (the reason `horde.passed`) | Walk the edge unseen (20 s): 5 Favor. [Hunter] read the ground (10 s): 5 Favor |
| **Last Orders** (the dead) | any holding's named person | one of their own leaders died within 3 km (the reason `leader.died`) | Bring his order book in unread (10 s at his seat): 4 Favor. [Known] read it on the way: 3 Favor and his last page, in his faction's voice |
| **The New Number** (Halcyon) | a Halcyon named person | a cure milestone in the last season day (the reason `cure.<percent>`) | Paint the new number on the board at the gate (hold, 10 s; the painted board stays): 3 Favor |
| **The Last Apples** (the house) | the cook | Overrun or later, the house under two upkeeps, an empty village within 1.2 km | Pick the last of the orchard (hold, 9 s): 4 Favor, +2 food |
| **Clean Glasses** (margin: The Book Has to Balance) | a Red Hand named person | the major waits for its settling day or its last page; not 16:00–21:00 | Six unbroken glasses from an empty village (30 s): 5 Favor |
| **Count for the Sitting** (margin: Every Name) | the clerk | the major waits for a sitting; before 17:00; another house within 1.5 km | Carry the count before half past six (5 s there): 3 Favor; or carry it and say the house is hungrier than it says: 3 Favor, a record. It changes no vote |
| **Hand to Hand** (margin: the Independent) | a Haven named person | the finale's routes are open, you've locked to none, a local at this house knows you | Promise: no pay, the local pinned, the record `errand.hands` with an epilogue line; six a player a season. Or don't. A promise moves no track. The finale can count them (`RZ_Errands.PromisedAt(house)`, `PromisedNear(pos, radius)`, by anyone or by one identity), and does: a promise eases one of the eight's yes ([FINALE.md](FINALE.md#the-rest-of-the-finale)) |
| **Weather, Write It Down** (margin: Await Relief) | the radio operator at a Bastion post | the major waits for Effective Strength; not 18:00–22:00 (the call's hours) | Take the reading on the ridge 200–400 m out (15 s): 4 Favor. Or tell him to write "weather" and skip it: no pay |
| **Clean Rifles** (margin: Await Relief) | a Bastion named person | the major waits for the muster; not 18:00–22:00 | Fetch the cleaning kits from the next town's police station (20 s): 4 Favor. [Soldier] show the new lads how it's done (a minute): 3 Favor, 1 Fame |
| **Clean Paper** (margin: Proof) | a Halcyon named person | the major waits for the reading; not around a change of shift (04:30–06:30, 16:30–18:30) | Fetch a ream from the next town's school (20 s): 4 Favor. [Lab Tech] rule the margins (30 s): 3 Favor, 1 Fame |
| **Chairs for the Reading** (margin: Proof) | Halcyon's technician or lab assistant | as Clean Paper | Carry three chairs out of an empty village's hall and set them by its door, one by one (carried for real): 4 Favor. Or tell him they can stand: no pay |
| **The Kid's Inspection** (champions: Bastion) | a Bastion named person; Champion with Bastion | always | Look over his post at the gate (20 s): a rumour, no pay. Or say you're nobody's sergeant |
| **A Story at the Fire** (champions: the Red Hand) | a Red Hand named person; Champion with the Red Hand | always | Tell it straight, or tell it bigger (a minute at the fire): a rumour either way, no pay. Or say there's nothing to tell |
| **The Visitors' Log** (champions: Halcyon) | a Halcyon named person; Champion with Halcyon | always | Sign the log (hold, 5 s): a world record with an epilogue line, no pay. Or be put down as anonymous |
| **They Say You're With …** (lock card) | a Haven named person | you've locked to a route that isn't the Haven's, a local at this house knows you | Titled by your route (Bastion, the Red Hand, Halcyon; the Independent's: *They Say You're Doing It Yourself*). Talk it through; walk them to the next town, out of the stand's way; or carry one true word to your side. No pay; the local remembers |

### The epilogue deck

✅ 20 built (`Cards/Epilogue/`, `RZ_EpiCard`). The last errands, for the week after the cure
([EPILOGUE.md](EPILOGUE.md#the-closing-work)): dealt by the same dealer, only while the epilogue runs
(`m_bAfterCure`), once per identity per subject (a name, a house, a holding, a day's road), and
outside the two cards a day. A card is dealt from a beat: only with one of the week's beats within
3 km of its giver, and **at most two of the deck offered or held by one player within 3 km** at once
(`RZ_EpiCard.DeckRoom`, after the card's own reason). They read the epilogue's record (the route, its
terms, the queue, the Names board, the named dead, who held each holding at the cure) and pay through
the small-pay ledger; nothing carries into next season. *Simplified:* any beat near the giver deals a
card (not the card's own beat).

| Card | Family, giver | True when | Ways |
|---|---|---|---|
| **Carry the Dose** | epilogue, a named person of the faction whose rules hold | the route has dose outlets, before processing | Carry the sealed box to a holdout in an empty village (15 s at the door, where the holdout stands in the doorway while the box is on its way): 6 Favor, 1 Fame. Open it yourself: the dose, if you're bitten and haven't had an outlet dose this life; 10 Infamy with the giver's faction either way |
| **Pack for the Queue** | epilogue, a Haven named person; Known with the Haven | the house has people and the queue stands, before processing | Fetch both things from their old house in an empty village (30 s): 4 Favor, 1 Fame. Bring one and say the other was gone (10 s): 2 Favor |
| **Borrowed** | epilogue, the storekeeper or cook | this house short, a fed house within 1.5 km | Ask that house for it back (10 s at its gate): 4 Favor, 2 food comes back. Say you couldn't find them: nothing moves |
| **Take a Name to the Board** | epilogue, a Haven named person | a named dead person still lying out within 4 km, the Names board stands | Write the name on the board (hold, 8 s): 3 Favor, 1 Fame; the wall lists it as home, by you |
| **The Toll Man's Last Debt** | epilogue, a Red Hand named person | the Red Hand's route or its book balanced, a house within 3 km, before processing | Carry the tins to the house (at the gate): 5 Favor, +2 food there. Sell them back to the crew: 4 Favor, +5 Haven Infamy |
| **Sign Here** | epilogue, a Bastion named person; Known with Bastion | Bastion's route on the roster or the column, the queue stands | Take three names at the queue (30 s): 5 Favor, 1 Fame. Write three yourself: 5 Favor; three in ten the corporal notices, +4 Bastion Infamy |
| **Copies for the Far House** | epilogue, a Halcyon named person | Halcyon's route, the method published | Carry the method to the lived-in house farthest from the lab (10 s, unarmed: a gun in your hands there spoils it): 6 Favor, 1 Fame. Sell the copies to the fence: 6 Red Hand Favor, +5 Halcyon Infamy |
| **Clear the Way** | epilogue, a Haven named person | the queue within 4 km, infected within 80 m of the bend, before processing | Hold the bend 400 m up the queue road for a minute: 5 Favor, 1 Fame (whatever's on the road comes to the noise) |
| **The Last Watch** | epilogue, an outpost's named person; Known with them | the holding was theirs at the cure; the evening change of watch (17:00–21:00) | Stand the gate for ten minutes, done before 21:30: 8 Favor, 2 Fame |
| **Last Letters** | epilogue, the Courier | the queue stands, before processing | Carry the bag to the queue (20 s, unarmed): 4 Haven Favor, 1 Fame |
| **One Last Toll** | epilogue, a Red Hand named person | the Red Hand's route or its book balanced, the queue stands, before processing | Stand the toll at the bend for two minutes: 6 Favor, +3 Haven Infamy. Talk the crew out of it (Trust 2 with the Red Hand): 3 Haven Favor |
| **Into the Record** | epilogue, a Halcyon named person | one of Halcyon's named dead lies out within 4 km | Fetch the notebook (15 s where they fell): 5 Favor, 1 Fame; the name goes home to the lab |
| **Come In** (amends) | epilogue, any holding's named person; Watched or Hunted by them | before processing | Walk up to the gate and wait a minute: amends (2 Infamy off) |
| **The Key Where It Was** | epilogue, a Haven named person | you read Petr's note in the empty kitchen (a new face's first spawn in the week), the queue stands, before processing | Take the key up to Petr in the queue (10 s): 5 Favor, 1 Fame. Use it yourself (30 s in his house in an empty village): his larder, +5 Haven Infamy |
| **The Tags** (His Effects, the epilogue's variant) | epilogue, a Bastion named person | a Bastion name still lying out within 1.5 km of the post | Bring the tags in (15 s where they lie): 6 Favor, 1 Fame; the name goes home to the nail. Sell the photo frame to the fence: 6 Red Hand Favor, +5 Bastion Infamy; the name is never brought home |
| **The Wreck's Name** (His Effects, the epilogue's variant) | epilogue, a named person of the faction that lost the truck | one of its trucks lost within 1.5 km in the last 60 season days | Fetch the driver's papers from the wreck (20 s): 5 Favor, 1 Fame; a hall line names the driver again. Sell the photo frame to the fence: 5 Red Hand Favor, +5 Infamy with the giver's faction |
| **Somebody Should Tell Them** (the epilogue's version of the lost card) | epilogue, a Haven named person | a pack within 3 km runs with a named young (never a player's character) | Read the marks below its ground (30 s unseen): 5 Favor, 1 Fame; the name joins Bring Them Home with an ID card to carry home. Tell them plainly: an empty grave by the house fence (a since-the-cure beat) |
| **The Boy** | epilogue, a Haven named person | the Kid walks within 3 km, the queue stands | Walk the Kid to the queue, or bring him to this house (one more mouth): 15 s together, 5 Favor, 1 Fame; the Kid leaves the roads (`table.boy_home`) |
| **Processing** | epilogue, a Haven named person | a pinned local at this house, the queue within 1.5 km | Walk them up to the queue (20 s, unarmed): 4 Favor, 1 Fame; they sit in the queue (`queued`). Talk them into supper (`supper`): they stay |
| **What the Cure Costs Here** (card 41) | epilogue, a Haven named person | a local at this house the terms don't reach (no outlet of its own, no Haven or open-crate outlet within 2 km) | Only the way the outlet table allows, by whose rules: Bastion's (register them at a post's list, 20 s; or vouch, Trusted with Bastion), the Red Hand's (pay 15 Red Hand Favor, put it on their line in the book, or haggle for 8, Trusted), anyone else's (sit with them a minute). Dosed: they tell people who brought it |


## Fallout

`RZ_Errands.Fallout`, called by a failed story job ([DESIGN.md](DESIGN.md#story-blocks)). One errand of
the job's kind waits at its subject for 3 hours: for the first **20 minutes** only the job's
participants can be dealt it, then anyone. A giver within 3 km of the subject deals it first (×3 its
score), and it pays at most **30%** of the failed job's base, whatever the way says. Done, it's gone;
a second fallout of the same kind at the same place replaces the first. Not saved.

| Failed job | Fallout errand |
|---|---|
| Protection Racket (time up) | The Pot, at the house |
| Crash Site (the box not handed in) | Buy It Back |
| Distress Call (the people died), Survivors Trapped (all dead) | The Name on the Tag, their names to the nearest house |
| Lost Colleague (time up while he hid) | Notebook 31 |

## Strangers

`Missions/Errands/RZ_Strangers.c` (spots and visits) and `RZ_StrangerKinds.c` (the routines), with
**Missions > Errands > Strangers**. A passer-by who asks one small thing, met **only where one would
make sense** (the author's decision 14), never because time passed. A player crossing empty country
meets nobody, however long it's been.

**Spots**, looked for 150, 250 and 350 m ahead along the way you've walked in the last 30 s (standing
still meets nobody; a stopped car is the exception), from facts the server already has:

| Spot | What makes it one | When |
|---|---|---|
| A road people use | the 250 m **footfall** cell has had players in it a lot lately (a count of player passes on the 30 s tick, halving every 6 real hours; not saved), or a wanderer's leg runs through it | by day; at night only within 300 m of a fire or a held place |
| A crossroads | 80–300 m from a road post, outside its barrier; or from a junction of two used roads (three road ends within 25 m in the road network, a road people use there) | any hour while the post stands; a junction any hour |
| A wanderer's route | within 300 m of the leg a living wanderer is walking (where they are to where they're going) | while they walk it |
| An approach | 100–800 m from a safehouse or a holding, outside its wire | any hour |
| A campfire | within 60 m of a stove fire lit in the last 30 minutes, or a Red Hand road post's brazier | after dark (a stove fire: while it's fresh) |
| A stopped car | your own car, stopped 20 s with you out of it (the stranger walks up 60–90 m off), or a wreck within 150 m of a road people use | your car any hour; wrecks by day |
| A fresh event | within 400 m of a death, bite, fire or shelling in the last 2 played hours, a found hook, or a live trace's place (Traces' kill-site candle, a scorched doorway) | while fresh |
| A town edge at dusk | within 100 m of a settlement's edge | at dusk; fog counts as dusk |
| Their own place | the place the routine's Why names (Seed Potatoes' farm in an empty village) | by day |
| The line | within 300 m of the 150 m warning point of a sealed checkpoint | while the Zone is sealed |

**Never:** another player outside your vanilla group within 300 m; within 2 km of the Long Table's
farm; driving; a shot within 60 m in 60 s or infected within 30 m; a running mission's 300 m, a set
piece's 600 m on its evening, the lab's 1 km during the final stand; a storm (in rain, only approaches
and fires); the first 5 minutes of a life; within 600 m of a lair, inside the infected's ground by day
while their party is out, or at night away from a fire or a held place; a spot that gave anyone a
stranger in the last 2 real hours, or you one in a real day. **The ceiling** only ever stops one: one
per player every 15–25 minutes, 6 on the server (the body budget's strangers). The routine's Why must
also hold. Several fitting: weighted, ×1.5 at a fresh event, ×1.3 at a crossroads, an approach or a
town edge.

**How they behave.** They call out once when you're within 40 m, stand still, and wait 5 minutes or
until you're 150 m away; a way that asks you to go somewhere or stay a while waits 10 minutes wherever
you are. Only the one they called to, or their group, can answer. They pay nothing, a line, one common
find from their pocket (a House find), or the house's few Favor through the small-pay ledger; one you
help may become a [local](#locals). **Where they stand**: 60–90 m from you, toward the spot or to either
side, somewhere no player can see (a building, a hedge, a fold of ground: the infected bodies' sight
test); with no such point, the spot itself. Done, they go only once nobody can see them (a minute at
most). **Robbing one**: a dead stranger's body stays 2 minutes; its pockets emptied with a player
within 3 m is +8 Haven Infamy for them, and no stranger finds them for a real day. **Shooting at one** (firing within 25 m of a stranger with no infected within 40 m of them) costs +4
Infamy with their people, once, and they run. **Killing one** costs what killing a wanderer costs (+15 Infamy with
their people: the Haven's, or the Red Hand's for a crew lad), and no stranger finds the killer for a
real day.

| Stranger | Spots | Why | Ways |
|---|---|---|---|
| **Up on the Roof** (was Rooftop Rescue) | a fresh event; a town edge | a town the infected took, or a fresh shelling or fire, within 800 m | Talk them down when no infected is within 25 m: +1 at the nearest house, 4 Haven Favor, a local. Or leave them |
| **The Last Tin** | a road; an approach; a route | you carry 2+ food, a house within 1.5 km on half portions; never to the face on the notice | Give him a tin: a true tip (which way the nearest lot walks at dusk), and he's a local at his gate. Trade two tins for his watch (a real wristwatch, **Props > Watch**; empty, a common find). Or not today |
| **The Funeral for the Car** | a stopped car (a wreck) | by day, a wreck near a town nobody lives in | Push it into the ditch with him (10 s, 8 heat): 3 Haven Favor. [Mechanic] *"It's not dead. It needs a battery."*: he goes to tell the Mechanic. Open its boot first; he lets you |
| **Which Way's Morton** | a road; a route | he's walking to a house 600 m–1.5 km on, with a road post or a lot's ground between | Point him round (+1 at the house). Walk him past it (500 m): +1, 3 Haven Favor, a local. Tell him the toll's cheap: a lie found when someone the Haven knows passes the post |
| **Behind the Door** | a town edge at dusk | the town is the infected's | Walk them out toward the house now (600 m; a gate bangs on the far side of town and draws the infected there): +1, 3 Haven Favor, a local. Come back at dawn: if you're back within 40 m between 05:00 and 08:00, they come out (+1, 3 Favor). Or leave them food |
| **Two in the Back** | a stopped car (a wreck) | by day, a house within 1.5 km | [Mechanic] fix it (a minute), give a battery, or push-start it downhill (30 s, 6 heat): they drive home, 3 Haven Favor, the father a local |
| **The Digger** | a fresh event (a death) | a death within 400 m in the last 2 played hours | Help his dig (30 s, 4 heat; [Elder] says the words): 3 Haven Favor, a local. Tell him who it was (the death's description, never a player's name). Take the dead's things: he sees (+3 Haven Infamy), and you get one find |
| **Still Walking** | an approach, at dusk | a man was walked out and let walk on (`walk.out`) in the last season day; never while the Edge of Town runs | Point him to the lab, walk him toward it (600 m), or sit with him until it's dark (2 min; [Nurse or Doctor] 1). Pays nothing. He gets the trial dose: his note on the nearest house's board, signed (*"I'm still walking, and I'm told that's the point."*) |
| **Warm Hands** | a campfire, after dark | — | Let him sit 5 real minutes: one true thing from the road (a lot that fed last night, their habit, a town gone bad). Give him a tin: a local at his house. Or send him off |
| **Seed Potatoes** | their own place, by day | an empty village within 1 km of the infected's ground or a town they hold, a house within 1.5 km | Dig his sack up (10 s, 6 heat): +2 food at the house, 4 Haven Favor. Say it was gone (the sack goes to the fence for you: 4 Red Hand Favor) and it's a lie. Or not your business |
| **Checkpoint Manners** | a crossroads (a Bastion post) | a Haven family with the post between them and their house, a rifle part in a tea towel | [Known: Bastion] say they're with you (+1, 3 Haven Favor). Walk them round the post (+1, 3 Favor). Hand the bundle in (+1, 2 Bastion Favor). A rumour later: the Red Hand bought something "for the rabbits" |
| **My Brother at Morton** | a road; a route | a house within 1.5 km lost people lately | Half the time he's his brother, half a crew lad (the truth deck): the tell (a red hand on his wrist) is named only to a Police or Ex-Con background, or Known with the Red Hand. Find him in a barn and tell him: reunited (3 Haven Favor), or, a crew lad and you knew, he never arrives (6 Red Hand Favor; seen by the Haven's, +12 Haven Infamy). Warn him on to another house (+1 there, a local). Call him out: right, he goes; wrong, *"Calling a grieving man a liar. Nice."* (+1 Haven Infamy) |
| **Wrong Side of the Toll** (dark) | a road; a crossroads | a Haven man with a Red Hand road post between him and his house | Pay his toll (10 Red Hand Favor) or [Trusted: Red Hand] say he's with you: +1 at his house. Walk him round it: +1, a local. Sell him to the toll: 6 Red Hand Favor, he never arrives, that house's errands are half as likely for you all season, seen by the Haven's +12 Haven Infamy |
| **He Waves at the Line** (dark) | the line | the Zone sealed | [Known: Haven, or Nurse] talk him back to the nearest house: +1, a pinned local. Walk out there with him: the line's rules are for you alone. Or leave him: a Haven headline |
| **The Face on the Notice** | an approach; a road | you're Kill on Sight with someone, alone, a Red Hand outpost within 1 km | Take his landlord's book from the crew hut unseen (20 s hidden), or burn the hut's books (15 s, 10 heat; a crew body seeing it +8 Red Hand Infamy): one common find. Or turn him down. The only stranger who doesn't run |
| **Found hooks**: someone hiding | a fresh event (a job's site) | the hook | Talk them out: +1 at the nearest house, 3 Haven Favor |
| **Found hooks**: someone got there first | a fresh event (a job's site) | the hook | Share (talk only), trade a tin for a find, or leave it to them |
| **Found hooks**: a bite on a stranger | a fresh event (a job's site) | the hook | Send them to the nearest house, or [Nurse or Doctor] look at the arm first: they wait at its gate for **Who's at the Gate** |

**Found hooks.** When a job ends at a site, one site in five (**Found hook chance**, 0.2) holds, for 2
hours, someone hiding there, someone who got there first, a bite on a stranger, or **an old mark**
from Traces' door-mark grammar: a marked door near the site, else a mark a few days old by whoever
holds the ground near there, maybe wrong now ([TRACES.md](TRACES.md#errands-old-mark)). The mark is
read once by whoever passes within 30 m, at their own level; nobody asks anything. With Traces off,
or no town near the site, it holds no old mark.

*Simplified:* a campfire is a stove fire or a crew's brazier at a road post (players have no campfire
of their own); escorts are a walk to a point, the stranger going on as data. Engine checks (VERIFY):
the out-of-sight point (a physics trace from the eyes stopped by buildings and hedges), reading
junctions from the road network (a junction's roads end at it), and a robbed body (its item count
falls).

## Locals

`World/RZ_Locals.c`. The people the season remembers. A local is made when a stranger or a card ends
with someone alive and placed: a seeded name, a short role (*"who had the empty sack"*), one line, a
home that is always a safehouse, a spot there (the gate, the stove, the bench), and what each player
did for them (helped, saved, lied to, crossed, sold, promised).

- **Pinned** locals (saved, promised, sold) are kept to the season's end: 30 a server, 4 a house.
  **Unpinned** ones: 20; past that, the oldest nobody has met for 3 real days goes first.
- **Shown** on one of the house's plain occupants after its two named people (never a leader's or a
  named person's body), as **Talk to {name}**: their line about what you did, to you only. A new life
  is greeted with *"You've a look of someone."*; someone who never met them as a stranger.
- **They live and die with the house.** When a house loses people, its locals are among them as often
  as their share, and the news (*"didn't come down to breakfast"*) goes only to the players who knew
  them, online. Killing a local's body costs +15 Haven Infamy, as a wanderer would.
- Saved with the errands.

Each memo has three lines, turning with the hour. **Missions > Errands > Pinned locals** (30) and
**Unpinned locals** (20) set the caps.

- **Shown near those who know them.** A local's body is labelled (Talk to, Ask how they are) only while a
  player who knows them is within 100 m; otherwise it's a plain occupant.
- **Coming to you.** Passing 80–250 m from a local's house, someone they remember helping or saving
  them may see them walk up (a stand-in 15–25 m off, out of your sight when they appear, for 90 s and
  gone once nobody sees them) with their line and *"We could use a hand there"*; once a real day per
  local per player, one on the server at a time, never in a fight, a car or near a job, and only with
  room in the body budget (`errand`, [INFECTED.md](INFECTED.md#the-one-body-budget)) and a hidden spot.
- **Their spot as a place.** A house errand's "here" step, for someone who knows a local there, is
  that local's spot at its hour: the gate at dusk, the stove or the bench at supper.

*Simplified:* while a knower is near, everyone near sees the same labels (VERIFY: per-player action
labels; today a label is the same for everyone).

## Threads and callbacks

- **Threads** (in the ledger, 12 a season, the oldest goes): a done routine that has a thread phrase
  (the Pot, Lights, Water, Filter Parts, the Gate at Dusk, the Burying, the Round, the Newcomer, the
  Children's Hour, the Wire, the post's fuel, the freezers, Short Change) leaves "who, where, what you
  did". The next time someone else at that place offers you an errand, they say it first, once:
  *"Marek Kovac says you brought something for the pot."* Other systems read the newest one for a
  holding or house (`RZ_Errands.ThreadLineFor(player, holding, house, once)`): a **leader's greeting**
  says it as its first line, once (the house's council head, or the leader of the holding within
  300 m); and with **Missions > Errands > Majors read errand memory** on (off by default), a sway at
  the Haven's sittings on a head whose house holds one brings one more voter round, once per head a
  season ([DESIGN.md](DESIGN.md#the-majors), Votes).
- **Callbacks**: a few ways leave a line for later in the one callback queue
  ([DESIGN.md](DESIGN.md#callbacks)), 2–6 hours of the asker's play on, near the giver: Behind the Red
  Door's truth, Boarded Up's family brought out, Not Past the Chapel's old man brought in, The Letter
  Nobody Can Take, The Last Apples.

## Twists

A card that can have a twist (`m_bTwists`: Vouch's newcomer who steals) has it at **Twist chance**
(0.2), at most one errand in three a player and never two in a row (the ledger counts). The same
replies show either way, and the twist never costs more than the errand's own stakes. Some cards and
strangers draw their hidden truth from the truth deck instead, the same for everyone at that subject
for 3 season days (Sleeve Down's bite, Vouch's Red Hand informer one time in three, My Brother at
Morton's crew lad one time in two). Lies told in errands go into the one lie store
([DESIGN.md](DESIGN.md#lies-and-witnesses)): they pay at most 60% of the true way and no Fame, and they're
found by events (a visit, a road, a rumour round, the next real reading).

## Board and radio asks

- **Board asks** (**Board asks**, on): a safehouse's notice board shows, above the players' notes and
  apart from them, up to two asks signed with the house's name: *"Levie asks: The Pot. Ask Mara, the
  cook."* They're only what's true for the reader now (the routines the house's people would deal
  you), and the ask itself is still **Anything need doing?** to that person. Exiles see none. On the
  board menu they're the first rows, by "{house} asks".
- **Radio asks** (**Radio asks**, on): every 30 s one player, round-robin. Near a house (300 m to
  1.5 km) where a routine is true for them, inside every rule of an unasked offer and the interruption
  budget, one personal line on the Haven's channel: *"If you're near Levie, Mara the cook could use a
  hand. Only if you're passing."* Never broadcast, so it can't bait an ambush. At most one a player
  every 60 real minutes (**Radio ask minutes**). Like the board's, it only says who to ask; the
  errand is still dealt at **Anything need doing?**

## Handed errands: the openings' chain

An opening's steps ([DESIGN.md](DESIGN.md#openings-how-every-life-starts)) are errands another
system hands a player, already taken (`RZ_Errands.Hand`, code in `Guidance/RZ_OpeningChain.c`,
`RZ_OpeningFavours.c` and `RZ_OpeningSteps.c`). They
are the **only errands, and the only jobs, handed to the player rather than asked for**:
`RZ_ErrandDef.IsOpening()` and `RZ_Errand.IsOpening()` are true for them and for nothing else. With
someone outside, the first step is handed in a conversation the player opens: before it, a **talk
objective** is handed and tracked, and talking to the contact ends it and hands the step there
(nobody outside, or the contact dead: the step is handed at once). Every step is done where it's
done (`m_bEndsThere`): none sends you back. The next starts there, said by the contact walking with
you ([DESIGN.md](DESIGN.md#openings-how-every-life-starts)). The chain is the door's favour, its
hand-over, the line and the runner. The old step 2s below stay as step definitions (The Fever is the
Nurse's and the Doctor's way through the favour) but no chain hands them; the old step 1 and the two
step 2s that ran through talk (Open Channel, Two Families) are gone.

| Step | Key | Way | The step |
|---|---|---|---|
| ✅ Outside the Door, Back to *the contact* (the talk objectives) | `opening.talk`, `opening.talk.next` | talk | "Talk to *the contact*.": never done by being there (`StepCounts` false), only by opening talk with them, when it ends quietly (no past job); 45 minutes before it lapses, which ends the opening |
| ✅ The favour (The Medicine, The Battery, The Last Box, The Spare Key, The Cool Box, A Battery for the Set, The Rifle, A Tin, The Rota, The Last Sack) | `opening.favour.<n>` | do | one named thing fetched for the contact at a place its words are true of, 90–450 m off (the neighbour's door, the lad's house, the house next door: 35–300 m): a work site's hold on its ground floor (the neighbour's: on the real door's doorstep), the thing (or a toolbox, a crate, papers) standing there; done there. Its thing is held for you until the hand-over; 45 minutes before it lapses ([DESIGN.md](DESIGN.md#openings-how-every-life-starts)) |
| ✅ Give Him the *thing* | `opening.favour.give.<n>` | give | done only by the hand-over reply on the contact's talk screen (*[Errand] Here's your battery.*), there and then (`RZ_Errands.FinishNow`); standing by him never counts (`StepCounts` false). He walks with you, so the step's place follows him |
| ✅ Your background's way (Hotwire It, The Tracks, Open Up, The Back Door, Kept Cold, Give It Here; the Nurse's and the Doctor's is The Fever) | `opening.favour.way.<background>`, `opening.fever` | do | handed instead of the fetch when its tagged reply is picked ([Mechanic] *Forget the battery. Show me the car.*): a hold at its place, done there; no hand-over follows. The Radio Operator's, the Raider's and the Elder's ways are replies that settle the favour there and then |
| ✅ Water | `opening.water` | do | the nearest well or water tower: hold **Fill what you can carry** at the cans by it (8 s; off: 15 s there), done there |
| ✅ The Cut | `opening.hand` | give | one medical item handed over at the giver when you pick the hand-over reply (*[Errand] Hold still. Let me dress that hand.*): done there and then, in that talk (`RZ_Errands.FinishNow`), and the next step is said into the same talk; never later for walking back |
| ✅ The Fever, Water for the Fever | `opening.fever`, `opening.boiled` | do | their son lies there (a body laid on the care work site's mattress, on the nearest house's ground floor, else beside his father), the tracker on him: hold **Treat his fever** at him (8 s), one medical item taken; **Fill what you can carry** at the well (8 s) |
| ✅ The Clock | `opening.clock`, `opening.clock_med` | do | their bitten friend lies there as the Fever's son does: hold **Give him the antidote** (or **Clean and bind the bite**) at him (8 s), the antidote (or a medical item) taken; 10 minutes before it lapses |
| ✅ The Pump | `opening.pump` | do | 5 s at a petrol station's pump with fuel in a can |
| ✅ Downwind | `opening.downwind` | sneak | 30 s within 25 m, only while hidden (`m_bUnseen`), quiet |
| ✅ The Station Keys | `opening.keys` | do | inside the locked building whose key the giver gave you |
| ✅ The Toll | `opening.toll` | talk | 10 s within 30 m of a Red Hand toll with no shot near it |
| ✅ A Sample | `opening.sample` | do | at the remains with a sample in your pack (the crate or effects prop: none) |
| ✅ A Fair Swap | `opening.swap` | give | 5 s with a wanderer (the place follows them) |
| ✅ Hold the Gate | `opening.gate` | do | 60 s within 15 m of the gate |
| ✅ The Debt | `opening.debt` | press | 10 s at a house's door (the crate prop) |
| ✅ The Line | `opening.line.walk`, `opening.line.watch` | do | walk with the contact to a road above the line (he leads; the marker on him), then watch it with him: the loudspeakers, a drone, a runner shot by the line in the warning zone ([DESIGN.md](DESIGN.md#the-line)). Both done by the scene, never by standing anywhere; 30 minutes before each lapses. Its short form: no step, the runner found already down |
| ✅ The Runner | `opening.case.seen`, `opening.case` | do | take Halcyon's cold case from the runner: the man you saw shot, where he fell (`.seen`), or (the line's short form) one who fell 200–600 m off (by a road, else a yard on the edge of town): done when it's in your pack, there; 90 minutes before it lapses. Then the case's lead ([DESIGN.md](DESIGN.md#the-case)) |
| ✅ The Case | `opening.case.lead` | do | take the case to Halcyon's field station (the nearest Halcyon holding, else the lab): done only by the hand-over reply at Halcyon's people (selling it to the Red Hand or handing it to Bastion drops it); 10 hours before it lapses, handed again to whoever carries a case |
| ✅ The Date on the Tag, Get It Back, The Case at the Post | `opening.case.date`, `.back`, `.post` | search, sneak | the stages the case opens: a work site inside the clinic (else the doctor's house) in Proof's clinic town, or inside the Red Hand's or Bastion's holding where it went; done there; 10 hours before each lapses |
| ✅ Where the Work Is | `opening.lead`, `opening.lead.note` | do | within 40 m of the nearest place with work, done there; 60 minutes before it lapses (no case here, or nobody outside: the scene's read gives it) |
| ✅ To the Haven, To Bastion, To the Red Hand, To Halcyon | `opening.walk.<faction>` | do | within 40 m of the place, done there (`m_bEndsThere`: nobody to go back to); 90 minutes before it lapses (`m_fLapseMinutes`); only when there's no case to find (levels 0-1) |

How a handed errand differs (`m_bHanded`): the dealer never deals it; its giver is the contact, not a
named person, so the "giver gone" guard doesn't apply; it pays nothing through the ledger (no Favor,
no Fame, no helpers' share), sets no cooldowns, adds no helped line to the hall and no deed (the
opening pays its own Fame at the end of the walk); the owner drops it quietly (`RZ_Errands.Drop`)
when the contact dies or the chain ends. Everything else is an errand's: the take and done lines in
the giver's voice (into the talk screen while it's open on them), the step notice, the journal row,
the Quests tab and the tracker, which the opening points at each step (and each talk objective,
whose map diamond is the contact, where they stand now) as it starts. While a
player's chain runs, nobody waves them over for another errand. Two hooks serve them: a handed
errand's way puts its prop at the place as a taken one does, and `RZ_ErrandDef.StepCounts` (true for
every other errand) lets a step 2 stop its clock while its own system isn't met, or move its place
after someone who walks.

## Telemetry and the admin view

- **Log lines**: `Errands: stranger <kind> shot at by <player>`, every deal (`Errands: dealt <key> to <player> by <giver> (asked|called out)`), take,
  done, overtake and fault; every stranger (`Errands: stranger <kind> for <player> at <grid> (<spot>)`,
  `Errands: stranger <kind> done by <player>`, `Errands: stranger <kind> killed by <player>`), found hook
  (`Errands: found hook <kind> at <grid> after <title>`), radio ask (`Errands: radio ask to <player> for
  <name> at <house>`), twist (`Errands: <key> for <player> has its twist`) and local (`Locals: <name> (<role>)
  made at <house> by <player> (pinned|unpinned)`); every fallout (`Errands: fallout <card> at <grid> from <title> (...)`); and every 30 minutes (**Telemetry**) a status line per family: dealt,
  declined, lapsed, overtaken, done, Favor, Fame and amends paid, and the ways taken, then strangers
  dealt by spot kind and helped by routine, then errands' lies told, found or buried and still open
  since the last line, and the twists dealt.
- **The admin view needs no typing**: on the talk screen a server admin sees one more reply on any
  named person, wanderer or occupant, **[Admin] Errands near here**: the live count, the nearest
  player's last deal and why each of the others wasn't dealt (*"lights: not true. pot: cooling
  down."*), their errands, the stranger spot kinds plausible where they stand now (*"road, approach"*),
  the strangers out and the locals. It's also written to the log.

## Saving and performance

- **Not saved:** a duty's table and its people, like every held errand.
- **Saved** (state.json, wiped with the season): each identity's ledger (cards done, cooldowns, the
  hour's Favor, cards today, errand Fame today, amends this life, the last three families, errands
  done, the work done for each faction (the ladder's second rung), the houses whose errands are half as likely for you, the twist count, the threads), the safehouses' named
  people, the locals, each house's count of errands done there and the Haven's reading day of each lot
  (Fresh Chalk). Held errands,
  offers, faults, soft holds, strangers, found hooks and the footfall grid are runtime only: a restart
  lets them lapse.
- **Cheap**: no map scans. A deal checks one giver's anchored definitions; steps are distance
  checks on the 5 s tick; faults, guards and call-outs run every 30 s.

## Simplified

*Simplified:* a step's place is a point with no scene person (strangers are the only scene
people); the lines have two or three variants each, not always three; a card dealt counts toward
the day's two even if declined; pay in kind is an effect, except a post's tagged ration or magazine
and a carried item (errands hand out no doses, crates, kit or keys); a deferred pay waiting for its
truck is saved (a real day at most) and still waits after a restart; the goods' tag is a server-side list (VERIFY in
[DESIGN.md](DESIGN.md#story-blocks)). The margin cards read the majors' waits (the run's stage), and the lock cards read
the finale's lock (`RouteOfPlayer`). The Quests tab's errand row uses the `mission_errand` icon.
