# Mod stack

ReforgerZ does **not** rebuild what the Workshop already does well. The server
runs existing mods as dependencies, and ReforgerZ is the layer on top that
ties them together: the Director, outbreak stages, settlements, missions,
backgrounds, reputation and the cure.

Rule of thumb: **other mods provide content and mechanics; ReforgerZ decides
when, where and why they show up.**

> I found these on the Arma Reforger Workshop in November 2026. Before you
> lock the stack in, check each page for the current game version, the
> last-updated date and the license. Several mods are WIP.

## Core (required)

| Mod | Role in ReforgerZ | Notes |
|---|---|---|
| [Bacon Zombies](https://reforger.armaplatform.com/workshop/622120A5448725E3) | Zombie AI, animations and prefabs (civilian, military, exploding, ranged, heavy) | The foundation most zombie mods build on. Its prefabs go into `RZ_ZombieSettings`. |
| [Special Zombies](https://reforger.armaplatform.com/workshop/69BD721866937F85) | Special variants plus destructible Zombie Hives | Special prefabs go in `m_aSpecialPrefabs`. Killing one drops a cure sample. |

## Survival layer (pick one)

| Mod | Why | Notes |
|---|---|---|
| [Metabolism](https://reforger.armaplatform.com/workshop/64E3272AFD56FEF7-Metabolism) | **Recommended.** Hunger and thirst only, **no sleep**. | WIP but widely used. |
| [ENDURE Survival Systems](https://reforger.armaplatform.com/workshop/6A621851A4A3E54C) | Seasons, wetness, cold, hunger/thirst | Good for a "Winter" season later. Check that it has no sleep or energy mechanic you don't want. |

No sleep and no time skip, by design. Skip any survival mod that adds
either one. Keep the server's time acceleration at a fixed rate and don't run
mods that skip time.

## Economy and building

| Mod | Role | Notes |
|---|---|---|
| [Shop System (ekudmada)](https://reforger.armaplatform.com/workshop/5D2D1436D1FA5A13-ShopSystem) | Faction traders (Military, Bandit, Institute) with currency **or barter** | The most downloaded shop mod. Phase 3 hooks reputation into prices and access. |
| [Kodiis Building System](https://reforger.armaplatform.com/workshop/644F5B4619455A52) | Base building, code locks, persistent storage, raidable bases | Last listed for 1.7, so **test it with vanilla persistence first**. |

## Persistence

| Option | Role |
|---|---|
| **Vanilla Persistence System (1.6+)** | Player bodies, inventories, vehicles. See the [BI wiki](https://community.bohemia.net/wiki/Arma_Reforger:Persistence_System). |
| ReforgerZ `state.json` | Director stage, settlement infection, cure progress, player profiles. |

The [Enfusion Persistence Framework](https://github.com/Arkensor/EnfusionPersistenceFramework)
is **deprecated** since 1.6, because the base game took over its job. Don't build on it.

## Optional extra content

| Mod | Use |
|---|---|
| [DarcMissions](https://reforger.armaplatform.com/workshop/5ED0FAC84A48D018) | Any-map AI missions (convoys, occupations, crash sites). Run it next to ReforgerZ missions with bandit or military factions for human threats. |
| [Ambient Zombies](https://reforger.armaplatform.com/workshop/692172F6EADE0B44) | Ambient roaming clusters. **Only if** ReforgerZ ambient spawning is turned off (`m_iAmbientPerPlayer = 0`), otherwise they double up. |
| [Zombie Hive](https://reforger.armaplatform.com/workshop/65F2F0F322747C37) | Destructible spawners. A good later addition for "nest" missions. |

## Don't stack with ReforgerZ

These are complete survival game modes. They compete with ReforgerZ rather
than plug into it, but they're worth playing for research:
[ReforgedZ](https://reforgedz.net/),
[The Encounter Project](https://reforger.armaplatform.com/workshop/631B4E07FFD86258),
[Last Stand Core](https://reforger.armaplatform.com/workshop/56C5B1F99E03EFE4-LastStandCore),
[ARMST Platform](https://reforger.armaplatform.com/workshop/6922BD179EEDD0D2).

> **Naming:** "ReforgedZ" already exists and is very close to "ReforgerZ".
> Consider a different Workshop name before release.
