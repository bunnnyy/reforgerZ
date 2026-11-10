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
| Known | Canned food, water, bandages, crowbars | Haven safehouses let you in and serve you |
| Trusted | Hunting rifles, ammo, backpacks, radios, jerrycans | **Neighbours:** Haven AI shares intel; nearest active mission is marked on spawn |
| Champion | Vehicles (pickup, van), fortification kits, good medkits | **Community:** respawn at any Haven safehouse with a small supply bag |
| Hunted | — | **Exile:** see below |

Each safehouse's quartermaster only stocks what its size allows: a Shelter sells Known
items, a Community or Settlement up to Trusted, a Town up to Champion
([WORLD.md](WORLD.md), "Safehouses and population"). Every safehouse is a spawn point for everyone.

#### Haven house rules and exile

| Rule | What happens |
|---|---|
| Firing within 50 m of a safehouse | +3 Haven Infamy (at most once every 20 s), unless an infected is within 40 m of the safehouse: then you're defending it |
| Declaring a bite at a safehouse | Once per life: with a medic (Settlement and up), the infection's time left ×1.5. Without one, no slowing, but you can't be caught |
| Hiding a bite | Inside a safehouse in your last 5 minutes without having declared it: 30% a minute to be caught, once per life, +15 Haven Infamy |
| Shooting at, robbing or killing a wanderer | +4 (shot at, once per wanderer), +8 (robbed), +15 (killed) Haven Infamy; they remember you |
| Hunted by the Haven (exile) | Safehouse services and the Haven quartermaster refuse you ("the door stays shut"); Haven wanderers refuse you and warn other players near you; a safehouse short on food may sell your rough location to the Red Hand (a hunter squad, and a message to players the Red Hand knows). Guards never shoot anyone, exiles included |

Food delivered to a safehouse pays 2 Haven Favor per item, plus 1 Fame per 3 items (at most 5).
Wanderers' attitude is your Haven standing (Bastion, reversed, for the Deserter) plus their
memory of you; see [WORLD.md](WORLD.md), "Wanderers".

### Task Force Bastion

| Tier | Shop (Bastion requisition) | Perk / service |
|---|---|---|
| Known | Military rations, morphine, flares | Checkpoints let you through without being searched |
| Trusted | Service rifles, body armour, helmets, NVGs | **Requisition:** once an hour, draw ammo or rations from Bastion's stock at half the Favor cost, at the Bastion quartermaster. *(Planned: drawn from a nearby outpost's stock instead, and only while a supply truck has restocked it; see [WORLD.md](WORLD.md).)* |
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

## Faction leaders

Every place a faction holds has a named leader ([LORE.md](LORE.md), "Who's in charge"),
and reputation is with people as well as factions. Code: `Factions/RZ_Leaders.c`,
`Factions/RZ_LeaderVoice.c`; where they sit is in [WORLD.md](WORLD.md), "Leaders".

| Leader | Where | Can die? |
|---|---|---|
| Stronghold leader (one per faction) | at the faction's quartermaster; Halcyon's at the lab | Never; never replaced; always Loyal |
| Council head | each Haven safehouse (the one at the Haven quartermaster is the stronghold) | Yes |
| Commander, crew boss, lead researcher | each outpost (interim leader posts until territory exists) | Yes |

Each has a seeded name, a title, a one-line backstory and **two traits**. They're saved.
Only the leader at the place you're standing near has a body; at a stronghold or
safehouse that's usually the quartermaster itself.

### Traits

Every effect is small and capped: one leader's combined multiplier never goes past ±20%.

| Trait | On contracts they give | Elsewhere |
|---|---|---|
| Greedy | Favor ×1.2; cross-faction trade jobs a little more often | Their quartermaster charges ×1.15 |
| Paranoid | Fame ×0.85 | Stronghold: Infamy with the faction ×1.15. Remembers being crossed harder |
| Merciful | Amends work off ×1.15 more Infamy (outpost and safehouse leaders) | Stronghold: Infamy gained ×0.9, worked off ×1.2; amends contracts more often. Forgives faster |
| Ruthless | Darker personal jobs more often | Stronghold: contracts that hurt a rival more often |
| Idealist | Hopeful personal jobs more often, dark ones less | Stronghold: harmless contracts more often. Halcyon Idealists remember anyone who sells samples to the Red Hand |
| Pragmatist | Cross-faction personal jobs more often | Deals with people their faction hates (their bad standing counts as Watched at worst). Stronghold: choice missions more often |
| Veteran | Favor ×1.1; Fame ×1.15 for Soldiers and Police; tough jobs more often | Respects Soldiers and Police. Stronghold: Trusted+ contracts more often |
| Talker | Talky jobs more often | Two rumours instead of one, two other factions in a first meeting; their rogue jobs leak more easily |

"Stronghold" effects come from the faction's stronghold leader and apply faction-wide.
Contract weights change by at most ±20%.

### Loyalty

Outpost and safehouse leaders have a loyalty: **Loyal** (60+ on a hidden 0–100 score),
**Independent**, or **Rogue** (below 25). It changes how your standing counts with them:

| Loyalty | Their attitude to you | Work |
|---|---|---|
| Loyal | Your faction standing plus their memory of you. Hunted by the faction = no | Voice the stronghold's radio contracts; no personal jobs |
| Independent | Half your standing plus double their memory | Off-book personal jobs |
| Rogue | Double their memory; bad standing doesn't count if you've helped them | Off-book jobs that work against the stronghold; their quartermaster sells you Known items even if the stronghold hunts you |

**Memory** (per player, saved): *helped* (did work for them: +1), *saved* (killed infected at
their door: +2), *crossed* (shot near them, turned them in, worked against their faction,
took the job the way they didn't want: −3, −2 for a Merciful leader, −4 for a Paranoid one). At −2 or below they won't talk to you. Their
quartermaster's prices move ×0.9 for people they owe and ×1.1 for people who crossed them
(doubled for Independent and Rogue leaders), within the ±20% cap.

**What moves loyalty** (at most 15 points an hour, except for the big events):

| Event | Change |
|---|---|
| A safehouse runs out of food / is fed again | −5 / +2 |
| The faction's contracts near them fail / succeed | −2 / +1 |
| They finish a rogue job without getting caught | −3 |
| Their own off-book delivery against orders (Our Own First, Hearts and Minds) | −2 |
| Every hour, back toward their temperament (55; Idealist +10, Paranoid +5, Pragmatist −10, Ruthless −5) | ±1–2 |
| Rein In: terms read / counter-offer taken | +25 / −5 |
| A rogue job is discovered and they keep their place | +25 |
| A new leader | rolled fresh: 40% Loyal, 40% Independent, 20% Rogue |

At most one leader per faction is Rogue at a time, and stronghold leaders are always Loyal,
so no faction ever splits. When a leader turns Rogue the stronghold puts out **Rein In**
([MISSIONS.md](MISSIONS.md), "Leader missions").

**Discovery.** A rogue job is discovered when it's done with a 15% chance, +35% if you were
seen by the faction's own people while doing it (you passed another of its places, or a Haven
wanderer saw you), +10% for a Talker leader, −5% for a Paranoid one. Discovered: everyone who
was paid takes **+10 Infamy** with the faction, and the leader is removed (half the time) or
pulled back into line. You can also turn a rogue leader in yourself: most rogue deliveries
have a drop at the stronghold, which pays you instead and costs the leader their place or
their nerve.

### Talking to leaders

Talk to a leader (their body, or `RZ_LeaderAction` on the quartermaster): **Ask for work**
(their personal job if they have one for you, else the stronghold's contracts),
**Ask for news** (other leaders' loyalties and off-book work, hungry safehouses, noise), and
**Ask what they think of you** (their opinion, what they remember, where they stand with their
stronghold if they trust you, and their traits). Radio contracts are briefed in the voice of
the nearest Loyal leader of the giver, with an intro and an outro by trait, and that leader's
traits set the pay.

**First contact.** The first time you talk to a leader they say who their faction is, who
they are, and what they think of one other faction (two for a Talker), with where its nearest
leader sits. The hook for new-player guidance is
`RZ_Leaders.GetFirstContactLine(faction, leaderId, playerId)` (leaderId −1 = the stronghold
leader; marks them as met) and `RZ_Leaders.DescribeStronghold(faction, fromPos)`.

**Deaths and replacements.** Off-screen, every half hour, an outpost or safehouse leader can be
bitten (1–4%, more in an infected town) or killed in a raid (1%, doubled at a hungry safehouse).
Killing one yourself is +15 Infamy with their faction. A new leader takes the chair after
2 hours of server time, with new traits and a new loyalty. All of it is headline news ("Word on
the roads"); new players see the last three headlines when they first spawn.

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

- `RZ_PlayerProfile`: Fame/Infamy/Favor arrays per faction for the current life, plus
  the persistent `m_aChampionOf` and `m_aUnlocked` (background unlock ids)
- `RZ_Reputation`: tier table, access ranks, standing checks, rival table, display names.
  Mixed tiers count one step lower for access: Respected = Trusted, Tolerated and
  Wildcard = Known, Uneasy = Stranger. Negative tiers have no access.
- `RZ_GameModeComponent`: `AddFame`, `AddInfamy`, `AddFavor`, `SpendFavor`, `GetTier`.
  Players are told when their tier changes, not on every point.
- Rival table (Fame with the first faction also adds this share as Infamy with the second):
  Haven → Red Hand 15%, Bastion → Red Hand 25%, Red Hand → Bastion 20% and Haven 15%,
  Halcyon → Red Hand 10%. Missions with an explicit rival use that instead of the table.
- `RZ_QuartermasterComponent` marks a faction contact. `RZ_FactionShopAction` on it has faction,
  required tier, Favor cost, item prefab and name. The server checks tier and Favor when the
  action is used and refuses with a message; the action is not greyed out on the client.
  Shop System can still be used for general trading.
- Missions: `m_eGiver` gets Fame, an optional `m_eRival` gets Infamy, `m_iFavorReward` pays out.
  `m_iLeaderId` is the leader who gives it: their traits scale the Fame and Favor
  (`RZ_Leaders.FameMult`, `FavorMult`), and they remember who did it
- `RZ_Leaders`: leaders, traits, loyalty, memory, discovery, headlines. `RZ_GameModeComponent.AddInfamy`
  runs every Infamy change through the stronghold leader's temper (`ScaleInfamy`); shop prices go
  through `PriceMult` in `RZ_FactionShopAction`
- Perks are checked in code where they apply (`RZ_FactionPerks`, `RZ_CureResearch`).
  House rules and exile are in `RZ_Safehouses` (and `RZ_Wanderers` for wanderers);
  the location leak goes through `RZ_FactionPerks.LeakLocation`
- AI hostility: Reforger factions are global, so per-player hostility is done by
  sending hunter squads at Hunted players, not by changing faction relations
- Season ending: Fame earned with each faction is summed over all players
  (`RZ_SaveData.seasonFame`); the winner is written to `last_season.json`

### What's in code and what isn't yet

| In code | Not yet |
|---|---|
| All tiers, Fame/Infamy/Favor, rivals, amends missions | Paying Favor to remove Infamy |
| Word gets around, all five unlockable backgrounds | Disguises, surrendering at checkpoints |
| Quartermaster shops (any item, any tier); Haven stock by safehouse size | Community respawns (the supply bag) |
| Requisition (half price at the quartermaster), sample scanner, Halcyon research bonus, Neighbours | Requisition from truck-supplied outpost stock; Fire Mission, Fence, Bounty board, Halcyon medic, immunity booster |
| Toll pass (message only), Halcyon Hunted gets no Fame for samples | AI checkpoints and roadblocks reacting to standing |
| Hunter squads (Bastion and Red Hand), Kill-on-Sight bounties | — |
| Haven safehouse spawn points, house rules, exile, Haven wanderers warning other players, safehouse location leaks | — |
| Named leaders: traits, loyalty, memory, voiced briefs, first contact, discovery, deaths and replacements, Rein In | Leaders for real territory outposts (the `RegisterOutpost` hook is ready) |
| Season winner by Fame | Next-season rules from the winner |
