# The UI layouts

The ReforgerZ layouts are in `UI/layouts/ReforgerZ/`: the HUD, the notice card, the notebook's
frame and its five pages, the notice board, the journal and radio rows, the talk screen, the
reader, the car repair and shop screens, the map icon and the background panel. They were written as text
from the trees below and checked against every widget
name and type the UI scripts look up.
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
- **No text under 18.** Every text in these layouts has a font size (and min font size) of 18 or
  more, the HUD's and the map icons' too. Where a label's slot was shorter than its text, the text
  overflows its slot by a few units rather than move what's below (the Workbench check in
  [UI.md](../docs/UI.md#controllers)).
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
  layout editor (not done yet). The map key's and the Radio page's static icons are named after
  their icon (`Key2_marker_safehouse`, `Chan6_status_secure_radio`).
- **Fonts.** The game's UI fonts (`UI/Fonts/` in 1.9) have no mono or serif face. Where the trees
  say *mono* (and the clue paper's *serif*), the layouts use Roboto:
  `UI/Fonts/Roboto/Roboto_Bold.fnt` or `Roboto_Regular.fnt`. Everything else is RobotoCondensed:
  `RobotoCondensed_Bold.fnt` for *condensed bold* headings and *bold* text,
  `RobotoCondensed_Regular.fnt` for *regular* text and text given only a size (both in
  `UI/Fonts/RobotoCondensed/`). Italic is a font property.
  Labels, like `(label "WHERE")`, are mono bold 18, dim.
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

1. `py tools/icon_atlas.py` packs the white icons (62) from `UI/Icons/png/128/` into
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
steps 1 to 3 again. **To do now:** step 1 has been run for `marker_range` (the 62nd icon, the packs'
range ring), so import the PNG again (step 2) and run step 3; until then the old atlas and imageset
have no ring, and the cells after `marker_radio` would be off by one if only one of the two changed. With **Icon imageset** empty, the scripts leave every image as the layout has
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
│     ├─ Text    InfectionNote  italic 18, dim, wraps
│     └─ Overlay PausedRow      4 below the note, fits its content, border #4FB8B0 50%
│        ├─ Image  (item_stabiliser icon, 18 × 18, #4FB8B0, static)
│        └─ Text   PausedText   mono bold 18, #4FB8B0
├─ Frame  RadioPanel            390 × 60, anchor top-left, offset (96, 54)
│  ├─ Image   (panel background)
│  ├─ Image   RadioAccent       4 × 60, left edge                   faction colour
│  ├─ Image   RadioIcon         34 × 34 at (12, 13)                 status_secure_radio, static, tinted
│  ├─ Text    RadioChannel      at (60, 10), mono bold 13           faction colour
│  └─ Text    RadioSub          at (60, 32), mono 18, dim
├─ HorizontalLayout NoisePanel  anchor bottom-left, offset (552, -62), 10 apart, 85% opacity
│  ├─ Image   (status_noise icon, 26 × 26, bone, static)
│  ├─ HorizontalLayout (3 apart)
│  │  └─ Image Noise1 … Noise6  14 × 8 each, no texture (tinted lit or dim)
│  └─ Text    NoiseText         mono bold 18, dim (QUIET, HEARD, LOUD, VERY LOUD)
├─ HorizontalLayout SeenPanel   anchor bottom-left, offset (588, -90), 10 apart, 85% opacity; hidden until the server sends a visibility (Stealth on)
│  ├─ HorizontalLayout (3 apart)
│  │  └─ Image Seen1 … Seen5    14 × 8 each, no texture (tinted lit, dim, or green when hidden)
│  └─ Text    SeenText          mono bold 18, dim (HIDDEN, LOW, SEEN, IN PLAIN SIGHT)
├─ VerticalLayout (NoticeArea)  460 wide, anchor top-right, offset (-96, 96)
│  ├─ VerticalLayout NoticeStack  the notice cards
│  └─ Text   NoticeQueued       under the stack, right-aligned, mono 18, faint ("+3 in your notebook")
└─ VerticalLayout SubtitlePanel  anchor bottom-centre, offset (0, -204), fits its content, hidden
   └─ Overlay (SubtitleBox)
      ├─ Image  (SubtitleBg)      #0C0E0F at 88%
      └─ SizeLayout (SubtitleWidth)  1,044 wide, padding 28 16 28 20
         └─ VerticalLayout (SubtitleBody)
            ├─ Text  SubtitleSpeaker  condensed bold 24, centred, tinted with the speaker's faction
            └─ Text  SubtitleText     regular 32, #F4F0E6, centred, wraps
```

Without `SubtitlePanel` the HUD shows spoken lines as cards.

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
   │  │  ├─ Text   CardKind     mono bold 18, fills the row
   │  │  └─ Text   CardTime     mono 18, faint, right
   │  ├─ Text   CardSpeaker  bold 18, bone, 10 above, wraps
   │  └─ Text   CardText     regular 18, #D9D2BE, wraps
   ├─ Image   CardAccent    full width × 4, top edge
   ├─ Image   CardHazard    full width × 8, top edge, hazard yellow, hidden; shown for the broadcast
   └─ ProgressBar CardLife  full width × 3, bottom edge, 70% opacity, no background (counts down)
```

Unlike the mockup, `CardHazard` is plain hazard yellow: its stripes would need a texture.

## Menus

### The notebook: `RZ_NotebookMenu.layout`

The notebook's frame (the `RZ_NotebookMenu` preset). Each tab's page layout is created into
`Page` when the tab is shown (**UI > Standing page** and the rest); the names inside the pages are
looked up through the whole menu, so a page's `Header` is the frame's.

```
Frame (root, full screen)
├─ Image   Dimmer          full screen, #08090A at 75%, so the world shows faintly
├─ Frame   Page            full screen, empty: the page goes here
├─ Text    PageEmpty       centred, italic 26, dim, "Nothing here yet." (shown when the page has no layout)
├─ Text    Heading         at (96, 54), condensed bold 46: the tab's name (set by the script)
├─ Text    Header          at (96, 112), 840 wide, mono 18, dim: the sub-line (set by each page)
├─ Image   HeadRule        stretched from 96 to -96 at y 150, 2 high
├─ Button  Tab0 … Tab5     168 × 40 each, 4 apart, right-aligned to -96 at y 100: Standing, Journal, Radio, Map key, Hall of fame, The season
│  └─ Overlay TabNBox
│     ├─ Image  TabNBg      the open tab bone, the others #1A1D20 (tinted by the script)
│     ├─ Panel  TabNLine    border, hidden on the open tab
│     ├─ Text   TabNText    mono bold 18, centred: dark on the open tab, dim on the others
│     └─ Text   TabBadgeN   mono bold 18, hazard yellow, right: the count of new things (hidden at 0)
├─ Button  CloseButton     44 × 44, top right at (-140, 54), with a Text "X"
├─ HorizontalLayout Prompts  bottom left at (96, -54): the footer prompts the script makes (vanilla input buttons)
├─ Text    PromptsText     the same place, mono bold 18, dim: the static footer, hidden once prompts are made
└─ Text    FooterNote      bottom right at (-96, -54), mono 18, faint
```

The pages are the old menus' layouts with the frame parts (overlay, heading, header, rule, tabs,
close button, footer) taken out, and their content moved into the 90% safe area: every horizontal
position and width scaled by 0.973 from the left margin, which moves from 72 to 96. Each page's
root is a full-screen `Frame` named after it (`StandingPage`, `JournalPage`, `RadioPage`,
`MapKeyPage`, `HallPage`, `SeasonPage`). Widget names inside are unchanged, so the drawing code is too.

**Mouse.** The tabs, `CloseButton`, the journal row's root (`MissionRow`), the radio filter chip
(`FilterButton`) and the notice board's `PostButton` are `Button`s. Long text sits in a
`ScrollLayout` so the wheel scrolls it, and on a pad up and down do: `ListScroll` (the journal),
`LogScroll` (the radio), `NotesScroll`, `LeadersScroll0` to `LeadersScroll3` and `PastScroll`.
Menus show the cursor while open; if one doesn't, check its entry in `chimeraMenus.conf`.

**Menu presets.** `Configs/System/chimeraMenus.conf` is an override of the vanilla file (same
GUID) with the ReforgerZ presets added, each with its layout and class. Workbench finds them.

| Preset | Class | Layout |
|---|---|---|
| `RZ_NotebookMenu` | `RZ_NotebookMenu` | `RZ_NotebookMenu.layout` |
| `RZ_NoticeBoardMenu` | `RZ_NoticeBoardMenu` | `RZ_NoticeBoardMenu.layout` |
| `RZ_ReaderMenu` | `RZ_ReaderMenu` | `RZ_ReaderMenu.layout` |
| `RZ_TalkMenu` | `RZ_TalkMenu` | `RZ_TalkMenu.layout` |
| `RZ_CarMenu` | `RZ_CarMenu` | `RZ_CarMenu.layout` |
| `RZ_ShopMenu` | `RZ_ShopMenu` | `RZ_ShopMenu.layout` |

**Input actions.** `Configs/System/chimeraInputCommon.conf` is an override of the vanilla input
config (same GUID) that adds `RZ_OpenNotebook` (**N**) in `IngameContext`, and `RZ_RadioFilter`
(**F**, pad **X**), `RZ_CarStart` (**T**, pad **Y**), `RZ_TurnPreview` (the pad's right stick) and
`RZ_Reply1` to `RZ_Reply7` (**1** to **7**) in `MenuContext`. The list with
the pad routes is in [UI.md](../docs/UI.md#controllers). Esc, Q/E, up/down and Enter use the
vanilla menu actions (B, LB/RB, the D-pad and A on a pad).

### Standing page: `RZ_StandingPage.layout`

Positions below are the old menu's (before the 0.973 scale into the safe area).

```
Frame StandingPage (root)
├─ (in the frame: Header, the player, background and life)
├─ Frame  (Factions)           at (72, 168), 1778 × 610
│  └─ Frame ×4, one per faction, 431 × 610, 18 apart; names end in the faction number:
│     0 the Haven, 1 Task Force Bastion, 2 the Red Hand, 3 Halcyon
│     ├─ Image   (panel background)
│     ├─ Image   AccentN         full width × 5, top               faction colour
│     ├─ Image   EmblemN         54 × 54 at (22, 18)               faction_* icon
│     ├─ Text    FactionNameN    at (92, 30), condensed bold 24
│     ├─ Frame   (tier box, 387 × 66 at (22, 88), background white 3.5%, border)
│     │  ├─ Image  TierIconN     42 × 42                           tier_* icon
│     │  ├─ Text   TierNameN     condensed bold 26: the reputation's name (Mixed Bag, Wildcard...)
│     │  └─ Text   TierNoteN     mono 18, dim: what it counts as, at most 42 characters
│     ├─ Text    FameValueN      right, mono bold 14     │ ProgressBar FameBarN   387 × 10
│     ├─ Text    InfamyValueN    right, mono bold 14 ("36 - 12 TO FORGIVE") │ ProgressBar InfamyBarN 387 × 10, fill #D2553F
│     │   (three 2-unit dark ticks over both bars at 15%, 40% and 75%: the tier bands)
│     ├─ Text    FavorValueN     mono bold 24, faction colour
│     ├─ Text    (label "LEADERS YOU'VE MET") │ Text LeadersN  wraps, 18, in a ScrollLayout
│     ├─ Frame   NeedBlockN
│     │  ├─ Text   (label "WHAT THEY NEED")
│     │  ├─ Text   NeedTitleN     mono bold 18, faction colour
│     │  └─ Text   NeedN          italic 18, wraps
│     └─ Overlay WarnN           bottom strip, 36 high, red tint 16%
│        └─ Text  (WarnTextN)    mono 18, "HUNTER SQUADS ARE OUT FOR YOU"
├─ Frame  (between the factions, at (72, 798), 880 × 196, label "BETWEEN THE FACTIONS")
│  └─ Text   Relations          wraps, 18
├─ Frame  TierGrid (the reputation grid, at (970, 798), 874 × 196): a 5 × 5 grid, labels on the
│  top row ("FAME" in the corner, then "INFAMY UNDER 15" ... "INFAMY 75+") and the left column
│  (the Fame bands); all 18; the 16 cells are Text RepCell_<fame band>_<infamy band> (0-3 each,
│  so RepCell_3_0 is Champion, top left), each over an Image TierGridCell<row><col>. Code writes each
│  cell's name, upper case, and the initials of the factions that put you there, bright where you
│  stand and dim elsewhere: no static names. The cells are condensed bold, so the longest names
│  ("SOFT-HEARTED DEVIL") fit 169 units at 18
└─ Text   Waiting              centred, italic 22, "Asking around..." (shown until the data arrives)
```

### Journal page: `RZ_JournalPage.layout` and `RZ_MissionRow.layout`

```
Frame JournalPage (root)
├─ (in the frame: Header, "3 OPEN")
├─ Frame  (list panel, at (72, 168), 660 × 840)
│  ├─ VerticalLayout (list column)
│  │  ├─ ScrollLayout (fills the column above LabStatus)
│  │  │  └─ VerticalLayout List   the rows
│  │  └─ Text  LabStatus      mono 18, dim, wraps: the lab's line ("Research 63 percent.
│  │                          Waiting: 14 samples' worth, about 4 days of work."); hidden without a lab
│  └─ Text  Empty             centred, italic 18, "The radio is quiet."
└─ Frame  (detail panel, at (750, 168), 1098 × 840)
   ├─ Image  DetailAccent     full width × 5, top
   ├─ Image  DetailIcon       56 × 56 at (34, 31)
   ├─ Text   DetailTitle      at (108, 30), condensed bold 34
   ├─ Text   DetailBy         at (108, 72), 18, dim
   └─ VerticalLayout (detail body, at (34, 112), 1030 × 712)
      ├─ Text   DetailBrief   italic 18, wraps, on a faint panel with a 3-unit faint bar on its left
      ├─ (WHERE and REWARDS side by side)
      │  ├─ Text (label "WHERE")   │ Text DetailWhere    mono bold 20, wraps: the direction, then the area
      │  └─ Text (label "REWARDS") │ Text DetailRewards  mono bold 18, wraps
      ├─ Text   (label "REQUIREMENTS") │ Text DetailReq  18, wraps
      └─ VerticalLayout CluesBlock
         ├─ Text  (label "WHAT YOU KNOW": puzzle clues read, or a questline's notes)
         └─ Text  DetailClues  serif 18, wraps, dark text #23211C on a paper-coloured panel #E4DDC8
                               (first the selected row's detail: "What you've found", "Who it went to")
```

An errand you hold is a row like any other (its icon `mission_unarmed` until `mission_errand` is
drawn); no new widget.

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
   ├─ Text   RowSub        at (70, 34), 18, dim
   ├─ Text   RowDir        right, at (-22, 10), mono bold 14
   └─ Text   RowTime       right, at (-22, 34), mono 18, faint
```

### Radio page: `RZ_RadioPage.layout` and `RZ_RadioRow.layout`

```
Frame RadioPage (root)
├─ (in the frame: Header, "WHAT YOU HEARD AND WERE TOLD THIS LIFE, NEWEST FIRST")
├─ Frame  (channels panel, at (72, 168), 450 × 840): a static list of the channels, static icons and names
└─ Frame  (log panel, at (546, 168), 1302 × 840)
   ├─ Button FilterButton   at (26, 16), 150 × 30, the bone chip; a click cycles the filter
   │  └─ Text FilterText    mono bold 18: ALL / OPEN CHANNEL / SECURE / BROADCAST / MISSIONS / WORLD
   ├─ ScrollLayout (under a rule at y 62, fills the rest)
   │  └─ VerticalLayout Log   the rows, newest first
   └─ Text   Empty          centred, italic 18, "Nothing on the air yet."
```

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
      ├─ Text   RowSpeaker    after RowChannel, 18, dim
      └─ Text   RowText       under them, 18, wraps
```

### Map key page: `RZ_MapKeyPage.layout`

Static: no names are looked up (the script sets only the frame's `Header`). It holds a
legend panel at (72, 168), 640 × 840 before the scale, with one row per marker the map draws: the icon in its colour
(32 × 32; the stronghold and outpost rows show three 24 × 24 icons in faction colours), the name
in bold 18 and one line in 18, dim. Under WHO HOLDS WHAT: stronghold, outpost, Haven safehouse,
Halcyon's lab. Under THE WORLD: world event and sealed checkpoint in hazard yellow, power station
and radio station in bone. The icons are static.

Unlike the mockup (`UI/Mockups/map_legend.html`), it has no Quartermaster or Perimeter rows,
because `RZ_UIFeed.BuildMap` sends neither, and the power station is bone, as on the map, not
hazard.

### Notice board: `RZ_NoticeBoardMenu.layout`

Not a notebook tab: a frame of its own like the notebook's (heading at (96, 54), `Header` at
(96, 112), `CloseButton` 44 × 44 at (-140, 54), the footer in mono 18 at (96, -54)), without tabs.
Opened by the server when a player reads a safehouse board, with **UI > Board menus** on. Its
panels are scaled into the safe area like the pages.

```
Frame (root)
├─ (frame: heading "SAFEHOUSE", Header "NOTICE BOARD: MORTON", CloseButton)
├─ HorizontalLayout Prompts  at (96, -54), 1100 × 40: the footer's prompts (A pin it up, right
│                            a ready-made note, down scroll, B close; X clear board for admins)
├─ Text     PromptsText     the same place, mono bold 18: the static footer, hidden once prompts are made
├─ Frame  (board panel, at (72, 168), 1100 × 840, the panel colour)
│  ├─ ScrollLayout
│  │  └─ Text  Notes       wraps, 18, bone
│  └─ Text  Empty          centred, italic 18, "Nothing pinned up."
└─ Frame  WriteBlock       at (1200, 168), 648 × 210, hidden for exiles
   ├─ Text     (label "LEAVE A NOTE")
   ├─ EditBox  NoteInput   one line, 576 × 32 inside a 600 × 44 box with a border, 18,
   │                       placeholder "Write your note here."
   ├─ Text     NoteLimit   18, dim, wraps
   └─ Button   PostButton  180 × 40, with a Text "PIN IT UP" (18): has the focus when you can write
```

With a pad: left and right put one of seven ready-made notes in the box (not while the box itself
has the focus), and A on PIN IT UP pins it, so writing a note needs no typing.

`NoteInput` is a single-line `EditBox`: the script reads it as an `EditBoxWidget`, and a
multi-line edit box isn't one.

### Hall of fame page: `RZ_HallPage.layout`

```
Frame HallPage (root)
├─ (in the frame: Header, "SEASON 3")
├─ Frame  (this season, at (72, 168), 580 × 840)
│  ├─ Text (label "THIS SEASON")
│  └─ Text SeasonList    wraps, 18 (each record: the category, then the name and value)
├─ Frame  (all time, at (670, 168), 580 × 840)
│  ├─ Text (label "ALL TIME")
│  └─ Text AllTimeList   wraps, 18
├─ Frame  (past seasons, at (1268, 168), 580 × 840)
│  ├─ Text (label "PAST SEASONS")
│  └─ Text PastList      wraps, 18, dim, in a ScrollLayout
└─ Text   Waiting        centred, italic 22, "Reading the names..." (until the data arrives)
```

### The season page: `RZ_SeasonPage.layout`

The hall page's three panels with their own names and labels (written as text, not opened in
Workbench yet).

```
Frame SeasonPage (root)
├─ Frame  MajorsPanel  (at (72, 168), 580 × 840)
│  ├─ Text MajorsLabel  "THE MAJORS"
│  └─ Text MajorsList   wraps, 18 (each major's title in capitals, then its lines; an ending told)
├─ Frame  YouPanel     (at (670, 168), 580 × 840)
│  ├─ Text YouLabel     "AND YOU"
│  └─ Text YouList      wraps, 18 (your lines, then UNFINISHED and what's open)
├─ Frame  ZonePanel    (at (1268, 168), 580 × 840)
│  ├─ Text ZoneLabel    "AS THE ZONE TELLS IT"
│  └─ Text ZoneList     wraps, 18, dim, in a ScrollLayout ZoneScroll (up and down scroll it)
└─ Text   Waiting       centred, italic 22, "Asking around..." (until the data arrives)
```

## Talk screen: `RZ_TalkMenu.layout` and `RZ_TalkReply.layout`

The talk screen (`RZ_TalkMenu` preset), from `UI/Mockups/talk.html`. Written as text.

```
Frame (root, full screen)
├─ Image   Dim             full screen, #08090A at 55% (flat; the mockup's gradient would need a texture)
└─ Frame   TalkPanel       full width, 660 high, anchored to the bottom
   ├─ Image  (TalkBg)        #0F1113
   ├─ Image  TalkAccent      full width × 5, top edge, tinted with the faction colour
   ├─ HorizontalLayout (TalkWho)  at (240, 34)
   │  ├─ Text  TalkName      condensed bold 30, tinted with the faction colour
   │  └─ Text  TalkSub       regular 21, dim, 18 after the name
   ├─ ScrollLayout SaidScroll  at (240, 86), 1,340 × 170 (LB/RB and the wheel scroll it)
   │  └─ SizeLayout (SaidWidth)  1,310 wide
   │     └─ Text TalkSaid    regular 38, #F4F0E6, wraps
   ├─ VerticalLayout Replies   at (240, 268), 1,120 wide: the reply rows
   ├─ VerticalLayout Prompts   at (1400, 268), 280 wide: the footer prompts the script makes
   ├─ Text   PromptsText     the same place, mono bold 18, dim: the static prompts, hidden once made
   └─ Text   (KeysHint)      at (1400, 440), mono 18, faint: "1-7 OR D-PAD AND A: A REPLY. ESC / B LEAVES"
```

`RZ_TalkReply.layout` (**UI > Talk reply layout**), one row, created into `Replies`. The root
is a `Button` named `ReplyRow` (a click says it):

```
Button ReplyRow (root, 4 below it)
└─ SizeLayout (ReplySize)    1,120 × 52
   └─ Overlay (ReplyBox)
      ├─ Image  ReplyBg       #1B1F22, shown on the selected row
      ├─ Image  ReplyBar      5 × 52, left edge, tinted with the faction colour, shown on the selected row
      └─ HorizontalLayout (ReplyLine)  padding 18 left, 20 right, centred vertically
         ├─ SizeLayout (ReplyNumBox) 34 × 34, 20 before the text
         │  └─ Overlay: Panel (ReplyNumLine, border #6B5222) and Text ReplyNum (mono bold 18, tinted)
         ├─ Text   ReplyText     regular 25, fills the row (bright on the selected row, faint when it can't be picked)
         ├─ Text   ReplyTag      mono 18, dim, right ("a job", "trade")
         └─ Text   ReplyNeed     mono 18, dim, right, hidden unless the reply can't be picked
```

Unlike the mockup, the pad-key ring isn't rounded (a `Panel` border), and the selected row has
no extra "A" key on its right (the prompts column shows it).

## Reader: `RZ_ReaderMenu.layout`

The reader (`RZ_ReaderMenu` preset), from `UI/Mockups/reader.html`. Written as text.

```
Frame (root, full screen)
├─ Image   Dim             full screen, #0A0C0D at 72%
└─ Frame   Paper           960 × 800, centred
   ├─ Image  (PaperBg)      #EDE7D6, the whole sheet (flat; a paper texture later)
   ├─ Text   PaperTitle     at (72, 60), 816 wide, condensed bold 34, ink #1C1A16
   ├─ Text   PaperHow       at (72, 110), regular 21, #5F5C55 (hidden when empty)
   ├─ Text   PaperText      at (72, 160), 816 × 520, regular 30, ink, wraps
   ├─ Image  (PaperRule)    across the paper, 100 above its foot, ink at 16%
   ├─ Button PrevButton     44 × 44 at the bottom left (72, -30), "<" in amber on ink (hidden on the first page)
   ├─ Text   PaperPages     at (128, -38), 100 wide, bold 21, #5F5C55, centred ("1 / 2")
   ├─ Button NextButton     44 × 44 at (240, -30), ">" (hidden on the last page)
   ├─ HorizontalLayout Prompts  at (300, -34): the footer prompts the script makes
   ├─ Text   PromptsText    the same place, bold 21, #5F5C55: the static footer, hidden once prompts are made
   └─ Button CloseButton    200 × 44 at the bottom right (-72, -30), "B / ESC  CLOSE" in amber on ink
```

Unlike the mockup, the paper has no grain or folds (they'd need a texture) and is a fixed 800 high.

## Car repair screen: `RZ_CarMenu.layout` and `RZ_CarJob.layout`

The car repair screen (`RZ_CarMenu` preset), from `UI/Mockups/car_repair.html`. Written as text.
Positions inside `CarPanel` are from its top left.

```
Frame (root, full screen)
├─ Image   Dim             full screen, #08090A at 60%
└─ Frame   CarPanel        1,440 × 820, centred
   ├─ Image  (CarBg)        #0F1113
   ├─ Image  (CarAccent)    full width × 5, top edge, amber
   ├─ Text   CarTitle       at (48, 34), 900 wide, condensed bold 34, bone ("The blue two-door")
   ├─ Text   CarDone        right-aligned, ending at (1,320, 44), mono 22, dim ("2 of 5 done")
   ├─ Button CloseButton    44 × 44 at (1,348, 30), "X" in amber
   ├─ Image  (HeadRule)     at (48, 92), 1,344 × 2, #2A2E31
   ├─ Frame  (CarView)      at (48, 120), 640 × 250, #0C0E0F with a #2A2E31 border
   │  ├─ Image  CarPicture   at (0, 10), 640 × 213, #7B8790: the car's side view (UI > Car picture)
   │  ├─ Text   (FrontLabel) at (14, 214), mono bold 18, faint, "FRONT"
   │  └─ Frame  Marker0 … Marker4   46 × 46 each, centred on the job's place on the car (the
   │     │                          mockup's points × 1.778, 10 lower with the picture: (135, 108),
   │     │                          (181, 101), (164, 171), (508, 135), (284, 98))
   │     ├─ Panel  MarkerRingN   the frame's size, border, hidden unless the job is selected (tinted)
   │     ├─ Image  MarkerDotN    32 × 32 at (7, 7), tinted by the job's state
   │     └─ Text   (MarkerNumN)  over the dot, mono bold 20, #15171A, "1" … "5"
   ├─ Text   CarStatus      at (48, 392), 640 × 300, regular 24, #F4F0E6, wraps: what the last job said (hidden when empty)
   ├─ Text   (JobsLabel)    at (730, 120), mono bold 18, dim, "THE JOBS"
   ├─ VerticalLayout Jobs   at (730, 152), 662 wide: the job rows
   ├─ HorizontalLayout Prompts  at (48, 740): the footer prompts the script makes
   ├─ Text   PromptsText    the same place, mono bold 18, dim: the static footer, hidden once prompts are made
   ├─ Text   StartNeed      right-aligned over the button, 20, dim ("Every part first"), hidden when it can start
   └─ Button StartButton    340 × 56 at (1,052, 730)
      └─ Overlay: Image StartBg (#1B1F22, amber when it can start) and Text StartText
         (condensed bold 24, "START THE ENGINE": faint, dark on amber when it can start)
```

`RZ_CarJob.layout` (**UI > Car job layout**), one row, created into `Jobs`. The root is a
`Button` named `JobRow` (a click does the job):

```
Button JobRow (root, 6 below it)
└─ SizeLayout (JobSize)    662 × 76
   └─ Overlay (JobBox)
      ├─ Image  JobBg          #1B1F22, shown on the selected row
      ├─ Image  JobBar         5 × 76, left edge, amber, shown on the selected row
      └─ HorizontalLayout (JobLine)  padding 18 left and right, centred vertically
         ├─ SizeLayout (JobNumBox) 40 × 40, 18 before the text
         │  └─ Overlay: Image JobDot (tinted by state) and Text JobNum (mono bold 20, #15171A)
         ├─ VerticalLayout (JobText)  fills the row
         │  ├─ Text  JobName      condensed bold 28 (bright on the selected row, dim when it needs something)
         │  ├─ Text  JobNeed      regular 21, dim: what you need or have, or "Fitting... 6 s"
         │  └─ SizeLayout (JobProgressBox) 360 × 6, 4 below
         │     └─ ProgressBar JobProgress   amber, shown while the job is under way
         └─ Overlay JobButtonBox  right, hidden when the job has no button
            ├─ Panel  (JobButtonLine)  border #6B5222
            └─ Text   JobButton        mono bold 22, amber ("Fit", "Pour", "Hotwire", "Use the key")
```

State colours (the script tints them): done olive `#9AA35E`, can do now or under way amber
`#E3A33B`, needs something first `#6E6A60`. Unlike the mockup, the markers, the ring and the pad
keys are square (no circle texture), the row's button shows its word without the pad key (the
footer prompts show A), and both wheels are drawn on the picture.

## Shop screen: `RZ_ShopMenu.layout` and `RZ_ShopItem.layout`

The quartermaster's shop screen (`RZ_ShopMenu` preset), from `UI/Mockups/shop.html`. Written as
text. Positions inside `ShopPanel` are from its top left.

```
Frame (root, full screen)
├─ Image   Dim             full screen, #08090A at 60%
└─ Frame   ShopPanel       1,600 × 880, centred
   ├─ Image  (ShopBg)       #121416
   ├─ Image  ShopAccent     full width × 5, top edge, tinted with the faction colour
   ├─ Text   ShopTitle      at (48, 28), condensed bold 36, bone ("Haven stores")
   ├─ Text   ShopSub        at (48, 74), regular 22, dim ("Ada Reyes, quartermaster in Morton")
   ├─ Text   ShopStanding   right-aligned, ending at (1,472, 30), regular 24, bone ("You're Trusted")
   ├─ Text   ShopFavor      right-aligned under it, mono bold 28, tinted with the faction colour ("64 Favor")
   ├─ Button CloseButton    44 × 44 at (1,508, 30), "X" in amber
   ├─ Image  HeadRule       at (48, 112), 1,504 × 2, tinted with the faction colour
   ├─ Button Tab0 … Tab4    284 × 48 each at (48 + 304 × N, 132), hidden when the tab is empty
   │  └─ Overlay: Image TabNBg (#1A1D20; the open tab in the faction colour) and
   │     Text TabNText (mono bold 20: FOOD AND WATER, MEDICAL, GEAR AND TOOLS, WEAPONS AND AMMO, SERVICES)
   ├─ Frame  (ItemList)     at (48, 200), 820 × 560, #0F1113
   │  ├─ ScrollLayout ItemScroll   fills it (the pad's up/down keep the selected row in view)
   │  │  └─ SizeLayout (ItemWidth) 820 wide
   │  │     └─ VerticalLayout Items   the item rows
   │  └─ Text  ItemsEmpty   centred, regular 24, dim, "Nothing on this shelf."
   ├─ Frame  Details        at (900, 200), 652 × 560, hidden when nothing is selected
   │  ├─ Frame  (PreviewBox)  652 × 300
   │  │  ├─ Image  (PreviewBg)     #0F1113
   │  │  ├─ Image  (PreviewFloor)  the bottom 60, #16191C
   │  │  ├─ Frame  PreviewSlot     fills the box: the script makes the item preview in it
   │  │  ├─ Image  PreviewDim      fills the box, black at 55%, shown when you can't have it
   │  │  ├─ Button PreviewButton   fills the box, no visual: a click turns the picture
   │  │  ├─ Panel  (PreviewLine)   border #22262A
   │  │  ├─ Text   (PreviewLabel)  at (16, 12), mono bold 18, faint, "3D PREVIEW"
   │  │  └─ Image  PreviewLock     56 × 56, centred, shown when it's above your standing
   │  ├─ Text   DetailName    at (0, 318), condensed bold 32
   │  ├─ Text   DetailDesc    at (0, 362), 652 × 110, regular 22, #D9D2BE, wraps: its description and weight
   │  ├─ Text   DetailPrice   at (0, 486), mono bold 28, tinted ("4 Favor")
   │  ├─ Text   DetailHave    at (0, 524), mono 20, dim ("you have 64")
   │  ├─ Text   DetailLocked  right-aligned at (320, 482), mono bold 22, red, "LOCKED", shown when you can't have it
   │  ├─ Text   DetailNeed    right-aligned at (220, 516), 432 wide, regular 22, dim: why not
   │  └─ Button BuyButton     240 × 60 at (412, 490), hidden when you can't have it
   │     └─ Overlay: Image BuyBg (the faction colour; dark when you can't afford it) and
   │        Text BuyText (condensed bold 26, #15171A: BUY, SIGN OUT, ASK)
   ├─ Text   ShopStatus     at (48, 772), 1,504 wide, regular 22, #F4F0E6, wraps: what the last purchase said (hidden when empty)
   ├─ HorizontalLayout Prompts  at (48, 822): the footer prompts the script makes
   └─ Text   PromptsText    the same place, mono bold 18, dim: the static footer, hidden once prompts are made
```

The picture is vanilla's `ItemPreviewWidget`, made by the script inside `PreviewSlot`
(`GameWidgetType.ItemPreviewWidgetTypeID`, anchored to fill it), so the layout needs no preview
widget of its own.

`RZ_ShopItem.layout` (**UI > Shop item layout**), one row, created into `Items`. The root is a
`Button` named `ItemRow` (a click selects it):

```
Button ItemRow (root, 4 below it)
└─ SizeLayout (ItemSize)    820 × 64
   └─ Overlay (ItemBox)
      ├─ Image  ItemBg         #1B1F22, shown on the selected row
      ├─ Image  ItemBar        5 × 64, left edge, tinted with the faction colour, on the selected row
      └─ HorizontalLayout (ItemLine)  padding 22 left and right, centred vertically
         ├─ Image  ItemLock    26 × 26, the status_door_locked icon, shown when it's above your standing
         ├─ Text   ItemName    regular 26, fills the row (bright on the selected row, faint when you can't have it)
         ├─ Text   ItemNeed    mono 20, dim, right: why not, shown when you can't have it
         └─ Text   ItemPrice   mono bold 22, right, tinted ("4 Favor"; empty for a service)
```

Unlike the mockup, the rows have no item icon (the picture on the right shows the item), and the
lock is the door-lock icon from the imageset.

## Map icon: `RZ_MapIcon.layout`

One marker on the real map, created into the map's root by `RZ_MapIconsUI` on the map
configuration when **UI > Map icon layout** names this file. Neither is set up yet
([UI.md](../docs/UI.md#built-and-planned)). The script centres the 28 × 28 icon on the spot and
tints it. For a range ring (`marker_range`) it sets the root's and **Icon**'s size every frame to the
pack's ground on the map (28 to 320 across), so **Icon** must sit in the root's frame slot, top-left,
not stretched by anchors.

```
Frame (root, 28 × 28, top-left anchor, ignores the cursor)
├─ Image  Icon       28 × 28, a marker_* icon (white; the script sets the image and tints it)
└─ Text   Label      under the icon, offset (-60, 30), 148 wide, centred, 18, bone, a dark shadow offset by 1
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
│  │  └─ Text   StateText           mono bold 18: AVAILABLE / LOCKED
│  ├─ Text   (label "PERK")  │ Text BgPerk      18, #BFD39B, wraps
│  ├─ Text   (label "FLAW")  │ Text BgFlaw      18, #E4826F, wraps
│  ├─ Text   (label "STARTING STANDING") │ Text BgStanding  mono bold 18, wraps
│  ├─ Text   (label "STARTING GEAR")     │ Text BgGear      18, wraps
│  └─ Overlay LockedBlock           with a border, shown when locked
│     └─ Text  LockedText           18, dim, wraps
├─ Text        Legacy               mono bold 18, dim
└─ ProgressBar LegacyBar            160 × 8, fill #E3A33B, beside Legacy
```

`BgGear` lists what the loadout spawned with the last time anyone took it, or the background's
gear line until then. A locked loadout is unavailable in the menu itself (`RZ_LoadoutLock.c`).
