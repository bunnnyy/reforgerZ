# UI and icons

The ReforgerZ HUD, menus and icon set: how they look, what each screen shows, how the scripts
feed them, and what's built. Building the layouts in Workbench is in
[UI/LAYOUTS.md](../UI/LAYOUTS.md); the setup steps are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui). The setting and the voices used in every line of
text are in [LORE.md](LORE.md).

**Status:** the icon set and the mockups are done. The scripts are written but, like everything
else, **never compiled**, and the layouts don't exist yet: they must be built in Workbench. Until
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
| HUD | `hud.png` | The infection timer: the golden window countdown, then Fever, Weakness and Final on one track, a paused tag under a stabiliser, a glow in the Final stage. Notice cards: a faction-coloured header, the speaker, the text, a bar that runs down; up to three at once. The secure-radio indicator while a cracked channel lasts. A subtle noise meter | `RZ_HudDisplay` | Written |
| Standing | `standing.png` | Per faction: tier, Fame, Infamy (with the tier bands marked), Favor, the leaders you've met and what they think of you, what the faction needs, a warning when it hunts you. Truces and feuds, and the tier grid | `RZ_StandingMenu` | Written; the tier grid is static |
| Journal | `journal.png` | Active contracts, events, puzzles, leader jobs and arcs with type icon, giver, rough direction and distance and time left. The selected one in full: brief, where, requirements, rewards, and the puzzle clues you've read | `RZ_JournalMenu` | Written |
| Background picker | `background_picker.png` | A panel in the vanilla deploy menu: the picked background's name, perk, flaw, starting standing, locked or available (with how to unlock), Legacy toward the Drifter | `RZ_BackgroundPanelComponent` | Written; hiding locked loadouts and listing the loadout's gear are planned |
| Radio log | `radio_log.png` | What you heard this life, newest first: the broadcast, open channels and cracked secure channels, each in its voice; a filter | `RZ_RadioLogMenu` | Written; the channel list is static |
| Map key | `map_legend.png` | The marker icons in use, with one line each | `RZ_MapKeyMenu` | Layout only; the map still uses vanilla markers (planned: these icons on the map) |

The menus share one frame: Standing, Journal, Radio and Map key are tabs (Q/E), Esc closes. Keys:
**K** standing, **J** journal, **U** radio, **F** filter in the radio log (input actions you add;
[LAYOUTS.md](../UI/LAYOUTS.md#menus)).

## How the scripts fit

Code: `Scripts/Game/ReforgerZ/UI/`. The UI only reads; nothing in it changes the game.

| Class | Runs on | Does |
|---|---|---|
| `RZ_UIFeed` | server, on the game mode's tick | Works out each player's infection state, the secure channel they hear, and a noise level (0–6 from the heat where they stand), and sends each **only when it changes** (the timer also resyncs every minute; the client counts down in between). Builds the standing, journal and background data when a menu asks. Remembers the clues each player has read while the puzzle runs |
| `RZ_PlayerUIComponent` | on the player controller | The pipe: owner-only RPCs from the server, one request RPC from the client (at most every 2 s), and the client's cache. Keeps the radio log on the client for this life |
| `RZ_HudDisplay` | client (`SCR_InfoDisplay`) | Draws the HUD five times a second from the cache; shows the notice cards |
| `RZ_StandingMenu`, `RZ_JournalMenu`, `RZ_RadioLogMenu`, `RZ_MapKeyMenu` | client (`ChimeraMenuBase`) | Ask for their data on open, redraw when a new copy arrives |
| `RZ_BackgroundPanelComponent` | client (widget handler) | Watches the picked loadout in the deploy menu and shows its background |
| `RZ_NoticeStyle` | client | Classifies a notice by its title (faction names, leader titles, channel names) into a kind and a faction for its card and the radio log. The 600-odd `NotifyPlayer` calls are unchanged |
| `RZ_UIW`, `RZ_UIText`, `RZ_UIColor` | client | Every engine UI call in one helper; payload text; the palette |

**Data is cheap.** Pushed values are a few numbers per change, to the owning player only. Menu data
is one string (one record per line, fields split by `|`, at most 6000 characters) sent when the
menu opens. Nothing is replicated to other players.

**Notices.** Every message still goes through `RZ_GameModeComponent.NotifyPlayer` and its
broadcast RPC. On the client, `QueueHint` classifies it, adds radio messages to the radio log, and
offers it to the HUD. If **UI > Styled notices** is off, or the HUD layout or the card layout is
missing, the vanilla hint queue shows it exactly as before.

**Turning it off.** Styled notices off gives the vanilla hints back. Leaving
`RZ_PlayerUIComponent` off the controller stops all sending. To remove the UI entirely, delete
`UI/` and the lines that use it: the `m_UISettings` attribute, `m_UIFeed` (creation, tick, spawn
and accessors) and the block at the top of `QueueHint` in `Core/RZ_GameModeComponent.c`, and the
`OnClueRead` call in `Puzzles/RZ_PuzzleMission.c`.

*Simplified:* notices are classified by their title, so an unusual title gets a plain card.
Journal rewards are the mission's base Fame and Favor ("about"), before leader traits and
background multipliers. The tier grid on the standing screen and the radio channel list are
static parts of the layouts.

## Built and planned

| | |
|---|---|
| Built | The icon set and its renderer; six mockups and previews; the feed, the HUD, four menus, the deploy-menu panel, styled notices with the vanilla fallback, the radio log (all written, never compiled) |
| Workbench | The layouts, the imageset, the menu and input config entries, adding the component and the info display ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui)) |
| Planned | The new icons on the map markers; hiding locked loadouts in the deploy menu; listing a loadout's gear; mouse and button support in the menus (keys only for now); a sound on new notices |
