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

WildlifeExpansion ([Wildlife](#wildlife)) does the same, for the same reason: what's said of Special
Zombies here goes for it too.

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
`addon.gproj` for this reason. Workbench runs on Windows and isn't affected.

**On Linux they also drop any other dependency missing from `EXTRA_ADDONS_DIR`**, with a line for
each, because a server that can't find one compiles nothing of ReforgerZ. Only Bacon Zombies must be
there (our scripts mod its classes); the rest only fill prefab slots, so the check still means
something without them, and its result line says which it went without
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#compile-check-without-workbench), which also says how to
fetch them on Linux). Open issue:
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
| Lamas vanilla plus gear pack | `64FD478E9982027A` | 4.2 | Bastion | Era-correct US kit beside vanilla's olive M65: an M1 helmet without its strap, a US raincoat; the dark patrol cap a search can turn up |
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

## Fire and weather

Dependencies in `addon.gproj` since 2026-11-13. Both load with ReforgerZ on the Linux server
(the compile check runs with them). Neither has been seen in play yet.

| Mod | Workshop ID | Size | Licence | What it adds | How it sits with ReforgerZ |
|---|---|---|---|---|---|
| [Make Fire Campfire & Wood](https://reforger.armaplatform.com/workshop/6983372A33E4D432) | `6983372A33E4D432` | 18 KB | APL-SA | A wood item, a fireplace you dig and light, and a fire you feed (about a minute per piece of wood, at most six) | Its scripts are in lowercase `scripts/`, so it doesn't break the Linux server, and it overrides no vanilla prefab. **Wood:** the mod gives it only at its own bush prefab, which no map has, so wood is a find in farms and houses (an extra find). **Building:** drop one wood, take an entrenching tool in hand (farms, garages and wrecks' boots have them), look at the wood and hold **Build fireplace** (13 s, two more wood from your pack); then light it. The fire burns people who stand in it |
| [Atmospheric Weather Mod](https://reforger.armaplatform.com/workshop/64ED6553B8AF6B62) | `64ED6553B8AF6B62` | 10.5 MB | APL-ND | New clouds, fogs, a heavy thunderstorm, strong winds, a faster weather cycle | Config only, no scripts: it replaces vanilla's weather states. ReforgerZ only reads rain, fog and wind ([DESIGN.md](DESIGN.md#weather)), so its fog and storms count as ours do: fog and rain news, storms that ground the helicopter |

A built fireplace nobody has stood by for three real days is cleared
([DESIGN.md](DESIGN.md#things-players-build)); one in use stays. Not built: a fire that draws the
infected (heat), and boiling water or cooking at a campfire.

## Wildlife

A dependency in `addon.gproj` since 2026-11-13. Not seen in play yet.

| Mod | Workshop ID | Size | Licence | What we use | What we don't |
|---|---|---|---|---|---|
| [WildlifeExpansion](https://reforger.armaplatform.com/workshop/68B3771125D0A209) | `68B3771125D0A209` | 10.4 MB | APL | Two prefabs, in **World > Wildlife**: the doe (`Prefabs/RedStagDoe.et`) and the buck (`Prefabs/RedStag_v2.et`). ReforgerZ puts herds of them out near players in the country and gives a shot deer its meat ([DESIGN.md](DESIGN.md#wildlife)) | Its herd object (we place the deer ourselves, to be able to take them away again), its Game Master entries, and its wolf, goat and test scripts, which nothing spawns |

Three things to know about it:

- **Its scripts are in `Scripts/`** (capital S), like Special Zombies', so on a Linux server it
  stops ReforgerZ loading ([above](#special-zombies-stops-reforgerz-loading-on-a-linux-server): the
  same cause, the same two ways out). The compile check, the runtime test and the zombie lab drop
  it from their throwaway `addon.gproj` with Special Zombies (the list is in
  `tools/strip_missing_deps.sh`); no ReforgerZ script names anything of the mod, so they lose
  nothing by it.
- **It's built for one machine.** A deer's moves, and its death, are worked out on each machine by
  itself and never sent: see the simplifications in [DESIGN.md](DESIGN.md#wildlife).
- **It overrides a few vanilla files:** seven blood and smoke particle files (a
  look only), and `Prefabs/Editor/Modes/EditorModeEdit.et` (Game Master's list of placeable
  things; Special Zombies, MoreGMProps and IH_Better_GM_Props change the same file, and whichever
  loads last wins there). It also mods `SCR_PlayerController` for a kill box its deer don't use.

## Scenery, night sounds and small things

Dependencies in `addon.gproj` since 2026-11-13, for dressing the world: sites, Traces scenes, mission
places and the map itself. **None has scripts**, so none can stop the Linux server compiling ours.
Each was loaded with ReforgerZ on the dedicated server (game 1.9), alone and then all five together:
the compile check passes and a boot on Arland runs with no script error of theirs. Every prefab the
kit names was spawned once on that server. **None has been seen in play**: look at each in Workbench.

What to pick from them, piece by piece, is in the **world-building kit**
([SITES.md](SITES.md#the-world-building-kit), `tools/camp_kit/world_kit.txt`).

| Mod | Workshop ID | MB | Licence | What we use | What it overrides | Checked on the server |
|---|---|---|---|---|---|---|
| [JTF Bronze Props](https://reforger.armaplatform.com/workshop/434DB1DB3BF700C4) | `434DB1DB3BF700C4` | 2.1 | APL-ND | 117 ready-made groups of base-game props (checkpoints, roadblocks, field graves, human remains, a refugee camp, a field hospital, crash sites, shot-up columns, junk and rubbish); 111 are in the kit. Its 421 "props" are Game Master entries for base-game props, so the kit names those props themselves | `Prefabs/Editor/Modes/EditorModeEdit.et` (Game Master's list of placeable things; [Wildlife](#wildlife) says who else changes it) | All 117 spawn. Graves, fence poles and plants in a group are the base game's old destructible entities: spawned at run time, each logs an error line and can't be damaged. 17 groups log a doubled `Hierarchy` component on a few parts |
| [MMHs IVY](https://reforger.armaplatform.com/workshop/5EFB27AF9491B331) | `5EFB27AF9491B331` | 23.2 | APL | 16 ivy pieces (up a wall, down from a roof edge, flat on the ground, a strand for a pole), 102 damaged props (the base game's wrecked furniture models, made placeable), and 36 presets grown over with ivy: wrecks, barricades, house ruins, two village houses, a cottage, a garage, three bunkers | Nothing. Nine of its prefabs and 77 of its materials sit at the same paths as base-game files but under their own IDs, so the game's own files stay as they are (spawned side by side: the base game's farmhouse, garage, cottage and M16A2 came up unchanged) | 154 of its 158 current pieces are in the kit. Ivy is a `Tree` entity: spawned at run time, each piece logs two error lines and can't be damaged. Four presets are left out (parts with no model, two missing materials). Last updated in 2023 |
| [Overgrown](https://reforger.armaplatform.com/workshop/68CC1E078D8243C4) | `68CC1E078D8243C4` | 12.3 | APL | Four S105 wrecks left for years, a hidden camp of 153 parts, and its own copies of nine base-game buildings and a cliff with the mod's own materials, for placing by hand where the map has room | Nothing: its 33 files at base-game paths have their own IDs too (the base game's church, farmhouse, barracks, canteen, guard tower and hospital came up unchanged beside its copies). It doesn't change the map's own buildings | 13 of 21 prefabs are in the kit. Left out: two oil rigs, a tree hide and a medical tent (errors, or no model on the server), and one wreck that names a rifle and a backpack the mod doesn't ship |
| [Dark Echoes](https://reforger.armaplatform.com/workshop/658B25CD90247D38) | `658B25CD90247D38` | 18.0 | APL-ND | The night: 20 recordings (a barn owl, bats, elk, coyotes, loons, a woodpecker, trills) worked into the base game's random outdoor sounds | Eight base-game sound files, by ID: the Everon and Kolguyev ambience, Arland's vegetation, insects, the industrial night, the two lists of random outdoor sounds, and `Sounds/Character/Character_Collisions.acp` | Loads clean. It shares no file path with Atmospheric Weather Mod or with ReforgerZ: Traces' sounds are our own prefabs ([TRACES.md](TRACES.md#sound-and-light)). Its copies of the random-sound lists and the collision sounds hold fewer base-game sounds than game 1.9's (466 and 47 fewer): listen for footsteps and falls that went quiet |
| [Suspicious Item Props](https://reforger.armaplatform.com/workshop/5DC80CC55F805CAE) | `5DC80CC55F805CAE` | 25.9 | APL | Sixteen small pick-up items. Four are in the loot ([below](#what-the-stack-gives-the-loot)): a stack of cash, a gold bar, a pearl necklace, a stack of credit cards. Ten stand at duties as display copies with nothing to pick up, and six have our own children with names and words of the time (two passports, a Soviet passport, two soldier's cards, painkillers), for errands ([ERRANDS.md](ERRANDS.md#the-sorting-duties)) | Three base-game files, by ID: `Character_FIA_Unarmed.et` (the unarmed FIA man becomes a stripped prisoner in a blindfold: nothing of ours spawns him, Game Master's FIA list does), `EditorModeEdit.et` and Workbench's `EditablePrefabsConfig.conf` | All sixteen spawn and are inventory items. Nine name an inventory icon the mod doesn't ship (`assets/weed.edds`): the passports, both military IDs, the wiring, the smartphone, the cocaine and the weed. Those stay out of the loot until they're seen in an inventory. Last updated in 2023 |

Three things to know:

- **Items, not scenery, in Suspicious Item Props.** Its prefabs are under `PrefabsEditable/Auto/Props/Civilian/Suspicious Items/`
  (a space in the path, so they can't go in the camp kit's `props.txt`). The smartphone is the wrong
  decade and stays unused; so does its arsenal box.
- **The same file path is not the same file.** Overgrown and MMHs IVY ship copies of base-game
  prefabs and materials at the base game's own paths, with new IDs. The game finds a file by its ID,
  so both live side by side. A reference with no ID (a bare path) could pick either: ReforgerZ has none.
- **Game Master's list.** JTF Bronze Props and Suspicious Item Props both replace `EditorModeEdit.et`,
  as four mods in the stack already do; whichever loads last wins there. ReforgerZ doesn't use it.

## What the stack gives the loot

No extra download: these mods are dependencies already, and some of their items are **Extra finds**
on the game mode (**Search > Extra finds**, [DESIGN.md](DESIGN.md#loot-in-houses)), each in the
kind of place it belongs. Checked against the mods' own files: every one is a pick-up item.

| From | Items | Where they turn up | What else it does |
|---|---|---|---|
| Functional Knives | Knife, machete | House, farm, body (rare) | A blade for the spear recipe |
| Metabolism | Water bottle | House, shop, farm, a wreck's boot | Drinkable; also a bottle for the Molotov recipe |
| Domins Civilian Expansion | Fire axe, gas mask, oxygen tank, fireman's helmet, paramedic jacket | Garage and farm (the axe), pharmacy, garage | The gas mask counts as a Halcyon head piece for the disguise |
| BasicHazmatSuit | Hazmat suit (hi-vis) | Pharmacy (rare) | The Halcyon coat for the disguise: findable now, not only bought or taken |
| MedicalAccessories | Face mask, blue nitrile gloves | Pharmacy | — |
| Clothing_pack_v2 | Denim and work overalls, a shirt, police trousers, a prisoner's uniform | Farm, house, police | — (its hats stay out: wrong GUIDs) |
| BanditGear | Vodka, cigarettes, a tracksuit, a baseball cap, cargo trousers, a waist pack, a hiking backpack, a balaclava | Shop, house, body, a wreck's boot | Vodka is alcohol: it cleans a fresh bite and brews the home remedy (and works as vanilla morphine does) |
| Lamas vanilla plus gear pack | US raincoat, boonie hat, M1 helmet | Farm, police (rare) | — |
| Suspicious Item Props | A stack of cash, a gold bar, a pearl necklace, a stack of credit cards | Shop and body (the cash), police (the gold bar, the cards), house (the necklace): rare in each | The Red Hand's fence buys the necklace and the gold bar at its flat price ([REPUTATION.md](REPUTATION.md#the-red-hand)): their prefab names hold two of its keywords. The cash and the cards are worth nothing yet: [missions will use them](#scenery-night-sounds-and-small-things) |

From Make Fire Campfire & Wood: wood, in farms and houses.

From WildlifeExpansion: nothing lies in houses, but a shot deer leaves raw meat ([Wildlife](#wildlife)).

From the base game, also as extra finds: a raw meat leg (farms; the stove's Cook meat), smoke
grenades, the M9 pistol and a Vz. 58 rifle (police, rare), entrenching tools, a pistol holster and
personal belongings; and the trousers, boots and Veshmeshok backpack the pool's keywords never reached.

## Not used

| Mod | Why |
|---|---|
| [DarcMissions](https://reforger.armaplatform.com/workshop/5ED0FAC84A48D018) | Its convoys, occupations and crash sites assume forces from outside; they don't fit the sealed Zone |
| [Ambient Zombies](https://reforger.armaplatform.com/workshop/692172F6EADE0B44) | Overlaps the ReforgerZ spawner, which already keeps roaming infected around each player |
| [ASAPsMeleeWeapons](https://reforger.armaplatform.com/workshop/673517886D70621E) (a baseball bat, 4 MB, APL) | Tried as a dependency on 2026-11-13 and taken out. With it loaded the Linux server compiles only vanilla scripts: nothing of ReforgerZ, Bacon Zombies or Metabolism (the compile check's sentinel fails), though the mod has no scripts of its own; the cause isn't known. Its animation set also names pistol animations game 1.9 no longer has (last built for 1.7), and the bat hits for 500 against the knives' 300. Worth another look only if Workbench on Windows loads the mode with it |
| [Melee Combat Extended Lite](https://reforger.armaplatform.com/workshop/661920E177402992) (17.7 MB, APL) | Asked for so players could punch; it has no punching (a crowbar, a tomahawk, a sledgehammer, a shield, usable bayonets). It replaces the player's animation files, two of them the same files Functional Knives replaces (`Combat.agf`, `player_main.aw`), so one mod's attacks would break; it overrides `Character_Base.et` alongside four mods in the stack, mods twenty vanilla classes (the character controller, the inventory screen, pick-up and equip actions), and keeps its scripts in `Scripts/`, the Linux problem above |
| [Congos Dynamic Lights](https://reforger.armaplatform.com/workshop/6A273E0A8061B51B) (3.2 MB, APL) | Asked for with the scenery mods on 2026-11-13 and not added: it is a second power grid, and the two can't both be right. It builds its own grid from the map's power lines, transformer stations and the diesel power plant, lights every street lamp and house light by it, and gives players **Restore**, **Disrupt** and **Shut Off Grid** at those stations. Ours decides power by town ([DESIGN.md](DESIGN.md#the-power-grid)): one station, restored by a group at its panel, and only its towns' lamps lit. They meet at the same things: the street lamps (`StreetLampComponent`: ours darkens them on each client, its own registry lights them), the station itself (ours takes the map's electrical cabinet as its control, and the mod overrides 61 base-game files, that cabinet's prefab `ElectricCabinetInterior_01.et` and the diesel generator sets among them, with the street lamps, the house lamps, the light switches and the portable generators), and the question of who has power (in its grid a place has power unless a station or a pole is down, so every town would be lit from the first night). It also mods thirteen base-game classes (the game mode, the player controller, the action menu, the interactive light, the power pole) and keeps its scripts in `Scripts/`, [the Linux problem above](#special-zombies-stops-reforgerz-loading-on-a-linux-server): with it loaded the compile check's sentinel fails, so nothing of a bridge could be compiled or run here. To adopt it later: decide that its grid is the master, feed it from ours (a bridge script that needs the mod to compile, as the hooks need Bacon Zombies), turn **World > Power grid > Street lamps** off, and test on Windows |
| DZ Restored (both listings) | Its licence allows it only on the official DZ Restored servers; 2.3 to 2.8 GB with about 60 dependencies |

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

**One vanilla prefab ReforgerZ replaces.** Vanilla's small house radio,
`Prefabs/Props/Civilian/RadioSmall_01_Base.et`, by GUID: a full copy with ReforgerZ's prompts added
beside its Turn on/off ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#notes-by-system)). Another mod that
replaces the same prefab wins or loses by load order; if it wins, the radio gets an invisible
interaction spot of ours again, a second prompt beside vanilla's, and nothing breaks.

> **Naming:** "ReforgedZ" already exists and is very close to "ReforgerZ". Consider a different
> Workshop name before release ([LORE.md](LORE.md), "Name").
