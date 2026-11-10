# Reputation, factions and unlocks

How the four factions treat you. Inspired by New Vegas: every faction remembers two
separate things about you, **Fame** (what you did for them) and **Infamy** (what you did
to them). You can be a hero to the Haven and a nightmare to the Red Hand, or loved and
feared by the same faction at once.

Missions are the main source ([MISSIONS.md](MISSIONS.md)). Most help one faction at the cost
of another, and some make you **choose who gets the prize**. Who the factions are as people is
in [LORE.md](LORE.md); how they hold territory and move supplies is in [DESIGN.md](DESIGN.md),
"Factions in the background". ✅ = in code; everything else is planned ([ROADMAP.md](ROADMAP.md)).

## The four factions

| Faction | Who they are | Wants | Rival of |
|---|---|---|---|
| **The Haven** | A network of safehouses run by ordinary people stuck in the Zone | Food, medicine, safe towns, people | The Red Hand. Uneasy with Bastion |
| **Task Force Bastion** | Soldiers left inside the line, still holding a few posts | Order, control, the cure as a strategic asset | The Red Hand. Distrusts Halcyon |
| **The Red Hand** | Raiders and opportunists who think the old rules are dead | Loot, territory, fear | Everyone, Bastion most of all |
| **Halcyon** | The researchers running the only lab | Samples, data, protection, the cure going public | Wary of Bastion seizing it |

The code uses the enum names `SURVIVORS`, `MILITARY`, `BANDITS` and `INSTITUTE`, and the
faction keys `SURV`, `MIL`, `BAND` and `INST`. Only the display names differ.

## Fame and Infamy

Each faction tracks **Fame 0–100** and **Infamy 0–100** for your current life.

- Helping a faction gives Fame with it, and usually Infamy with its rival.
- Hurting a faction (killing its people, stealing, sabotage, siding against it) gives Infamy.
- Neither goes down on its own. **Amends** contracts work Infamy off while you're Watched ✅;
  paying Favor to work it off is planned.

### Standing tiers

| Fame ↓ / Infamy → | Low (< 15) | Some (15–39) | High (40–74) | Extreme (75+) |
|---|---|---|---|---|
| **Low (< 15)** | Stranger | Watched | Hunted | Kill on Sight |
| **Some (15–39)** | Known | Uneasy | Hunted | Kill on Sight |
| **High (40–74)** | Trusted | Tolerated | Wildcard | Hunted |
| **Extreme (75+)** | **Champion** | Respected | Wildcard | Wildcard |

- **Positive tiers:** Known → Trusted → Champion. They unlock shops, perks and missions.
- **Mixed tiers** count one step lower for access: Respected = Trusted; Tolerated and
  Wildcard = Known; Uneasy = Stranger.
- **Negative tiers:** Watched → Hunted → Kill on Sight. The faction comes after you.

Players are told when their tier changes, not on every point.

### Favor (currency)

Every faction pays in its own **Favor**, earned from its missions and spent at its
quartermaster. Reputation decides **what** you can buy; Favor decides **how much**. Favor
is per life, like reputation.

### Rivals

Fame with one faction also adds a share as Infamy with its rival: Haven → Red Hand 15%,
Bastion → Red Hand 25%, Red Hand → Bastion 20% and Haven 15%, Halcyon → Red Hand 10%.
A mission with its own rival uses that instead. A truce halves it, a feud makes it ×1.5
([Truces and feuds](#morale-needs-truces-and-feuds)).

### Lives and carry-over

Reputation is per character, so a new life starts fresh with only its background's starting
standing ([DESIGN.md](DESIGN.md), "Backgrounds and personal goals"). Two things carry over:

- **Word gets around** ✅: if any past character reached Champion with a faction, every new
  character starts at Known with it.
- **Unlocks** ✅: Champion with a faction unlocks a background (Soldier, Doctor, Raider, Elder);
  10 Legacy points unlock the Drifter.

## What each faction gives you

Shops are the faction's own **quartermasters**: any item, crate or vehicle, sold for Favor at
a required tier ✅. The items below are a suggested starting list. Prices are the base cost ×
the local leader's traits and memory (within ±20%), × a shortage (+25–50% where a supply truck
didn't arrive), × the faction's morale (0.95–1.1). The server checks tier and Favor when you buy.

### The Haven

| Tier | Shop | Perk / service |
|---|---|---|
| Known | Canned food, water, bandages, crowbars, car parts | A welcome message ✅ (safehouse services are open to anyone the Haven hasn't exiled) |
| Trusted | Hunting rifles, ammo, backpacks, radios, jerrycans | **Neighbours** ✅: the direction to the nearest job you'd be paid for, on spawn and every 10 min |
| Champion | Vehicles, fortification kits, good medkits | **Community**: respawn at a safehouse with a small supply bag |
| Hunted | — | **Exile** ✅: see below |

Each safehouse's quartermaster stocks by its size: a Shelter sells Known items, a Community or
Settlement up to Trusted, a Town up to Champion ([DESIGN.md](DESIGN.md), "Haven safehouses").

### Task Force Bastion

| Tier | Shop | Perk / service |
|---|---|---|
| Known | Military rations, morphine, flares | Pass Bastion's road checks inside the Zone unsearched (planned: needs checkpoints as AI; the game only says Bastion knows you) |
| Trusted | Service rifles, body armour, helmets, NVGs | **Requisition** ✅: once an hour, ammo or rations at half the Favor cost. While Bastion's trucks run, it's drawn at a Bastion outpost's quartermaster from the stock its trucks brought, and refused while that outpost is short |
| Champion | Machine guns, grenades, military trucks | **Fire Mission**: mark a horde with smoke for an artillery strike |
| Hunted | — | Bastion AI shoots you ✅; hunter squads get your last position ✅ |

### The Red Hand

| Tier | Shop (black market) | Perk / service |
|---|---|---|
| Known | Knives, pistols, lockpick-style tools | **Toll pass**: roadblocks let you pass for a toll (message only ✅; roadblocks as AI planned) |
| Trusted | Suppressors, scoped rifles, explosives | **Fence**: sell loot for Favor; ambush crews ignore you |
| Champion | Rare military gear, stolen antidotes (only the doses stolen from the lab ✅) | **Bounty board**: post bounties on other players |
| Hunted | — | Red Hand AI shoots you ✅; hunter squads come for you ✅ |

### Halcyon

| Tier | Shop (lab stores) | Perk / service |
|---|---|---|
| Known | Antiseptic, painkillers, sample containers | — |
| Trusted | Antidotes (only while the lab has doses in stock ✅), stim kits, gas masks | **Sample scanner** ✅: special infected within 300 m are revealed every minute |
| Champion | Experimental immunity booster (planned item) | **Lab access** ✅: your samples count 25% more. Calling in a Halcyon medic is planned |
| Hunted | — | The lab takes your samples without Fame ✅, and Halcyon AI shoots you ✅ |

### Consequences for everyone

- **Hunter squads** ✅: every 10 minutes, Bastion or the Red Hand sends an AI group 400 m from
  a player they Hunt, toward where they were. It's hostile to that player within 800 m.
- **Bounties** ✅: Kill on Sight with any faction puts a public bounty on you, announced every
  10 minutes with a rough location, claimed by whoever is near when you die.
- **Choice missions** ✅: you decide which faction's drop point gets the item. The winner gives
  Fame; the one you double-crossed gives Infamy.
- **Disguises** (planned): a faction's uniform gets you past its checkpoints while Watched or
  Uneasy; caught, your Infamy gain doubles.

## Haven house rules and exile

Who the Haven are, and why they keep these rules, is in [LORE.md](LORE.md), "The Haven". ✅ all.

| Rule | What happens |
|---|---|
| Firing within 50 m of a safehouse | +3 Haven Infamy (at most once every 20 s), unless an infected is within 40 m of the safehouse: then you're defending it. Drawing a weapon is fine |
| Declaring a bite at a safehouse | Once per life: with a medic (Settlement and up), the infection's time left ×1.5. Without one, no slowing, but you can't be caught |
| Hiding a bite | Inside a safehouse in your last 5 minutes without having declared it: 30% a minute to be caught, once per life, +15 Haven Infamy |
| Shooting at, robbing or killing a wanderer | +4 (shot at, once per wanderer), +8 (robbed), +15 (killed) Haven Infamy; they remember you |
| Hunted by the Haven (**exile**) | Safehouse services and the Haven quartermaster refuse you ("the door stays shut"); Haven wanderers refuse you and warn other players near you. The guards shoot an exile only while they carry a firearm within 100 m of a safehouse (with a minute's grace after spawning at one); an unarmed exile just finds the door shut |
| A hungry safehouse and an exile | A safehouse short on food may sell an exile's rough location to the Red Hand (25% every 10 minutes): a hunter squad that really shoots them, and a message to players the Red Hand knows |

Food delivered to a safehouse pays 2 Haven Favor per item, plus 1 Fame per 3 items (at most 5).
Wanderers' attitude is your Haven standing (Bastion, reversed, for the Deserter) plus their
memory of you ([DESIGN.md](DESIGN.md), "Wanderers"). The Mechanic wanderer sells car parts for
12 Haven Favor (a quarter off if they remember you helped) and gives a fixable car's keys to
players they like.

## Faction leaders

Every place a faction holds has a named leader ([LORE.md](LORE.md), "Who's in charge"), so
reputation is with people as well as factions. Where they sit and how they're simulated is in
[DESIGN.md](DESIGN.md), "Leaders". Code: `Factions/RZ_Leaders.c`, `Factions/RZ_LeaderVoice.c`. ✅ all.

Each leader has a seeded name, a title, a one-line backstory and **two traits**, and is saved.
Stronghold leaders never die, are never replaced and are always Loyal.

### Traits

Every effect is small and capped: one leader's combined multiplier never goes past ±20%.

| Trait | On contracts they give | Elsewhere |
|---|---|---|
| Greedy | Favor ×1.2; cross-faction trade jobs a little more often | Their quartermaster charges ×1.15 |
| Paranoid | Fame ×0.85 | Stronghold: Infamy with the faction ×1.15. Remembers being crossed harder |
| Merciful | Amends work off ×1.15 more Infamy | Stronghold: Infamy gained ×0.9, worked off ×1.2; amends contracts more often. Forgives faster |
| Ruthless | Darker personal jobs more often | Stronghold: contracts that hurt a rival more often |
| Idealist | Hopeful personal jobs more often, dark ones less | Stronghold: harmless contracts more often. Halcyon Idealists remember anyone who sells samples to the Red Hand |
| Pragmatist | Cross-faction personal jobs more often | Deals with people their faction hates (bad standing counts as Watched at worst). Stronghold: choice missions more often |
| Veteran | Favor ×1.1; Fame ×1.15 for Soldiers and Police; tough jobs more often | Stronghold: Trusted+ contracts more often |
| Talker | Talky jobs more often | Two rumours instead of one; their rogue jobs leak more easily |

"Stronghold" effects come from the stronghold leader and apply faction-wide. Contract weights
change by at most ±20%.

### Loyalty

Outpost and safehouse leaders have a loyalty: **Loyal** (60+ on a hidden 0–100 score),
**Independent**, or **Rogue** (below 25).

| Loyalty | Their attitude to you | Work |
|---|---|---|
| Loyal | Your faction standing plus their memory of you. Hunted by the faction = no | Voice the stronghold's radio contracts; no personal jobs |
| Independent | Half your standing plus double their memory | Off-book personal jobs |
| Rogue | Double their memory; bad standing doesn't count if you've helped them | Off-book jobs against the stronghold; their quartermaster sells you Known items even if the stronghold hunts you |

**Memory** (per player, saved): *helped* (+1), *saved* (killed infected at their door: +2),
*crossed* (shot near them, turned them in, worked against their faction: −3; −2 for a Merciful
leader, −4 for a Paranoid one). At −2 or below they won't talk to you. Their quartermaster's
prices move ×0.9 for people they owe and ×1.1 for people who crossed them (doubled for
Independent and Rogue leaders), within the ±20% cap.

**What moves loyalty** (at most 15 points an hour, except the big events):

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

At most one leader per faction is Rogue at a time, so no faction ever splits. When a leader
turns Rogue, the stronghold puts out **Rein In** ([MISSIONS.md](MISSIONS.md), "Leader missions").

**Discovery.** A rogue job is discovered with a 15% chance, +35% if the faction's own people saw
you doing it, +10% for a Talker leader, −5% for a Paranoid one. Discovered: everyone paid takes
**+10 Infamy** with the faction, and the leader is removed (half the time) or pulled back into
line. You can also turn a rogue leader in: most rogue deliveries have a drop at the stronghold.

### Talking to leaders

Talk to a leader (their body, or `RZ_LeaderAction` on the quartermaster): **Ask for work**
(their personal job, else the stronghold's contracts), **Ask for news** (other leaders'
loyalties, hungry safehouses, noise, rumours) and **Ask what they think of you**. Radio
contracts are briefed in the voice of the nearest Loyal leader of the giver, whose traits set
the pay.

**First contact.** The first time you talk to a leader they say who their faction is, who
they are, and what they think of one other faction (two for a Talker), with where its nearest
leader sits. Hooks: `RZ_Leaders.GetFirstContactLine(faction, leaderId, playerId)` and
`RZ_Leaders.DescribeStronghold(faction, fromPos)`.

**Deaths.** Off-screen, every half hour, an outpost or safehouse leader can be bitten (1–4%) or
killed in a raid (1%, doubled at a hungry safehouse). Killing one yourself is +15 Infamy with
their faction. A new leader takes the chair after 2 hours. All of it is headline news.

## Morale, needs, truces and feuds

Factions change with the world (code: `Factions/RZ_FactionLife.c`). The radio chatter, life
events, remembered moments and rumours this feeds are in [DESIGN.md](DESIGN.md), "Faction life".
Every effect is small, capped and recovers. ✅ all.

**Needs.** Each faction has one need at a time, worked out every 5 minutes. Leaders and named
people mention it, and contracts that answer it are picked more often (×1.2):

| Need | When | Contracts it favours |
|---|---|---|
| Food (Haven) | a safehouse is short, or a Haven truck was lost in the last hour or so | deliveries and choice missions |
| Supplies (Bastion, the Red Hand) | one of its trucks was lost recently | deliveries and choice missions |
| Security | a leader died or an outpost was lost recently (the Haven: a death) | holds, kill missions, work against a rival |
| Revenge (Bastion, the Red Hand) | a feud; for the Red Hand also an outpost lost | work that costs a rival Infamy |
| Samples (Halcyon) | the cure isn't done | Halcyon's contracts |
| People (Haven) | its safehouses hold fewer than 6 people each on average | rescues and escorts |

**Morale** (5–95, starts at 60, recovers 2 points every 10 minutes toward 60): a lost truck −10,
a lost outpost −10, a dead leader −8, Under Pressure −5 (+5 when it ends), a lost final stand
−15 (Halcyon); a delivered truck +4, a new or won outpost +5, a breakthrough +10 (Halcyon), a
contract for them +1 (−1 if it failed).

| Morale | Their people | Their shops | The Haven |
|---|---|---|---|
| Under 30 | jumpier: retaliation reaches ×1.25 as far and lasts ×1.25 as long | ×1.1 | rescued survivors walk off 2.5× as often (at most half) |
| Over 75 | calmer: ×0.85 | ×0.95 | half as often |

A demoralised faction also puts out a little less work (×0.9). Contract weights from needs and
morale stay within 0.85–1.2.

**Truces and feuds.** Temporary: 45–90 minutes each, at most two at once, one per pair of
factions, and a 30-minute pause after one ends.

| | Starts | Rival Infamy between the two | Territory |
|---|---|---|---|
| Truce | every 30 minutes, 30% for a pair of rivals with low morale (both under 45) or a bad outbreak | halved | neither takes the other's towns |
| Feud | a truck lost near the other's holding (50%), a stolen truck delivered to them (60%), an outpost taken by them (60%); or the 30-minute roll for rivals who've been losing | ×1.5 | the stronger takes the weaker's towns with no strength margin |

They're headlines, never alerts.

## What makes AI shoot you

AI shoots individual players, not everyone on the player faction. ✅ all. Each faction's AI
fires on a player when one of these is true for that player:

| Faction (AI) | Default stance | Shoots you when |
|---|---|---|
| The Haven (guards, occupants, survivor groups, wanderers) | Peaceful | You're Hunted by the Haven **and** carry a firearm within 100 m of a safehouse; or you're near an armed wanderer with a grudge against you |
| Task Force Bastion | Wary: no shooting (the Perimeter works as before) | You're Hunted or Kill on Sight by Bastion |
| The Red Hand | Neutral: they leave you alone | You're Hunted or Kill on Sight by the Red Hand; or you're near one of their crews in a fight |
| Halcyon | Peaceful | You're Hunted or Kill on Sight by Halcyon |

The stance is a setting per faction (**Shoots from**: Hunted, Watched, Strangers or Never).
Setting the Red Hand to *Strangers* makes its crews shoot anyone below Known, which makes the
Known toll pass the difference between being shot and not.

**Fights.** Some AI is spawned to fight, whatever your standing:

| Who | Shoots |
|---|---|
| A hunter squad | The player it was sent after, within 800 m |
| The Red Hand camp in Raid the Red Hand, the raiders in Trader Caravan and Lab Defence | Every player within 350 m of them while they live |
| The killers' camp in The Witness | Players within 150 m who aren't at least Known with the Red Hand |

In Trader Caravan the trader is put on a variant the Red Hand is hostile to, so the raiders go
for him while he stays friendly to players.

**Wanderers fight back.** A wanderer whose body is an armed AI group shoots, within 80 m, a
player who robbed or shot at them; the Deserter draws on Bastion's Champions. The Kid never does.

**Shooting back.** Shoot or kill a faction's AI that ReforgerZ spawned (guards and named people
at holdings, supply truck crews, safehouse guards) and that faction's AI within 300 m turns on
you for 3 minutes. Another hit restarts the timer, never adds to it, so it can't become lasting
hostility. A kill also costs +3 Infamy with the faction, at most 12 an hour. It doesn't count
with an infected within 40 m of the victim, near a mission fight or hunter squad, or if that
faction already shoots you. Leaders, wanderers and mission fights keep their own rules. A
setting turns it off.

**Players and each other.** Player-against-player damage is unchanged. At the AI level, players
stay friendly to each other, with one exception: while the Haven shoots you, you also count as
hostile to players nobody is shooting, which can show in name tags and vanilla team-kill accounting.

**How it works.** Reforger decides hostility per faction, so each player's *character* is moved
onto one of 15 hidden copies of the player faction (`SURV_H1` to `SURV_H15`) whose number is a
bitmask of the AI factions that shoot them: 1 the Haven, 2 Bastion, 4 the Red Hand, 8 Halcyon.
A modded faction check decodes the key. The player controller stays on `SURV`, so the deploy
menu, chat, groups and map markers don't change. Standing is re-read when a tier changes or you
spawn; nearness is checked on the server tick.

**Fallback.** If AI doesn't follow the modded check, set **Mode** to *Outlaw*: anyone any
faction would shoot goes onto one `SURV_OUTLAW` faction, hostile to every AI faction. It's
blunter (a Bastion-Hunted player is then shot by the Haven and Halcyon too) but uses only
vanilla relations. Setup is in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md), "Factions".

## The season's winner

When the final stand is won, the cure goes to whichever faction the server gave the most Fame
this season ✅. What that means is in [LORE.md](LORE.md), "Endings".

## How it maps to code

- `RZ_PlayerProfile`: Fame, Infamy and Favor per faction for the current life, plus the
  persistent Champion list and background unlocks.
- `RZ_Reputation`: the tier table, access ranks, standing checks, the rival table, display names.
- `RZ_GameModeComponent`: `AddFame`, `AddInfamy`, `AddFavor`, `SpendFavor`, `GetTier`. Every
  Infamy change goes through the stronghold leader's temper (`RZ_Leaders.ScaleInfamy`).
- `RZ_QuartermasterComponent` marks a faction contact; `RZ_FactionShopAction` sells one item
  (`RZ_SupplyLines.FindShopState` for shortages, `PriceMult` for leaders and morale).
- Missions: `m_eGiver` gets Fame, an optional `m_eRival` gets Infamy, `m_iFavorReward` pays out,
  `m_iLeaderId` is the leader whose traits scale it.
- `RZ_FactionPerks`: perks, hunter squads, bounties, the safehouse location leak (`LeakLocation`).
  House rules and exile are in `RZ_Safehouses` and `RZ_Wanderers`.
- AI hostility: `RZ_Hostility` works out each player's bitmask; `RZ_Factions` builds and decodes
  the keys; the modded `SCR_Faction` in `RZ_FactionRelations.c` turns them into relations;
  `RZ_Retaliation` handles shooting back.
- The season's winner: Fame per faction summed over all players (`RZ_SaveData.seasonFame`),
  written to `last_season.json`.
