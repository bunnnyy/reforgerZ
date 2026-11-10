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

### Rescued survivors and safehouse population (no crowding)

A rescued survivor isn't kept as an AI body. They become **+1 population**
at the nearest Haven safehouse.

- **Few bodies on screen.** A safehouse shows at most 3–6 AI occupants, and only while a
  player is within 300 m. A safehouse of 4 and one of 40 look about the same; the difference is in
  what they offer.
- **Population tiers** (diminishing returns, hard cap of 30 per safehouse):

  | Population | Safehouse becomes |
  |---|---|
  | 0–4 | Shelter: a spawn point, basic Haven shop |
  | 5–11 | Community: more shop stock, one guard on watch |
  | 12–19 | Settlement: a medic (heals for Favor), a radio (intel on missions nearby) |
  | 20–30 | Town: Haven Champion shop items stocked here, two guards |

- **Overflow** moves on its own: past the cap, new arrivals go to the next safehouse.
- **Upkeep.** Every few hours population needs food. If the Haven supply line
  to a safehouse is cut (see Supply lines), a few people leave, either to another
  safehouse or to become wanderers. The safehouse shrinks but never drops below Shelter.
- **Some rescued people stay on as characters.** About 1 in 10 becomes a named
  wanderer, so the people you save turn up on the roads later.

### Wanderers

A small roster of named survivors travels the map: 6–10 per server, scaled
by map size. Each one has a role, a route and opinions.

| Role | What they do | Typical reaction to you |
|---|---|---|
| Trader | Moves goods between safehouses; best prices | Trades if you're not Hunted by the Haven |
| Doctor | Treats people; can slow an infection for Favor | Helps the injured; refuses the Red Hand's Trusted |
| Scavenger | Knows where loot is; sells rumours | Swaps a rumour for food or ammo |
| Deserter | Ex-Bastion, paranoid | Friendly if Bastion hates you, hostile if Bastion loves you |
| Preacher | Wanders between churches, rings bells | Harmless, but draws zombies; keeps the world "weird" |
| Courier | Carries messages between factions | Hands out unarmed delivery jobs |
| Kid | A lost child heading for a safehouse | Follows you if you're kind; flees from gunfire |

**How they behave.** Each wanderer has a simple state machine:
travel → rest at a safehouse → trade or talk → move on.

**They react to the world:**
- If their destination falls to infection, they change route.
- If a horde is nearby, they flee or hide in a building.
- During Night of the Dead or a storm, they shelter until it ends.
- If their faction loses an outpost, they avoid that area.
- They can get bitten. A bitten wanderer can trigger a "Bitten Friend"-style event.

**They react to you.** Their attitude toward you is your reputation with their faction, plus their own
**memory of you** (stored per player, per wanderer):
- *Helped me* (escorted, healed, traded fairly) → better prices, rumours, warnings ("there's a horde past the bridge")
- *Robbed or shot at me* → refuses you, warns others: Haven safehouses hear about it (+ Haven Infamy)
- *Saved my life* → gives a one-off gift and remembers you for the rest of the season

**They tie into everything else:**
- **Unarmed missions** (below) come mostly from wanderers.
- **Barter chains:** the Scavenger wants a radio, the Courier has one but wants medicine,
  and the Doctor has medicine but wants a fuel can. A small puzzle across the map.
- **Death is permanent and makes the news.** "Old Marta the trader was killed on the coast road."
  A replacement shows up a day later, so the roster refreshes.

**Performance.** When no player is within 400 m, a wanderer is just a position moving along its route on the
server, with no entity. Only the 1–2 wanderers near each player exist as AI.

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
- **Faction strength** moves a little with contracts done for or against the faction, and drifts back toward the middle over time.

### Supply lines

- Factions run AI convoys between their Stronghold and Outposts, as data until a player is near.
- **Raiding a convoy** lowers that faction's shop **stock** in the towns it was heading for,
  temporarily: some items run out for about 30 minutes. Prices don't spike and tiers don't lock.
- **Escorting a convoy** restocks those shops sooner and gives Fame.
- Haven convoys also carry the food that keeps safehouse population up.

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
- **Weather:** rain dampens noise, fog limits sight lines for everyone, and storms can down
  helicopters, which spawns a crash-site event.
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
2. **The Last Convoy.** Bastion announces a big evacuation convoy. Raiders scout it.
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

## Build order

1. Safehouses with population tiers, and rescued survivors joining them
2. The wanderer roster with simulated travel, reactions and memory
3. Noisemakers, the "being noticed" check, and the first 6 unarmed missions
4. Strongholds and outposts, the territory tick, supply lines as data
5. Light infection adaptation; day/night and weather modifiers
6. Story arcs and headlines
