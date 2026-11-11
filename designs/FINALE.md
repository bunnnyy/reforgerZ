# The cure finale: the fight for control

**Status: designed, not built.** Approved by the author on 2026-11-11.

## What changed in review

The author's decisions (numbered as in the plan's "Decisions for you") and the plan's conflicts
3–25, applied below. The alternatives that were not chosen are gone.

- **The new lore is confirmed** (decision 2, conflict 10). The captain's Day 2 log (*"13:10. Results to
  Command by radio: 4 R / {count}. Ack."*), Sedlak leading the Day 1 cordon detail, the woman bitten on
  Day 4, the eight who walked out of the Day 6 trial and the radio ham are canon. The "lore to confirm"
  marks are gone, and the lore goes into LORE.md in the finale's commit (§17).
- **Twice is Private Marek Fiala, alive** (decision 3, conflicts 4 and 5). He's struck off as KIA and his
  name is still on the board, so his term is *"A bottle, and my KIA rubbed out."* Bastion's route can
  bring him home (§6.2, Processing Again). He hides from Bastion, so his yes is one step harder for
  anyone in Bastion's uniform or locked to its route. Struck Off and Hunt the Deserter never use his name.
- **"Struck off" is only Bastion's word for a deserter** (conflict 7). The book's word is "paid": the
  Haven's door is "mark his line paid", and under *pay when you can* the unpaid lines are "marked paid".
- **Old lore fixed:** the crews take their own cut only when a man's worth less than a real dose
  (conflict 6); the two Bastion men have been at the lab since Day 5 (conflict 8); Wolff stayed on Day 3
  with three of her team, and the clipboard says *"Four of us staying"* and hangs at the lower store, not
  HC1's keypad store (conflict 9); the CO's manifest says *"List to follow"*, because "attached" now
  means "Civilians, attached" (conflict 3).
- **The gate news is reworded** (conflict 11): *"What's missing isn't in a sample case..."*, since every
  route asks for live tissue (§3, §14).
- **Take the Lab stays as the Red Hand route's once-a-season theft** (decision 6, conflict 12). Any
  Red Hand-locked player who is Trusted with the Red Hand can start it (built: Champion), and every
  Red Hand-locked player counts for the bay hold (§9).
- **No disguise near the lab during the stand** (conflict 13): one `IsFinalStand` check in
  `RZ_Disguise`, within 1.5 km, like stealth's (§9, §14).
- **Claims** (conflict 14): route stages never claim the lab. The controller claims it only for the batch
  hold and the stand; stages claim their own subjects (§5).
- **Bastion brings its own pieces** (conflict 15): the gate usually comes in Aftermath, when Bastion's
  presence is 10%, so the route's road posts, truck and squad are stage blocks, and a presence floor of
  0.25 applies while it has an active lock (§6.2).
- **The finale's own limits** (conflict 16) replace the limits every questline keeps (§13), and
  QUESTLINES.md's rule becomes "No faction wins through a questline except the cure finale, and none
  falls" (§17).
- **No crossing-lines mark** (conflict 17): finale stages never write it; the lock's rules replace it and
  end at the cure (§5).
- **The finale's own lane in the season budget** (conflict 18): morale ±10 and strength ±5 per faction,
  so the majors spending theirs never clips the finale to words (§13).
- **At most one half rule set next season** (conflict 19): the deal partner if there was a deal, else the
  Fame leader, then the majors' story rules (§13).
- **The runtime test comes first** (conflict 20): the reused missions and every questline must start and
  end once in the runtime test before this is built (§14).
- **The Haven's price door is "In pencil"** (conflict 21), built on reputation's Knock, so the two
  mechanics no longer share a name (§4).
- **Promises never ease a stronghold leader** (conflict 22). Hand to Hand eases only the eight's yes and
  shortens the lab-gate sit-in. The director's check counts the locals at the gate, not promises (§2, §6.5).
- **Hand it in goes through `Forgive()`** (conflict 23), inside reputation's 30 per faction per life (§6.2).
- **This file is the source for the Independent's pieces** (conflict 24): its own subject (the eight and
  the ham), at most one unit from another route's subject, the lab-alone track only for the slow finish,
  and a lock counting at a stand only if it came before the batch was ready.
- **The finisher's Fame chunk stays** (decision 4, conflict 25). Season Fame decides the hall of fame's
  "Zone's Fame" line, who the cure is handed over to when nobody holds the room, and the Fame leader's
  next-season rules at half (§10, §13).
- **The Independent gets no Fame chunk** unless it controls and chooses *one side*; then that faction
  gets half (decision 5, §10).
- **The Coat Run re-labels the batch inside the lab** (decision 7, §9). The batch never leaves the cold.
- **Acts I and III are open** to anyone not locked elsewhere; the piece and the last step are sworn
  (decision 8, §5).
- **Killing locked players** (decision 9): free at the stand between locked sides; outside it +3 Infamy
  with the victim's route faction (the Haven, for an Independent), 12 an hour at most (§8).
- **Halcyon's route is "The Lower Store"** (decision 15). "Proof" stays the major's (§6.4).
- **From the author's other decisions:** the epilogue lasts one week, 168 real hours (decision 12); the
  persuade rank is your access rank, a mixed tier one lower (decision 11); day parties follow the
  author's clarification in [INFECTED.md](INFECTED.md#day-parties) (§7); and `docs/FINALE.md` is added
  only as the finale is built (decision 16, §17).

---

It builds on hooks that are built:
- the 90% gate, `AddFinaleResearch` and the finale fallback (`Cure/RZ_CureResearch.c`);
- the final stand, and Take the Lab (`RZ_Mission_TakeTheLab`, on `RZ_Mission_Hold`);
- season Fame (`m_aSeasonFame`);
- the questline engine, world facts and claims;
- stealth, per-player AI hostility (`SURV_H1`–`SURV_H15`), safe zones and Live Capture.

It keeps the edges the New Vegas plan fixed in its section 3: the commit node, `CanCommit`, `RouteOf`, route assets and the lock rules. The Independent's pieces are the ones in §6.5: its own subject (the eight and the ham), at most one unit from another route's subject, and the lab-alone track as a sixth track used only for the slow finish (§11). The plan's section 3 follows this file. It assumes the plan's phases 1–5 and tribes and crawlers ([INFECTED.md](INFECTED.md)). Every dependency has a fallback (§14). Every change to built behaviour is listed in §14.

**Words used here**

| Word | Means |
|---|---|
| The gate | Research held at 90% while a finale is registered (built: `IsAtFinaleGate`) |
| Route | One of the five finale questlines |
| Track | A route's 10 points for the last tenth of the cure |
| Piece | What a route brings the lab to finish the batch |
| Door | The one thing a faction route needs that a rival faction holds |
| Other way | The slower road round a door. Every door has one |
| Locked | Committed to one route for the season |
| Active lock | The player has 2+ hours played this season and was online in the last 2 real days |
| Open act | Acts I and III. Anyone not locked to another route may decide them |
| Sworn act | Acts II and IV. Only that route's locked players may decide them |
| Finish | A route's track reaches 10. The first finish makes the batch |
| Holder | The side whose name is on the batch right now |
| Controller | The holder when the stand is won. It controls the cure |
| Side | At the stand: one route, or two routes in a pact |

---

## 1. The finale in one page

At 90% the lab stops. Its batch is missing one thing, and every faction thinks it knows what. Five routes open at once: the Haven, Bastion, the Red Hand, Halcyon and Independent.

Before they lock, players can work for any route. A player locks to one route at its commit node, the point of no return. From then on they work only for that route, and against the others.

**Three things decide the season, in this order:**

| What | Winning it means | When | How it's contested |
|---|---|---|---|
| **The finish** | Your route's piece is in first. The lab makes the batch from it. Your faction gets the route's capped chunk of season Fame. Your side starts the stand holding the clean room | The last 10% (about 4–9 real days) | The race, the doors, sabotage, theft of route items, pacts |
| **The clean room** | Your side holds it when the stand is won, so you control the cure | The stand: 15 minutes, plus at most 3 of overtime | Force, Take the Lab, the Coat Run, a partner turning on you |
| **The terms** | Your route's people decide how the cure reaches people | The release, straight after the stand | Only the controller decides. A pact partner adds one clause |

**Both count.** Finishing pays Fame, and holding the room wins control. A route can finish and lose the room, or take the room without finishing. Strongholds and the lab never fall: the clean room changes hands, the lab doesn't.

**The ring of doors.** Each faction route needs one thing another faction holds, in a ring:
- the Haven needs the Red Hand;
- the Red Hand needs Halcyon;
- Halcyon needs Bastion;
- Bastion needs the Haven.

Each door follows an old grudge:
- the Haven and the Red Hand over family and debts;
- Bastion and the Haven over the Day 2 column;
- the Red Hand and Halcyon over samples and stickers;
- Halcyon and Bastion over the Day 4 night at the lab doors.

So deals, sabotage and betrayal come out of the structure. Every door has a slower other way, so no route is ever locked out.

| Route | Questline | Its piece | Its door (who holds it) | The other way | Its admission, repeated at the lock |
|---|---|---|---|---|---|
| The Haven | The One Who Didn't Turn | Her blood, given three times by her own yes | Her boy's line in the book (**the Red Hand**) | She gives without it, one step harder: *"He's a grown man. He chose his crew."* | *"The houses decide who goes first, and the houses kept the List."* |
| Bastion | Held for Collection | Three good tubes from the Day 2 box, signed in at the lab | The slips on the bus driver's clipboard (**the Haven**) | Any three of four ledgers name the tubes, and the CO signs a manifest | *"It reaches those who comply, and the register is a census."* |
| The Red Hand | The Cook | A case of cut doses cooked on a tray of the lab's batch, proved on someone who asked | A tray of the 90% batch (**Halcyon**) | Cook on four stolen antidote doses: a harder base | *"The price is the queue. The cut dose was sold the same way."* |
| Halcyon | The Lower Store | The reference stock from the closed site, compared in the lab | Item 114, the key to Bastion's chain (**Bastion**) | The sergeant's second key, or cut the chain | *"The paper starts on Day 1. The three weeks are on page one, or not at all."* |
| Independent | One Each | Blood from three of the eight who walked out of the Day 6 trial, each by their own yes, and a tray off the books | Whoever holds each of the eight (**every faction**) | They walk out on their own; or one unit from another route's subject | *"Nobody guards it and nobody counts it, and every faction remembers who took it from them."* |

Every route also needs **live tissue** from the infected (§7).

The director: *"Whoever brings me the last piece gets to write on the label. I'll make it either way."*

**What locking means.** The lock card says this in plain words:
- You work for this route until the cure. You can't switch, and dying doesn't undo it.
- Other routes' work refuses you: *"You've picked your side. This isn't it."* Ordinary errands and missions carry on.
- Disguise looks are one step harder with every faction whose route you didn't pick. For the Independent, that's all four.
- Your cards for this route become playable, and the rest go dormant. What you know stays yours.
- At the stand you count only while your lock is active, and only if you locked before the batch was ready.

**The timeline at the design point** (5 regulars, the built pacing):

| When | What |
|---|---|
| About day 39 (day 34 with 20 regulars, day 45 with 2) | The gate. Five routes open, and commits open |
| Gate + 1–2 days | Act I done on most routes. Most regulars have locked. The first pacts |
| Gate + 2–4 days | Pieces and live tissue. Doors traded, guarded and stolen |
| Gate + 4–5 days uncontested, 6–9 contested | The first finish. The batch takes 12 real hours and 2 hours of play |
| About day 44–49 | The seal and the stand. Control. The cure. Then the epilogue's week (168 real hours) and the wipe |
| Nobody finishes | The built fallback makes the batch about 17 days after the gate (about day 56) |

---

## 2. Before 90%: nothing locks

**Route assets** are the missions design's renamed "leans":
- at most one per mission outcome;
- at most 3 per player per route in a season;
- shown on the journal's finale page as "What you'd bring".

An asset does nothing until its owner locks to that route. Then it's a card the owner plays once:

| Asset | In that route |
|---|---|
| Ally | Someone the route can call on once. They open a door, vouch at one check (one step easier), or carry a route item up to 600 m (an escort block that ends at a place, never a follower) |
| Proof | One talk check one step easier, or one search or deduction skipped |
| Standing | One leader's price a third lower, or one leader's check one step easier |

Assets for other routes go dormant at the lock and lapse at the cure. No asset can close a route.

**Hand to Hand** (errands) allows up to 6 promises per player. In the finale a promise does two things only:
- a promised local living in the same house as one of the eight makes that person's yes one step easier;
- each promised local present at the Independent's lab-gate sit-in takes a minute off it (§6.5).

A promise never eases a stronghold leader's check: the director's last persuade counts the locals standing at the gate, promised or not (§6.5). Promises also feed the ending's scenes. They never move a track.

**Pitches** (tone):
- From 75%, route topics answer *"Ask me again when it's nearly done."*
- From 80%, "Ask about the cure" works at the chair, the CO, the boss and the director. The Independent pitch comes from the Doctor wanderer, else the Courier, else an unsigned sheet at the lab gate.
- At 85%, each pitch goes on air once, naming nobody.
- Each pitch gives its best argument, then its admission.

**Seeds.** Each route plants rare, unmarked seeds from 25–50% research (§6), so a curious player can know a subject before the gate. Before the gate, every subject's place refuses force:
- *"It's Bastion's chain. Somebody signed for it."*
- The captain's door: *"Not collection."*
- The still is only smoke at night.

**Limits that hold before 90%** (the plan):
- a mark makes a check at most one step harder;
- a sulk lasts at most 2 hours of play;
- Infamy works off as usual.

---

## 3. At 90%: the routes open

Research reaches 90% and stops at the built gate. It holds there because the five route questlines report `IsCureFinale`.

1. The gate line is heard when someone is in the world: *"The lab has gone as far as samples can take it: 90 percent. What's missing isn't in a sample case, and every faction in the Zone thinks it knows where it is."* (reworded from the built line, §14). The broadcast changes to tone's gate line: *"The status of this area is under review."* The stall timers don't count at the gate.
2. At the next change of shift (05:30 or 17:30), the lab's open channel says: *"Ninety. It won't go further on what we have. There's more than one way to finish it, and most of them aren't in this building."*
3. The same night, each route opens with its own line:

| Route | Where | The line |
|---|---|---|
| The Haven | The Haven's secure channel | *"All houses: nobody says her name on the radio. She moves tonight."* |
| Bastion | The 20:30 call | *"...Stores, detached. Held for collection, {held_count}. Fuel, {fuel_words}. Request instructions. Over."* A long hiss. *"No reply."* Under Bastion, Command, the stronghold answers instead: *"Stores, Command. Hold for collection. Command out."* |
| The Red Hand | The Red Hand's open channel | *"Ninety percent and it won't leave the fridge. Ours has been riding round in a lorry since Day 9. Just saying."* |
| Halcyon | The duty officer's overnight numbers | *"Batch at ninety. Reference: none."* |
| Independent | The open channel, after the lab's numbers | An old voice reads them back slowly, the way you check a sum: *"Ninety. Ninety. Got it."* Then the Doctor: *"Four lots of people are about to tell you the cure is theirs. It isn't anyone's yet. If you'd rather it went to everyone, any radio will do."* |

4. Every route's Act I opens, the commit topics open, and the journal's finale page opens for everyone.

**What players see**
- **The journal's finale page** shows everyone:
  - the five tracks and the lab's own track (§11);
  - the route's worth: *"About 375 season Fame to the faction that finishes."*;
  - "What you'd bring".
  
  Who holds what, and a route's notes, are shown only to that route's players.
- **The lab's readout** at each change of shift reads the tracks by faction, never by player: *"Ninety-four. The Haven's work is at seven, Bastion's at five, the Red Hand's at three, ours at six. Nobody's is at four."*
- **The cure percent.** `{cure_percent}`, the HUD and the lab's line show 90 plus the furthest track, up to 99 until the seal.
- **Rumours** say who is close, and some of them are wrong.

**What carries on**
- **Samples.** They are still taken and paid, into the backlog until the cold store is full. Then they're refused and the player keeps them, as built. They don't move research past 90. The backlog is the lab's reserve for a remake after a lost stand, and samples shorten Halcyon's comparison (§6.4).
- **Other work.** Ordinary missions and errands carry on. Majors still running carry on, and never move research.
- **The end.** All finale content stops at the cure.

---

## 4. The lock

### Where, and who may commit

**Where you commit**
- **A faction route:** an `always` topic at that faction's stronghold leader, or at any Loyal seat leader. A leader who refuses to talk to you can't block it.
- **The Independent route:** the open channel, at any powered house radio or on a carried one.

**Who may commit.** The engine decides (`CanCommit`), never the text:
- research is at the gate or past it, and the stand hasn't been won;
- you haven't locked this season;
- `CanSideWith(you, faction)` says yes, by parley (you're owed), or by price (below). The Independent route is always open.

### The confirm node

A dialogue node tagged **[Point of no return]** shows three plain lines:
- **the admission:** the route's own line (§1);
- **what you bring:** your assets and notes for this route;
- **what you leave:** your assets for other routes go dormant. Notes about other routes stay yours, and selling one later is crossing that faction.

Example, at a Loyal Bastion seat:
> *"From now until the cure, you stand with Bastion on the lab road. Their cure reaches those who comply, and the register is a census. You bring: one proof (the black box in the stove). You leave: the woman's house, which you know. Dying doesn't change it."* [Write me down] [Not yet]

**Fallback without the dialogue engine:** two action-menu steps. "Commit to {route}", then "Confirm (no return)" within 30 s. The three lines arrive as an owner-only notice.

### Price doors

A price door is for a player the route's faction hunts. It opens the route only, and forgives nothing: Infamy stays and works off as usual. While that player is unarmed inside one of the route's own stage scenes, the faction's people there hold fire (the built `StandDown`, as the parley uses it).

| Route | The price door |
|---|---|
| The Haven | **In pencil.** Built on reputation's Knock (the parley at a safehouse door for exiles who are owed). At the knock, at dusk, unarmed, at any safehouse door, an exile who isn't owed can offer 4 food for the pot and a name to read: one of your own dead characters, or a crossed-out name you found. A council head (never the chair) writes you on the List in pencil. That opens the route, not the door |
| Bastion | **Sign on, attached.** Surrender at a checkpoint as built (unarmed, searched) and hand over one firearm. Sedlak writes you on the roster: *"Civilian, attached."* |
| The Red Hand | **A line in the book.** The boss writes you a debt of 60 Red Hand Favor. Red Hand Favor you earn pays the line first |
| Halcyon | **The volunteers' book.** At a field station, unarmed, give a draw of your own blood by your own yes (or hand in three samples), and sign with a notebook number |
| Independent | None: it's always open |

### How long

- **For the season.** The lock is per identity for the season, and dying doesn't undo it. All locks end at the cure.
- **Late joiners** and the undecided can commit until the stand is won. A lock made after the batch is ready doesn't count at that stand (for holding or deciding). It counts at a later stand.
- **Active locks** count for route marks, pacts, holding and deciding. A lock with no activity for 3 real days pauses its route's stage contracts (§11).

### After the lock

These are the plan's rules; the finale owns them.
- **Only your route.** No errand or mission offers you route work for another route. An option that leans toward another route refuses you when you pick it. Labels stay the same for everyone.
- **Disguises.** Looks are −1 with every faction whose route you didn't choose. For the Independent, that's all four.
- **Parley and settling up** close with any faction whose route is **fighting** yours (§8).
- **What they call you.** Your word gains an overlay: "attached", "the other crew", "an uncontrolled variable"; for the Independent, "nobody's".
- **Route lines** are face to face only. They're cool on the first visit and back to normal over two real days. Shared channels never single out anyone's route.
- **Route marks** on the verge count active locks only, and they're **private**: every route with an active lock gets the same share of places (`finale.marks_public` 0), so the walls don't give numbers away.

### Choosing where to lock

1. **Before 90%**, you build up route assets through missions, and standing with factions. The finale page lists what you'd bring to each route.
2. **From 80%**, you can ask each pitch-giver. Every pitch makes its admission before you choose.
3. **At 90%**, you can scout. Do Act I (or catch tissue, Act III) for any route you like. You keep the pay and the notes, and you owe nobody.
4. **You lock** at a route's commit node whenever you're ready: at once, or after scouting several routes.
5. **After the lock**, your notes about other routes are yours to keep, give to your own route, or sell. Selling one is crossing the faction it's about: +10 Infamy with them if a lookout sees, and their leader remembers.

**Worked examples**
- **Mara worked for the Haven all season.** She has two Haven assets: an ally (the deserter the Haven hid) and a proof (a manifest read at supper). At 90% she finds the woman (the Haven's Act I). Out of curiosity she also finds the cold store's grid (Bastion's Act I). Then she locks at the chair.
  - Her ally vouches for her at the woman's door.
  - Her proof skips the bite book.
  - The grid is still in her notes. She gives it to a Haven friend who wants the woman's Day 2 tube out of Bastion's box.
- **Pavel worked for the Red Hand until it hunted him.** He can't side with the Red Hand by standing. He could pay its price door (a line in the book), or go Independent. He goes Independent on the radio, and his two Red Hand assets go dormant. The Haven calls him "nobody's"; Halcyon calls him "an uncontrolled variable".
- **Ana joins on day 41, at 93%.** Unsided, she's courted once a visit. She catches a lone one in the hills and chooses, at the hand-in, to credit Bastion's route, which pays her (Act III is open work). She likes the people, and signs on at a Loyal outpost two days before the Haven finishes. She counts at the stand.

---

## 5. How every route runs

**The same four acts on every route:**

| Act | Kind | Track | What |
|---|---|---|---|
| I | Open | +2 | The question: what's missing, where it is, who holds the door |
| II | Sworn | +3 | The piece, through the door or the other way |
| III | Open | +2 | Live tissue (§7) |
| IV | Sworn, last | +3 | Bring it in, and the last step at the lab |

**How the acts and points run**
- **Order.** Acts II and III run side by side. Act IV needs both.
- **Points** come when a step is done or something is handed in, never for carrying. They're never taken back, so a stolen item costs a route time, not points. A lost subject moves the route to its branch, which costs time.
- **The cap.** A track never passes 9 before Act IV's last step.
- **The finish.** A route finishes when its track reaches 10 with a tray in the lab (the Red Hand's case brings its own). Steps resolve one at a time on the server, so two routes can never finish together.

**Open and sworn work**

| | Open (Acts I and III) | Sworn (Acts II and IV) |
|---|---|---|
| Who decides | Anyone not locked to another route who can side with the route's faction (anyone at all, for the Independent), and the route's own locked players | Only players locked to that route |
| Who helps | Anyone | Anyone not locked elsewhere. A helper (60 s within 150 m, or a step done) gets half the stage's Favor and no Fame. A player locked to another route is refused at every choice |
| Pay | The stage contract's pay (at most 10 Fame and 20 Favor with the route's faction) to the decider, once per route per player | The same, to locked players only |
| Ends | The first time it's done. Everyone who took part keeps their notes | When its steps are done |

**Lanes.** Routes run in their own lane, outside **Max active** and the faction slots. They never fizzle: a give-up retries the stage. They freeze at another route's finish and end at the cure.

**Claims.** Route stages never claim the lab (`LAB`). The controller claims it only for the batch hold and the stand. Stages claim their own subjects instead: `Person(key)`, `Tray(n)`, the store and the still. So several routes' stages can run at the lab at once: the Haven's last draw, the Red Hand's case check, Halcyon's comparison hold and the Independent's batch sheet.

**No crossing-lines mark.** Finale stages never write the questline engine's crossing-lines mark ([QUESTLINES.md](../docs/QUESTLINES.md#crossing-lines)). The lock's rules (§4) replace it, and they end at the cure.

**Shared check rules**
- **Stage people** are props, with a body only within 300–400 m of a player.
- **Sneak** uses the questline sneak (`m_Quiet`), the heat limit and `RZ_Stealth.SightRadius`. Seen at a sneak usually costs +8 Infamy with the owner.
- **Persuade** uses `TryPersuade`. It's a fixed score, not a roll (built): rank 0–3 (your access rank with that faction; a mixed tier counts one lower), ★ background +1, a step easier or harder from flags, kept between 1 and 4.
- **Threaten** uses the one Fear score. It costs +3 Infamy. It's never used on stronghold leaders, or inside safe zones and safehouses.
- **Bribes** use `TryBribe`.
- **Lies** pay at most 60% and no Fame.

**Things every route fights over**

| Thing | Who wants it | Rule |
|---|---|---|
| **The trays:** the lab's three trays of the 90% batch | The Red Hand's door. Every finish needs one in the lab. The Independent sets one aside | The director refuses a second tray to a route that has one out. One is rebuilt each real day |
| **The cold** | Every carry | The lab's transport case (25 minutes, lent to anyone Trusted with Halcyon), the vet's tank (a real day for a bag), the works' flask (if canon has one), pharmacy cool bags and couriers' cases (15–30 minutes, always there). One holder at a time. A drained source is gone for the season |
| **Her Day 2 tube, R1** | The Haven (it counts as one bag) and Bastion (one of its three) | One tube, one use |
| **Live tissue** | Every route, once | Scarce on purpose (§7) |
| **The annex** (Mobile Lab, `SetAnnex`) | A draw point, a cold stop, a tissue hand-in | One route at a time, lent by the director to anyone Trusted with Halcyon |
| **The helicopter** (`HeliAvailable`, `pilot.stays`) | A carry with no clock | One flight a real day, with no storm |

**Route items count only for their own route,** with these exceptions, which are the crossings:
- her R1 tube counts for the Haven;
- one tube counts toward Halcyon's second best;
- a bag, a tube or a vial can be the cook's harder base;
- any one unit counts for the Independent.

---

## 6. The five routes

### 6.1 The Haven: The One Who Didn't Turn

**Why it's the Haven's route:**
- **Day 2:** she was on the bus Lieutenant Sedlak turned round at the north road. Her Day 2 tube is R1 in Bastion's box (§6.2).
- **Day 4:** she was bitten in the church doorway, the morning the priest opened it. She declared it, as every house asks. The priest sat up with her all night, and in the morning she asked for tea.
- **Day 6:** she was meant to walk up the hill with the volunteers. Her council head kept her back, and the eleventh line of the lab's Day 6 ledger says *"did not present"*. Then her house crossed her off the List with the pencil it uses for the dead, so the lab wouldn't come for her and Bastion couldn't count her. Hers is the one name on the List crossed out as a kindness.
- **Since then** she's been "the lodger", moved from house to house at dusk: a mouth on half portions, in a network whose rule is that everyone works.
- **Her son** runs with a Red Hand crew, and his line is in the book.
- **The Preacher** wanderer is the Day 4 priest.
- **Nobody**, the writing included, ever says why she didn't turn.

**The hard rule:** nothing is drawn without her yes. *"They crossed me off to keep me safe. Nobody asked me either."*

| | |
|---|---|
| Piece | Her blood three times: two stored bags (her R1 tube counts as one) and a last draw, fresh, at dawn |
| Door | Her boy's line in the book (the Red Hand) |
| Other way | She gives without it, one step harder |
| Pitch | *"The cure goes hand to hand, to the people the others would leave till last."* |
| Canon | `unturned.name`, `.son`, `.crew` (the Red Hand seat nearest her first house), `.house` (a season flag: every 3 real days at dusk the rota moves her, as data, to the next Haven house no other story holds, avoiding the chair's house while there's another; on a one-house map she stays put) |

**Seeds** (each about one line in fifteen of its kind, from 25%):
- *"The house in {town} puts a bowl out by the back door every night. Says it's for the cat. They haven't got a cat."*
- A medic, after a bite is declared: *"We had one who didn't take, back in the first week. Don't ask me about her."*
- The lab's readout: *"Day 6 cohort: eleven referred, ten treated, eight discharged. One did not present."*
- The Preacher, asked to stop his bells: *"I sat up with a woman once who didn't need me to."*
- Wrong rumours, never marked: *"The Haven's got a woman who can't be bitten."*

**Her terms**, heard when she's found:
- *"Not up that hill."*
- *"Nobody gets it first because they've got guns."*
- *"Say my name. Out loud, where I can hear it."*
- *"And my boy."*

The Haven can meet the first three itself. The fourth is the door.

**Act I. The Crossed-Out Name** (open, +2)
- **When:** from the gate, usually 1–2 real days. The chair gives it to anyone Known with the Haven, never to exiles: *"There's someone. We kept her from you. We kept her from everyone."*
- **Goal:** her name, her house now, and tonight's spot. Tonight's spot is the house or an outbuilding near it, never the same two nights running, and it goes cold at the next dusk. Speak to her there, by day or at dusk, never after dark, with weapons left 20 m off.
- **Ways through:**
  - **Talk.** Council heads answer by trait:
    - Merciful heads tell anyone Known.
    - Loyal heads send you to the chair.
    - Paranoid heads give anyone below Trusted a decoy.
    - Independent heads want a job done for the house first.
  - **The people who were there.**
    - A ★ Nurse or Doctor reads a house's bite book: Day 4, declared, no time of turning ever written.
    - The Preacher (Known with the Haven) names this week's town.
    - With `walk.walks_stay`, the walker remembers waiting at the church door.
  - **The records.**
    - The lab's Day 6 ledger: ask the clerk (Known with Halcyon), sneak it on the night shift, or pay a courier 20 Halcyon Favor.
    - Bastion's Day 2 processing list, with her name beside R1: Trusted with Bastion, or 15 Bastion Favor.
    - The List itself, with `list.read`.
  - **Sneak.** Watch a house at dusk from 100 m for the second bowl carried out, or tail the walker on a rota night past two lookouts. Seen: +8 Haven Infamy, and she's moved at once.
  - **Bribe.** 3 food to her hungry house, and its council head answers you. Or 20 Red Hand Favor to the strays crew for tonight's spot.
  - **Trade.** Buy a note (§8).
- **Rivals:**
  - They watch, bribe and buy notes.
  - A hungry house may sell tonight's spot to a crew (25% every 10 minutes while it's short), so feeding the house protects her.
  - A Haven-locked player can **move her**, once a real day: walk her to the next house at night, unseen, and every rival's notes on tonight's spot go cold.

**Act II. Three Times** (sworn, +3: her yes +1, each stored bag +1)
- **When:** after Act I. One visitor at a time, and each player can put their case once. Draws are at dusk or after dark, at least 12 real hours apart (× Wait scale).
- **Goal:** her yes for the Haven route, then two bags stored at the lab. *"Day 2 I was on the bus. Day 4 I was in the church door. Day 6 I was meant to walk up that hill, and I didn't. I'd like to be useful before I'm a candle."*
- **Her yes.** Persuade at difficulty 3, on Haven rank:
  - ★ Nurse, Doctor or Elder: +1.
  - Each term met: one step easier.
  - Someone she believes beside you: one step easier. That's the Preacher, the walker (`walk.walks_stay`), or her son once he's free.
  - Under workers first (`haven.rule` 2) she was sent to the far house as a mouth: one step harder.
- **Meet her terms:**
  - **Up the hill:** the draw point is the edge of her town, where the houses used to walk the bitten out at dusk. The route never uses the lab's back room.
  - **Guns:** promise the bite book. This binds the terms to *by the bite book*: a real cost.
  - **Say my name:** the chair reads it uncrossed at supper in every house. It needs Trusted, ★ Elder, or Known with `list.read`, and costs Haven morale −5 through the finale's lane in the season budget (§13). Under Every Name it's already met.
- **The door: "And my boy."** Mark his line paid:
  - **Trade:** a pact with the Red Hand route. One of her bags as the cook's base is the obvious price.
  - **Pay:** 30 Red Hand Favor, Trusted with the Red Hand. *"A debt is a debt. It was paid, by her."* Half price under Paid at the Door.
  - **Threaten** his crew boss (Fear, +3 Infamy).
  - **Trick:** a look at the fence's desk at night (difficulty 3). Found out, the debt doubles.
  - **Walk him out** of his camp at night. He comes because she asked, and his line moves to her house: dying doesn't pay a debt, and neither does leaving.
  - **The other way:** she gives without it, one step harder.
- **The draw:**
  - **Somewhere to draw:** the annex, or an improvised clinic in a hall (a generator, a pharmacy fridge, and a vet's centrifuge for 25 Red Hand Favor at the fence).
  - **Someone to draw:** a house medic, the Doctor wanderer, a field lead, or a ★ Nurse, Doctor or Lab Tech player.
  - **Hold it:** 4 minutes while the generator's heat brings the infected.
  - **Or keep it quiet:** after dark, no shots within 150 m, heat under 35. No waves come, and it's a minute shorter.
- **The carry, cold:**
  - the transport case: 25 minutes;
  - a cool bag: 15 minutes;
  - the vet's tank: a real day;
  - +10 minutes with a ★ Nurse or Doctor carrying;
  - by helicopter, no clock.
- **R1, her tube**, counts as one bag:
  - **trade** with Bastion's route: her tube for the clipboard is the obvious swap;
  - **buy** it: 40 Bastion Favor, Trusted;
  - **take** it from whoever holds the box. Bastion's box then has three tubes and no slack.
- **"One more."** A lost or spoilt bag needs another draw, with her yes. She gives five at most, never a sixth.
- **"Two is enough"** (Trusted with the Haven, or ★ Nurse or Doctor): spare her the dawn. Act IV then becomes 2 real days of the lab's work instead of the walk. It's kinder and slower.
- **Betray:** carry a bag elsewhere. At the still it's a harder base for the cook; in Bastion's box it's a unit. A Haven-locked carrier takes +15 Haven Infamy, and she needs one more draw.
- **Rivals:**
  - **Ask her yes for their own route.** Her yes goes to one route at a time, and each yes elsewhere uses one of her draws.
  - **Spoil a case**, once each (§8).
  - **Custody** (§8): she gives that holder nothing. In daylight, Haven players can hold her door for 3 minutes against a custody walk.
  - **Ambush a bag** on the road.

**Act III. Who It Was** (open, live tissue, +2; §7)
- The Haven's own way: a ★ Elder or Nurse reads the tribe's list of the taken and says who the leader was. That house asks for him alive, and its medic's sedative gives a second wake reset.
- The words: *"Whoever they were, bring them in alive. We'll say the name if we know it."*

**Act IV. Up the Hill** (sworn, last, +3)
- **When:** at dawn (05:00–09:00), at least 12 real hours after the last stored bag: the hour the ten walked up on Day 6. It needs a tray in the lab.
- **Goal:** the last draw, fresh, in the lab.
- **Ways through:**
  - **Walk her up**, only with her yes: *"I'll walk. Not carried."* It's an escort block that ends at the lab gate, at her pace. Houses on her road light candles. If the Preacher walks with her, his bells bring the infected; ask him and he stops.
  - **The dawn run.** Draw at first light and carry it up in 12 minutes (+5 with a ★ Nurse or Doctor), or by helicopter with no clock.
  - **Sneak** the courier road before dawn, with heat under 35 all the way.
  - **Force:** a truck's cab through the roadblocks (not under No Vehicles).
  - **Bribe:** 30 Red Hand Favor buys no roadblock this morning. 20 Halcyon Favor opens the gate to a holder the lab doesn't trust.
  - **Betray:** hand her or the bag to another route on the last mile. The act reopens under the new holder the next dawn, and only with her yes.
- **Rivals:** they intercept. At 8 points this is a Last Mile target.

**The Haven's terms** (chosen at the release, §9):

| Terms | Needs |
|---|---|
| **Her house, and her boy** | — |
| **The far house first** (the mouths) | Only under workers first (`haven.rule` 2) |
| **By the bite book:** the sick first, whoever they are | Forced if the guns term was promised |
| **The crossed-out first:** the List read from the bottom | `list.read`, or her name read at supper |

**If she's lost.** She can die only as a body near players, in a raid or on a road, or walk off alone after a third betrayal (custody, a diverted bag, a broken term). Being taken by another route doesn't lose her: only her holder changes. Bags already at the lab still count. Then one of two branches:
- **What She Left.** Her notebook is at the last house she slept in: temperatures, the scar, a joke a day. If she walked off, it's on the notice board: *"For whoever needs it. Not for whoever took me."* Carried to the lab, it stands in for her remaining draws at 1 point every 2 real days.
- **Twice** (R2; §6.2). He moves himself.
  - His yes: Haven or Red Hand rank, whichever is higher, at difficulty 3. ★ Soldier or Ex-Con +1. Bread makes it one step easier. One step harder for anyone in Bastion's uniform or locked to Bastion's route: he knows Bastion is looking for him.
  - He gives up to six bags: *"Take what you want. Then leave me be."*

**Remake step** (after a lost stand): her next draw if she agrees, or her tube.

### 6.2 Bastion: Held for Collection

**Why it's Bastion's route:**
- **The order.** At 06:00 on Day 2, the order to walk civilians to the road checkpoints said to screen them first, *"with the samples to go along with them"*.
- **The tubes.** At a tent short of the north road barrier, the battalion's medical officer, Captain {canon:held.doctor}, took a tube from everyone in Sedlak's column: the bus, forty cars and the line on foot, {canon:held.count} people. Sedlak rolled up her sleeve first so the column would, and her detail did the same.
- **The slips.** Every tube got a number. Every person got a slip with the number and their name, stapled to the bus driver's clipboard for the barrier.
- **The log.** The captain's log has two lines that afternoon: *"13:10. Results to Command by radio: 4 R / {count}. Ack."* and *"14:00. Barrier not lifted. Column turned back."* Nobody says what anyone outside did with that number in the fifty minutes between them.
- **The freezers.** The samples had nobody left to go with, so he wrote *"Held for collection"* and took them in two chest freezers to the battalion's cold store in the hills. That's `held.store`, not the stronghold's bunker.
- **Since then.** He's still there. The roster says *"Detached, stores: 1."* Since Day 6 everyone has known nobody is coming. He kept the freezers anyway: it's what he was told, and they're all he has.

**His two rules:**
- He opens to "collection", and only the CO can say it.
- A tube goes only with its slip: *"The samples go with the people. That was the order."*

Halcyon reads R as "has seen it, and won". The four marked tubes:

| Tube | Who |
|---|---|
| R1 | The woman the Haven hid (§6.1) |
| R2 | **Private Marek Fiala**, LORE's *"KIA Day 7. Body not recovered."* He was on Sedlak's cordon detail on Day 1, bitten through the sleeve, and never turned. He gave his tube with the column on Day 2. He walked out of Zeman's post on Day 7 with what came out of the barn, which keeps LORE's "the only one to walk out" in the first ten days true. He was struck off: his name is still on the board, his nail is empty, and nobody answers for him at the roll. He runs with the Red Hand crew nearest the store now, and the crews call him Twice: *"Killed in action once. Bitten once. Neither took."* He keeps out of Bastion's sight, because he knows someone is sent to bring a struck-off man home. Bastion's route can do it (Processing Again). Struck Off and Hunt the Deserter never use his name |
| R3 | A Scavenger wanderer who won't go near a uniform |
| R4 | Dead |

| | |
|---|---|
| Piece | Three good tubes from the box, signed in at the lab |
| Door | The slips on the bus driver's clipboard (the Haven) |
| Other way | The four ledgers: any three name the four tubes, and the CO signs a manifest: *"Collection. List to follow."* |
| Pitch | *"Order: a queue, a register, nobody trampled at the gate."* |
| Canon | `held.store`: high ground 1.5–5 km from Bastion's stronghold and 2–4 km from the lab where the map allows, 800 m from settlements, outside the warning zone, never `origin.site` or the Deserter's Map bunker; snapped to a military building within 400 m by prefab name, else the Locked Bunker's hatch; the distances relax on small maps. `held.count` (280–340), `held.marked`, `held.doctor` |

**Bastion brings its own pieces.** With 5 regulars the gate comes about day 39, inside Aftermath (from about day 31), where Bastion's presence is 10% and its presence-scaled posts and contracts mostly don't run. On 2 regulars it runs in Overrun (×0.25). So the route's stages bring their own pieces as stage blocks, never from the presence-scaled pools: up to 2 stage road posts on the store–lab roads, one truck and crew, and one squad. While Bastion's route has an active lock, the majors' `SetPresenceFloor(Bastion, 0.25)` ([MAJORS.md](MAJORS.md)) applies, inside the season budget.

**Seeds:**
- the store's line on the nightly call all season, with the temperature creeping up with research;
- *"Diesel 20 L: stores (HFC)"* in post ledgers (a ★ Soldier knows what HFC means);
- a weekly 05:30 diesel run (as data, with a body only within 600 m);
- Day 2 slips in safehouse drawers: *"NR-2 / 147. {name}, Bus 1. Retain this slip and present it at the barrier."*;
- from 75%, the director: *"Nobody in this Zone was tested before the bites started."*

**Act I. Request Instructions** (open, +2)
- **When:** after the gate's call. One last diesel run leaves at 05:30 two mornings later. Then the store's fuel clock starts: 40 hours of play, never more than 60. Once it's dry, one tube spoils for every 8 hours of play it stays dry.
- **Goal:** the store's grid, and what's in it.
- **Ways through:**
  - **Talk.** Sedlak at the roster board (Trusted with Bastion, ★ Soldier, or Known with a Day 2 slip): *"Only the CO can say collection."* A quartermaster (Trusted): *"Everything's signed for. Even the man nobody signs out."*
  - **Sneak** the quartermaster's fuel ledger at night, past two lookouts (seen: +8 Bastion Infamy). Or **tail** the last run, never closer than 150 m (★ Hunter: 100 m).
  - **Bribe** the quartermaster: 20 Bastion Favor.
  - **The Haven's way.** The bus driver (Known with the Haven) saw the medics' truck take the cold boxes up the hill road (Trail of Clues).
  - **The Red Hand's way.** The fence reads the diesel line in the book: 15 Red Hand Favor, free if Trusted. The boss now knows (`held.rh_knows`).
  - **Halcyon's way.** The field stations' sensors show the infected drifting toward one empty hillside. Read it from two high points and find the hum (★ Radio Op: a bearing from the captain's 20:30 log).
  - **The Deserter** wanderer, who talks only to players Bastion doesn't like.
- **Rivals:**
  - The grid sells once per faction for 15 of its Favor.
  - The Red Hand's Dry Run (Hijack the Convoy on the last diesel run) starts the fuel clock 10 hours short.

**Act II. Collection** (sworn, +3: the store open +1, the box out +1, names for its tubes +1)
- **When:** once the route knows the grid. The captain logs on his set at 20:30 and sleeps 23:00–05:00. The CO speaks on the call at 20:30.
- **Opening the store:**
  - **The release.** Persuade the CO at the stronghold radio at 20:30 (Bastion 3, or Trusted as ★ Soldier or Police).
    - One step easier under `orders.stance` 3, with `orders.copy`, or after a refuel.
    - One step harder under stance 4.
    - The CO reads it on the call: *"Stores held for collection released to bearer, on my authority. Over."* It never changes the CO's loyalty or seat.
  - **Bribe:** 40 Bastion Favor, and the adjutant slips *"Bearer to collect"* among the night's signal forms. The captain opens, but the CO finds out at the receipt: *the roster* terms close.
  - **Refuel** (anyone). Two jerrycans (one for a ★ Mechanic) in the generator, then hold within 60 m for 90 s while the hum brings a horde. That's +10 hours, up to 60. The captain remembers who kept his freezers cold: his checks are one step easier for them.
  - **A face.** Escort the bus driver to the door: *"You took my blood on Day 2, and my passengers'. I drove them back."* The captain packs the box for you, and the terms are locked to *the column*.
  - **Threaten.** He's been alone with two freezers since Day 2, so he frightens easily (Fear, +3 Bastion Infamy). He opens, and pulls one plug as you leave: one tube spoils.
  - **Trick.** A forged signal form saying "collection" (★ Radio Op, or a real 20:30 transcript as the cover detail). Found out at the receipt: +10 Bastion Infamy, and *the roster* closes.
  - **Sneak.** Between 23:00 and 05:00, open the outer door with item 113 (from the CO's safe, or Halcyon's wrong-key trick) or a pick (★ Ex-Con). Take the box while he sleeps, never within 20 m of him awake. At 20:30 he logs *"Stores missing."*
  - **Force.** Cut the door (60 s, loud) and take the box off him: +15 Bastion Infamy (+25 if you're Trusted with Bastion). He pulls the plugs before he goes down: one tube spoils.
- **The door: names for the tubes.** The captain releases a tube only with its slip.
  - **Trade:** a pact with the Haven route. Her R1 for the clipboard is the obvious swap.
  - **Persuade** the driver (Known with the Haven): *"They were my passengers."*
  - **Sneak** the clipboard out of the house office at night (seen: +8 Haven Infamy).
  - **Threaten** him (Fear, +3).
  - **Force:** +20 Haven Infamy.
  - **With Counted** (`haven.rule` 3), the census stands in for the slips.
- **The other way: the four ledgers.** What became of each passenger is spread over four ledgers: the Haven's List, Bastion's census and nail board, the lab's Day 6 ledger, and the Red Hand's book. Any three name all four tubes.
  - For each: talk to its holder, read it at night (+8 Infamy if seen), pay 20 of its Favor, or solve Patient Zero's Ledger's deduction.
  - Then the CO signs a manifest. This sets `held.named`.
  - **Her name is the most dangerous thing learnt here.** Whoever solves it chooses who's told: the chair (who moves her early), the director, the CO, or a crew boss (40 Red Hand Favor).
- **No names at all.** The CO orders the captain to open:
  - under Bastion, Command, he obeys;
  - under any other stance he obeys, and one tube is "spoiled in transit" as his protest;
  - under Expended he opens to nobody but the column.
- **Rivals:**
  - **The Red Hand's Cold Meat** raid (Trusted, with `held.rh_knows`): a tube for the still.
  - **The drain** (20 s at the tank, unseen): −10 hours, at most 10 a real day from all drains.
  - **The Haven's sit-in.** The column sits unarmed at the stronghold gate for 10 minutes during the call; a shot within 100 m breaks it. R1 is then released "to the column": to the Haven.
  - **Halcyon's courier.** Once the captain agrees to pack, hold the door for 3 minutes and the box leaves signed to Halcyon: a tube for second best.
  - **Guard the door:** the Haven moves the clipboard into the chair's office.

**Act III. Contact, Captured** (open, live tissue, +2; §7)
- Bastion's own way: a squad with a cage on a truck, brought as a stage block (Contracts > Bastion squad; Trusted with Bastion). The contact board gives the last place a big one was seen. In Aftermath (Bastion's presence 0.1) the board is old, and a ★ Soldier reads the tracks to bring it up to date.
- The words: *"A contact, alive. Bring it in on a stretcher and sign for it."*

**Act IV. Signed For** (sworn, last, +3: the box signed in with three good tubes +1, the receipt +2)
- **When:** once the box is out. It stays cold for 45 minutes (60 with a ★ Doctor or Nurse carrying), counted only outside a freezer and while someone is online.
  - Freezers: the store, field stations, the lab, the stronghold's medical store, the still.
  - A Haven safehouse with a medic or power holds it for 30 minutes, once per house.
- **Goal:** the box at the lab. The lab tests it against a tray (1 hour of play). The receipt is read on the next 20:30 call: *"Command, Bastion. Stores held for collection: collected. Over."* A long hiss. *"Bastion out."*
- **Ways through:**
  - **By road** (Trusted with Bastion): the route's stage truck and crew (Convoy Escort's truck, as a stage block). The route's stage road posts wave it through. Not under No Vehicles.
  - **By air.** Hold an LZ by the store for 2 minutes; the helicopter carries it with no clock.
  - **On foot, quietly,** through the hills with heat under 35 (★ Hunter 45). A field station on the way is a freezer.
  - **The Haven's road** (Known with the Haven): house to house, with lookouts who know today's road posts.
  - **The crews' roads** (Trusted with the Red Hand): a crew driver on the toll roads. He drives to the still unless he's paid 20 Red Hand Favor.
  - **Fight or bribe** a roadblock (20 Red Hand Favor; Trusted are waved through).
  - **Hand it in.** A thief with Bastion Infamy can surrender at a checkpoint, unarmed, with the box. The usual 8 Infamy comes off through `Forgive()`, inside reputation's limit of 30 worked off per faction per life, and the box goes to Bastion's route. With that budget spent, the box is still taken in for Bastion's route, and no Infamy comes off.
- **Rivals:**
  - The route's stage road posts (up to 2) stand on the roads between the store and the lab while the box is out. The box is contraband for anyone below Known with Bastion.
  - **The Red Hand** ambushes it. A box at a Red Hand drop goes to the still's freezer for a real day plus 8 hours of play. Bastion can buy it back (40 Red Hand Favor, four jerrycans, or item 114), talk it back (Trusted), or raid it back.
  - **Halcyon** signs it in at a field station in Halcyon's name.

**Fewer than three good tubes: Processing Again.** The route asks the living for blood, by consent only. There's never a dead end:

| Source | How |
|---|---|
| **The living reactive** (needs `held.named`). Each gives once, to whichever route asks first and is told yes | R1, the woman: a yes to Bastion uses one of her draws. R2, Twice: talk (Trusted with the Red Hand: *"A bottle, and my KIA rubbed out."*), or buy his time off the crew boss (40 Red Hand Favor). His yes is one step harder for anyone in Bastion's uniform or locked to Bastion's route. **Bastion's way: bring him home.** Find him through the Deserter wanderer or the crew's book, and walk him to a post (an escort block that ends there). The sergeant rubs out his KIA, he answers *"Here"* at the roll, and the captain releases his tube *"to its own person"*. That meets his term as well. R3, the Scavenger: come unarmed and out of uniform, and share food twice |
| **Donors** | A "Give blood" action (Blood Drive's) at Bastion posts, Haven medics and field stations. Once a life per player, Bastion Fame +5. Every D donors at one post make one unit. D is 3, plus 1 per 4 regular players, from 4 to 8 |
| **The long way** | Sign the receipt short with at least one good tube. Each missing tube adds 2 real days, with Power the Lab at least every 2 real days |

**Bastion's terms** (chosen at the release):

| Terms | Means | Needs |
|---|---|---|
| **The roster** | Soldiers first, then civilians attached. Anyone can sign on as attached at a post to be on it | Not if the box came by bribe, a forged form, sneak or force |
| **The column** | The living of the Day 2 column first, by their slips | The slips, or `held.named` |
| **By need** | Triage: the sick first, whoever they are | Known under `orders.stance` 2, else Trusted |
| **The census** | Signed for everyone, Command's way: names taken at the table | Stance 3 only |

**Remake step:** one session of the lab's work on what it kept.

**The coda** is the majors' (Bastion's last call and the 05:30 walk). This route adds one clause and no second coda: the captain walks beside Sedlak with his empty box, and the four R numbers are read last, by name with `held.named`. If the route brought Twice home, he answers *"Here"* to his own name ([MAJORS.md](MAJORS.md)).

### 6.3 The Red Hand: The Cook

**Why it's the Red Hand's route.** At 90% the lab's batch works in the clean room and spoils fifteen minutes out of it. The cut dose works half the time (built) and rides for a week in a lorry cab. The Red Hand can cook a cure, but it can't fence the one thing that opens the gates, because only the lab's numbers count. So for once it walks in through the front door.

**The cook** (fitted to LORE):
- **Who she is.** She cooks the dusk meal (18:30) at the Red Hand stronghold. She ran the school kitchen in town until Day 2. The crews call her Doc because she isn't one.
- **Day 2.** She queued at Rudi Kasper's forecourt all day. That night Rudi sold her forty litres round the back and let her pay later, because he knew her. Her line is on Rudi's first page, under the boss's ten litres.
- **Day 7.** She took Rudi his supper the evening the boss told the brothers he'd been bitten. She knows he wasn't; everyone inside does. She says it once, flatly: *"He let people pay later. That's what he died of."*
- **Her debt.** Rudi's tally became the boss's book, and every "pay later" in it became owed to the boss. A debt is a debt, and dying doesn't pay it. She's paid hers off in the kitchen ever since.
- **The still.** After supper she cooks the other thing, on an old farm still in a barn nobody lives near. That's the cut dose. Every one goes out half now, half when you come back. The ones who come back pay; the ones who don't are bad debts.
- **The tally.** Beside the book she keeps her own pages: which pan each line's dose came from. The Red Hand ran the biggest trial in the Zone and wrote the misses up as debts. On a tray of the lab's batch, she says, it would hold every time and keep in a pocket.

**Its grey** is the boss's price against her "pay when you can". That was Rudi's way, and it's why he was put out. The boss: *"He let people pay later. You can't run a road like that."*

| | |
|---|---|
| Piece | A case of cut doses cooked on a tray of the 90% batch, proved on someone bitten who asked |
| Door | A tray of the 90% batch (Halcyon) |
| Other way | Cook on four antidote doses instead: the black market's stolen stock (built cap 4) or the quartermaster's. It's a harder base: a step lower, and two nights |
| Pitch | *"It reaches anyone who can pay, including people nobody else would serve."* |
| Canon | `redhand.cook`; `redhand.still` (outside a small town 1–4 km from a Red Hand seat that nobody holds, never within 1.5 km of the Long Table's farm); `cook.sister` (at the Haven safehouse with a medic nearest the stronghold); `cook.lines`, `.paid`, `.unpaid` (every cut dose a player takes gets a line, written at the cut dose's cure roll; a few seeded lines a real day keep a quiet server's book full) |

**Seeds:**
- The quartermaster, selling one: *"Half now, half when you come back. You're in the book. Line 212."*
- The fence buys veterinary antiviral.
- From 50%, headlines mention smoke at night past the still's town.
- From 75%: *"The lab's doses go off in a quarter of an hour. Mine keep a week. Work that out."*

**Act I. The Kitchen Book** (open, +2)
- **When:** from the gate, at the dusk meal. About one session. The boss: *"My cook says she can finish it. Find out if she's lying, and what she wants."*
- **Goal:** find her and hear her four needs: a tray of theirs, a night on the still, a dose that holds in a pocket, and the lab's word. Hear her own term too: *"Mark my line paid the way you marked yours."* Whoever hears her gets `cook.heard` (personal).
- **Ways through:**
  - **Talk** at the dusk meal (Known with the Red Hand). A paid line in the book gets the whole story at once: *"You came back. Paid twice."*
  - **Persuade:** Red Hand 2. ★ Ex-Con, Lab Tech, Doctor or Nurse +1.
  - **Her sister** (Known with the Haven; 3 food) says where the still is.
  - **Follow her** after supper, 50 m back, past one crew lookout. Unseen, you learn where the still is (`cook.still_known`).
  - **Test a dose.** Carry a cut dose to the lab with no cold box. A ★ Lab Tech or Doctor reads the cut in 30 s (`cook.dose_read`).
  - **Bribe** the fence: 20 Red Hand Favor.
  - **Copy the tally.** It's in a tin at the still by day. Seen by the lookouts: +8 Red Hand Infamy, and the tin moves to the stronghold. Two copies at most.
- **Rivals:**
  - **Bastion** stops her on the road: a checkpoint holds her for its commander (+15 Red Hand Infamy). She then cooks in the post's mess under guard, a step lower.
  - **Anyone** can tell the boss someone's asking. Outsiders then need Trusted, and the lookouts double.

**Errand: Who Came Back** (open, from Act I until the proof). Her tally's last page names three paid lines who now live with other factions:
- one at a Haven house;
- one on Bastion's census, or in the stockade;
- one in the lab's volunteers' ledger, whose cut didn't hold, so he rules a pan out.

Each line confirmed is a step at the proof. A false answer is sabotage: a ★ Lab Tech or Doctor at the still throws it out, and the liar takes +10 Red Hand Infamy.

**Act II. A Tray and a Night** (sworn, +3: a tray at the still +1, the pan cut +2)
- **When:** talk or trade for the tray at any hour. Anything else only on the lab's night shift (20:30–05:00), never while a stand runs. The still runs the first night the base is there.
- **The door: a tray.**
  - **Trade** in a pact with Halcyon: a page of her tally for a tray. The lab keeps the page (`cook.lab_copy`), so the method goes on the paper. One step easier under `book.last` 1.
  - **Ask** the director (Trusted with Halcyon): *"I want to see it keep."*
  - **Buy:** 40 Red Hand Favor to an Independent or Rogue field lead, who walks a tray out at night "for the field annex".
  - **With Two Tins a Copy,** the director's pages go back for a tray.
  - **With `authors.bastion_key`,** a deal with Bastion gets you in through the clean room door: unseen, no fight.
  - **Steal:** Steal the Antidotes' 90 s hold, at the night cold store (★ Lab Tech: twice as fast). Halcyon morale −5, and Lab Defence follows.
  - **Trick** the back door in a coat (a look, difficulty 3).
  - **Rob** a courier or another route's tray on the road.
  - **The other way:** four antidote doses. Or a reference vial, a marked tube, or one of her bags (only by her own yes, and it uses one of her draws).
- **The tray's clock:** 15 minutes out of the cold (25 with a ★ Nurse or Doctor). Cold stops pause it: a Haven clinic, a Bastion cold room, the annex, the transport case. At the still, her freezer keeps it a real day while the generator has diesel.
- **The still.** It needs two jerrycans and six boxes of veterinary antiviral (her shelf counts). *"Not in daylight. The smoke shows."* Keep it running while she cuts the batch:
  - **Fight:** hold it for 8 minutes against its waves. The pan is cut, with no step.
  - **Run it cold:** 12 minutes with no light, no shots within 150 m and heat under 35 (★ Hunter 45). A clean run is a step.
  - **A ★ Mechanic** fixes the leaking coil, and the heat halves. A ★ Lab Tech, Doctor or Nurse reading her thermometer makes a fought run clean.
  - **Talk her through it** when she loses her nerve (*"Rudi used to say I'd poison somebody."*): persuade 1, or her sister's letter.
  - **The rival crew boss** comes at midnight, wanting her cooking for him. Fight his crew (no Red Hand Infamy: it's the boss's quarrel), bribe him off (30 Red Hand Favor), talk him down (Trusted, or ★ Ex-Con), threaten him (Fear), or let him in for a cut (`cook.crew_cut`).
- **Rivals:**
  - **Bastion** raids the still with the grid.
  - **Halcyon** watches from the barn (a lab notebook, or 20 Halcyon Favor): a step, and `cook.lab_copy`.
  - **The Haven** stands watch and fetches water. With two of them there at the end, a Haven–Red Hand truce is rolled.
  - **Sabotage:** expired antibiotics in her crate (a step lower, unless a ★ Lab Tech or Nurse checks the crate first). Or drain the still's diesel: the tray spoils within the hour unless someone refuels.
  - **Guard the door:** Halcyon moves the trays into the night cold store and doubles the night shift.

**Act III. Fresh Stock** (open, live tissue, +2; §7)
- The Red Hand's own way: **the moat.** A crew keeps one tribe fat as a moat against Bastion, and its leader is old and well fed. The crew boss sells the lair and the scouts' hours (30 Red Hand Favor, or Trusted). Taking it opens the moat: the moat rule stops for that tribe until its succession.
- The words: *"A live one. The pit crews know where they lie up."*

**Act IV. The Lab's Word** (sworn, last, +3: the proof holds +1, the case checked at the lab +2)
- **When:** a real day after the still, because the dose has to keep out of the cold. By day only (05:30–18:30).
- **The proof:** one dose to someone bitten who asks for it, told plainly: *"It's ours. It's never been tried on a tray of theirs. It might not hold."* They're watched for 5 minutes while the infected come.
  - The chance is 60%. Each step adds 15%: a paid line confirmed, a clean run, Halcyon watching, or `authors.formula_sold`.
  - A harder base takes 15% off, and so does each uncaught sabotage. It's kept between 40% and certain.
  - **If it fails,** the volunteer has the cut dose's usual failure (built: time ×0.6) and keeps their trial-dose chance at the lab. She cooks again, a step higher. The third pan always holds.
- **The volunteer:**
  - one of the crew: his crew boss must agree (persuade, Red Hand 2). The crews take their own cut only when a man's worth less than a real dose, and every man knows it;
  - a Haven member who declared a bite (Known with the Haven);
  - a Bastion soldier, with his commander's leave;
  - a lab volunteer (Trusted with Halcyon);
  - Twice, unless Bastion has brought him home: *"I've walked off two. Give it here."* His blood is then spoken for;
  - yourself, if you're bitten: +10 Red Hand Fame either way.
- **Catch the swap.** With `cook.crew_cut`, the rival crew boss swaps a dose. Check the case's seal (10 s; ★ Police), and he's walked to the boss and replaced. That's the route's one leader change.
- **Tell the volunteer it's poison** (a rival): they refuse, and another volunteer is needed.
- **The case to the lab gate.** The case has no clock: carrying it across the map is the first proof. The director checks it in an hour of play. Everyone on a rival route is told a courier is out, with a rough spot every 10 minutes.
  - **Escort** it: ring the lab road with crews for 3 minutes.
  - **The front door:** the duty officer (Known with Halcyon, or a vouch).
  - **The lorry:** force through a checkpoint.
  - **Sneak** off the roads. A Trusted courier is waved through Red Hand roadblocks.
  - **Bribe:** buy the case off the cook's crew boss before it leaves (60 Red Hand Favor) and carry it yourself.
- **Her own term:** *"Mark my line paid the way you marked yours."*
  - Persuade the boss at difficulty 3.
  - Or use her leverage: she'll tell the Haven and Bastion what the shutter was. The boss: *"Everyone inside knows. Outside it's a rumour. You'd make it a fact?"*
  - Met: the boss marks her line on the first page *"Paid"*, in the same hand as the boss's own (`cook.line_paid`).

**The Red Hand's terms** (chosen at the release):

| Terms | Means | Needs |
|---|---|---|
| **The boss's price** | Two tins a dose (30 Favor), sold at the crossroads | Trusted with the Red Hand, or the boss persuaded. The director signs under protest (Halcyon morale −5) |
| **Pay when you can** (Rudi's way) | One tin now, one when you come back. The unpaid lines are read out and marked paid | Someone who heard her (`cook.heard`), or the cook there in person. Talk the boss round the night before (Red Hand 3; 2 under `book.debts_dead`) |
| **The first pan to the houses** | Through her sister's Haven house, for nothing. The boss writes that house a line | Known with the Haven |
| **The book first** | Anyone with a line in the book is dosed against it, and their line grows. A man who owes nothing can walk away | — |

The prices at the counter come from the epilogue's dose-outlet table, which reads these terms ([EPILOGUE.md](EPILOGUE.md)).

**Branches.** None is a dead end. She's never killed off-screen.
- **Taken by another route:** not lost. The route goes on with her held elsewhere. She can be taken back with a Bust Out, an escort, or by talking her out.
- **Dead: The Book Without Her.** The Red Hand sends her off loud. Then three extra steps:
  1. her tally: from her body, the tin, the book itself (with the boss's leave or after dark), or her sister reading her handwriting;
  2. read the cut (a deduction like Patient Zero's Ledger);
  3. a paid line who agrees to give blood.
  
  After that, a ★ Lab Tech, Doctor, Nurse or Mechanic player runs the still: twice as long, and a step lower.
- **Dead, and the tally burnt: The Bottles.** People keep the bottle that saved them, and she wrote the pan on every label. Who Came Back becomes required, with five paid lines, then two nights. The proof starts at 45%.
- **The still lost:** she moves it onto the Red Hand lorry in an empty town, or into a barn under No Vehicles.

**Remake step:** she "kept enough back for one more". One night on the still, if a tray is free.

### 6.4 Halcyon: The Lower Store

"Proof" is the Halcyon major's name, not this route's. **The route never re-decides the paper's first page: that belongs to the major.**

**Why it's Halcyon's route.** The lab's batch works on what's in the Zone now. It can't show that it works on what got out first, because it has nothing to compare it with. That missing thing is the **reference stock**:
- **What it is:** the first isolate, from the man brought into the clinic three weeks before the gates closed. Its canister is labelled with his surname and the date the Institute was told (`origin.vials`).
- **Where it is:** in a dewar in the closed site's **lower store**, behind a Bastion chain.
- **Day 1.** At 04:10 the cordon detail chained the door and pasted an order on it: *"By order: lower store to be secured and not opened. Day 1, 04:10. Lt H. Sedlak."* Two keys were cut, and both were signed for.
- **Day 2.** The detail went down to walk the column and never came back up.
- **Day 3.** Dr Wolff went up with her team for the rest of the samples (LORE), found the chain, sent her driver down and stayed with three of them. Her clipboard hangs by the lower store's door (not HC1's keypad store) and reads: *"Day 3. Chained. Soldiers gone. Four of us staying."* HC1's three bodies from her team and the driver's *"she stayed because she wanted to"* stay as built.

So the director has known all season where the stock is, and has never asked Bastion for the key: asking means saying what was in a store that was full before Day 1.

| | |
|---|---|
| Piece | The reference stock in the lab's freezers, and the comparison run |
| Door | Item 114, the key to Bastion's chain (Bastion). It's in the CO's safe 60% of the time, otherwise on an outpost commander's bootlace (`bastion.site_key`) |
| Other way | The second key, held by a sergeant (`origin.sergeant`: a Deserter wanderer, or none). Or cut the chain |
| Pitch | *"Published, fast, checked, with every author named."* |
| Canon | `origin.site.lower` (a second lockable building within 150 m of `origin.site`, else a steel hatch); `cure.cold.1` (the vet's farm, a Haven named person); `cure.cold.2` (a works with a bulk tank, or none) |

**Seeds:**
- the order on the chained door;
- Bastion's secure channel: *"Item one-one-four stays in the safe. CO's orders."*;
- the vet's invoice: *"Straws, 40. Tank topped up at the works."*;
- from 75%, *"Reference stock still wanted"* on the open channel.

**Act I. Reference: None** (open, +2)
- **When:** from the gate. About one session.
- **Goal:** what's missing, where it is, and who holds the way in.
- **Ways through:**
  - **Talk.** The duty officer (Halcyon 2, or Known as a ★ Lab Tech or Doctor) reads the overnight numbers, then says where.
  - **Persuade** the director's stand-in (Halcyon 3, or 2 with `weeks.record`): *"We've known where it was since Day 3."* You get the vial map (`ref.map`, personal).
  - **A field lead** (Independent or Rogue): it's Wolff's method, and the key is Bastion's.
  - **The sergeant**, at dusk only: *"We chained a door up there on Day 1. Two keys. I've still got mine. Nobody ever asked."*
  - **Listen.** A ★ Radio Op hears Bastion's secure channel, or works it out unseen from "Reference stock still wanted".
  - **Sneak** the records room on the night shift (+8 Halcyon Infamy if seen).
  - **Already known:** under The Whole of It (`proof.first_page` 2) the director has nothing left to hide, and the act is done the moment anyone asks.
- **Rivals:**
  - The grid sells to the boss's man for 30 Red Hand Favor (`ref.redhand_knows`), or to the adjutant (`ref.bastion_knows`).
  - With `ref.redhand_knows`, the CO posts a guard at the site, and a Red Hand crew may be there first.

**Act II. Item 114, and Something Cold** (sworn, +3: a way through the chain +1, cold +1, the right vials out +1)
- **When:** the key board can be reached only 20:30–20:40, while the call is read. The sergeant can be found only at dusk. A Bastion guard, if posted, changes at 05:30 and 17:30. A Red Hand crew sleeps through midday. Nothing spawns until a player is within 400 m.
- **The door: item 114.**
  - **Trade** in a pact with Bastion. Its condition goes into Halcyon's terms: the result is read on Bastion's call first.
  - **Bastion's terms**, through the adjutant (Bastion 2, or Known as a ★ Soldier or Police; Known under `authors.terms` 1–2 or Under Seal). The key is signed out to you by name, and Bastion's condition goes into the terms.
  - **Persuade** the CO (Trusted with Bastion): *"It's been signed for since Day 1. Sign it out."*
  - **The outpost commander**, if canon put the key on his bootlace. An Independent one hands it over after his own job; a Loyal one sends you to the CO.
  - **Bribe** the quartermaster: 40 Bastion Favor for 24 real hours. If it isn't back in time: +15 Bastion Infamy.
  - **Sneak** the key board, 20:30–20:40 (★ Police or Soldier: the journal shows the round). Seen: +10 Bastion Infamy.
  - **Trick:** a wax copy off the commander's bootlace (a look). Or a Rogue commander sells one (40 Red Hand Favor; it works once).
  - **The Red Hand's way:** a crew's ex-con picks the chain (30 Red Hand Favor; a ★ Ex-Con does it in 90 s).
  - **The Haven's way** (with `orders.haven`): the chair trades silence about the Day 3 page for the key.
- **The other way:**
  - **The sergeant's key**, at dusk. He gives it if you're Known with the Haven, below Trusted with Bastion, a ★ Soldier, or have shared food twice. He asks that nobody be told where he sleeps; walking him to a Bastion seat afterwards is a betrayal. Threaten him (Fear) and he hands it over and runs, and the Deserter's lines go cold for the season.
  - **Cut the chain:** a crowbar (3 minutes, loud, a horde), or explosives off the Red Hand's Trusted shelf (louder, and the dewar's neck cracks: 2 fewer vials fit). Either way Bastion's seal is broken: +10 Bastion Infamy, and Bastion's condition is off.
- **Bastion's trick** (a rival): hand over item 113, the wrong key. It doesn't fit the chain. It opens the outer door of Bastion's own cold store (§6.2).
- **The cold:**
  - **The vet's tank** (6 vials). She lends it to anyone Known with the Haven, for a promise: the first ampoules go to her house. Or bribe her (4 food, or 25 Haven Favor). Or sneak it from her barn at night (her dogs are lookouts; seen: +8 Haven Infamy, and her price doubles). Or requisition it (Known with Bastion; +5 Haven Infamy, and she remembers).
  - **The works' flask** (4 vials): 3 minutes at loud valves that bring a horde.
  - **The courier's case** (2 vials), always there, on a 30-minute clock (★ Nurse or Doctor: 40).
  - **Split the vet's tank** with another route: 3 vials and a bag, and the journal records the alliance.
- **The vials.** The dewar holds four canisters.
  - With the map, go straight to the right one. Otherwise work it out from the clinic's bracelet and the date (a ★ Lab Tech reads the labels). A wrong canister brings the infected and costs a vial.
  - Then hold at the dewar: a vial a minute (★ Lab Tech: every 40 s), up to what the container takes.
  - **Bribe:** the Bastion guard looks away for 25 Bastion Favor. A crew already up there leaves for 30 Red Hand Favor.
  - **Betray:** pocket a vial before the count. It goes wherever you take it: the still, Bastion's safe, or the chair.
- **Rivals:**
  - The Haven's draws want the same tank.
  - Anyone can drain a cold source (§8).
  - **Guard the door:** the CO keeps 114 on his person, or posts a guard at the site.

**Act III. The Index** (open, live tissue, +2; §7)
- Halcyon's own way: its tagged index. Halcyon knows where its tagged ones lie up, because *"the tissue gets better with age"*. A field lead lends the cage. Track Them Home (tribes design) finds the lair at dawn.
- The words: *"The index. Alive, please, and tagged before it wakes."*

**Act IV. Samples First** (sworn, last, +3: the stock in the lab's freezers +1, the comparison done +2)
- **The way down.** Within ten minutes of the container leaving the site, the Red Hand's open channel says: *"Something cold's coming down the hill. Lab stickers on it."* While it's outdoors, every rival route's locked players get a rough 500 m circle every 10 minutes.
  - **Drive** (★ Mechanic: it starts first time).
  - **Fly:** on Bastion's terms, or free with `pilot.flew_for` Halcyon.
  - **Walk by night** with heat under 35 the whole way.
  - **The Haven's road:** house to house, and each house will keep the container overnight in its cold.
  - **Bastion's escort** (Secure the Shipment).
  - **The crews' roads:** Known pays 20 Red Hand Favor, Trusted is waved through, or buy an escort for "a taste": one vial for the still.
  - **Decoys:** an empty case carried another way.
- **The family at dusk.** Near a safehouse, a mother with a bitten child flags you down.
  - *Samples first:* go on (+5 Haven Infamy if they're from a Haven house).
  - *People first:* walk them to the lab for the trial dose, and leave the container at the safehouse in the Haven's keeping (`ref.haven_holds`, +10 Haven Fame). The Haven gives it back for names on the paper.
- **Rival contracts on it:**
  - the Red Hand's Something Cold: take it to a fence;
  - Bastion's Signed In: stop it at a checkpoint and count it;
  - the Haven's Keep It Overnight: hide it until dawn and trade it back.
- **The comparison.** It takes a real day plus 4 hours of play, counted only while the lab has power. A dark lab pauses it and never resets it.
  - Each Power the Lab finished meanwhile takes a quarter off.
  - Each sample handed in after the gate takes an hour off the real-time part, never below half.
  - Without Wolff's method it takes twice as long. Her method can come from `weeks.record`, the field lead's copy (`weeks.public`), the CO's safe (`weeks.bastion`), or a sample from Wolff in the isolation room (`weeks.captured`).
  - It needs a tray in the lab. At the end, hold the lab for 4 minutes while the centrifuge draws the infected (Lab Defence's waves).

**Halcyon's terms: the order of the doses** (chosen at the release):

| Terms | Means |
|---|---|
| **The authors first** | Halcyon's own first: researchers, couriers and guards, and the two Bastion men detached at the lab since Day 5 |
| **The volunteers first** | The other two and the Day 6 cohort first, by name with `other_two.named` or `every_name.page_six_read`, else by number |
| **By the numbers** | The newly bitten first, because they're likeliest to walk out |
| **Published** | The method on the open channel at six. Home batches anywhere, at the trial's odds. The lab keeps no order at all |

**Second best** (the stock counts as lost only when no vial is left anywhere it could come back from). The earliest infection the Zone still has, tried in order:
1. **Dr Wolff**, if `weeks.captured` put her in the isolation room: a sample while the lab has power. A Haven player can refuse her to the lab: persuade the duty officer (Haven 2, or ★ Elder; `ref.wolff_spared`), and the route moves to the next source.
2. **Bastion's box:** one marked tube, by Bastion's terms, bought, or taken. It costs Bastion's route a tube.
3. **The long way:** three specials' samples from the site's town, then 3 real days plus 6 hours of play with the lab powered.

**Remake step:** half a comparison, with one vial less.

### 6.5 Independent: One Each

**Why it's nobody's route.** Everyone wants to hand the cure out. Nobody wants to hand it over.
- On Day 6, ten bitten Haven people walked up the hill for the first trial dose, and eight walked out (LORE).
- The lab has read its numbers on the open channel at every change of shift since, *"in case somebody outside is writing them down"*. Nobody outside is. Somebody inside was.
- Whatever the trial dose taught the blood of those eight, they've carried it longest. Nobody owns them.

**Canon `day6.cohort`** names the eight and where each is now:
- most at Haven houses;
- one attached on a Bastion roster;
- one in a Red Hand crew's book;
- one stayed at a field station and works there;
- one or two dead.

**The radio ham** (`ham.house`) is a retired schoolteacher with a long-wire aerial strung between two trees.
- Since Day 6 he has copied every number the lab read into school exercise books: *"In case. Somebody should."*
- His house is a house in an empty village, never within 2 km of the Long Table's farm. If there's none, the Courier wanderer carries the books.
- He's a named person and is never killed off-screen.

**The plan's pieces**

| What | Where it comes from |
|---|---|
| Its pitch | The Doctor wanderer, else the Courier, else an unsigned sheet at the lab gate |
| Its commit | The open channel |
| Its standing rule | Always open |
| Its assets | The open channel, "nobody" choices, letting people go |
| Its promises | Hand to Hand |
| Its trick | The Coat Run (§9) |
| Its terms | Chosen by its deciders after the stand |
| Its outlet | The ONE EACH crate |

| | |
|---|---|
| Piece | Three bags, each from one of the eight by their own yes, and a tray off the books |
| Doors | Whoever holds each person: a council head, Sedlak, a crew boss, a field lead |
| Other way | They walk out on their own. Or one unit from another route's subject stands in for one bag (at most one) |
| Pitch | *"For everyone, on nobody's terms."* |

**Act I. Eight Walked Out** (open, +2)
- **When:** from the gate, after the Doctor's line. About one session.
- **Goal:** who the eight are and where they are now. Three found alive end the act.
- **Ways through:**
  - **The exercise books.** The lab read the discharges by number with dates; a house medic matches them to faces.
  - **The lab's Day 6 ledger.** Ask the clerk (Known with Halcyon), sneak it on the night shift, or pay a courier 20 Halcyon Favor. It's the same ledger the Haven route reads.
  - **The Haven's houses.** Their medics remember who walked back down the hill.
  - **The Doctor wanderer:** *"I sat with two of them that night."*
  - **A ★ Radio Op** hears the ham at the change of shift, reading the lab's numbers back to himself.
  - **Bribe** the fence for the one in the book: 15 Red Hand Favor.
- **Rivals:**
  - Halcyon can close the ledger: the clerk refuses everyone below Trusted for a real day.
  - Bastion pulls its attached one back inside the wire for a real day.

**Act II. Asking** (sworn, +3: +1 per bag)
- **When:** each draw at dusk at the edge of that person's town. They're different people, so the draws can run side by side.
- **Goal:** three bags in the lab's cold store. The lab never refuses blood; the director logs it *"for everyone"*.
- **Their yes and their keeper's leave:**

| Where they are | Getting their yes and their leave |
|---|---|
| A Haven house | The council head lets them choose, by trait: Merciful yes; Loyal asks the chair; Paranoid wants Trusted; Independent wants a job first. Their own yes: Haven rank, difficulty 2. ★ Nurse +1. A Hand to Hand local from that house: one step easier |
| Bastion's roster, attached | Sedlak's leave (Known with Bastion). Or they walk out on their own at dusk, unseen, and Bastion goes looking for them as for one of its own. Whoever walked them takes +10 Bastion Infamy |
| A Red Hand crew's book | The crew boss owns their debt. Pay it (30 Red Hand Favor), talk him round (Trusted, or ★ Ex-Con), threaten him (Fear), or put the debt in your own name: *"A debt is a debt, but it can be yours."* |
| A field station | The field lead's leave. Halcyon wants her blood for the paper: give the lab half, and two draws are needed |

- **One unit instead of one bag** (at most one), each by its own person's yes:
  - **A draw from her**, "for everyone": difficulty 2 if you promised her house Hand to Hand, else 3. It's one of her five.
  - **R3's tube.** Bring R3 himself to the captain, who releases a tube to its own person with his slip: *"Collection is the person. He's the person."*
  - **One dose from the cook:** *"One tin. Same as anyone."* The Red Hand's case is one short.
  - **One vial**, through the chain as Halcyon does.
- **Someone to draw:** the Doctor wanderer, a house medic, a field lead, or a ★ Nurse, Doctor or Lab Tech player.
- **The carry** (a cool bag, 15 minutes), on air or quiet:
  - **On air.** The Doctor reads it on the open channel: *"Two cold bags, the {road} road, now. Nobody's."* Everyone knows, and the duty officer takes it without a vouch.
  - **Quiet.** No broadcast, but the duty officer needs a vouch from a player Known with Halcyon, or the Doctor in person.
- **Fallback** if fewer than three are alive or willing: Twice or the Scavenger (by consent), or donors at one Haven medic. Never a dead end.
- **Rivals:**
  - Any route can ask the same person. Each gives one yes.
  - They ambush the run.
  - Bastion reels its attached one back in.

**Act III. Nobody's Index** (open, live tissue, +2; §7)
- The Independent's own way: it has no cage. Borrow Live Capture's case from a field lead (Known with Halcyon), buy one off the fence with a stolen tag frequency (30 Red Hand Favor), or build one from a farm's cattle trailer (★ Mechanic).
- The words: *"Somebody's, once. Bring them in."*

**Act IV. For Everyone** (sworn, last, +3: a tray off the books +1, the batch sheet +2)
- **When:** at a change of shift, after Acts II and III.
- **A tray off the books.** The lab won't run a batch under nobody's name on one of its own trays.
  - **Persuade** a field lead (Independent or Rogue: some *"would rather give it away"*; `authors.open` helps). Halcyon 2, or ★ Lab Tech.
  - **Bribe** the duty officer: 30 Halcyon Favor, and the tray "spoiled".
  - **The sit-in.** At least two people sit unarmed at the lab gate through the 17:30 change of shift for 10 minutes; a shot within 100 m breaks it. The duty officer sets a tray aside *"for the people at the gate"*. Each promised Hand to Hand local present takes a minute off.
  - **Trade** with the Red Hand route: buy a tray at the still. It's a deal (§8), and the Red Hand's condition goes into the terms.
  - **Steal:** Steal the Antidotes' 90 s hold at the store room. Halcyon Infamy as built, and Lab Defence follows.
- **The batch sheet: "Released to: —".**
  - **Persuade** the director (Halcyon 3). One step easier under The Whole of It (*"Everyone. Fine."*), with an Idealist or Merciful director, and for each local standing at the gate (two at most). It counts who's there, promised or not, never promises.
  - **A field lead** runs it on the night shift without the director. She's replaced afterwards: the route's one leader change.
  - **Sneak** the batch sheet out of the records room at night and fill it in yourself (★ Lab Tech reads the filing codes). Seen: +8 Halcyon Infamy.
  - **Bribe** the duty officer: 40 Halcyon Favor.
- **Rivals:** Halcyon refuses a second tray to anyone with one out. A rival can rob the tray if it leaves the lab.

**The Independent's terms** (chosen at the release, if it controls):

| Terms | Means |
|---|---|
| **Open** | The ONE EACH crate at the queue: free, once per identity, first come first served |
| **One side, with a condition** | It goes to one faction's outlet, plus one condition: *all of them, exiles too* (the Haven); *no names taken* (Bastion); *one tin, not two* (the Red Hand, price capped at 15 Favor); *a home batch at every house* (Halcyon, the method free) |
| **Sold** | The same crate with a price chalked on it: 30 Favor of any one faction, the buyer's choice. The payment goes nowhere |
| **Kept back** | A padlock that isn't the lab's. Only the built stock: *"BACK SOON."* |

At the release, the deciders see how many locals they promised Hand to Hand.

**Remake step:** one more bag from the eight, by consent.

---

## 7. Live tissue (Act III of every route)

This comes from the tribes design's season table, with the author's change: tribes are out by day too, in small day parties ([INFECTED.md](INFECTED.md#day-parties)). INFECTED.md owns the tribes' side; this section owns the order of specimens and the acts. Every route has one live-tissue act, given by its own giver and built on the tribes design's Capture the Index. It's open work. **Player text never says "alpha", "tribe" or "crawler"**: each route has its own words.

**Best first.** The stage asks for the best specimen in reach (a lair or a current place within 5 km of the lab by road):

| Order | Specimen | Where | Counts |
|---|---|---|---|
| 1 | A leader, Prime or older | At its lair by day, past the scouts, with part of the tribe out in day parties; or on its trail at night, 100–150 m behind the hunters | +2 at once. A great one: +3 (still never past 9 before Act IV) |
| 2 | Else a lone one | Alone on a ridge | +2 after 12 real hours of the lab's work on it |
| 3 | Else two guards | Round the leader at its lair (the guard stays there by day), or on its trail at night | +2 after 12 real hours |
| 4 | Else two specials from Live Capture | When tribes are off, none is in reach, or every tribe in reach gave tissue in the last 6 season days | +2 after a real day |
| 5 | Else two special samples handed in, plus a real day's wait | When the special prefabs are empty too | +2 after the wait |

**How a capture works** (Live Capture's rules, built):
- Wound it to a third of its health, get within 3 m and tie it, then hold it for 30 s while it fights and its guard comes.
- Carry the case to the lab or an annex before it wakes: **Contracts > Specimen wake minutes**, 12.
- The wake clock resets once at a Halcyon field station, and twice under No Vehicles. There's no clock by helicopter.

**The rules**
- **Scarce on purpose.** The same tribe never gives tissue twice within 6 season days. On a one-tribe map the first route takes the leader, and the next takes a lone one, guards or specials. Being second costs only time.
- **No hoarding.** A route whose tissue is in gets no more capture stages.
- **Theft.** The case can be taken from its carrier, like a robbed truck. Whoever hands it in credits a route: a locked player credits their own; an unsided deliverer chooses the route at the hand-in and is paid by it.
- **Fury.** Each leader caught means a succession and a Furious tribe. For 2 counted nights its trails run along the capture road toward the lab, and by day its day party walks that road, inside the day caps. If it's still Furious at the stand, it comes with the last wave. Only the most recently Furious tribe comes.
- **Day parties** also cross carries by day, so no road through the wild is safe at any hour. A day party is hunters and young in twos to fours, at most a quarter of the tribe, never more than 6 bodies near one player cluster, inside its range. By day the tribe never comes in full: a scout's day call brings at most 4, never the leader. The built stealth judges whether they see a carrier.
- **Until tribes are built,** every route takes two specials.

**Each route's own way**

| Route | Its own way |
|---|---|
| The Haven | Name who it was. Its house asks for him alive and lends a second wake reset |
| Bastion | A squad with a cage, and the contact board |
| The Red Hand | The moat's leader, sold by its crew. The moat opens |
| Halcyon | Its own tagged index, a lent cage, and Track Them Home |
| Independent | A borrowed, bought or built cage |

---

## 8. What routes do to each other

### Fighting routes

Two routes are **fighting** while either has done a hostile route act against the other in the last 2 real days (**Fight memory days**). The hostile acts are:
- took or spoiled its route item;
- killed one of its locked players at a route place or on a carry;
- drained its cold;
- sold its note;
- ran a Last Mile contract on it;
- took custody of its subject.

During the stand, every two sides are fighting. Fighting closes parley and settling up with the other route's faction (the plan's rule) and sets the stand's hostility bits (§9). Nothing else reads it.

### Pacts and deals

**A pact** is an agreement between two routes:
- **Proposed** by a locked player at their own stronghold leader's route topic (the Independent: on the open channel). It names the other route, says what each side gives, and always includes **standing together** at the stand.
- **Agreed** by a tally of each route's active locks, open for one real day. It needs a majority of the votes cast on each side, and at least one vote from each. On a small server one player per route is enough. The tally is `RZ_QuestVote` once the majors build it; until then, the controller keeps a simple count.
- **One pact per route.** Bastion and the Red Hand can't make a pact while a feud from the majors stands (Expended, Bastion's file).
- **Ending a pact** by a tally of either route before the stand costs nothing. **Breaking one** is a betrayal (below).
- **What it does:** parley and settling up stay open, looks are normal, a door it names opens by trade with no check, and at the stand the two routes are one side. Their groups leave each other alone.

**Joining at the gate (the deal).** Once a season, during the batch hold, a route at 7 or more in a pact with the finisher can join it.
- One locked player from each route agrees at the lab gate.
- Both routes hold the clean room together, and the partner adds one **condition** to the terms (the list in §6.5).
- No Fame moves: the chunk is the finisher's.
- The partner's rules apply at half next season (§13).

### The moves

All of these are sworn work, all cost something, and all are capped.

| Move | What it does | Cost and limit |
|---|---|---|
| **Ambush a carry** | Route items drop when their carrier dies (the robbed-truck rule). Give it back (10 Favor with its faction: "found it"), keep it (it goes back to its seat after 2 hours of play with nobody near, or a real day), spoil it, or ransom it | Ordinary Infamy for the fight. Spoiling one of her bags: +10 Haven Infamy (*"That was her blood."*), and she has to give again |
| **Drain** | 20 s at a cold container on the ground, or at a source: its contents spoil, and that step is done again. A source drained is gone for the season. Cool bags and couriers' cases are always there | Once per container or source. Seen: +5 Infamy with its owner for a container, +15 for a source |
| **Spoil the case** | Tell a subject a true thing about a rival route: her about the List, the back room (HC2 done, or `walk.knows_other_two`) or "expended"; the cook about who'll sell her doses; the captain about who turned the column back; the CO about what was in the lower store before Day 1 (`weeks.public` or `weeks.bastion`) | That rival's persuade with that subject is one step harder for the act. Once per subject per route |
| **Guard the door** | The door's own route moves its door once: the clipboard into the chair's office, 114 onto the CO, the trays into the night cold store with the night shift doubled, her boy's line into the boss's own hand | Sneak and trick ways one step harder, and talk needs a pact. Once per door. The other way stays |
| **Custody** | A group prefab walks a subject to an outpost, a crew camp or a field station, never a stronghold or the lab | A betrayal: +20 Infamy with the subject's faction for everyone who took part. The subject gives that holder nothing. It buys only time, and a hostage for a deal |
| **Sell the plan** | A route's own Rogue seat leader sells its stand plan to a rival: Take the Lab's timing (the bay hold becomes 150 s) or the Coat Run's night (the look one step harder). This is the Red Hand's "crews eat crews" | Once per route a season |
| **Break a pact** | Use a pact's door against the partner, keep what you were given without giving, or turn on the partner at the stand | +15 Infamy with the partner's faction for every player of the breaking route who took part. A war deed, the word "turncoat" for the season, and the partner's stronghold leader remembers. The pact ends |
| **Hand it over** | A route item put down by one route's player and picked up by a rival's within 10 s, with no damage done, is logged as "handed over" | +15 Infamy with the giver's own route faction (none for an Independent giver). The journal shows each item's chain of carriers to its route's players |
| **Last Mile** | When a route reaches 8, the open channels say *"One of them is close."* Its moving items are announced with a rough area every 10 minutes (Smuggler's Run's reports), and every other route's locked players get contracts against it: Take It, Drain It, Search It (Bastion), Keep It Overnight (the Haven), Buy It (the Red Hand), Sign It In (Halcyon), Nobody's (Independent: hand it to the lab's cold store "for everyone", where it counts for nobody) | As the contracts |

**Notes** are personal knowledge: a name, a house, a grid, a key's place.
- Any note can be written into a carried note item.
- Any faction's leader buys a note for 15 of their Favor. A false note sells the same way.
- Lies are found out by events first: a visit, road news, or the owning system's tick. Then the buyer's route loses what it learnt, and that leader remembers you crossed them.

### Kills between route players

- **Outside the stand,** killing a player locked to a faction route costs +3 Infamy with that faction, at most 12 an hour. Killing an Independent-locked player costs +3 with the Haven (*"The houses feed whoever is out on the open road."*). Each is tagged *war*, so the ending can tell a war from a crime.
- **At the stand,** within 150 m of the lab, fire between locked players of different sides costs nothing (war).
- **Nothing new is paid for these kills.** There's no Fame for killing route players, and the built bounties are unchanged.

**Never.** No move touches a stronghold leader, a stronghold, the lab, or a subject's consent. Subjects are named people with bodies only near players, and they're never killed off-screen. Killing one on-screen costs +20 Infamy with their faction and moves that route to its branch.

---

## 9. The finish, the batch and the stand

### The finish

When a route's last step lands, its track reaches 10 and `RZ_CureFinale.Finish(route)` runs:
- **Fame:** the route's Fame chunk is paid to its faction (§10). The Independent's is held until the release.
- **The holder:** the route becomes the holder.
- **The race ends:** the other tracks freeze. Their running stage contracts and route errands are called off, with no pay and no penalty. The race doesn't reopen this season.
- **The batch hold starts:** 12 real hours **and** 2 hours of play, both. Everyone gets a session to rally.

### The batch hold

The lab's safe zone holds: nobody fights at the lab. During the hold:
- players can still lock, and count at this stand;
- a deal at the gate can be made (§8);
- the Coat Run can be tried (below).

### The seal

When the hold ends, the batch is ready.
- Every channel says: *"The batch seals in an hour."* It says so again at 30 and at 10 minutes.
- At the hour (with someone online), research goes to 100 and the stand starts. As built, the stand's clock waits for the first player to reach the lab.
- An admin can set **Finale > Seal hour** instead: the seal comes at the first such server hour after the batch is ready, with the same calls.

### Sides and the clean room

Each route present is a side, and a pact is one side. Unlocked players are bystanders: they can fight the infected, they're never targets of route groups, and they can't hold the room.

| Rule | |
|---|---|
| **The clean room** | A 10 m radius round the lab prefab's clean room point. VERIFY a named child entity. The fallback is the point built Take the Lab uses: a land point 15–25 m from the lab's position |
| **Who counts** | Active-locked players who locked before the batch was ready, who are online, alive and conscious, and inside the room. VERIFY reading unconsciousness; the fallback is "alive". Unlocked players and AI never count |
| **The starting holder** | The finisher, with its partner after a deal. After a passed Coat Run, the Independent. After the slow finish, nobody |
| **Taking it** | **Force** (player damage on): a side that is the only side inside counts up. **Tally** (player damage off): a side with more counted players inside than every other side counts up. When its count reaches its take time, it becomes the holder. The count falls back 1 s for every second it isn't counting up |
| **The take time** | 90 s, minus 5 s per point on the side's frozen track (a pact uses the higher track), never under 45 s. The finisher retakes in 45 s |
| **Holding it** | The holder doesn't have to be inside. It only has to stop anyone else's count reaching their take time |
| **The end** | At 15:00 the holder controls the cure. If a challenger's count is above zero, **overtime** runs until the room is taken or the count falls to zero, 3 minutes at most. The holder wins every tie |
| **Nobody holds** | Only when there was no starting holder and nobody ever took the room: handed over (§11) |
| **A restart** | The holder and the counts are saved. The stand restarts from 15 minutes once a player reaches the lab (built) |

### Take the Lab (the Red Hand route, once a season)

- **Who:** at the stand, any Red Hand-locked player who is Trusted with the Red Hand can start it (the built contract needs Champion; under a finale it needs Trusted). Every Red Hand-locked player counts for the bay hold, under the stand's usual rule: an active lock made before the batch was ready. It's offered only when the Red Hand's last page allows it (§12); without that major, it's always offered. It leaves the random contract pool while a finale is registered.
- **Betrayal:** if the Red Hand route is in a pact with the holder, using it is a betrayal.
- **Where:** the crew's lorry backs up to the **loading bay**, a second point 25–40 m from the clean room on the side of the lab away from its gate. The fallback is a land point 25–40 m from the lab.
- **The hold:** 120 s at the bay with no player of the holder's side within 15 m. While the bay is contested, the count pauses.
  - With `authors.batch_sold` (the night guard was paid): 90 s.
  - If a rival bought the plan: 150 s.
- **Done:** the Red Hand route is the holder at once. Everyone else, the old holder included, takes the room back the usual way.
- **Without a finale registered,** built Take the Lab is unchanged (`StealBatch`).

### The Coat Run (Independent, once a season)

- **When:** during another route's batch hold, at a change of shift (05:30 or 17:30).
- **What:** an Independent-locked player in a clean Halcyon coat walks past the duty officer into the clean room and re-labels the sealed batch *"Released to: —"*. The batch never leaves the lab: it spoils out of the cold, as Cold Chain shows.
- **The look** (the disguise design): difficulty 3, with the lock's −1. It takes Halcyon's full kit (the coat, and a hood or mask with a respirator: the beaks), a real notebook number and the night, or a ★ Lab Tech. Each holder-route player standing at the clean room door who calls them out makes it one step harder (two at most).
- **Passed:** the Independent is the starting holder. The batch's route finds out at the seal call, from the duty officer: *"The seal's been changed. It says 'nobody'. Witnessed: me."* The finisher keeps its Fame.
- **Caught:** the built catch (+3 Halcyon Infamy, the coat taken, one of three catches a life). The batch's route is told at once, and the Coat Run is spent.
- **At the stand** no disguise works near the lab: during the stand, within 1.5 km of the lab, `RZ_Disguise` clears no faction bits (one `IsFinalStand` check, like stealth's `FINAL_STAND_RADIUS`).

### The infected and the groups

- **The waves are as built:** a horde every 2.5 minutes, plus one per 6 players at the lab (3 at most). The tribes design draws them from nearby crawlers and the nearest tribe, within 70–130% of today's waves. A Furious tribe comes with the last wave.
- **Stealth** never lowers anything within 1.5 km of the lab during the stand (built).
- **The abandon rule stays:** if nobody is within 150 m for 90 s, the batch is lost. A rival standing at the lab prevents it.
- **Groups** come as the majors set them (§12), **at most two friendly groups** in all, 40–120 m out. If more qualify, the finisher's faction goes first, then by active locks at the lab.
  - Groups never enter or count for the room.
  - A group fights the infected, and the players of sides its faction's route is fighting. That uses the built per-player hostility: during the stand, within 1.5 km of the lab, a player locked to a rival side carries that faction's bit. VERIFY that spawned groups honour the bits; the fallback is groups that fight only the infected.
  - A faction whose route has no active lock sends a group that fights only the infected.
- **Halcyon never takes sides.** Its bit is never set, and its guard fights only the infected. The lab and its staff are never targets.
- **The lab's safe-zone rule** (2 Infamy and the guards' fire within 80 m) doesn't apply between locked players during the stand. It still applies to unlocked shooters and to fire at Halcyon's people. Killing an unlocked player at the stand costs +10 Halcyon Infamy.
- **Shooting your own side:** you don't count as a holder or decider at that stand, and you take +10 Infamy with your route's faction.

### PvE servers

**Finale > Contest** is *Auto*: Force where player damage is on, Tally where it's off. VERIFY that the server's setting can be read; if it can't, the admin sets it. In Tally, groups never target players.

### What everyone sees

Within 150 m of the lab, a line under the noise meter: *"CLEAN ROOM: Bastion. The Haven taking: 0:35 of 0:60."* It names routes, never players. UI.md and UI/LAYOUTS.md own it.

### How it was won (`finale.how`)

The first row that fits is used:

| How | When |
|---|---|
| **theft** | The controller got the room by Take the Lab or the Coat Run, and kept it |
| **betrayal** | The controller was the holder's pact partner and took the room from it, or broke the pact at this stand |
| **deal** | Two routes in a pact hold it together at the end |
| **force** | The controller took the room from another side, or held off a challenge whose count reached 30 s |
| **held** | Nobody else came for it |
| **handed over** | Nobody held it (§11) |

### The release: terms and deciders

- **The cure goes out** the moment the stand is won. As built, infection falls 70% everywhere and the line stands down.
- **The terms.** The duty officer brings the release form to the clean room. The controller's locked players within 30 m have 3 minutes to choose the terms from their route's list (§6). The acts they did open or close options.
- **The decide scene.** It uses the missions design's hold: 5 seconds, one objection per player, and Stand with. Until that's built: the first locked player's pick, with a 10-second window for one objection.
- **A deal splits the choice:** the finisher chooses the terms, and the partner adds its condition.
- **Nobody chooses:** the stronghold leader's traits pick. The Independent's default is open.
- **The deciders** are up to four of the players who chose, longest in the clean room first (in a deal, two from each route). They're recorded by identity, with names frozen at the deed.

### The stand's Fame

This replaces the built "Halcyon +50 and the Haven +30 for each player at the lab". It's paid after the winner and the record (as built), so it can't decide anything.

| Who | Gets |
|---|---|
| The controller's players who were at the lab for half the stand, or in the clean room for 60 s | +30 Fame with the route's faction (Independent: with the Haven) |
| Everyone at the lab for half the stand who never fired on the final holders, bystanders included | +20 Halcyon Fame: the lab never fell |
| Rival sides' players | Nothing from the stand. They keep their route pay |

All of it counts toward the 50-per-faction finale cap (§13). The hall of fame's "Held the final stand" lists the controller's holders and the bystanders. Rivals count only as a number on the stand board.

---

## 10. Fame: the capped chunk

**What it is.** One payment, B, added to a faction's **season Fame** (a new `AddFactionBonus` into `m_aSeasonFame`). It never goes into any player's standing. It's paid at the finish, once a season, and it's never taken back or paid again: a lost stand and a remake pay nothing more.

**Its size is fixed at the gate,** from a snapshot, so finale work can't inflate it:
- B = **Route worth** (0.25) × the leading faction's season Fame when research first reached 90.
- Never less than **Floor** (150). Never more than **Ceiling** (0 = none).
- The finale page shows it from the gate.

**The soft line.** At payment, B counts in full until the receiving faction reaches **Soft line** (1.10) × the best other faction's season Fame. Anything above the line counts half.

**The Independent.** It finishes for nobody, so B goes to no faction. If it controls and chooses *one side*, that faction gets B/2 at the release, checked against its own line. Under any other terms, or if it doesn't control, nobody gets it.

**Deals** don't split B: the chunk is the finisher's.

**Typical numbers** (a model, not measured; calibrate from the status log and `last_season.json`):
- Season Fame counts only Fame actually gained, after each player's 0–100 clamp: about 9 per player-hour.
- **5 regulars:** about 4,400 by the gate on day 39, typically 1,500 / 1,200 / 1,000 / 700. So B ≈ 375, about 9 days of the leader's own Fame.
- **10 regulars:** about 8,000, so B ≈ 675.
- **2 regulars:** about 1,800 in all, so B is the floor, 150.
- A route's own stage pay is small by comparison: about 6 players × 30 Fame.

**Worked examples** (5 regulars; at the gate the Haven has 1,500, Bastion 1,200, Halcyon 1,000 and the Red Hand 700, so B = 375):
1. **Bastion finishes** on 1,260, with the Haven on 1,560. The line is 1,716. 1,260 + 375 = 1,635 is under it, so all of B counts: Bastion leads season Fame. At the stand the Haven takes the room. **The Haven controls the cure (force); Bastion leads the season's Fame.** Both are true, and both show.
2. **The Haven finishes while leading**, on 1,560 against Bastion's 1,260. The line is 1,386 and the Haven is already past it, so B counts half: 1,560 + 188 = 1,748.
3. **The Independent finishes and controls,** and chooses one side: Bastion. Bastion gets 188, checked against its own line.

**What decides what now:**

| Control decides | Season Fame decides |
|---|---|
| Who has the cure, its terms and aftermath, the controller's next-season rules, and the epilogue's route | The hall of fame's season Fame line (*"The cure: the Haven, by force. The Zone's Fame: Bastion, 1,640."*); the faction when nobody holds the room ("handed over"); and the Fame leader's next-season rules at half, when it isn't the controller and there was no deal (§13) |

**Settings:** **Finale > Route worth** (0.25), **Floor** (150), **Ceiling** (0), **Soft line** (1.10). The log line: *"Finale: Bastion's route paid 375 season Fame (375 full, 0 half). Season Fame: ..."*

---

## 11. Fallbacks

| Case | What happens |
|---|---|
| **The slow finish** (no route finishes) | The built fallback keeps its timing. After a real week at the gate (÷ season speed) it adds 1% a real day (**Finale fallback**, × season speed). While a finale is registered, that goes to the **lab-alone track**, which the readout reads (*"The lab's own: four."*). At 10, the lab makes the batch on its own work, with the same hold and seal. Nobody starts as holder, and locked routes can still take the room. No route Fame is paid. It sets `cure.slow`. It comes about 17 days after the gate (about day 56 at 5 regulars) |
| **No route at all** (nobody locked, or no locked player takes the room) | The same, and nobody holds the room: the cure is **handed over** to the faction with the most season Fame. That's today's rule, kept only as the fallback (`finale.route` 0) |
| **A lost stand** (nobody at the lab for 90 s) | As built: the batch is ruined and research drops to 90. The lab never falls. The finisher stays finished: its Fame was paid once, and is neither taken back nor paid again. Locks and pacts stand. The next batch comes from the finisher's **remake step** (§6), or the lab's backlog (built: the gate stays open, 3% a real day, so samples matter again), whichever is first. Then the seal, and the finisher starts as holder again. With no finisher and an empty backlog, the lab-alone track adds 1% a real day from the next real day. Each lost stand is recorded by day. Take the Lab and the Coat Run don't come back |
| **A route's subject is lost** | Each route has a branch, never a dead end: What She Left and Twice; Processing Again; The Book Without Her and The Bottles; Second best; the eight's fallback |
| **No tribes** | Two specials (§7) |
| **Small servers** | One player can run a route alone: every act has a solo way. In Tally mode a lone holder keeps a room nobody contests |
| **A route with no active lock** for 3 real days | Its stage contracts pause, and its claims (her house, the still, the store) are let go, so the subjects stay free for other routes. They're claimed again when a lock is active |
| **Prefabs missing** | Every approach that needs one (the annex, the still, the group prefabs, the cage) is offered only when it's set. Under No Vehicles: carried cases, foot runs, an extra wake reset, no truck cab |
| **The wipe** | Only after the cure and its epilogue. *On cure* stays the only default; timed wipes run only if an admin picks them, and then the majors' `ResolveAtSeasonEnd` and the epilogue's last-day rules apply |

---

## 12. How the majors feed it

The majors ([MAJORS.md](MAJORS.md)) finish before the gate. Their flags change the finale by one step, a price, a door or a branch, never more. Each page in the open (`majors.pages_open`) makes one finale check one step easier, once. **The finale owns the stand's rules, its Fame and the winner.**

| Major result | What changes in the finale |
|---|---|
| **Every Name** (`list.read`) | Her name was read with the rest: the Haven's Act I gets it free, and "say my name" is met |
| **The Closed Book** (`list.buried`) | Her name stays the Haven's: other routes need the records to find it |
| **The Far House** (`haven.rule` 2) | She was sent to the far house as a mouth. Her house is known, her house sells tonight's spot more readily, the Haven's case is one step harder, and *the far house first* is on the table |
| **Counted** (`haven.rule` 3) | Bastion's door is open: the census stands in for the slips |
| **Paid at the Door** (`haven.rule` 4) | Her boy's line costs half |
| `proof.haven_gate` | The Haven's group stands at the lab gate at the stand |
| **The Call Stands** (stance 1) | The release as written, and 114 stays in the CO's safe. Bastion's group at the lab gate |
| **All Stations** (stance 2) | *By need* opens at Known. The captain opens one step easier. A Haven–Bastion pact needs no day-long tally (one vote each, at once) |
| **Bastion, Command** (stance 3) | The CO is Command: the release is one step easier, the captain obeys, and no tube spoils. *The census* opens. A Bastion checkpoint stands on the lab's main road during the stand: a rival's carry through it is searched (a look, or a 90 s wait while its clock runs), and other roads stay open |
| **Expended** (stance 4) | The captain stops answering the CO: the release is one step harder, and he opens only to the column. A Rogue commander sells 114. Bastion sends nobody to the stand |
| **The book balances: bill paid** (`book.last` 1) | The lab trusts the boss: the Red Hand's tray trade is one step easier. Take the Lab isn't offered |
| **Stood for nothing** (`book.last` 2) | The Red Hand's group holds the lab road at the stand. Take the Lab isn't offered |
| **Sold by the page** (`book.last` 3) | The Haven bought its line back: her boy's line is the Haven's to strike. Take the Lab isn't offered |
| **Sent off loud** (`book.last` 4, `book.debts_dead`) | Every debt died with the book. Her boy is free (the door is open), and the cook will cook for any route that brings a tray. *Pay when you can* needs Red Hand 2. Take the Lab isn't offered |
| **Bastion's file** (`book.last` 5) | Bastion holds the book: it reads the paid lines (Who Came Back), and can stop the cook on the road at Known. Take the Lab is offered. No Bastion–Red Hand pact |
| **The boss takes** (`book.last` 6), or **the book doesn't balance** | Take the Lab is offered. A Rogue crew boss is likelier, so Sell the Plan is likelier |
| **The Whole of It** (`proof.first_page` 2) | The director has nothing left to hide. Halcyon's Act I is done when asked, the Independent's batch sheet is one step easier, and the director lends the transport case to any route that puts names on the paper |
| **Under Seal** (`proof.first_page` 4), `authors.terms` 1–2 | 114 at Known with Bastion. A Bastion–Halcyon pact starts made, and either side may end it by tally |
| **Day 1** (`proof.first_page` 1) | No change. The stock comes only through the route's own acts |
| **Two Tins a Copy** (`proof.redhand`) | The Red Hand knows where the lower store is, and the tray becomes a trade for the director's pages |
| `authors.batch_sold` | Take the Lab's bay hold is 90 s |
| `authors.bastion_key` | Bastion holds a key to the clean room: the Red Hand's tray by a deal with Bastion |
| HC1 (`weeks.*`) | Wolff's method, her fate, and who holds her notebook: Halcyon's comparison and second best |

**Changes to the majors' text** (planned, so changed in the same revision):
- The Red Hand's Act 6 and Proof's Act V both say the stand's rules and the winner don't change. That becomes: the finale owns the stand.
- Under a finale, Take the Lab makes the Red Hand route the holder instead of losing the batch.
- Proof's Act V: the winner is no longer chosen on Fame. Halcyon's Act V and the stand are one event.
- HC3's ending reads the finale:
  - **Under Bastion's guard:** Bastion's group stood at the gate, and Bastion's side held the room or was the holder's partner.
  - **Open doors:** the Haven or the Independent controls.
  - **The Red Hand's batch:** the Red Hand took it by theft.
  - **The lab alone:** any other result.
- HC3's Take the Lab offer goes to the Red Hand route through the controller.
- The codas stay the majors'. The finale adds only clauses: the captain and the four R numbers at Bastion's walk, and her name at the Haven's Gate if she was found.

---

## 13. Endings

### The losing routes

- **Pay:** a losing route keeps everything it was paid. There's no penalty for losing.
- **Codas:** its subject gets its coda whoever won.
- **Channels:** shared channels never gloat. Winners' lines name factions, never players, and never mock a loser.
- **The stand board** counts the routes that came for it (*"CAME FOR IT: {n}"*). It never names them.
- **Marks:** losing routes' marks come down at the first dusk of the epilogue, taken down by their own people.
- **The contest ends at the cure.** Locks end. Every grudge and hunter squad the contest started stands down once. Parley reopens with everyone: *"It's over. Talk."* Fame and Infamy stay.

**The finale's lines** (the epilogue owns the carriers):

| Route | Controlled | Finished, lost the room | Never finished |
|---|---|---|---|
| The Haven | Every evening on the Haven's channel: *"If you can hear this: her name is {unturned_name}. She was bitten on Day 4, and she's still here. Bring water if you can."* By terms: *"The first box went to her house. Her boy carried it."* / *"Somebody finally drove out to the far house."* / *"The houses read the bite book, not the List."* / *"They went down the List from the bottom."* | *"She gave three times. It went out on somebody else's terms. We read her name anyway."* | *"She was never needed. The Haven uncrossed her name anyway."* |
| Bastion | The receipt call. By terms: the roster at the table / the column's slips first / *"The sick first. Whoever."* / names taken at the queue | At the last call: *"Stores held for collection: signed for. Not ours."* | The call keeps its last line to the end: *"Stores held for collection, {held_count}. Minus nineteen. Over."* |
| The Red Hand | By terms: *"Two tins a dose at the crossroads. Lab stickers extra."* / *"They read the book from the back. The last name was on the first line. Paid."* / *"The first pan went to a Haven house for nothing. The cook's sister stood at the door."* / *"Every dose went on somebody's line. The book has never been so full."* With `cook.line_paid`: *"The boss marked one more line paid. On the first page, again."* | *"Our cook's pan is in their batch. Write that down."* | The cook at supper, 18:30, as always: *"Soup. It keeps."* |
| Halcyon | By terms: the authors / the volunteers by name / the newly bitten / *"THE METHOD. READ IT. COPY IT. PASS IT ON."* The canister's label is the paper's first table, never its first page | *"The reference came down the hill. Somebody else wrote the label."* | The readout keeps saying *"Reference stock: wanted."* |
| Independent | The epilogue's four terms scenes: ONE EACH, one side, sold, the padlock | *"Eight walked out on Day 6. Three gave blood on Day {day}. Somebody else wrote the label."* Locals promised Hand to Hand: *"You said we'd be first. We weren't. You came anyway."* | Chalk on the lab gate: *"WE TRIED."* |

**Branch lines**
- Her death: *"The Haven crossed her name off on Day 4 to keep her alive. It wrote it back in on Day {day}."*
- She walked off: every Haven house puts a bowl out at night.
- The cook dead: *"They sent her off loud."*

### What the finale hands the epilogue

The finale reports at the cure, in this order on the cure's tick: the finale reports, the hall of fame freezes, the contest ends, the record is snapshotted, the beats are built. The snapshot waits for the release (at most 3 minutes plus the hold).

| Key | Values |
|---|---|
| `finale.route` (the controller) | 0 none, 1 the Haven, 2 Bastion, 3 the Red Hand, 4 Halcyon, 5 Independent (the faction + 1) |
| `finale.finisher` | The same values (0: the lab alone) |
| `finale.how` | 1 held, 2 force, 3 deal, 4 betrayal, 5 theft, 6 handed over |
| `finale.terms` | The controller's choice from its route's list. For the Independent: 1 open, 2 one side, 3 sold, 4 kept back |
| `finale.condition`, `finale.partner` | The partner's or the Independent's condition (a faction + 1), and the pact partner in a deal or a betrayal |
| Records | Up to 4 deciders with frozen names; the stand's holders with their seconds at the lab and in the room; stands lost (by day); batches taken (Take the Lab and the Coat Run, with the day and the route) |
| Per route | `route.<r>.state`: 0 not worked, 1 worked, 3 finished, 4 subject lost. The subjects' fates: `unturned.named`, `held.processed`, `cook.line_paid`, `ref.haven_holds`, `eight.bags` |
| Per identity | `RouteAtLock` |
| `finale.marks_public` | 0 |

The epilogue owns the carriers: route marks, the lab scene, the queue's table, the dose outlets, the leaders' "The cure" topic, the stand board, and the hall of fame's "The season" block. The cure line and the first-spawn news say the route, how it was won and the terms. **There are no slides.** Dose outlets are the epilogue's: given on the spot, only to the bitten, one per life. The terms pick only the outlet's variant: the queue's order, the price board's price, or the ONE EACH crate.

The epilogue lasts one week (168 real hours). It owns the closing work that lets every story, the routes' included, reach an ending inside that week, and it reads the keys above ([EPILOGUE.md](EPILOGUE.md)).

### Personal pay and caps

| Who | Gets |
|---|---|
| The decider of each act | That stage's contract pay: at most 10 Fame and 20 Favor with the route's faction. Independent acts pay 20 Haven Favor and no Fame |
| Unlocked helpers at a sworn act | Half the stage's Favor, no Fame |
| The controller's deciders | +15 Fame and 30 Favor with the route's faction |
| The controller's holders, and the stand's bystanders | The stand's Fame (§9) |
| Locked to a route that finished but lost the room | +10 Fame with the route's faction |
| Locked to a route that never finished | Nothing, and no penalty |
| Independent deciders who control | Open: +10 Fame with each of the four factions. One side: +15 with that faction. Sold or kept back: none, and +5 Infamy with every faction |
| Betrayals (custody, a diverted bag or vial, an item handed over, a broken pact) | +15 Infamy with the betrayed faction (custody of the woman: +20 Haven) |

**Caps**
- Over all finale content: 50 Fame per faction per player.
- Per player, per faction, per route ending: 40 Infamy and 60 Favor.
- A lie pays at most 60% and no Fame. Nobody is paid for standing nearby.

**The finale's own limits.** The finale is the one place allowed past the limits every questline keeps ([QUESTLINES.md](../docs/QUESTLINES.md#limits-every-questline-keeps)). Instead of 25 Fame per ending it has the caps above and the route Fame chunk (§10). Instead of research +3 it makes one `AddFinaleResearch` call, at the seal (§14). It's the one way a faction wins through a questline. No faction falls.

### World effects

One call each, through the finale's own lane in `RZ_MajorBudget`: morale ±10 and strength ±5 per faction, kept apart from the majors' ±20 and ±8, so the majors spending theirs never clips the finale. One truce or feud per pair.

| Controller and terms | World |
|---|---|
| The Haven, any terms | Haven morale +8 (after the reading's −5). Her house +2 people. `unturned.named`. A Haven–Halcyon truce if the annex was lent |
| Bastion: the roster or the census | Bastion morale +8, strength +3. Haven morale −4 |
| Bastion: the column or by need | Bastion morale +5, Haven morale +5. The captain is back on the roster |
| Bastion and the Haven in a deal | A Haven–Bastion truce for the epilogue |
| The Red Hand: the price or the book first | Red Hand morale +8, strength +3. Halcyon morale −5 |
| The Red Hand: pay when you can | Red Hand +5, Haven +5. A Haven–Red Hand truce is rolled |
| The Red Hand: houses first | Haven +8, Red Hand −5. The cook joins her sister's house |
| Halcyon: authors first | Halcyon +6, Haven −5 |
| Halcyon: volunteers first | Haven +5, Halcyon −3 |
| Halcyon: by the numbers | Halcyon +5 |
| Halcyon: published | Halcyon −5, Haven +5. Infection −3 in every safehouse town |
| Independent: open | Every faction −3 (*"nobody's"*) |
| Independent: one side | That faction +5 |
| Independent: sold | Red Hand +3: *"Somebody's undercutting us with our own business model. I'm nearly proud."* |
| Independent: kept back | Every faction −5 |
| A finisher that lost the room | Its faction's morale −3 at the stand, recovering |

### Next season

- **The controller's rules** come from the built winner table, in full.
- **Independent, by its terms** (in place of a controller's rules):
  - open: half the Haven's step and half Halcyon's (one more safe town, starting infection ×0.9);
  - one side: that faction's rules at half;
  - sold: only the Red Hand's black-market rule;
  - kept back: nothing.
- **At most one half rule set on top:**
  - **after a deal,** the partner's rules at half;
  - **otherwise the Fame leader:** when the season's Fame leader isn't the controller (or, under the Independent's *one side*, isn't that side), its rules apply at half (**Finale > Fame leader rule**, on).
- **Then the majors' story rules.**
- Everything is merged through `RZ_SeasonMods.Add` and `Cap`, within the built caps, so the whole stays LORE's small, capped nudge.

---

## 14. Engine needs

### Built hooks it uses

| Hook | Use |
|---|---|
| `IsAtFinaleGate`, `HasFinale`, through `RZ_Questline.IsCureFinale` and `RZ_Questlines.HasCureFinale` | The gate holds while the five routes are registered |
| `AddFinaleResearch(amount, faction, reason)` | Still the only way past 90. The controller calls it **once, at the seal**, for what's left to 100, credited to the finisher's faction (−1 for the Independent or the lab alone). One call means one notice, and research never sits above 90 while still counting as "at the gate" |
| `Fallback`, **Finale fallback**, the week's grace | Become the lab-alone track |
| `StartFinalStand`, `TickFinalStand`, `LoseBatch`, `CompleteCure`, the abandon rule, `m_bStandReached`, `LAB_DEFENCE_RADIUS` | The stand, unchanged in its waves and timing |
| `RZ_Mission_TakeTheLab` (`RZ_Mission_Hold`), `RZ_QM_AuthorsTakeTheLab` | The bay hold. `StealBatch` stays only for servers with no finale |
| `OnCureCompleted`, `m_aSeasonFame`, `GetSeasonLeader`, `RZ_SeasonRecord`, `last_season.json`, `RZ_Seasons`, the hall of fame's `AddStand` | Winner, record and hall |
| The questline engine: `MissionStage`, `WaitStage`, `IsReady`, `OnLeaderTalk`, `OnWandererTalk`, `TryPersuade`, `TryBribe`, `RZ_QuestApproach.SideWith`, who took part, the journal, saved runs, `ReplaceLeader` (one per route at most), Wait scale | The five routes |
| World facts: flags, canon (`CanonPos`, `origin.site`), claims, `Fill`, `Changed`, `CanRunNow`, `HeliAvailable`, `IsPowered` | Subjects, doors, crossings |
| `RZ_Stealth.SightRadius`, `m_Quiet`, and the stand's 1.5 km exemption | Sneak ways, and the stand's infected |
| Per-player hostility (`SURV_H1`–`SURV_H15`), safe zones, `StandDown` | Route groups at the stand, price doors |
| Live Capture (the case, Specimen wake minutes), Mobile Lab (`SetAnnex`), Power the Lab, Lab Defence, Steal the Antidotes, Cold Chain, Convoy Escort, Hijack the Convoy, Secure the Shipment, Smuggler's Run, Bust Out, Blood Drive, Patient Zero's Ledger, the Locked Bunker's lock, Trail of Clues, `RZ_Escort`, Surrender, checkpoints, radio channels, `RZ_Mission.CountPlayersNear` | Ways through the acts |

### Changes to built code

| Where | Change |
|---|---|
| `Cure/RZ_CureResearch.c` | (1) While a finale is registered, `Fallback()` hands its amount to the controller's lab-alone track instead of `m_fProgress`. (2) Milestone 4 (reaching 100 by the backlog after a lost stand, or at the seal) asks the controller: outside a seal it marks the batch ready and waits; at the seal it starts the stand. (3) `Describe()` and `{cure_percent}` read the controller's `ShownPercent()` while a finale is registered. (4) `TickFinalStand` gives the controller the players at the lab each tick, and asks it for overtime before `CompleteCure`. (5) `LoseBatch` tells the controller. (6) `StealBatch` is used only with no finale registered. (7) `CompleteCure` asks the controller who controls; `OnCureCompleted` runs after the release; the stand Fame is the controller's (§9). The controller saves the stands-lost record, because the built `m_iBatchesLost` isn't saved. (8) The gate news (`NEWS_GATE`) is reworded to *"The lab has gone as far as samples can take it: 90 percent. What's missing isn't in a sample case, and every faction in the Zone thinks it knows where it is."* The built "isn't in a special's blood" would mislead, since every route asks for live tissue |
| `Core/RZ_GameModeComponent.c` | `OnCureCompleted`: the winner is the controller's faction, and `GetSeasonLeader` is used only when the cure is handed over. A new `AddFactionBonus(faction, amount, why)` into `m_aSeasonFame` (saved, logged). `RZ_SeasonRecord` gains `route`, `how`, `terms`, `finisher`, `deciders` and `routeFame`. `winner` keeps meaning the faction whose rules apply in full (−1 for an Independent cure that isn't one side). `GetEndingText` works by route and terms |
| `Core/RZ_Seasons.c` | Next season: the controller's rules (the Independent by terms), then at most one half rule set (a deal partner, else the Fame leader), then the majors' story rules (a half copy of a rule set, then `Add` and `Cap`). The epilogue default (**Epilogue** hours) moves from 48 to 168, one week; the epilogue design owns it |
| `Director/RZ_Questlines.c` | A route lane: outside Max active and the faction slots; starts at the gate; never fizzles (a give-up retries the stage); freezes at a finish; open and sworn stages gated by `RouteOf`; helpers get half Favor |
| `Missions/Questlines/RZ_Questline.c` | `IsCureFinale()` is true on the five routes. New `OpenStage()` and `SwornStage()` builders |
| `Missions/RedHand/RZ_RedHandContracts.c`, `RZ_MissionRegistry.c` | Take the Lab leaves the random pool while a finale is registered. A finale mode: the bay point, a side-aware hold (the built hold counts any player) that counts every Red Hand-locked player, and a contested pause. It starts at Trusted with the Red Hand for a Red Hand-locked player (built: Champion). Its success is a holder change |
| `Missions/Questlines/Halcyon/RZ_Quest_Authors.c` | `OfferTakeTheLab` goes through the controller, to the Red Hand route only |
| `Factions/RZ_Checkpoints.c` | The lab's safe-zone rule skips fire between locked players during the stand. Bastion's search takes a tagged finale item as contraband from anyone below Known with Bastion |
| `Factions/RZ_Disguise.c` | During the stand, within 1.5 km of the lab, the disguise clears no faction bits: one `IsFinalStand` check, like stealth's `FINAL_STAND_RADIUS` |
| `Factions/RZ_Hostility.c` | During the stand, within 1.5 km of the lab, the controller sets and clears faction bits for rival sides' players. Halcyon's bit is never set |
| `Core/RZ_LooseItems.c` | Finale items are never removed |
| `Cure/RZ_Infection.c` | The cut dose's cure roll writes a line to the cook's book |
| `World/RZ_WorldFacts.c` | New claim subjects `Person(key)` and `Tray(n)`, beside the tribes' `Tribe(id)`. Canon rules for `unturned.*`, `held.*`, `redhand.cook`, `redhand.still`, `cook.sister`, `bastion.site_key`, `origin.sergeant`, `cure.cold.1/2`, `origin.vials`, `origin.site.lower`, `day6.cohort` and `ham.house`. New tokens: `{route}`, `{route_name}`, `{track:<route>}`, `{finale_worth}`, `{unturned_name}`, `{held_count}`, `{fuel_words}`. The built stand claim (`Tick` claims `LAB` for `CURE_OWNER`) becomes the controller's, for the batch hold and the stand only; route stages never claim `LAB` |
| `Persistence/RZ_HallOfFame.c` | "Held the final stand" lists the controller's holders and the bystanders. The season block |

### New code

| File | What |
|---|---|
| `Cure/RZ_CureFinale.c` | The controller (saved). It holds: six tracks (five routes and the lab alone), `AddStep(route, points, why)` and `Finish(route)`; locks per identity (`RouteOf`, `CanCommit`, `Commit`, active locks); `Asset()` and cards; fighting routes; pacts and tallies; the batch hold and the seal; deals; the Coat Run; the stand's room and bay counts, overtime and holders; how, terms and deciders; the route Fame (the snapshot, the soft line); `ShownPercent()`; the end of the contest at the cure; and the interface in §13 |
| `Missions/Questlines/Finale/` | The five routes (`RZ_Route_Haven.c` and the rest), on `RZ_QuestMission` and `RZ_QM_HalcyonScene`. Shared stage classes: door, draw, cold carry, tissue, decide scene. About 20–25 stage missions, mostly built from existing blocks |
| `Missions/Questlines/Finale/RZ_FinaleItems.c` | Finale items: cold containers (cool bag, courier's case, transport case, the vet's tank, the works' flask, Bastion's box, a tray, a bag, the pan's case, the specimen case), keys (114, 113, the second key, the wax copy), the clipboard and notes. Each keeps its holder, place, time left and carrier chain (saved). They drop on death, are never cleaned up, and go back to their seat after 2 hours of play with nobody near. The Drain action (20 s) works on sources and on containers on the ground |
| `World/RZ_FinalePeople.c` | `Person(key)` records for the woman, Twice, the captain, the cook, the sergeant, the vet, the eight and the ham: alive, place (as data), holder, consent bits, draws and betrayals (saved). Bodies only within 300–400 m. Never killed off-screen while claimed. **Consent is a check in the draw and dose actions:** they refuse without the person's yes |

### From the planned pieces, each with a fallback

| Needs | From | Until it's built |
|---|---|---|
| The commit node, `always` topics, pact topics | Dialogue | `RZ_LeaderAction` "Side with them on the cure", then a confirm action within 30 s; the radio through `/t` |
| `CanSideWith` | Reputation | Yes unless Hunted; Hunted needs the price door; Independent always |
| `Asset()` | Missions | No assets |
| The decide-scene hold | Missions | First pick, with a 10 s objection window |
| `RZ_QuestVote` | Majors | The controller's own simple count |
| The look (the Coat Run) | Disguise | The Coat Run is off |
| Capture the Index, tribes | Tribes and crawlers | Two Live Capture specials |
| The owner-only payload | Phase 0 | The built `NotifyPlayer` (which leaks text to other clients; fix it first) |
| `RZ_SeasonStory`, outlets, carriers | Epilogue | Today's ending text, with the controller as the winner |
| Majors' flags | Majors | Read as 0 |
| `Forgive()` and its 30-per-faction budget | Reputation | The built surrender (the usual 8 off) |
| The finale's lane in `RZ_MajorBudget`, `SetPresenceFloor` | Majors | The controller keeps the finale's own ±10 morale and ±5 strength per faction. No presence floor; the Bastion route's stage blocks still come |

### Settings

Attribute defaults only, with no initialisers, created with `RZ_Util.NewWithDefaults`. Under **Finale**:

| Setting | Default |
|---|---|
| Route worth, Floor, Ceiling, Soft line | 0.25, 150, 0, 1.10 |
| Batch real hours, Batch play hours | 12, 2 |
| Seal call minutes, Seal hour | 60, none |
| Take seconds, Take per point, Take floor | 90, 5, 45 |
| Overtime seconds | 180 |
| Clean room radius | 10 |
| Bay seconds | 120 |
| Contest | Auto (Force, Tally) |
| Coat Run, Take the Lab, Pacts | on, on, on |
| Groups at the lab | 2 |
| Fight memory days | 2 |
| Active lock days, Active lock hours | 2, 2 |
| Fame leader rule | on |
| Marks public | off |

### VERIFY

- The lab prefab's clean room child entity.
- Reading a character's unconscious state.
- Reading the server's player-damage setting.
- That spawned groups honour the hostility bits.
- That carried finale items keep their place over a restart.
- Disguise's coat detection.
- Game-time windows under time acceleration.
- Road distance for "in reach" (fallback: straight line × 1.3).
- Placing the loading bay on every map.

### Log lines

- *"Finale: Bastion locked: 5 (active 4)."*
- *"Finale: the Haven +1 (her yes), track 3."*
- *"Finale: pact the Haven + Bastion (tally 3–1, 2–0)."*
- *"Finale: Halcyon finished. Batch hold to {time}."*
- *"Finale: clean room Bastion → the Haven (force) at 11:20."*
- *"Finale: controller the Haven, how force, terms 3, deciders 4."*

### UI, Workbench, tests

- **UI** (in UI.md and UI/LAYOUTS.md):
  - the journal's finale page: the tracks, the route's worth, "What you'd bring", your route's notes, items and pact, the doors and subjects you know, and the seal time;
  - the lock card;
  - the stand's HUD line.
- **Workbench** (in WORKBENCH_SETUP.md, with the VERIFY count):
  - the still prop (fallback: a puzzle object);
  - the clean room marker and the bay marker on the lab prefab (optional);
  - the ham's aerial (optional);
  - the existing group prefabs (Bastion squad, Red Hand group, Halcyon guard, Territory > Haven guard).
- **Before any of this is built** (plan task 37, before phase 5): the last runtime test started none of the 15 questlines, and Convoy Escort, Hijack the Convoy, Mobile Lab and the Locked Bunker didn't start, because their start conditions weren't forced. The finale reuses all of them. So the runtime test first forces their preconditions, and gets each reused mission and every questline to start and end once.
- **Runtime test** (a scripted finale at Wait scale 0.1):
  - a commit refused below the gate;
  - two routes racing, one pact;
  - a stand with a scripted take and overtime;
  - a lost stand and a remake;
  - the slow finish through the lab-alone track.

---

## 15. Guardrails

**Sealed Zone**
- Nothing crosses the line and nothing comes in. Command never answers: the release, the receipt and every call end with *"No reply"*. Under Bastion, Command, the stronghold answers, and that's still inside the line.
- The Day 2 *"Ack."* and the Institute's *"Continue."* come from before the gates closed.
- Nobody says what the site was for, or why she didn't turn.
- The gates open only at the wipe. Codas stop 150 m short of the barrier.

**No sleep and no time skip.** Every dawn, dusk and night window is a game-clock wait. The batch counts real time and play time, and the seal is real time. Clocks count only while someone is online, except the lab's work and the fallback, which are built to run on the server clock.

**Nothing overpowered**
- Strongholds and the lab never fall: the clean room changes hands, the lab doesn't. The store, the still, the site and the draw points are never strongholds. The stockade is reached only through Bust Out.
- Stronghold leaders are talk-only and never threatened or replaced, and the CO's loyalty never changes. At most one leader changes per route (a field lead or the rival crew boss).
- At most two friendly groups at the lab, and they never hold the room. The stand's waves don't change.
- The route Fame is fixed at the gate, paid once, soft-capped and visible. Each player earns at most 50 Fame per faction from all finale content.
- Thefts are once a season. Effects go through the finale's own lane in the season budget, and the finale's own limits replace the questline limits (§13).
- No free loot: the epilogue's outlets own the doses.

**Consent.** No blood is drawn and no untested dose is given without the person's own informed yes. Custody moves people, costs Infamy, counts as a betrayal and gets nothing drawn.

**No companions.** Every walk is an escort block that ends at a place: her walk ends at the lab gate, and so does the cook's case. Nobody follows a player.

**Performance first**
- People, rotas, diesel runs, cold clocks, doors, tracks and holders are data and season flags.
- Bodies only within 300–600 m of a player.
- At most two groups at the lab, and the contest adds no bodies of its own.
- Finale items are tracked and returned, never left as litter.

**Any map, mostly abandoned.** Every place is a canon rule relative to settlements, landmarks and building types, with a fallback: a steel hatch, the lorry still, the Courier for the ham, Take the Lab's point for the clean room. The cold store, the lower store and the still are in empty places, and towns are held only while an act runs there.

**Grey choices and dark humour**
- Every route makes its admission before the lock.
- Every door has a cost, and the kind ways usually pay least.
- Jokes go up or sideways (*"Nobody's is at four."*), never down at the bitten, the volunteers, exiles, children or a named dead person.
- The Long Table is never in it: the still and the ham are kept 1.5 and 2 km from the farm, and no route reads its flags.

**In-world epilogue.** No slides (§13).

**The wipe only after the cure.** *On cure* stays the only default, and the fallback guarantees a cure about 17 days after the gate.

---

## 16. Exploits covered

| Attempt | Why it fails |
|---|---|
| Die to undo a lock | The lock is per identity for the season |
| Alts flood a route to hold, decide or vote | Only active locks count (2+ hours played, online in the last 2 real days). The take time reads the frozen track, and in Force mode headcount doesn't matter |
| Stay unsided, then pile onto the winning side at the stand | A lock made after the batch is ready doesn't count at that stand |
| Scout every route's open acts for pay | Each open act ends once and pays a player once per route, within the 50-per-faction cap |
| Unlocked "mercenaries" deciding sworn acts | Refused at every choice. Helpers get half the Favor and no Fame |
| Lock to A and feed B | Hand-ins and choices check `RouteOf`. A handed-over item is logged, costs +15 Infamy with your own faction, and shows in your route's carrier chain |
| One player binds a whole route to a pact | Pacts are tallies of each route's active locks |
| Hedge by pacting everyone | One pact per route, not shared onward, and deals don't move Fame |
| Farm the route Fame | Snapshot at the gate, paid once a season, soft line, never repaid on a remake |
| Hoard trays | One tray out per route, and one rebuilt each real day |
| Hoard a rival's piece | It can't be handed in for your route (except the crossings in §5), and it goes home after 2 hours of play or a real day |
| Catch every leader to deny rivals | A route with its tissue in gets no more capture stages. Specials are always there |
| Kill a rival's subject | Never off-screen. On-screen it costs +20 Infamy and moves that route to its branch, which is never a dead end |
| Drain every cold source | Each source drains once, at +15 Infamy. Cool bags and couriers' cases are always there |
| Hold a tissue case hostage | It wakes after 12 minutes, and its tribe comes for it |
| Sell false notes | Found out by events. The buyer loses what it learnt, and the seller is remembered |
| Stall the stand with an endless challenge | Overtime is 3 minutes at most, and the holder wins ties |
| Lure everyone away to ruin a rival's batch | It needs nobody within 150 m for 90 s. The finisher keeps its Fame, and the remake is fast |
| Log out inside the clean room | An offline player isn't inside |
| Restart mid-stand | The holder and the counts are saved. The stand restarts as built once a player reaches the lab |
| Repeated thefts | Take the Lab and the Coat Run are each once a season |
| Disguise into the room | No disguise works within 1.5 km of the lab during the stand. The Coat Run is once a season, with call-outs |
| Hunt locked players away from the stand | +3 Infamy with the victim's route faction (the Haven, for an Independent), 12 an hour at most, tagged war. Nothing new is paid for it |
| Grief bystanders at the stand | The safe-zone rule still covers unlocked players, +10 Halcyon Infamy for a killed bystander, and bystanders never count |
| Shoot your own route's players | You don't count as a holder or decider at that stand, and you take +10 Infamy with your route's faction |
| Escape hunting through a price door | It opens the route only. Infamy stays, and fire is held only inside that route's own scenes |
| Intel from walls or radio | Marks are an equal share, channels never name players' routes, and lock counts are never shown |
| Grudges after the cure | The contest stands down at the cure |

---

## 17. Docs in the same commits

- **`docs/FINALE.md` is added only as the finale is built** (decision 16). It then owns the finale: the routes, the lock, the contest, the stand's control, the route Fame and the fallbacks. It's too big for a section, as tribes got INFECTED.md. Add it to CLAUDE.md's Doc map and the README's doc list in that commit. DESIGN.md's cure section then links to it instead of repeating it.
- **DESIGN.md:**
  - "Cure research and the season's end": the gate, the seal, the stand as the control fight, and the new winner rule (control, with season Fame as the fallback), with a link;
  - "Season pacing": the finale row;
  - "Seasons": the next-season rules for the Independent and deals;
  - "Questlines": route lanes, open and sworn stages;
  - "Hall of fame": who held.
- **REPUTATION.md:** "The season's winner"; the lock and its price doors; war deeds; parley with fighting routes; the safe-zone exception at the stand.
- **LORE.md:**
  - "Endings": the cure goes to whoever holds the clean room when the stand is won;
  - the confirmed lore (decision 2): Sedlak's Day 1 cordon detail and the chain on the lower store, the Day 2 tubes, the captain's 13:10 log line and the four marked R, Twice as Fiala (struck off, alive, and how Bastion's route brings him home), Wolff's four on Day 3, the woman, the cook, the eight and the ham;
  - the Red Hand's "crews eat crews" row: Sell the Plan, planned.
- **QUESTLINES.md:**
  - the Crossings rows (`finale.*`, `unturned.*`, `held.*`, `cook.*`, `ref.*`, `eight.*`);
  - "Limits every questline keeps": *"No faction wins through a questline except the cure finale (FINALE.md), and none falls."*;
  - "Crossing lines": finale stages never write the mark.
- **MISSIONS.md:** the route acts, marked built only when built. Take the Lab moves to the finale, started at Trusted by a Red Hand-locked player.
- **UI.md and UI/LAYOUTS.md:** the finale page, the lock card and the stand line.
- **WORKBENCH_SETUP.md:** settings, the VERIFY entries and count, log lines, and the still and bay props.
- **ROADMAP.md:** to the Dropped list, one line each:
  - the earlier route draft's any-side switching (replaced by the lock);
  - its most-Fame winner (replaced by control);
  - Take the Lab stealing the batch under a finale (replaced by the holder change).
- **README.md:** the finale in the feature summary and status.

---

## 18. Risks

| Risk | What to watch, and the answer so far |
|---|---|
| **PvP inside the stand** | A player-against-player fight inside the built 15-minute stand changes the season's climax on servers that play as PvE. Tally mode keeps it working with player damage off, but the take time (90 s minus 5 s a point), the 10 m room, overtime and the body budget (hordes, a Furious tribe, two groups) all need play-tests at 5 and 20 players |
| **Small servers and time zones** | A server may have one active lock per route, or none, and the 60-minute seal call won't reach everyone. Every act has a solo way, Seal hour exists and the fallback guarantees a cure, but a 3-player server may mostly see uncontested or slow finishes |
| **Scale** | Five routes, about 20–25 stage missions, a controller, finale items and people, pacts, and the stand's room and bay make the largest feature in the mod, plus roughly 1,500 lines of text. The finale is plan phase 6 and shouldn't start before phases 1–5. Each dependency has a fallback, but the full experience waits for dialogue, reputation, missions, disguise, the epilogue, tribes and the majors |
| **Changes to built behaviour** | They must land with their docs in the same commit: the winner rule (control instead of most Fame), the stand's Fame, Take the Lab under a finale (and its Trusted start), the lab's safe-zone exception and the disguise exception at the stand, the gate news, the fallback becoming the lab-alone track, the cure percent read from the controller, and the epilogue's week. Admins need them in the changelog |
| **Engine unknowns** | The clean room child entity, reading unconsciousness, reading the player-damage setting, spawned groups honouring the hostility bits (the Haven's bit already shows in name tags), carried items over restarts, and the coat look. Each has a fallback. They belong in the phase 0 spikes |
| **The ring of doors as a stall** | A door-holding route can stall a rival by guarding its door. The other way always exists, but if it's too slow, one faction can effectively decide another's season. Tune the other ways' costs in play-tests |
| **Asymmetry** | The Independent has no group at the stand, and Halcyon's neutral lab gives its route none either. Either may be under-picked, or become a target. Watch lock counts by route in the first seasons, and tune with pacts, the Coat Run and kill Infamy |
| **The Fame chunk may feel hollow** | The route Fame chunk (about 375 at the design point) is a model, not measured. With control picking the winner, it may feel hollow unless the journal and the hall of fame make the season's Fame line visible. Calibrate Route worth from the status log and `last_season.json` |
| **Pacing** | An uncontested route takes about 4–5 real days, but contested routes with live tissue on a one-tribe map can run long. The built fallback (about 17 days) is the backstop. Check route length against the 45-day season in play-tests |
| **Tribes come first** | Tribes and crawlers aren't built. Until they are, every tissue act falls back to two specials, so leader scarcity, Fury and day parties go untested with the finale |
| **Save size** | Locks per identity, tracks, items with holders, clocks and carrier chains, people, pacts and the stand's counts come to an estimated 20–40 KB. Measure `state.json` after the finale lands |
