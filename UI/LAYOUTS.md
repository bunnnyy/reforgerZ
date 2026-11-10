# Layouts for the Workbench layout editor

Layouts (`.layout`) and imagesets can't be written reliably outside Workbench, so this file gives
everything needed to build them by hand: the widget trees, the **names the scripts look up**,
sizes, anchors, colours and fonts. What each screen is for and how the scripts work is in
[docs/UI.md](../docs/UI.md); the mockups in `UI/Mockups/` (previews in `UI/Previews/`) show the look.

## Conventions

- **Reference resolution** 1920×1080. Sizes are in layout units at that size; anchor so the
  layout scales (the HUD pieces anchor to their corner, menus fill the screen).
- **Names matter.** The scripts find widgets with `FindAnyWidget(name)`. Every name in `code`
  below must be spelt exactly. A widget that's missing is skipped, so a simpler layout still works.
- **Types** are the Workbench widget types: `Frame`, `Overlay`, `VerticalLayout`,
  `HorizontalLayout`, `Image`, `Text` (multi-line where noted, with wrapping on), `ProgressBar`.
- **Panels** are an `Image` with no texture, colour `#121416` at 90% alpha, behind the content.
  Accent bars are an `Image` 4–5 units thick; the scripts tint them (`SetColor`).
- **Icons** are `Image` widgets. Set the default image from the icon imageset in the editor; the
  scripts swap them by name when **UI > Icon imageset** is set, and tint them. Mono icons are white.
- **Fonts:** use the vanilla fonts (search the Resource Browser for "RobotoCondensed" and
  "RobotoMono"; the names differ between versions). Headings: condensed bold, upper case, wide
  letter spacing. Data (timers, distances, channel names): mono. Body: regular.

| Colour | Hex | Use |
|---|---|---|
| Bone | `#E8E2D0` | main text |
| Dim | `#A39E90` | secondary text |
| Faint | `#6E6A60` | labels, rules |
| Panel | `#121416` (90%) | backgrounds |
| Line | `#2A2E31` | borders, empty bars |
| The Haven | `#E3A33B` | accent |
| Task Force Bastion | `#9AA35E` | accent |
| The Red Hand | `#C8352B` | accent |
| Halcyon | `#4FB8B0` | accent |
| Hazard | `#F2C230` on `#141414` | infection, the broadcast, the Perimeter |
| Stages | `#E2B33C`, `#E07A2E`, `#D2412F` | Fever, Weakness, Final |

## Importing the icons

1. Copy `UI/Icons/png/128/*.png` (crisper when scaled) into the addon, e.g.
   `UI/Textures/ReforgerZ/Icons/`. Copy `png/color/128/` too if you want the coloured map markers.
2. In the Resource Browser, select them and **Import**. Each becomes an `.edds` texture. In the
   import settings use the UI preset if there is one (no mipmaps, alpha kept, sRGB colour).
3. Create an imageset (`UI/Imagesets/ReforgerZ/RZ_Icons.imageset`) in the imageset editor and add
   every texture as an image **named exactly like the file** (`faction_haven`, `status_fever`...).
   The scripts ask for these names. The full list is in [Icons/README.md](Icons/README.md).
4. Set **RZ_GameModeComponent > UI > Icon imageset** to it.

If the imageset step gives trouble, skip it: set each `Image` in the layouts to its texture
directly and leave **Icon imageset** empty. The scripts then leave the images as they are and
only tint them (the HUD's stage icon then doesn't change with the stage).

## HUD: `RZ_Hud.layout`

The layout of `RZ_HudDisplay`. Add `RZ_HudDisplay` to the player controller's
`SCR_HUDManagerComponent` info displays and set its layout to this file. Root: a full-screen
`Frame` with no background, not hit-testable.

```
Frame  (root, full screen)
├─ Frame  InfectionPanel        430 × 150, anchor bottom-left, offset (44, -44)
│  ├─ Image   (panel background)
│  ├─ Image   InfectionAccent   5 × full height, left edge          tinted by stage
│  ├─ Image   FinalGlow         full size, red #D2412F 35%, shown in the Final stage
│  ├─ Image   InfectionIcon     46 × 46 at (16, 14)                 status_* icon, tinted
│  ├─ Text    InfectionStage    at (76, 14), mono bold 13, letter spacing 0.16 em, upper case
│  ├─ Text    InfectionClock    at (76, 32), mono bold 44, bone
│  ├─ Frame   GoldenRow         at (16, 92), 398 × 40               shown in the golden window
│  │  └─ ProgressBar GoldenBar  398 × 8, fill hazard #F2C230, background #2A2D2F
│  ├─ HorizontalLayout StageTrack at (16, 92), 398 × 8, spacing 3  hidden in the golden window
│  │  ├─ ProgressBar TrackFever     fill weight 25, fill #E2B33C
│  │  ├─ ProgressBar TrackWeakness  fill weight 15, fill #E07A2E
│  │  └─ ProgressBar TrackFinal     fill weight 10, fill #D2412F
│  ├─ Text    InfectionNote     at (16, 108), italic 14, dim, wraps at 398
│  └─ Frame   PausedRow         at (16, 120), auto width, 26 high, border #4FB8B0 50%
│     ├─ Image  (item_stabiliser icon, 18 × 18, #4FB8B0)
│     └─ Text   PausedText      mono bold 12, #4FB8B0
├─ Frame  RadioPanel            390 × 60, anchor top-left, offset (44, 40)
│  ├─ Image   (panel background)
│  ├─ Image   RadioAccent       4 × 60, left edge                   faction colour
│  ├─ Image   RadioIcon         34 × 34 at (12, 13)                 status_secure_radio
│  ├─ Text    RadioChannel      at (60, 10), mono bold 13, spacing 0.14 em
│  └─ Text    RadioSub          at (60, 32), mono 13, dim
├─ HorizontalLayout NoisePanel  anchor bottom-left, offset (500, -52), spacing 10, 85% opacity
│  ├─ Image   (status_noise icon, 26 × 26, bone)
│  ├─ HorizontalLayout (spacing 3)
│  │  └─ Image Noise1 … Noise6  14 × 8 each, no texture (tinted lit or dim)
│  └─ Text    NoiseText         mono bold 12, dim
├─ VerticalLayout NoticeStack   460 wide, anchor top-right, offset (-44, 96), spacing 12
└─ Text   NoticeQueued          under the stack, right-aligned, mono 12, faint
```

## Notice card: `RZ_NoticeCard.layout`

One card, created into `NoticeStack` for each notice (**UI > Notice layout**). Without this
layout, or without `NoticeStack`, every message uses the vanilla hint.

```
Frame  (root, 460 wide, height fits content)
├─ Image   (panel background, border #2A2E31)
├─ Image   CardAccent    full width × 4, top edge
├─ Image   CardHazard    full width × 8, top edge, hazard stripes texture (yellow/black 45°), hidden by default
├─ VerticalLayout        padding 12 16 16 16
│  ├─ HorizontalLayout   spacing 10
│  │  ├─ Image  CardIcon     26 × 26
│  │  ├─ Text   CardKind     mono bold 12, spacing 0.16 em, fills the row
│  │  └─ Text   CardTime     mono 12, faint, right
│  ├─ Text   CardSpeaker  bold 17, bone, margin top 10
│  └─ Text   CardText     regular 16, #D9D2BE, multi-line, wraps at 428
└─ ProgressBar CardLife  full width × 3, bottom edge, no background (counts down)
```

## Menus

Four menus share a frame: a heading, a sub-line (`Header`), four tabs (Standing, Journal, Radio,
Map key; the open one in bone with dark text) and a footer of keys. Build the frame once and copy
it. Root: a full-screen `Frame` with a dark overlay (`#08090A` at 75%) so the world shows faintly.

Register the menus in a `chimeraMenus.conf` override (copy the vanilla one into the addon and
add four entries), each with its layout and class:

| Preset | Class | Layout |
|---|---|---|
| `RZ_StandingMenu` | `RZ_StandingMenu` | `RZ_StandingMenu.layout` |
| `RZ_JournalMenu` | `RZ_JournalMenu` | `RZ_JournalMenu.layout` |
| `RZ_RadioLogMenu` | `RZ_RadioLogMenu` | `RZ_RadioLogMenu.layout` |
| `RZ_MapKeyMenu` | `RZ_MapKeyMenu` | `RZ_MapKeyMenu.layout` |

Add four input actions to the addon's input config (a `chimeraInputCommon.conf` override), in the
character context: `RZ_OpenStanding` (default **K**), `RZ_OpenJournal` (**J**), `RZ_OpenRadio`
(**U**), and `RZ_RadioFilter` (**F**, menu context). Esc, Q/E and up/down use the vanilla menu actions.

### Standing: `RZ_StandingMenu.layout`

```
Frame (root)
├─ (menu frame: heading "STANDING", Text Header, tabs, footer)
├─ Text   Waiting              centred, "Asking around..." (shown until the data arrives)
├─ HorizontalLayout            at (72, 168), spacing 18
│  └─ Frame ×4, one per faction, 431 × 610; names end in the faction number:
│     0 the Haven, 1 Task Force Bastion, 2 the Red Hand, 3 Halcyon
│     ├─ Image   (panel background)
│     ├─ Image   AccentN         full width × 5, top               faction colour
│     ├─ Image   EmblemN         54 × 54 at (22, 18)               faction_* icon
│     ├─ Text    FactionNameN    condensed bold 24, upper case, spacing 0.1 em
│     ├─ Frame   (tier box, 387 × 66, background white 3.5%, border)
│     │  ├─ Image  TierIconN     42 × 42                           tier_* icon
│     │  ├─ Text   TierNameN     condensed bold 26, upper case
│     │  └─ Text   TierNoteN     mono 13, dim
│     ├─ Text    FameValueN      right, mono bold 14     │ ProgressBar FameBarN   387 × 10
│     ├─ Text    InfamyValueN    right, mono bold 14     │ ProgressBar InfamyBarN 387 × 10, fill #D2553F
│     │   (put three 2-unit dark ticks over both bars at 15%, 40% and 75%: the tier bands)
│     ├─ Text    FavorValueN     mono bold 24, faction colour
│     ├─ Text    (label "LEADERS YOU'VE MET") │ Text LeadersN  multi-line, 15
│     ├─ Frame   NeedBlockN
│     │  ├─ Text   (label "WHAT THEY NEED")
│     │  ├─ Text   NeedTitleN     mono bold 12, faction colour
│     │  └─ Text   NeedN          italic 14, multi-line
│     └─ Text    WarnN           bottom strip, red tint 16%, mono 13 "HUNTER SQUADS ARE OUT FOR YOU"
├─ Frame  (between the factions, at (72, 798), 880 × 196)
│  └─ Text   Relations          multi-line, 16
└─ Frame  (tier grid, at (970, 798), 874 × 196): a static 5 × 5 grid of the tier names
```

### Journal: `RZ_JournalMenu.layout` and `RZ_MissionRow.layout`

```
Frame (root)
├─ (menu frame: heading "JOURNAL", Text Header, tabs, footer)
├─ Frame  (list panel, at (72, 168), 660 × 840)
│  ├─ VerticalLayout List      fills the panel (inside a ScrollLayout if you like)
│  └─ Text  Empty              "The radio is quiet." centred
└─ Frame  (detail panel, at (750, 168), 1098 × 840)
   ├─ Image  DetailAccent      full width × 5, top
   ├─ Image  DetailIcon        56 × 56
   ├─ Text   DetailTitle       condensed bold 34, upper case
   ├─ Text   DetailBy          15, dim
   ├─ Text   DetailBrief       italic 18, multi-line, left bar 3 units in the accent
   ├─ Text   (label "WHERE")        │ Text DetailWhere    mono bold 26 / 15
   ├─ Text   (label "REWARDS")      │ Text DetailRewards  mono bold 13
   ├─ Text   (label "REQUIREMENTS") │ Text DetailReq      16, multi-line
   └─ Frame  CluesBlock
      ├─ Text  (label "CLUES YOU'VE READ")
      └─ Text  DetailClues     serif 16 on a paper-coloured panel #E4DDC8 with dark text #23211C
```

`RZ_MissionRow.layout` (**UI > Mission row layout**), 660 × 62:

```
Frame (root)
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
├─ (menu frame: heading "RADIO", tabs, footer with "F  Filter")
├─ Frame  (channels panel, at (72, 168), 450 × 840): static list of the channels (icons and names)
└─ Frame  (log panel, at (546, 168), 1302 × 840)
   ├─ Text   FilterText     mono bold 12 in a bone chip: ALL / OPEN / SECURE / BROADCAST
   ├─ VerticalLayout Log   fills the panel, newest first
   └─ Text   Empty          "Nothing on the air yet."
```

`RZ_RadioRow.layout` (**UI > Radio row layout**), 1302 wide, height fits:

```
Frame (root)
├─ Image  RowAccent     full size, the accent at 6% (secure rows read as tinted)
├─ Text   RowTime       at (26, 14), mono 14, faint
├─ Image  RowIcon       30 × 30 at (114, 14)
├─ Text   RowChannel    at (158, 14), mono bold 12, upper case, spacing 0.14 em
├─ Text   RowSpeaker    after RowChannel, 14, dim
└─ Text   RowText       at (158, 36), 17, multi-line, wraps at 1110
```

### Map key: `RZ_MapKeyMenu.layout`

Static: the menu frame and the legend panel from `UI/Mockups/map_legend.html`: one row per marker (icon 32 × 32 in its colour, name in
bold 15, one line in 13 dim). No names are looked up. The marker icons on the map itself are
still the vanilla markers (`Core/RZ_MapMarkers.c`); giving them these icons is planned.

## Background panel: `RZ_BackgroundPanel.layout`

Put it into the vanilla deploy menu: open the deploy menu's layout (search the Resource Browser
for the deploy or role-selection menu layout), save an override in the addon, and add this layout
as a child on the right (560 wide, anchor top-right, offset (-48, 120)). Add the
`RZ_BackgroundPanelComponent` script handler to its root.

```
Frame (root, 560 wide)
├─ Image  (panel background, top bar 5 units bone)
├─ Text   (label "BACKGROUND")
├─ Frame  PanelBody                shown once the picked loadout is a background
│  ├─ Image  (background icon, 58 × 58: item_radio or similar; static)
│  ├─ Text   BgName                 condensed bold 36, upper case
│  ├─ HorizontalLayout (state chip, border)
│  │  ├─ Image  StateIcon           16 × 16 (status_door_unlocked / status_door_locked)
│  │  └─ Text   StateText           mono bold 12: AVAILABLE / LOCKED
│  ├─ Text   (label "PERK")  │ Text BgPerk      17, #BFD39B, multi-line
│  ├─ Text   (label "FLAW")  │ Text BgFlaw      17, #E4826F, multi-line
│  ├─ Text   (label "STARTING STANDING") │ Text BgStanding  mono bold 14, multi-line
│  └─ Frame  LockedBlock            dashed border, shown when locked
│     └─ Text  LockedText           15, dim, multi-line
├─ Text        Legacy               mono bold 12, dim
└─ ProgressBar LegacyBar            160 × 8, fill #E3A33B
```

Starting gear isn't listed separately: the perk text names it. Reading the loadout's items is planned.
