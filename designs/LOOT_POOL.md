# The loot pool: what to learn from DayZ

**Designed, not built.** Approved by the author on 2026-11-15. It waits with the other designs until
the stages are in ([FEATURES.md](../docs/FEATURES.md)).

Our loot compared with DayZ Standalone and the DayZ mod for Arma 2, and what we take from them.
How our loot works today is in [DESIGN.md](../docs/DESIGN.md#loot-in-houses) and what the mods add in
[MOD_STACK.md](../docs/MOD_STACK.md#what-the-stack-gives-the-loot); this file never restates it, only
what would change.

Sources: DayZ Standalone's own loot file for Chernarus (`types.xml` in Bohemia's
[DayZ-Central-Economy](https://github.com/BohemiaInteractive/DayZ-Central-Economy), current), and
the DayZ mod's loot tables (`CfgLoot` in [DayZMod/DayZ](https://github.com/DayZMod/DayZ), its last
version, a few years old).

## The three side by side

| | DayZ Standalone | DayZ mod (Arma 2) | ReforgerZ today |
|---|---|---|---|
| **Building kinds** | 17 tags: Town, Village, Farm, Industrial, Hunting, Military, Police, Medic, Firefighter, School, Office, Coast, Prison, Lunapark, Historical, Contaminated area, Seasonal | About 15 classes: Residential, Farm, Industrial, Supermarket, Hospital, Hunting, Military, Military special, Office, Church, Castle, Boat, Construction, Powerlines, Toilet, plus crash sites | 6 kinds from the prefab name: house, shop, farm, pharmacy, garage (fire stations too), police; plus bodies and wrecks' boots |
| **Items** | About 1,100 that spawn; a town alone can hold 361 kinds, most of them clothes | A few hundred, in shared groups (food, trash, ammo, medical, parts) | About 250 across all tables (house 40 common / 13 rare, the smallest 13/4) |
| **How rare is rare** | A map-wide count per item (`nominal`), refilled when it drops under `min`: 100 9 mm rounds, 40 dressings, 6 AK74s, 3 AKMs, 10 NVGs | A weight in each building's list: a Makarov 4, an M4A1 1, an L115A3 0.1 | Per slot: 10% of finds rare (20% behind a lock), 10% electronics; every item in a list equally likely; no map-wide count |
| **Tiers or zones** | Tier 1 on the coast to Tier 4 in the north-west; an item lists the tiers it spawns in | None; the building class is the tier | None; only the town's loot level (how picked-over it is) and powered towns' electronics |
| **Rare places** | Military bases (Tier 4), contaminated zones (FAL, M4, suppressors, the best optics), heli crashes | Military special buildings, heli crashes (four kinds) | None. The best finds are a locked police station or pharmacy |
| **Refill** | Each item's own restock timer and lifetime on the ground | Per building, on a timer | Per building: 45 min ÷ the town's loot level, never with a player within 20 m |
| **Where loot lies** | Proxies in each building model (floor, shelves, ground) | Building loot positions | Furniture tops and floor points we find ourselves |

## What we take

All of it fits the guardrails: no outside aid, nothing overpowered, no free loot, data until a
player is near. Items are only ones the stack already has (the base game and the Workshop mods in
[MOD_STACK.md](../docs/MOD_STACK.md)), except the seeds, which are ours to make.

### 1. More building kinds

Found from the prefab name like today's six. Each gets its own table and slot count.

| Kind | Name words (to check on Everon) | Why |
|---|---|---|
| **School** | school, kindergarten | DayZ's school set: snacks, a bat, notebooks, children's bags. Cheap, everyday, and towns read truer |
| **Office** (and town hall) | office, townhall, post, bank | Maps, a writing pad, watches, cash: what the fence buys, things to read |
| **Church** | church, chapel | Very little: a little food, water, a note left for someone. Mostly a place for traces ([TRACES.md](../docs/TRACES.md)) |
| **Fire station** (split from garage) | firestation, fire_station | The fire axe, fireman's helmet, firefighter suit and gas mask move here from the garage's rare list, so they have one home |
| **Medical, low and high** | Low: pharmacy, drugstore, clinic. High: hospital | DayZ and the mod both keep the good medicine in hospitals. A pharmacy keeps dressings, aspirin and gloves; a hospital adds saline, morphine, first-aid kits and the rare treatments |
| **Industrial** | factory, warehouse, depot, sawmill, hangar (civil) | Wire, nails, scrap, jerrycans, a fuse, tools: what the bench needs, now mostly a garage's |
| **Hunting** (stands and lodges) | hunting, huntinglodge, hunterstand, lodge | Knife, binoculars, compass, map, boonie hat, raincoat; rare: a shotgun or hunting rifle where the stack has one |
| **Coast** (piers' sheds, boathouses) | boathouse, fisher, pier_shed | Raincoats, rope, a dry bag where a mod has one. Small: fishing has no items in the stack |
| **Prison** | prison, jail | The prisoner's uniform and very little else. Only if the map has one |
| **Army post** (abandoned) | barrack, military, bunker, guardhouse, radar | See 3. Never a place a faction holds (kept clear as today) |

### 2. Medicine in two levels

The mod's MedicalLow and MedicalHigh: a pharmacy, a body and a house draw from **low** (dressings,
gauze, aspirin, tourniquets); only a **hospital** and a locked pharmacy draw from **high** (morphine,
saline, first-aid kits, the treatments). Makes the hospital worth the walk and the risk.

### 3. Rare places: the Zone's own crash sites

No helicopters drop in (the Zone is sealed), but the line's own things come down:

| Rare place | Where | What it has | Why it's there |
|---|---|---|---|
| **A downed Perimeter drone** | Inside the Zone, within 1 km of the line, at most 2 on the map at once, a new one every few hours | Its battery (torch cells), wire, scrap, a fuse; rare: the drone's camera (sells to the fence), a radio | Drones fail; the line doesn't come in for them. The contracts' crashed drone prefab is the look |
| **An abandoned army post** | Barracks, bunkers and guardhouses no faction holds | Magazines, military clothes, helmets, vests, MREs, canteens, smoke; rare: one rifle | Left when the line went up. Each rifle counts against a map-wide cap (4) |
| **A wreck's boot on the old evacuation road** | Wrecks within 500 m of a checkpoint | Better boot loot: a backpack, food for days, a first-aid kit | The last people who tried to drive out |

### 4. Map-wide caps for the rare things only

DayZ's counts across the map, but only for what must stay rare: rifles (4 lying in the world at
once), pistols (8), vests and helmets (6), the treatments (as today, plus a cap of 5). Counted as data
(rolled, lying, or in a building's kept list), so a rifle nobody picks up still holds its place.
Everyday things stay as now: a slot chance and the town's loot level.

### 5. Shared groups

The mod's best idea for us: a few named groups (Food, Drink, Medical low, Medical high, Tools,
Clothes everyday, Fence goods, Electronics), and each building kind is a mix of groups with weights
instead of its own long list. One list to change when a mod adds food. It replaces the long
per-kind lists and the "every item equally likely" rule with weights.

### 6. Zones by distance from the line

The one tier we add, and it fits the story: the nearer the Perimeter, the less was taken (people
fled inward, away from the snipers). Within 1 km of the line rare finds are ×1.5; past the middle of
the Zone ×1. The danger pays for it: the drones and the loudspeakers are there.

### 7. Seeds and vegetables for farming

When farming is built ([FARMING.md](FARMING.md)): a farm's and a shop's table gets seed packets, and
a farm's gets the vegetables themselves. Reforger and the stack have **no seed items** and no
tomatoes, cucumbers, bell peppers or pumpkins: four seed packets and four vegetables are ours to
make (eight small prefabs), as the crafting items were. DayZ's Farm loot has the same set
(tomato, pepper, pumpkin, zucchini seeds, about 30 each on the map), which is a good sign.

## After the change: our kinds and what they hold

| Kind | Everyday | Rare (capped where marked) |
|---|---|---|
| House | Food, drink, medical low, clothes, wood, someone's belongings, a tin, salt | A knife, binoculars, a compass, a backpack |
| Shop | Food, drink, vodka, cigarettes, watches, maps, gloves, hats; seed packets (with farming) | Binoculars, a compass, a hiking backpack |
| Farm | Food, raw meat, water, wood, work clothes, a spade, jerrycans; seeds and vegetables (with farming) | A fire axe, a machete, binoculars |
| School | Snacks, water, a bat (where a mod has one), a writing pad, children's things | — |
| Office | Maps, writing pads, watches, cash, credit cards | A gold bar, a radio |
| Church | A little food and water | A note left for someone |
| Pharmacy | Medical low, gloves, face masks | A medical bag, a gas mask |
| Hospital | Medical low and high, saline, gloves | Morphine, first-aid kits, the treatments (capped), a hazmat suit |
| Garage | Jerrycans, the wrench kit, torches, work gloves, overalls, wire, nails | A fuse, an oxygen tank, a backpack |
| Fire station | Work gloves, overalls, torches | A fire axe, a fireman's helmet, a firefighter suit, a gas mask |
| Industrial | Wire, nails, scrap, jerrycans, planks of wood, work clothes | A fuse, a hard hat, tools |
| Hunting | Knife, binoculars, compass, map, raincoat, boonie hat | A hunting rifle (capped) |
| Coast | Raincoats, rope, food | A dry bag |
| Police | Magazines, torches, dressings, caps, a holster | A pistol (capped), a Vz. 58 (capped), smoke, a vest, a helmet, a radio |
| Prison | The prisoner's uniform, food | — |
| Army post | Magazines, military clothes, MREs, canteens, smoke | A rifle (capped), a helmet, a vest (capped) |
| Downed drone | Torch cells, wire, scrap, a fuse | The drone's camera, a radio |
| Body, wreck's boot | As today; the evacuation road's boots a little better | As today |

## What we don't copy

| DayZ thing | Why not |
|---|---|
| 260 kinds of town clothing in colours | Weight without play: our clothes do something (gloves lower the bite chance, colours count for disguise), and a long list makes food rare again |
| Blood bags and blood types, the blood tester | No such system here and none planned: vanilla's saline does the job |
| Military bases full of rifles, grenades, mines, NVGs, Tier 4 | Nothing overpowered: Bastion holds the military places, NVGs and machine guns are its Trusted and Champion shop items, and a free rifle a run would undo the faction shops |
| Contaminated zones' best gear | Our gas is the infection; the best gear comes from factions, not a zone |
| Helicopter crashes | The Zone is sealed: nothing flies in. The downed drone takes their place |
| Branded canned food (dozens of soda and can brands) | Only items the stack has. Metabolism's and the base game's food stays the set |
| Fishing (rod, hook, net) and animal traps as loot | No fishing in the stack. Only if a fishing system is ever designed |
| Lunapark, Historical (swords, chainmail), seasonal masks | Jokes that don't fit the Zone's tone ([WRITING.md](../docs/WRITING.md)) |
| Each item's own lifetime and restock timer | Our per-building refill and the town's loot level do this as data, more cheaply |

**DayZ items with no equivalent in our stack:** seeds, vegetables, blood bags, the blood tester,
fishing gear, NVGs as loot (the base game's are faction kit), branded food and drink, sewing kits,
a can opener, a hoe (the farming design uses a shovel), the alarm clocks and flags.

## Order of work

1. Shared groups and weights (5), and medicine in two levels (2): the base everything else uses.
2. The new building kinds (1), checked on Everon by their names; the fire station split.
3. Map-wide caps for the rare things (4).
4. Rare places: the army posts first, then the downed drones and the evacuation road's boots (3).
5. Zones by distance from the line (6).
6. Seeds and vegetables, in the same commit as farming's first step (7).
