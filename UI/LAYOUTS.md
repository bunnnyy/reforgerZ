# The UI layouts

The ReforgerZ layouts are in `UI/layouts/ReforgerZ/`: the HUD, the notice card, the notebook's
frame and its five pages, the notice board, the journal and radio rows, the talk screen, the
reader, the car repair, shop and crafting screens, the spawn screen, the map icon and the map legend. They were written as text
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
- **Icons** are `Image` widgets. An icon a script swaps gets its image by name from **UI > Icon
  imageset**, and its tint. An icon marked *static* below has its image set in the layout: `Texture`
  is the `RZ_Icons` imageset and `Image` the icon's name (without them it draws as a flat square in
  its colour). The Radio page's static icons are named after their icon (`Chan6_status_secure_radio`).
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

1. `py tools/icon_atlas.py` packs the white icons (72) from `UI/Icons/png/128/` into
   `UI/Textures/ReforgerZ/RZ_Icons.png`: 1024 × 1152, eight 128-pixel cells to a row, in name
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
steps 1 to 3 again. **To do now:** steps 1 and 3 have been run for the Standing page's nine icons
(`standing_*` and `need_*`, 72 in all), so import the PNG again (step 2). Until then the imported
`.edds` is the 63-icon atlas while the imageset lists 72 in a taller atlas, so icons show the wrong
cells. The imageset's `.meta` (its GUID) is unchanged, so nothing else needs doing. With **Icon imageset** empty, the scripts leave every image as the layout has
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
├─ HorizontalLayout StandPanel  anchor bottom-left, offset (552, -30), 90% opacity; hidden unless the cure finale's stand line is fresh
│  └─ Text    StandText         mono bold 18, bone ("CLEAN ROOM: BASTION. THE HAVEN TAKING: 0:35 OF 1:00.")
├─ HorizontalLayout LookPanel   anchor bottom-left, offset (552, -122), 8 apart, 95% opacity; hidden unless borrowed kit's look is coming (RZ_Hud_Look)
│  ├─ Image   LookIcon          24 × 24, the tier_watched eye (set by code), gold
│  └─ Text    LookText          bold 20 (min 18), bone ("PASS AS BASTION: 2 OF 2", or "PASS AS BASTION: SEEN THROUGH")
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
├─ Button  Tab0 … Tab4     168 × 40 each, 4 apart, right-aligned to -96 at y 100: Standing, Journal, Radio, Hall of fame, The season
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
`HallPage`, `SeasonPage`). Widget names inside are unchanged, so the drawing code is too.

**Mouse.** The tabs, `CloseButton`, the journal row's root (`MissionRow`), the radio filter chip
(`FilterButton`), the Standing page's rows (`Row0` to `Row3`, which also open under the mouse) and
the notice board's `PostButton` are `Button`s. Long text sits in a `ScrollLayout` so the wheel
scrolls it, and on a pad up and down do: `ListScroll` (the journal), `LogScroll` (the radio),
`NotesScroll` and `PastScroll`. On the Standing page up and down open the next row instead.
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
| `RZ_SpawnMenu` | `RZ_SpawnMenu` | `RZ_SpawnMenu.layout` |
| `RZ_CraftMenu` | `RZ_CraftMenu` | `RZ_CraftMenu.layout` |

**Input actions.** `Configs/System/chimeraInputCommon.conf` is an override of the vanilla input
config (same GUID) that adds `RZ_OpenNotebook` (**N**, pad **View + RB**) in `IngameContext`, and
`RZ_RadioFilter` (**F**, pad **X**), `RZ_CarStart` (**T**, pad **Y**), `RZ_TurnPreview` (the pad's right
stick) and `RZ_Reply1` to `RZ_Reply5` (**1** to **5**, pad **LT/RT + D-pad**) in `MenuContext`, and `RZ_MapLegend` (**I**, pad **Y**) in
`MapContext`. The list with
the pad routes is in [UI.md](../docs/UI.md#controllers). Esc, Q/E, up/down and Enter use the
vanilla menu actions (B, LB/RB, the D-pad and A on a pad).

### Standing page: `RZ_StandingPage.layout`

Design G2: one row per faction, mostly icons, one row open at a time (behaviour in
[UI.md](../docs/UI.md#the-notebook)). Positions are at 1920×1080, in the 90% safe area. Every icon
has its image set in the layout (the `RZ_Icons` imageset and a name, so none draws as a flat
square); the script swaps the ones that change. The layout is written by a script, so it has no
mockup of its own.

```
Frame StandingPage (root)
├─ (in the frame: Header, the player, background and life, then the whole picture as said)
├─ Frame  ColumnHeads          at (96, 164), 1728 × 34: one static icon (28) and a label (mono bold
│                              18, dim) over each column: FACTION, WHAT THEY CALL YOU (standing_word),
│                              FAVOR (standing_favor), LEADERS (standing_leader), NEEDS
│                              (status_shortage), KIT (standing_kit)
├─ Frame  Rows                 at (96, 206), 1728 × 720
│  └─ Button RowN ×4 (style blank; N is the faction: 0 the Haven, 1 Task Force Bastion, 2 the Red
│     │  Hand, 3 Halcyon). The script places them: the open row 444 high, the others 84, 8 apart
│     └─ Frame RowFrameN (fills the button)
│        ├─ Image  RowBgN        panel, full size
│        ├─ Image  RowOpenN      bone 6%, full size, shown on the open row
│        ├─ Panel  RowLineN      border; bone on a pinned row, #2A2E31 otherwise (tinted)
│        ├─ Image  AccentN       8 wide, full height, left edge         faction colour
│        ├─ Image  EmblemN       52 × 52 at (28, 16)                    faction_* icon, faction colour
│        ├─ Text   FactionNameN  at (96, 25), 320 × 34, condensed bold 28 ("TASK FORCE BASTION")
│        ├─ Image  TierIconN     48 × 48 at (440, 18)                   tier_* icon, tier colour
│        ├─ Text   TierNameN     at (500, 16), 480 × 52, condensed bold 40, tier colour: what they call you
│        ├─ Image  FavorIconN    36 at (1000, 24)                       standing_favor, faction colour
│        ├─ Text   FavorValueN   at (1046, 24), 140 × 36, mono bold 30
│        ├─ Image  LeaderIconN   36 at (1196, 24)                       standing_leader
│        ├─ Text   LeaderCountN  at (1242, 26), 120 × 34, mono bold 26: the count or NONE
│        ├─ Image  NeedIconN     40 at (1380, 22)                       need_*, status_shortage or item_sample; hidden without a need
│        ├─ Text   NeedTitleN    at (1430, 28), 170 × 30, mono bold 22: FOOD, PEOPLE... or NONE
│        ├─ Image  KitIconN      40 at (1620, 22)                       standing_kit: dim, yellow, red; hidden without a kit line
│        └─ Frame  DetailN       inset 28 left and right, 96 from the top, 16 from the bottom; shown on the open row
│           ├─ Image  DetailRuleN   2 high across the top, #2A2E31
│           ├─ Frame  SeeBlockN     at x 0, 400 wide, from 18 down
│           │  ├─ Image SeeIconN    30, standing_word │ Text SeeLabelN "HOW THEY SEE YOU", mono bold 20, dim
│           │  ├─ ScrollLayout SeeScrollN (44 from the top to 96 from the bottom)
│           │  │  └─ Text SeeTextN  22, wraps: the word, the note, left to forgive, the hunters
│           │  └─ Frame NextBlockN  the bottom 88, hidden at the top
│           │     ├─ Image NextIconN 26, standing_next │ Text NextLabelN "NEXT", mono bold 18, dim
│           │     └─ Text  NextTextN  at (0, 32), 400 × 56, condensed bold 22, wraps (one or two lines)
│           ├─ Frame  LeadersBlockN at x 424, 400 wide
│           │  ├─ Image LeadersIconBigN 30, standing_leader │ Text "LEADERS YOU'VE MET"
│           │  └─ ScrollLayout LeadersScrollN └─ Text LeadersTextN  22, wraps: names and titles, then a verdict
│           ├─ Frame  NeedBlockN    at x 848, 400 wide
│           │  ├─ Image NeedIconBigN 30, the need's icon │ Text "WHAT THEY NEED"
│           │  ├─ Text  NeedNameN   at (0, 44), mono bold 24, faction colour
│           │  └─ ScrollLayout NeedScrollN (from 80) └─ Text NeedTextN  italic 22, wraps
│           └─ Frame  KitBlockN     at x 1272, 400 wide
│              ├─ Image KitIconBigN 30, standing_kit │ Text "IN THEIR KIT"
│              └─ ScrollLayout KitScrollN └─ Text KitTextN  22, wraps
├─ Text   RelationsLabel       at (96, 938), mono bold 18, dim, "BETWEEN THE FACTIONS"
├─ Text   Relations            at (356, 936), 1468 × 44, 18, wraps: the truces and feuds on one line
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

An errand you hold is a row like any other (its icon `mission_errand`); no new widget.

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

The talk screen (`RZ_TalkMenu` preset), New Vegas style: a compact panel in the lower middle, so
the person stays in view. Written as text. Positions inside the panel are from its top left.

```
Frame (root, full screen; no dim, the world shows as it is)
└─ Frame   TalkPanel       1,190 × 376, anchored bottom centre, its bottom 56 up (x 365–1555, y 648–1024)
   ├─ Image  (TalkBg)        #121416 at 85%
   ├─ Panel  (TalkLine)      border #2A2E31
   ├─ Image  TalkAccent      full width × 5, top edge, tinted with the faction colour
   ├─ HorizontalLayout (TalkWho)  at (28, 14), 1,134 × 34
   │  ├─ Text  TalkName      condensed bold 28, tinted with the faction colour
   │  └─ Text  TalkSub       regular 20, dim, 16 after the name
   ├─ ScrollLayout SaidScroll  at (28, 52), 1,134 × 94: three lines (LB/RB and the wheel scroll it)
   │  └─ SizeLayout (SaidWidth)  1,104 wide
   │     └─ Text TalkSaid    regular 26, #F4F0E6, wraps
   ├─ VerticalLayout Replies   at (28, 152), 1,134 × 180: the reply rows, five at most
   ├─ HorizontalLayout Prompts at (28, 336), 760 × 34: the footer prompts the script makes, one line
   ├─ Text   PromptsText     at (28, 340), mono bold 18, dim: the static prompts, hidden once made
   └─ Text   (KeysHint)      right-aligned, ending at (1162, 340), mono 18, faint: "1-5 OR D-PAD AND A: A REPLY"
```

`RZ_TalkReply.layout` (**UI > Talk reply layout**), one row, created into `Replies`. The root
is a `Button` named `ReplyRow`: the script adds a handler that lights the row when the cursor comes
onto it and says it on a click.

```
Button ReplyRow (root, 2 below it)
└─ SizeLayout (ReplySize)    1,134 × 34
   └─ Overlay (ReplyBox)
      ├─ Image  ReplyBg       #1B1F22, shown on the selected row
      ├─ Image  ReplyBar      4 × 34, left edge, tinted with the faction colour, shown on the selected row
      └─ HorizontalLayout (ReplyLine)  padding 16 left and right, centred vertically
         ├─ SizeLayout (ReplyNumBox) 28 × 28, 16 before the text
         │  └─ Overlay: Panel (ReplyNumLine, border #6B5222) and Text ReplyNum (mono bold 18, tinted)
         ├─ Text   ReplyText     regular 25, fills the row (bright on the selected row, grey when there's
         │                       nothing new behind it, faint when it can't be picked)
         ├─ Text   ReplyTag      mono 18, dim, right ("a job", "trade")
         └─ Text   ReplyNeed     mono 18, dim, right, hidden unless the reply can't be picked
```

Unlike the mockup (`UI/Mockups/talk.html`), the panel isn't full width and the world isn't dimmed,
the pad-key ring isn't rounded (a `Panel` border), and the selected row has no extra "A" key on its
right (the prompts line shows it).

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

## Crafting screen: `RZ_CraftMenu.layout` and `RZ_CraftRow.layout`

The crafting screen at a workbench (`RZ_CraftMenu` preset), from `UI/Mockups/crafting.html`.
Written as text. Positions inside `CraftPanel` are from its top left.

```
Frame (root, full screen)
├─ Image   Dim             full screen, #08090A at 60%
└─ Frame   CraftPanel      1,600 × 880, centred
   ├─ Image  (CraftBg)      #121416
   ├─ Image  (CraftAccent)  full width × 5, top edge, amber
   ├─ Text   CraftTitle     at (48, 28), condensed bold 36, bone ("Workbench")
   ├─ Text   CraftSub       at (48, 74), regular 22, dim (what the bench looks like)
   ├─ Text   CraftReady     right-aligned, ending at (1,472, 44), regular 24, bone ("You can make 3 of 8")
   ├─ Button CloseButton    44 × 44 at (1,508, 30), "X" in amber
   ├─ Image  (HeadRule)     at (48, 112), 1,504 × 2, amber
   ├─ Text   (RecipesLabel) at (48, 132), mono bold 18, dim, "RECIPES"
   ├─ Frame  (RecipeList)   at (48, 166), 820 × 594, #0F1113
   │  ├─ ScrollLayout RecipeScroll  fills it (the pad's up/down keep the selected row in view)
   │  │  └─ SizeLayout (RecipeWidth) 820 wide
   │  │     └─ VerticalLayout Recipes   the recipe rows
   │  └─ Text  RecipesEmpty  centred, regular 24, dim, "Nothing set up to make anything."
   ├─ Frame  Details        at (900, 132), 652 × 628, hidden when nothing is selected
   │  ├─ Frame  (PreviewBox)  652 × 240
   │  │  ├─ Image  (PreviewBg)     #0F1113
   │  │  ├─ Image  (PreviewFloor)  the bottom 50, #16191C
   │  │  ├─ Frame  PreviewSlot     fills the box: the script makes the item preview in it
   │  │  ├─ Panel  (PreviewLine)   border #22262A
   │  │  └─ Text   (PreviewLabel)  at (16, 12), mono bold 18, faint, "WHAT YOU MAKE"
   │  ├─ Text   DetailName    at (0, 252), condensed bold 32
   │  ├─ Text   DetailDesc    at (0, 296), 652 × 102, regular 22, #D9D2BE, wraps: its description and weight
   │  ├─ Text   (TakesLabel)  at (0, 402), mono bold 18, dim, "IT TAKES"
   │  ├─ Frame  Input0 … Input2  652 × 40 each at (0, 432 + 42 × N), hidden when the recipe has fewer inputs
   │  │  ├─ Image  InputNBar     5 × 28 at (0, 6), olive when you have it, red when you don't
   │  │  ├─ Text   InputNName    at (20, 4), 360 wide, regular 24, bone ("Empty tin")
   │  │  ├─ Text   InputNCount   right-aligned at (380, 6), 130 wide, mono 22, dim ("1 of 2")
   │  │  └─ Text   InputNState   right-aligned at (522, 6), 130 wide, mono bold 20: HAVE (olive) or MISSING (red)
   │  ├─ Text   DetailTime    at (0, 562), 400 wide, mono bold 22, amber ("About 8 seconds at the bench", "Working... 6 s")
   │  ├─ ProgressBar CraftProgress  at (0, 594), 380 × 6, amber, shown while the work is under way
   │  ├─ Text   DetailNoise   at (0, 602), 400 wide, regular 20, dim ("Some hammering. It carries a little.")
   │  └─ Button CraftButton   240 × 60 at (412, 562)
   │     └─ Overlay: Image CraftBg (amber when it can go; #1B1F22 otherwise) and
   │        Text CraftText (condensed bold 26, "CRAFT": dark on amber, faint otherwise)
   ├─ Text   CraftStatus    at (48, 772), 1,504 wide, regular 22, #F4F0E6, wraps: what the last pick said (hidden when empty)
   ├─ HorizontalLayout Prompts  at (48, 822): the footer prompts the script makes
   └─ Text   PromptsText    the same place, mono bold 18, dim: the static footer, hidden once prompts are made
```

The picture is vanilla's `ItemPreviewWidget`, made by the script inside `PreviewSlot`
(`RZ_ItemPreview`), as on the shop screen.

`RZ_CraftRow.layout` (**UI > Craft row layout**), one row, created into `Recipes`. The root is a
`Button` named `RecipeRow` (a click selects it):

```
Button RecipeRow (root, 4 below it)
└─ SizeLayout (RecipeSize)    820 × 64
   └─ Overlay (RecipeBox)
      ├─ Image  RecipeBg         #1B1F22, shown on the selected row
      ├─ Image  RecipeBar        5 × 64, left edge, amber, shown on the selected row
      └─ HorizontalLayout (RecipeLine)  padding 22 left and right, centred vertically
         ├─ Image  RecipeIcon    32 × 32, the recipe's item_* icon (set by code), hidden without one or without the imageset
         ├─ Text   RecipeName    regular 26, fills the row (bright on the selected row, faint when you're short)
         ├─ Text   RecipeState   mono bold 20, right: READY (olive), "2 MISSING" (dim) or WORKING (amber)
         └─ Text   RecipeTime    mono bold 22, amber, right ("8 s")
```

## Map icon: `RZ_MapIcon.layout`

One marker on the real map, created into the map's root by `RZ_MapIconsUI` on the map
configuration when **UI > Map icon layout** names this file. Neither is set up yet
([UI.md](../docs/UI.md#built-and-planned)). The script centres the 48 × 48 icon on the spot and
tints it. For a range ring (`marker_range`) it sets the root's and **Icon**'s size every frame to the
pack's ground on the map (48 to 320 across), so **Icon** must sit in the root's frame slot, top-left,
not stretched by anchors.

```
Frame (root, 48 × 48, top-left anchor, ignores the cursor)
├─ Image  Icon       48 × 48, a marker_* icon (white; the script sets the image and tints it)
└─ Text   Label      under the icon, offset (-116, 50), 280 × 130 (up to four lines), centred, condensed bold 26, bone, a dark shadow offset by 1
```

Markers within 40 m of an earlier one share its label, one line each (`RZ_MapIconsUI.MergeLabels`); their
own labels are empty, so a stronghold, its quartermaster and a job at the same house never draw their
labels on top of each other. Range rings keep their own.

## Map legend: `RZ_MapLegend.layout`

The map key on the full-screen map (**UI > Map legend layout**), created into the map's root by
`RZ_MapIconsUI` when the map opens, with the icon layer. Written as text. The script sets each
`Key_<icon>` image from the imageset and tints it as the map does (the stronghold in Bastion's
olive and the outpost in the Red Hand's red, as examples; the safehouse amber, the lab teal, the
event, the checkpoint and the range ring hazard yellow, the stations bone), and fades the root
with the zoom (`SetOpacity`; hidden at 0). Positions are from the root's top left.

```
Frame MapLegend (root, 640 × 400, anchor bottom-left, offset (96, -190): above vanilla's key hints and scale bar)
├─ Image  (LegendBg)       #121416 at 90%
├─ Image  (LegendAccent)   full width × 5, top edge, bone
├─ Panel  (LegendLine)     border #2A2E31
├─ Text   (LegendTitle)    at (20, 16), condensed bold 28, "MAP KEY"
├─ Text   (LegendSub)      right-aligned, ending at (620, 24), mono bold 20, dim, "COLOUR: WHOSE IT IS"
├─ Text   (label "WHO HOLDS WHAT") at (20, 64), mono bold 20   ├─ Text (label "THE WORLD") at (330, 64)
├─ Image  Key_marker_stronghold   40 × 40 at (20, 102)  ├─ Image Key_marker_event       at (330, 102)
├─ Image  Key_marker_outpost      at (20, 152)          ├─ Image Key_marker_checkpoint  at (330, 152)
├─ Image  Key_marker_safehouse    at (20, 202)          ├─ Image Key_marker_power       at (330, 202)
├─ Image  Key_marker_lab          at (20, 252)          ├─ Image Key_marker_radio       at (330, 252)
│                                                      └─ Image Key_marker_range       at (330, 302)
├─ Text   (KeyName_<icon>) beside each icon, 12 to its right, condensed bold 22, bone: Stronghold,
│         Outpost, Haven safehouse, Halcyon's lab; World event, Sealed checkpoint, Power station,
│         Radio station, A pack's ground
├─ HorizontalLayout LegendPrompt   bottom left at (20, -14): the script makes the RZ_MapLegend prompt ("Hide the key")
└─ Text   LegendPromptText         the same place, mono bold 20, dim, "I / Y  HIDE THE KEY": hidden once the prompt is made
```

Unlike the mockup (`UI/Mockups/map_legend.html`), it has no Quartermaster or Perimeter rows,
because `RZ_UIFeed.BuildMap` sends neither.

## Spawn screen: `RZ_SpawnMenu.layout` and `RZ_SpawnRow.layout`

The spawn screen (`RZ_SpawnMenu` preset), from `UI/Mockups/background_picker.html`, in place of
vanilla's deploy menu. Written as text. It fills the screen (no world behind it: you have no body
yet).

```
Frame (root, full screen)
├─ Image   (Backdrop)      full screen, #0B0D0E
├─ Text    (Heading)       at (96, 54), condensed bold 46, "WHO YOU WERE"
├─ Text    (Header)        at (96, 112), mono 18, dim: one line on what the pick decides
├─ Image   (HeadRule)      at (96, 150), 1,728 × 2, #3A3F42
├─ Frame   (List)          at (96, 170), 560 × 730
│  ├─ Image (ListBg)        #0F1113
│  ├─ ScrollLayout RowScroll  fills it (the pad's up/down keep the picked row in view)
│  │  └─ SizeLayout (RowWidth) 560 wide
│  │     └─ VerticalLayout Rows   the background rows
│  └─ Text  ListEmpty       centred, regular 24, dim, "Waiting for the Zone to answer." (hidden)
├─ Frame   (Panel)         at (696, 170), 1,128 × 730; positions inside from its top left
│  ├─ Image (PanelBg)       #121416 at 90%; Image (PanelAccent) 5 high, bone; Panel (PanelLine) border
│  ├─ Text  (label "BACKGROUND") at (32, 24)
│  ├─ Frame PanelBody       fills the panel, shown once a background is picked
│  │  ├─ Text  BgName        at (32, 52), condensed bold 46 (the script sends it in capitals)
│  │  ├─ Image StateIcon     24 × 24 at (32, 118) (status_door_unlocked / status_door_locked)
│  │  ├─ Text  StateText     at (66, 118), mono bold 20, tinted: AVAILABLE, LOCKED, NOT ON THIS SERVER
│  │  ├─ Image (PanelRule)   at (32, 160), 1,064 × 2, #2A2E31
│  │  ├─ Text  (label "PERK") at (32, 180)  │ Text BgPerk at (32, 210), 500 × 150, regular 24, #BFD39B, wraps
│  │  ├─ Text  (label "FLAW") at (32, 372)  │ Text BgFlaw at (32, 402), 500 × 90, regular 24, #E4826F, wraps
│  │  ├─ Text  (label "STARTING GEAR") at (580, 180)     │ Text BgGear at (580, 210), 516 × 150, regular 24, wraps
│  │  ├─ Text  (label "STARTING STANDING") at (580, 372) │ Text BgStanding at (580, 402), 516 × 110, mono bold 22, wraps
│  │  └─ Overlay LockedBlock at (32, 530), 1,064 × 110, #0D0F11 at 82% with a border, shown when locked
│  │     └─ Text  LockedText  regular 22, dim, wraps, padding 20 left and right: how to unlock it
│  ├─ Text  Legacy          at (32, 672), mono bold 20, dim ("LEGACY 6 / 10 - THE DRIFTER AT 10")
│  └─ Overlay (LegacyTrack) at (700, 680), 240 × 10: Image (LegacyTrackBg) #2A2E31 and ProgressBar LegacyBar, fill #E3A33B
├─ Text    (label "START")  at (696, 928)
├─ Button  PlacePrev       40 × 40 at (788, 922), "<" on #1A1D20 (a click: the previous place)
├─ Text    PlaceName       at (838, 924), 420 wide, centred, condensed bold 28: the spawn point's name
├─ Button  PlaceNext       40 × 40 at (1,268, 922), ">" (the next place); both arrows hidden with one place
├─ Text    PlaceCount      at (1,322, 930), mono bold 18, dim ("2 OF 7")
├─ Text    SpawnStatus     at (696, 974), 740 wide, regular 22, #F4F0E6: why nothing happened (hidden when empty)
├─ Button  DeployButton    360 × 72 at (1,464, 920)
│  └─ Overlay: Image DeployBg (bone; #1B1F22 while it can't go) and Text DeployText (condensed bold 30:
│     DEPLOY, DEPLOY IN 0:08, ON YOUR WAY; dark on bone, dim when it can't go)
├─ HorizontalLayout Prompts  bottom left at (96, -54): the footer prompts the script makes
└─ Text    PromptsText     the same place, mono bold 18, dim: the static footer, hidden once prompts are made
```

`RZ_SpawnRow.layout` (**UI > Spawn row layout**), one background, created into `Rows`. The root is
a `Button` named `SpawnRow` (a click picks it):

```
Button SpawnRow (root, 4 below it)
└─ SizeLayout (RowSize)    560 × 50
   └─ Overlay (RowBox)
      ├─ Image  RowBg        #1B1F22, shown on the picked row
      ├─ Image  RowBar       5 × 50, left edge, bone, on the picked row
      └─ HorizontalLayout (RowLine)  padding 22 left and right, centred vertically
         ├─ Image  RowLock   24 × 24, the status_door_locked icon, shown when it's locked
         ├─ Text   RowName   regular 26, fills the row (bright on the picked row, faint when locked)
         └─ Text   RowState  mono bold 18, dim, right: LOCKED, or empty
```

Unlike the mockup, the panel has no background icon and the gear is one line of text, not chips.
