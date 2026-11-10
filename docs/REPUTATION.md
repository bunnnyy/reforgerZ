# Reputation, factions and unlocks

Inspired by how New Vegas handles reputation: every faction remembers two
separate things about you, **Fame** (what you did for them) and **Infamy**
(what you did to them). Your standing comes from both, so you can be a hero
to the Haven and a nightmare to the Red Hand, or loved and feared by the same
faction at once.

Missions are the main source. Most missions help one faction at the cost of
another, and some make you **choose who gets the prize**.

## The four factions

| Faction | Who they are | Wants | Rival of |
|---|---|---|---|
| **The Haven** (Survivors) | A network of civilian safehouses | Food, medicine, safe towns, people | Red Hand. Uneasy with Bastion's martial law |
| **Task Force Bastion** (Military) | What's left of the army, holding checkpoints | Order, control, the cure as a strategic asset | Red Hand. Distrusts Halcyon |
| **The Red Hand** (Bandits) | Raiders and opportunists who think the old rules are dead | Loot, territory, fear | Everyone, Bastion most of all |
| **Halcyon** (Institute) | Scientists running the cure lab | Samples, data, protection, the cure going public | Wary of Bastion seizing it |

(The code still uses the enum names SURVIVORS / MILITARY / BANDITS / INSTITUTE.
Only the display names change.)

## Fame and Infamy

Each faction tracks **Fame 0–100** and **Infamy 0–100** for your current life.

- Helping a faction gives Fame with it, and usually Infamy with its rival.
- Hurting a faction (killing its people, stealing, sabotage, siding against it in a choice mission) gives Infamy.
- Neither value goes down on its own. Infamy can be worked off with **amends**: specific missions or a payment of Favor that convert Infamy into nothing.

### Standing tiers

| Fame ↓ / Infamy → | Low (< 15) | Some (15–39) | High (40–74) | Extreme (75+) |
|---|---|---|---|---|
| **Low (< 15)** | Stranger | Watched | Hunted | Kill on Sight |
| **Some (15–39)** | Known | Uneasy | Hunted | Kill on Sight |
| **High (40–74)** | Trusted | Tolerated | Wildcard | Hunted |
| **Extreme (75+)** | **Champion** | Respected | Wildcard | Wildcard |

- **Positive tiers:** Known → Trusted → Champion. They unlock shops, perks, gear and missions.
- **Mixed tiers:** Uneasy, Tolerated, Respected, Wildcard. They give partial access at worse prices, and the faction keeps an eye on you.
- **Negative tiers:** Watched → Hunted → Kill on Sight. The faction comes after you.

### Favor (currency)

Every faction pays in its own **Favor**, which you earn from its missions and
spend at its quartermaster. Reputation decides **what** you can buy. Favor is
**how much** you can buy. Favor is per life, like reputation.

### Lives and carry-over

Reputation is per character, so a new life starts fresh with only your
background's starting standing. One exception, so deaths don't feel like
total wipes:

- **Word gets around (Legacy):** if any of your past characters reached
  *Champion* with a faction, every new character starts at *Known* with it.
- Reaching Champion also **unlocks a new background** for future lives (see below).

## What each faction gives you

### The Haven

| Tier | Shop (Haven quartermaster) | Perk / service |
|---|---|---|
| Known | Canned food, water, bandages, crowbars | Haven safehouses let you in; fresh spawn point option at a Haven safehouse |
| Trusted | Hunting rifles, ammo, backpacks, radios, jerrycans | **Neighbours:** Haven AI shares intel; nearest active mission is marked on spawn |
| Champion | Vehicles (pickup, van), fortification kits, good medkits | **Community:** respawn at any Haven safehouse with a small supply bag |
| Hunted | — | Haven doors are barred, and their AI warns other players about you |

### Task Force Bastion

| Tier | Shop (Bastion requisition) | Perk / service |
|---|---|---|
| Known | Military rations, morphine, flares | Checkpoints let you through without being searched |
| Trusted | Service rifles, body armour, helmets, NVGs | **Requisition:** call in a military supply crate once per hour |
| Champion | Machine guns, grenades, military trucks | **Fire mission:** mark a horde with smoke for an artillery strike (long cooldown) |
| Hunted | — | Checkpoints open fire; patrols get your last known position |

### The Red Hand

| Tier | Shop (black market) | Perk / service |
|---|---|---|
| Known | Knives, pistols, lockpick-style tools | Red Hand roadblocks let you pass for a toll instead of shooting |
| Trusted | Suppressors, scoped rifles, explosives | **Fence:** sell loot for Favor; Red Hand ambush crews ignore you |
| Champion | Anything stolen (rare military gear, stolen antidotes) | **Bounty board:** post bounties on other players |
| Hunted | — | Ambush crews spawn on roads near you |

### Halcyon

| Tier | Shop (lab stores) | Perk / service |
|---|---|---|
| Known | Antiseptic, painkillers, sample containers | You can buy antidotes when the lab has stock |
| Trusted | Antidotes (when produced), stim kits, gas masks | **Sample scanner:** special infected within 300 m are revealed |
| Champion | Experimental immunity booster (shrugs off one bite) | **Lab access:** your deliveries count 25% more; you can call in a Halcyon medic |
| Hunted | — | The lab refuses your samples: they're taken without reward, and its guards turn hostile |

## Making it hurt: rivalries and choices

- **Rival penalties.** Fame with one faction often gives Infamy with its rival.
  Clear a Red Hand camp for Bastion and the Red Hand remembers.
- **Choice missions.** The objective is an item (a black box, data drive,
  stolen antidotes, a prisoner). You decide whose drop point gets it. The winner gives Fame;
  the faction you double-crossed gives Infamy.
- **Disguises.** Wearing a faction's uniform lets you pass its checkpoints
  while Watched or Uneasy. Get caught (shoot, or get too close for too long) and the
  Infamy you gain doubles.
- **Bounties.** Kill on Sight with a faction puts a public bounty on you.
  Other players see a rough location and can claim it for that faction's Favor and Fame.
- **Amends.** Each faction has missions that work Infamy off, so a
  mistake doesn't lock you out for the rest of the life.

## The season's ending is decided by players

When the cure reaches 100% and the final stand is won, the cure goes to
whichever faction the server **contributed the most Fame to this season**.
That sets the tone of the Aftermath and the next season's starting rules:

| Cure goes to | Aftermath | Next season starts with |
|---|---|---|
| Haven | The cure spreads person to person. Towns rebuild. | More safe towns, more Haven safehouses |
| Bastion | Martial law. The cure only reaches those who comply. | Strong checkpoints, curfew, more military loot |
| Red Hand | The cure is sold to the highest bidder. | Lawless roads, the black market everywhere |
| Halcyon | The cure is published openly. | Lower infection everywhere, more specials (they adapt) |

## Unlockable backgrounds (Legacy)

| Background | Unlock | Perk / flaw |
|---|---|---|
| Soldier | Champion with Bastion in any life | Service rifle and armour; Red Hand starts at Watched |
| Doctor | Champion with Halcyon | Can treat bites slowly without an antidote; unarmed |
| Raider | Champion with the Red Hand | Black market from the start; Haven and Bastion start at Watched |
| Elder | Champion with the Haven | Starts at Trusted with the Haven; old and slow (more noise) |
| Drifter | 10 Legacy points | Starts Known with every faction; no gear at all |

## How it maps to code

- `RZ_PlayerProfile`: Fame/Infamy/Favor arrays per faction, plus a persistent `m_aChampionOf` set
- `RZ_Reputation`: tier calculation, rival table, display names
- `RZ_FactionShopAction`: an action on a quartermaster prefab with faction, required tier, Favor cost and item prefab.
  Shop System can still be used for general trading. Faction shops stay in our own code, so tiers are enforced.
- Missions: `m_eGiver` gets Fame, an optional `m_eRival` gets Infamy, `m_iFavorReward` pays out
- Perks are checked in code where they apply (scanner, requisition, toll pass...)
- AI hostility: Reforger factions are global, so per-player hostility is done by
  sending hunter squads or checkpoint attack waypoints at Hunted players, not by
  changing faction relations
