# The UI layouts

The ReforgerZ layouts are in `UI/layouts/ReforgerZ/`: the HUD, the notice card, six menus, the
journal and radio rows, the map icon and the background panel (12 files). They were written as
text from the trees below and checked against every widget name and type the UI scripts look up.
This file is their reference: the widget trees, the **names the scripts look up**, sizes, anchors,
colours and fonts. When you change a layout, change its tree here too. What each screen is for,
how the scripts work and what's built is in [docs/UI.md](../docs/UI.md); the mockups in
`UI/Mockups/` (previews in `UI/Previews/`) show the intended look. Where a layout differs from its
mockup, its section says so.

## Conventions

- **Reference resolution** 1920×1080. Sizes are in layout units at that size; the HUD pieces
  anchor to their corner, menus fill the screen. Everything that matters stays inside the 90% safe
  area: 96 units from the left and right edges, 54 from the top and bottom
  ([UI.md](../docs/UI.md#controllers), "Readable on a TV").
- **Names matter.** The scripts find widgets with `FindAnyWidget(name)`. Every name in the trees
  must be spelt exactly. A widget that's missing is skipped, so a simpler layout still works. The
  layouts name every widget; one shown in brackets, a description like `(panel background)` or a
  name like `(NoticeArea)`, is never looked up.
- **Types matter where a script uses them:** text goes into a `Text`, icons into an `Image`, bars
  into a `ProgressBar`, the note into an `EditBox`, and clicks come from a `Button`. The layouts
  also use `Frame`, `Overlay`, `VerticalLayout`, `HorizontalLayout`, `ScrollLayout` (the wheel
  scrolls it), `SizeLayout` (a fixed width or height) and `Panel` (borders).
- **Panels** are an `Image` with no texture, colour `#121416` at 90% alpha, behind the content.
  Borders are a `Panel` with the `outline_2px` style. Accent bars are an `Image` 4–5 units thick;
  the scripts tint them (`SetColor`). A bar's background is an `Image` under its `ProgressBar`.
  Gaps ("10 apart") are the slots' padding. Text that "wraps" has wrapping on.
- **Icons** are `Image` widgets, and no layout sets an image yet. An icon a script swaps gets its
  image by name from **UI > Icon imageset**, and its tint. An icon marked *static* below draws as
  a flat square in its colour until it's given its image from the `RZ_Icons` imageset in the
  layout editor (not done yet). The map key's and the radio log's static icons are named after
  their icon (`Key2_marker_safehouse`, `Chan6_status_secure_radio`).
- **Fonts.** The game's UI fonts (`UI/Fonts/` in 1.9) have no mono or serif face. Where the trees
  say *mono* (and the clue paper's *serif*), the layouts use Roboto:
  `UI/Fonts/Roboto/Roboto_Bold.fnt` or `Roboto_Regular.fnt`. Everything else is RobotoCondensed:
  `RobotoCondensed_Bold.fnt` for *condensed bold* headings and *bold* text,
  `RobotoCondensed_Regular.fnt` for *regular* text and text given only a size (both in
  `UI/Fonts/RobotoCondensed/`). Italic is a font property.
  Labels, like `(label "WHERE")`, are mono bold 12, dim.
- **No upper case or letter spacing.** Layouts have neither property, so static labels are typed
  in capitals and the mockups' wide spacing is gone. Text a script sets shows as the script sends
  it: some in capitals (the infection stage, tier names, the menu headers, the background's
  name), some in mixed case (faction names such as "The Haven", mission titles).

| Colour | Hex | Use |
|---|---|---|
| Bone | `#E8E2D0` | main text |
| Dim | `#A39E90` | secondary text, labels |
| Faint | `#6E6A60` | times, hints |
| Panel | `#121416` (90%) | backgrounds |
| Line | `#2A2E31` | borders, empty bars |
| The Haven | `#E3A33B` | accent |
| Task Force Bastion | `#9AA35E` | accent |
| The Red Hand | `#C8352B` | accent |
| Halcyon | `#4FB8B0` | accent |
| Hazard | `#F2C230` on `#141414` | infection, the broadcast, the Perimeter |
| Stages | `#E2B33C`, `#E07A2E`, `#D2412F` | Fever, Weakness, Final |

## Importing the icons

`tools/icon_atlas.py` packs the icons into one texture atlas and writes the imageset for it. It
needs Python and Pillow ([PC_SETUP.md](../docs/PC_SETUP.md)).

1. `py tools/icon_atlas.py` packs the 60 white icons from `UI/Icons/png/128/` into
   `UI/Textures/ReforgerZ/RZ_Icons.png`: 1024 × 1024, eight 128-pixel cells to a row, in name
   order.
2. In Workbench's Resource Browser, import that PNG with the vanilla **TextureUIAtlas** preset. It
   becomes `RZ_Icons.edds`.
3. `py tools/icon_atlas.py imageset` writes `UI/Imagesets/ReforgerZ/RZ_Icons.imageset`: one image
   per icon, **named like its file** (`faction_haven`, `status_fever`...). The scripts ask for
   these names; the full list is in [Icons/README.md](Icons/README.md). It reads the texture's
   name from `RZ_Icons.edds.meta`, so import first. It writes the imageset's `.meta` only the
   first time, so the GUID stays.
4. **UI > Icon imageset** on `RZ_GameMode.et` points at the imageset (already set).

After adding or changing an icon, re-render the PNGs ([UI.md](../docs/UI.md#the-icon-set)) and do
steps 1 to 3 again. With **Icon imageset** empty, the scripts leave every image as the layout has
it and only tint it.

## HUD: `RZ_Hud.layout`

The layout of `RZ_HudDisplay`, an info display on the player controller's
`SCR_HUDManagerComponent` (`Prefabs/Characters/Core/RZ_PlayerController.et` has it). Root: a
full-screen `Frame` with no background that ignores the cursor.

```
Frame  (root, full screen)
├─ Frame  InfectionPanel        430 × 150, anchor bottom-left, offset (96, -54)
│  ├─ Image   (panel background)
│  ├─ Image   InfectionAccent   5 × full height, left edge          tinted by stage
│  ├─ Image   FinalGlow         full size, red #D2412F 35%, shown in the Final stage
│  ├─ Image   InfectionIcon     46 × 46 at (16, 14)                 status_* icon, tinted by stage
│  ├─ Text    InfectionStage    at (76, 14), mono bold 13           tinted by stage
│  ├─ Text    InfectionClock    at (76, 32), mono bold 44, bone
│  ├─ Frame   GoldenRow         at (16, 92), 398 × 40               shown in the golden window
│  │  └─ ProgressBar GoldenBar  398 × 8, fill hazard #F2C230, background #2A2D2F
│  ├─ HorizontalLayout StageTrack at (16, 92), 398 × 8, 3 apart    hidden in the golden window
│  │  ├─ ProgressBar TrackFever     fill weight 25, fill #E2B33C
│  │  ├─ ProgressBar TrackWeakness  fill weight 15, fill #E07A2E
│  │  └─ ProgressBar TrackFinal     fill weight 10, fill #D2412F   (each on #2A2D2F)
│  └─ VerticalLayout (NoteColumn) at (16, 102), 398 wide
│     ├─ Text    InfectionNote  italic 14, dim, wraps
│     └─ Overlay PausedRow      4 below the note, fits its content, border #4FB8B0 50%
│        ├─ Image  (item_stabiliser icon, 18 × 18, #4FB8B0, static)
│        └─ Text   PausedText   mono bold 12, #4FB8B0
├─ Frame  RadioPanel            390 × 60, anchor top-left, offset (96, 54)
│  ├─ Image   (panel background)
│  ├─ Image   RadioAccent       4 × 60, left edge                   faction colour
│  ├─ Image   RadioIcon         34 × 34 at (12, 13)                 status_secure_radio, static, tinted
│  ├─ Text    RadioChannel      at (60, 10), mono bold 13           faction colour
│  └─ Text    RadioSub          at (60, 32), mono 13, dim
├─ HorizontalLayout NoisePanel  anchor bottom-left, offset (552, -62), 10 apart, 85% opacity
│  ├─ Image   (status_noise icon, 26 × 26, bone, static)
│  ├─ HorizontalLayout (3 apart)
│  │  └─ Image Noise1 … Noise6  14 × 8 each, no texture (tinted lit or dim)
│  └─ Text    NoiseText         mono bold 12, dim (QUIET, HEARD, LOUD, VERY LOUD)
├─ HorizontalLayout SeenPanel   anchor bottom-left, offset (588, -90), 10 apart, 85% opacity; hidden until the server sends a visibility (Stealth on)
│  ├─ HorizontalLayout (3 apart)
│  │  └─ Image Seen1 … Seen5    14 × 8 each, no texture (tinted lit, dim, or green when hidden)
│  └─ Text    SeenText          mono bold 12, dim (HIDDEN, LOW, SEEN, IN PLAIN SIGHT)
└─ VerticalLayout (NoticeArea)  460 wide, anchor top-right, offset (-96, 96)
   ├─ VerticalLayout NoticeStack  the notice cards
   └─ Text   NoticeQueued       under the stack, right-aligned, mono 12, faint ("+2 QUEUED")
```

## Notice card: `RZ_NoticeCard.layout`

One card, created into `NoticeStack` for each notice (**UI > Notice layout**). Without this
layout, or without `NoticeStack`, every message uses the vanilla hint.

```
SizeLayout  (root, 460 wide, height fits the content, 12 below it: the gap between cards)
└─ Overlay
   ├─ Image   (panel background)
   ├─ Panel   (border #2A2E31)
   ├─ VerticalLayout       padding 16
   │  ├─ HorizontalLayout
   │  │  ├─ Image  CardIcon     26 × 26, 10 before the kind
   │  │  ├─ Text   CardKind     mono bold 12, fills the row
   │  │  └─ Text   CardTime     mono 12, faint, right
   │  ├─ Text   CardSpeaker  bold 17, bone, 10 above, wraps
   │  └─ Text   CardText     regular 16, #D9D2BE, wraps
   ├─ Image   CardAccent    full width × 4, top edge
   ├─ Image   CardHazard    full width × 8, top edge, hazard yellow, hidden; shown for the broadcast
   └─ ProgressBar CardLife  full width × 3, bottom edge, 70% opacity, no background (counts down)
```

Unlike the mockup, `CardHazard` is plain hazard yellow: its stripes would need a texture.

## Menus

Four menus share a frame: Standing, Journal, Radio and Map key. Each layout has its own copy:

```
Frame (root, full screen)
├─ Image   (dark overlay, #08090A at 75%, so the world shows faintly)
├─ Text    (Heading)       at (72, 40), condensed bold 46: the menu's name
├─ Text    Header          at (72, 100), mono 17, dim: the sub-line
├─ Image   (rule under the header, at y 138)
├─ Button  Tab0 … Tab3     132 × 36 each, 4 apart, top right at y 88: Standing, Journal, Radio, Map key
│  └─ Text  (tab name)     mono bold 13; the open tab bone with dark text, the others dark with a border
├─ Button  CloseButton     36 × 36, top right at (-108, 40), with a Text "X"
├─ Text    (footer keys)   bottom left, mono bold 13, dim ("ESC  CLOSE      Q / E  TABS")
└─ Text    (footer note)   bottom right, mono 13, dim
```

The notice board and the hall of fame use the same frame without the four tabs.

**Mouse.** The tabs, `CloseButton`, the journal row's root (`MissionRow`), the radio filter chip
(`FilterButton`) and the notice board's `PostButton` are `Button`s. Long text sits in a
`ScrollLayout` so the wheel scrolls it: `List`, `Log`, `Notes`, `Leaders0` to `Leaders3` and
`PastList`. Menus show the cursor while open; if one doesn't, check its entry in
`chimeraMenus.conf`.

**Menu presets.** `Configs/System/chimeraMenus.conf` is an override of the vanilla file (same
GUID) with six presets added, each with its layout and class. Workbench finds them.

| Preset | Class | Layout |
|---|---|---|
| `RZ_StandingMenu` | `RZ_StandingMenu` | `RZ_StandingMenu.layout` |
| `RZ_JournalMenu` | `RZ_JournalMenu` | `RZ_JournalMenu.layout` |
| `RZ_RadioLogMenu` | `RZ_RadioLogMenu` | `RZ_RadioLogMenu.layout` |
| `RZ_MapKeyMenu` | `RZ_MapKeyMenu` | `RZ_MapKeyMenu.layout` |
| `RZ_NoticeBoardMenu` | `RZ_NoticeBoardMenu` | `RZ_NoticeBoardMenu.layout` |
| `RZ_HallOfFameMenu` | `RZ_HallOfFameMenu` | `RZ_HallOfFameMenu.layout` |

**Input actions.** `Configs/System/chimeraInputCommon.conf` is an override of the vanilla input
config (same GUID) that adds `RZ_OpenNotebook` (**N**) in `IngameContext`, and `RZ_RadioFilter`
(**F**, pad **X**) and `RZ_Reply1` to `RZ_Reply7` (**1** to **7**) in `MenuContext`. The list with
the pad routes is in [UI.md](../docs/UI.md#controllers). Esc, Q/E, up/down and Enter use the
vanilla menu actions (B, LB/RB, the D-pad and A on a pad).

### Standing: `RZ_StandingMenu.layout`

```
Frame (root)
├─ (menu frame: heading "STANDING", Header (the player, background and life), tabs, footer)
├─ Frame  (Factions)           at (72, 168), 1778 × 610
│  └─ Frame ×4, one per faction, 431 × 610, 18 apart; names end in the faction number:
│     0 the Haven, 1 Task Force Bastion, 2 the Red Hand, 3 Halcyon
│     ├─ Image   (panel background)
│     ├─ Image   AccentN         full width × 5, top               faction colour
│     ├─ Image   EmblemN         54 × 54 at (22, 18)               faction_* icon
│     ├─ Text    FactionNameN    at (92, 30), condensed bold 24
│     ├─ Frame   (tier box, 387 × 66 at (22, 88), background white 3.5%, border)
│     │  ├─ Image  TierIconN     42 × 42                           tier_* icon
│     │  ├─ Text   TierNameN     condensed bold 26
│     │  └─ Text   TierNoteN     mono 13, dim
│     ├─ Text    FameValueN      right, mono bold 14     │ ProgressBar FameBarN   387 × 10
│     ├─ Text    InfamyValueN    right, mono bold 14     │ ProgressBar InfamyBarN 387 × 10, fill #D2553F
│     │   (three 2-unit dark ticks over both bars at 15%, 40% and 75%: the tier bands)
│     ├─ Text    FavorValueN     mono bold 24, faction colour
│     ├─ Text    (label "LEADERS YOU'VE MET") │ Text LeadersN  wraps, 15, in a ScrollLayout
│     ├─ Frame   NeedBlockN
│     │  ├─ Text   (label "WHAT THEY NEED")
│     │  ├─ Text   NeedTitleN     mono bold 12, faction colour
│     │  └─ Text   NeedN          italic 14, wraps
│     └─ Overlay WarnN           bottom strip, 36 high, red tint 16%
│        └─ Text  (WarnTextN)    mono 13, "HUNTER SQUADS ARE OUT FOR YOU"
├─ Frame  (between the factions, at (72, 798), 880 × 196, label "BETWEEN THE FACTIONS")
│  └─ Text   Relations          wraps, 16
├─ Frame  (tier grid, at (970, 798), 874 × 196): a static 5 × 5 grid of the tier names
└─ Text   Waiting              centred, italic 22, "Asking around..." (shown until the data arrives)
```

### Journal: `RZ_JournalMenu.layout` and `RZ_MissionRow.layout`

```
Frame (root)
├─ (menu frame: heading "JOURNAL", Header ("3 OPEN"), tabs, footer)
├─ Frame  (list panel, at (72, 168), 660 × 840)
│  ├─ VerticalLayout (list column)
│  │  ├─ ScrollLayout (fills the column above LabStatus)
│  │  │  └─ VerticalLayout List   the rows
│  │  └─ Text  LabStatus      mono 13, dim, wraps: the lab's line ("Research 63 percent.
│  │                          Waiting: 14 samples' worth, about 4 days of work."); hidden without a lab
│  └─ Text  Empty             centred, italic 18, "The radio is quiet."
└─ Frame  (detail panel, at (750, 168), 1098 × 840)
   ├─ Image  DetailAccent     full width × 5, top
   ├─ Image  DetailIcon       56 × 56 at (34, 31)
   ├─ Text   DetailTitle      at (108, 30), condensed bold 34
   ├─ Text   DetailBy         at (108, 72), 15, dim
   └─ VerticalLayout (detail body, at (34, 112), 1030 × 712)
      ├─ Text   DetailBrief   italic 18, wraps, on a faint panel with a 3-unit faint bar on its left
      ├─ (WHERE and REWARDS side by side)
      │  ├─ Text (label "WHERE")   │ Text DetailWhere    mono bold 20, wraps: the direction, then the area
      │  └─ Text (label "REWARDS") │ Text DetailRewards  mono bold 13, wraps
      ├─ Text   (label "REQUIREMENTS") │ Text DetailReq  16, wraps
      └─ VerticalLayout CluesBlock
         ├─ Text  (label "WHAT YOU KNOW": puzzle clues read, or a questline's notes)
         └─ Text  DetailClues  serif 16, wraps, dark text #23211C on a paper-coloured panel #E4DDC8
```

Unlike the mockup, `DetailWhere` is one `Text` (the script fills both lines into it), not a big
direction over a small area, and the brief's bar isn't tinted with the accent.

`RZ_MissionRow.layout` (**UI > Mission row layout**), as wide as the list and 62 high. The root
is a `Button` named `MissionRow` (clicking it selects the row), with no visual of its own:

```
Button MissionRow (root)
└─ (SizeLayout, 62 high, holding a Frame)
   ├─ Image  RowSelected   full size, bone 7%, hidden unless selected
   ├─ Image  RowAccent     4 × 62, left edge
   ├─ Image  RowIcon       30 × 30 at (26, 16)
   ├─ Text   RowTitle      at (70, 10), bold 17
   ├─ Text   RowSub        at (70, 34), 13, dim
   ├─ Text   RowDir        right, at (-22, 10), mono bold 14
   └─ Text   RowTime       right, at (-22, 34), mono 12, faint
```

### Radio: `RZ_RadioLogMenu.layout` and `RZ_RadioRow.layout`

```
Frame (root)
├─ (menu frame: heading "RADIO", Header "WHAT YOU HEARD THIS LIFE, NEWEST FIRST", tabs, footer with "F  FILTER")
├─ Frame  (channels panel, at (72, 168), 450 × 840): a static list of the channels, static icons and names
└─ Frame  (log panel, at (546, 168), 1302 × 840)
   ├─ Button FilterButton   at (26, 16), 150 × 30, the bone chip; a click cycles the filter
   │  └─ Text FilterText    mono bold 12: ALL / OPEN / SECURE / BROADCAST
   ├─ ScrollLayout (under a rule at y 62, fills the rest)
   │  └─ VerticalLayout Log   the rows, newest first
   └─ Text   Empty          centred, italic 18, "Nothing on the air yet."
```

The radio log's `Header` is static text: its script doesn't set it.

`RZ_RadioRow.layout` (**UI > Radio row layout**), as wide as the log, height fits:

```
Overlay (root)
├─ Image  RowAccent     full size at 6% opacity, tinted with the row's accent
├─ Image  (rule, bottom edge, white 5%)
└─ HorizontalLayout
   ├─ Text   RowTime       at (26, 14), 74 wide, mono 14, faint
   ├─ Image  RowIcon       30 × 30 at (114, 14)
   └─ (1110 wide)
      ├─ Text   RowChannel    at (158, 14), mono bold 12
      ├─ Text   RowSpeaker    after RowChannel, 14, dim
      └─ Text   RowText       under them, 17, wraps
```

### Map key: `RZ_MapKeyMenu.layout`

Static: no names are looked up, and its `Header` is static text. It holds the menu frame and a
legend panel at (72, 168), 640 × 840, with one row per marker the map draws: the icon in its colour
(32 × 32; the stronghold and outpost rows show three 24 × 24 icons in faction colours), the name
in bold 15 and one line in 13, dim. Under WHO HOLDS WHAT: stronghold, outpost, Haven safehouse,
Halcyon's lab. Under THE WORLD: world event and sealed checkpoint in hazard yellow, power station
and radio station in bone. The icons are static.

Unlike the mockup (`UI/Mockups/map_legend.html`), it has no Quartermaster or Perimeter rows,
because `RZ_UIFeed.BuildMap` sends neither, and the power station is bone, as on the map, not
hazard.

### Notice board: `RZ_NoticeBoardMenu.layout`

Not a tab: no `Tab0`–`Tab3` (the menu frame's heading, `Header` and `CloseButton` only). Opened by
the server when a player reads a safehouse board, with **UI > Board menus** on.

```
Frame (root)
├─ (menu frame: heading "SAFEHOUSE", Header "NOTICE BOARD: MORTON", CloseButton, footer "ESC  CLOSE")
├─ Frame  (board panel, at (72, 168), 1100 × 840, the panel colour)
│  ├─ ScrollLayout
│  │  └─ Text  Notes       wraps, 18, bone
│  └─ Text  Empty          centred, italic 18, "Nothing pinned up."
└─ Frame  WriteBlock       at (1200, 168), 648 × 210, hidden for exiles
   ├─ Text     (label "LEAVE A NOTE")
   ├─ EditBox  NoteInput   one line, 576 × 32 inside a 600 × 44 box with a border, 16,
   │                       placeholder "Write your note here."
   ├─ Text     NoteLimit   13, dim, wraps
   └─ Button   PostButton  180 × 40, with a Text "PIN IT UP"
```

`NoteInput` is a single-line `EditBox`: the script reads it as an `EditBoxWidget`, and a
multi-line edit box isn't one.

### Hall of fame: `RZ_HallOfFameMenu.layout`

Not a tab either: the menu frame's heading ("HALL OF FAME"), `Header` (the season) and `CloseButton`.

```
Frame (root)
├─ (menu frame: heading "HALL OF FAME", Header "SEASON 3", CloseButton, footer "ESC  CLOSE")
├─ Frame  (this season, at (72, 168), 580 × 840)
│  ├─ Text (label "THIS SEASON")
│  └─ Text SeasonList    wraps, 16 (each record: the category, then the name and value)
├─ Frame  (all time, at (670, 168), 580 × 840)
│  ├─ Text (label "ALL TIME")
│  └─ Text AllTimeList   wraps, 16
├─ Frame  (past seasons, at (1268, 168), 580 × 840)
│  ├─ Text (label "PAST SEASONS")
│  └─ Text PastList      wraps, 14, dim, in a ScrollLayout
└─ Text   Waiting        centred, italic 22, "Reading the names..." (until the data arrives)
```

## Map icon: `RZ_MapIcon.layout`

One marker on the real map, created into the map's root by `RZ_MapIconsUI` on the map
configuration when **UI > Map icon layout** names this file. Neither is set up yet
([UI.md](../docs/UI.md#built-and-planned)). The script centres the 28 × 28 icon on the spot and
tints it.

```
Frame (root, 28 × 28, top-left anchor, ignores the cursor)
├─ Image  Icon       28 × 28, a marker_* icon (white; the script sets the image and tints it)
└─ Text   Label      under the icon, offset (-60, 30), 148 wide, centred, 12, bone, a dark shadow offset by 1
```

## Background panel: `RZ_BackgroundPanel.layout`

The panel for the vanilla deploy menu. Its root already carries the `RZ_BackgroundPanelComponent`
script handler and anchors itself top-right, offset (-48, 120), 560 wide. It isn't in the game
yet: it goes into an override of the deploy menu's layout, which isn't made
([setup step 5](../docs/WORKBENCH_SETUP.md#setting-up-the-ui)).

```
Frame (root, 560 wide, anchor top-right, offset (-48, 120))
├─ Image  (panel background, top bar 5 units bone)
├─ Text   (label "BACKGROUND")
├─ VerticalLayout PanelBody        shown once the picked loadout is a background
│  ├─ Image  (background icon, 58 × 58: item_radio or similar, static)
│  ├─ Text   BgName                 condensed bold 36
│  ├─ (state chip, with a border)
│  │  ├─ Image  StateIcon           16 × 16 (status_door_unlocked / status_door_locked)
│  │  └─ Text   StateText           mono bold 12: AVAILABLE / LOCKED
│  ├─ Text   (label "PERK")  │ Text BgPerk      17, #BFD39B, wraps
│  ├─ Text   (label "FLAW")  │ Text BgFlaw      17, #E4826F, wraps
│  ├─ Text   (label "STARTING STANDING") │ Text BgStanding  mono bold 14, wraps
│  ├─ Text   (label "STARTING GEAR")     │ Text BgGear      15, wraps
│  └─ Overlay LockedBlock           with a border, shown when locked
│     └─ Text  LockedText           15, dim, wraps
├─ Text        Legacy               mono bold 12, dim
└─ ProgressBar LegacyBar            160 × 8, fill #E3A33B, beside Legacy
```

`BgGear` lists what the loadout spawned with the last time anyone took it, or the background's
gear line until then. A locked loadout is unavailable in the menu itself (`RZ_LoadoutLock.c`).
