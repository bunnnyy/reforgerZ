# UI and icons

The ReforgerZ HUD, menus and icon set: how they look, what each screen shows, how the scripts
feed them, and what's built. The layouts' widget trees and the icon atlas are in
[UI/LAYOUTS.md](../UI/LAYOUTS.md); the setup steps are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui). The setting is in [LORE.md](LORE.md), and the voices used in
every line of text are in [WRITING.md](WRITING.md).

**Status:** the icon set, the mockups, the layouts, the icon atlas and its imageset, and the
menu and input config overrides are built. In Workbench play the HUD works: the styled notice
cards, the stealth visibility bars and the noise meter show. The notebook, the talk screen, the
reader, the car repair screen, the shop screen, the crafting screen, the cooking screen, the radio screen, the object menu, the power panel, the death brief, the subtitles and notice pacing
are written (their layouts as text) and compile, but haven't been played. The spawn screen, the map key and the map's
icons are seen in Workbench play with the keyboard; pad not yet ([Built and planned](#built-and-planned)).

## Default first, then menus

The author's standing rule: **everything uses as much of the game's own stuff as possible, and
menus as much as possible, even ones we have to make, except for what has to be done quickly in a
rush.** For each interaction, in this order:

1. **Vanilla.** If the base game already does it (its inventory, looting, pick-up, healing, doors,
   jerrycans and fuel points, vehicles, radios), use that and drop ours. Check the base game's
   scripts and prefabs first; only where vanilla really does it.
2. **A menu.** Anything with several options or something to read goes in a screen: one of ours
   that's built (the talk screen, the shop, the car repair screen, the crafting and cooking
   screens, the radio screen, the power panel, the reader, the notebook, the notice board) or, for props and places, the generic
   [object menu](#the-object-menu). Pad first: D-pad, A, B to close, readable on a TV.
3. **A quick prompt.** One world action is kept only for what you do in a hurry: ringing a bell,
   sounding a siren, arming a noisemaker, an antidote or a med, forcing a door in a fight.

Every menu keeps its world prompts as the fallback: with its layout slot empty, or when the client
can't open it, the old prompts come back.

### Every interaction, by kind

The audit of 2026-11-12: 9 vanilla, 20 menu, 11 quick.

| Interaction | Kind | How |
|---|---|---|
| A wreck's boot | Vanilla | **Open the boot** is vanilla's open-storage action: the vanilla inventory ([DESIGN.md](DESIGN.md#car-wrecks-one-boot-to-open)) |
| Bodies | Vanilla | Looted with the vanilla inventory, like any body |
| Loose loot in houses, keys, papers lying about | Vanilla | Vanilla's **Pick up** |
| Treating another player | Vanilla | Vanilla's heal; a Nurse or Doctor also slows a bite ([DESIGN.md](DESIGN.md#world-interactions)) |
| A petrol station's pump | Vanilla | With **Cars > Fuel pump station**: a vanilla fuel point where a held jerrycan fills (**Refuel canister**). Empty slot: the hand pump, in the object menu ([DESIGN.md](DESIGN.md#fuel)) |
| Fuel into a car | Vanilla | A running car is fuelled with vanilla's jerrycan; a fixable car's Fuel job drains 5 litres from one and the can is kept |
| A fixed car: engine, lights, lock, driving | Vanilla | Once it first starts it's an ordinary vanilla vehicle |
| Opening and closing doors | Vanilla | Vanilla's door action (refused while the door is locked) |
| A house radio: switching it on and off | Vanilla | Vanilla's **Turn on/off** and its music, on the radio; our **Listen to the radio** is in the same prompt list |
| A workbench | Menu | [The crafting screen](#the-crafting-screen) |
| A fixable car: look, parts, fuel, hotwire, start | Menu | [The car repair screen](#the-car-repair-screen) |
| The power station: the door, the breakers, the generator, the main switch, its upkeep | Menu | [The power panel](#the-power-panel), one prompt (**Open the panel**) |
| A stove: boil water, brew a remedy, cook | Menu | [The cooking screen](#the-cooking-screen); without its layout, [the object menu](#the-object-menu) |
| A generator found in town: start, stop, refuel | Menu | The object menu |
| A house radio: listen, tune, crack a channel, call, view | Menu | The object menu, from **Listen to the radio** on vanilla's radio itself, beside its **Turn on/off** (the call opens the talk screen, **View radio** [the radio screen](#the-radio-screen)) |
| A well (**Interactions > Well spots** on) | Menu | The object menu |
| Puzzle objects: read, use, choose | Menu | The object menu: the read is its text, the uses and choices its rows |
| Keypads and terminals | Menu | The object menu: the twelve keys as rows, what's typed on the Enter row |
| Safehouse services (the Haven quartermaster and the medic) | Menu | [The talk screen](#the-talk-screen): **Talk to the quartermaster** or **Talk to the medic**, the services its replies; without it, the object menu (**Ask the quartermaster**, **Ask about the safehouse**) |
| A quartermaster's stock, requisition and services | Menu | [The shop screen](#the-shop-screen): **Trade**, a reply in the quartermaster's talk (its own prompt without the talk screen) |
| A quartermaster's trial dose (Halcyon) | Menu | The talk screen: **Ask for the trial dose**, greyed with what it needs until it applies; without it, its own prompt |
| Leaders | Menu | The talk screen: talk to them, or **Ask for whoever's in charge** in a quartermaster's talk; without it, **Talk to whoever's in charge** and their asks in the object menu |
| People: wanderers, residents, named people, posts | Menu | The talk screen |
| The notebook item | Menu | A vanilla gadget: taken into the hand (N, a quick slot, the inventory's **Equip**) it opens [the notebook](#the-notebook), as vanilla's map opens in the hand. On the ground, **Read the notebook**, and its name and season actions as one object menu, **Your name and the season** |
| Samples and the lab: deliver, research progress | Menu | The object menu at the lab (a single prompt where there's only one) |
| A radio item: the open channel, a scrambled channel, what's on the air | Menu | **Call on the open channel** opens the talk screen; **Work on a scrambled channel** joins the item's object menu when it has others; **View radio**, held up in hand beside vanilla's Turn on/off or on the ground, opens [the radio screen](#the-radio-screen) |
| Key tags | Menu | [The reader](#subtitles-and-the-reader) (the prompt names the tag too) |
| Notes, letters, flyers and orders | Menu | The reader |
| A safehouse's notice board | Menu | The notice board menu |
| Ringing a church bell | Quick | **Ring the bell** |
| Sounding a siren | Quick | **Sound the siren** |
| A locked door: unlock, lock, force | Quick | On the door leaf; forcing is for a fight |
| A fresh bite: clean it | Quick | **Clean the bite**, in a hurry |
| A doorway to burn | Quick | **Douse the doorway**, can in hand |
| The antidote | Quick | **Inject antidote** |
| Meds | Quick | **Take it** |
| Tagging the leader of a pack | Quick | **Tag it**, close and unnoticed |
| The call box | Quick | **Play the recorded call** |
| The noisemaker | Quick | **Wind the alarm clock** or **Light the fuse** |
| Barricades, traps and a carried generator | Quick | Set from the ground where they lie |

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
| The Long Table | A preserving jar with a cloth tied over it, labelled only "9". The family labels its jars by the day, not by what's in them, and Day 9 was the first soup ([LORE.md](LORE.md#the-long-table)). It isn't a faction and it's a secret, so no screen shows the mark: the Long Table is told by absence ([TRACES.md](TRACES.md#the-long-table-absence)). It's the family's emblem in the set and on the icon sheet only |

**Selected and hovered.** The selected row of a list has a lighter fill, a bar on its left edge
(amber, or the faction colour on a faction's screen) and brighter text. The mouse over a row or a
button gets the same look ([Controllers](#controllers), "Hover lights what the pad would").

**Type.** Headings in a condensed bold, upper case with wide spacing (stencil-like); timers,
distances and channel names in a mono face; body text in a plain sans. In the game the layouts
make do with the vanilla fonts ([LAYOUTS.md](../UI/LAYOUTS.md#conventions)).

## The icon set

88 icons, hand-written SVG on a 64×64 grid with 4-unit strokes, one colour (`currentColor`) on a
transparent background, so the game can tint them. Every icon with its intended use is listed in
[UI/Icons/README.md](../UI/Icons/README.md); the contact sheet is `UI/Previews/icons_sheet.png`.

| Category | Count | Icons |
|---|---|---|
| Faction emblems | 6 | the Haven, Bastion, the Red Hand, Halcyon, the infected, the Long Table |
| Standing tiers | 8 | Stranger, Known, Trusted, Champion (dog tags with 0–3 marks), Watched (an eye), Hunted (a crosshair), Kill on Sight (a crossed-out target), and one mixed mark for Uneasy, Tolerated, Respected and Wildcard |
| Mission types | 8 | contract, world event, puzzle, leader job, unarmed, errand, Perimeter, story arc |
| Map markers | 18 | lab, safehouse, stronghold, outpost, world event, quartermaster and leader, power station, radio station, checkpoint, the range ring, and you-are-here (an arrow, with a disc to sit it on). Each but the ring sits on a plate whose shape says what it is: square for places factions hold, hexagon for the lab, diamond for events, circle for infrastructure, octagon for the sealed checkpoints. These name the places in the map's feed and serve the journal; the map draws a place from the parts: a solid disc (`marker_disc`), a thin ring to go round it (`marker_ring`), and the stations' and checkpoints' glyphs without their plates (`glyph_power`, `glyph_radio`, `glyph_checkpoint`; the set also has an event's, `glyph_event`) ([The map](#the-map)). The tracked job's objective uses no marker of its own: the tracker's filled diamond (`track_current`) and `marker_disc`. The range ring (a broken ring with three claw marks, no plate) is a pack's ground: the map scales it to the ground, and the journal's pack rows use it |
| Map badges | 4 | what a faction's place is, a tiny glyph in a ring: stronghold (a keep), outpost (a flag), safehouse (a house), the lab (a flask). On a dark chip at a map marker's bottom right, and in the map key |
| Items | 18 | key, sample, antidote, stabiliser, low-grade med, noisemaker, battery, tyre, spark plugs, fuel can, wind-up radio, molotov, bandage, tin-can tripwire, spike trap, purification tablets, planks, jewellery |
| Status and HUD | 13 | bitten, golden window, Fever, Weakness, Final, secure radio, open channel, broadcast, noise, shortage, locked, unlocked and forced door |
| Standing and needs | 9 | the Standing page's Favor (a coin), leader met, their kit, the next name and how they see you; four of a faction's needs (food, people, security, revenge; supplies and samples use `status_shortage` and `item_sample`) |
| Voices | 1 | a note read out (a sheet with a turned corner). The other three ways a spoken line reaches you reuse icons above: in person the speech bubble (`standing_word`), a radio channel the open channel (`status_radio_open`, or `status_secure_radio` cracked), a loudspeaker the broadcast (`status_broadcast`) ([Subtitles](#subtitles-and-the-reader)) |
| Objectives | 3 | the current objective (a filled diamond), the next known or an optional one (an open diamond), and done (a tick): the quest tracker and the Quests tab |

**Files.** Sources in `UI/Icons/src/`. White PNGs at 64 and 128 px in `UI/Icons/png/64/` and
`png/128/`. Coloured map and emblem variants (25, with a dark backing behind the plated markers'
cut-out glyphs, and one per faction for strongholds, outposts and quartermasters) in
`png/color/`. Re-render after editing: `node tools/render_ui.js icons` (needs Node and Playwright's
Chromium, nothing else). The game reads the white icons from one atlas,
`UI/Textures/ReforgerZ/RZ_Icons.edds`, through the `RZ_Icons` imageset: after re-rendering, pack
them again with `tools/icon_atlas.py` ([LAYOUTS.md](../UI/LAYOUTS.md#importing-the-icons)). The atlas
PNG and the imageset hold all 88; the imported `.edds` is older until the PNG is imported again (step 2
there). The Standing page's nine, the two you-are-here icons and the ten map parts and badges were
rendered with headless Chrome the way `render_ui.js` does (no Playwright on the author's PC); the
contact sheet doesn't show `voice_note` or the map parts and badges yet.

## Screens

Each screen has an HTML mockup in `UI/Mockups/` over a procedural background (no screenshots),
rendered to `UI/Previews/<name>.png` at 1920×1080 (×2) with `node tools/render_ui.js screens`.

| Screen | Preview | What it shows | Script | Status |
|---|---|---|---|---|
| HUD | `hud.png` | The infection timer: the golden window countdown, then Fever, Weakness and Final on one track, a paused tag under a stabiliser, a glow in the Final stage. Notice cards: a faction-coloured header, the speaker, the text, a bar that runs down; two at once, the most important first ([Notices](#notices)). The secure-radio indicator while a cracked channel lasts. A subtle noise meter, and above it how visible you are: five segments and HIDDEN, LOW, SEEN or IN PLAIN SIGHT (only with [Stealth](DESIGN.md#stealth) on). Under the noise meter during the cure finale's stand, within 150 m of the lab: the stand's line (`StandPanel`, `StandText`: *"CLEAN ROOM: BASTION. THE HAVEN TAKING: 0:35 OF 1:00."*, routes never players; sent by `RZ_PlayerUIComponent.SendStand` every 2 s, gone 5 s after the last, [FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room)). Above the visibility meter while borrowed kit's look is coming: **the look line** (`RZ_Hud_Look`: `LookPanel`, the Watched eye `LookIcon` and `LookText`, *"PASS AS BASTION: 2 OF 2"* or *"…: SEEN THROUGH"*; owner-only through `RZ_UIFeed.PushLook`, sent only when it changes, gone once the look is done, the kit comes off or you're 200 m on; [REPUTATION.md](REPUTATION.md#borrowed-kit)). Top left, **the quest tracker** ([below](#the-quest-tracker), mockup `tracker.png`): only the job you track, its current objective, the known next one and a rough area (within a kilometre an arrow and metres); your other jobs are in the Quests tab only. Nothing to press, so it's the same on a pad (Track is in the notebook's Quests tab) | `RZ_HudDisplay` | Works in play: cards, visibility and noise seen; the timer, the radio indicator, the stand's line, the look line and the tracker not seen yet |
| The notebook | — | One menu with five tabs, the five rows below, in this order ([below](#the-notebook)) | `RZ_NotebookMenu` | Frame and pages built as text, not checked in play |
| Notebook: Standing | `standing.png` (the old four-column page; the page is now design G2) | In the header, the whole picture after your name ([REPUTATION.md](REPUTATION.md#what-people-call-you)). One row per faction, stacked down the page with the faction's colour as a left bar, mostly icons: the emblem and name; **what they call you**, large (the reputation's name, or the tier word with Reputation > Names off; the tier icon beside it, both in the tier's colour); Favor (a coin and the number); the leaders you've met (a bust and how many, or NONE); what they need (its icon and word, or NONE); and a kit tell (a jacket, only for Bastion, the Red Hand and Halcyon with borrowed kit on: dim while they don't know you, yellow once they know your face or caught you, red while you're blown). No Fame or Infamy numbers, bars or grid. Up/down or the mouse only **highlights** a row (a bone wash and border); a click, A or Enter **opens** it (or closes it again). At most one row is open, filling the free space while the others shrink to their strip; with none open (the start) the four share the page. The open row has four blocks: *How they see you* (their word as a sentence, what the name counts as, what's left to forgive while there's Infamy, a line when they hunt you, and **Next**: the next name up with what earns it, "TRUSTED: 12 more Fame" or "KNOWN: work off 8 Infamy", none at the top), *Leaders you've met* (names and titles, then the nearest one's verdict), *What they need* (the need and its line) and *In their kit* (whether they know your face, catches, blown time, their tells, what their enemies do to their colours: the payload's `K` records). The other rows stay one line, so all four names are always on screen. Under the rows, **between the factions**: the truces and feuds on one line | `RZ_StandingPage` | Page built, not checked in play |
| Notebook: Quests | `tracker.png` (right) | Every job you have (only jobs offered to you in person: [MISSIONS.md](MISSIONS.md#who-gets-offered-what)), in three groups: the season's story, questlines and jobs, errands; in each the ones you've taken first, then the ones offered. Each row: the type icon, the name, its rough area, and TRACKED. The selected job's detail: the faction, who gave it and the area on one line, a short brief in the giver's voice, its **objectives** New Vegas style, the reward as far as you know it, and what you've found. **A** tracks it or untracks it ([below](#notebook-quests)) | `RZ_QuestsPage` | Page built as text, not checked in play |
| Notebook: Radio | `radio_log.png` | What you heard and were told this life, newest first: the broadcast, open channels and cracked secure channels, each in its voice, the mission and world notices, the world events (news, never jobs) and the work you heard of with who to ask; six filters (All, Open channel, Secure, Broadcast, Missions, World) | `RZ_RadioPage` | Page built, not checked in play; the channel list is static |
| Notebook: Journal | `journal.png` (the mockup still shows the old job list) | Two groups. **PAST JOBS**: every job you had that ended, newest first (the last 40, kept on your profile): its type icon, name, who gave it and where, DONE, FAILED or ABANDONED in its colour, the season day, and one line of how it ended. **WHAT YOU KNOW**: what you know that isn't a job, as before: the packs you know of, Read on doors, the finale's page, and with Dialogue > Trees a **Promises** row and a **What you know** row ([DIALOGUE.md](DIALOGUE.md#promises-and-callbacks)). Under the list, the lab's line ([below](#notebook-journal)) | `RZ_JournalPage` | Page built as text, not checked in play |
| Notebook: The season | — | One page that scrolls: the four majors as they stand (act, rule or stance, leans, any last call) and each ending told; in the epilogue, *As the Zone tells it* (the cure line and the beats you've met, all of them in processing) and *And you* with what's unfinished ([QUESTLINES.md](QUESTLINES.md#the-seasons-lines-and-the-season-tab), [EPILOGUE.md](EPILOGUE.md#carriers)); then the hall of fame: this season's records and names, all time, and the last seasons in a few lines ([DESIGN.md](DESIGN.md#seasons)) | `RZ_SeasonPage` | Page built as text (one scroll since the hall merged in), not checked in play; no mockup |
| The spawn screen | `background_picker.png` | Our own screen in place of vanilla's deploy menu: the 13 backgrounds as a list, the picked one's name, perk, flaw, starting gear, starting standing, locked or available (with how to unlock), Legacy toward the Drifter, the kind of place you start at (never a picker), and Deploy ([below](#the-spawn-screen)) | `RZ_SpawnMenu`, `RZ_LoadoutLock.c` | Built as text (layouts too); **UI > Spawn row layout** set; seen in Workbench play with the keyboard (pick, deploy, a locked background); pad not yet |
| The death brief | — | At a death, one card over the spawn screen: the name, the background and which life, the day and place, how it ended, how long you lived, and a few of the life's numbers, then one closing line; A, Space, a click or B gives way to the spawn screen ([below](#the-death-brief)) | `RZ_DeathBrief`, `RZ_DeathBriefMenu` | Built as text (layout too), preset added; not checked in play; no mockup |
| Notice board | — | The notes on the safehouse board you're at, newest first (author, how long ago, the note), a box to write one with a Pin it up button, and seven ready-made notes for a pad. Opened when you read the board, with **UI > Board menus** on (the default) | `RZ_NoticeBoardMenu` | Layout built, not checked in play; no mockup |
| Talk | `talk.png` (still shows the old reading bar) | New Vegas style, in two steps: their line on its own in a subtitle band, typed out a box at a time with a bouncing ▼ and "2 / 4", then your replies on their own in an outlined box, with what each needs; their name top right ([below](#the-talk-screen)) | `RZ_TalkMenu` | Layouts built as text, not checked in play; the boxes, typing and paging by moving compile, not played |
| Nametags | — | A small tag over each person within 8 m you can talk to and can see: name, role, who they're with in their colour; three at most ([below](#nametags)) | `RZ_NametagClient` | `NametagPanel` built as text in the HUD layout, compiles, not checked in play |
| Subtitles | `hud.png` (the old one-line name) | What people say outside the talk screen, at the bottom centre: how it reaches you (an icon), the name in their faction colour and their role, for someone in person an arrow to them and how far; the line in bright bone. Radio channels' voices in their own box at the top centre, clear of the quest tracker ([below](#subtitles-and-the-reader)) | `RZ_HudDisplay` | `SubtitlePanel` and `RadioLinePanel` built as text in the HUD layout, not checked in play |
| The reader | `reader.png` | A note, letter, order or flyer on a sheet of paper in the middle of the screen, a page at a time ([below](#subtitles-and-the-reader)) | `RZ_ReaderMenu` | Layout built as text, not checked in play |
| Car repair | `car_repair.png` | A fixable car's job card (each job, what it needs, the button that does it) beside a side view of the car with each job lit where it is, and Start the engine ([below](#the-car-repair-screen)) | `RZ_CarMenu` | Layouts built as text, not checked in play |
| Shop | `shop.png` | A quartermaster's stock in tabs, each item with its Favor price and the standing it needs, a picture of the selected item and a details panel with Buy ([below](#the-shop-screen)) | `RZ_ShopMenu` | Layouts built as text, not checked in play |
| Crafting | `crafting.png` | A workbench's recipes as a list, and for the picked one a picture of what it makes with its description, each input against what you carry (have or missing), how long and how loud, and Craft ([below](#the-crafting-screen)) | `RZ_CraftMenu` | Layouts built as text, preset and **UI > Craft row layout** set; seen in the layout preview, not checked in play |
| Cooking | — (the crafting screen's) | A stove's list (boiled water, the home remedy, the stove recipes), in the crafting screen with the stove's words and a button named for what it does: Boil, Brew, Cook ([below](#the-cooking-screen)) | `RZ_CraftMenu` | Built as text (the crafting screen's layouts); **UI > Cook row layout** not set yet, not checked in play |
| Radio | — | Every channel the radio hears with its latest line, and under them the log of what you heard this life, filtered by the picked channel ([below](#the-radio-screen)) | `RZ_RadioMenu` | Layouts built as text, preset added; **UI > Radio channel layout** not set yet, not checked in play; no mockup |
| Object menu | — | A prop's or a place's name, what you see, and its options as a list, with a countdown while work is under way ([below](#the-object-menu)) | `RZ_ObjectMenu` | Layouts built as text, not checked in play; no mockup |
| Power panel | — | The power station's control: its name and state, what you see, the card inside the door, a board of six breakers to close in the card's order with a READY lamp, the spin-up's countdown or the service gauge, and the step's button ([below](#the-power-panel)) | `RZ_PowerMenu` | Layouts built as text, preset and **UI > Power breaker layout** set, not checked in play; no mockup |
| The map | `map_legend.png` (the old line-icon style; the markers are now design A) | The markers on the vanilla map, per player (a faction-coloured disc with its emblem, a near-black ring, a badge for what the place is), the tracked job's current objective (an amber diamond, or a search area) and no other job, and the map key: a compact legend in the bottom-left, shown zoomed out and fading as you zoom in ([below](#the-map)) | `RZ_MapIconsUI` | Icon layout and **UI > Map legend layout** set; the old icons and legend seen in Workbench play with the keyboard (it fades as you zoom in, I hides it); the design A markers and key not checked in play yet (they need the atlas imported again); pad not yet |

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

**Rough areas in words.** Every job's place in the notebook, and on the tracker past a kilometre, is a rough area
(`RZ_QuestText.RoughArea`): the nearest settlement or the side of it ("Morton", "East of Levie") and
a band, a few km (under 4 km) or far off, with a compass word ("East of Levie, a few km north-west");
under a kilometre the metres and the way instead ("Camurac, 340 m north-west"), never "close by". A contract points at its rough circle until the player
knows the spot ([MISSIONS.md](MISSIONS.md#who-gets-offered-what)). Within a kilometre the tracker
gives an arrow and metres instead ([The quest tracker](#the-quest-tracker)). **Mouse:** click a tab to switch, the close button to
close, a journal row to select it, the filter chip to cycle the radio filter; the wheel scrolls the
lists (they sit in a ScrollLayout). Those widgets are Buttons in the layouts. On a pad, up and
down scroll the same lists.

**Questlines in the Quests tab.** A questline you're part of ([DESIGN.md](DESIGN.md#questlines)) is
a row of its own: the questline's title, its current stage as the "by" line, its kind ("Questline",
or "The season's story" for a major), the stage's text as the brief, "It depends on how it ends." as
the reward, and its notes ("what you know so far") in the clues block. Its stage missions are rows
too, with their own time and pay. Both use the story arc icon (`mission_arc`); there's no
questline icon of its own. A major is a job once you took part in it; until then it's on The season
tab.

### Notices

Notice cards are for what matters now. Everything else goes **quietly into the notebook** (the
Radio tab) without a card, and the line under the cards counts it, "+3 in your notebook", until you
next open the notebook or start a new life.

**At spawn: one card, then quiet.** A new life shows **one card**: who you are and where, in a line
or two (the background's name, then where you wake: "A farmhouse cellar in Morton. Day 11. The last
bottle is empty, so you open the door.", from the opening, or "You come to east of Levie."; a locked background adds
"Soldier is locked. You're Survivor this life."). The rest goes into the notebook, no card each:

| What | Where it goes |
|---|---|
| Your perk, flaw and goal | The Radio tab, under the background's name |
| The scene where you wake, and its clue if none could be placed | The Radio tab, under the scene's title |
| The outbreak and the lab, the government broadcast | The Radio tab, World |
| The season: its rules, the session's news (*Word on the roads*), the hall of fame so far | **One** entry, *This season* |
| What's on the radio | One entry, *Radio*: the world events (news) and the work going, each with who to ask; the jobs offered to you are on the Quests tab |
| Pay owed while you were away (errands) | **One** entry, *While you were away*, all of it |
| The epilogue's news | One entry, *Word on the roads* |
| Anything else in the **quiet start** (the leaders' greets, standing, the Perimeter, disguise, traces, a mission offer) | The Radio tab (World), no card |
| Tips | Not before **Guidance > Tips wait after spawn** (90 s), then only from context (below) |

**The quiet start** (**UI > Quiet start**, 30 s): after a new life only the spawn card and urgent
notices are cards; every other notice is logged in the notebook and counted under the cards
(`RZ_GameModeComponent.QueueHint`). **The first minutes** (**UI > First minutes**, 300 s): cards
that aren't urgent come at most one per **First minutes gap** (45 s). After that, at most **UI >
Notice cards at once** (2) on screen, a new one at most every **Gap** (2 s). Waiting ones queue most
important first, then oldest first. Each card comes in with vanilla's own hint sound
(`SCR_SoundEvent.HINT`), the one the vanilla hint it replaces plays. A radio channel's voice skips the cards: it goes to the radio-line box at the top centre
([Subtitles](#subtitles-and-the-reader)), and the Radio tab keeps it as before.

| Priority | Means | Examples |
|---|---|---|
| Urgent | danger to you, now | Bitten, Fever, Weakness, The end of it, Horde, THE FINAL STAND, the clean room changing hands and overtime ([FINALE.md](FINALE.md)) |
| High | about you, now | the spawn card, your standing changed, a background unlocked, every mission notice (offered, "this is the place", done, failed, refused), the finale's lock card ("[Point of no return]") and "Written down", the batch and the seal calls |
| Normal | radio and faction voices | open and secure channels, the broadcast, faction notices, world events, anything with an unusual title |
| Low | news | guidance reminders for veterans; the finale's stand line within 150 m of the lab ("CLEAN ROOM: Bastion. The Haven taking: 0:35 of 0:60.") |
| Quiet | the notebook only, never a card | the welcome's world news, the season summary, pay owed while you were away (`RZ_ENoticeTag.QUIET`); work you heard of, with who to ask (`RZ_ENoticeTag.HEARD`, listed under Missions; it doesn't count on the Quests tab) |

- **Urgent** cards skip every gap and the quiet start, and when the stack is full the least
  important card on screen is cut short to make room.
- **Stale ones are dropped.** A notice that isn't urgent and waits more than **Wait** (45 s) gets no
  card: one the Radio tab lists is there already (counted under the cards), the rest are dropped.
- At most **Notices waiting** (12) wait; past that the least important one the Radio tab lists goes
  to the notebook only (or, if none is listed, the least important is dropped).
- There's no separate notification log: the notebook's Radio tab keeps the last **Radio tab
  entries** (60) radio, mission, world and quiet notices of this life. Filters (X or F, or click the
  chip): **All, Open channel, Secure, Broadcast, Missions, World.** Missions: work you heard of
  and who to ask, offers made to you, "this is the place", done, failed, refused. World: world events, the quiet entries and what came in the quiet
  start.

**Tips from context.** The guidance tips ([DESIGN.md](DESIGN.md#new-player-guidance)) stay, one at a
time: each fires when its moment comes (you're bleeding, a quartermaster is close, it's getting
dark, you're too loud, you're near the line), at most one every **Tip gap** (90 s), and none that
isn't critical in a life's first **Tips wait after spawn** (90 s).

How a notice's importance is known: the server tags about twenty call sites
(`NotifyPlayerAs`/`NotifyAllAs` with `RZ_ENoticeTag`: MISSION, EVENT, WORLD, URGENT, HIGH, LOW, QUIET, SPAWN, HEARD;
mission messages through `RZ_Mission.NotifyParticipants` and `NotifyInterested`); every other
notice is classified by its title on the client (`RZ_NoticeStyle.Classify`). Without the HUD the
vanilla hint queue sorts the same way, with no expiry (it has nowhere else to go).
*Simplified:* an odd title gets Normal; nothing is lost, it only waits.

**Notice lanes** sit on the server, above this pacing: they decide whether something that speaks to
a player unasked (a leader catching your eye, the Long Table's host on the road, wanderers and
errands) may speak at all, at most one such contact every 4 minutes. Urgent notices
and anything you asked for are never held. The rules are in [DESIGN.md](DESIGN.md#notice-lanes).

### The quest tracker

Small, in the top-left corner of the HUD (`TrackerPanel`, 360 units wide at (24, 24), moved down
under the secure-radio panel while that shows), for the one job you follow. The house mockup is
`UI/Mockups/tracker.html` (left; preview `UI/Previews/tracker.png`):

- **The tracked job and nothing else:** its type icon (from the imageset, tinted the giver's
  colour) and its name; its **current objective** after a filled diamond (`track_current`); the
  **known next objective**, dim, after an open diamond (`track_next`), when there is one; and the
  **area line**, on a line of its own in amber (`TrackerAreaRow`), as below. No box: the text sits
  on the world with a dark shadow.
- **The area line.** Far off, the rough area in words ("Morton, a few km north-west",
  `RZ_QuestText.RoughArea`). **Within a kilometre** of a spot you know: an amber arrow
  (`TrackerArrow`, the `marker_self` icon, 20 across) turned toward it from where your character
  faces, and the metres ("Camurac, 120 m": to 10 m from 100 m up, to 5 m below, to 1 m under 10,
  `RZ_QuestText.Metres`), counting down to 0 m; the arrow goes in the last 2 m, where it would only
  spin. A **search area** (you know only the rough circle): within a
  kilometre of its edge, the arrow to its centre and the metres to its edge ("Search area, 240 m"),
  then *In the search area* once you're inside it, no arrow; the centre is off the real spot, so
  the arrow never gives the spot away. Dead, or a job with no place: the words alone, no arrow.
  The arrow turns every frame and the text is set only when it changes, client-side from the
  position the tracker's record carries (as the subtitles' arrow, [below](#subtitles-and-the-reader)).
- **The arrival radius** is the job's own: an errand's step its way's radius (25 m unless the way
  says), 15 m back with the giver; a mission the radius it counts you as there at
  (`m_fParticipationRadius`), at most 50 m so the arrow still leads into a big site, 25 m when it
  has none; a questline's place 25 m.
- **Your other jobs are not on the HUD.** They're in the notebook's Quests tab only.
- **Hidden** while nothing is tracked. Owner-only: sent to your machine alone, only when it changes,
  checked every 2 s on the UI feed's tick (`RZ_QuestTracker`, payload kind 13: `T` and `O` records;
  `T` carries the area's words, the place alone, where it points, the arrival radius and the search
  area's radius); the HUD redraws it only when a new copy arrives, and only the area line between.

**What's tracked.** Nothing, until you pick a job in the Quests tab or take one. Taking a job (being
counted on it: a job offered to you that you reached, a questline you took part in, an errand you
agreed to) tracks the newest one if nothing is tracked. When the tracked job ends, the newest one still
taken is tracked. Picking the tracked job again stops tracking it, and then nothing is tracked
until you take a new job. The choice is saved on your profile (`m_iTrackedId`, with the title to
find it again after a restart), so it survives a relog. A job you haven't taken is tracked only if
you picked it yourself; a major the whole Zone shares is a job only once you took part. An opening's
chain ([DESIGN.md](DESIGN.md#openings-how-every-life-starts)) is tracked by itself as each step
starts (`RZ_QuestTracker.Follow`), whatever was tracked before.

**On the map** the tracked job's current objective is the only job shown, to you alone: a diamond
on a spot you know, a search area while you know only the rough one ([The map](#the-map)).

**Objectives** come from the job ([MISSIONS.md](MISSIONS.md#objectives)): only what you've learned,
no placeholders, nothing hidden.

### Notebook: Quests

The one place to choose what the tracker follows, and the one place your other jobs are listed (the
house mockup, `UI/Mockups/tracker.html`, right). Every job you have, in three groups, each under its
heading (`RowGroup`). A job is yours only once a person or a thing offered it to you
([MISSIONS.md](MISSIONS.md#who-gets-offered-what)): work you only heard of is on the Radio tab with
who to ask, world events are news there and never here, and a job that ended moves to the Journal.
Empty, the tab says *"No jobs yet. Ask a leader or a quartermaster for work, or use a safehouse
radio. The radio only says who's asking."*

| Group | Jobs |
|---|---|
| THE SEASON'S STORY | The majors you took part in ([QUESTLINES.md](QUESTLINES.md)) |
| QUESTLINES AND JOBS | Questlines you're part of, missions, contracts, events, leaders' jobs |
| ERRANDS | The errands you hold ([ERRANDS.md](ERRANDS.md)) |

In each group the ones you've taken come first, then the ones offered. Each row: the type icon, the
name, the rough area under it (a major's act first: "Act II · Brod, a few km east"), TRACKED on the
tracked one, and the time left or *Offered* under that. The D-pad (or a click) chooses a job; its
detail:

| Block | What it says |
|---|---|
| The heading | The name, then one line: the faction, who gave it and the area ("The Haven · from Elder Mara · Morton, a few km north-west"; a questline's stage stands for "from") |
| The brief | A few lines in the giver's voice: what it is and why it matters (the job's own brief, cut to about three sentences at a sentence's end; `RZ_QuestText.Short`). Only what you were told |
| Objectives | New Vegas style, up to six: done ones struck through and faint with a tick; the current one bright after a filled diamond; the known next ones dim after an open diamond; an optional one as "(Optional) ..."; a failed one as "Failed: ...", red |
| Reward, as far as you know | About so much Fame and Favor with the giver, "It depends on how it ends.", or "No pay. It shows you the way, that's all." for a job that pays nothing (Where the Work Is) |
| Requirements | Only on a job that's offered and asks something of you |
| What you know | What you've found, who it went to, the clues you've read (hidden when empty) |

**Track.** **A** on a pad (Enter on a keyboard) tracks the chosen job; on the tracked one it stops
tracking it. Y and T do the same, and so does the TRACK button with the mouse (UNTRACK on the
tracked job). The server answers with the tracker and a fresh list
(`RZ_PlayerUIComponent.AskTrack`, at most three a second). The footer says so: *"D-pad: choose · A:
track / untrack · LB/RB: tabs · B: close"*, as vanilla's input buttons (the key or the pad button by
the device in use), or as that text where they can't be made.

### Notebook: Journal

What's behind you and what you know, in two groups under their headings (`RowGroup`), with the same
row and detail as the Quests tab:

| Group | Rows |
|---|---|
| PAST JOBS | Every job you had that ended, newest first: contracts and missions you were offered and took part in (or that a leader or a parley offered you alone), questlines you were part of, errands you held (the openings' steps among them). Never a world event (news, not a job) or work you only heard of |
| WHAT YOU KNOW | The packs you know of, Read on doors, the finale's page, Promises and What you know (below) |

A past job's row: its type icon tinted the giver's colour, its name, *"from Sergeant Hale · Morton"*,
its result on the right in its colour (DONE green, FAILED red, ABANDONED dim) and the season day
under it (*"Day 4"*; *"Season 2, day 9"* for an older season). Its detail: the same line with the
day, how it ended as the brief (a contract's deed, *"Brought medicine to Morton."*; its fail or
called-off line; *"You walked away from it."*; an errand's giver's thanks, or *"Mara stopped
waiting."*), the place, and RESULT where a job's REWARDS go. Nothing to press but choose: a past job
asks the server for no detail.

**How it's kept** (`Missions/RZ_JobHistory.c`): per player on the server, on the profile (saved with
it), the newest **40**, across lives. A mission adds its row when it ends (`RZ_MissionManager.Finish`:
done, failed, or abandoned when you stepped out, it was called off, or someone else saw to a job
offered to you alone), a questline at its end (`RZ_Questlines.EndRun`: an ending is done, going quiet
abandoned, for everyone who took part, online or not), an errand when a held one ends
(`RZ_Errands.End`). The rows go to the owner as their own payload (`RZ_UIFeed.PAYLOAD_HISTORY`, kind
22, the journal's `M` records with the result word, the day and the result), asked when the Journal
opens; the page asks the journal's payload too, for what you know. Under the list, the lab's line:
research, what waits in its backlog and how long that takes ("Research 63 percent. Waiting: 14
samples' worth, about 4 days of work.", [DESIGN.md](DESIGN.md#cure-research-and-the-seasons-end)).

### The notebook

One menu with five tabs, in this order: **Standing, Quests, Radio, Journal, The season.** The map
key isn't a tab: it's on the map ([The map](#the-map)). The hall of fame is part of The season.

| Open it | Keyboard and mouse | Gamepad / console |
|---|---|---|
| A key | **N** (`RZ_OpenNotebook`): the notebook item comes into your hand, then the notebook opens; without the item, it just opens | **View + RB**, the same |
| The pause menu | Esc, then **Notebook** | Start, then **Notebook**, A |
| The notebook item | N takes it into your hand; or its quick slot; or **Equip** in the inventory | View + RB; or its quick slot on the quick slot wheel; or **Equip** in the inventory |

- It opens on the tab you last used this session (the first time: Quests).
- LB/RB or Q/E switch tabs; clicking a tab works too. Up/down (D-pad, stick, arrows) move in the
  journal's and the Quests tab's lists, highlight the next faction's row on the Standing page,
  and scroll the other tabs' lists; A or Enter selects the journal row with the focus, on the
  Quests tab tracks or untracks the job with the focus (Y or T too), and on the Standing page opens
  the highlighted row or closes it; X or F (or a click on the chip) cycles the Radio filter; B or
  Esc closes.
- The footer shows the open tab's prompts (`RZ_NotebookPage.Prompts`, made again on each tab
  switch): Standing choose and open, Quests choose and track / untrack, Radio filter, Journal
  choose, and on every tab tabs and close.
- On the Standing page the mouse highlights the row it's over and a click opens it (a click on the
  open row closes it). Rows open only on a click, A or Enter; one at most, and with none open the
  four share the page. The highlighted and open rows are kept for the session. Hover only does
  what up/down does, so nothing needs it.
- A tab with something new since you last looked carries a count: Quests the mission notices (not
  the work you only heard of), Radio everything it lists. Opening the notebook also clears the "+N in your notebook" line.
- Opened from the item, the item is put away again when the notebook closes.
- You can't move while it's open (it's a menu, like vanilla's). The world keeps running.

**How it works.** `RZ_NotebookMenu` holds the shared frame (heading, sub-line, tabs with their
counts, close button, footer prompts) and an empty `Page` frame. Each tab is a page
(`RZ_NotebookPage`: `RZ_StandingPage`, `RZ_QuestsPage`, `RZ_RadioPage`, `RZ_JournalPage`,
`RZ_SeasonPage`) with its own layout (**UI > Standing page** and the rest); switching tabs removes
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

**The notebook item** (`Prefabs/ReforgerZ/UI/RZ_Notebook.et`, `RZ_NotebookItem.c`): a small
notebook every new life is given a few seconds after spawn (**UI > Notebook prefab**; one that
doesn't fit lies at your feet). It's a vanilla gadget, as the map and the compass are: vanilla's
`Item_Base` with the notebook's mesh, a gadget component (`RZ_NotebookGadgetComponent`) held in the
left hand like the MX991 flashlight (**Gadget anim variable** MX991 and the MX991's hold), and
the gadget slot type, so it goes in a pack or vest like the map.

| Way in | What happens |
|---|---|
| N / View + RB | `RZ_NotebookItem.OpenByKey`: as vanilla's M with the map, the item comes into the hand through the gadget manager and the notebook opens there. With no item, in a seat (it isn't held in vehicles) or if the character can't take it, the notebook just opens; if the item isn't in the hand 1.5 s later, the notebook opens anyway (and the log says so) |
| Its quick slot | A notebook new to you goes on the last free quick slot that isn't a weapon slot (once per item: take it off and it stays off), so a pad reaches it on the quick slot wheel. Using the slot is vanilla's: the gadget comes into the hand |
| **Equip** in the inventory | Vanilla's: the gadget comes into the hand; the inventory closes and the notebook opens |
| **Read the notebook** | On the item: on the ground, and in your hand while you hold it up and look at it (vanilla's inspection). Opens the notebook on your machine, the fallback if the hand doesn't work |

In the hand, the notebook opens 0.3 s later (the hold shows first, as vanilla's map waits), from
the gadget's own `IN_HAND` mode switch as vanilla's map does, and from the HUD's 0.2 s watch
(`RZ_NotebookItem.CheckHeld`, which asks the gadget manager, the character's left hand and the
item's own mode) in case the switch doesn't run on your machine; once each time it comes into the
hand. When the notebook closes, the item is put away (only the notebook: a compass in the hand
stays). Another gadget in the hand is put away first, as vanilla's quick slot does. *Why it
didn't open before:* the item was a destructible prop with no hold animation set, so the
character never finished taking it into the hand and the gadget never reached `IN_HAND`.
*Simplified:* it's given at spawn by script rather than added to each of the 13 loadouts, so it
works with any loadout.

**Errands and the detail.** A held errand is a Quests row of its own (kind ERRAND, the
`mission_errand` icon: a small parcel tied with string): the giver as "by", where, the direction (to the
step's place, then back to the giver), the real minutes before it lapses, and what you said you'd
do. On the map only while tracked ([The map](#the-map)). Selecting any row asks the server for that row's **detail**
(`RZ_PlayerUIComponent.AskDetail`, payload kind 10, `D|id|told|found|went`, at most 1,500
characters, a few a second at most; the server answers only for a row that player's own journal
shows, nothing for any other id): its objectives (`O|id|state|line`, first, so the cap never cuts
them), and its "What you've found" and "Who it went to" at the top of the clues block
(`DetailClues`), so the journal's 6,000-character list is never cut for them.

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

**The finale page.** While the cure's routes are open ([FINALE.md](FINALE.md#the-gate)), the journal
has one row of kind FINALE (the lab icon, `marker_lab`; its id `-400000`): the title "The finale",
"How things stand" where a job's time left goes, the lab's readout as its brief; its detail is the
page a desk's *How do things stand?* gives (`RZ_CureFinale.PageFor`: the readout by faction, the
route's worth, your route, its pact and things, or what you'd bring, and the seal). Lock counts are
never shown.

**The jobs' order.** The server sends the jobs in each player's own order
(`RZ_MissionReasons.OrderScore`, [DESIGN.md](DESIGN.md#story-blocks)): near first, then started by a
reason, then from a leader they helped; the rows after them keep their place.

**Door marks as decals.** With **World > Traces > Door drawing** on *Decals* and the kit's atlas set,
the HUD's update (`RZ_HudDisplay`) also draws the marked doors near you as decals on their walls
(`RZ_DoorMarkClient`, every 2 s, within 70 m), from a short list the game mode replicates. Nothing to
press: a door's words come by standing at it ([TRACES.md](TRACES.md#doors-you-can-see)).

The notice board opens when you read a safehouse board, with **UI > Board menus** on (the default).

### The talk screen

New Vegas style, in **two steps** (the mockup is `talk.html`): the world isn't dimmed, the person
stays in view, and their name (bold 40) and sub-line, "The Haven - council head - knows your name"
(24, bone), sit at the top right in their faction colour, with a shadow, from start to end. Their name
is shown **only there**: never in front of their line. Their line shows without quotation marks
(the screen takes a leading and trailing pair of straight or curly quotes off each line, and the
speaker's own "Name: " in front, `RZ_TalkMenu.Spoken`; someone else's name in front of their line
stays). When they have nothing to say first ([DIALOGUE.md](DIALOGUE.md#no-filler-greeting)) the
screen **opens straight on step 2**, your replies, with no line.

1. **Their line on its own.** When they speak, only their line shows, in a subtitle band in the
   lower middle (1,320 wide, #121416 at 85%, a 5 px bar down its left in the faction colour; the
   line regular 30, bright bone), **a box at a time**: about three rows of 80 characters a box.
   Short lines (each of their lines starts a row) share a box while they fit; a long one is cut
   after the last sentence that fits past the box's middle, else at a space. The box **types out**
   at 50 characters a second; **A** (or Enter, Space, a click) finishes it at once. Once it's all
   there a small **▼** (the `marker_self` icon turned down, in the faction colour) bounces gently
   at the band's bottom right, with **"2 / 4"** beside it when what they're saying takes more than
   one box (this box of the speech, in the faction colour). The same press goes on: the next box,
   or after the last, the replies. **Nothing moves on by itself.** Under the band, the footer reads
   **"A Skip"** while it types and **"A Continue"** once the ▼ shows, with **"B Leave"** (vanilla's
   prompt buttons, the key or the pad button by the device in use; without them, a static line
   says the same: "A: continue · B: leave", or "Enter, Space or click: continue · Esc: leave" on
   keys). A box that still overflows (a very long unbroken line) scrolls with the right stick.
2. **Then your replies on their own.** The band becomes the reply box (1,440 wide, the same panel
   with a 2 px outline in the faction colour), holding only your choices (regular 28), **at most
   five**: the selected one with a fill and an outline in the faction colour; a reply's tag before
   it in the faction colour, in brackets ("[a job]", "[news]", "[trade]", "[Speech - Bastion 2/3]");
   a reply you can't take faint, its tag too, with what it needs after it ("(you have 1)"); a reply
   with nothing new behind it **grey** (it still works). Every reply is what you say, word for word
   (*Show me what you've got.*, not "Trade": [WRITING.md](WRITING.md#talk)), without quotation marks.
   The last is always the way out, said to this person (*I'll let you get on.*, *That's all for
   now.*, *I'll be going.*). Under the box, the footer: "D-pad: choose · A: say it · LB/RB: page · B: leave" (the
   pad's or the keys' glyphs, by the device in use), with "1-5: a reply · Q/E: page · Esc: leave"
   on its right.
3. **Their answer.** The reply you say fades (the others go at once) and their answer takes the
   band: step 1 again. If no answer comes in 2 s (the server didn't take the pick), the replies
   come back.

The list is built on the server ([DIALOGUE.md](DIALOGUE.md#the-talk-screen)): a longer list comes a
page at a time, four replies and the way out. **The page indicator**: an up arrow at the box's top
left while there's a page before, a down arrow at its bottom left while there's one after (the
static `marker_self` icon in the faction colour; the down one is turned 180°), and "1/2" bottom right
(bold 22, the faction colour); with one page they're hidden. **Moving past the end turns the page:**
down on the last reply goes to the next page with its first reply selected, up on the first to the
page before with its last selected (the D-pad, the stick, the arrow keys or the wheel; it doesn't go
round, and moves wait up to 1 s for the page to come). LB/RB (Q/E) also turn the page and go round
from either end; a click on an arrow turns it too. A page turn stays on the replies. A new list (a
node, back to the hub) starts on its first page. A server admin's replies sit under one **[Admin]**
reply that opens them as their own list, with **Back.** as its way out (B and Esc go back too); it
stays on the replies.

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Their line: finish the typing, then go on (the next box, then the replies) | Enter, Space, or a click (`RZ_TalkSkip`) | A |
| Choose | up/down, the wheel, or **1–5** (`RZ_Reply1`–`5`) to say one at once; the cursor on a reply lights it | D-pad or left stick, or LT/RT + D-pad to say one at once ([Controllers](#controllers)) |
| Say it | Enter, or click | A |
| Leave (Back. in the [Admin] list), from either step | Esc | B |
| Turn the page ("1/2", the arrows) | down past the last reply, up past the first; Q/E, or click an arrow | the same with the D-pad; LB/RB |
| Read the rest of a box that overflows | the wheel | right stick (`RZ_TalkScroll`) |

The keys that choose, say and turn pages work only on the replies; B and Esc work on both steps. A
press of A sends both `MenuSelect` and `RZ_TalkSkip`; the screen takes one step a press (0.15 s
apart), and the press that ends their line never also says the reply that comes up (0.3 s).

**It opens only when you talk.** The talk screen opens only from your own interaction: a person's
**Talk to ...**, a quartermaster's **Ask for whoever's in charge**, or a radio's **Call on the open
channel**. Nobody opens it on you by looking at them or walking up: someone who wants you calls out
as a [subtitle](#subtitles-and-the-reader) with their name (a stranger, a road post, an opening's
contact; [DIALOGUE.md](DIALOGUE.md#who-opens-a-conversation)), their nametag shows as you come near,
and where it's a step of a job the [tracker](#the-quest-tracker) and the map's diamond point at them
("Talk to Vera Holm.").

**One Talk prompt per person.** Everyone who talks has exactly one prompt, **Talk to ...**, and
everything else they offer is a reply on their hub (vanilla's own actions on them stay). For the
people with the puzzle components that's their read; for a quartermaster and a safehouse's medic,
who have none, it's **Talk to the quartermaster** / **Talk to the medic** (`RZ_TalkAction`), and
their old prompts are replies, said as speech (*Show me what you've got.*, *Who's in charge here?
I'd like a word.*): **Trade** opens [the shop screen](#the-shop-screen), **Ask for
whoever's in charge** hands the conversation to the seat's leader, **Ask for the trial dose** on
Halcyon's shows only greyed with what it needs until it applies, and the Haven's house services
(hand over food, get patched up, tell them you're bitten, the radio, the board...) are rows, greyed
with what they need where it's plain (no food on you, no bite) ([DIALOGUE.md](DIALOGUE.md#the-talk-screen)). The quartermasters' old
"Check standing and work" is gone: your standing and your work are in [the notebook](#the-notebook).
Without the talk screen (its layout slot empty, or it can't open on a client) the old prompts come
back, as before.

**With Dialogue > Trees on** ([DIALOGUE.md](DIALOGUE.md#trees-hubs-topics-and-nodes)) the same
screen and payload carry the trees, with no new widgets: the person's built replies, then their
topics (a "heard" tag on one you've opened that still has more in it, grey once there's nothing
new); in a node, its choices with the tag the check or price writes ("[Persuade - Haven - 1 of 2]",
"[Fear 1 of 2 - +3 Infamy]", "[2 food]", "[Doctor]"), faint with what it needs when you can't take
one ("you have 1 food", "you've tried that"), grey when you've seen where it leads. What they say
after a check opens with the result ("[Persuaded]"). Tags are cut at 40 characters.

How it works is in [DIALOGUE.md](DIALOGUE.md#the-talk-screen). The screen reads `PAYLOAD_TALK`:
`T|serial|faction|name|sub|said|page|pages` (line breaks as `^`, the page from 0), then
`R|id|label|tag|need|enabled|heard` per reply (five at most); `X|reason` closes it. The line and the replies come
together; the two steps, the boxes and the typing are the client's (`RZ_TalkMenu`): a node after a reply you said, or with a
new line, starts at step 1, and one with the same line (a page turn, the [Admin] list) stays on the
replies (a page asked for by moving selects its first or last reply). A pick goes back as `RpcAsk_TalkPick(serial, id)`, and a page turn as the same call with `RZ_Dialogue.MORE` or
`PAGE_BACK` (the server takes it only while there's more than one page); Esc, B or another menu on
top send Leave, which the server takes whatever the serial (B pressed while an answer is on its
way still leaves). If the menu can't open on a client, that client shows the line as a card,
leaves, and shows the person's actions again (`RZ_TalkMenu.Available`).

### Nametags

Walk up to someone you can talk to and a small tag shows over their head: their **name** (condensed
bold 24, bone), their **role** under it (20, dim: "Council head", "Toll man", "Trader"), and **who
they're with**, the faction's name in its colour ("The Haven"); no line for someone with none. Only
people with a Talk prompt (never the infected, never players), only within **Dialogue > Nametag
range** (8 m) and in sight, **three at most** (the nearest, **Nametag max**), and none while a menu
is open or you're dead. Names are what the talk screen calls them: a wanderer's own name ("Old
Marta"), "Resident" for a house's people, "Stranger", a quartermaster's or the medic's role
("Quartermaster", "Medic"), a mission's people by their Talk prompt ("The captain"); a house's
people, strangers, the Long Table, posts and a mission's people show no role. Vanilla's nametags
(players only) stay as they are. Nothing to press, so it's the same on a pad.

**Cheap by design.** The server works out who's near each player every 1.5 s (one sphere query of
range + 4 m round them, like barks) and sends that player only the nearest four, each as its body's
RplId, name, role and faction (`PAYLOAD_NAMETAGS`, kind 20: `N|id|name|role|faction` per person, ""
for none), and only when the list changes; who someone is comes from the talk screen's adapters,
worked out once a minute per body (`RZ_Nametags`). The client (`RZ_NametagClient`, from the HUD's
update) picks who shows four times a second (the distance and one sight trace each, at most four)
and every frame moves at most three tags (one projection each; the text is set only when a tag's
person changes). The widgets are `NametagPanel` with `Nametag0`–`2` in the HUD layout
([UI/LAYOUTS.md](../UI/LAYOUTS.md#hud-rz_hudlayout)); without them, nothing shows.

### Subtitles and the reader

**Subtitles.** What people say *outside* the talk screen shows at the **bottom centre**: a solid
dark backing (12, 14, 15 at 88%), a centred head row, then the line in bright bone (32), centred,
1,100 units wide, its bottom 204 above the screen's (clear of the infection panel and the noise
meter). The head row says who and from where:

| Part | What it shows |
|---|---|
| Source icon (28) | how the line reaches you, in the speaker's colour: a speech bubble in person (`standing_word`), a loudspeaker (`status_broadcast`: Bastion's loudspeakers, the Perimeter line, a speaker on a post, a loudhailer), a paper sheet (`voice_note`: a sign, a board or a note read out). A radio's voice has the radio (`status_radio_open`) when it can't go to its own box |
| Name (bold 24) | in their faction colour, grey (163, 158, 144) when they have none: "Marta Weber" |
| Role (22, dim) | after a dot, what they are, with whose: "· Haven council head in Morton". The faction's short name goes in front unless the name or role already has it ("· Bastion checkpoint"); cut at 48 characters; hidden when there's none |
| Arrow and distance (24, 22 bold, bone) | **in person only**: the you-are-here arrow (`marker_self`) turned to the speaker from where you face, and "12 m" (1.4 km past a kilometre), redrawn every frame while the line shows, from their body while it streams in, else from where they stood. Hidden for loudspeakers, notes and radio, and when the server had no place for them |

The name and role come from the title the line is said under ("Marta Weber, cook at Morton" is
Marta Weber, then the rest). How it reaches you is worked out from that title and the line
(`RZ_SayLine.SourceFor`): a title naming a loudspeaker or loudhailer, or a line opening "A
loudhailer" or "A speaker on a post", is a loudspeaker; a line opening "A sign", "A new sign",
"The board", "The roster board", "Under the toll sign" or "A note" is a note; a radio channel's
title ("Radio: Bastion net", "A radio") is the radio; anything else is in person. The server sends
the name, role, faction, source, the channel, where the speaker stood and their body's `RplId`
with each line (`RZ_PlayerUIComponent.SendSayLine`, `RpcDo_SayLine`). The body is the caller's
(a bark's speaker) or, when the caller has none, the nearest living person within 15 m of the
player who isn't a player or one of the infected, people you can talk to first
(`RZ_SpeakerFinder`, one small sphere query per line). *Simplified:* that guess can pick the
wrong person in a crowd, and a line about a door or a voice behind one points at whoever stands
nearest; a speaker further than 15 m gets no arrow. The old `RpcDo_Say` (title, faction, line)
still works: the client works the rest out itself, with no place to point.

**Radio lines.** A radio channel's voice never shows where a person in front of you would: it
goes to its own box at the **top centre** (`RadioLinePanel`, 560 wide at (680, 24)), clear of the
quest tracker in the top-left corner and the notice cards on the right. A dark panel
with a left bar in the channel's colour; the radio (`status_radio_open`, `status_secure_radio` for
a cracked channel) and the channel's name in dim italics ("Bastion net", 20); the voice's name when
there is one (bold 22, in its colour); the line (26, bone, wraps). Radio voices are the notices
titled as a channel: "Radio: …" (the factions' open channels, the reel, the station), "Cracked:
…", "Wind-up radio: …", "Radio chatter" and "The open channel", and a spoken line from a radio
("A radio" in a house). They are still kept in the notebook's Radio tab; a plain "Radio" or "Radio
station" notice (how the set works, the console) stays a card, and so does the government
broadcast. Paced like the subtitles; a long one is cut at about three rows (130 characters); at
most six wait.

The notice cards stay top right; the three places never mix. One line at a time in each place,
on screen for **Subtitle shortest** to **longest** (4–12 s) by length at **reading speed** (11
characters a second, slow enough to read while moving; raised from 2.5–8 s at 18 after play-tests
found the lines gone too soon); a subtitle longer than two rows (130 characters) is split at sentence ends
into parts shown in turn. At most four subtitles wait; a newer line from the same speaker
replaces theirs still waiting. While the reader or the talk screen is open, neither shows: the line on screen goes at once and the rest wait until it closes, so a subtitle never sits under a conversation. Nothing to press, so nothing to
bind. A spoken line goes only to its player (`RZ_GameModeComponent.Say`, `SayNear`). With **UI >
Subtitles** off, without the panel in the HUD layout, or without the UI component, it's a notice
card, as before (a radio voice without `RadioLinePanel` too).

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
| The screen guard | "Not now. Something's coming." when danger closes a talk |
| Dialogue barks | a person you pass who has a line for you ([DIALOGUE.md](DIALOGUE.md)), with their body for the arrow |
| Traces | the town loudspeaker (a loudspeaker), the radio in an empty house (the radio-line box) |

Everything else stays a card, except radio voices, which go to the radio-line box. While the talk
screen is open, the person's own lines go into it instead, and so does whatever a reply tells you,
but the journal's notices are cards too and danger, radio and tips stay out of it
([DIALOGUE.md](DIALOGUE.md#the-talk-screen)).

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

### The crafting screen

As in `crafting.html`, in the shop's style: **Use the workbench** at a workbench
([DESIGN.md](DESIGN.md#light-crafting)) opens a panel 1,600 × 858 in the middle of the screen, the
world dimmed. The header is "Workbench", the town (when the bench is in one), what the bench looks
like, and on the right "You can make 3 of 8". On the left, under RECIPES, the recipes as a list: each with its item icon (where the icon
set has one), its name, READY in olive or "2 MISSING" (how many of its inputs you're short of)
dim, and how long it takes in amber ("8 s"). A recipe you're short for is dim; one whose output slot
is empty isn't listed. On the right, for the picked recipe: a picture of what it makes (vanilla's
item preview of the output prefab, as the shop draws its items, labelled WHAT YOU MAKE), its name,
its own description and weight, then IT TAKES: one row per input, its name ("Empty tin", "Wire or
string"), "1 of 2" and HAVE in olive or MISSING in red. Under them how long it takes ("About 8
seconds at the bench") and how loud ("Some hammering. It carries a little.": the heat it adds), and
**Craft**, amber when it can go now, dark while you're short or at work. While the work is under way
the row says WORKING, the time reads "Working... 6 s" over a thin amber bar. Under both, what the
last pick said ("You clear a space and start on the Tin-can tripwire.", "Done: ...").

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Pick a recipe | up/down, or click it | D-pad or left stick |
| Craft | Enter, or the Craft button | A |
| Close | Esc, or the close button | B |

The text is TV-sized: recipe names 26, inputs 24, the smallest labels 18. You can't move while it's
open; the world keeps running. Work you start goes on after the screen closes, as long as you stay
close (the timed action's own rule). The screen guard closes it in danger, more than **Dialogue >
Range** (6 m) from the bench, or after **Idle seconds** (120) with no pick.

**How it works.** The read action on a workbench's spot ("Use the workbench") reaches the server
(`RZ_Crafting.Look`), which opens a screen-guard entry and sends `PAYLOAD_CRAFT` (kind 12) to that
player: `B|serial|title|look|status|ready|recipes|town|hint|lit`, `W|recipe|seconds left|seconds`
while one is under way at this bench, a stove's status tiles (`S` records, [below](#the-status-strip-and-locked-rows)), then per recipe `R|index|name|prefab|icon|seconds|heat|ready` and per input
`N|index|name|have|need`; `X|reason` closes it. Have and missing are counted on the server against
the player's inventory, each carried item counted for one input only. Craft goes back as
`RpcAsk_Craft(serial, index)` (at most two a second); the client sends it only for a ready recipe
with nothing under way (it says what's missing instead). The server checks the screen is open on
that serial, the bench is there and you're within range and not busy, then runs the bench's own
code (`HandleRecipe`: the inputs checked, the timed work started); what it says is caught for the
status line and the screen is sent again. When the work is done the server checks the inputs again
and takes each with `TryDeleteItem`; the output is given only when every one came out. Up to sixteen
recipes are listed (the action menu has room for the first eight). The picture and its description
come from the output's preview copy on the client (`RZ_ItemPreview`).

**Fallbacks.** While the screen is used, the bench's **Make:** actions hide
(`RZ_CraftMenu.HidesSlotsOn`: the spot's read label is "Use the workbench"). With **UI > Craft row
layout** empty, "Use the workbench" lists what the first eight recipes take as a notice and the
**Make:** actions are as before. A client that can't open the menu (no `RZ_CraftMenu` preset or
layout) says so and shows the actions from then on; when the server can't open one (at its cap of
open screens) the client shows the actions for a minute. *Simplified:* the picture doesn't turn,
and a recipe added as data has no icon and names its inputs by their first keyword.

### The cooking screen

The crafting screen at a stove, oven or fireplace ([DESIGN.md](DESIGN.md#light-crafting)): **Use the
stove** opens it, with the same panel, rows, picture and keys, and the stove's words; the author's
pick of the mockups ("F with H's top status"). The header is "Stove", the town and what it looks
like ("Cold, but it would light...", or "The fire's going and the pot's on..." while something
cooks), "You can cook 2 of 3" on the right. Under it, the stove's
[status strip](#the-status-strip-and-locked-rows): **FIRE** (LIT or COLD), **FUEL** (ENOUGH: a
stove's fuel isn't counted) and **ON THE HOB** (EMPTY, or what's cooking with its seconds counting
down, "BOILED WATER · 12 S"); the panel is 1,600 × 960 with it.

On the left, under RECIPES, the stove's list: **Boiled water** (untreated water), **Home remedy**
(something strong to drink, herbs; the med icon) and the **stove recipes**
(**Interactions > Crafting > Stove recipes**; the food icon), each hidden while what it makes isn't set. Each row's tag on the
right is READY in olive, SHORT 1 (how many of its inputs you're short of) in red, NONE dim when you
carry none of it, or WORKING in amber. On the right, the picked row's detail: its picture, name and
description, IT TAKES with each input's have and need on the right ("1 / 1", olive when you have it,
red when you don't), "A lit stove: YES" (olive), or "A lit stove: NO, you'll light it" (dim: the
work lights it, so it never stops you), "About 30 seconds on the stove" and how far the smoke carries
("A little smoke from the chimney. Someone may see it.", or "Smoke, and the smell carries..." from 3
heat), and the button named for what the picked row does: **BOIL**, **BREW** or **COOK**, dark while
you're short. The keys are the crafting screen's; the footer's A says the same verb ("A Boil · B
Close") and dims while the button is dark.

**How it works.** The stove's read ("Use the stove") reaches the server (`RZ_Crafting.UseStove`),
which opens a screen-guard entry of its own kind (`KIND_COOK`) and sends `PAYLOAD_COOK` (kind 15)
with the crafting screen's records; each `R` record ends with the button's verb, the `B` record's
`lit` is 1 while the fire's going, and the stove's tiles come as `S` records (`RZ_Crafting.StoveStatus`:
the fire is lit while anyone's boiling, brewing or cooking on that stove, the player's own work first).
The client opens the same menu (`RZ_CraftMenu`, the `RZ_CraftMenu` preset) in its stove mode. A pick goes back as
`RpcAsk_Craft`, and the server runs the stove's own code (`HandleStove`: boiling, brewing, or a stove
recipe's timed work, `TASK_COOK`); when it's done the inputs are checked again and taken, and what it
says is caught for the status line. Up to sixteen rows are listed (the action menu has room for the
first four).

**Fallbacks.** While the screen is used, the stove's **Boil water**, **Brew a home remedy** and
**Cook:** actions hide (`RZ_CraftMenu.HidesSlotsOn`: the spot's read label is "Use the stove"), and
the stove isn't an object menu. With **UI > Cook row layout** empty, the read is "Look at the stove"
and the stove's first four are rows of [the object menu](#the-object-menu) (or world prompts without
it), as before, with the same tiles and its rows locked while you're short. A client that can't open
the menu says so and shows the actions; at the server's cap of open screens, the actions for a minute.
*Simplified:* the strip is sent with the screen, so another player's cooking on the same stove shows
the next time it's sent (your own pick, or your work ending).

### The radio screen

**View radio** opens it ([DESIGN.md](DESIGN.md#radios-open-and-secure)): on the wind-up radio held
up in hand and looked at (vanilla's inspection, beside its **Turn on/off**), on a radio lying on the
ground, and as a house radio's fourth option. A panel 1,400 × 900 in the middle of the screen, the
world dimmed, in the crafting screen's style: the radio's name and the town (a house set on weak
batteries says so), then under CHANNELS one row per channel: **Everything on the radio** (how many
lines you heard), the **government broadcast**, the four **open channels** (the Haven, Bastion, the
Red Hand, Halcyon) and the three **secure channels**. Each row has the channel's icon in its colour,
its name, its latest line (what's on the air now, else the newest you heard on it) and its state:
ON AIR, QUIET, TOO FAINT (a house set without power pulls in two open channels), SCRAMBLED, or
CRACKED: 14 MIN while you listen to one. A channel with nothing coming through is dim. Under them,
THE LOG: the picked channel's lines this life, newest first, in the notebook's radio rows (time,
icon, channel, who, the line).

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Pick a channel (the log follows) | up/down, or click it | D-pad or left stick |
| Scroll the log | Q/E, or the wheel | LB/RB |
| Close | Esc, or the close button | B |

The text is TV-sized: channel names 24, their lines 20, the smallest labels 18. You can't move while
it's open; the world keeps running. The screen guard closes it in danger, or more than **Dialogue >
Range** (6 m) from a radio that isn't on you; it never goes idle, since there's nothing to pick. The
picked channel is kept for the session.

**How it works.** The action (`RZ_ViewRadioAction`) or the house radio's option reaches the server
(`Interactions/RZ_RadioScreen.c`), which opens a screen-guard entry (`KIND_RADIO`) and sends
`PAYLOAD_RADIO` (kind 16): `V|title|sub|powered` and `C|kind|faction|name|state|line` per channel,
the open channels' lines being what each says now. A house radio's option picked in the object menu
closes that menu (`X|replaced`) and the radio screen takes its place. The client (`RZ_RadioMenu`)
adds the rest from what it already has: the log the notebook's Radio tab keeps
(`RZ_PlayerUIComponent.GetRadioLog`, read only) and the cracked channel and its minutes (the HUD's
feed). It sends nothing back but its closing, and changes nothing in the game: tuning and cracking
stay the radio's own actions.

**Fallbacks.** With **UI > Radio channel layout** empty, no radio has **View radio** and the house
radio's options are as before. A client that can't open the menu (no `RZ_RadioMenu` preset or
layout) says the log is in the notebook's Radio tab and hides **View radio** from then on; at the
server's cap of open screens it hides for a minute. *Simplified:* the latest line on an open channel
is read when the screen opens, not again while it's open (new lines you hear join the log).

### The object menu

One small generic screen in the crafting screen's style, for props and places with several
options or something to read ([Default first, then menus](#default-first-then-menus)): a stove
(without [the cooking screen](#the-cooking-screen)), a generator, a house radio, a well, a puzzle object or keypad, the safehouse services, a
quartermaster's asks, the lab, the notebook item's name and season. The author's pick of the mockups
("B with A's tag and the lock"). A panel 1,200 × 940 in the middle of the screen (810 for a thing
without a status strip), the world dimmed: the name ("Radio", "Ask the quartermaster") and the
town; under them the thing's [status strip](#the-status-strip-and-locked-rows) (a house radio's POWER
OFF, RECEIVE ON, TRANSMIT NO, SIGNAL WEAK) and one line saying what would change it ("A running
generator within 40 m would power it."); then what you see (the read's text: "Cold, but it would
light...", or for a house radio the line it caught, under "LAST HEARD · BASTION"), and under WHAT YOU
CAN DO the options, one row each. An option you can't do now is **locked**: dim, a lock before it
and a short tag on the right saying why (NEEDS POWER, OPERATOR ONLY). While work you started is
under way (boiling water, cracking a channel), "Busy with the boiling... 12 s" over a thin amber
bar. Under the list, what the last pick said.

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Pick an option | up/down, or click it | D-pad or left stick |
| Do it | Enter, or click the row | A (dimmed on a locked row; A there only says why, in the line under the list) |
| Close | Esc, or the close button | B |

The selected row has the amber bar on its left and a lighter fill, locked or not, so the D-pad
moves through every row. The text is TV-sized: options 26, the description 24, the tiles' values 24,
the smallest labels 18. You can't move
while it's open; the world keeps running. Work you start goes on after it closes, as long as you
stay close (the timed action's own rule). The screen guard closes it in danger, more than
**Dialogue > Range** (6 m) from the thing, or after **Idle seconds** (120) with no pick.

**How it works.** It lists the thing's own actions (`Interactions/RZ_ObjectScreen.c`). Every action
that can be a row derives from `RZ_GroupedAction`; on one entity the puzzle slots (read, use,
choice, keypad) form a menu when the puzzle object asks for one (a labelled prop that isn't a
person; a world interaction spot by its kind, `RZ_Interactions.UsesMenu`), and any other grouped
actions when two or more show. While this machine uses the menu, one prompt stays: the read
("Look at the stove") or, without one, the first action under the group's title ("Ask the
quartermaster"). Using it reaches the server, which runs the read with what it says caught
(`RZ_ScreenCapture`; a paper goes into the text, not the reader), opens a screen-guard entry and
sends `PAYLOAD_OBJECT` (kind 14) to that player: `O|serial|title|town|text|status|working|left|total|read
label|hint`, the thing's tiles as `S` records, and `R|index|label|enabled|reason|tag` per row (enabled
0: locked, with its full reason and its tag); `X|reason` closes it. A locked row's pick never leaves
the client: it puts the full reason in the status line. Any other pick goes back as
`RpcAsk_ObjectPick(serial, index)` (at most two a second). The server checks the menu is open on
that serial, the thing is there and you're within range and the row isn't locked (a locked one sends
the menu back with its reason), then runs that row's own `Run` (the code its world prompt runs);
what it says is caught for the status line and the menu is sent again with the strip and the rows as
they are now. A timed action's end or stop is caught the same way.

**On vanilla's own prop.** A thing can carry the slots itself instead of an invisible spot, so its
vanilla prompts and ours are one list: vanilla's small house radio does (the repo's copy of
`RadioSmall_01_Base.et`, [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#add-to-prefabs-you-already-have)).
Its read and four choices sit in its own `SCR_TurnOnAction` context, so looking at the radio shows
**Turn on** (or **Turn off**) and **Listen to the radio** together: D-pad to pick, A to use. The
puzzle object there is **Hidden until tagged**: none of ours show until a player is near and the
server has labelled it (`RZ_Interactions.CanHost`), and they hide again when everyone leaves.

**Fallbacks.** With **UI > Object row layout** empty, every action is a world prompt as before. A
client that can't open the menu (no `RZ_ObjectMenu` preset or layout) shows the description as a
card, tells the server (`RpcAsk_ObjectFailed`: the prompt it used runs, and that player gets world
prompts from then on) and shows the prompts; when the server is at its cap of open screens the
client shows the prompts for a minute. A dedicated server never hides a prompt, so a client
without the menu can use any of them.

### The status strip and locked rows

Two pieces the object menu and the cooking screen share: a row of readout tiles saying what state a
thing is in, and rows you can't use now shown locked with the reason.

**The strip.** Up to four tiles in a row under the screen's header: each a dark tile (`#2A2D30`)
with a small label in capitals (mono bold 18, `#B8B2A3`), a square lamp and a one-word value (mono
bold 24) beside it, and a 4-unit bar along its bottom, both lamp and bar in the state's colour. The
mockups' label was 16; it's 18 for the TV rule ([Controllers](#controllers)).

| State | Lamp and bar | Value | Means |
|---|---|---|---|
| On, ok | olive `#9AA35E` | `#C4CC8A` | on, working, you have it |
| Off, blocked | red `#D9574C` | `#F08A80` | off, empty, missing: something to fix |
| Partly | amber `#E8B23A` | `#E8B23A` | weak, low, under way |
| Unknown, idle | faint `#6E6A60` | dim `#A39E90` | nothing going on, or it doesn't matter now |

A value with a countdown ("BOILED WATER · 12 S") counts down on the client. In the object menu one
line under the strip says what would change it. Things without tiles show no strip, and the screen
closes up the space. What each thing shows:

| Thing | Tiles | The line under them |
|---|---|---|
| A house radio | POWER (ON, or OFF in red), RECEIVE (ON: its batteries hear the channels), TRANSMIT (YES with power; NO in red without, dim when calling isn't on), SIGNAL (STRONG with three or more open channels coming through, WEAK one or two, NONE) | Without power: "A running generator within 40 m would power it." Its read's text has "LAST HEARD · " and the channel it caught over it |
| A generator found in town | ENGINE (RUNNING, or STOPPED dim), FUEL ("~25 MIN", amber under 10 minutes, EMPTY in red), YOUR CAN (FULL when you carry fuel, NONE dim) | What it does running, or that a jerrycan would fill it |
| A stove (the cooking screen, and the object menu without it) | FIRE (LIT, COLD dim), FUEL (ENOUGH: not counted), ON THE HOB (EMPTY, or what's cooking and its seconds) | None |
| A well (**Interactions > Well spots** on) | WATER (UNTREATED, amber), BOTTLE (YES, NO in red), TABLETS (YES, NO dim) | "Boil it on a stove or purify it before you drink." |

The power station has none: its own screen is [the power panel](#the-power-panel), and without it
its prompts are never an object menu.

**Locked rows.** In the object menu, a row you can't do now is dim, has the lock icon
(`status_door_locked`, shown only with the icon imageset) before its name and, on the right, a tag of
one or two words in capitals saying why, dark on a hazard-yellow chip (`#F2C230`). The selected row
keeps its amber bar and lighter fill. A on it does nothing but put the full reason (the sentence the
thing's own code says when you try) in the status line, and the footer's A reads dimmed while it's
selected. The tags ([WRITING.md](WRITING.md#by-channel) has the rule for writing one):

| Row | Tag | When |
|---|---|---|
| Any row of a stove, generator, house radio or well | BUSY | Work of yours is under way (a house radio's **View radio** never locks) |
| A house radio's **Work on a scrambled channel** | OPERATOR ONLY, LISTENING, NO CHANNEL, TOO LOUD | Not a Radio Operator; already on a cracked channel; no faction place in range; too much noise or an infected close (the crack's own checks, in its order) |
| A house radio's **Call on the open channel** | NEEDS POWER | No power where it stands |
| A generator's **Start or stop** | NEEDS FUEL, ONE RUNNING | Stopped with an empty tank; one of yours already runs (stopping it never locks) |
| A generator's **Refuel** | NEEDS FUEL | No full jerrycan on you |
| A stove's **Boil water** | NEEDS WATER | No untreated water on you |
| A stove's **Brew** and **Cook:** rows | SHORT 1, SHORT 2 | How many of its inputs you're short of |
| A well's **Fill a bottle** rows | NEEDS BOTTLE, NEEDS TABLETS, NOT SET UP | No empty bottle; no tablets (purify); no water items configured |
| Any other action that has its own reason | NO SAMPLES, CURE OUT, STORE FULL, NOTHING, LOCKED | The lab's deliveries, a hand-over, a door: the action's own reason, given a tag by name; a reason of two words or fewer is its own tag, anything longer NOT NOW |

On the cooking screen the rows aren't locked: their tag is READY, SHORT 1 or NONE
([above](#the-cooking-screen)) and the button stays dark while you're short.

**How it works.** The server builds the strip with each send (`Interactions/RZ_ObjectStatus.c`): a
world interaction spot's from its own system (`RZ_SecureRadio.HouseRadioStatus`,
`RZ_Traps.FoundGeneratorStatus`, `RZ_Crafting.StoveStatus` and `WellStatus`), anything else's from
the menu's opener (`RZ_GroupedAction.MenuStatus`, none by default). Each tile goes as
`S|label|value|state|left` (state 0 unknown, 1 on, 2 off, 3 partly; left the seconds its value
counts down). A row's lock is `RZ_GroupedAction.Locked`: by default the action's own
`CanBePerformed` and reason, with a tag by `RZ_ObjectStatus.TagFor`; a spot's choice
(`RZ_PuzzleChoiceAction`) asks its system first (`RZ_ObjectStatus.SpotLock`), with the same
sentence the action says when it runs. The client draws the tiles into `Tile0` to `Tile3` of the
screen's layout (`UI/RZ_StatusStrip.c`), which also keeps the footer's A to relabel and dim it.
*Simplified:* the world prompts aren't locked (they still say why when used); a strip is as fresh as
the last send; SIGNAL is a word, not bars.

### The power panel

The power station's control as a screen ([DESIGN.md](DESIGN.md#the-power-grid)). The control has
one prompt, **Open the panel**, and everything happens in here: no other prompt on it, and never
**Use**. A panel 1,400 × 860 in the middle of the screen, the world dimmed: the station's name
("The substation outside Morton") and its state ("Dark. The control room is open."), what you
see, then two halves. On the left, on paper, **the card inside the door**: "AFTER A BLACKOUT, CLOSE
IN THIS ORDER:", three breakers by name, and one to leave open ("Leave PUMPS open. It's burnt
out."); before the door is forced, *Behind a padlocked door.* On the right, **the board**: six
breakers in a row, each with its number, its name, a lamp and a handle (up and lit when closed,
down and dark when open), and under them a READY lamp. Under the card, the spin-up's countdown
("SPINNING UP: 64 s. STAY WITHIN 60 M.") or, once it runs, the service gauge, each over a bar, and
what the last move said. Under the board, the step's button: **Force the door**, **Start the
generator** (dim with "The breakers first" until the board reads READY), **Throw the main switch**,
or **Refuel and grease the generator** (dim with "Done for now" for 10 minutes after you did).
While the generator spins up there's no button: close the panel and hold the area.

**The breakers.** Close the card's three in its order: each one holds and its lamp lights, and
after the third the board reads READY. Anything else trips the lot back open: a breaker out of
turn, opening one that's closed, and the burnt one, which bangs and carries further (heat 8 at
the station; a wrong one 2). Six names on the board in random places, so the card has to be read;
a fast player is done in fifteen seconds. One board per station, shared: two players at the panel
see each other's moves.

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Pick a breaker or a button | left/right, or click it | D-pad or left stick left/right |
| Between the board and the buttons | up/down | D-pad or left stick up/down |
| Flip it or press it | Enter, or click it | A |
| Close | Esc, or the close button | B |

The text is TV-sized: the card 26, what you see 24, the button 24, the smallest labels 18. You can't
move while it's open; the world keeps running. The screen guard closes it in danger, more than
**Dialogue > Range** (6 m) from the control, or after **Idle seconds** (120) with no move.

**How it works.** The read action on the control (`RZ_ReadClueAction`, labelled **Open the panel**;
every choice slot is labelled empty, so hidden, and the puzzle object never asks for the object
menu) reaches the server (`RZ_PowerGrid.HandleRead`), which opens a screen-guard entry (kind 20)
and sends `PAYLOAD_POWER` (kind 21) to that player: `P|serial|title|state|text|status|step|left|total|gauge|ready`,
then from the door's forcing on `B|index|name|closed` per breaker and `C|line` per line of the card,
and `A|index|label|enabled|need` per button; `X|reason` closes it. A move goes back as
`RpcAsk_PowerMove(serial, move)` (0–5 a breaker, 10 + n button n; at most two a second, and the
client doesn't send faster). The server checks the panel is open on that serial, the control is
there and you're within range, then runs the step's own code (`World/RZ_PowerPanel.c`, the
grid's `ForceDoor`, `StartGenerator`, `ThrowSwitch`, `Maintain`); what it says is caught for the
status line, and every open panel is drawn again. What the station says to everyone near
(the generator settling, dying) goes into the status line of a player with the panel open.

**Fallbacks.** With **UI > Power breaker layout** empty, the control has its old world prompts:
**Read the panel** (what you see as a notice) and the step's one choice (the breakers are done in
one go with the generator). A client that can't open the panel (no `RZ_PowerMenu` preset or
layout) tells the server, which puts the old prompts back for everyone (every client has the same
mod) and says what you see as a card; when the server is at its cap of open screens, what you
see is a card and the panel opens next time. The old prompts never show unlabelled slots: while
the generator spins up every slot is labelled empty (hidden), where an empty label list used to
show the control prefab's **Use** and **Choose**.

### The spawn screen

As in `background_picker.html`: our own screen opens where vanilla's deploy menu and role selection
would, after the welcome screen and after every death. There's no faction or group step: the
faction is forced to `SURV` (`SCR_MenuSpawnLogic`'s **Forced faction**). On the left, the 13
backgrounds in the game mode's order, a locked one dim with a lock and LOCKED. On the right, the
picked one: its name in large type, AVAILABLE or LOCKED, its perk and flaw, its **starting gear**
(what that loadout really spawned with the last time anyone took it, noted by the server up to
ten things a few seconds after the spawn, by prefab name; until then the background's own gear
line, `RZ_Background.m_sGear`), its starting standing ("The Haven +10 Fame"), how to unlock it
when it's locked, and your Legacy toward the Drifter. Under it, **Start**: only the kind of place
the picked background starts at ("A clinic, or a house near one"), never where. Nobody picks a
start: the screen asks for one of the open spawn points at random, and the server's opening rolls
the real place ([DESIGN.md](DESIGN.md#openings-how-every-life-starts)). Then **Deploy**, which counts down while the respawn timer holds you
("DEPLOY IN 0:08") and is dim while the pick is locked. A line under Start says why nothing
happened, if it didn't.

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Background | up/down, or click a row | D-pad or left stick up/down |
| Deploy | Enter, or click Deploy | A |
| The pause menu (to leave or for settings) | Esc | B |

It opens on the background you picked last this session, else the first that isn't locked, with
the focus on that row. The text is TV-sized: names 26, the panel's text 22 to 24, the heading 46.

**How it works.** `RZ_SpawnMenu.c` mods `SCR_PlayerDeployMenuHandlerComponent.Update`: where vanilla
would open its deploy menu or role selection, it opens the `RZ_SpawnMenu` preset instead (and
closes vanilla's if one is open); the welcome screen stays vanilla's. The screen asks for the
backgrounds payload when it opens (`PAYLOAD_BACKGROUNDS`: `G|legacy|Legacy for the Drifter`, then
`B|id|name|loadout|perk|flaw|locked|unlock text|Haven|Bastion|Red Hand|Halcyon|gear|where you
start` per background, the last the kind of place, `RZ_OpeningText.ScreenPhrase`) and finds each background's loadout by name in the loadout manager. Deploy spawns
through the vanilla requests, as the deploy menu did: `SCR_PlayerLoadoutComponent.RequestLoadout`
with the picked loadout, then, once the server says yes, `SCR_RespawnComponent.RequestSpawn` with
an `SCR_SpawnPointSpawnData` (the loadout's prefab and the spawn point). On an OK the screen
closes; on a refusal it says so and you can pick again. **Fallbacks:** with **UI > Spawn row
layout** empty, or if the screen can't open on a client (no preset or layout: the log says so),
vanilla's deploy menu opens as before.

**Locked backgrounds.** A locked background's loadout can't be spawned (**UI > Lock loadouts**, on).
The server decides, whatever the client sends, so it holds with a mouse, a gamepad or a modified
client: `RZ_LoadoutLock.c` mods `SCR_LoadoutManager.CanAssignLoadout_S` (a locked loadout is never
assigned) and `SCR_BaseGameMode.CanPlayerSpawn_S` (a spawn is refused while the assigned loadout
is locked, or when it asks for a locked loadout's prefab). The screen only shows it, from the
backgrounds payload, and won't send a locked pick (it says how to unlock it instead). With the
spawn screen off, the client marks the loadout unavailable in vanilla's deploy menu
(`SCR_BasePlayerLoadout`, modded). If a locked pick gets through anyway, it plays as the default
background. Before the server-side refusal, Workbench play spawned the locked Soldier loadout as
the default background; the refusal itself is still to be checked in play.

### The death brief

When you die, one card of the life that just ended opens over the spawn screen (or vanilla's deploy
menu). Clean and short: a 1,040 × 700 dark panel with a bone rule along its top, the world dimmed
behind it.

| Part | What it shows |
|---|---|
| Kicker (bold 22, dim, upper case) | the season day and where you fell: "DAY 14, NEAR MORTON" (the nearest settlement, else the grid) |
| Name (bold 48, bone, upper case) | your player name |
| Who (26, dim) | the background and which life it was: "Nurse, life 3" |
| The lines (26: labels dim, values bold bone) | at most seven, in this order: **How it ended**; **Lived**; **Infected put down**; **People killed** (only when there were any); **Jobs done**; **Travelled**; **Standing** (only with Fame gained) |
| Closing (26, bone) | one line in the narrator's plain voice, by how it ended ([WRITING.md](WRITING.md#tone)): no joke, nothing about the body |
| Footer | the Continue prompt by device, and **CONTINUE** at the bottom right |

What each line says, and where it comes from (all this life's, server-side):

| Line | Value | From |
|---|---|---|
| How it ended | Turned; The infected; Killed by *a player's public name* (or "another survivor"); Killed by Task Force Bastion, the Red Hand or Halcyon; Killed by other survivors (guards, wanderers, farm people); Your own doing; Nobody saw how | turned: the game mode's `TurnPlayer`; a player: the killer the bounties use; otherwise the faction of whoever dealt the last blow (the damage manager's instigator), and with nothing recorded, an infected within 6 m of the body |
| Lived | "41 minutes, Day 3" or "2 hours 5 min, Days 3 to 4" | the life's played time (`m_fLifeSeconds`) and the season days it spanned |
| Infected put down | a count, or None | `m_iLifeInfected`: each infected death counts for the nearest living player within 120 m |
| People killed | a count | the life's kills of faction people (`m_aKills`, the tally reputation keeps) |
| Jobs done | a count, or None | contracts and jobs finished this life (`m_iLifeMissions`) |
| Travelled | "800 m", "3.4 km" | `m_fLifeMetres`: the ground covered each server tick, on foot or in a car (a jump over 300 m in one tick, a respawn or a teleport, doesn't count) |
| Standing | "+35 Fame with the Haven" | the faction this life gained the most Fame with (`m_aFameGained`) |

The closing lines: a life under 10 minutes, *"Not long. It rarely is, out here, for anyone who comes out of hiding."*; otherwise one per
way it ended (turned, the infected, a player, a faction, other survivors, your own doing, nobody saw).

| | Keyboard and mouse | Gamepad |
|---|---|---|
| Continue to the spawn screen | Enter, Space, Esc, or a click anywhere | A or B |

Presses in its first 0.6 s don't count (you may still be firing). It never holds up the respawn:
the spawn screen and its respawn timer run under it, and it closes by itself if you're alive again.

**How it works.** At a death (`RZ_FactionPerks.OnPlayerDied`, before any other system hears of it)
`RZ_DeathBrief.OnDied` builds the brief once and sends it as its own payload kind (30, kept apart
from `RZ_UIFeed`'s list) to that player only: `H|name|who|day and place`, then
`L|label|value` per line, then `C|closing line`. The two new counters are on the profile (saved,
reset with every new life); the game mode's tick adds the ground covered and `OnZombiesKilled`
credits the infected. The client holds the brief until a respawn screen is open (ours, vanilla's
deploy menu or role selection; at most 20 s), then opens the `RZ_DeathBriefMenu` preset on top of
it. The spawn screen and vanilla's menus only take input while they're on top, so the press that
closes the brief must not reach them: A's `MenuSelect` comes on the release and closes it on the
next frame, and anything else (Space, a click, A's press through `RZ_TalkSkip`, the button) closes
it 0.4 s later, after the release.

**Fallbacks.** **UI > Death brief** (on) off: nothing at a death, as before. No UI component on the
player's controller: nothing. A client that can't open the screen (no preset or layout) shows the
same lines as one notice card. If you're already alive again when it arrives, it's dropped.
*Simplified:* infected deaths are credited to the nearest player, whoever fired; the cause is what
the engine recorded for the last blow (a bleed-out after a shot names the shooter, a fall names
nobody); players killed aren't counted.

### The map

`RZ_MapIconsUI` is a component on the map configuration: while the map is open it asks the server
for this player's list (on open, then every 20 s) and draws one marker per place, with a label,
following pan and zoom. Each marker is 34 across (design A, approved by the author):

| Part | Size | What it is |
|---|---|---|
| Ring | 34, about 2 thick | a thin hollow ring (`marker_ring`), near-black (#121416), with a small gap to the disc |
| Disc | 28 | a solid disc (`marker_disc`) in its owner's colour: the faction's; hazard yellow for the sealed checkpoints; steel grey (#4E5459) for the power and radio stations |
| On the disc | 18 | a faction's emblem (`faction_*`) in bone; for a place without a faction, its glyph without a plate (`glyph_power`, `glyph_radio` in bone; `glyph_checkpoint` near-black, as bone wouldn't read on yellow) |
| Badge | 17, bottom right, over the ring's corner | only on a faction's place: a dark chip (`marker_disc`, #121416) with a bone glyph in a 1.5-thick ring saying what the place is: `badge_stronghold` (a keep), `badge_outpost` (a flag), `badge_safehouse` (a house), `badge_lab` (a flask) |
| Label | condensed bold 22, black, a soft light shadow | under the marker, so it reads on the light map and on a TV: the place's kind alone (Stronghold, Outpost, Field station for Halcyon's, Safehouse, Lab, Power station, Radio station, Checkpoint), as the disc's colour and badge already say whose it is |

The feed still names each place by its plated icon (`marker_stronghold` and so on); the layer turns
that into the glyph and the badge (`RZ_MapIconsUI.Glyph` and `Badge`). **One place, one marker:**
Halcyon's stronghold sits at the lab, so within 250 m of the lab only the lab's marker is sent; a
safehouse within 250 m of the Haven's stronghold is the stronghold's house, so only the stronghold's
is (`RZ_UIFeed.MAP_SAME_PLACE`). Markers still within 40 m of each other share one label.
The packs your character knows of are **range
rings** (`marker_range`, in hazard yellow), drawn as before, without disc or badge: a line's last field is a radius in metres, and the ring is sized to
it on the map every frame (48 to 320 units across), its label centred under it, so it grows and
shrinks with the zoom: Heard of, half a kilometre round the rough area; Read and Mapped, the pack's
ground (Mapped adds where it lies up to the label). The list is what this player may know ([DESIGN.md](DESIGN.md#settlements-spawn-points-and-the-map)).
With **UI > Map icon layout** set, the vanilla text markers for events and the lab are off; without
it, the layer does nothing and those markers stay.

**Your objective.** Of the jobs, only the [tracked one](#the-quest-tracker) is on the map, and only
its current objective, sent to its player alone (`RZ_QuestTracker.MapObjective`, the feed's last
line, so it's drawn above the rest). No other contract, world event, questline or errand has a
marker. It comes in two looks, in the objective's amber (#E8B23A, `RZ_UIColor.Objective`):

| What you know | On the map |
|---|---|
| **A known spot** (an errand's or questline's place; a contract or event you're close to, Trusted with its giver, or a Radio Operator for) | An **amber diamond** on the spot: the tracker's filled diamond (`track_current`) in a 44 box tinted near-black (#121416) as the outline, the same in a 36 box in amber on it, and a dark centre dot (`marker_disc`, 8), centred like the other markers. Its label is the job's name only (the objective's words stay on the tracker); a place within 40 m of it (its stronghold, a quartermaster) puts its own name under that line instead of a label of its own |
| **A search only** (a contract or event while you know only its rough area) | The **search area**: the rough circle (its centre off the real spot, the spot inside it) as a soft amber fill (about 14%) with a thin solid amber edge, 2 wide at every zoom, at least 48 across, with the job's name centred under it. It's drawn on a canvas the layer makes in the map's root (`RZ_UIW.MakeCanvas`: a filled polygon and a closed line), under the markers, not stretched from an image, so its edge stays thin. Once you learn the spot it becomes the diamond at the next refresh |

Nothing shows while nothing is tracked or the job has no place (a questline between stages, a job
for the whole Zone). The old dashed range ring with claw marks is a pack's alone.

**You are here.** Above the other icons, the layer draws your own position: a hazard-yellow arrow
(`marker_self`) on a dark disc (`marker_self_back`), 44 across, turned to where your character (or
the vehicle you're in) faces, moved every frame while the map is open. It's worked out on your
machine from the entity you control; nothing is sent, and it's hidden while you control nothing
(dead, spectating). It needs only **UI > Map icon layout**, so it shows whatever vanilla's map
config does; whether vanilla can show its own player marker in this game mode hasn't been checked.

**The map key** sits on the map, not in the notebook: a compact legend in the bottom-left corner,
inside the safe area and 190 px up, above vanilla's own bottom-left widgets (the map's key hints and
the scale bar, about the first 330 px across and the bottom 120 px) (`RZ_MapLegend.layout`, **UI > Map
legend layout**; it needs the icon layer).
It shows the markers as the map draws them, in three blocks: *Who holds what*, the four factions'
discs (ring, disc and emblem, 34 across) with their names (Haven, Bastion, Red Hand, Halcyon);
*What the place is*, the four badges (26 across) with theirs (Stronghold, Outpost, Safehouse,
Lab); and *The world*, the range ring for a pack's ground, the discs for a sealed checkpoint and the
power and radio stations, and last a row for the job you track: the diamond (Your objective) and
an amber disc with its edge (Search area). Names are 22, and it says the colour is whose it is. It's there in full while the map is zoomed out and fades as you zoom in: full while
a kilometre of map is at most **Legend full zoom** (150) units across on screen, gone at **Legend
gone zoom** (450), fading in between, so it never covers the streets you're reading. One input
shows or hides it, `RZ_MapLegend` (**I**, pad **Y**); its own prompt says so, and the choice lasts
the session.

## Controllers

Everything in ReforgerZ must work on console (Xbox, PlayStation) and with a gamepad on PC, as well
as with keyboard and mouse. This is a rule for the whole mod, agreed with the author (CLAUDE.md,
"Design guardrails"); this section owns the details.

| Rule | What it means |
|---|---|
| Every input has a gamepad binding or a pad route | Each ReforgerZ action in `Configs/System/chimeraInputCommon.conf` binds a gamepad input next to its key, or what it does is reached another way on a pad (the table below) |
| Every screen works without a mouse | It opens from something a pad can reach, moves with the D-pad or left stick, switches tabs with the shoulder buttons (`MenuTabLeft`/`MenuTabRight`), confirms with A (`MenuSelect`) and closes with B (`MenuBack`). Each screen gives a widget the focus when it opens (`RZ_UIW.Focus`), so the D-pad has something to move from |
| Nothing needs hover, typing or a chat command | What a tooltip says also shows when the item has focus; whatever a chat command does also has a button or an action |
| Hover lights what the pad would | On every screen the mouse over a row or a button shows the pad's selected look, and moving off returns it (below). It never fires for a pad or the keys: only when the mouse was the last thing used (`RZ_UIW.ByMouse`) |
| Default first, then menus | Vanilla's own feature where there is one, a menu for anything with options or text, a single prompt only for what's done in a hurry ([the rule and the audit](#default-first-then-menus)) |
| Readable on a TV | Sizes at 1920×1080: no text under 18 px and body text 26 px or more on the new screens (talk, reader, subtitles, the radio-line box); the tabs and footers of the older screens are 18 px. Nothing important within 96 px of the left and right edges or 54 px of the top and bottom (a 90% safe area). Footer prompts are vanilla's input button (`RZ_UIW.AddPrompt`, `WLib_InputNavigationButton`), which shows the key or the pad button by the device in use |

**The input actions** (all in `Configs/System/chimeraInputCommon.conf`; menus also use vanilla's
`MenuUp`/`MenuDown`, `MenuSelect`, `MenuBack` and `MenuTabLeft`/`MenuTabRight`):

| Action | Keyboard | Gamepad | Context |
|---|---|---|---|
| `RZ_OpenNotebook` | N (the notebook item into the hand, then the notebook) | View + RB, the same (also the pause menu's **Notebook** button, the item's quick slot, or **Read the notebook**) | `IngameContext` |
| `RZ_RadioFilter` | F | X | `MenuContext` |
| `RZ_TrackJob` | T, Space | Y | `MenuContext`: Track or untrack the selected job on the notebook's Quests tab, as A (`MenuSelect`), the TRACK button and a double click on the job's row (two clicks within 0.4 s) do there |
| `RZ_BoardClear` | Delete | X | `MenuContext`: the notice board's Clear, server admins only |
| `RZ_Reply1` … `RZ_Reply5` | 1 … 5 | LT + D-pad up, right, down, left for 1–4; RT + D-pad up for 5 (the D-pad and A also pick the focused reply) | `MenuContext` |
| `RZ_CarStart` | T | Y | `MenuContext` |
| `RZ_TurnPreview` | none: left/right (`MenuLeft`/`MenuRight`) or a click | right stick (horizontal) | `MenuContext` |
| `RZ_TalkScroll` | none: the mouse wheel | right stick (vertical): a box of what they say on the talk screen, when it overflows | `MenuContext` |
| `RZ_TalkSkip` | Space, or a click (left mouse button) | A (with `MenuSelect`: one step a press, finishing the typing, then the next box, then the replies; the press that ends their line never says a reply) | `MenuContext` |
| The talk screen's pages (vanilla `MenuDown`/`MenuUp` past the ends, `MenuTabLeft`/`MenuTabRight`) | down past the last reply, up past the first; Q/E, or a click on an arrow | the same with the D-pad or stick; LB/RB, the page back and on (round from either end) | `MenuContext` |
| `RZ_MapLegend` | I | Y | `MapContext`: show or hide the map key on the full-screen map |
| Kick free of a hold (vanilla `CharacterJump`, listened to only while held) | Space | the pad's jump button (A) | `IngameContext` |

**Breaking a hold** ([INFECTED.md](INFECTED.md#how-they-move-and-hunt), Attacks). When something has
your ankle, `RZ_PlayerEffectsComponent` listens to the base game's Jump for the hold's few seconds:
**Hold kicks** (3) presses kick you free. It needs no binding of ours, so it is the same button on a
pad and a console. The prompt is the base game's hint with its timer ("Something has your ankle" /
"Kick free: press Jump 3 times."), shown for the hold and hidden when it ends; it has no widget of
ours. The stagger and the knockdown have no prompt, only a line.

N is free in play (vanilla uses it only in the group menu and the map's pencil tool, other
contexts). The pad has no free single button in play, so the notebook is a combo, View + RB (Start's
pause menu and the notebook item reach it too). The combos are chosen to miss vanilla's: vanilla
pairs View (the map on its own) with the D-pad (weapon groups), X (the group menu), Y (the editor), A
(hints), B (tasks) and Menu (the scoreboard), never with RB; and the triggers in a menu only move
between pages (`MenuNavLeft`, `MenuNavRight`), which the talk screen doesn't use, so LT and RT with
the D-pad say a reply at once there (the D-pad also moves the focus, harmlessly: the reply is picked
by its number). The talk screen's pages are on LB/RB, which no reply combo uses, so a page turn never
says a reply; what they say scrolls with the right stick instead, and moving past the last or first
reply turns the page too (the same D-pad that chooses). On their line, A only steps it (finish the
typing, the next box, the replies): the replies take A once they show, and a press or click that
ended the line doesn't also say the reply that comes up under it (the screen ignores it for 0.3 s).
Not yet checked with a pad. X is free in the board menu, so the radio filter and the board's Clear
share it, each only in its own screen; Y and T are the car screen's Start and the notebook's Track,
each only in its own screen.

**Hover.** Every row and button a screen hears clicks on also hears the cursor, through the one
handler they share (`RZ_MenuClick` into `RZ_MenuBase.OnWidgetHover`), so the mouse and the pad show
the same thing:

| What's under the cursor | What it does | Screens |
|---|---|---|
| A row a click acts on | Becomes the selected row, drawn in place, so A or Enter then does what a click would | The talk screen's replies, the Standing page's rows, the object menu's options, the car repair screen's jobs (and the ring on the car), the power panel's breakers and its two buttons |
| A row a click only picks | Lit like the selected row while the cursor is on it; the selection, its detail and what A does stay as they were, so passing over rows on the way to BUY, CRAFT or DEPLOY changes nothing | The shop's items, the crafting and cooking screens' recipes, the spawn screen's backgrounds, the radio screen's channels, the notebook's Quests and Journal rows |
| Any other button | A lighter fill and an amber bar on its left edge, made in the button's Overlay the first time (`RZ_UIW.HoverMark`), off when the cursor leaves | The notebook's and the shop's tabs, every close button, CRAFT, BUY, DEPLOY, START THE ENGINE, TRACK, the radio filter chip, PIN IT UP, the reader's arrows, the death brief's CONTINUE |

Rows lit by hover are redrawn in place, never rebuilt, so the list stays where it is under the cursor;
and while the mouse is in use a list doesn't scroll to its selection (`RZ_UIW.KeepInView`), which the
pad and keys still get. Left out: the shop's 3D preview (a click turns it), the death brief's
full-screen click-off area and the talk screen's page arrows (pictures, no Overlay).

**The check** (done in the files; not yet played with a pad, the VERIFY list's UI part):

| Part | Checked | Fixed |
|---|---|---|
| Input actions | Every `RZ_` action in `chimeraInputCommon.conf` has a pad binding or a route (the table above) | `RZ_BoardClear` added with its pad button; `RZ_MapLegend` came with one (Y) |
| The screens | The spawn screen, the notebook, the notice board, the talk screen, the reader, the car repair screen, the shop, the crafting and cooking screens, the radio screen, the object menu, the power panel and the death brief each open from something a pad reaches (an action, the pause menu, the notebook item, a talk; the spawn screen and the death brief open by themselves; **View radio** on a radio held up is vanilla's inspection, which a pad reaches), close with B (the spawn screen's B opens the pause menu; the death brief's A closes it too), set the focus and move with the D-pad and shoulders. The HUD, the subtitles, the nametags and the map icons only show; the map key on the map shows and hides with Y | The notice board had no prompts, no focus on its button and no way to write without typing: it now has prompts by device, A on Pin it up, seven ready-made notes on left and right, and is on by default (**UI > Board menus**) |
| Static hints | Footers and hints that named only keys | The radio filter's hint ("F / X"), the talk screen's keys hint ("1-5: a reply · Q/E: page · Esc: leave", with D-pad, A, LB/RB and B in the footer) and its line's footer (vanilla's prompts, "A Skip" while it types and "A Continue" after, with a static "A: continue · B: leave" fallback that names Enter, Space and Esc on keys) |
| Hover, typing, chat | No screen needs hover or a tooltip; hover only shows what the D-pad would (below). The only typing is the board's note box (now optional) | Every chat command has a way without it: `/note` the board's ready-made notes, `/noteclear` the board's X, `/mydead`, `/season` (also the notebook's The season tab) and `/anonymous` the notebook item's actions, `/lastentry` the Names board's *Write your line*, `/tribes`, `/talkdebug`, `/traces` and `/rep` (the nearest player) as **[Admin]** replies when an admin talks to any person; `/noteclear all`, `/rep` for a named player far away and `/epilogue` stay typed admin extras |
| Text on a TV | Every layout's text sizes | All text in the ReforgerZ layouts is 18 or more (it was down to 11 on the old Standing page; 12–17 on the other pages, the notice board, the notice cards, the HUD and the map icons). The Standing page (G2) has nothing under 18; its names are 40 and its detail text 22. |

**What needs Workbench** (it can't be checked as text):
- Open each page in the layout editor at 1920×1080 and look for text that now runs past its slot:
  the HUD's infection stage above its clock (2 over); on the Standing page, the longest names
  ("SOFT-HEARTED DEVIL" at 40 in 480 units, "TASK FORCE BASTION" at 28 in 320) and two Next lines
  in their 56 units. Where it touches, move the
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
| `RZ_UIFeed` | server, on the game mode's tick | Works out each player's infection state, the secure channel they hear, and a noise level (0–6 from the heat where they stand), and sends each **only when it changes**; with Stealth on, stealth's own update (every second) hands it each player's visibility, sent the same way as a level 0–5 and whether it counts as hidden (the timer also resyncs every minute, and at once when the time left jumps more than 5 s from what the client is counting down; the client counts down in between). A bite cleaned in time still shows the golden window until it would have closed, so the HUD never tells early whether the clean worked. Builds the standing, journal (the jobs with their kind and current objective, the tracked one, the questlines the player is part of, the notes and the lab's line), background, map, notice board, hall of fame and season tab data when asked (the season tab its own kind, 11, after the kinds only sent). Remembers the clues each player has read while the puzzle runs, the outposts each player has been near (this session) and each loadout's gear |
| `RZ_QuestTracker`, `RZ_QuestText`, `RZ_ObjList` (`UI/RZ_Quests.c`) | server, on the UI feed's tick | Each player's jobs, the tracked one (saved on the profile), the order they were taken in, each job's objectives and the rough-area wording; sends the HUD's tracker (payload kind 13) only when it changes, and the tracked job's current objective for the map ([The quest tracker](#the-quest-tracker)) |
| `RZ_PlayerUIComponent` | on the player controller | The pipe: owner-only RPCs from the server, a request RPC and a command RPC from the client (each payload kind and each command at most every 2 s; a command sent too soon gets a short notice instead of vanishing), and the client's cache. Keeps the Radio tab's notices on the client for this life and the notebook's unread counts. A notice board payload from the server opens the board menu; a reader or talk payload opens those screens. Carries talk picks and closed screens back |
| `RZ_Dialogue`, `RZ_TalkSpeaker` (`RZ_TalkGeneric`, `RZ_TalkLeader`, `RZ_TalkStaff`), `RZ_TalkNeeds`, `RZ_DialogueTrees` | server | Conversations: sessions, capture, replies and their needs, picks; with Trees on, topics, nodes, checks and their tags ([DIALOGUE.md](DIALOGUE.md)) |
| `RZ_TalkAction` (`Dialogue/RZ_TalkStaff.c`) | every machine | The one **Talk to the quartermaster** / **Talk to the medic** prompt on a person with no puzzle components; while the talk screen is used, the person's other ReforgerZ actions hide (`RZ_TalkMenu.HidesActionsOn`) |
| `RZ_TalkMenu` | client (`ChimeraMenuBase`) | The talk screen ([The talk screen](#the-talk-screen)) |
| `RZ_Nametags`, `RZ_NametagClient` | server, on the dialogue's tick; client, from the HUD's update | [Nametags](#nametags): each player's list of the people near them to talk to, and the tags over the ones in sight |
| `RZ_Reader`, `RZ_ReaderMenu` | server, client | The reader: one player's paper, a page at a time ([Subtitles and the reader](#subtitles-and-the-reader)) |
| `RZ_Cars` (its repair screen part), `RZ_CarMenu` | server, client | The car repair screen ([above](#the-car-repair-screen)) |
| `RZ_ShopScreen`, `RZ_ShopOpenAction`, `RZ_ShopMenu` | server, server, client | The shop screen ([above](#the-shop-screen)) |
| `RZ_Crafting` (its screen part), `RZ_CraftMenu`, `RZ_ItemPreview` | server, client, client | The crafting screen and the cooking screen ([above](#the-crafting-screen)) |
| `RZ_RadioScreen`, `RZ_ViewRadioAction`, `RZ_RadioMenu` | server, every machine, client | The radio screen ([above](#the-radio-screen)) |
| `RZ_ObjectScreen`, `RZ_ActionMenus`, `RZ_GroupedAction`, `RZ_ObjectMenu` | server, every machine, every machine, client | The object menu ([above](#the-object-menu)) |
| `RZ_ObjectStatus`, `RZ_StatusStrip` | server, client | The status strip's tiles and the locked rows' reasons and tags, shared by the object menu and the cooking screen ([above](#the-status-strip-and-locked-rows)) |
| `RZ_PowerPanel` (`World/RZ_PowerPanel.c`), `RZ_PowerMenu` | server, client | The power panel and its breakers ([above](#the-power-panel)) |
| `RZ_ScreenCapture` | server | While a screen acts for its player, catches what would have been that player's notice for the screen's status line |
| `RZ_ScreenGuard` | server, once a second | Watches open talk, reader, repair, shop, crafting, cooking and radio screens, object menus and power panels and closes them in danger (below) |
| `RZ_PlayerEffectsComponent` | on the player controller | The infected's close moves on the owner's machine: a hold (with the hint and Jump to kick free), a stagger, a knockdown; the server decides each one ([INFECTED.md](INFECTED.md#how-they-move-and-hunt)) |
| `RZ_ChatCommands` | client | `/note`, `/noteclear`, `/tribes`, `/traces`, `/rep`, `/talkdebug`, `/mydead`, `/season`, `/anonymous`, `/lastentry`, `/epilogue`, sent as commands to the server, which checks them again. Registered with the first notice. Each has a way without typing ([Controllers](#controllers)), but the admins' `/epilogue` |
| `RZ_HudDisplay` | client (`SCR_InfoDisplayExtended`) | Draws the HUD five times a second from the cache; shows the notice cards, the subtitles (the arrow to an in-person speaker every frame), the radio-line box and the quest tracker |
| `RZ_SayLine`, `RZ_ESaySource` | server, client (`UI/RZ_UIData.c`) | A spoken line's who and how: name, role, faction, source, channel, where the speaker is ([Subtitles](#subtitles-and-the-reader)) |
| `RZ_SpeakerFinder` | server (`UI/RZ_UIFeed.c`) | The nearest person to a player, for a spoken line whose caller gave no body |
| `RZ_DoorMarkClient` | client, from the HUD's update | Traces' door marks as decals near the player, from the game mode's replicated list and the kit's atlas ([TRACES.md](TRACES.md#doors-you-can-see)); nothing without the atlas |
| `RZ_TraceLightsClient` | client, from the HUD's update | Traces' lit windows (S2) near the player, from the game mode's replicated list: a small warm point light each within 150 m ([TRACES.md](TRACES.md#sound-and-light)); nothing unless **World > Traces > Lit windows** is on |
| `RZ_NotebookMenu` and its pages (`RZ_NotebookPage`), `RZ_NoticeBoardMenu` | client (`ChimeraMenuBase`) | Ask for their data on open (each page when it's shown), redraw when a new copy arrives; `RZ_MenuClick` passes mouse clicks to them ([The notebook](#the-notebook)) |
| `RZ_Notebook`, `RZ_PauseNotebook`, `RZ_NotebookItem` (`RZ_NotebookGadgetComponent`, `RZ_ReadNotebookAction`) | client | Open the notebook: N (through the item in the hand), the pause menu's button, the notebook item (a gadget) and its **Read the notebook** |
| `RZ_DeathBrief`, `RZ_DeathBriefMenu` | server, client (`ChimeraMenuBase`) | The death brief: this life's two new counters (infected put down, ground covered), the brief built once at a death and sent to that player; the card over the spawn screen ([above](#the-death-brief)) |
| `RZ_SpawnMenu`, `SCR_PlayerDeployMenuHandlerComponent` (modded, `RZ_SpawnMenu.c`) | client (`ChimeraMenuBase`) | The spawn screen in place of vanilla's deploy menu: the backgrounds, the kind of place you start at, Deploy through the vanilla loadout and spawn requests ([above](#the-spawn-screen)) |
| `SCR_BasePlayerLoadout`, `SCR_LoadoutManager`, `SCR_BaseGameMode` (modded, `RZ_LoadoutLock.c`) | server (the client only marks the loadout unavailable in vanilla's deploy menu) | Never assigns or spawns a locked background's loadout |
| `RZ_MapIconsUI` | client (map component) | The icon layer on the map, you-are-here, and the map key in its corner ([The map](#the-map)) |
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
screen, the shop screen, the crafting and cooking screens, the radio screen, the object menu and the power panel stop you moving while the world keeps running, so the server watches every open one, once a second on the game
mode's `SecondTick`, and closes it when: you took damage or fired; an infected is within
**Dialogue > Danger near** (6 m) of you or of the person you talk to, or one within **Danger
closing** (30 m) came 2 m closer since the last check; you died, or the person is gone; and, for
talk only, you're more than **Range** (6 m) from them or **Idle seconds** (120) passed with no
reply (the repair, shop, crafting and cooking screens, the object menu and the power panel the same, from the car, the quartermaster, the bench, the stove, the thing or the control, with no pick; the radio screen from a radio not on you, and never idle). In danger the person says "Not now. Something's coming. Get your gun up." as a subtitle. At most **Open at most** (32) screens
are open on the server; each check is a few distance tests. The client says when the player
closes one (`RpcAsk_ScreenClosed`).

**Turning it off.** Styled notices off gives the vanilla hints back. **Dialogue > Nametags** off
(or no `NametagPanel` in the HUD layout) takes the nametags away. Emptying **UI > Spawn row
layout** gives vanilla's deploy menu back. **UI > Death brief** off shows nothing at a death. Emptying **UI > Car job
layout** gives a fixable car its prompts and notices back, emptying **UI > Shop item layout**
a quartermaster its Buy actions, emptying **UI > Craft row layout** a workbench its Make actions,
emptying **UI > Cook row layout** a stove its Boil, Brew and Cook actions, emptying **UI > Radio
channel layout** takes **View radio** away, and
emptying **UI > Object row layout** every prop and place its world prompts, and emptying **UI > Power
breaker layout** the power station's control its old prompts. Taking
`RZ_PlayerUIComponent` off the player controller (`Prefabs/Characters/Core/RZ_PlayerController.et`)
stops all sending. To remove the UI entirely, delete the `UI/` code folder and the lines that use
it: the `m_UISettings` attribute, `m_UIFeed` (creation, tick, spawn,
`RecordGearSoon` and accessors), the `UsesMapIcons` check in `AddMapMarker`, the screen capture and the owner-only
branch at the top of `NotifyPlayer` and the block at the
top of `QueueHint` (with the chat commands' registration) in `Core/RZ_GameModeComponent.c`, the
board menu call in `World/RZ_NoticeBoard.c` (`SendMenu`), the `OnClueRead` call in
`Puzzles/RZ_PuzzleMission.c`, the repair screen section of `Interactions/RZ_Cars.c` (with its
calls in `Inspect`, `Hide`, `CompleteFit` and `CompleteHotwire`, and `OnTaskStopped` in
`RZ_Interactions.c`), the crafting screen section of `Interactions/RZ_Crafting.c` (with its calls
in `Look`, `UseStove`, the stove's and the bench's completions, and `OnTaskStopped` in `RZ_Interactions.c`), the `RZ_CarMenu` and
`RZ_CraftMenu` checks in `Puzzles/RZ_PuzzleActions.c`, `Factions/RZ_ShopScreen.c` with
`RZ_ShopOpenAction` and the `RZ_ShopMenu` checks in the quartermaster actions
(`Factions/RZ_Quartermaster.c`, `RZ_FactionServices.c`), and the object menu's `RZ_ObjectMenu.Available`
check in `RZ_ActionMenus` (`Interactions/RZ_ObjectScreen.c`: with it always false, every action is a prompt). `Interactions/RZ_RadioScreen.c` with `RZ_RadioMenu.c` (and the house radio's option in
`RZ_SecureRadio.HandleHouseRadio` and its label in `RZ_Interactions`, and the action on
`RZ_WindupRadio.et`) go together. `RZ_LoadoutLock.c`, `RZ_SpawnMenu.c` and `RZ_MapIcons.c` can each be deleted on their
own (without the first, a locked pick spawns and plays as the default background). Then remove what names the UI's classes: point `RZ_GameMode.et` back at the vanilla player
controller and clear its **UI** settings, and delete the two overrides in `Configs/System/` and
the layouts in `UI/layouts/ReforgerZ/`.

*Simplified:* notices are classified by their title, so an unusual title gets a plain card. The
map's icons don't cluster or scale with zoom; outposts a player has seen are remembered for the
session only.
Journal rewards are the mission's base Fame and Favor ("about"), before leader traits and
background multipliers. The radio channel list is a static part of its layout. The Standing page's
Next line works from the Fame and Infamy bands only: it doesn't know about the credit rule or
Infamy that may fade.

## Built and planned

| | |
|---|---|
| Built | The icon set and its renderer; twelve mockups and previews (with the talk screen, the reader, a subtitle, the car repair screen, the shop, the crafting screen, and the quest tracker with the Quests tab). In code: the feed, the subtitles with the speaker's name in faction colour, a source icon and, in person, an arrow and distance to them, the radio-line box, the HUD with notice cards paced by importance and subtitles, owner-only notices and spoken lines, the notebook (five tabs as pages, N, the pause menu button, the notebook item; the Quests tab with each job's objectives and Track, the per-row detail, the Journal's past jobs, the season page with the hall of fame), the quest tracker on the HUD (the tracked job only), one card at spawn and the quiet start, the talk screen and its engine (with the admins' errand view as a reply), the reader, the car repair screen, the shop screen, the crafting screen at workbenches and the cooking screen at stoves, the radio screen (View radio), the object menu on props and places with its status strip and locked rows (the strip shared with the cooking screen), the power panel at the power station, the screen guard, the notice board menu, chat commands, the spawn screen in place of vanilla's deploy menu (the backgrounds with the loadout's gear, the kind of place you start at, Deploy) and locked backgrounds refused by the server, the death brief over it at each death, the map's icon layer with you-are-here, the tracked job's objective (a diamond or a search area, no other job) and the map key on the map, styled notices with the vanilla fallback, each new card with vanilla's hint sound; the journal in each player's order (`RZ_MissionReasons.OrderScore`: near, started by a reason, from a leader they helped) and its finale page, one row while the routes are open (the readout, the route's worth and your route, as a desk's *How do things stand?*) |
| Workbench files | The layouts ([LAYOUTS.md](../UI/LAYOUTS.md)), the notebook's, talk screen's, reader's, car repair, shop, crafting, radio, object menu, power panel and spawn screens', the death brief's and the map key's written as text, with images on the static icons; the icon atlas and the `RZ_Icons` imageset; the car picture (`RZ_CarSide.edds`, set in **UI > Car picture**); the menu and input config overrides in `Configs/System/` (the notebook, notice board, reader, talk, car, shop, crafting, spawn screen, death brief, radio and power panel presets; N, 1–5, T, the right stick, the filter's X and the map key's I and Y); **Trade** on the four quartermaster prefabs; the notebook item `Prefabs/ReforgerZ/UI/RZ_Notebook.et`, written as text; the player controller `Prefabs/Characters/Core/RZ_PlayerController.et`, with the feed and the HUD; the game mode's **UI** settings, the spawn row, map legend, power breaker and season page included. What each holds is in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#setting-up-the-ui) |
| Works in play | Seen in Workbench play on the repo's world, with no script exceptions: the styled notice cards, the stealth visibility bars and the noise meter; on Everon, with the keyboard (pad not yet): the spawn screen (the 13 backgrounds, the panel, the start town picker and Deploy), the map key and the map's icons (in their old line-icon style); the notebook's five pages with their icons (the Radio page's channel list too) and the HUD's noise icon. In the layout preview at 1920 × 1080: the crafting, car repair, shop, reader, talk and notice board screens, each within its slots |
| Not checked in play | The map's design A markers (disc, ring, emblem, badge) and the key's discs and badges; the objective's diamond and search area, and their row in the key; the infection timer and the secure-radio indicator (they need a bite or a cracked channel); notice pacing, the spawn card and the quiet start; the quest tracker, the Quests tab, its groups and Track (A, Y and T); the tabs' new order and the Journal's past jobs; the notebook's pause menu button and the notebook item; the talk screen; the reader; the car repair screen and its picture; the shop screen and its item picture; the crafting screen; the cooking screen (**UI > Cook row layout** isn't set yet); the radio screen (**UI > Radio channel layout** isn't set yet, and **View radio** in hand needs vanilla's inspection checked); the object menu (its preset and **UI > Object row layout** aren't set yet); the power panel (its preset and **UI > Power breaker layout** are set); the death brief; the subtitles; mouse clicks and the pad (the spawn screen and the map key with a pad too); the notice board menu and its ready-made notes; the chat commands; the season page (**UI > Season page** is set) |
| Planned | Mockups of the notice board, the object menu, the radio screen and the season page; the Journal's mockup with its past jobs; the lock card as a screen of its own (today a notice, or the [Point of no return] node with **Dialogue > Trees** on: [FINALE.md](FINALE.md#the-lock)) |
