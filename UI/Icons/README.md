# ReforgerZ icons

The icon set for the ReforgerZ UI: 88 icons. Art direction, colours and how the icons are used in the game are in [docs/UI.md](../../docs/UI.md).

- **Sources:** `src/<name>.svg`, hand-written, 64×64 viewBox, one colour (`currentColor`), 4-unit strokes, transparent background.
- **Mono exports:** `png/64/` and `png/128/`, white on transparent so the game can tint them (`ImageWidget.SetColor`).
- **Colour variants:** `png/color/64/` and `png/color/128/`, for the map and the faction emblems. Plated map markers get a dark backing behind the cut-out glyph. Stronghold, outpost and quartermaster markers come in one colour per faction (`_haven`, `_bastion`, `_redhand`, `_halcyon`).
- **Contact sheet:** [`UI/Previews/icons_sheet.png`](../Previews/icons_sheet.png).
- **Re-render** after editing a source: `node tools/render_ui.js icons`. `icons.json` lists every icon (name, category, use); add new ones there too.

Importing them into Workbench (`.edds` and an imageset) is in [UI/LAYOUTS.md](../LAYOUTS.md#importing-the-icons).

## Faction emblems (6)

| Icon | Name | Use |
|---|---|---|
| <img src="src/faction_haven.svg" width="32"> | `faction_haven` | The Haven: the kids' bear, patched, one eye stitched shut, as stencilled on safehouse doors. Standing screen, notice cards, the journal |
| <img src="src/faction_bastion.svg" width="32"> | `faction_bastion` | Task Force Bastion: a helmet with the days without relief chalked on as a tally. Standing screen, notice cards |
| <img src="src/faction_redhand.svg" width="32"> | `faction_redhand` | The Red Hand: a red handprint slapped on at an angle, still running. Standing screen, notice cards |
| <img src="src/faction_halcyon.svg" width="32"> | `faction_halcyon` | Halcyon: the plague doctor ("the beaks"), hat, goggle and long beak. Standing screen, notice cards, the journal |
| <img src="src/faction_infected.svg" width="32"> | `faction_infected` | The infected and the outbreak: a virion with a bite taken out of it. Outbreak stage, infected markers, hazard signage |
| <img src="src/faction_longtable.svg" width="32"> | `faction_longtable` | The Long Table (not a faction, and a secret): a preserving jar labelled only "9". Its emblem in the set only: no screen shows it, as the Long Table is told by absence. Never on the map or the standing screen |

## Standing tiers (8)

| Icon | Name | Use |
|---|---|---|
| <img src="src/tier_stranger.svg" width="32"> | `tier_stranger` | Stranger: they don't know you. An empty tag |
| <img src="src/tier_known.svg" width="32"> | `tier_known` | Known: one mark on the tag |
| <img src="src/tier_trusted.svg" width="32"> | `tier_trusted` | Trusted: two marks |
| <img src="src/tier_champion.svg" width="32"> | `tier_champion` | Champion: a solid tag with a star cut through |
| <img src="src/tier_watched.svg" width="32"> | `tier_watched` | Watched: they have an eye on you |
| <img src="src/tier_hunted.svg" width="32"> | `tier_hunted` | Hunted: you're in their sights; hunter squads |
| <img src="src/tier_kill_on_sight.svg" width="32"> | `tier_kill_on_sight` | Kill on Sight: a crossed-out target; public bounty |
| <img src="src/tier_mixed.svg" width="32"> | `tier_mixed` | Uneasy, Tolerated, Respected and Wildcard: a tag half one way, half the other |

## Mission types (7)

| Icon | Name | Use |
|---|---|---|
| <img src="src/mission_contract.svg" width="32"> | `mission_contract` | A faction contract: a page with a stamp |
| <img src="src/mission_event.svg" width="32"> | `mission_event` | A world event, open to everyone |
| <img src="src/mission_puzzle.svg" width="32"> | `mission_puzzle` | A puzzle mission: look closer, read the clues |
| <img src="src/mission_leader.svg" width="32"> | `mission_leader` | A leader's personal job: someone wants a word |
| <img src="src/mission_unarmed.svg" width="32"> | `mission_unarmed` | An unarmed mission: no guns, noise and wits |
| <img src="src/mission_errand.svg" width="32"> | `mission_errand` | An errand you hold: a small parcel tied with string, someone's small ask. The journal's errand row |
| <img src="src/mission_perimeter.svg" width="32"> | `mission_perimeter` | A Perimeter mission: the drone line at the edge of the Zone |
| <img src="src/mission_arc.svg" width="32"> | `mission_arc` | A story arc mission: one chapter of a longer story |

## Map markers (18)

The plated `marker_*` icons name each kind of place in the map's feed. The map itself draws a place as a disc: `marker_ring` (near-black), `marker_disc` (the owner's colour), and on it the faction's emblem or, for a place without a faction, its `glyph_*` ([docs/UI.md](../../docs/UI.md#the-map)).

| Icon | Name | Use |
|---|---|---|
| <img src="src/marker_lab.svg" width="32"> | `marker_lab` | Halcyon's lab (hexagon plate) |
| <img src="src/marker_safehouse.svg" width="32"> | `marker_safehouse` | A Haven safehouse |
| <img src="src/marker_stronghold.svg" width="32"> | `marker_stronghold` | A faction's stronghold (never falls) |
| <img src="src/marker_outpost.svg" width="32"> | `marker_outpost` | A faction outpost (can change hands) |
| <img src="src/marker_event.svg" width="32"> | `marker_event` | A world event on the map (diamond plate) |
| <img src="src/marker_quartermaster.svg" width="32"> | `marker_quartermaster` | A quartermaster and the leader who runs the place |
| <img src="src/marker_power.svg" width="32"> | `marker_power` | The power station |
| <img src="src/marker_radio.svg" width="32"> | `marker_radio` | The radio station |
| <img src="src/marker_checkpoint.svg" width="32"> | `marker_checkpoint` | A sealed Perimeter road checkpoint (octagon plate) |
| <img src="src/marker_range.svg" width="32"> | `marker_range` | A pack's ground: a broken ring with three claw marks, no plate. The map sizes it to the ground; the journal's pack rows |
| <img src="src/marker_self.svg" width="32"> | `marker_self` | You are here: a heading arrow, turned to where you face, no plate. The map's own marker (on `marker_self_back`) and the HUD tracker's arrow |
| <img src="src/marker_self_back.svg" width="32"> | `marker_self_back` | The disc behind you-are-here on the map, tinted dark so the arrow reads on any ground |
| <img src="src/marker_disc.svg" width="32"> | `marker_disc` | A solid disc: a map marker's middle, tinted in its owner's colour, and the dark chip under a place badge |
| <img src="src/marker_ring.svg" width="32"> | `marker_ring` | A thin hollow ring round a map marker's disc, with a small gap, tinted near-black |
| <img src="src/glyph_power.svg" width="32"> | `glyph_power` | The power station's bolt, no plate: bone on its steel-grey map disc |
| <img src="src/glyph_radio.svg" width="32"> | `glyph_radio` | The radio station's mast, no plate: bone on its steel-grey map disc |
| <img src="src/glyph_event.svg" width="32"> | `glyph_event` | A world event's exclamation mark, no plate: dark on its hazard-yellow map disc |
| <img src="src/glyph_checkpoint.svg" width="32"> | `glyph_checkpoint` | A sealed checkpoint's striped barrier, no plate: dark on its hazard-yellow map disc |

## Map badges (4)

What a faction's place is: a tiny glyph in a ring, drawn in bone on a dark chip (`marker_disc`) at a map marker's bottom right, and in the map key.

| Icon | Name | Use |
|---|---|---|
| <img src="src/badge_stronghold.svg" width="32"> | `badge_stronghold` | A stronghold: a small keep |
| <img src="src/badge_outpost.svg" width="32"> | `badge_outpost` | An outpost: a flag |
| <img src="src/badge_safehouse.svg" width="32"> | `badge_safehouse` | A safehouse: a house |
| <img src="src/badge_lab.svg" width="32"> | `badge_lab` | The lab: a flask |

## Items (18)

| Icon | Name | Use |
|---|---|---|
| <img src="src/item_key.svg" width="32"> | `item_key` | A key for a real lock (tagged keys) |
| <img src="src/item_sample.svg" width="32"> | `item_sample` | A cure sample for Halcyon |
| <img src="src/item_antidote.svg" width="32"> | `item_antidote` | An antidote: cures the infection |
| <img src="src/item_stabiliser.svg" width="32"> | `item_stabiliser` | A stabiliser: pauses the infection for 20 minutes |
| <img src="src/item_med.svg" width="32"> | `item_med` | A low-grade med (antibiotics, antiviral, cut dose, suppressant, remedy): buys time |
| <img src="src/item_noisemaker.svg" width="32"> | `item_noisemaker` | A noisemaker (wound alarm clock, tin rattle): pulls the infected toward it |
| <img src="src/item_battery.svg" width="32"> | `item_battery` | Car part: a battery |
| <img src="src/item_tyre.svg" width="32"> | `item_tyre` | Car part: a tyre |
| <img src="src/item_sparkplugs.svg" width="32"> | `item_sparkplugs` | Car part: spark plugs |
| <img src="src/item_fuelcan.svg" width="32"> | `item_fuelcan` | Car part: a fuel can (jerrycan), full or empty |
| <img src="src/item_radio.svg" width="32"> | `item_radio` | The wind-up radio every new life starts with |
| <img src="src/item_molotov.svg" width="32"> | `item_molotov` | Crafted: a molotov |
| <img src="src/item_bandage.svg" width="32"> | `item_bandage` | Crafted: an improvised bandage |
| <img src="src/item_tripwire.svg" width="32"> | `item_tripwire` | Crafted: a tin-can tripwire kit |
| <img src="src/item_spiketrap.svg" width="32"> | `item_spiketrap` | Crafted: a spike trap kit |
| <img src="src/item_tablets.svg" width="32"> | `item_tablets` | Crafted: purification tablets (clean water) |
| <img src="src/item_planks.svg" width="32"> | `item_planks` | Planks: barricades and spike traps |
| <img src="src/item_jewellery.svg" width="32"> | `item_jewellery` | Jewellery (the wedding and other finds) |

## Status and HUD (13)

| Icon | Name | Use |
|---|---|---|
| <img src="src/status_bitten.svg" width="32"> | `status_bitten` | Bitten: the bite mark (golden window, then the timer) |
| <img src="src/status_golden.svg" width="32"> | `status_golden` | The golden window: minutes to clean the bite before it takes |
| <img src="src/status_fever.svg" width="32"> | `status_fever` | Infection stage 1: Fever |
| <img src="src/status_weakness.svg" width="32"> | `status_weakness` | Infection stage 2: Weakness (out of breath, stamina drains) |
| <img src="src/status_final.svg" width="32"> | `status_final` | Infection stage 3: the Final stage (minutes left) |
| <img src="src/status_secure_radio.svg" width="32"> | `status_secure_radio` | Listening on a cracked secure channel |
| <img src="src/status_radio_open.svg" width="32"> | `status_radio_open` | An open channel on the radio (faction chatter, the Haven, Bastion net); the radio-line box and a radio voice's subtitle |
| <img src="src/status_broadcast.svg" width="32"> | `status_broadcast` | The government broadcast (and loudspeakers: a loudspeaker's subtitle) |
| <img src="src/status_noise.svg" width="32"> | `status_noise` | Noise and heat: how much attention you're drawing |
| <img src="src/status_shortage.svg" width="32"> | `status_shortage` | A shortage: short of food or supplies (prices up) |
| <img src="src/status_door_locked.svg" width="32"> | `status_door_locked` | A locked door (needs its key, or force it) |
| <img src="src/status_door_unlocked.svg" width="32"> | `status_door_unlocked` | An unlocked door (you have the key, or it was opened) |
| <img src="src/status_door_forced.svg" width="32"> | `status_door_forced` | A forced door (crowbar: noise, and the lock is broken) |

## Standing and needs (9)

The Standing page's columns and blocks, and what a faction needs. The other two needs use icons above: supplies `status_shortage`, samples `item_sample`.

| Icon | Name | Use |
|---|---|---|
| <img src="src/standing_favor.svg" width="32"> | `standing_favor` | Favor: a stamped coin. The Standing page's Favor column |
| <img src="src/standing_leader.svg" width="32"> | `standing_leader` | A leader you have met: a bust with rank chevrons. The Standing page's leaders column and its *Leaders you've met* block |
| <img src="src/standing_kit.svg" width="32"> | `standing_kit` | Their kit: a jacket. The Standing page's kit column (tinted by what they make of you in it) and its *In their kit* block |
| <img src="src/standing_next.svg" width="32"> | `standing_next` | The next name up: two chevrons. The Standing page's *Next* line |
| <img src="src/standing_word.svg" width="32"> | `standing_word` | How they see you: a speech bubble with quote marks. The Standing page's *How they see you* block; someone speaking in person, before their name in a subtitle |
| <img src="src/need_food.svg" width="32"> | `need_food` | A faction needs food: a tin can |
| <img src="src/need_people.svg" width="32"> | `need_people` | A faction needs people: two figures |
| <img src="src/need_ammo.svg" width="32"> | `need_ammo` | A faction needs security: three rounds |
| <img src="src/need_revenge.svg" width="32"> | `need_revenge` | A faction wants revenge: a dagger |

## Voices (1)

How a spoken line reaches you, before the speaker's name in a subtitle ([docs/UI.md](../../docs/UI.md#subtitles-and-the-reader)). In person is `standing_word`, a radio channel `status_radio_open` (or `status_secure_radio`), a loudspeaker `status_broadcast`; this is the fourth.

| Icon | Name | Use |
|---|---|---|
| <img src="src/voice_note.svg" width="32"> | `voice_note` | A note, sign or board read out: a sheet with a turned corner and a hand's lines |

## Objectives (3)

The marks before a job's objectives on the quest tracker and in the Quests tab, New Vegas style.

| Icon | Name | Use |
|---|---|---|
| <img src="src/track_current.svg" width="32"> | `track_current` | The current objective: a filled diamond (hazard yellow) |
| <img src="src/track_next.svg" width="32"> | `track_next` | The next objective you know of, or an optional one: an open diamond (dim) |
| <img src="src/track_done.svg" width="32"> | `track_done` | An objective done: a tick, beside the struck-through line (the Quests tab) |
