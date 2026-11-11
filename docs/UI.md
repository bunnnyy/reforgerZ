# UI and icons

The ReforgerZ HUD, menus and icon set: how they look, what each screen shows, how the scripts
feed them, and what's built. The layouts' widget trees and the icon atlas are in
[UI/LAYOUTS.md](../UI/LAYOUTS.md); the setup steps are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui). The setting and the voices used in every line of
text are in [LORE.md](LORE.md).

**Status:** the icon set, the mockups, the 12 layouts, the icon atlas and its imageset, and the
menu and input config overrides are built. In Workbench play the HUD works: the styled notice
cards, the stealth visibility bars and the noise meter show. The menus haven't been checked in
play, the background panel and the map icons aren't in the game yet, and the menu keys clash with
vanilla keys ([Built and planned](#built-and-planned)).

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
| The Long Table | `#B07A5A` muted brown | the table's wood |
| Hazard | `#F2C230` on `#141414` | infection, the broadcast, the Perimeter, world events |
| Bone, dim, panel | `#E8E2D0`, `#A39E90`, `#121416` | text and backgrounds |

**The faction marks are original.** No real organisation's insignia and no copied biohazard
trefoil:

| Mark | What it is |
|---|---|
| The Haven | The kids' bear: a battered teddy, patched, one eye stitched shut, stencilled on every safehouse door with children behind it ([LORE.md](LORE.md#the-haven)) |
| Task Force Bastion | A helmet with the days without relief chalked on as a tally: every night the call to Command ends *"Requesting relief"*, the log says *"No reply"*, and the sergeant chalks up a day ([LORE.md](LORE.md#task-force-bastion)) |
| The Red Hand | A red handprint slapped on at an angle, fingers splayed, the paint still running: the hand the boss painted over Rudi's marks on the shutter ([LORE.md](LORE.md#the-red-hand)). Deliberately not an upright heraldic hand |
| Halcyon | The plague doctor: a hooded head with a brimmed hat, one goggle and a long beak, bowed politely. The Zone calls Halcyon's people "the beaks" ([LORE.md](LORE.md#halcyon)) |
| The infected | A virion, slightly out of round, its spikes ending in knobs, with a bite taken out of it |
| The Long Table | A preserving jar with a cloth tied over it, labelled only "9". The family labels its jars by the day, not by what's in them, and Day 9 was the first soup ([LORE.md](LORE.md#the-long-table)). It isn't a faction and it's a secret, so the mark is never on the map or the standing screen: it's for the journal and the hall of fame once a player has found the family out, and scratched somewhere at the farm (planned: no script uses it yet) |

**Type.** Headings in a condensed bold, upper case with wide spacing (stencil-like); timers,
distances and channel names in a mono face; body text in a plain sans. In the game the layouts
make do with the vanilla fonts ([LAYOUTS.md](../UI/LAYOUTS.md#conventions)).

## The icon set

61 icons, hand-written SVG on a 64×64 grid with 4-unit strokes, one colour (`currentColor`) on a
transparent background, so the game can tint them. Every icon with its intended use is listed in
[UI/Icons/README.md](../UI/Icons/README.md); the contact sheet is `UI/Previews/icons_sheet.png`.

| Category | Count | Icons |
|---|---|---|
| Faction emblems | 6 | the Haven, Bastion, the Red Hand, Halcyon, the infected, the Long Table |
| Standing tiers | 8 | Stranger, Known, Trusted, Champion (dog tags with 0–3 marks), Watched (an eye), Hunted (a crosshair), Kill on Sight (a crossed-out target), and one mixed mark for Uneasy, Tolerated, Respected and Wildcard |
| Mission types | 7 | contract, world event, puzzle, leader job, unarmed, Perimeter, story arc |
| Map markers | 9 | lab, safehouse, stronghold, outpost, world event, quartermaster and leader, power station, radio station, checkpoint. Each sits on a plate whose shape says what it is: square for places factions hold, hexagon for the lab, diamond for events, circle for infrastructure, octagon for the sealed checkpoints |
| Items | 18 | key, sample, antidote, stabiliser, low-grade med, noisemaker, battery, tyre, spark plugs, fuel can, wind-up radio, molotov, bandage, tin-can tripwire, spike trap, purification tablets, planks, jewellery |
| Status and HUD | 13 | bitten, golden window, Fever, Weakness, Final, secure radio, open channel, broadcast, noise, shortage, locked, unlocked and forced door |

**Files.** Sources in `UI/Icons/src/`. White PNGs at 64 and 128 px in `UI/Icons/png/64/` and
`png/128/`. Coloured map and emblem variants (24, with a dark backing behind the plated markers'
cut-out glyphs, and one per faction for strongholds, outposts and quartermasters) in
`png/color/`. Re-render after editing: `node tools/render_ui.js icons` (needs Node and Playwright's
Chromium, nothing else). The game reads the 60 white icons from one atlas,
`UI/Textures/ReforgerZ/RZ_Icons.edds`, through the `RZ_Icons` imageset: after re-rendering, pack
them again with `tools/icon_atlas.py` ([LAYOUTS.md](../UI/LAYOUTS.md#importing-the-icons)).

## Screens

Each screen has an HTML mockup in `UI/Mockups/` over a procedural background (no screenshots),
rendered to `UI/Previews/<name>.png` at 1920×1080 (×2) with `node tools/render_ui.js screens`.

| Screen | Preview | What it shows | Script | Status |
|---|---|---|---|---|
| HUD | `hud.png` | The infection timer: the golden window countdown, then Fever, Weakness and Final on one track, a paused tag under a stabiliser, a glow in the Final stage. Notice cards: a faction-coloured header, the speaker, the text, a bar that runs down; two at once, the most important first ([Notices](#notices)). The secure-radio indicator while a cracked channel lasts. A subtle noise meter, and above it how visible you are: five segments and HIDDEN, LOW, SEEN or IN PLAIN SIGHT (only with [Stealth](DESIGN.md#stealth) on) | `RZ_HudDisplay` | Works in play: cards, visibility and noise seen; the timer and the radio indicator not seen yet |
| The notebook | — | One menu with five tabs, the five rows below, in this order ([below](#the-notebook)) | `RZ_NotebookMenu` | Frame and pages built as text, not checked in play |
| Notebook: Standing | `standing.png` | Per faction: tier, Fame, Infamy (with the tier bands marked), Favor, the leaders you've met and what they think of you, what the faction needs, a warning when it hunts you. Truces and feuds, and the tier grid | `RZ_StandingPage` | Page built, not checked in play; the tier grid is static |
| Notebook: Journal | `journal.png` | Active contracts, events, puzzles, leader jobs, arcs and questlines with type icon, giver, rough direction and distance and time left. The selected one in full: brief, where, requirements, rewards, and the puzzle clues you've read (or a questline's notes). Under the list, the lab's line: research, what waits in its backlog and how long that takes ("Research 63 percent. Waiting: 14 samples' worth, about 4 days of work.", [DESIGN.md](DESIGN.md#cure-research-and-the-seasons-end)) | `RZ_JournalPage` | Page built, not checked in play; the mockup has no lab line yet |
| Background picker | `background_picker.png` | A panel in the vanilla deploy menu: the picked background's name, perk, flaw, starting standing, starting gear, locked or available (with how to unlock), Legacy toward the Drifter. Locked loadouts are unavailable in the menu itself | `RZ_BackgroundPanelComponent`, `RZ_LoadoutLock.c` | Layout built, not in the deploy menu yet |
| Notebook: Radio | `radio_log.png` | What you heard and were told this life, newest first: the broadcast, open channels and cracked secure channels, each in its voice, and the mission and world notices; six filters (All, Open channel, Secure, Broadcast, Missions, World) | `RZ_RadioPage` | Page built, not checked in play; the channel list is static |
| Notebook: Map key | `map_legend.png` | The marker icons the map draws, with one line each | `RZ_MapKeyPage` | Static page built, not checked in play |
| Notebook: Hall of fame | — | This season's records and names, all time, and the last seasons in a few lines ([DESIGN.md](DESIGN.md#seasons)) | `RZ_HallPage` | Page built, not checked in play; no mockup |
| Notice board | — | The notes on the safehouse board you're at, newest first (author, how long ago, the note), and a box to write one with a Post button. Opened when you read the board, with **UI > Board menus** on | `RZ_NoticeBoardMenu` | Layout built; **Board menus** is off; no mockup |
| Talk | `talk.png` | A person's name and who they are, what they say in large type, your replies with what each needs ([below](#the-talk-screen)) | `RZ_TalkMenu` | Layouts built as text, not checked in play |
| Subtitles | `hud.png` | What people say outside the talk screen, at the bottom centre: the speaker in their faction colour, the line in bright bone ([below](#subtitles-and-the-reader)) | `RZ_HudDisplay` | `SubtitlePanel` built as text in the HUD layout, not checked in play |
| The reader | `reader.png` | A note, letter, order or flyer on a sheet of paper in the middle of the screen, a page at a time ([below](#subtitles-and-the-reader)) | `RZ_ReaderMenu` | Layout built as text, not checked in play |
| The map | — | The marker icons on the vanilla map, per player ([below](#the-map)) | `RZ_MapIconsUI` | Layout built, not on the map yet |

The notice board isn't a notebook tab: it opens when you read a safehouse board, and Esc, B or
the close button close it.

**Chat commands** (`RZ_ChatCommands`, client): `/note <text>` pins a note on the board you're at,
`/noteclear` and `/noteclear all` clear boards (server admins). They work without any layout.
Without the board menu, reading the board is a notice and notes are written with `/note`.

**Rough areas in the journal.** A contract's "where" and direction are the rough circle ("around
Morton (within 500 m)", "about 1.2 km NE") until the player knows the spot
([MISSIONS.md](MISSIONS.md#who-gets-offered-what)). **Mouse:** click a tab to switch, the close button to
close, a journal row to select it, the filter chip to cycle the radio filter; the wheel scrolls the
lists (they sit in a ScrollLayout). Those widgets are Buttons in the layouts. On a pad, up and
down scroll the same lists.

**Questlines in the journal.** A questline you're part of ([DESIGN.md](DESIGN.md#questlines)) is
a row of its own: the questline's title, its current stage as the "by" line, "Questline" where
the time left would be, the stage's text as the brief, "It depends on how it ends." as the
rewards, and its notes ("what you know so far") in the clues block. Its stage missions are rows
too, with their own time and pay. Both use the story arc icon (`mission_arc`); there's no
questline icon of its own.

### Notices

Notice cards come **a few at a time, by importance**: at most **UI > Notice cards at once** (2) on
screen, a new one at most every **Gap** (2 s). Waiting ones queue most important first, then
oldest first.

| Priority | Means | Examples |
|---|---|---|
| Urgent | danger to you, now | Bitten, Fever, Weakness, The end of it, Horde, THE FINAL STAND |
| High | about you, now | your background card, a locked background, your standing changed, a background unlocked, every mission notice (offered, "this is the place", done, failed, refused) |
| Normal | radio and faction voices | open and secure channels, the broadcast, faction notices, world events, anything with an unusual title |
| Low | news and welcome | at spawn: the outbreak, the broadcast, word on the roads, this season, the hall of fame, the "Radio" summary; guidance reminders for veterans |

- **At spawn** the background card comes first, then the state of the world, one by one. For
  **Calm after spawn** (4 s) after a new life only urgent and high cards show.
- **Urgent** cards skip the gap, and when the stack is full the least important card on screen is
  cut short to make room.
- A low or normal notice the Radio tab lists (radio, mission and world ones) that waits more than
  **Wait** (45 s) isn't shown: it's already in the notebook. The line under the cards counts them,
  "+3 in your notebook", until you next open the notebook or start a new life.
- At most **Notices waiting** (12) wait; past that the least important one the Radio tab lists goes
  to the notebook only (or, if none is listed, the least important is dropped).
- There's no separate notification log: the notebook's Radio tab keeps the last **Radio tab
  entries** (60) radio, mission and world notices of this life. Filters (X or F, or click the chip):
  **All, Open channel, Secure, Broadcast, Missions, World.** Missions: offers, "this is the place",
  done, failed, refused. World: world events and the state-of-the-world notes from your spawn.

How a notice's importance is known: the server tags about twenty call sites
(`NotifyPlayerAs`/`NotifyAllAs` with `RZ_ENoticeTag`: MISSION, EVENT, WORLD, URGENT, HIGH, LOW;
mission messages through `RZ_Mission.NotifyParticipants` and `NotifyInterested`); every other
notice is classified by its title on the client (`RZ_NoticeStyle.Classify`). Without the HUD the
vanilla hint queue sorts the same way, with no expiry (it has nowhere else to go).
*Simplified:* an odd title gets Normal; nothing is lost, it only waits.

### The notebook

One menu with five tabs, in this order: **Standing, Journal, Radio, Map key, Hall of fame.**

| Open it | Keyboard and mouse | Gamepad / console |
|---|---|---|
| A key | **N** (`RZ_OpenNotebook`) | — |
| The pause menu | Esc, then **Notebook** | Start, then **Notebook**, A |
| The notebook item | In the inventory, take it into your hand | The same, or from a quick slot |

- It opens on the tab you last used this session (the first time: Journal).
- LB/RB or Q/E switch tabs; clicking a tab works too. Up/down (D-pad, stick, arrows) move in the
  journal's list and scroll the other tabs' lists; A or Enter selects the journal row with the
  focus; X or F (or a click on the chip) cycles the Radio filter; B or Esc closes.
- A tab with something new since you last looked carries a count: Journal the mission notices,
  Radio everything it lists. Opening the notebook also clears the "+N in your notebook" line.
- Opened from the item, the item is put away again when the notebook closes.
- You can't move while it's open (it's a menu, like vanilla's). The world keeps running.

**How it works.** `RZ_NotebookMenu` holds the shared frame (heading, sub-line, tabs with their
counts, close button, footer prompts) and an empty `Page` frame. Each tab is a page
(`RZ_NotebookPage`: `RZ_StandingPage`, `RZ_JournalPage`, `RZ_RadioPage`, `RZ_MapKeyPage`,
`RZ_HallPage`) with its own layout (**UI > Standing page** and the rest); switching tabs removes
the old page's widgets and creates the new page's layout inside `Page`, so the menu stays open and
the pad focus is set again (the page's first row, else the open tab). A tab whose page layout is
empty says "Nothing here yet". Each page asks for its payload when it's shown and redraws when a
new copy arrives, as the old menus did. Nothing is saved; the last tab lives on the client for
the session.

**The pause menu button** (`RZ_PauseNotebook.c`, a `modded class PauseMenuUI`): when vanilla's
pause menu opens, a button named `RZ_Notebook` is used if the layout has one; otherwise one is
made from vanilla's own pause menu button layout and moved just after Field Manual, in the same
focus column. Its click closes the pause menu and opens the notebook. No layout override is
needed.

**The notebook item** (`Prefabs/ReforgerZ/UI/RZ_Notebook.et`, `RZ_NotebookGadgetComponent`): a
small notebook every new life is given a few seconds after spawn (**UI > Notebook prefab**; one
that doesn't fit lies at your feet). Taken into the hand by its owner it opens the notebook; when
the notebook closes, it goes back. *Simplified:* it's given at spawn by script rather than added
to each of the 13 loadouts, so it works with any loadout.

The notice board opens when you read a safehouse board, with **UI > Board menus** on.

### The talk screen

As approved (`talk.html`): the world stays in view, dimmed (#08090A at 55%); a panel 660 tall
across the bottom (#0F1113) with a 5 px top border in their faction colour. Inside, 240 in from
each side: their name in that colour (bold 30) and the sub-line, "The Haven - council head - knows
your name" (21, dim); what they say (38, bright bone, in a box that scrolls with LB/RB or Q/E
when it's long); your replies as a numbered list (25), the selected one lighter with an accent
bar, a dim tag on the right ("a job", "news", "trade", "20 Haven Favor"), a reply you can't take
dim with what it needs; the last is always "I'll be going.". The footer prompts sit in a column
to the right of the replies.

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Choose | up/down, or **1–7** (`RZ_Reply1`–`7`) to say one at once | D-pad or left stick |
| Say it | Enter, or click | A |
| Leave | Esc | B |
| Read more of a long line | Q/E, or the wheel | LB/RB |

How it works is in [DESIGN.md](DESIGN.md#talking-to-people). The screen reads `PAYLOAD_TALK`:
`T|serial|faction|name|sub|said` (line breaks as `^`), then `R|id|label|tag|need|enabled` per
reply; `X|reason` closes it. A pick goes back as `RpcAsk_TalkPick(serial, id)`; Esc, B or another
menu on top send Leave. If the menu can't open on a client, that client shows the line as a card,
leaves, and shows the person's actions again (`RZ_TalkMenu.Available`).

### Subtitles and the reader

**Subtitles.** What people say *outside* the talk screen shows at the **bottom centre**: a solid
dark backing (12, 14, 15 at 88%), the speaker's name on its own line in their faction colour (bold
24, bone when they have no faction), the line in bright bone (32), centred, 1,100 units wide, its
bottom 204 above the screen's (clear of the infection panel and the noise meter). The notice
cards stay top right; the two never mix. One line at a time, on screen for **Subtitle shortest**
to **longest** (2.5–8 s) by length at **reading speed** (18 characters a second); a line longer
than two rows (130 characters) is split at sentence ends into parts shown in turn. At most four
wait; a newer line from the same speaker replaces theirs still waiting. While the reader is open
they wait. Nothing to press, so nothing to bind. A spoken line goes only to its player
(`RZ_GameModeComponent.Say`, `SayNear`; `RpcDo_Say`). With **UI > Subtitles** off, without
`SubtitlePanel` in the HUD layout, or without the UI component, it's a notice card, as before.

Which lines are subtitles:

| System | Lines |
|---|---|
| `RZ_Checkpoints` | road post warnings, the search, the toll, camp warnings and the gate, the safe-zone rule, "You again" (the outcomes with numbers stay cards: a surrender's Infamy, the toll paid) |
| `RZ_Perimeter` | the loudspeakers, the final warning |
| `RZ_Territory` | routine lines (shift change, the roll, supper) |
| `RZ_Safehouses` | the shut door, the people who won't look at an exile |
| `RZ_Wanderers` | refusing, running when shot at, warning others about an exile, the one who comes looking for a new player |
| `RZ_LongTable` | the host stepping out on the road |
| `RZ_Leaders` | a refusal at attitude −2 |
| The screen guard | "Not now!" when danger closes a talk |

Everything else stays a card; radio voices stay cards (they're radio). While the talk screen is
open, the person's own lines go into it instead.

**The reader.** Notes open on a sheet of paper in the middle of the screen, as in `reader.html`: the
world dimmed; the paper (#EDE7D6, ink #1C1A16) 960 × 800: the title (bold 34), a dim line on how it
looks ("Folded twice, in pencil", 21, hidden when there's none), the text (30); at the foot the
pages "1 / 2" with arrows (LB/RB, Q/E, the D-pad left/right, or click), the footer prompts and the
close button (B or Esc). Pages break at the end of a paragraph near **Reader page characters**
(700). You can't move while it's open; danger closes it ([the screen guard](#how-the-scripts-fit)).

What opens in the reader: puzzle notes and anything a mission tags with a "Read ..." label (letters,
orders), guidance notes and safehouse flyers, and the readable things a search turns up. Bodies,
objects and people stay as they are.

How: `RZ_Reader.Show(gm, playerId, title, how, text)` opens a screen-guard entry and sends
`PAYLOAD_READER` (`P|title|how`, then the text, at most 5,600 characters) to that player only; the
client splits the pages and opens `RZ_ReaderMenu`, and tells the server when the player closes it.
An empty `PAYLOAD_READER` from the server closes it. With **UI > Reader** off, no UI component, or
the server at its cap of open screens, it's a notice card; a client that can't open the menu (no
preset or layout) shows it as a card too. *Simplified:* the "how it looks" line is set only for
flyers so far.

### The background picker

The panel reads the picked loadout twice a second. Its **starting gear** is what that loadout
really spawned with the last time anyone took it (the server notes up to ten things a few seconds
after the spawn, by prefab name); until then, the background's own gear line (`RZ_Background.m_sGear`).
A locked background's loadout is **unavailable** in the vanilla menu (**UI > Lock loadouts**, on):
`RZ_LoadoutLock.c` mods `SCR_BasePlayerLoadout`, so the client marks it unavailable from the last
backgrounds payload. The 1.9 menu leaves unavailable loadouts out of its list when it builds it and
greys their buttons when it refreshes (which one players see is a VERIFY). Vanilla never asks the
server, so the server checks on spawn: a locked pick spawns as the default background. The panel
says how to unlock it. In Workbench play the player spawned with the locked Soldier loadout and
played as the default background: the server's check works; the menu's side is still to be
checked.

### The map

`RZ_MapIconsUI` is a component on the map configuration: while the map is open it asks the server
for this player's list (on open, then every 20 s) and draws one icon per place, in its colour
(the faction's, hazard yellow for events and the sealed checkpoints, bone for the stations), with
a label, following pan and zoom. The list is what this player may know ([DESIGN.md](DESIGN.md#settlements-spawn-points-and-the-map)):
contracts are never on it. With **UI > Map icon layout** set, the vanilla text markers for
events and the lab are off; without it, the layer does nothing and those markers stay.

## Controllers

Everything in ReforgerZ must work on console (Xbox, PlayStation) and with a gamepad on PC, as well
as with keyboard and mouse. This is a rule for the whole mod, agreed with the author (CLAUDE.md,
"Design guardrails"); this section owns the details.

| Rule | What it means |
|---|---|
| Every input has a gamepad binding or a pad route | Each ReforgerZ action in `Configs/System/chimeraInputCommon.conf` binds a gamepad input next to its key, or what it does is reached another way on a pad (the table below) |
| Every screen works without a mouse | It opens from something a pad can reach, moves with the D-pad or left stick, switches tabs with the shoulder buttons (`MenuTabLeft`/`MenuTabRight`), confirms with A (`MenuSelect`) and closes with B (`MenuBack`). Each screen gives a widget the focus when it opens (`RZ_UIW.Focus`), so the D-pad has something to move from |
| Nothing needs hover, typing or a chat command | What a tooltip says also shows when the item has focus; whatever a chat command does also has a button or an action |
| Readable on a TV | Sizes at 1920×1080: no text under 18 px and body text 26 px or more on the new screens (talk, reader, subtitles); the tabs and footers of the older screens are 18 px. Nothing important within 96 px of the left and right edges or 54 px of the top and bottom (a 90% safe area). Footer prompts are vanilla's input button (`RZ_UIW.AddPrompt`, `WLib_InputNavigationButton`), which shows the key or the pad button by the device in use |

**The input actions** (all in `Configs/System/chimeraInputCommon.conf`; menus also use vanilla's
`MenuUp`/`MenuDown`, `MenuSelect`, `MenuBack` and `MenuTabLeft`/`MenuTabRight`):

| Action | Keyboard | Gamepad | Context |
|---|---|---|---|
| `RZ_OpenNotebook` | N | none: the pause menu's **Notebook** button, or the notebook item | `IngameContext` |
| `RZ_RadioFilter` | F | X | `MenuContext` |
| `RZ_Reply1` … `RZ_Reply7` | 1 … 7 | none: the D-pad and A pick a reply | `MenuContext` |

N is free in play (vanilla uses it only in the group menu and the map's pencil tool, other
contexts). The pad has no free button in play, so Start (the pause menu) and the notebook item are
its way into the notebook.

**Status.** The screens, the HUD and the chat commands were built for keyboard and mouse and
haven't been checked with a gamepad yet. A check of the whole mod, and the fixes it finds, are
planned ([ROADMAP.md](ROADMAP.md)).

## How the scripts fit

Code: `scripts/Game/ReforgerZ/UI/`. The UI only reads; nothing in it changes the game.

| Class | Runs on | Does |
|---|---|---|
| `RZ_UIFeed` | server, on the game mode's tick | Works out each player's infection state, the secure channel they hear, and a noise level (0–6 from the heat where they stand), and sends each **only when it changes**; with Stealth on, stealth's own update (every second) hands it each player's visibility, sent the same way as a level 0–5 and whether it counts as hidden (the timer also resyncs every minute, and at once when the time left jumps more than 5 s from what the client is counting down; the client counts down in between). A bite cleaned in time still shows the golden window until it would have closed, so the HUD never tells early whether the clean worked. Builds the standing, journal (with the questlines the player is part of and the lab's line), background, map, notice board and hall of fame data when asked. Remembers the clues each player has read while the puzzle runs, the outposts each player has been near (this session) and each loadout's gear |
| `RZ_PlayerUIComponent` | on the player controller | The pipe: owner-only RPCs from the server, a request RPC and a command RPC from the client (each payload kind and each command at most every 2 s; a command sent too soon gets a short notice instead of vanishing), and the client's cache. Keeps the radio log on the client for this life. A notice board payload from the server opens the board menu |
| `RZ_Dialogue`, `RZ_TalkSpeaker` (`RZ_TalkGeneric`, `RZ_TalkLeader`), `RZ_TalkNeeds` | server | Conversations: sessions, capture, replies and their needs, picks ([DESIGN.md](DESIGN.md#talking-to-people)) |
| `RZ_TalkMenu` | client (`ChimeraMenuBase`) | The talk screen ([The talk screen](#the-talk-screen)) |
| `RZ_Reader`, `RZ_ReaderMenu` | server, client | The reader: one player's paper, a page at a time ([Subtitles and the reader](#subtitles-and-the-reader)) |
| `RZ_ScreenGuard` | server, once a second | Watches open talk screens and readers and closes them in danger (below) |
| `RZ_ChatCommands` | client | `/note`, `/noteclear`, sent as commands to the server, which checks them again. Registered with the first notice |
| `RZ_HudDisplay` | client (`SCR_InfoDisplayExtended`) | Draws the HUD five times a second from the cache; shows the notice cards and the subtitles |
| `RZ_NotebookMenu` and its pages (`RZ_NotebookPage`), `RZ_NoticeBoardMenu` | client (`ChimeraMenuBase`) | Ask for their data on open (each page when it's shown), redraw when a new copy arrives; `RZ_MenuClick` passes mouse clicks to them ([The notebook](#the-notebook)) |
| `RZ_Notebook`, `RZ_PauseNotebook`, `RZ_NotebookGadgetComponent` | client | Open the notebook: N, the pause menu's button, the notebook item |
| `RZ_BackgroundPanelComponent` | client (widget handler) | Watches the picked loadout in the deploy menu and shows its background and gear |
| `SCR_BasePlayerLoadout` (modded, `RZ_LoadoutLock.c`) | server and client | Makes a locked background's loadout unavailable |
| `RZ_MapIconsUI` | client (map component) | The icon layer on the map |
| `RZ_NoticeStyle` | client | Classifies a notice by its title (faction names, leader titles, channel names) into a kind and a faction for its card and the radio log. The 600-odd `NotifyPlayer` calls are unchanged |
| `RZ_UIW`, `RZ_UIText`, `RZ_UIColor` | client | Every engine UI call in one helper; payload text; the palette |

**Data is cheap.** Pushed values are a few numbers per change, to the owning player only. Menu data
is one string (one record per line, fields split by `|`, at most 6000 characters, cut after the last whole record that fits) sent when the
menu opens; the map's every 20 s while it's open. Each kind can be asked for at most every 2 s.
Nothing is replicated to other players.

**Notices.** Every message still goes through `RZ_GameModeComponent.NotifyPlayer` (or
`NotifyPlayerAs`, with a tag). Text for one player goes **only to that player's machine**: an
owner-only RPC on their `RZ_PlayerUIComponent` (`RpcDo_Notice`; spoken lines `RpcDo_Say`). Only a
controller without the component gets the old broadcast that each client filters for itself; the
server picks one of the two per message, never both. `NotifyAll` stays a broadcast. On the client,
`QueueHint` classifies it, adds radio messages to the radio log, and offers it to the HUD. If **UI
> Styled notices** is off, or the HUD layout or the card layout is missing, the vanilla hint queue
shows it exactly as before.

**The screen guard** (`RZ_ScreenGuard`, server). The talk screen and the reader stop you moving
while the world keeps running, so the server watches every open one, once a second on the game
mode's `SecondTick`, and closes it when: you took damage or fired; an infected is within
**Dialogue > Danger near** (6 m) of you or of the person you talk to, or one within **Danger
closing** (30 m) came 2 m closer since the last check; you died, or the person is gone; and, for
talk only, you're more than **Range** (6 m) from them or **Idle seconds** (120) passed with no
reply. In danger the person says "Not now!" as a subtitle. At most **Open at most** (32) screens
are open on the server; each check is a few distance tests. The client says when the player
closes one (`RpcAsk_ScreenClosed`).

**Turning it off.** Styled notices off gives the vanilla hints back. Taking
`RZ_PlayerUIComponent` off the player controller (`Prefabs/Characters/Core/RZ_PlayerController.et`)
stops all sending. To remove the UI entirely, delete the `UI/` code folder and the lines that use
it: the `m_UISettings` attribute, `m_UIFeed` (creation, tick, spawn,
`RecordGearSoon` and accessors), the `UsesMapIcons` check in `AddMapMarker` and the block at the
top of `QueueHint` (with the chat commands' registration) in `Core/RZ_GameModeComponent.c`, the
board menu call in `World/RZ_NoticeBoard.c` (`SendMenu`), and the `OnClueRead` call in
`Puzzles/RZ_PuzzleMission.c`. `RZ_LoadoutLock.c` and `RZ_MapIcons.c` can each be deleted on their
own. Then remove what names the UI's classes: point `RZ_GameMode.et` back at the vanilla player
controller and clear its **UI** settings, and delete the two overrides in `Configs/System/` and
the layouts in `UI/layouts/ReforgerZ/`.

*Simplified:* notices are classified by their title, so an unusual title gets a plain card. The
map's icons don't cluster or scale with zoom; outposts a player has seen are remembered for the
session only.
Journal rewards are the mission's base Fame and Favor ("about"), before leader traits and
background multipliers. The tier grid on the standing screen and the radio channel list are
static parts of the layouts.

## Built and planned

| | |
|---|---|
| Built | The icon set and its renderer; six mockups and previews. In code: the feed, the HUD, six menus with mouse support (four tabs, the notice board, the hall of fame), chat commands, the deploy-menu panel with the loadout's gear, locked loadouts unavailable, the map's icon layer, styled notices with the vanilla fallback, the radio log |
| Workbench files | The 12 layouts ([LAYOUTS.md](../UI/LAYOUTS.md)); the icon atlas and the `RZ_Icons` imageset; the menu and input config overrides in `Configs/System/` (Workbench finds the six menu presets); the player controller `Prefabs/Characters/Core/RZ_PlayerController.et`, with the feed and the HUD; the game mode's **UI** settings, all but the map icon layout. What each holds is in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui) |
| Works in play | Seen in Workbench play on the repo's world, with no script exceptions: the styled notice cards, the stealth visibility bars and the noise meter |
| Not checked in play | The infection timer and the secure-radio indicator (they need a bite or a cracked channel); the menus, their keys and their mouse clicks; the notice board menu (**UI > Board menus** is off); the chat commands; the deploy menu's side of locked loadouts ([above](#the-background-picker)) |
| Not done | The override of the vanilla deploy menu's layout that holds the background panel, so the panel doesn't show. The map configuration override with `RZ_MapIconsUI`, and **UI > Map icon layout** (empty: set, it turns the vanilla text markers off), so the map shows the vanilla markers. Images for the icons the scripts never swap (the map key's, the radio log's channel list, the HUD's radio, noise and paused icons, the background panel's): they draw as flat squares in their colour |
| Planned | A sound on new notices; mockups of the notice board and the hall of fame; the Long Table's mark in the journal and the hall of fame once a player has found the family out, and at the farm |
