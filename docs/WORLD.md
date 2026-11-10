# Living world

How the world changes on its own, without becoming overpowered or lagging
the server. Three rules apply throughout:

- **Simulate as data, spawn as bodies only near players.** Population,
  wanderers, territory and convoys are numbers on the server. AI entities
  exist only within a few hundred metres of a player.
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
  *Simplification:* there are no supply trucks yet, so food only comes from players and the
  starting stock. Bastion's Requisition contract takes 4 food when the supplies go to Bastion.
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
- "Their faction loses an outpost" isn't in: there's no territory yet (section 3).

**Planned:** barter chains across three wanderers; Haven Champion "Community" respawns with a supply bag.

### Leaders ✅ (built, `Factions/RZ_Leaders.c`)

A named person runs every place a faction holds. What they do for and against you is in
[REPUTATION.md](REPUTATION.md), "Faction leaders"; their personal jobs are in
[MISSIONS.md](MISSIONS.md), "Leader missions". Here is where they are and how they're simulated.

| Seat | Built now | Planned |
|---|---|---|
| Stronghold (one per faction) | At the faction's quartermaster; Halcyon's at the lab. Without a quartermaster: the Haven's at its first safehouse, the others in a town that suits them | Same, at the real stronghold |
| Haven safehouse | One council head per safehouse | — |
| Outpost | **Interim leader posts**: 1 per faction for Bastion, the Red Hand and Halcyon (a "field station", never more than one), picked once in a town that suits the faction (bigger towns for Bastion, rougher ones for the Red Hand, quiet ones for Halcyon), saved. Mission makers can place `RZ_OutpostMarkerEntity` instead | Territory (section 3) calls `RZ_Leaders.RegisterOutpost(faction, pos, name)` for each outpost and `UnregisterOutpost(id, why)` when it's lost; the interim posts are turned off then |

On Arland that's about 8–10 leaders (4 strongholds, 1–2 council heads, 3 posts); with
territory it would be 12–20.

- **Data first.** A leader is a saved record (name, title, backstory, traits, loyalty,
  memories). A body exists only while a player is within 200 m of their seat. At a
  stronghold or safehouse, the quartermaster there *is* the leader unless a leader prefab
  is set (it gets `RZ_LeaderAction`s, and its puzzle components are labelled if it has them).
  Outpost leaders without a prefab use the puzzle civilian.
- **Stronghold leaders** never die and are never replaced; their bodies take no damage.
- **Others die off-screen** (bites, raids, Bastion acting on a tip) or if a player kills them,
  and are replaced two hours later by someone new. Deaths, replacements and big loyalty
  changes are headlines ("Word on the roads"); the last three are shown on a player's first spawn.
- **Personal jobs** come up "occasionally": every two minutes, an Independent or Rogue leader
  with a player near them and no job on has a 35% chance to think of one (at most one per
  30 minutes per leader), and asking them for work tries straight away. They're offered only
  in person.

**Simplifications:** interim posts are leader seats only (no camp, guards or shop of their own
yet). A leader's "people" who can see a rogue job are their faction's other seats (within 60 m)
and, for the Haven, its wanderers. Off-screen deaths only happen with nobody within 400 m.

## 2. Unarmed missions: low level, but still a challenge

For new characters, Unarmed backgrounds (Nurse, Lab Tech, Unemployed), or
anyone who doesn't want a gunfight. These are about **stealth, noise and
thinking**, and they pay small amounts of Fame with whoever offers them.

Two small mechanics make them work:
- **Noisemakers.** A thrown bottle or firecracker, an alarm clock, a car alarm. Each one adds heat at a spot, which pulls zombies toward it.
- **Being noticed.** A mission can fail if heat near you passes a threshold or a zombie stays close for too long.

| # | Mission | From | Challenge |
|---|---|---|---|
| 1 | **Pharmacy Lure** | Haven | Infected fill a pharmacy. Use noisemakers to draw them out, then sneak in for the medicine |
| 2 | **Silent Courier** | Courier | Carry a letter through an infested town without raising heat over a limit |
| 3 | **The Safe Code** | Scavenger | A safe needs a 4-digit code. Clues are on notes in 3 houses; wrong guesses set off an alarm |
| 4 | **Breaker Sequence** | Bastion | Restore power to a building by flipping breakers in the right order. The wrong order sets off an alarm |
| 5 | **Lead Them Away** | Haven | A horde is drifting toward a safehouse. Use noise to lead it to a marked quarry without getting caught |
| 6 | **Rooftop Rescue** | Haven | A survivor is stuck on a roof. Find a ladder route and time it between zombie waves |
| 7 | **Trail of Clues** | Wanderer | Follow belongings and notes from a missing person's house to wherever they ended up |
| 8 | **Car Keys** | Mechanic wanderer | A working car, but the keys are in one of 5 houses. Search quietly, drive it back |
| 9 | **Scout the Town** | Bastion | Visit 4 observation points in town and stay unnoticed at each for 20 s |
| 10 | **Night Lantern** | Halcyon | Reach a high point at night with no light and switch on a beacon |
| 11 | **Water Run** | Haven | Carry heavy water containers back. Choose the slow safe road or the fast risky one |
| 12 | **Barter Chain** | Wanderers | Trade item to item across three wanderers to get what the first one wanted |
| 13 | **Distraction** | Anyone | Keep the zombies in one area busy with noise for 3 minutes while another player loots |
| 14 | **Medicine Before Dark** | Doctor wanderer | Bring a specific medicine to a bitten wanderer before nightfall |
| 15 | **Quiet Count** | Halcyon | Count the specials in a town without getting close. Answer correctly for the reward |

The higher-tier version of a contract can be armed; the entry-level version
doesn't need to be.

## 3. Factions in the background (strongholds never fall)

- **Most of the map is abandoned.** At most around 20% of towns are held by
  any faction at all. The rest are empty, looted or infested, which is where
  the scavenging, missions and danger are. A faction holding a town is the exception.
  - On a small map like Arland: each faction has its Stronghold and 1 Outpost.
  - On a large map like Everon: a Stronghold and 2–3 Outposts each.
  - Factions only claim towns that make sense for them. Bastion takes towns
    near roads and airfields, the Haven takes quiet villages, the Red Hand takes crossroads and outskirts,
    and Halcyon only ever holds the lab town plus at most one field station.
  - Haven safehouses don't count as holding a town: they're a single building in an otherwise abandoned place.
  - **Factions abandon places too.** An outpost that sees little traffic
    for long enough is dropped and goes back to being abandoned.
- Each faction has **one Stronghold** that **cannot be captured or destroyed**.
  At worst it can be **Under Pressure** for a while: a few services pause,
  then it recovers on its own. It's always a safe place for that faction's friends.
- Each faction also holds a few **Outposts** in other towns. Every
  20–30 minutes a quiet "territory tick" may do **one** thing, based on faction strength:
  set up a new outpost in a nearby *abandoned* town (only if under the cap),
  take one from a neighbouring faction, or abandon a weak one.
  - **Floor:** a faction always keeps its Stronghold and at least one Outpost.
  - **Recovery:** a lost outpost tends to swing back after a while if the faction is still strong.
  - Changes go into the **headlines** only. No server-wide alerts.
- Every outpost has a named leader (section 1, "Leaders"): the territory tick registers
    and unregisters them through `RZ_Leaders.RegisterOutpost` / `UnregisterOutpost`.
- **Faction strength** moves a little with contracts done for or against the faction, and drifts back toward the middle over time.

### Supply lines

- **A real truck does the resupplying.** Each faction's outposts and safehouses
  only restock when an actual supply truck, a real vehicle with an AI driver,
  drives in from the Stronghold and parks. No truck, no restock.
  - Each faction has at most one truck on the road at a time, so the AI cost stays small.
  - A truck runs every 40–60 minutes, at a random time within that window so it feels real.
  - If a truck gets stuck and no player is within 1 km, it's quietly moved past the obstacle.
    If a player is nearby, it stays stuck, and that's a chance to help or rob it.
- **Shortages.** If a truck is destroyed, robbed or never arrives, that faction's shops in those towns:
  - run out of some items,
  - **raise prices** (+25% to +50%),
  - and may **lock their top tier** until the next truck gets through.
  - The shortage lasts until the next successful delivery: **40–60 minutes**, random each time.
- **Players can drive the truck.** Escort it for Fame. Drive it in yourself if the driver is dead, for more Fame.
  Or steal it and deliver it to a rival's stronghold. That's Red Hand Fame, and the owner's Infamy.
- Haven trucks also carry the food that keeps safehouse population up (planned: they add to
  the safehouse food stock that players fill by hand today).

## 4. Infection adapts, a little

Small, capped changes that keep late game interesting without making it a wall:

- With each cure breakthrough, specials become slightly more common: **+5% per breakthrough, +15% at most**.
- **One new trait per outbreak stage**, never more. For example, Collapse brings slightly faster runners,
  Overrun gives some zombies a helmet, and Aftermath adds the occasional screamer.
- **Favourite spots.** If players keep returning to the same area, a small horde
  checks it out now and then (at most once an hour per area). It's a nudge, not a punishment.

## 5. Time and environment

- **Day and night, no time skip.** At night zombies hear further and specials appear, while
  human AI patrols less. It's a good time for stealth and unarmed missions.
- **Weather:** rain dampens noise, fog limits sight lines for everyone, and storms can
  ground Bastion's helicopter.
- **Power grid:** a group can restore a power station. The region gets lights, working radios
  and electronics loot, but the noise draws hordes there. It slowly fails unless someone maintains it.
- **Radio station:** players can take it over to broadcast news or warnings. Cosmetic and social only: no Fame bonus.

## 6. Player-to-player (toned down)

- **Notice board** at each safehouse: short text notes left between sessions. Harmless and social.
- **Mission locations:** contracts give a rough area; the exact spot appears once
  you're within 500 m. Radio Operators and Trusted players see it from the start. No other perks.
- No propaganda bonuses, nothing that gives Fame for free.

## 7. Story director: arcs

The Director strings events into short storylines. Each step only fires if
the previous one happened, so every server lives through different plots.
Examples:

1. **The Missing Courier.** A Halcyon courier stops reporting. A wanderer has seen
   Red Hand men with lab cases. A Red Hand camp suddenly sells antidotes. The arc ends with a
   lab defence, or a contract to steal the antidotes back.
2. **The Last Convoy.** Bastion announces a big convoy pulling an outpost's people and stock back to its stronghold. Raiders scout it.
   Players escort it, raid it, or stay out of it. Either way, a Bastion outpost is restocked or runs dry.
3. **The Preacher's Flock.** The Preacher rings bells in more and more towns. Hordes
   follow him. Find and stop him, protect him, or use his bells as a distraction.
4. **The Deserter's Map.** A dying deserter hands out a map piece. Two more pieces are
   held by wanderers. Together they lead to a hidden Bastion bunker.
5. **Fever Town.** A safehouse's infection creeps up. A Halcyon contract checks the water,
   a Haven contract evacuates the vulnerable, and a Red Hand rumour says someone poisoned it.

**Headlines:** every session (and on join) players get 3–4 lines of server news:
outposts that changed hands, wanderers lost, the cure percentage, the biggest rescue.

## 8. Town memory (data only, no lag)

- **Town flags:** Looted, Barricaded, Powered, Burned. These are plain values, not objects in the world.
- **Loot comes back per town over about 2 hours.** A looted town still has some loot,
  so nobody gets locked out, but the best stuff takes time to return.
- **No physical traces.** No graffiti and no extra bodies beyond what the
  game already cleans up. Your dead character's **journal** is data. It
  shows up as a line in the headlines or as a rumour from a wanderer, not as an object.

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
| shortage | in a safehouse that's short of food | bring tins | |
| clue | your first puzzle clue (once) | "Notes like that usually point somewhere. Check your map and compass." | |
| hungry, car, channel | **hooks only** (see below) | eat; a car that might run; a scrambled channel nearby | |

**Hooks left for later systems.** Hunger comes from a Workshop mod ReforgerZ can't read, there
are no fixable cars or secure radio channels in code yet, and shop shortages wait for supply
trucks. Each has its tip ready: the system (or another mod) calls
`RZ_Guidance.Report(playerId, "hungry" | "car" | "channel" | "shortage", place)`.

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
3. Noisemakers, the "being noticed" check, and the first 6 unarmed missions
4. ✅ Named leaders at strongholds, safehouses and interim leader posts
5. Strongholds and outposts, the territory tick, supply lines as data
6. Light infection adaptation; day/night and weather modifiers
7. Story arcs and headlines (leader headlines are already in)
8. ✅ New-player guidance: experience levels, contextual tips, the gentle start, the first-life goal,
   spawn scenarios, the wind-up radio, faction voices, a wanderer who finds new players
