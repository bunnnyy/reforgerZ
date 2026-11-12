# Mod stack

ReforgerZ doesn't rebuild what the Workshop already does well. The server runs a few existing
mods as dependencies, and ReforgerZ is the layer on top: the Director, settlements, the
Perimeter, factions, missions, backgrounds and the cure.

**Rule of thumb:** other mods provide content and mechanics; ReforgerZ decides when, where and
why they show up.

**What the code actually depends on:** Bacon Zombies 1.8.0, by name. The behaviour layer
(`Zombies/Behaviour/RZ_BaconHooks.c`) modifies Bacon's classes, so ReforgerZ doesn't compile without
Bacon Zombies: it's a hard dependency, not a swappable one (the hooks are listed in its row below).
No other ReforgerZ script references another mod's classes. Every other link is a prefab slot you
fill in Workbench (zombie prefabs, a nest, water and food items) or a keyword matched against prefab
names (food, water, fuel, parts). The repo's prefabs use the two required mods (below). Special
Zombies can be swapped for another mod that offers the same kind of prefab: refill the slots and
rebase the prefabs built on it.

> These were found on the Arma Reforger Workshop in November 2026. Before you lock the stack
> in, check each page for the current game version, the last-updated date and the license.
> Several are WIP.

## Required

Both are dependencies in `addon.gproj` (with the mods below), so Workbench and every server need them (the compile check
needs Bacon Zombies; it leaves Special Zombies out, see [below](#special-zombies-stops-reforgerz-loading-on-a-linux-server)).

| Mod | Role | How ReforgerZ uses it |
|---|---|---|
| [Bacon Zombies](https://reforger.armaplatform.com/workshop/622120A5448725E3) | Zombie AI, animations and prefabs (civilian, military, exploding, ranged, heavy) | Its groups fill **Zombies > Common** and **Horde prefabs**; its civilian characters fill **Zombies > Crawlers** (it has no slow prefab: crawlers move as fast as the rest, about 3.7 m/s), its exploder the bloated crawlers, and its empty `Group_Zombies_Base` **Zombies > Empty group** (runtime groups, [INFECTED.md](INFECTED.md#runtime-groups)); its variants fill the runner and helmet lists in **Zombies > Adaptation** and the Wedding Party's guests. Its zombie faction, hostile to all, comes with the vanilla FactionManager base, which Bacon overrides. Its heavy fills **Zombies > Tribes > Alpha prefabs**, its military characters the Guard list, and its civilian and military characters the Hunter, Scout and Young lists ([INFECTED.md](INFECTED.md#tribe-bodies)). **The behaviour layer hooks Bacon 1.8.0 by name** (`Zombies/Behaviour/RZ_BaconHooks.c`: `InfectedCharacter.ComputeLoiterPointInRadius`, `UpdateStance`, `AIBroadcastTarget`, and the nodes `AIGetShooter.ProcessEvent`, `AIRequestClimb`, `AICanMeleeTarget`, `AIInfectedTaskSpeak`), so a Bacon update that renames one breaks the compile; the compile check catches it at once. Bite detection works with any zombie mod |
| [Special Zombies](https://reforger.armaplatform.com/workshop/69BD721866937F85) | Special variants and destructible hives (needs Bacon Zombies) | Its specials fill **Zombies > Special prefabs** (they drop cure samples) and the screamer list in **Zombies > Adaptation**. The **Missions > Nest** prefab is built on its hive, `ZombieHive_Minor` (it spawns its own infected), and the Wedding Party's bride on its Shambler. To run without it, use Bacon's heavy, exploding and ranged zombies as specials, rebase the Nest and the bride, and remove it from `addon.gproj`. **It breaks ReforgerZ on a Linux server** (below) |

### Special Zombies stops ReforgerZ loading on a Linux server

**On a Linux dedicated server, with Special Zombies as a dependency, none of ReforgerZ's scripts
load.** There's no error: the server starts, and the game mode simply isn't there. Special Zombies
keeps its scripts in `Scripts/` (capital S); ours are in lowercase `scripts/`, which the
case-sensitive Linux server needs. With both loaded, the Game module never finds ours.

How it was found: the [compile check](WORKBENCH_SETUP.md#compile-check-without-workbench) printed
OK for every commit after Special Zombies went into `addon.gproj`, but compiled nothing of
ReforgerZ. A sentinel script with one known warning showed it: the warning was missing with Special
Zombies loaded and back without it. The code merged in that time had two compile errors nobody saw.

Until it's resolved, either:
- **host on Windows** (paths there aren't case-sensitive), or
- **drop Special Zombies** (as in the row above) for a Linux server.

The compile check, the runtime test and the zombie lab drop it from their throwaway copy of
`addon.gproj` for this reason. Workbench runs on Windows and isn't affected. Open issue:
[ROADMAP.md](ROADMAP.md#get-it-playable).

## Hunger and thirst

| Mod | Role | How ReforgerZ uses it |
|---|---|---|
| [Metabolism](https://reforger.armaplatform.com/workshop/64E3272AFD56FEF7-Metabolism) | Hunger and thirst, **no sleep** | A dependency in `addon.gproj` since 2026-11-12. Version 0.0.3; it brings Enfusion Persistence Framework and Enfusion Database Framework with it, and modifies the vanilla game mode, character and player controller prefabs. It has one water item (`WaterBottle.et`, no unsafe/safe pair): **Clean water** is that bottle and **Dirty water** is `RZ_Water_Dirty.et`, the same bottle with a 50% sickness chance; its food items feed safehouses by the **Food keywords**. ReforgerZ can't read hunger itself, so the "hungry" tip waits for a bridge to call `RZ_Guidance.Report`. Compiles with ReforgerZ on 1.9 |

## Optional

| Mod | Role | How ReforgerZ uses it |
|---|---|---|
| [Zombie Hive](https://reforger.armaplatform.com/workshop/65F2F0F322747C37) | Destructible zombie spawners | Not needed: the **Missions > Nest** prefab is built on Special Zombies' hive, so Burn the Nest doesn't need it. Only a choice of base for the Nest if you run without Special Zombies |
| [Shop System (ekudmada)](https://reforger.armaplatform.com/workshop/5D2D1436D1FA5A13-ShopSystem) | General trading with currency or barter | Not needed for factions: their shops are our own quartermasters, gated by standing ([REPUTATION.md](REPUTATION.md)). Use it for general trading, or as the **Wandering Trader** prefab |
| [Kodiis Building System](https://reforger.armaplatform.com/workshop/644F5B4619455A52) | Base building, code locks, storage | Not integrated. Last listed for 1.7, so test it with vanilla persistence first. ReforgerZ's own barricades are temporary and leave no traces |
| [ENDURE Survival Systems](https://reforger.armaplatform.com/workshop/6A621851A4A3E54C) | Seasons, wetness, cold, hunger and thirst | An alternative to Metabolism, and a candidate for a planned Winter season. Check it has no sleep or energy mechanic first |

**No sleep and no time skip**, by design. Skip any survival mod that adds either. Keep the
server's time acceleration fixed.

## The camps' look

Dependencies in `addon.gproj` since 2026-11-12, for how the faction bases and their people look
([DESIGN.md](DESIGN.md#camp-life), [People's looks](DESIGN.md#peoples-looks)). Each passed the compile check with ReforgerZ on 1.9. Sizes are the
Workshop's; the server downloads their own dependencies (Functional Knives, IH_Better_GM_Props, and Zelik's Character, whose Face and Head slots the people's masks and balaclavas use) by itself.

| Mod | Workshop ID | MB | For | Note |
|---|---|---|---|---|
| SBBG - Wrecks | `6529635700E0B228` | 3.0 | all | Vanilla wrecks made placeable (UAZ-469 and UAZ-452, Ural and M923 cargo, an M997 ambulance, a BRDM, helicopter parts): blocked roads, the column, ambush sites |
| Lamas vanilla plus gear pack | `64FD478E9982027A` | 4.2 | Bastion | Worn, era-correct US kit: dark BDUs and M65 trousers, a night-camo jacket, a raincoat, patrol and woodland caps, a UN-blue PASGT helmet |
| BanditGear | `66B523AD189F3233` | 101.9 | Red Hand | Dirty scavenged clothing, balaclavas, gloves, Molotovs, spray-painted cars |
| BasicHazmatSuit | `68AB4B626C8AB069` | 49.3 | Halcyon | Hooded hazmat suit with a gas mask in three colours. APL-ND |
| MedicalAccessories | `66052DD2B655BF72` | 12.0 | Halcyon, Haven | Medical face mask and nitrile gloves |
| Clothing_pack_v2 | `666D2C5F02F88A56` | 68.1 | Haven | Farmers, carpenters, overalls, a mechanic's uniform, prisoner uniforms (the convicts). Some hats have wrong GUIDs: don't use them |
| Domins Civilian Expansion | `630B3E95B52F31DF` | 63.1 | Haven, Halcyon | Paramedic and fireman uniforms, gas mask, oxygen tank. Brings Functional Knives |
| Timmmmmbs Garbage Composition | `62E8BD33444EA5A4` | 4.7 | Red Hand, Haven | Junk and scrap barricades. Brings IH_Better_GM_Props |
| Placeable Posters | `62B40182B1190ACF` | 12.7 | all | Posters with your own textures. Built on the demolition charge and placed like one (on a wall), not free-standing props |
| SecurityFences | `6AB0C0CC87D6EAB1` | 20.2 | Bastion, Halcyon | Mesh fence, razor coil, gates. Its keypad gates are a system, and its licence forbids changes |
| MoreGMProps | `66A920DA264A9F98` | 122.7 | Bastion | Hesco and T-walls, burn piles |

**Rejected:** Borkeds Props (`670895D328DC3AA9`, 1980s lab computers) ships old copies of vanilla
scripts and stops the game's script module compiling on 1.9 (`SCR_CampaignBuildingLayoutComponent.c`:
`TRAIT_BUILDING_OVERRIDE_KEEP_BUSH` not found). Don't add it until it's updated.

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

**Vanilla screens ReforgerZ touches.** It overrides two vanilla configs by GUID,
`chimeraMenus.conf` and `chimeraInputCommon.conf` (to add its menus and keys), and mods the pause
menu's script (`PauseMenuUI`) to add a **Notebook** button at run time, without overriding the
pause menu's layout ([UI.md](UI.md#the-notebook)). Another mod that overrides either config file
replaces ReforgerZ's, or is replaced by it, depending on load order: the notebook, talk and reader
screens then don't open. A mod that only overrides the pause menu's layout is fine, as long as
it keeps vanilla's Field Manual or Settings button (the Notebook button goes after it).

> **Naming:** "ReforgedZ" already exists and is very close to "ReforgerZ". Consider a different
> Workshop name before release ([LORE.md](LORE.md), "Name").
