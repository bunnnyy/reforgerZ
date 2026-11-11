# Errands

The small, personal jobs one named person asks of one player, each true because something in the
world is true right now, and over the moment it stops being true. Contracts are the factions' work
on the radio ([MISSIONS.md](MISSIONS.md)); questlines are the long stories
([QUESTLINES.md](QUESTLINES.md)); errands are everything in between. What's still to build (the
the epilogue deck's last six, the other margin cards, promises and threads, His Effects in full) is
designed in [designs/ERRANDS.md](../designs/ERRANDS.md). In the epilogue week a few of the epilogue
deck's asks stand at the week's places instead of on the dealer: *Bring a cup* at the queue, *Read
the names at supper* at a house, taking a name home ([EPILOGUE.md](EPILOGUE.md#places-and-scenes)).

Code: `Missions/Errands/` (`RZ_Errands.c` the dealer and the small-pay ledger, `RZ_Errand.c` the
parts, `RZ_ErrandRoutines.c`, `RZ_ErrandRoutinesPeople.c` and `RZ_ErrandRoutinesLate.c` the routines, `RZ_ErrandRegistry.c`
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
- [Pay: nothing for free](#pay-nothing-for-free)
- [The faults table](#the-faults-table)
- [The routines](#the-routines)
- [The wanderers' asks](#the-wanderers-asks)
- [The cards](#the-cards)
- [Fallout](#fallout)
- [Strangers](#strangers)
- [Locals](#locals)
- [Twists](#twists)
- [Board and radio asks](#board-and-radio-asks)
- [Telemetry and the admin view](#telemetry-and-the-admin-view)
- [Saving and performance](#saving-and-performance)
- [Simplified, and not built yet](#simplified-and-not-built-yet)

## The idea

| | Contract | Errand |
|---|---|---|
| Offered by | the faction, by radio or quartermaster | one named person, in person (never a leader) |
| Offered to | everyone who qualifies | one player, and their helpers |
| Size | 10–40 min | a few minutes, one step, within 1.5 km |
| Exists because | weights, needs, reasons | one live fact |
| Pays | Fame, Favor, crates | a little Favor; Fame up to a ceiling; an effect in kind |

Two layers:

- **Routines** repeat, on cooldowns: per player per place a real day, per player anywhere 3 real
  hours, per place server-wide 20 minutes. 34 at the four holding kinds, the wanderers' asks, and
  18 stranger routines.
- **Cards** are hand-written, **once per player per season** (keyed to the identity, not the
  character), at most **2 a real day**. 63, every one true only while its reason is (a world fact, a
  place, your standing or background, the season's stage, a major's wait or the finale's lock).

Errands are never stages and never on the map. Every one has a way without a gun. They spawn no
AI, no crates and no items. Nobody in the Zone calls them errands: the Haven asks you to "do us a
turn", Bastion has "a detail", the Red Hand "a little something", Halcyon "a small request".

## Givers

A giver is a **named person**: a name, a role and a place. **Leaders never give errands**, and no
errand reads or writes a leader's memory, regard, loyalty, attitude or vote, so no errand can close
a stronghold leader (or a finale route) to anyone.

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
  in the action menu or on the [talk screen](DIALOGUE.md#the-talk-screen). A wanderer's is **Need a
  hand?**, in their free third slot (not on the Kid, the Mechanic, whose slot is Ask about cars, or
  a bitten wanderer, whose slot is the antidote).
- **Waved over.** A safehouse's or holding's shown named person may call a passing player within
  30 m: *"Mara waves you over."* That's an unasked contact: it asks the notice lanes'
  [interruption budget](DESIGN.md#notice-lanes) (`TryContact`, errands last), and every rule of the
  dealer must hold.
- **The offer.** The giver says what they need, in their own voice, and their replies become the
  errand's ways (three at most) and **Not now**, each with what it needs as a tag:
  *"Hand over two tins [2 food]"*, *"Wire in a car battery [Mechanic] [1 battery]"*,
  *"Soften it on the way [Trusted - Haven]"*, *"Add a threat of your own [Feared | +3 Infamy]"*.
  Tags show costs and requirements, never outcomes. The replies are that player's for 90 seconds,
  or until they pick or walk 20 m off; anyone else at the same person hears *"Give us a minute."*
- **Not now** is always free: nothing is remembered, and the offer stands 10 minutes.
- **The reason in their voice.** Asked when nothing's true, or when it isn't safe, the giver says
  why: *"Not while that lot's at the fence."*, *"You've your hands full already."*, *"Nothing on the
  board for you. Check back after the shift change."*
- **Every line is personal**: the giver's lines are spoken (subtitles, or the talk screen under the
  giver's name), the step and the pay are notices to that player only.

## The dealer

`RZ_Errands` runs on the game mode's tick beside the mission manager, outside the random picks and
the contract cap. Steps run on the 5 s tick; checks, faults and call-outs every 30 s.

**Asked** errands skip the gaps, but not safety: no shot within 60 m in the last minute, no infected
within 30 m, not in a vehicle, not Hunted by the giver's faction, fewer than 2 held.

**Unasked** (a call-out) also needs: alive 3 minutes in this life, not in a bite's Final stage, no
open offer, 8 minutes since the last unasked offer, not within 300 m of a running mission's spot,
600 m of the set piece holding the evening ([the calendar](DESIGN.md#the-set-piece-calendar)), 1 km
of the lab during the final stand, or 2 km of the Long Table's farm; and room in the budget. Three
players at most each 30 s, round-robin.

**Candidates** are the routines and cards anchored to the giver's place (a safehouse, a Bastion
post, a Red Hand camp, a Halcyon station or the lab, any holding, a wanderer and their role),
checked cheapest first: the card done this season or two cards today; the routine's cooldowns; a
running clock for timed ones; for whom (standing, background); **Why** (the live facts, which fill
the places and the text); **Still**; the subject free; a way that fits here. Each scores
weight × need × fit × novelty × story: ×1.2 if it answers the giver faction's need, ×1.2 if the
giver has the role, ×1.2 if they remember you well (regard 2+), ×0.8 if uneasy, ×0.3 if its family
was one of your last three, ×1.5 for a card. The pick is weighted among the top three. At most 12
errands live on the server.

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
| Give | Hand items over, on the spot or at the step's place: food, a fuel can, a battery, something medical, an antidote (by the prefab's name; mission items never count) |
| Do | Go to a place and stay there long enough: search a farm, stand the gate, dig, count, fetch water |
| Sneak | The same, only while hidden (built [stealth](DESIGN.md#stealth)): seen, and the clock stops |
| Talk | A reply that resolves on the spot, sometimes with a Trust check ([RZ_Checks](DESIGN.md#checks-and-ways)), fixed, no roll |
| Pay | Favor with the giver's faction, paid when picked |
| Press | A threat, through `RZ_Checks.Threaten` (Fear against 2; +3 Infamy whatever happens) |
| Report, leave | Hand the matter to someone else, or let it go the giver's way |

**Places**, all within 1.5 km of the offer: the giver's place, its gate (15–25 m), 200–400 m out,
300–500 m out, the nearest empty village within 1.2 km, the nearest other town, or the card's own
place (a wreck, an impact, another house, the warning point at the line). A way whose place doesn't
exist here isn't offered.

**Done** is coming back: within 15 m of the giver's place (a wanderer: where they stand now), or
asking them again. **Helpers** are the asker's vanilla group within 50 m for 2 minutes or when the
step was done, three at most.

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
- **"Most people helped"**: each errand done counts on the hall of fame's line of that name, until
  the cure. It pays nothing.

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
| **The Pot** | Haven safehouse, the cook | food under one upkeep, and no Haven truck on its way to it | Hand over two tins [2 food]: 4 Favor, 1 Fame, +2 food. Search the gardens at the nearest empty village (30 s): 5 Favor, +2 food. Writes `errand.fed` |
| **Lights** | safehouse, the storekeeper | the house's fuel fault, dusk within 20 real min (or dusk) | A fuel can, or [Mechanic] a car battery: 5 Favor, the fault fixed. Writes `errand.lit` |
| **Water for {house}** | safehouse, the lookout | the house's water fault | Fetch water from the next town (20 s): 5 Favor (×1.3 quiet), +2 food, fixed |
| **Filter Parts** | safehouse, the storekeeper | the water fault, 8+ people, Known | The pump house part at the next town (25 s; [Mechanic or Lab Tech] 12 s): 8 Favor, fixed |
| **The Gate at Dusk** | safehouse, the lookout | dusk near, one guard or none, a town at 60+ infection within 1.5 km, not Every Ridge Calls | Stand at the gate 10 real min: 5 Favor |
| **The Burying** | safehouse, the clerk | someone died at the house lately | Dig at the edge of town (30 s): 3 Favor; a world record |
| **The Round** | safehouse, the clerk | [Nurse or Doctor], no medic, 4+ people | A minute at each bed (2 min): 4 Favor |
| **The Newcomer** | safehouse, the clerk | the house took someone in this season day | A coat from the next town (20 s), or [Nurse or Doctor] check him over (30 s): 4 Favor |
| **The Children's Hour** | safehouse | 5+ people, 08:00–17:00 | Watch the yard 5 real min: 4 Favor |
| **Make It Right** (amends) | safehouse, the lookout | the Haven Watches you | Keep the gate (3 min) or carry the night water (15 s at the next town): amends |
| **Shift Change** | Bastion post | the relief fault | Wake the relief 200–400 m out (10 s): 4 Favor, fixed. Report him: 1 Fame. Stand the post (3 min): 6 Favor, fixed |
| **The Set** | Bastion post, the radio operator | the set fault, 17:30–20:30 | A battery: 5 Favor, fixed |
| **The Wire** | Bastion post | a Bastion truck lost within 1.5 km in a season day, or a town at 50+ within 1.5 km; not Watched | Man the wire 5 min: 6 Favor |
| **The Count** | Bastion post | a Bastion truck lost near, or the post short; Known | Count three stacks (30 s), then write what's there (6 Favor) or what they want (10 Favor; a world record) |
| **The Bag** | Bastion post | night, Bastion morale under 40 | Put his letter in the mail bag: no pay; Bastion morale +1 |
| **Fuel for the Post** | Bastion post | the post short after a lost truck | A fuel can: 6 Favor |
| **The Latrine** (amends) | Bastion post | Watched or Stranger with Bastion | Dig it (30 s): Watched, amends; Stranger, 3 Favor |
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
| **His Lighter** (the dead) | Red Hand camp, a crewman or the cook | a death within 1 km in the last season day (never to whoever caused it) | Sell it to the fence: 6 Favor. Take it to his mother at a house within 1.5 km (5 s): 1 Haven Fame, a local (what he owed is in the book against her now). [Known: Bastion] sell it to Bastion's post as proof of one less: 5 Bastion Favor and a lie the rumour round finds (+4 Red Hand Infamy) |
| **The Market** | Red Hand camp, the fence | a Wedding Party cleared within 3 km in the last season day (its signal `wedding`) | Find the ring (20 s) and sell it to him: 8 Favor. Give it to the groom's mother at a house (5 s there): 2 Haven Fame, and the fence calls you soft. Sell it to her for two of the house's tins: 4 Favor, the house −2 food |
| **The Receiver** | Halcyon station or the lab, the technician | a tagged pack within 6 km, and the receiver down today (half the days, by the season's seed) | A battery at the mast 200–400 m out (10 s), or [Lab Tech] re-seat the cable (15 s): 5 Favor. Never near the pack |

*Simplified:* His Lighter's "crewman" is anyone who died near the camp, and The Market's ring is found
at the party's spot as a step, not as an item.

Four missions left the random pool for these and are gone from the code: Fuel for the Lights
(Lights), Water Purifier (Filter Parts), Water Run (Water for {house}) and Medicine Before Dark
(Before Dark).

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
| **The Truck Didn't Come** | aftermath, the storekeeper | this house's truck lost within 1.5 km in 2 played hours (never to whoever robbed it, or their group) | Salvage the wreck (30 s): 4 Favor, +2 food. [Known: Red Hand] ask a toll lad who: 3 Favor, a rumour naming no one |
| **The Smoke Was Ours** | aftermath, a Bastion named person | shells within 1.5 km in 2 played hours (never to whoever called them) | Count at the impact (10 s): 5 Favor. Two rations to the nearest house: 1 Bastion Fame, 1 Haven Fame, +2 food there. Tell the Haven whose shells: a world record |
| **Is It True?** | aftermath, a Haven named person | a rumour going round names a town within 1.5 km | Go and look (15 s): 4 Favor. Tell them it's true: the rumour runs 6 hours longer, no pay |
| **The Letter Nobody Can Take** | lost, the Courier | a house that lost people lately, another lived-in house within 1.5 km | Carry it there: 4 Haven Favor. Or give it back |
| **The Trader's Detour** | lost, a Trader | his destination's town at 60+ infection, or a mission there | Scout his road ahead (20 s): 4 Haven Favor; he remembers you helped |
| **The Deserter's Way Round** | line, the Deserter | he likes you, a Bastion road post within 1.5 km | Meet him past the post (10 s): 4 Haven Favor; helped. Turn him in at the post: 2 Bastion Fame, a rumour, a world record |
| **Nothing Goes Out** | line, a Bastion named person | the perimeter sealed, a sealed checkpoint within 1.65 km, Bastion morale under 50 | Leave his letter under a stone at the warning point, 150 m short (5 s), read or not: 3 Favor; the world record `errand.letter_at_line` with its epilogue line (carrier: a prop) |
| **His Effects** | dead, any faction's named person (Bastion: The Locker; others: The Wreck's Name) | that faction's truck lost within 1.5 km in a season day (never to whoever robbed it) | Bring his things back (15 s at the wreck): 5 Favor, 1 Fame; Bastion's boxes "for when the gates open" as an epilogue line. [Known: Red Hand] sell them to the fence: 5 Red Hand Favor, "kit lost" |
| **The Name on the Tag** | dead, the clerk | a death within 1.5 km in a season day (the background and how long they lasted, never the name), or the fallout of a Distress Call or Survivors Trapped whose people died | Leave the satchel on a cairn where they fell (10 s): 4 Favor. Or pin the line to the board. A world record with an epilogue line (carrier: the board) |
| **Not for the Crew** | crews, a Red Hand named person | a bite within 1 km in the last played hour, a Halcyon station within 1.5 km, before the cure | Walk him to the station: 4 Halcyon Favor, a world record. Your own antidote: 6 Red Hand Favor. Tell the book-keeper: 2 Red Hand Fame |
| **Wildcard Money** | crews, a Red Hand named person | Wildcard or Tolerated with the Red Hand; another Red Hand holding within 1.5 km | Carry the cut there: 6 Favor; skim a little: 8 Favor; give it to a hungry house within 1.5 km: +2 food there, 2 Haven Fame, +8 Red Hand Infamy |
| **Clean Slate** (amends) | any faction's named person | that faction Watches you | The worst shift going (a minute): amends |
| **Dress Rehearsal** (disguises) | crews, a Red Hand named person; Known with the Red Hand, below Known with Bastion; disguises on | a Bastion road post within 2 km | Walk up to it in Bastion kit and pass the look, 10 s at the barrier, and walk back: 10 Favor, 2 Fame. At the barrier and not passing: it's over, and the open channel laughs at someone in borrowed green (no name) |
| **The Officer's Coat** (house; disguises) | a Haven named person; you're disguised in Bastion kit; disguises on | a Bastion holding or post within 1.5 km | Tell her you're not Bastion (regard). Take the tin "for the paperwork": the house −1 food, a lie found by rumour (+3 Haven Infamy). [Known: Bastion] ask at the post yourself (10 s): 4 Favor, and *"He's on the roster as KIA, or struck off."* |
| **The Name on the Tape** (the dead; disguises) | a Bastion named person; Known with Bastion; disguises on | one of Bastion's people died within 1.5 km this season day and is still out there (the nail list, [REPUTATION.md](REPUTATION.md#whose-kit-it-was)); never to whoever killed them | Bring the tag and jacket home (15 s where they fell): 4 Favor, 1 Fame; the name is marked home and read on the nightly call. [Known: Red Hand] sell the jacket to the fence: 10 Red Hand Favor, and the board lists "kit lost". Once per dead name |
| **Spot the Tells** (Bastion; disguises) | a Bastion named person; Known with Bastion; disguises on | two or more different players caught by Bastion within 2.5 km in the last 2 season days | Find the scout by the tells (20 s among the three on duty): right (always with a Soldier or Police background, else two times in three) 5 Favor, 2 Fame; wrong, Private Hollis spends the night in the stockade and nobody pays. [Known: Red Hand] take the scout's money and point at Hollis: 10 Red Hand Favor, 35% it comes out (+5 Bastion Infamy) |
| **Paint** (the crews; disguises; with Name Day's Brush) | a Red Hand named person; Known with the Red Hand; disguises on | night, a Bastion road post within 2.5 km | Paint a red hand on the barrier (20 s there): in Bastion kit asking for a light (you must be passing as Bastion at the barrier, else it's over), from the back unseen, or in plain sight (Bastion's timed grudge and +3 Bastion Infamy). 2 Fame, the crew name Brush if you have none, and "a civilian with a paint tin" on Bastion's board |
| **Wrong Way Round** (the crews) | a Red Hand named person; Trusted with the Red Hand | a lived-in house within 2 km with no Red Hand holding within 1.5 km of it | Look at the house's backwards red hands (10 s there), then: tell the boss (2 Fame; the house loses a person; +5 Haven Infamy); take a cut (6 Haven Favor; 35% the house finds out, +5 Haven Infamy); warn them off (2 Haven Fame); recruit them (2 Fame; the house loses a person) |
| **Laundry Day** (help across lines) | a Haven named person; Known with the Haven | a Haven and Bastion truce, a Bastion post and a Red Hand roadblock both within 1.5 km | Carry the uniforms' sack to the post (10 s there) the long way, never within 150 m of the roadblock (else the crew keeps it and it's over), or pay the toll twice (20 Red Hand Favor): 2 Haven Fame, 1 Bastion Fame, +2 food at the house. [Known: Red Hand] sell the sack at the roadblock: 15 Red Hand Favor, the house −2 food, 35% found out (+3 Haven Infamy) |
| **Civvies** (help across lines) | a Haven named person | a Bastion holding within 3 km: a deserter struck off as KIA shelters at the house (a seeded name, never the finale's canon deserter) | Bring civilian clothes from the next town (20 s there): 2 Fame, +1 person. [Known: Red Hand] sell where the deserter sleeps (10 s with a runner): 10 Red Hand Favor, a rumour, and the back room is empty |
| **Masks Off** (help across lines) | a Halcyon named person; Known with Halcyon | a house within 2 km took a bite declaration (its reason is up) | Find the beak at the house (15 s there), a Haven orderly selling samples: turn them in (2 Fame, coat 17 back in the record), let them go (2 Haven Fame), or take the coat (no good at Halcyon's looks; 35% the lab finds out, +3 Halcyon Infamy) |
| **Notebook 31** (the dead; merged with His Notebook and Into the Record) | a Halcyon named person; disguises on | one of Halcyon's people died within 2 km this season and the coat and notebook are still out there; never to whoever killed them | Bring both back (15 s there): 2 Fame, the name on the authors' list at the change of shift. [Known: Red Hand] sell the notebook: 15 Red Hand Favor, 35% found out (+5 Halcyon Infamy). Keep it: its number is a cover detail at one Halcyon look |
| **Vouch** (house; the two Vouch cards merged) | a Haven named person (Known with the Haven) or a Red Hand one (Trusted) | always: a newcomer, seeded | Speak for them: the record `vouched.<name>`. The Haven: +1 person; with the card's twist they steal (−1 food, +5 Haven Infamy on you). The Red Hand: one time in three he's selling to Bastion, a liability in the lie store (+5 Red Hand Infamy when found). [Ex-Con] ask about inside first: the truth, before you decide. Or don't vouch. No pay |
| **You're the One** (champions) | a Haven named person; Champion with the Haven | always | Write his daughter's name in the council book (10 s): a line on the board. Or say you're no one: *"That's what the last one said."* No pay |
| **Pay Packet** (backgrounds: Unemployed) | the clerk | a house of 6+, not short | Hand out the payroll (3 min): 4 Favor, 1 Fame |
| **Behind the Red Door** (the lost) | a Haven named person; the Kid lives at this house, and you're Known with the Haven or he remembers you | the next town within 1.5 km | Find the red door (15–25 s there), then tell him the truth (4 Favor), bring him a thing from his room (4 Favor), or tell him she's coming: he waits at the gate at dusk from then on (a pinned local) |
| **Somebody Should Tell Them** (the lost) | the clerk | the house lost people lately, the infected's ground within 1.5 km | Read the marks at their ground's edge unseen (15 s), [Hunter] tell them, or check the book of names (20 s): the reason `house.lost_to`; regard, no pay |
| **Both Hands** (the crews) | any holding's named person; Mixed Bag, Useful Trouble or Favourite Rascal with them | a rival's holding within 1.5 km (a crew; the Red Hand's: a Bastion post) | Talk him out (Trust 2, 20 s there): 6 Favor, 1 Fame; his lot seeing you, +2 Infamy with them. Or leave him |
| **Name Day** (the crews) | a Red Hand named person; Known with the Red Hand, no crew name yet | a town the infected hold, or a Fight Pit running, within 1.5 km | Belfry: fetch the church clock (20 s there): crew name Legs. Ringside: one round in the pit (a minute): crew name Sorry. 5 Favor, 1 Fame each |
| **Professional Courtesy** (backgrounds: Raider) | a Red Hand named person | always | Give it straight: 4 Favor, 1 Fame. Steer him wrong: 2 Favor and a lie (+3 Red Hand Infamy when found). Tell Bastion at a post within 1.5 km: 2 Bastion Fame; a crew body seeing it, +6 Red Hand Infamy |
| **A Debt Gone Quiet** (the crews) | a Red Hand named person; Feared band 2+ with anyone | a lived-in house within 1.5 km | Find him and lean on him (15 s there): 6 Favor, +3 Haven Infamy (he was paying for medicine). Pay it yourself (10 Favor): 1 Fame. Say he's dead: 3 Favor and a lie (+3 Red Hand Infamy when found) |
| **Off the Report** (Bastion) | a Bastion named person; Known+ with Bastion, some Bastion Infamy | always | Lose the report: 4 Favor and a lie (+3 Bastion Infamy when found). Tell him to own it. [Soldier] tell him how to answer for it: 3 Favor, 1 Fame |
| **Dress Uniform** (backgrounds: Soldier) | a Bastion named person; Soldier, Known with Bastion | a death within 2 km this season day | Fold it the regulation way (30 s): 4 Favor |
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
| **Is She Gone?** (the infected) | a Haven named person | a pack within 1.5 km standing its vigil for a leader the house knew by name | Watch from the lair's edge, unseen (15 s): 4 Favor. [Elder] sit at supper while they say her name (a minute): 3 Favor, a record with an epilogue line |
| **Boarded Up** (players' traces) | a Haven named person | a town within 1.5 km that nobody holds, at 30+ infection, with a player's barricade standing | Bring the family out (10 s): +1 at the house, 4 Favor. Leave a tin there: 3 Favor. *"Whoever boarded this, tell them it held."* (never the builder's name) |
| **Old Habits** (backgrounds: Ex-Con) | a Bastion named person | a Red Hand camp within 1.5 km | Walk the wire and say where you'd come in (a minute at the gate): 5 Favor, 1 Fame. Tell him the wrong place: 3 Favor and a lie (+3 Bastion Infamy when found) |
| **Names on the Call** (Bastion) | the radio operator | civilians sheltered at Bastion's posts this season (`bastion.sheltered`), a house within 1.5 km | Pin the list on the house's board (5 s there): 4 Favor |
| **The Chalk on the Door** (the house) | the clerk | a town within 1.5 km with a work tally ([MISSIONS.md](MISSIONS.md#one-choice-jobs-and-work)) | Go and read the wall (15 s): 3 Favor. Say it's still true: 2 Favor and a lie |
| **After the Noise** (aftermath) | the lookout | a horde passed a town within 1.5 km (the reason `horde.passed`) | Walk the edge unseen (20 s): 5 Favor. [Hunter] read the ground (10 s): 5 Favor |
| **Last Orders** (the dead) | any holding's named person | one of their own leaders died within 3 km (the reason `leader.died`) | Bring his order book in unread (10 s at his seat): 4 Favor. [Known] read it on the way: 3 Favor and his last page, in his faction's voice |
| **The New Number** (Halcyon) | a Halcyon named person | a cure milestone in the last season day (the reason `cure.<percent>`) | Paint the new number on the board at the gate (10 s): 3 Favor |
| **The Last Apples** (the house) | the cook | Overrun or later, the house under two upkeeps, an empty village within 1.2 km | Pick the last of the orchard (30 s): 4 Favor, +2 food |
| **Clean Glasses** (margin: The Book Has to Balance) | a Red Hand named person | the major waits for its settling day or its last page; not 16:00–21:00 | Six unbroken glasses from an empty village (30 s): 5 Favor |
| **Count for the Sitting** (margin: Every Name) | the clerk | the major waits for a sitting; before 17:00; another house within 1.5 km | Carry the count before half past six (5 s there): 3 Favor; or carry it and say the house is hungrier than it says: 3 Favor, a record. It changes no vote |
| **Hand to Hand** (margin: the Independent) | a Haven named person | the finale's routes are open, you've locked to none, a local at this house knows you | Promise: no pay, the local pinned, the record `errand.hands` with an epilogue line; six a player a season. Or don't. A promise moves no track |
| **They Say You're With …** (lock card) | a Haven named person | you've locked to a route that isn't the Haven's, a local at this house knows you | Titled by your route (Bastion, the Red Hand, Halcyon; the Independent's: *They Say You're Doing It Yourself*). Talk it through; walk them to the next town, out of the stand's way; or carry one true word to your side. No pay; the local remembers |

### The epilogue deck

✅ 13 built (`Cards/Epilogue/`, `RZ_EpiCard`). The last errands, for the week after the cure
([EPILOGUE.md](EPILOGUE.md#the-closing-work)): dealt by the same dealer, only while the epilogue runs
(`m_bAfterCure`), once per identity per subject (a name, a house, a holding, a day's road), and
outside the two cards a day. They read the epilogue's record (the route, its terms, the queue, the
Names board, the named dead, who held each holding at the cure) and pay through the small-pay ledger;
nothing carries into next season. *Simplified:* any number may be offered at once, as with any
errand; the ones a beat would deal are dealt by their condition, not by the beat.

| Card | Family, giver | True when | Ways |
|---|---|---|---|
| **Carry the Dose** | epilogue, a named person of the faction whose rules hold | the route has dose outlets, before processing | Carry the sealed box to a holdout in an empty village (15 s at the door): 6 Favor, 1 Fame. Open it yourself: the dose, if you're bitten and haven't had an outlet dose this life; 10 Infamy with the giver's faction either way |
| **Pack for the Queue** | epilogue, a Haven named person; Known with the Haven | the house has people and the queue stands, before processing | Fetch both things from their old house in an empty village (30 s): 4 Favor, 1 Fame. Bring one and say the other was gone (10 s): 2 Favor |
| **Borrowed** | epilogue, the storekeeper or cook | this house short, a fed house within 1.5 km | Ask that house for it back (10 s at its gate): 4 Favor, 2 food comes back. Say you couldn't find them: nothing moves |
| **Take a Name to the Board** | epilogue, a Haven named person | a named dead person still lying out within 4 km, the Names board stands | Write the name on the board (10 s there): 3 Favor, 1 Fame; the wall lists it as home, by you |
| **The Toll Man's Last Debt** | epilogue, a Red Hand named person | the Red Hand's route or its book balanced, a house within 3 km, before processing | Carry the tins to the house (at the gate): 5 Favor, +2 food there. Sell them back to the crew: 4 Favor, +5 Haven Infamy |
| **Sign Here** | epilogue, a Bastion named person; Known with Bastion | Bastion's route on the roster or the column, the queue stands | Take three names at the queue (30 s): 5 Favor, 1 Fame. Write three yourself: 5 Favor; three in ten the corporal notices, +4 Bastion Infamy |
| **Copies for the Far House** | epilogue, a Halcyon named person | Halcyon's route, the method published | Carry the method to the lived-in house farthest from the lab (10 s): 6 Favor, 1 Fame. Sell the copies to the fence: 6 Red Hand Favor, +5 Halcyon Infamy |
| **Clear the Way** | epilogue, a Haven named person | the queue within 4 km, before processing | Hold the bend 400 m up the queue road for a minute: 5 Favor, 1 Fame (whatever's on the road comes to the noise) |
| **The Last Watch** | epilogue, an outpost's named person; Known with them | the holding was theirs at the cure | Stand the gate for ten minutes: 8 Favor, 2 Fame |
| **Last Letters** | epilogue, the Courier | the queue stands, before processing | Carry the bag to the queue (20 s): 4 Haven Favor, 1 Fame |
| **One Last Toll** | epilogue, a Red Hand named person | the Red Hand's route or its book balanced, the queue stands, before processing | Stand the toll at the bend for two minutes: 6 Favor, +3 Haven Infamy. Talk the crew out of it (Trust 2 with the Red Hand): 3 Haven Favor |
| **Into the Record** | epilogue, a Halcyon named person | one of Halcyon's named dead lies out within 4 km | Fetch the notebook (15 s where they fell): 5 Favor, 1 Fame; the name goes home to the lab |
| **Come In** (amends) | epilogue, any holding's named person; Watched or Hunted by them | before processing | Walk up to the gate and wait a minute: amends (2 Infamy off) |

*Not built* (designed in [designs/EPILOGUE.md](../designs/EPILOGUE.md#the-epilogue-deck-last-errands)):
The Key Where It Was (needs the newcomer's scenario), His Effects' epilogue variants, Somebody Should
Tell Them's epilogue version, The Boy, Processing and What the Cure Costs Here.

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
| A crossroads | 80–300 m from a road post, outside its barrier | any hour while the post stands |
| A wanderer's route | within 300 m of the leg a living wanderer is walking (where they are to where they're going) | while they walk it |
| An approach | 100–800 m from a safehouse or a holding, outside its wire | any hour |
| A campfire | within 60 m of a stove fire lit in the last 30 minutes, or a Red Hand road post's brazier | after dark (a stove fire: while it's fresh) |
| A stopped car | your own car, stopped 20 s with you out of it (the stranger walks up 60–90 m off), or a wreck within 150 m of a road people use | your car any hour; wrecks by day |
| A fresh event | within 400 m of a death, bite, fire or shelling in the last 2 played hours, or a found hook | while fresh |
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
help may become a [local](#locals). **Killing one** costs what killing a wanderer costs (+15 Infamy with
their people: the Haven's, or the Red Hand's for a crew lad), and no stranger finds the killer for a
real day.

| Stranger | Spots | Why | Ways |
|---|---|---|---|
| **Up on the Roof** (was Rooftop Rescue) | a fresh event; a town edge | a town the infected took, or a fresh shelling or fire, within 800 m | Talk them down when no infected is within 25 m: +1 at the nearest house, 4 Haven Favor, a local. Or leave them |
| **The Last Tin** | a road; an approach; a route | you carry 2+ food, a house within 1.5 km on half portions; never to the face on the notice | Give him a tin: a true tip (which way the nearest lot walks at dusk), and he's a local at his gate. Trade two tins for his watch (a common find). Or not today |
| **The Funeral for the Car** | a stopped car (a wreck) | by day, a wreck near a town nobody lives in | Push it into the ditch with him (10 s, 8 heat): 3 Haven Favor. [Mechanic] *"It's not dead. It needs a battery."*: he goes to tell the Mechanic. Siphon it first; he lets you |
| **Which Way's Morton** | a road; a route | she's walking to a house 600 m–1.5 km on, with a road post or a lot's ground between | Point her round (+1 at the house). Walk her past it (500 m): +1, 3 Haven Favor, a local. Tell her the toll's cheap: a lie found when someone the Haven knows passes the post |
| **Behind the Door** | a town edge at dusk | the town is the infected's | Walk them out toward the house now (600 m; a gate bangs on the far side of town and draws the infected there): +1, 3 Haven Favor, a local. Come back at dawn: if you're back within 40 m between 05:00 and 08:00, they come out (+1, 3 Favor). Or leave them food |
| **Two in the Back** | a stopped car (a wreck) | by day, a house within 1.5 km | [Mechanic] fix it (a minute), give a battery, or push-start it downhill (30 s, 6 heat): they drive home, 3 Haven Favor, the father a local |
| **The Digger** | a fresh event (a death) | a death within 400 m in the last 2 played hours | Help her dig (30 s, 4 heat; [Elder] says the words): 3 Haven Favor, a local. Tell her who it was (the death's description, never a player's name). Take the dead's things: she sees (+3 Haven Infamy), and you get one find |
| **Still Walking** | an approach, at dusk | a man was walked out and let walk on (`walk.out`) in the last season day; never while the Edge of Town runs | Point him to the lab, walk him toward it (600 m), or sit with him until it's dark (2 min; [Nurse or Doctor] 1). Pays nothing |
| **Warm Hands** | a campfire, after dark | — | Let him sit 5 real minutes: one true thing from the road (a lot that fed last night, their habit, a town gone bad). Give him a tin: a local at his house. Or send him off |
| **Seed Potatoes** | their own place, by day | an empty village within 1 km of the infected's ground or a town they hold, a house within 1.5 km | Dig his sack up (10 s, 6 heat): +2 food at the house, 4 Haven Favor. Say it was gone (the sack goes to the fence for you: 4 Red Hand Favor) and it's a lie. Or not your business |
| **Checkpoint Manners** | a crossroads (a Bastion post) | a Haven family with the post between them and their house, a rifle part in a tea towel | [Known: Bastion] say they're with you (+1, 3 Haven Favor). Walk them round the post (+1, 3 Favor). Hand the bundle in (+1, 2 Bastion Favor). A rumour later: the Red Hand bought something "for the rabbits" |
| **My Sister at Morton** | a road; a route | a house within 1.5 km lost people lately | Half the time he's her brother, half a crew lad (the truth deck): the tell (a red hand on his wrist) is named only to a Police or Ex-Con background, or Known with the Red Hand. Find her in a barn and tell him: reunited (3 Haven Favor), or, a crew lad and you knew, she never arrives (6 Red Hand Favor; seen by the Haven's, +12 Haven Infamy). Warn her on to another house (+1 there, a local). Call him out: right, he goes; wrong, *"Calling a grieving man a liar. Nice."* (+1 Haven Infamy) |
| **Wrong Side of the Toll** (dark) | a road; a crossroads | a Haven woman with a Red Hand road post between her and her house | Pay her toll (10 Red Hand Favor) or [Trusted: Red Hand] say she's with you: +1 at her house. Walk her round it: +1, a local. Sell her to the toll: 6 Red Hand Favor, she never arrives, that house's errands are half as likely for you all season, seen by the Haven's +12 Haven Infamy |
| **She Waves at the Line** (dark) | the line | the Zone sealed | [Known: Haven, or Nurse] talk her back to the nearest house: +1, a pinned local. Walk out there with her: the line's rules are for you alone. Or leave her: a Haven headline |
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

*Simplified:* a stranger stands at the spot itself rather than at an out-of-sight point 60–90 m off
(VERIFY); a crossroads is a road post's surroundings, not every junction; a campfire is a stove fire or
a crew's brazier at a road post (players have no campfire of their own); robbing or shooting at a
stranger has no rule of its own (only killing one); escorts are a walk to a point, the stranger going
on as data; Still Walking's *"a board note at his house later"* and the dose are words only.

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

*Simplified:* a local is shown while any player is near the house (not only one who knows them); their
lines are one per memo, not three; locals don't yet speak through threads or the epilogue's scenes.

## Twists

A card that can have a twist (`m_bTwists`: Vouch's newcomer who steals) has it at **Twist chance**
(0.2), at most one errand in three a player and never two in a row (the ledger counts). The same
replies show either way, and the twist never costs more than the errand's own stakes. Some cards and
strangers draw their hidden truth from the truth deck instead, the same for everyone at that subject
for 3 season days (Sleeve Down's bite, Vouch's Red Hand informer one time in three, My Sister at
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
  every 60 real minutes (**Radio ask minutes**).

## Telemetry and the admin view

- **Log lines**: every deal (`Errands: dealt <key> to <player> by <giver> (asked|called out)`), take,
  done, overtake and fault; every stranger (`Errands: stranger <kind> for <player> at <grid> (<spot>)`,
  `Errands: stranger <kind> done by <player>`, `Errands: stranger <kind> killed by <player>`), found hook
  (`Errands: found hook <kind> at <grid> after <title>`), radio ask (`Errands: radio ask to <player> for
  <name> at <house>`), twist (`Errands: <key> for <player> has its twist`) and local (`Locals: <name> (<role>)
  made at <house> by <player> (pinned|unpinned)`); every fallout (`Errands: fallout <card> at <grid> from <title> (...)`); and every 30 minutes (**Telemetry**) a status line per family: dealt,
  declined, lapsed, overtaken, done, Favor, Fame and amends paid, and the ways taken, then strangers
  dealt by spot kind and helped by routine.
- **The admin view needs no typing**: on the talk screen a server admin sees one more reply on any
  named person, wanderer or occupant, **[Admin] Errands near here**: the live count, the nearest
  player's last deal and why each of the others wasn't dealt (*"lights: not true. pot: cooling
  down."*), their errands, the stranger spot kinds plausible where they stand now (*"road, approach"*),
  the strangers out and the locals. It's also written to the log.

## Saving and performance

- **Saved** (state.json, wiped with the season): each identity's ledger (cards done, cooldowns, the
  hour's Favor, cards today, errand Fame today, amends this life, the last three families, errands
  done, the houses whose errands are half as likely for you, the twist count), the safehouses' named
  people, the locals, each house's count of errands done there and the Haven's reading day of each lot
  (Fresh Chalk). Held errands,
  offers, faults, soft holds, strangers, found hooks and the footfall grid are runtime only: a restart
  lets them lapse.
- **Cheap**: no map scans. A deal checks one giver's anchored definitions; steps are distance
  checks on the 5 s tick; faults, guards and call-outs run every 30 s.

## Simplified, and not built yet

*Simplified:* a step's place is a point, with no scene person or prop at it (strangers are the only
scene people); the lines have two or three variants each, not always three; a card dealt counts toward
the day's two even if declined; pay in kind is an effect (errands hand out no items; a stranger's gift
is one common find). The margin cards read the majors' waits (the run's stage), and the lock cards read
the finale's lock (`RouteOfPlayer`). Not built yet, and designed in
[designs/ERRANDS.md](../designs/ERRANDS.md): the epilogue deck's last six ([above](#the-epilogue-deck)), the other
majors' margin cards, promises
and callbacks from errands, threads, His Effects in full (the effects as an item, a body near a post),
chore slots per holding, and the `mission_errand` icon (the journal uses the unarmed icon).
