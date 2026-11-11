# Mod stack

ReforgerZ doesn't rebuild what the Workshop already does well. The server runs a few existing
mods as dependencies, and ReforgerZ is the layer on top: the Director, settlements, the
Perimeter, factions, missions, backgrounds and the cure.

**Rule of thumb:** other mods provide content and mechanics; ReforgerZ decides when, where and
why they show up.

**What the code actually depends on:** nothing by name. No ReforgerZ script references another
mod's classes. Every link is a prefab slot you fill in Workbench (zombie prefabs, a nest, water
and food items) or a keyword matched against prefab names (food, water, fuel, parts). A mod can
be swapped for another that offers the same kind of prefab.

> These were found on the Arma Reforger Workshop in November 2026. Before you lock the stack
> in, check each page for the current game version, the last-updated date and the license.
> Several are WIP.

## Required

Both are dependencies in `addon.gproj`, so Workbench, the compile check and every server need them.

| Mod | Role | How ReforgerZ uses it |
|---|---|---|
| [Bacon Zombies](https://reforger.armaplatform.com/workshop/622120A5448725E3) | Zombie AI, animations and prefabs (civilian, military, exploding, ranged, heavy) | Its group prefabs go in **Zombies > Common / Special / Horde prefabs**, and its faction must be hostile to all four ReforgerZ factions. Bite detection works with any zombie mod |
| [Special Zombies](https://reforger.armaplatform.com/workshop/69BD721866937F85) | Special variants and destructible hives (needs Bacon Zombies) | Specials go in **Special prefabs** (they drop cure samples); runner, helmet and screamer variants in **Zombies > Adaptation**. To run without it, use Bacon's heavy, exploding and ranged zombies as specials and remove it from `addon.gproj` |

## Recommended

| Mod | Role | How ReforgerZ uses it |
|---|---|---|
| [Metabolism](https://reforger.armaplatform.com/workshop/64E3272AFD56FEF7-Metabolism) | Hunger and thirst, **no sleep** | Its food items feed safehouses (by the **Food keywords**); its unsafe and safe water items fill the **Dirty / Clean water** slots. ReforgerZ can't read hunger itself, so the "hungry" tip waits for a bridge to call `RZ_Guidance.Report` |

## Optional

| Mod | Role | How ReforgerZ uses it |
|---|---|---|
| [Zombie Hive](https://reforger.armaplatform.com/workshop/65F2F0F322747C37) | Destructible zombie spawners | A hive prefab in **Missions > Nest** enables Burn the Nest |
| [Shop System (ekudmada)](https://reforger.armaplatform.com/workshop/5D2D1436D1FA5A13-ShopSystem) | General trading with currency or barter | Not needed for factions: their shops are our own quartermasters, gated by standing ([REPUTATION.md](REPUTATION.md)). Use it for general trading, or as the **Wandering Trader** prefab |
| [Kodiis Building System](https://reforger.armaplatform.com/workshop/644F5B4619455A52) | Base building, code locks, storage | Not integrated. Last listed for 1.7, so test it with vanilla persistence first. ReforgerZ's own barricades are temporary and leave no traces |
| [ENDURE Survival Systems](https://reforger.armaplatform.com/workshop/6A621851A4A3E54C) | Seasons, wetness, cold, hunger and thirst | An alternative to Metabolism, and a candidate for a planned Winter season. Check it has no sleep or energy mechanic first |

**No sleep and no time skip**, by design. Skip any survival mod that adds either. Keep the
server's time acceleration fixed.

## Not used

| Mod | Why |
|---|---|
| [DarcMissions](https://reforger.armaplatform.com/workshop/5ED0FAC84A48D018) | Its convoys, occupations and crash sites assume forces from outside; they don't fit the sealed Zone |
| [Ambient Zombies](https://reforger.armaplatform.com/workshop/692172F6EADE0B44) | Overlaps the ReforgerZ spawner, which already keeps roaming infected around each player |

## Persistence

| What | Saved by |
|---|---|
| Player bodies, inventories, vehicles (including fixed cars) | The **vanilla Persistence System** (1.6+). See the [BI wiki](https://community.bohemia.net/wiki/Arma_Reforger:Persistence_System) |
| Everything ReforgerZ simulates | ReforgerZ's own `state.json` ([DESIGN.md](DESIGN.md), "Persistence") |

The Enfusion Persistence Framework is deprecated since 1.6, because the base game took over its
job. Don't build on it.

## Don't stack with ReforgerZ

These are complete survival game modes. They compete with ReforgerZ rather than plug into it,
but they're worth playing for research:
[ReforgedZ](https://reforgedz.net/),
[The Encounter Project](https://reforger.armaplatform.com/workshop/631B4E07FFD86258),
[Last Stand Core](https://reforger.armaplatform.com/workshop/56C5B1F99E03EFE4-LastStandCore),
[ARMST Platform](https://reforger.armaplatform.com/workshop/6922BD179EEDD0D2).

> **Naming:** "ReforgedZ" already exists and is very close to "ReforgerZ". Consider a different
> Workshop name before release ([LORE.md](LORE.md), "Name").
