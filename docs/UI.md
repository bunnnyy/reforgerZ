# UI and icons

The ReforgerZ HUD, menus and icon set: how they look, what each screen shows, how the scripts
feed them, and what's built. Building the layouts in Workbench is in
[UI/LAYOUTS.md](../UI/LAYOUTS.md); the setup steps are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui). The setting and the voices used in every line of
text are in [LORE.md](LORE.md).

**Status:** the icon set and the mockups are done. The scripts compile but have never run, and the layouts don't exist yet: they must be built in Workbench. Until
then every message shows as the vanilla hint, as before.

## Art direction

The Zone looks like the signs and marks people left in it: quarantine signage, stencilled
military markings and hand-made survivor marks. **Flat, high-contrast, readable at 32 px** and on
a map. No gradients or glows in the icons; the HUD uses only a dark panel, one accent and plain type.

| Colour | Hex | Used for |
|---|---|---|
| The Haven | `#E3A33B` warm amber | candlelight in a window |
| Task Force Bastion | `#9AA35E` olive | old paint on a checkpoint |
| The Red Hand | `#C8352B` red | wet paint |
| Halcyon | `#4FB8B0` clinical teal | the lab |
| Hazard | `#F2C230` on `#141414` | infection, the broadcast, the Perimeter, world events |
| Bone, dim, panel | `#E8E2D0`, `#A39E90`, `#121416` | text and backgrounds |

**The faction marks are original.** No real organisation's insignia and no copied biohazard
trefoil:

| Mark | What it is |
|---|---|
| The Haven | A house with a candle in it: every safehouse lights one for the dead ([LORE.md](LORE.md#the-haven)) |
| Task Force Bastion | A crenellated wall with one bar across it: *hold the line* |
| The Red Hand | A paint handprint slapped on a wall at an angle, still running. Deliberately not an upright heraldic hand |
| Halcyon | A sun over still water inside a lab hexagon (the halcyon days that a cure would bring) |
| The infected | A spiked virion with a bite taken out of it |

**Type.** Headings in a condensed bold, upper case with wide spacing (stencil-like); timers,
distances and channel names in a mono face; body text in a plain sans. In the game, use the
vanilla fonts ([LAYOUTS.md](../UI/LAYOUTS.md#conventions)).

## The icon set

60 icons, hand-written SVG on a 64×64 grid with 4-unit strokes, one colour (`currentColor`) on a
transparent background, so the game can tint them. Every icon with its intended use is listed in
[UI/Icons/README.md](../UI/Icons/README.md); the contact sheet is `UI/Previews/icons_sheet.png`.

| Category | Count | Icons |
|---|---|---|
| Faction emblems | 5 | the Haven, Bastion, the Red Hand, Halcyon, the infected |
| Standing tiers | 8 | Stranger, Known, Trusted, Champion (dog tags with 0–3 marks), Watched (an eye), Hunted (a crosshair), Kill on Sight (a crossed-out target), and one mixed mark for Uneasy, Tolerated, Respected and Wildcard |
| Mission types | 7 | contract, world event, puzzle, leader job, unarmed, Perimeter, story arc |
| Map markers | 9 | lab, safehouse, stronghold, outpost, world event, quartermaster and leader, power station, radio station, checkpoint. Each sits on a plate whose shape says what it is: square for places factions hold, hexagon for the lab, diamond for events, circle for infrastructure, octagon for the sealed checkpoints |
| Items | 18 | key, sample, antidote, stabiliser, low-grade med, noisemaker, battery, tyre, spark plugs, fuel can, wind-up radio, molotov, bandage, tin-can tripwire, spike trap, purification tablets, planks, jewellery |
| Status and HUD | 13 | bitten, golden window, Fever, Weakness, Final, secure radio, open channel, broadcast, noise, shortage, locked, unlocked and forced door |

**Files.** Sources in `UI/Icons/src/`. White PNGs at 64 and 128 px in `UI/Icons/png/64/` and
`png/128/`. Coloured map and emblem variants (23, with a dark backing behind the plated markers'
cut-out glyphs, and one per faction for strongholds, outposts and quartermasters) in
`png/color/`. Re-render after editing: `node tools/render_ui.js icons` (needs Node and Playwright's
Chromium, nothing else).

## Screens

Each screen has an HTML mockup in `UI/Mockups/` over a procedural background (no screenshots),
rendered to `UI/Previews/<name>.png` at 1920×1080 (×2) with `node tools/render_ui.js screens`.

| Screen | Preview | What it shows | Script | Status |
|---|---|---|---|---|
| HUD | `hud.png` | The infection timer: the golden window countdown, then Fever, Weakness and Final on one track, a paused tag under a stabiliser, a glow in the Final stage. Notice cards: a faction-coloured header, the speaker, the text, a bar that runs down; up to three at once. The secure-radio indicator while a cracked channel lasts. A subtle noise meter, and above it how visible you are: five segments and HIDDEN, LOW, SEEN or IN PLAIN SIGHT (only with [Stealth](DESIGN.md#stealth) on) | `RZ_HudDisplay` | Written |
| Standing | `standing.png` | Per faction: tier, Fame, Infamy (with the tier bands marked), Favor, the leaders you've met and what they think of you, what the faction needs, a warning when it hunts you. Truces and feuds, and the tier grid | `RZ_StandingMenu` | Written; the tier grid is static |
| Journal | `journal.png` | Active contracts, events, puzzles, leader jobs, arcs and questlines with type icon, giver, rough direction and distance and time left. The selected one in full: brief, where, requirements, rewards, and the puzzle clues you've read (or a questline's notes) | `RZ_JournalMenu` | Written |
| Background picker | `background_picker.png` | A panel in the vanilla deploy menu: the picked background's name, perk, flaw, starting standing, starting gear, locked or available (with how to unlock), Legacy toward the Drifter. Locked loadouts are unavailable in the menu itself | `RZ_BackgroundPanelComponent`, `RZ_LoadoutLock.c` | Written |
| Radio log | `radio_log.png` | What you heard this life, newest first: the broadcast, open channels and cracked secure channels, each in its voice; a filter | `RZ_RadioLogMenu` | Written; the channel list is static |
| Map key | `map_legend.png` | The marker icons in use, with one line each | `RZ_MapKeyMenu` | Layout only |
| Notice board | — | The notes on the safehouse board you're at, newest first (author, how long ago, the note), and a box to write one with a Post button. Opened when you read the board, with **UI > Board menus** on | `RZ_NoticeBoardMenu` | Written; no mockup |
| Hall of fame | — | This season's records and names, all time, and the last seasons in a few lines ([DESIGN.md](DESIGN.md#seasons)) | `RZ_HallOfFameMenu` | Written; no mockup |
| The map | — | The marker icons on the vanilla map, per player ([below](#the-map)) | `RZ_MapIconsUI` | Written |

The menus share one frame: Standing, Journal, Radio and Map key are tabs (Q/E), Esc closes. Keys:
**K** standing, **J** journal, **U** radio, **F** filter in the radio log, **O** the hall of fame
(input actions you add; [LAYOUTS.md](../UI/LAYOUTS.md#menus)). The notice board and the hall of
fame aren't tabs: Esc or the close button closes them.

**Chat commands** (`RZ_ChatCommands`, client): `/note <text>` pins a note on the board you're at,
`/noteclear` and `/noteclear all` clear boards (server admins), `/hall` shows the hall of fame
as a notice. They work without any layout. Without the board menu, reading the board is a
notice and notes are written with `/note`.

**Rough areas in the journal.** A contract's "where" and direction are the rough circle ("around
Morton (within 500 m)", "about 1.2 km NE") until the player knows the spot
([MISSIONS.md](MISSIONS.md#who-gets-offered-what)). **Mouse:** click a tab to switch, the close button to
close, a journal row to select it, the filter chip to cycle the radio filter; the wheel scrolls the
lists (they sit in a ScrollLayout). Those widgets are Buttons in the layouts.

**Questlines in the journal.** A questline you're part of ([DESIGN.md](DESIGN.md#questlines)) is
a row of its own: the questline's title, its current stage as the "by" line, "Questline" where
the time left would be, the stage's text as the brief, "It depends on how it ends." as the
rewards, and its notes ("what you know so far") in the clues block. Its stage missions are rows
too, with their own time and pay. Both use the story arc icon (`mission_arc`); there's no
questline icon of its own.

### The background picker

The panel reads the picked loadout twice a second. Its **starting gear** is what that loadout
really spawned with the last time anyone took it (the server notes up to ten things a few seconds
after the spawn, by prefab name); until then, the background's own gear line (`RZ_Background.m_sGear`).
A locked background's loadout is **unavailable** in the vanilla menu (**UI > Lock loadouts**, on):
`RZ_LoadoutLock.c` mods `SCR_BasePlayerLoadout`, so the client marks it unavailable from the last
backgrounds payload. The 1.9 menu leaves unavailable loadouts out of its list when it builds it and
greys their buttons when it refreshes (which one players see is a VERIFY). Vanilla never asks the
server, so the server checks on spawn: a locked pick spawns as the default background. The panel
says how to unlock it.

### The map

`RZ_MapIconsUI` is a component on the map configuration: while the map is open it asks the server
for this player's list (on open, then every 20 s) and draws one icon per place, in its colour
(the faction's, hazard yellow for events and the sealed checkpoints, bone for the stations), with
a label, following pan and zoom. The list is what this player may know ([DESIGN.md](DESIGN.md#settlements-spawn-points-and-the-map)):
contracts are never on it. With **UI > Map icon layout** set, the vanilla text markers for
events and the lab are off; without it, the layer does nothing and those markers stay.

## How the scripts fit

Code: `scripts/Game/ReforgerZ/UI/`. The UI only reads; nothing in it changes the game.

| Class | Runs on | Does |
|---|---|---|
| `RZ_UIFeed` | server, on the game mode's tick | Works out each player's infection state, the secure channel they hear, and a noise level (0–6 from the heat where they stand), and sends each **only when it changes**; with Stealth on, stealth's own update (every second) hands it each player's visibility, sent the same way as a level 0–5 and whether it counts as hidden (the timer also resyncs every minute, and at once when the time left jumps more than 5 s from what the client is counting down; the client counts down in between). A bite cleaned in time still shows the golden window until it would have closed, so the HUD never tells early whether the clean worked. Builds the standing, journal (with the questlines the player is part of), background, map, notice board and hall of fame data when asked. Remembers the clues each player has read while the puzzle runs, the outposts each player has been near (this session) and each loadout's gear |
| `RZ_PlayerUIComponent` | on the player controller | The pipe: owner-only RPCs from the server, a request RPC and a command RPC from the client (each payload kind and each command at most every 2 s; a command sent too soon gets a short notice instead of vanishing), and the client's cache. Keeps the radio log on the client for this life. A notice board payload from the server opens the board menu |
| `RZ_ChatCommands` | client | `/note`, `/noteclear`, `/hall`, sent as commands to the server, which checks them again. Registered with the first notice |
| `RZ_HudDisplay` | client (`SCR_InfoDisplayExtended`) | Draws the HUD five times a second from the cache; shows the notice cards |
| `RZ_StandingMenu`, `RZ_JournalMenu`, `RZ_RadioLogMenu`, `RZ_MapKeyMenu`, `RZ_NoticeBoardMenu`, `RZ_HallOfFameMenu` | client (`ChimeraMenuBase`) | Ask for their data on open, redraw when a new copy arrives; `RZ_MenuClick` passes mouse clicks to them |
| `RZ_BackgroundPanelComponent` | client (widget handler) | Watches the picked loadout in the deploy menu and shows its background and gear |
| `SCR_BasePlayerLoadout` (modded, `RZ_LoadoutLock.c`) | server and client | Makes a locked background's loadout unavailable |
| `RZ_MapIconsUI` | client (map component) | The icon layer on the map |
| `RZ_NoticeStyle` | client | Classifies a notice by its title (faction names, leader titles, channel names) into a kind and a faction for its card and the radio log. The 600-odd `NotifyPlayer` calls are unchanged |
| `RZ_UIW`, `RZ_UIText`, `RZ_UIColor` | client | Every engine UI call in one helper; payload text; the palette |

**Data is cheap.** Pushed values are a few numbers per change, to the owning player only. Menu data
is one string (one record per line, fields split by `|`, at most 6000 characters, cut after the last whole record that fits) sent when the
menu opens; the map's every 20 s while it's open. Each kind can be asked for at most every 2 s.
Nothing is replicated to other players.

**Notices.** Every message still goes through `RZ_GameModeComponent.NotifyPlayer` and its
broadcast RPC. On the client, `QueueHint` classifies it, adds radio messages to the radio log, and
offers it to the HUD. If **UI > Styled notices** is off, or the HUD layout or the card layout is
missing, the vanilla hint queue shows it exactly as before.

**Turning it off.** Styled notices off gives the vanilla hints back. Leaving
`RZ_PlayerUIComponent` off the controller stops all sending. To remove the UI entirely, delete
`UI/` and the lines that use it: the `m_UISettings` attribute, `m_UIFeed` (creation, tick, spawn,
`RecordGearSoon` and accessors), the `UsesMapIcons` check in `AddMapMarker` and the block at the
top of `QueueHint` (with the chat commands' registration) in `Core/RZ_GameModeComponent.c`, the
board menu call in `World/RZ_NoticeBoard.c` (`SendMenu`), and the `OnClueRead` call in
`Puzzles/RZ_PuzzleMission.c`. `RZ_LoadoutLock.c` and `RZ_MapIcons.c` can each be deleted on their
own.

*Simplified:* notices are classified by their title, so an unusual title gets a plain card. The
map's icons don't cluster or scale with zoom; outposts a player has seen are remembered for the
session only.
Journal rewards are the mission's base Fame and Favor ("about"), before leader traits and
background multipliers. The tier grid on the standing screen and the radio channel list are
static parts of the layouts.

## Built and planned

| | |
|---|---|
| Built | The icon set and its renderer; six mockups and previews; the feed, the HUD, six menus with mouse support (four tabs, the notice board, the hall of fame), chat commands, the deploy-menu panel with the loadout's gear, locked loadouts unavailable, the map's icon layer, styled notices with the vanilla fallback, the radio log (all compile, never run) |
| Workbench | The layouts, the imageset, the menu and input config entries, adding the component, the info display and the map component ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui)) |
| Planned | A sound on new notices; mockups of the notice board and the hall of fame |
