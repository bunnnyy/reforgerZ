# Living world

How the world changes on its own, without becoming overpowered or lagging
the server. Three rules apply throughout:

- **Simulate as data, spawn as bodies only near players.** Population,
  wanderers and territory are numbers on the server. AI entities
  exist only within a few hundred metres of a player. The one exception is supply trucks: at
  most one real truck per faction is on the road (section 3).
- **Effects are capped and recover.** Nothing a player or the Director does wipes
  out a faction, a town or a safehouse for good.
- **Background, not foreground.** Territory and adaptation are slow, low-key
  systems you notice over a session, not something you manage.

## 1. People

### Safehouses and population ✅ (built, `World/RZ_Safehouses.c`)

A safehouse is a single Haven-held spot in an otherwise abandoned town. A rescued
survivor isn't kept as an AI body: they become **+1 population** at the nearest safehouse.

- **Where they are.** At the first start of a season, up to 3 safehouses (one per 8
  settlements, at least 1) are picked: the first at the Haven quartermaster, the rest in quiet,
  low-infection villages, at least 1.2 km apart, away from the lab and the quarantine line.
  Positions, population and food are saved. Mission makers can place
  `RZ_SafehouseMarkerEntity` entities instead; placing any turns the picking off.
- **Few bodies on screen.** While a player is within 300 m, a safehouse shows 3–6 occupants
  (never more than its population), its guards, its medic and its own Haven quartermaster.
  They're removed again when everyone has left. A safehouse of 4 and one of 30 look about the
  same; the difference is in what they offer.
- **Population tiers** (hard cap of 30 per safehouse; past the cap, new arrivals go to the
  next nearest safehouse):

  | Population | Safehouse becomes | Built effects |
  |---|---|---|
  | 0–4 | Shelter | A spawn point; the quartermaster stocks Known items only; 3 occupants |
  | 5–11 | Community | Trusted stock, one guard, 4 occupants |
  | 12–19 | Settlement | A medic (heals for Favor, takes bite declarations) and a radio (missions within 3 km); 5 occupants |
  | 20–30 | Town | Champion stock, two guards, 6 occupants |

- **Upkeep.** Every 3 hours of real time with players online, a safehouse eats one food
  unit per 5 people. Food is a plain stock per safehouse. Players top it up by handing food
  items to the quartermaster (prefab names containing food words; 2 Haven Favor per item).
  If the stock is short, 1–3 people leave: to the nearest safehouse that can feed them, or onto
  the roads (one may become a named wanderer). Upkeep never takes a safehouse below 2 people.
  Haven supply trucks bring 6 food when they park at a safehouse (section 3, "Supply lines").
  Bastion's Requisition contract takes 4 food when the supplies go to Bastion.
- **Rescues add population.** Survivors Trapped adds 3 (4 with a ★ Nurse or Doctor), Rescue the
  Kids adds 3, and escorting the Kid wanderer adds 1. About 1 in 10 rescued survivors won't stay
  behind walls and becomes a named wanderer instead, if the roster has room.

### House rules and the dark side ✅ (built)

What the Haven expects of you, and what happens when it turns you out. Standing effects are
in [REPUTATION.md](REPUTATION.md), "Haven house rules and exile".

- **No shooting.** Drawing a weapon in a safehouse is fine. Firing within 50 m of one costs
  Haven Infamy, unless an infected is within 40 m of the safehouse at that moment.
- **Declare your bite.** Telling a safehouse you're bitten slows the infection (×1.5 of the
  time left, once per life) if it has a medic. A bitten player who hasn't said so and is
  inside a safehouse in their last 5 minutes may be caught: Haven Infamy.
- **Exile.** Hunted by the Haven: safehouse services refuse you ("the door stays shut"),
  Haven wanderers warn other players near you, and a safehouse short on food may sell your
  rough location to bounty hunters. Come within 100 m of a safehouse with a firearm and its
  guards shoot; unarmed, exile stays a shut door. How the AI tells one player from another
  is in [REPUTATION.md](REPUTATION.md), "What makes AI shoot you".
- **Grey contracts.** Walk Him Out, The Ledger and A Quiet Arrangement
  ([MISSIONS.md](MISSIONS.md), "Haven house contracts").

### Wanderers ✅ (built, `World/RZ_Wanderers.c`)

A small roster of named survivors travels the map: 6–10 per server (6, plus one per 6
settlements above 8). Each one has a role, a route and opinions. Names come from a seed, and
the roster, positions and memories are saved.

| Role | What they do | Typical reaction to you |
|---|---|---|
| Trader | Moves between safehouses; sells goods for Haven Favor, cheaper if you've helped them | Trades if you're not Hunted by the Haven |
| Doctor | Patches you up, or slows an infection (×1.25 of the time left, once per life) for Favor | Refuses the Red Hand's Trusted |
| Scavenger | Sells a rumour pointing at a puzzle mission in progress, or at a stash, for Favor or one food item | Swaps a rumour for food |
| Deserter | Ex-Bastion, paranoid; tells you what Bastion is up to | Friendly if Bastion hates you, refuses if Bastion likes you, draws on Bastion's Champions |
| Preacher | Walks between towns; while he rests in one, his bells add heat there | Harmless; agrees to stop the bells for an hour if asked |
| Courier | Hands out an unarmed delivery job (letters to a safehouse) | Any non-Hunted player |
| Kid | A lost child, wandering | Follows you to a safehouse if you show the way; runs from gunfire |

**How they behave.** A simple state machine: travel → rest (10–20 min at a safehouse,
3–8 min in a town) → choose somewhere new, 0.6–3.5 km away.

**They react to the world** (checked every 30 s):
- A destination that becomes overrun (infection 75+), loud, or near the quarantine line is
  dropped for another. Routes that cross water or the line are never chosen.
- A horde or a lot of noise nearby (an infected within 40 m, or heat 50+): they stop and hide
  for a few minutes.
- During Night of the Dead or a storm, they shelter until it ends.
- Rarely, an infected within 12 m bites one. **Bitten on the road:** players within 1.5 km
  hear about it; the wanderer heads for the nearest safehouse, and an antidote used next to
  them within 20 minutes saves them (and they remember who did). A Doctor nearby slows it.
  Otherwise they turn, and a special appears there if players are near.

**They react to you.** Attitude is your standing with their faction (the Haven for everyone
but the Deserter, who reads Bastion the other way round) plus their **memory of you**, stored
per player:
- *Helped me* (traded, treated, fed, escorted, paid a debt) → better Trader prices, and they
  add a warning when they talk ("one of the changed ones, 300 m NE").
- *Robbed or shot at me* → they refuse you and run, or, if their body is an armed AI group,
  shoot you when you come within 80 m. Robbing or shooting near them (with no infected close)
  costs Haven Infamy.
- *Saved my life* (killing infected right next to them, or an antidote) → a one-off gift of
  Haven Favor, and they remember it for the rest of the season.

**Talking.** "Talk" gives one piece of road news: where the noise is, which safehouse is out of
food, an event nearby, or where they're heading. Actions on the body: the role's special,
"Share some food" (barter: a food item for goodwill and news), and "Rob".

**Death is permanent and makes the news.** "Old Marta the trader was killed on the road to
the safehouse in Morton." Whoever shot at them in the last 30 s takes Haven Infamy. A
replacement (half the time the same role) shows up a real day later.

**New-player guidance** (section 9) sends a Courier or Scavenger to brand-new players with
`RZ_Wanderers.NudgeToward(playerId, roleMask, maxStart)` and adds a line of faction gossip to
their talk. `DescribeNearest(pos, roleMask)` (the nearest one with distance and direction) is
there for other systems.

**Performance.** When no player is within 400 m, a wanderer is just a position moving in a
straight line on the server tick, with no entity and no AI pathing. At most two bodies exist
near each player.

**Simplifications:**
- Routes are straight lines between places, not roads.
- A body that's an AI group walks to its destination on its own. A body that's a character
  without AI, or a prop (the fallback puzzle civilian), stands still while someone is near and
  walks on, unseen, once they leave. Fleeing without AI means the body is removed and they
  move on out of sight. An escorted Kid without an AI body goes ahead unseen while you stay within 150 m.
- Only a wanderer whose body is an armed AI group fights back (the Deserter at Bastion's
  Champions, anyone at players who robbed or shot at them; never the Kid). Wanderer bodies are
  on the Haven's faction, so this works through the player's Haven bit within 80 m of the
  wanderer: any other Haven AI there (a safehouse's guards, if the wanderer is resting at one)
  joins in. Without an armed AI body, the Deserter's refusal is still a pistol and a retreat.
- Wanderers don't react to outposts changing hands (section 3); they only pass on the gossip.

**Planned:** Haven Champion "Community" respawns with a supply bag. (Barter chains across three
wanderers are the Barter Chain contract, section 2.)

### Leaders ✅ (built, `Factions/RZ_Leaders.c`)

A named person runs every place a faction holds. What they do for and against you is in
[REPUTATION.md](REPUTATION.md), "Faction leaders"; their personal jobs are in
[MISSIONS.md](MISSIONS.md), "Leader missions". Here is where they are and how they're simulated.

| Seat | Built now | Planned |
|---|---|---|
| Stronghold (one per faction) | At the faction's quartermaster; Halcyon's at the lab. Without a quartermaster: the Haven's at its first safehouse, the others in a town that suits them. This is the territory's Stronghold (section 3) | — |
| Haven safehouse | One council head per safehouse | — |
| Outpost | One per territory outpost (section 3): the territory calls `RZ_Leaders.RegisterOutpost(faction, pos, name)` for each and `UnregisterOutpost(id, why)` when it's lost or dropped. With **Territory** off, **interim leader posts** instead: 1 per faction for Bastion, the Red Hand and Halcyon (a "field station", never more than one), picked once in a town that suits the faction, saved. Mission makers can place `RZ_OutpostMarkerEntity` either way | — |

On Arland that's about 9–10 leaders (4 strongholds, 1–2 council heads, 4 outposts); on Everon
about 16–20.

- **Data first.** A leader is a saved record (name, title, backstory, traits, loyalty,
  memories). A body exists only while a player is within 200 m of their seat. At a
  stronghold or safehouse, the quartermaster there *is* the leader unless a leader prefab
  is set (it gets `RZ_LeaderAction`s, and its puzzle components are labelled if it has them).
  Outpost leaders without a prefab use the puzzle civilian.
- **Stronghold leaders** never die and are never replaced; their bodies take no damage.
- **Others die off-screen** (bites, raids, Bastion acting on a tip) or if a player kills them,
  and are replaced two hours later by someone new. Deaths, replacements and big loyalty
  changes are headlines ("Word on the roads"); the latest is in the session news on a player's
  first spawn (section 7).
- **Personal jobs** come up "occasionally": every two minutes, an Independent or Rogue leader
  with a player near them and no job on has a 35% chance to think of one (at most one per
  30 minutes per leader), and asking them for work tries straight away. They're offered only
  in person.

**Simplifications:** with territory off, interim posts are leader seats only (no guards or
shop); territory outposts have guards, named people and a quartermaster when the prefabs are
set. A leader's "people" who can see a rogue job are their faction's other seats (within 60 m)
and, for the Haven, its wanderers. Off-screen deaths only happen with nobody within 400 m.

## 2. Unarmed missions: low level, but still a challenge ✅ (built, `Missions/Unarmed/`)

For new characters, Unarmed backgrounds (Nurse, Lab Tech, Unemployed), or
anyone who doesn't want a gunfight. These are about **stealth, noise and
thinking**, and they pay small amounts of Fame with whoever offers them.
What each one does in code, its rewards and simplifications are in
[MISSIONS.md](MISSIONS.md), "Unarmed missions".

Two small mechanics make them work:
- **Noisemakers.** A thrown bottle or firecracker, an alarm clock, a car alarm. Each one adds heat
  at a spot, which pulls zombies toward it: the heat map keeps the last few loud noises, and
  infected a mission controls walk to one that's close.
- **Being noticed.** A mission can fail if heat near you passes a threshold or a zombie stays close for too long.

| # | Mission | From | Challenge | Status |
|---|---|---|---|---|
| 1 | **Pharmacy Lure** | Haven | Infected fill a pharmacy. Use noisemakers to draw them out, then sneak in for the medicine | ✅ |
| 2 | **Silent Courier** | Courier | Carry a letter through an infested town without raising heat over a limit | ✅ |
| 3 | **The Safe Code** | Scavenger | A safe needs a 4-digit code. Clues are on notes in 3 houses; wrong guesses set off an alarm | covered by The Last Experiment and Time-Lock Safe |
| 4 | **Breaker Sequence** | Bastion | Restore power to a building by flipping breakers in the right order. The wrong order sets off an alarm | covered by Junction Boxes |
| 5 | **Lead Them Away** | Haven | A horde is drifting toward a safehouse. Use noise to lead it to a marked quarry without getting caught | ✅ |
| 6 | **Rooftop Rescue** | Haven | A survivor is stuck on a roof. Find a ladder route and time it between zombie waves | ✅ |
| 7 | **Trail of Clues** | Wanderer | Follow belongings and notes from a missing person's house to wherever they ended up | ✅ |
| 8 | **Car Keys** | Mechanic wanderer | A working car, but the keys are in one of 5 houses. Search quietly, drive it back | planned (needs fixable cars) |
| 9 | **Scout the Town** | Bastion | Visit 4 observation points in town and stay unnoticed at each for 20 s | covered by Recon |
| 10 | **Night Lantern** | Halcyon | Reach a high point at night with no light and switch on a beacon | covered by Lights in the Window and Signal Fires |
| 11 | **Water Run** | Haven | Carry heavy water containers back. Choose the slow safe road or the fast risky one | ✅ |
| 12 | **Barter Chain** | Wanderers | Trade item to item across three wanderers to get what the first one wanted | ✅ |
| 13 | **Distraction** | Anyone | Keep the zombies in one area busy with noise for 3 minutes while another player loots | ✅ |
| 14 | **Medicine Before Dark** | Doctor wanderer | Bring a specific medicine to a bitten wanderer before nightfall | ✅ |
| 15 | **Quiet Count** | Halcyon | Count the specials in a town without getting close. Answer correctly for the reward | ✅ |

The higher-tier version of a contract can be armed; the entry-level version
doesn't need to be.

## 3. Factions in the background (strongholds never fall)

### Territory ✅ (built, `Factions/RZ_Territory.c`)

- **Most of the map is abandoned.** Only a few towns are held, and expansion stops once
  about a fifth of all settlements are held (strongholds included; **Held share**, 0.2). The
  rest are empty, looted or infested, which is where the scavenging, missions and danger are.
- **One Stronghold per faction**, where its stronghold leader sits: its quartermaster
  (Halcyon's at the lab; with leaders off, the same places by the same rules). It can never be
  captured or destroyed. At worst it is **Under Pressure** for 25 minutes: while a faction's
  strength is 25 or less or its morale 20 or less, each territory tick has a 50% chance to start
  it. Then no supply truck leaves and the stronghold quartermaster's Champion tier is locked.
  It ends on its own (+5 strength, +5 morale), with a two-hour pause before it can happen again.
- **Outposts, by map size:** under 24 settlements (Arland) 1 per faction, under 48 2, more 3
  (**Outposts per faction** overrides it, 1–4). Halcyon never has more than one: its field
  station. Each faction can grow one past its starting count.
- **Towns that make sense.** A town scores by faction; "on the roads" is read from how many
  towns lie within 1.6 km (a hub), which works on any map:

  | Faction | Prefers |
  |---|---|
  | Bastion | towns (size 2+), hubs, names with "air", "base", "field" or "port" |
  | Red Hand | hubs (crossroads) and small places (outskirts), some infection |
  | Haven | quiet villages: small, low infection, few neighbours, away from the Red Hand |
  | Halcyon | quiet, low-infection places |

  Never: a town that's already held (or within 400 m of a holding), a Haven safehouse town
  (safehouses don't count as holding a town, but nobody sets up next to one), the lab town, a
  town at 85+ infection, or anything near the quarantine line. Not too close to the faction's
  own stronghold (under 700 m), and rarely the far side of the map (over 6 km).
- **The territory tick**, every 20–30 minutes while players are online, does at most **one**
  thing. First the floor (a faction with no outpost founds one), then each faction in random
  order by its strength:

  | Strength | Does (first that applies) |
  |---|---|
  | 60+ | win back a town it lost (after 30 min, within 6 h: re-found it if it's empty, or take it from a weaker holder); found an outpost near its holdings if under its count (or half the time if there's room); at 65+ take the nearest outpost of a neighbour at least 10 weaker (any weaker during a feud), within 4 km of its holdings |
  | 41–59 | under its count: found one (half the time); else 20%: drop an outpost with almost no traffic that has stood 3 hours |
  | 40 or less | 60%: drop its quietest outpost (not one with real traffic) |

  The Haven and Halcyon never take towns. Nobody takes or drops an outpost a player is near,
  a faction's last outpost, a placed marker, or one from a faction it has a truce with.
- **Traffic** is player-minutes within 400 m, plus 5 for each contract done for the faction
  within 1.5 km; it fades by a quarter every tick.
- **Faction strength** (10–90, starts at 50): +2 for the giver of a finished contract, −1 for a
  rival it hurt, −1 for a failed one (at most 6 points an hour per faction from contracts);
  +1 per truck delivered, −2 per truck lost; +3 / −5 when a town changes hands. It drifts 2
  points an hour back toward 50.
- **Headlines only.** Every change goes into the headlines ("Bastion has set up a post in
  Morton. Captain Weber is in charge.", "The Red Hand has driven Bastion out of Levie.") without
  a server-wide alert: players see them on their first spawn, and leaders mention the latest.
- **Leaders.** Each outpost's leader is registered through `RZ_Leaders.RegisterOutpost` and
  leaves with `UnregisterOutpost`; a town that changes hands gets a new leader. With territory on,
  the leaders' interim posts are turned off; on the first run with an older save, the saved
  interim posts become outposts, so their leaders keep their names, loyalty and memories.
  `RZ_OutpostMarkerEntity` entities are fixed outposts: never dropped or taken.
- **Bodies only near players.** Within 300 m of a holding: its guards (the faction's guard
  prefab), two of its three **named people** (a guard or occupant with a name in the faction's
  style and a role: the cook, the sentry, the toll man...), and at an outpost the faction's
  quartermaster (its shop; Bastion's Requisition). Named people are saved and come back on
  every visit; a town that changes hands gets new ones. Talking to one gives a line or two in
  the faction's voice, what the faction needs, and now and then a remembered moment.
- **Routines** (atmosphere only): shift change at 05:30 and 17:30, the dusk meal at 18:30,
  night watch from 20:30. A player near a holding hears the routine change (at most every
  20 minutes per holding), and the people there mention it. Leaders, quartermasters and
  occupants can always be talked to and traded with, whatever the hour.

*Simplifications:* towns are judged by size, infection, name and how many towns are near, not by
real roads or airfields. A town "taken" changes hands as data while nobody is there: there's no
battle to watch. Guards and named people stand where they're spawned.

### Supply lines ✅ (built, `Factions/RZ_SupplyLines.c`)

- **A real truck does the resupplying.** Outposts and Haven safehouses only restock when a
  real vehicle with an AI crew drives in from the faction's Stronghold and parks (within 50 m).
  No truck, no restock. The one exception to "bodies only near players": at most one truck per
  faction is on the road, so the AI cost stays small. Places within 300 m of their stronghold
  are stocked by hand and never need a truck.
- **Departures** every 40–60 minutes at a random time (the first 10–40 minutes after a start),
  only while players are online and not while the stronghold is Under Pressure. The truck goes
  where it's needed most: a short place first, then the least stock, then the longest wait.
  The crew boards on the next tick and drives to a move waypoint at the destination.
- **Stuck:** a truck that hasn't moved 8 m in 90 s, with no player within 1 km, is quietly moved
  80–150 m on toward its destination. With a player near, it stays stuck.
- **Failure:** destroyed, robbed (its crew killed) or late (twice the trip at 4 m/s, plus 15
  minutes). The destination is then **short** until the next delivery, which is 40–60 minutes
  away: about 30% of its shop's items are out (the same ones until the truck comes), prices are
  +25% to +50%, half the time the Champion tier is locked, and an outpost's stock drops to 0.
  Players within 600 m are told, and `RZ_Guidance.Report(..., "shortage", place)` fires for
  them (and for anyone who walks up to a short place later).
- **Stock:** a delivery fills an outpost to 8 units; it uses one every 45 minutes with players
  online. Bastion's Requisition draws one unit per item ([REPUTATION.md](REPUTATION.md)).
- **Haven trucks carry food:** arriving at a safehouse adds 6 food (`RZ_Safehouses.AddFood`).
- **Players and trucks.** Ride along within 150 m for 40% of the trip and see it arrive: +4 Fame
  and +5 Favor with the owner (not if they hunt you). Its crew dead, drive it in yourself:
  the delivery counts and pays +8 Fame. Or drive it into a rival's stronghold (within 80 m):
  +6 Red Hand Fame (the receiver's Fame, for a Red Hand truck) and +10 Infamy with the owner.
  Killing the crew is shooting the faction's people: see "Shooting back" in
  [REPUTATION.md](REPUTATION.md). Trucks aren't contracts; Convoy Escort and Hijack the Convoy
  are still planned ([MISSIONS.md](MISSIONS.md)).
- **No traces.** A finished truck stays parked while anyone is within 300 m (up to 15 minutes),
  then it and its crew are removed. A robbed truck can be taken for 20 minutes; one a player
  sits in is never removed under them.

*Simplifications:* trucks drive straight to one waypoint (the AI picks the road) and don't drive
back. A restart takes a truck off the road with no shortage. "In the truck" is a player in a
vehicle within 6 m of it. Supply trucks need territory; with it off, or without a faction's
truck and crew prefabs (or the move waypoint), that faction's places count as always supplied.

### Factions feel alive ✅ (built, `Factions/RZ_FactionLife.c`, `RZ_FactionVoice.c`)

Capped, background and cheap. Morale, needs, truces and feuds, and what they change for
players, are in [REPUTATION.md](REPUTATION.md), "Morale, needs, truces and feuds".

- **Radio chatter** with real information, every 10 minutes, one faction at a time, on its
  open channel: the next truck leaving its stronghold (if within 20 minutes), where its truck on
  the road is heading, a short place, an outpost's next shift or night watch, the Red Hand's toll
  hours (and Bastion's truck on the road, which it watches), Halcyon's research and samples
  wanted. The Haven's channel reaches everyone with a radio; the others reach people Known with
  that faction, and Radio Operators, who tune rival open channels too.
- **Secure channels** are a hook for a later step: `RZ_FactionLife.GetSecureTraffic(faction)`
  is what a faction says where nobody's meant to hear (its truck's route, the next departure).
  A Radio Operator within 400 m of a faction's holding when it chatters gets the "channel" tip.
- **Life events** in the headlines, one every 35 minutes or so (±30%): funerals for leaders
  and truck crews who died (10–20 minutes later), Bastion promotions (a named private or
  corporal is promoted, and the new rank sticks), recruiting in a strong faction's town (strength
  58+), a researcher walking out of Halcyon when its morale is under 40.
- **Remembered moments**: the night the lab nearly fell (a lost final stand), the convoy that
  burned on the road to Morton, the day Bastion lost Levie, the day a leader was shot, the horde
  at Morton (one every two hours at most), the week the towns started falling, the first
  antidote, the day the cure went out. The last 8 are saved. Leaders and named people bring
  one up a quarter of the time ("Not since the night the lab nearly fell."), wanderers now and then.
- **Rumours**: a leader's news and a wanderer's talk include gossip about another faction
  (rivals twice as often): its strength, a town it holds, what it needs, where its truck is going,
  or a smear. About a rival it's biased or false 35% of the time; about anyone else, and in a
  wanderer's gossip, 10%. A false one inverts the strength, says a town was lost that wasn't,
  names the wrong destination or invents a need.

## 4. Infection adapts, a little ✅ (built, `Director/RZ_Adaptation.c`)

Small, capped changes that keep late game interesting without making it a wall:

- **Specials after breakthroughs.** Each cure breakthrough makes specials slightly more common:
  the stage's special chance ×1.05 per breakthrough, ×1.15 at most. A relative bump, so an
  Early Outbreak 1% becomes 1.15%, never a new floor.
- **One new trait per outbreak stage**, never more, and they add up: Collapse swaps 15% of common
  spawns for **runners**, Overrun puts **helmeted** ones in the same 15% (runners or helmets at
  random), Aftermath makes 10% of specials **screamers**. A screamer that comes within 40 m of a
  player shrieks: +20 heat where the player stands, at most once a minute per screamer, and
  players within 150 m hear it. Each trait is a prefab list (**Zombies > Adaptation**); an empty
  list means no runners or helmets, and screamers then use the ordinary special prefabs.
- **Favourite spots.** The map is split into 500 m squares that count player-minutes (fading by a
  fifth every 10 minutes). Once a square has 60, three ordinary infected are spawned out of sight
  250–350 m away and walk over to look, at most once an hour per square. Never within 150 m of a
  safehouse, and never around a brand-new player in their gentle first minutes. A nudge, not a
  punishment.

*Simplification:* "sees" is distance only (no line of sight). Runners and helmets are whatever
the prefabs are; the code doesn't change speed or armour itself.

## 5. Time and environment ✅ (built, `Director/RZ_Environment.c`, `World/RZ_PowerGrid.c`, `World/RZ_RadioStation.c`)

Time is never skipped or changed. The environment is read every 30 s and nudges other systems.

- **Day and night** (the game clock's night, 20:30 to 05:00, as in `RZ_GameTime`):
  - **They hear further:** a quarter of every bit of heat also reaches the four neighbouring
    heat squares (250 m each).
  - **Specials** are ×1.25 as common.
  - **Human AI patrols less:** each holding shows one guard fewer (never under one), standing
    5–15 m from its centre instead of 15–35 m; half the hunter-squad rounds are skipped.
  - A good time for stealth and unarmed missions.
- **Weather:**
  - **Rain** dampens noise: heat added ×0.7 while it rains (on top of the Storm event's own ×0.6).
  - **Fog** limits sight lines for everyone (that's the engine's fog). Infected may spawn a
    little closer to players in it (the closest spawn distance ×0.75).
  - **Storms** (the Storm event, or real rain of 0.6+ with wind of 10 m/s+) ground Bastion's
    helicopter: the Last Helicopter contract isn't offered. Wanderers shelter.
  - Players hear one line when rain or fog sets in (at most every 20 minutes each).
- **Power grid:** one power station per map: a placed `RZ_PowerStationMarkerEntity`, else a real
  power or industrial building near a town (prefab names with "power station", "substation",
  "transformer", "factory", "warehouse"...), else the edge of the biggest town. Its control
  object (its own prefab, else the fuse box, else the generic puzzle object) exists while a
  player is within 300 m. Restoring it takes three steps there:
  1. **Force the control room door** (loud).
  2. **Reset the breakers and start the generator:** a horde is sent at it, and someone has to
     stay within 60 m for 90 s while it spins up (nobody there for 30 s and it dies).
  3. **Throw the main switch.**

  Then every town within 2.5 km is **Powered** (town memory, section 8) and everyone hears it.
  What power does: a hook for lights (`RZ_PowerGrid.IsPoweredAt(pos)`; no lamps are switched on
  yet), working radios (faction chatter every 6 minutes instead of 10, and the radio station
  can broadcast twice as often), and an electronics loot bonus for the search system
  (`RZ_PowerGrid.LootMult(pos, "electronics")`, ×1.5). But the generator hums: 10 heat a minute
  at the station, which draws hordes. It **slowly fails**: about four hours with players online.
  Players in the region are warned at 20%. **Refuel and grease the generator** restores 60% with
  a fuel item (prefab name with "jerrycan" or "fuel", used up) or 25% without, once per player
  every 10 minutes. When it fails, the lights go out and it's back to step 2. No Fame for any of
  it: the power is the reward.
- **Radio station:** one broadcast point per map: a placed `RZ_RadioStationMarkerEntity`, else a
  real radio mast or antenna near a town, else high ground near a mid-sized town. Take it over
  by staying at the console for 30 s with no infected within 15 m; it's yours until you walk 150 m
  away. Then send one of four preset messages to everyone, signed with your name and filled in
  with real places: a horde warning (the loudest town), food for a safehouse (the hungriest),
  meet here (the station's grid), and "still here" (the day count). One broadcast every 10
  minutes server-wide, 5 while the station has power. Cosmetic and social only: no Fame, no
  Favor, nothing changes in the world.

*Simplifications:* the weather accessors are engine calls that may need fixing (VERIFY); without
them only the night and the Storm event apply. "Fog limits sight lines" is the engine's own fog;
the code only moves spawns. The power station's stages are actions on one object, not real
breakers.

## 6. Player-to-player (toned down)

- **Notice board** at each safehouse: short text notes left between sessions. Harmless and social.
- **Mission locations:** contracts give a rough area; the exact spot appears once
  you're within 500 m. Radio Operators and Trusted players see it from the start. No other perks.
- No propaganda bonuses, nothing that gives Fame for free.

## 7. Story director: arcs ✅ (built, `Director/RZ_StoryArcs.c`, `Director/RZ_Headlines.c`)

The Director strings events into short storylines. Each step only fires if
the previous one really happened (a mission ran, a player heard a rumour, a truck left), so
every server lives through different plots. An arc whose next step can't happen fizzles out
quietly. Arcs reuse what the mode already has as their steps: missions and events, wanderers,
leaders, territory, supply trucks, safehouses and the headlines.

- **When:** at most one arc at a time. About every hour with players online (±25%), 60% of the
  time one starts: a random arc that hasn't run in the last four hours and can run now.
- **Saved:** the running arc and when each last ran. An arc waiting on a mission isn't saved
  (missions don't survive a restart); it just ends.

| # | Arc | Steps (each needs the one before) |
|---|---|---|
| 1 | **The Missing Courier** | A Halcyon courier stops reporting (headline; the Lost Colleague contract, renamed). Once that search has ended, wanderers say they saw Red Hand men with lab cases near a Red Hand outpost (or a rough town). Once a player has heard it from a wanderer (within 40 min), a headline: that camp sells antidotes in Halcyon cases. Ten minutes later the Red Hand comes for more: a **Lab Defence**. Held or lost, a closing headline |
| 2 | **The Last Convoy** | Bastion announces a convoy to pull an outpost's people and stock back to its stronghold (headline and an alert). Its next real supply truck goes to that outpost within 10 minutes. Once it leaves: Red Hand lookouts on the ridge (headline), the Red Hand's Known hear it's worth robbing, Bastion's Known are asked to ride along. Delivered: the post is restocked and stays. Lost or stolen: it runs dry (a normal shortage). Players escort it, raid it or stay out of it, with the normal truck rewards |
| 3 | **The Preacher's Flock** | The Preacher wanderer rings his bells in every town (twice the heat, a headline per town). After two towns, the hordes follow him: up to three small hordes at his position, only with a player within 600 m. Ends when someone asks him to stop (his "stop the bells" option), after an hour, or if he dies. A job finished within 800 m of him while it runs makes a headline: his bells were used as cover |
| 4 | **The Deserter's Map** | A world event: a dying deserter's body by a road holds a map piece naming two living wanderers. Once it's read, talking to each of them gives their piece; with both, the grid of a hidden Bastion bunker. Reach it (four infected come when you get close): a reward crate. Fails if a holder dies before giving their piece |
| 5 | **Fever Town** | A safehouse's town creeps up in infection (20 points at most, a headline). Only if a player goes within 300 m of it in the next 40 minutes: a Halcyon contract checks the water there and in two towns nearby (Fever Town: the water). Only if that succeeds: a headline (the well is bad; a Red Hand rumour says someone poisoned it) and a Haven contract (Fever Town: get them out, hold the gate for 2 minutes): done, two people move to another safehouse and the added infection goes away; failed, two die |
| 6 | **The Quiet Congregation** (the Haven's dark side) | Rare: only when a safehouse has lost 6+ people lately (hunger, raids, contracts; fading by one an hour) and the Preacher is alive; at most once every 12 hours, and only 35% of the times it could start. The Preacher walks there (headline). Once he arrives (or after 30 min), the house prays with him: half portions at upkeep, bells at the gate (a little heat), its people say so, and the gossip says it "borrows" from the other safehouses. Players resolve it by bringing 6 food there, or by asking the Preacher to stop while he's there. Nobody does, and after 90 minutes he walks on and two people go with him. Always ends |

**Headlines:** the news is kept with the leaders (the last 10): leader news, territory changes,
faction life events, arcs, the power grid and burned towns. A player's first spawn of a session
gets 3–4 lines of server news ("Word on the roads"): the latest headline, the last wanderer lost
on the roads, the biggest rescue so far this season, now and then a dead character's journal
(section 8) or a line of Haven gossip (a hungry safehouse on **half portions**, or **"borrowing"**
flour from a fed one), and always the cure percentage. Leaders mention the latest headline;
wanderers and leaders bring up the running arc, and wanderers the Haven gossip, now and then.

*Simplifications:* The Missing Courier's search is the Lost Colleague contract with a new name
and brief; the "Red Hand camp selling antidotes" is a headline, not a shop. The convoy is the
normal supply truck sent to a chosen outpost. The flock's hordes are spawned at the Preacher,
not walking after him from town to town. The congregation's "borrowing" is talk only; its half
portions are real.

## 8. Town memory (data only, no lag) ✅ (built, `World/RZ_TownMemory.c`)

- **Town flags:** Looted, Barricaded, Powered, Burned. These are plain values per settlement,
  not objects in the world, and they're saved.
  - **Looted:** each town has a loot level (0–1). The search system (planned) reports what a
    search took and scales what it finds by what's left. A town under 0.5 counts as Looted.
  - **Barricaded:** set by a building or fortification system (planned).
  - **Powered:** set by the power grid for the towns in its region (section 5).
  - **Burned:** a fire system (planned) reports each building that burns; a town is Burned once
    half its buildings have. The building count is a guess by size (12, 30 or 60) until a system
    reports the real one. Each burned building stops counting after an hour, so towns recover.
- **Loot comes back per town over about 2 hours.** A looted town never drops under 0.2, so
  nobody gets locked out, but the best stuff takes time to return.
- **Hooks for the later world interactions** (all static on `RZ_TownMemory`):
  `GetLootLevel(pos)`, `ReportSearched(pos, share)`, `ReportBarricaded(pos, on)`,
  `ReportBuildingBurned(pos)`, `ReportBuildingCount(pos, count)`. A town that becomes Burned is a
  quiet headline.
- **No physical traces.** No graffiti and no extra bodies beyond what the
  game already cleans up. Your dead character's **journal** is data: a life that lasted 10
  minutes leaves one line (where, your background, how long, the day, a personal line). The last
  six are kept and saved; the newest turns up now and then in the session news (section 7), and
  wanderers bring one up in their talk ("Found a notebook near Morton..."). It's never an object.

## 9. New-player guidance ✅ (built, `Guidance/`)

The world teaches itself through things you find and people you hear, never tutorial pop-ups.
Every line is in a speaker's voice ([LORE.md](LORE.md), "Writing guide").

### Experience: guidance fades, but never disappears

Each player has a **guidance level** worked out from lives that lasted at least 10 minutes
(shorter lives don't count). It's saved on the player's profile.

| Level | Counted lives | What they get |
|---|---|---|
| 0, brand new | 0 | Everything: every tip in full, the gentle start, a wanderer who comes looking, scenarios leaning toward the Haven, the radio every 12 min |
| 1, learning | 1–2 | Every tip in full, full scenario scenes, wanderer gossip every time |
| 2, familiar | 3–7 | Every tip except the first-special one; one-line scenario scenes; gossip now and then |
| 3, veteran | 8+ | The floor: a scenario clue, the open channels (every 42 min), and critical tips (a bite, being Hunted) as one short line |

- **Refresher:** a player who comes back after 30+ real days gets two levels more guidance for
  their next two counted lives, and the once-only tips can come again.
- **Floor:** a veteran still gets a scenario clue, the open channels, and the short line for critical tips.

### Contextual tips

When something happens, the nearest plausible person says **one line** about it:

1. a faction leader within 40 m (their body, or the quartermaster they stand in for), in their
   faction's voice, unless they refuse you; or someone at a Haven safehouse whose people are
   shown, in the Haven's voice, unless you're exiled;
2. else a wanderer within 60 m (not the Kid, not one you robbed or shot at);
3. else a voice on the radio, if you carry one;
4. else a short plain hint.

The Haven suggests its medic; the Red Hand suggests a stolen antidote. Each tip comes at most
once per life, has a cooldown across lives, and waits 90 s after another tip (critical ones
don't wait). The table is data: **Guidance > Tips** on the game mode, with the defaults in
`RZ_GuidanceTips.c`. More can be added and fired with `RZ_Guidance.Report(playerId, id)`.

| Tip | Trigger | Says | Critical |
|---|---|---|---|
| bitten | the infection starts | antidote, or tell a safehouse medic | ✅ |
| special | a special dies within 80 m of you (once) | take the sample to the lab | |
| samples | samples in your pack (checked every 30 s) | where the lab is | |
| tier | your first rise to Known, Trusted or Champion with a faction (once) | the shop opens up | |
| watched | Watched by a faction | amends contracts | |
| hunted | Hunted or Kill on Sight | they shoot on sight; amends | ✅ |
| perimeter | within about 450 m of the edge, or 350 m of a checkpoint (once) | nobody gets through | |
| loud | heat 45+ where you stand | throw a noisemaker the other way | |
| night | the first night (once) | they hear further, specials come out | |
| safehouse | your first safehouse (once) | house rules: guns fine, no firing, declare bites | |
| shortage | in a safehouse that's short of food, or near a place whose supply truck didn't arrive | bring tins; the next delivery | |
| clue | your first puzzle clue (once) | "Notes like that usually point somewhere. Check your map and compass." | |
| channel | a Radio Operator near a faction's holding when it talks on the radio | a scrambled channel nearby | |
| hungry, car | **hooks only** (see below) | eat; a car that might run | |

**Hooks left for later systems.** Hunger comes from a Workshop mod ReforgerZ can't read and
there are no fixable cars in code yet. Each has its tip ready: the system (or another mod) calls
`RZ_Guidance.Report(playerId, "hungry" | "car", place)`. Supply trucks already report
"shortage" for shops whose truck didn't arrive (section 3), and faction life reports "channel"
to Radio Operators near a faction's holding; cracking secure channels is still to come.

### Gentle first 15 minutes

For the first 15 minutes of a brand-new player's life, zombie spawns near them run at half
density: towns within the spawner's activation radius of them and the wandering ones around them.

### First-life goal

A player's first life that counts always gets the personal goal **Find the Haven**: reach a
Haven safehouse (the nearest one that isn't where they woke). It pays like any other goal
(a Legacy point and a little Haven Fame). A life that ends before 10 minutes doesn't count,
so the next one gets the same goal.

### Where you wake: spawn scenarios

Every new life starts a few metres from a small clue, picked at random (brand-new players lean
toward the Haven, and never the same as last life if there's another). A one-line scene says
where you woke, in the nearest town. Each one points somewhere:

| Scenario | The clue | Points to | Needs |
|---|---|---|---|
| A back road | a car in the ditch; the dashboard note: "Gone to the safehouse in *town*. They're sharing food." with an arrow | the nearest Haven safehouse | a safehouse |
| A basement | scratched into the door: the roads out end in concrete, the line shoots, nobody gets out (with the direction of the nearest checkpoint or edge) | the Perimeter | the Perimeter on |
| Church notice | Halcyon's flyer, "Infected samples wanted. Reward.", on the nearest church door within 300 m if there is one | the lab | a lab |
| A farmhouse kitchen | a wind-up radio on the table plays the broadcast, then the Haven's open channel | the Haven | a safehouse |
| An attic | a dead raider with a red hand on his jacket and a list of toll roads | the Red Hand's boss | a Red Hand stronghold leader |
| Behind the barricade | a dead Bastion soldier's orders: "Hold the line. Await relief.", and his notes up to today: nobody is coming | Bastion's stronghold | a Bastion stronghold leader |

The clue is a prop that can be read: its own prefab if one is set and readable, otherwise the
puzzle note (or body) next to it. With no prefab at all, the clue's text comes with the scene.
Clues are removed after 20 minutes once nobody is within 30 m, and after 40 minutes anyway. A
veteran still gets a scenario, with a one-line scene: that's part of the floor.

*Simplification:* you don't really wake in a basement or an attic. The scene says so, and the
clue lies next to wherever the spawn point put you.

### The wind-up radio

Every new life gets a wind-up radio (a configurable item) in their inventory, or at their feet
if it doesn't fit. Radio Operators and anyone already carrying a radio don't get another. While
you carry one (`RZ_Radio.HasRadio`) you pick up the **open channels**: the Haven ("If you can
hear this, *town* is open. Bring water if you can. Bring yourself either way."), every other
time, and Bastion's net or the Red Hand's taunts in between. The government broadcast already
reaches everyone, so the radio only adds it when the server's broadcast loop is off. It's
low-frequency: about every 12 minutes for a brand-new player, every 18, 30 and 42 minutes as they
gain experience. Everything goes through the hint queue. Only open channels: secure channels are
for a later system (and the "channel" tip hook).

### Faction voices

- **First contact** with a leader is already in ([REPUTATION.md](REPUTATION.md), "Talking to
  leaders"): who they are and what they think of another faction, in their voice.
- **Bastion loudspeakers:** within 80–450 m of a Bastion seat you hear its loudspeaker: curfew,
  the Haven safehouse for shelter, armed groups on the roads.
- **Red Hand taunts:** near a Red Hand seat, someone on a roof asks whether Bastion is still
  waiting for relief, and says they pay faster than Halcyon. Each of these comes at most once
  every 30 minutes per player (an hour for veterans).
- **Halcyon flyers** hang in every Haven safehouse while its people are shown: samples wanted
  at the lab, declare bites at the Haven clinic, Bastion's curfew.
- **Wanderer gossip:** a wanderer's talk adds one line about a faction and where it sits,
  preferring one whose stronghold leader you haven't met. Always for players at level 0–1,
  about a third of the time after that.

### A wanderer finds you

Two to seven minutes into a brand-new player's life, the nearest Courier or Scavenger comes
looking for them (until 15 minutes in; if none is free, it tries again every 2 minutes). One
more than 600 m away whom nobody can see is moved, as data, to 600 m first, so they arrive in
minutes. When their body appears within sight, the player hears that someone on the road is
waving at them; they wait 5 minutes. Their talk includes faction gossip.

## Build order

1. ✅ Safehouses with population tiers, and rescued survivors joining them
2. ✅ The wanderer roster with simulated travel, reactions and memory
3. ✅ Noisemakers, the "being noticed" check, and the unarmed missions
4. ✅ Named leaders at strongholds, safehouses and outposts
5. ✅ Strongholds and outposts, the territory tick, real supply trucks, faction life
6. ✅ Light infection adaptation; day/night and weather modifiers, the power grid and the radio station
7. ✅ Story arcs and session headlines
8. ✅ New-player guidance: experience levels, contextual tips, the gentle start, the first-life goal,
   spawn scenarios, the wind-up radio, faction voices, a wanderer who finds new players
