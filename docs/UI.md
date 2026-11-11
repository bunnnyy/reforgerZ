# UI and icons

The ReforgerZ HUD, menus and icon set: how they look, what each screen shows, how the scripts
feed them, and what's built. The layouts' widget trees and the icon atlas are in
[UI/LAYOUTS.md](../UI/LAYOUTS.md); the setup steps are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui). The setting is in [LORE.md](LORE.md), and the voices used in
every line of text are in [WRITING.md](WRITING.md).

**Status:** the icon set, the mockups, the layouts, the icon atlas and its imageset, and the
menu and input config overrides are built. In Workbench play the HUD works: the styled notice
cards, the stealth visibility bars and the noise meter show. The notebook, the talk screen, the
reader, the car repair screen, the shop screen, the subtitles and notice pacing are written (their
layouts as text) and compile, but haven't been played; the background panel and the map icons aren't in the game yet
([Built and planned](#built-and-planned)).

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

62 icons, hand-written SVG on a 64×64 grid with 4-unit strokes, one colour (`currentColor`) on a
transparent background, so the game can tint them. Every icon with its intended use is listed in
[UI/Icons/README.md](../UI/Icons/README.md); the contact sheet is `UI/Previews/icons_sheet.png`.

| Category | Count | Icons |
|---|---|---|
| Faction emblems | 6 | the Haven, Bastion, the Red Hand, Halcyon, the infected, the Long Table |
| Standing tiers | 8 | Stranger, Known, Trusted, Champion (dog tags with 0–3 marks), Watched (an eye), Hunted (a crosshair), Kill on Sight (a crossed-out target), and one mixed mark for Uneasy, Tolerated, Respected and Wildcard |
| Mission types | 7 | contract, world event, puzzle, leader job, unarmed, Perimeter, story arc |
| Map markers | 10 | lab, safehouse, stronghold, outpost, world event, quartermaster and leader, power station, radio station, checkpoint, and the range ring. Each but the ring sits on a plate whose shape says what it is: square for places factions hold, hexagon for the lab, diamond for events, circle for infrastructure, octagon for the sealed checkpoints. The range ring (a broken ring with three claw marks, no plate) is a pack's ground: the map scales it to the ground, and the journal's pack rows use it |
| Items | 18 | key, sample, antidote, stabiliser, low-grade med, noisemaker, battery, tyre, spark plugs, fuel can, wind-up radio, molotov, bandage, tin-can tripwire, spike trap, purification tablets, planks, jewellery |
| Status and HUD | 13 | bitten, golden window, Fever, Weakness, Final, secure radio, open channel, broadcast, noise, shortage, locked, unlocked and forced door |

**Files.** Sources in `UI/Icons/src/`. White PNGs at 64 and 128 px in `UI/Icons/png/64/` and
`png/128/`. Coloured map and emblem variants (25, with a dark backing behind the plated markers'
cut-out glyphs, and one per faction for strongholds, outposts and quartermasters) in
`png/color/`. Re-render after editing: `node tools/render_ui.js icons` (needs Node and Playwright's
Chromium, nothing else). The game reads the white icons from one atlas,
`UI/Textures/ReforgerZ/RZ_Icons.edds`, through the `RZ_Icons` imageset: after re-rendering, pack
them again with `tools/icon_atlas.py` ([LAYOUTS.md](../UI/LAYOUTS.md#importing-the-icons)). The atlas
PNG holds all 62; the imported `.edds` and the imageset are still the 61 before the range ring until
it's imported again (steps 2 and 3 there).

## Screens

Each screen has an HTML mockup in `UI/Mockups/` over a procedural background (no screenshots),
rendered to `UI/Previews/<name>.png` at 1920×1080 (×2) with `node tools/render_ui.js screens`.

| Screen | Preview | What it shows | Script | Status |
|---|---|---|---|---|
| HUD | `hud.png` | The infection timer: the golden window countdown, then Fever, Weakness and Final on one track, a paused tag under a stabiliser, a glow in the Final stage. Notice cards: a faction-coloured header, the speaker, the text, a bar that runs down; two at once, the most important first ([Notices](#notices)). The secure-radio indicator while a cracked channel lasts. A subtle noise meter, and above it how visible you are: five segments and HIDDEN, LOW, SEEN or IN PLAIN SIGHT (only with [Stealth](DESIGN.md#stealth) on). Under the noise meter during the cure finale's stand, within 150 m of the lab: the stand's line (`StandPanel`, `StandText`: *"CLEAN ROOM: BASTION. THE HAVEN TAKING: 0:35 OF 1:00."*, routes never players; sent by `RZ_PlayerUIComponent.SendStand` every 2 s, gone 5 s after the last, [FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room)) | `RZ_HudDisplay` | Works in play: cards, visibility and noise seen; the timer, the radio indicator and the stand's line not seen yet |
| The notebook | — | One menu with five tabs, the five rows below, in this order ([below](#the-notebook)) | `RZ_NotebookMenu` | Frame and pages built as text, not checked in play |
| Notebook: Standing | `standing.png` | Per faction: the reputation's name (the tier icon beside it) and its note (what it counts as; for the Wildcard tier "COUNTS AS KNOWN - SEARCHED - TOLL 20"), Fame, Infamy (with the tier bands marked, and "36 - 12 TO FORGIVE" while there's Infamy), Favor, the leaders you've met and what they think of you, what the faction needs, a warning when it hunts you. Truces and feuds, and the reputation grid: the 16 names, with each faction's initial (H, B, R, C) in the cell you stand in ([REPUTATION.md](REPUTATION.md#reputations)) | `RZ_StandingPage` | Page built, not checked in play |
| Notebook: Journal | `journal.png` | Active contracts, events, puzzles, leader jobs, arcs and questlines with type icon, giver, rough direction and distance and time left. The selected one in full: brief, where, requirements, rewards, and the puzzle clues you've read (or a questline's notes). Under the list, the lab's line: research, what waits in its backlog and how long that takes ("Research 63 percent. Waiting: 14 samples' worth, about 4 days of work.", [DESIGN.md](DESIGN.md#cure-research-and-the-seasons-end)) | `RZ_JournalPage` | Page built, not checked in play; the mockup has no lab line yet |
| Background picker | `background_picker.png` | A panel in the vanilla deploy menu: the picked background's name, perk, flaw, starting standing, starting gear, locked or available (with how to unlock), Legacy toward the Drifter. Locked loadouts are unavailable in the menu itself | `RZ_BackgroundPanelComponent`, `RZ_LoadoutLock.c` | Layout built, not in the deploy menu yet |
| Notebook: Radio | `radio_log.png` | What you heard and were told this life, newest first: the broadcast, open channels and cracked secure channels, each in its voice, and the mission and world notices; six filters (All, Open channel, Secure, Broadcast, Missions, World) | `RZ_RadioPage` | Page built, not checked in play; the channel list is static |
| Notebook: Map key | `map_legend.png` | The marker icons the map draws, with one line each | `RZ_MapKeyPage` | Static page built, not checked in play |
| Notebook: Hall of fame | — | This season's records and names, all time, and the last seasons in a few lines ([DESIGN.md](DESIGN.md#seasons)) | `RZ_HallPage` | Page built, not checked in play; no mockup |
| Notice board | — | The notes on the safehouse board you're at, newest first (author, how long ago, the note), a box to write one with a Pin it up button, and seven ready-made notes for a pad. Opened when you read the board, with **UI > Board menus** on (the default) | `RZ_NoticeBoardMenu` | Layout built, not checked in play; no mockup |
| Talk | `talk.png` | A person's name and who they are, what they say in large type, your replies with what each needs ([below](#the-talk-screen)) | `RZ_TalkMenu` | Layouts built as text, not checked in play |
| Subtitles | `hud.png` | What people say outside the talk screen, at the bottom centre: the speaker in their faction colour, the line in bright bone ([below](#subtitles-and-the-reader)) | `RZ_HudDisplay` | `SubtitlePanel` built as text in the HUD layout, not checked in play |
| The reader | `reader.png` | A note, letter, order or flyer on a sheet of paper in the middle of the screen, a page at a time ([below](#subtitles-and-the-reader)) | `RZ_ReaderMenu` | Layout built as text, not checked in play |
| Car repair | `car_repair.png` | A fixable car's job card (each job, what it needs, the button that does it) beside a side view of the car with each job lit where it is, and Start the engine ([below](#the-car-repair-screen)) | `RZ_CarMenu` | Layouts built as text, not checked in play |
| Shop | `shop.png` | A quartermaster's stock in tabs, each item with its Favor price and the standing it needs, a picture of the selected item and a details panel with Buy ([below](#the-shop-screen)) | `RZ_ShopMenu` | Layouts built as text, not checked in play |
| The map | — | The marker icons on the vanilla map, per player ([below](#the-map)) | `RZ_MapIconsUI` | Layout built, not on the map yet |

The notice board isn't a notebook tab: it opens when you read a safehouse board, and Esc, B or
the close button close it. Left and right put a ready-made note in its box and A pins it up, so
a pad writes a note without typing; a server admin clears the board with X (Delete), as
`/noteclear` does.

**Chat commands** (`RZ_ChatCommands`, client) are extras; each has a way without typing
([Controllers](#controllers)). `/note <text>` pins a note on the board you're at (the board menu);
`/noteclear` and `/noteclear all` clear boards (server admins; the board menu's X clears one);
`/tribes` (server admins) writes the infected to the server log (`/tribes stir` and `/tribes site`
are play-test aids; the status log prints the summary anyway: [INFECTED.md](INFECTED.md#admin-visibility));
`/rep <name>` (server admins) reads a player's reputation ([REPUTATION.md](REPUTATION.md#reputations));
`/talkdebug` (server admins) reads the conversation trees of the person you're talking to; `/traces`
(server admins) writes the door marks near you to the server log, with `town`, `write` and `clear`
as play-test aids ([TRACES.md](TRACES.md#admin-saving-and-the-log)); `/mydead
hidden` or `/mydead shown` keeps your name out of public text if you die out there
([INFECTED.md](INFECTED.md#your-dead-run)). They work without any layout. With **Board menus** off,
reading the board is a notice and notes are written with `/note` only.

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
| Urgent | danger to you, now | Bitten, Fever, Weakness, The end of it, Horde, THE FINAL STAND, the clean room changing hands and overtime ([FINALE.md](FINALE.md)) |
| High | about you, now | your background card, a locked background, your standing changed, a background unlocked, every mission notice (offered, "this is the place", done, failed, refused), the finale's lock card ("[Point of no return]") and "Written down", the batch and the seal calls |
| Normal | radio and faction voices | open and secure channels, the broadcast, faction notices, world events, anything with an unusual title |
| Low | news and welcome | at spawn: the outbreak, the broadcast, word on the roads, this season, the hall of fame, the "Radio" summary; guidance reminders for veterans; the finale's stand line within 150 m of the lab ("CLEAN ROOM: Bastion. The Haven taking: 0:35 of 0:60.") |

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

**Notice lanes** sit on the server, above this pacing: they decide whether something that speaks to
a player unasked (a leader catching your eye, the Long Table's host on the road, later radio offers,
wanderers and errands) may speak at all, at most one such contact every 4 minutes. Urgent notices
and anything you asked for are never held. The rules are in [DESIGN.md](DESIGN.md#notice-lanes).

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

**The journal's errands and detail.** A held errand is a journal row of its own (kind ERRAND, the
unarmed icon until `mission_errand` is drawn): the giver as "by", where, the direction (to the
step's place, then back to the giver), the real minutes before it lapses, and what you said you'd
do. Never on the map. Selecting any row asks the server for that row's **detail**
(`RZ_PlayerUIComponent.AskDetail`, payload kind 10, `D|id|told|found|went`, at most 1,500
characters, a few a second at most): its "What you've found" and "Who it went to" go at the top of
the clues block (`DetailClues`), so the journal's 6,000-character list is never cut for them.

**The packs in the journal.** Each pack your character knows of is a row of kind PACK (the range ring
icon, `marker_range`; its id `-(200000 + pack)`), listed by level: Mapped first, then Read, then Heard
of. The place is the title, the level is "by" and stands where a job's time left goes, there's a
direction to its rough area, and the names you've heard under the place with what you know as its
brief and detail; the rewards line says what each level shows. Your last character's fate is a row
too ([INFECTED.md](INFECTED.md#knowing)). On the map each is a range ring ([below](#the-map)).

**Read on doors.** Once your character has read a door mark ([TRACES.md](TRACES.md#reading-a-door)), the
journal has one row of kind DOORS (the puzzle icon, `mission_puzzle`; its id `-300000`): the title
"Read on doors", "N read" where a job's time left goes, the newest line as its brief; its detail lists
the last 12 lines, newest first, each with the town and the day. It goes with the life.

**Door marks as decals.** With **World > Traces > Door drawing** on *Decals* and the kit's atlas set,
the HUD's update (`RZ_HudDisplay`) also draws the marked doors near you as decals on their walls
(`RZ_DoorMarkClient`, every 2 s, within 70 m), from a short list the game mode replicates. Nothing to
press: a door's words come by standing at it ([TRACES.md](TRACES.md#doors-you-can-see)).

The notice board opens when you read a safehouse board, with **UI > Board menus** on (the default).

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

**With Dialogue > Trees on** ([DIALOGUE.md](DIALOGUE.md#trees-hubs-topics-and-nodes)) the same
screen and payload carry the trees, with no new widgets: the person's built replies, then their
topics (a "heard" tag on one you've had), **Something else.** for the next page of topics; in a
node, its choices with the tag the check or price writes ("[Persuade - Haven - 1 of 2]", "[Fear 1
of 2 - +3 Infamy]", "[2 food]", "[Doctor]"), dim with what it needs when you can't take one ("you
have 1 food", "you've tried that"). What they say after a check opens with the result
("[Persuaded]"). Tags are cut at 40 characters.

How it works is in [DIALOGUE.md](DIALOGUE.md#the-talk-screen). The screen reads `PAYLOAD_TALK`:
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

### The car repair screen

As approved (`car_repair.html`): **Look over the car** at a fixable car ([DESIGN.md](DESIGN.md#cars-found-rare-and-hard-to-fix))
opens a panel 1,440 × 820 in the middle of the screen, the world dimmed behind it. The header is the
car ("The blue two-door") and "2 of 5 done". On the left, a side view of the car facing left (a
white line drawing, tinted) with five numbered markers where the jobs are: 1 spark plugs at the
front of the bonnet, 2 the battery by the windscreen, 3 the tyre on the front wheel, 4 fuel above
the rear wheel, 5 the ignition behind the windscreen. Under it, what the last job said. On the
right the job card, one row per job with the same number and colour: its name, what you need or
what you have ("In your bag", "None on you", "Needs a wrench or a toolkit", "No keys. About 45 s,
loud, can fail") and the button that does it (Fit, Pour, Hotwire, Use the key). A job under way
shows "Fitting... 6 s" and a thin bar. Olive is done, amber you can do now (or are doing), grey you
need something first. The selected row is lighter with an amber bar, and its marker on the car
gets a ring. **Start the engine** sits bottom right, dim with what it still needs until it can go.

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Pick a job | up/down | D-pad or left stick |
| Do it | Enter, or click the row | A |
| Start the engine | **T** (`RZ_CarStart`), or click the button | Y |
| Close | Esc, or the close button | B |

You can't move while it's open; the world keeps running. A job you start goes on after the screen
closes, as long as you stay close (the timed action's own rule). The screen guard closes it in
danger, more than **Dialogue > Range** (6 m) from the car, or after **Idle seconds** (120) with no
pick ([How the scripts fit](#how-the-scripts-fit)).

**How it works.** The look reaches the server (`RZ_Cars.Inspect`), which opens a screen-guard entry
and sends `PAYLOAD_CAR` to that player: `C|serial|car|done|5|status|can start|what start needs`,
`I|what a look says` (the old notice, for the fallback), `W|job|seconds left|seconds` while a job
is under way, and `J|job|state|name|need|button` per job; `X|reason` closes it. A pick goes back as
`RpcAsk_CarJob(serial, job)`, at most two a second. The server checks the screen is open on that
serial, the car is still there and you're within range and not busy, then runs the car's own code
(`Fit` for one part, the hotwire or the key, `TryStart`); what it would have said as a notice is
caught (`RZ_ScreenCapture`) and becomes the status line, and the screen is sent again. A fit or a
hotwiring that finishes or stops sends it again too; starting the car closes it and its line is a
card. **Fallbacks:** with **UI > Car job layout** empty, a client that can't open the menu (it says
so, shows the look as a card, and uses the actions from then on), or the server at its cap of open
screens (the client gets `F|busy` and shows the actions for a minute), the car's prompts and
notices work as before. The client hides the car's prompts only while it would use the screen
(`RZ_CarMenu.HidesSlotsOn`: the spot's read label is "Look over the car").

**The picture** is `UI/Textures/ReforgerZ/RZ_CarSide.png` (1,440 × 480, white on transparent,
rendered from `src/RZ_CarSide.svg` with `node tools/render_ui.js textures`), imported as a texture
and set in **UI > Car picture**. Without it the markers sit on a plain dark panel. Both wheels are
drawn: which job is missing shows on the markers, not the picture.

### The shop screen

As approved (`shop.html`): **Trade** at a quartermaster opens a panel 1,600 × 880 in the middle of the screen, the
world dimmed. The header says whose stores they are ("Haven stores", "Ada Reyes, quartermaster in
Morton") and, on the right, your standing with them and your Favor ("You're Trusted", "64 Favor" in
the faction's colour), over a 2 px line in that colour. Under it the categories as tabs: **Food
and water, Medical, Gear and tools, Weapons and ammo, Services** (a tab with nothing on it is
hidden). On the left the items: name, and the price in Favor; an item above your standing is dim
with a lock and what it needs ("Needs Champion with the Haven"), and one nobody can have right now
says why ("Not until the next truck", "No doses left"). On the right a details panel, topped by a
picture of the selected item: vanilla's item preview of its prefab (as the inventory draws items)
on a dark box with a lighter floor strip, labelled "3D PREVIEW", darkened when you can't have it.
Then its name, its own description and weight, the price and "you have 64", and **Buy** (or
**Sign out** for a requisition, **Ask** for a service). Under both, what the last purchase said.
Nothing is sold here that the Buy actions don't sell: never a vehicle or a car part
([REPUTATION.md](REPUTATION.md#what-each-faction-gives-you)).

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Category | Q/E, or click a tab | LB/RB |
| Item | up/down, or click it | D-pad or left stick |
| Turn the picture | left/right, or click it | right stick (`RZ_TurnPreview`), or D-pad left/right |
| Buy | Enter, or the Buy button | A |
| Close | Esc, or the close button | B |

**How it works.** `RZ_ShopOpenAction` ("Trade", on the four quartermaster prefabs) reaches the
server, which opens a screen-guard entry and sends `PAYLOAD_SHOP` (`Factions/RZ_ShopScreen.c`):
`S|serial|faction|title|sub|standing|Favor|status`, then `I|index|tab|name|price|state|need|prefab|button`
per row: the quartermaster's own `RZ_FactionShopAction`s (the price and refusal from `Quote`, the
same checks a Buy makes: standing, a Rogue leader's favour, the Haven's exile and safehouse size,
the lab's stock, shortages, the top tier under pressure, leaders' and morale's prices), then its
`RZ_RequisitionAction`s and `RZ_FactionServiceAction`s on the Services tab. `X|reason` closes it.
A purchase goes back as `RpcAsk_ShopBuy(serial, index)` (at most two a second); the server checks
the screen is open on that serial, the quartermaster is there and you're within **Dialogue >
Range**, then runs that row's action code (`Buy`, `Requisition`, `Run`), catches what it says for
the status line and sends the screen again. The client won't send a purchase it can see you can't
afford (it says so instead); the server checks anyway. The tab an item goes on is guessed from its
prefab path and name (*simplified*: food, medicine, weapons and magazines by their vanilla
folders and common words; anything else is gear). The picture and its description come from the
item's preview copy on the client; the picture turns about its up axis. The sub-line names the
leader of that seat, if any, as the quartermaster (*simplified*: at a stronghold or safehouse the
quartermaster is the leader).

**Fallbacks.** While the screen is used, a quartermaster's Buy, Requisition and service actions
hide and **Trade** shows (`RZ_ShopMenu.HidesActionsOn`). With **UI > Shop item layout** empty,
**Trade** is hidden and the actions are as before. A client that can't open the menu says so and
shows the actions from then on; when the server can't open one (at its cap of open screens) the
client shows the actions for a minute.

### The background picker

The panel reads the picked loadout twice a second. Its **starting gear** is what that loadout
really spawned with the last time anyone took it (the server notes up to ten things a few seconds
after the spawn, by prefab name); until then, the background's own gear line (`RZ_Background.m_sGear`).
A locked background's loadout can't be spawned (**UI > Lock loadouts**, on). The server decides,
whatever the client sends, so it holds with a mouse, a gamepad or a modified client:
`RZ_LoadoutLock.c` mods `SCR_LoadoutManager.CanAssignLoadout_S` (a locked loadout is never
assigned) and `SCR_BaseGameMode.CanPlayerSpawn_S` (a spawn is refused while the assigned loadout
is locked, or when it asks for a locked loadout's prefab). The menu only shows it: the client marks
the loadout unavailable from the last backgrounds payload (`SCR_BasePlayerLoadout`, modded). The
1.9 menu leaves unavailable loadouts out of its list when it builds it and greys their buttons when
it refreshes (which one players see is a VERIFY). The panel says how to unlock it. If a locked pick
gets through anyway, it plays as the default background. Before the server-side refusal, Workbench
play spawned the locked Soldier loadout as the default background; the refusal itself is still to
be checked in play.

### The map

`RZ_MapIconsUI` is a component on the map configuration: while the map is open it asks the server
for this player's list (on open, then every 20 s) and draws one icon per place, in its colour
(the faction's, hazard yellow for events, the sealed checkpoints and the packs, bone for the
stations), with a label, following pan and zoom. The packs your character knows of are **range
rings** (`marker_range`): a line's last field is a radius in metres, and the ring is sized to it on the
map every frame (28 to 320 units across), so it grows and shrinks with the zoom: Heard of, half a
kilometre round the rough area; Read and Mapped, the pack's ground (Mapped adds where it lies up to the
label). The list is what this player may know ([DESIGN.md](DESIGN.md#settlements-spawn-points-and-the-map)):
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
| `RZ_BoardClear` | Delete | X | `MenuContext`: the notice board's Clear, server admins only |
| `RZ_Reply1` … `RZ_Reply7` | 1 … 7 | none: the D-pad and A pick a reply | `MenuContext` |
| `RZ_CarStart` | T | Y | `MenuContext` |
| `RZ_TurnPreview` | none: left/right (`MenuLeft`/`MenuRight`) or a click | right stick (horizontal) | `MenuContext` |
| Kick free of a hold (vanilla `CharacterJump`, listened to only while held) | Space | the pad's jump button (A) | `IngameContext` |

**Breaking a hold** ([INFECTED.md](INFECTED.md#how-they-move-and-hunt), Attacks). When something has
your ankle, `RZ_PlayerEffectsComponent` listens to the base game's Jump for the hold's few seconds:
**Hold kicks** (3) presses kick you free. It needs no binding of ours, so it is the same button on a
pad and a console. The prompt is the base game's hint with its timer ("Something has your ankle" /
"Kick free: press Jump 3 times."), shown for the hold and hidden when it ends; it has no widget of
ours. The stagger and the knockdown have no prompt, only a line.

N is free in play (vanilla uses it only in the group menu and the map's pencil tool, other
contexts). The pad has no free button in play, so Start (the pause menu) and the notebook item are
its way into the notebook. X is free in the board menu, so the radio filter and the board's Clear
share it, each only in its own screen.

**The check** (done in the files; not yet played with a pad, the VERIFY list's UI part):

| Part | Checked | Fixed |
|---|---|---|
| Input actions | Every `RZ_` action in `chimeraInputCommon.conf` has a pad binding or a route (the table above) | `RZ_BoardClear` added with its pad button |
| The screens | The notebook, the notice board, the talk screen, the reader, the car repair screen and the shop each open from something a pad reaches (an action, the pause menu, the notebook item, a talk), close with B, set the focus and move with the D-pad and shoulders. The HUD, the subtitles, the map icons and the deploy panel only show | The notice board had no prompts, no focus on its button and no way to write without typing: it now has prompts by device, A on Pin it up, seven ready-made notes on left and right, and is on by default (**UI > Board menus**) |
| Static hints | Footers and hints that named only keys | The radio filter's hint ("F / X"), the talk screen's keys hint ("1-7 or D-pad and A") |
| Hover, typing, chat | No screen uses hover or a tooltip. The only typing is the board's note box (now optional) | Every chat command has a way without it: `/note` the board's ready-made notes, `/noteclear` the board's X, `/mydead`, `/season` and `/anonymous` the notebook item's actions, `/lastentry` the Names board's *Write your line*, `/tribes`, `/talkdebug`, `/traces` and `/rep` (the nearest player) as **[Admin]** replies when an admin talks to any person; `/noteclear all`, `/rep` for a named player far away and `/epilogue` stay typed admin extras |
| Text on a TV | Every layout's text sizes | All text in the ReforgerZ layouts is 18 or more (it was down to 11 on the Standing page; 12–17 on the other pages, the notice board, the notice cards, the HUD and the map icons). The reputation grid's headers now read "FAME" and "INFAMY UNDER 15" and so on, and its cells are condensed bold so the longest names fit. The vanilla deploy menu's own footer (15–16) is vanilla's |

**What needs Workbench** (it can't be checked as text):
- Open each page in the layout editor at 1920×1080 and look for text that now runs past its slot:
  the Standing page's labels over their bars (FAME, INFAMY, LEADERS YOU'VE MET, WHAT THEY NEED: 6
  units over 16-unit slots), the reputation grid's header row (4 over), the HUD's infection stage
  above its clock and the map key's names over their lines (2 over). Where it touches, move the
  widget below down a few units or shorten the label.
- Play every screen with a pad on PC (and on a console build when there is one): the focus, the
  prompts' glyphs, the notice board's ready-made notes (A on Pin it up, and that left and right
  don't fire while you type in the box with a keyboard), X on the board as an admin and as a player.
- Check that the console's on-screen keyboard opens for the board's note box (VERIFY); the
  ready-made notes work without it either way.

## How the scripts fit

Code: `scripts/Game/ReforgerZ/UI/`. The UI only reads; nothing in it changes the game.

| Class | Runs on | Does |
|---|---|---|
| `RZ_UIFeed` | server, on the game mode's tick | Works out each player's infection state, the secure channel they hear, and a noise level (0–6 from the heat where they stand), and sends each **only when it changes**; with Stealth on, stealth's own update (every second) hands it each player's visibility, sent the same way as a level 0–5 and whether it counts as hidden (the timer also resyncs every minute, and at once when the time left jumps more than 5 s from what the client is counting down; the client counts down in between). A bite cleaned in time still shows the golden window until it would have closed, so the HUD never tells early whether the clean worked. Builds the standing, journal (with the questlines the player is part of and the lab's line), background, map, notice board and hall of fame data when asked. Remembers the clues each player has read while the puzzle runs, the outposts each player has been near (this session) and each loadout's gear |
| `RZ_PlayerUIComponent` | on the player controller | The pipe: owner-only RPCs from the server, a request RPC and a command RPC from the client (each payload kind and each command at most every 2 s; a command sent too soon gets a short notice instead of vanishing), and the client's cache. Keeps the Radio tab's notices on the client for this life and the notebook's unread counts. A notice board payload from the server opens the board menu; a reader or talk payload opens those screens. Carries talk picks and closed screens back |
| `RZ_Dialogue`, `RZ_TalkSpeaker` (`RZ_TalkGeneric`, `RZ_TalkLeader`), `RZ_TalkNeeds`, `RZ_DialogueTrees` | server | Conversations: sessions, capture, replies and their needs, picks; with Trees on, topics, nodes, checks and their tags ([DIALOGUE.md](DIALOGUE.md)) |
| `RZ_TalkMenu` | client (`ChimeraMenuBase`) | The talk screen ([The talk screen](#the-talk-screen)) |
| `RZ_Reader`, `RZ_ReaderMenu` | server, client | The reader: one player's paper, a page at a time ([Subtitles and the reader](#subtitles-and-the-reader)) |
| `RZ_Cars` (its repair screen part), `RZ_CarMenu` | server, client | The car repair screen ([above](#the-car-repair-screen)) |
| `RZ_ShopScreen`, `RZ_ShopOpenAction`, `RZ_ShopMenu` | server, server, client | The shop screen ([above](#the-shop-screen)) |
| `RZ_ScreenCapture` | server | While a screen acts for its player, catches what would have been that player's notice for the screen's status line |
| `RZ_ScreenGuard` | server, once a second | Watches open talk, reader, repair and shop screens and closes them in danger (below) |
| `RZ_PlayerEffectsComponent` | on the player controller | The infected's close moves on the owner's machine: a hold (with the hint and Jump to kick free), a stagger, a knockdown; the server decides each one ([INFECTED.md](INFECTED.md#how-they-move-and-hunt)) |
| `RZ_ChatCommands` | client | `/note`, `/noteclear`, `/tribes`, `/traces`, `/rep`, `/talkdebug`, `/mydead`, `/season`, `/anonymous`, `/lastentry`, `/epilogue`, sent as commands to the server, which checks them again. Registered with the first notice. Each has a way without typing ([Controllers](#controllers)), but the admins' `/epilogue` |
| `RZ_HudDisplay` | client (`SCR_InfoDisplayExtended`) | Draws the HUD five times a second from the cache; shows the notice cards and the subtitles |
| `RZ_DoorMarkClient` | client, from the HUD's update | Traces' door marks as decals near the player, from the game mode's replicated list and the kit's atlas ([TRACES.md](TRACES.md#doors-you-can-see)); nothing without the atlas |
| `RZ_NotebookMenu` and its pages (`RZ_NotebookPage`), `RZ_NoticeBoardMenu` | client (`ChimeraMenuBase`) | Ask for their data on open (each page when it's shown), redraw when a new copy arrives; `RZ_MenuClick` passes mouse clicks to them ([The notebook](#the-notebook)) |
| `RZ_Notebook`, `RZ_PauseNotebook`, `RZ_NotebookGadgetComponent` | client | Open the notebook: N, the pause menu's button, the notebook item |
| `RZ_BackgroundPanelComponent` | client (widget handler) | Watches the picked loadout in the deploy menu and shows its background and gear |
| `SCR_BasePlayerLoadout`, `SCR_LoadoutManager`, `SCR_BaseGameMode` (modded, `RZ_LoadoutLock.c`) | server (the client only marks the loadout unavailable) | Never assigns or spawns a locked background's loadout |
| `RZ_MapIconsUI` | client (map component) | The icon layer on the map |
| `RZ_NoticeStyle` | client | Classifies a notice by its title (faction names, leader titles, channel names) into a kind, a faction and a priority for its card and the Radio tab; a server tag (`RZ_ENoticeTag`) decides where there is one. The 600-odd plain `NotifyPlayer` calls are unchanged |
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
`QueueHint` classifies it, keeps radio, mission and world notices for the Radio tab, and offers it to the HUD. If **UI
> Styled notices** is off, or the HUD layout or the card layout is missing, the vanilla hint queue
shows it exactly as before.

**The screen guard** (`RZ_ScreenGuard`, server). The talk screen, the reader, the car repair
screen and the shop screen stop you moving while the world keeps running, so the server watches every open one, once a second on the game
mode's `SecondTick`, and closes it when: you took damage or fired; an infected is within
**Dialogue > Danger near** (6 m) of you or of the person you talk to, or one within **Danger
closing** (30 m) came 2 m closer since the last check; you died, or the person is gone; and, for
talk only, you're more than **Range** (6 m) from them or **Idle seconds** (120) passed with no
reply (the repair and shop screens the same, from the car or the quartermaster, with no pick). In danger the person says "Not now!" as a subtitle. At most **Open at most** (32) screens
are open on the server; each check is a few distance tests. The client says when the player
closes one (`RpcAsk_ScreenClosed`).

**Turning it off.** Styled notices off gives the vanilla hints back. Emptying **UI > Car job
layout** gives a fixable car its prompts and notices back, and emptying **UI > Shop item layout**
a quartermaster its Buy actions. Taking
`RZ_PlayerUIComponent` off the player controller (`Prefabs/Characters/Core/RZ_PlayerController.et`)
stops all sending. To remove the UI entirely, delete the `UI/` code folder and the lines that use
it: the `m_UISettings` attribute, `m_UIFeed` (creation, tick, spawn,
`RecordGearSoon` and accessors), the `UsesMapIcons` check in `AddMapMarker`, the screen capture and the owner-only
branch at the top of `NotifyPlayer` and the block at the
top of `QueueHint` (with the chat commands' registration) in `Core/RZ_GameModeComponent.c`, the
board menu call in `World/RZ_NoticeBoard.c` (`SendMenu`), the `OnClueRead` call in
`Puzzles/RZ_PuzzleMission.c`, the repair screen section of `Interactions/RZ_Cars.c` (with its
calls in `Inspect`, `Hide`, `CompleteFit` and `CompleteHotwire`, and `OnTaskStopped` in
`RZ_Interactions.c`) the `RZ_CarMenu` check in `Puzzles/RZ_PuzzleActions.c`, `Factions/RZ_ShopScreen.c` with
`RZ_ShopOpenAction` and the `RZ_ShopMenu` checks in the quartermaster actions
(`Factions/RZ_Quartermaster.c`, `RZ_FactionServices.c`). `RZ_LoadoutLock.c` and `RZ_MapIcons.c` can each be deleted on their
own (without the first, a locked pick spawns and plays as the default background). Then remove what names the UI's classes: point `RZ_GameMode.et` back at the vanilla player
controller and clear its **UI** settings, and delete the two overrides in `Configs/System/` and
the layouts in `UI/layouts/ReforgerZ/`.

*Simplified:* notices are classified by their title, so an unusual title gets a plain card. The
map's icons don't cluster or scale with zoom; outposts a player has seen are remembered for the
session only.
Journal rewards are the mission's base Fame and Favor ("about"), before leader traits and
background multipliers. The radio channel list is a static part of its layout; the standing page's
reputation grid is filled in by code (`RepCell_<fame band>_<infamy band>`).

## Built and planned

| | |
|---|---|
| Built | The icon set and its renderer; eleven mockups and previews (with the talk screen, the reader, a subtitle, the car repair screen and the shop). In code: the feed, the HUD with notice cards paced by importance and subtitles, owner-only notices and spoken lines, the notebook (five tabs as pages, N, the pause menu button, the notebook item; the journal's errand rows and per-row detail), the talk screen and its engine (with the admins' errand view as a reply), the reader, the car repair screen, the shop screen, the screen guard, the notice board menu, chat commands, the deploy-menu panel with the loadout's gear, locked loadouts unavailable, the map's icon layer, styled notices with the vanilla fallback |
| Workbench files | The layouts ([LAYOUTS.md](../UI/LAYOUTS.md)), the notebook's, talk screen's, reader's, car repair and shop screens' written as text; the icon atlas and the `RZ_Icons` imageset; the car picture's PNG (to import: **UI > Car picture** is empty); the menu and input config overrides in `Configs/System/` (the notebook, notice board, reader, talk, car and shop presets; N, 1–7, T, the right stick and the filter's X); **Trade** on the four quartermaster prefabs; the notebook item `Prefabs/ReforgerZ/UI/RZ_Notebook.et`, written as text; the player controller `Prefabs/Characters/Core/RZ_PlayerController.et`, with the feed and the HUD; the game mode's **UI** settings, all but the map icon layout. What each holds is in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui) |
| Works in play | Seen in Workbench play on the repo's world, with no script exceptions: the styled notice cards, the stealth visibility bars and the noise meter |
| Not checked in play | The infection timer and the secure-radio indicator (they need a bite or a cracked channel); notice pacing; the notebook, its pages, the pause menu button and the notebook item; the talk screen; the reader; the car repair screen and its picture; the shop screen and its item picture; the subtitles; mouse clicks and the pad; the notice board menu and its ready-made notes; the chat commands; the deploy menu's side of locked loadouts ([above](#the-background-picker)) |
| Not done | The override of the vanilla deploy menu's layout that holds the background panel, so the panel doesn't show. The map configuration override with `RZ_MapIconsUI`, and **UI > Map icon layout** (empty: set, it turns the vanilla text markers off), so the map shows the vanilla markers. Images for the icons the scripts never swap (the map key's, the radio channel list, the HUD's radio, noise and paused icons, the background panel's): they draw as flat squares in their colour |
| Planned | The `mission_errand` icon (errand rows use `mission_unarmed`); a sound on new notices; mockups of the notice board and the hall of fame; the Long Table's mark in the journal and the hall of fame once a player has found the family out, and at the farm; the cure finale's own screens: the journal's finale page and the lock card as a screen (today they're a desk's "How do things stand?" and the readout under each route's journal entry, and the lock card as a notice or the [Point of no return] node: [FINALE.md](FINALE.md#simplified-and-not-built-yet)) |
