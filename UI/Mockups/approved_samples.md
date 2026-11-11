# Screens the author approved on 2026-11-11 (inline samples, 680 px wide)

The author's words: car repair "is perfect, keep that" (job card + rough side view of the car,
short bonnet); talk, subtitles and reader "perfect", with "the text a bit easier to read, like what
they are saying" (the readable versions below); the quartermaster shop screen was shown and asked
for ("rather than the 100 scroll items in interact"). Palette from docs/UI.md: panel #121416,
deeper panel #0f1113, bone #E8E2D0, bright bone #F4F0E6 for spoken text, dim #A39E90, faint
#6d6a62, lines #22262a / #2e3236, Haven amber #E3A33B (accent and pad-button rings #6b5222),
Bastion olive #9AA35E (done), Red Hand red #C8352B, Halcyon teal #4FB8B0, hazard #F2C230.
Pad prompts are small rounded keys (A, B, X, Y, LB/RB) in the accent colour.

## 1. Car repair (job card with the car)

Header: "The blue two-door" left, "2 of 5 done" right (dim). Under it a line drawing of a small car
facing LEFT (clear front: short bonnet with a pale headlight, raked windscreen, two side windows
with a B-pillar, steeper rear window, short boot with a red tail light, bumpers as short lines,
wheel arches; the missing front tyre drawn as a dashed circle, the rear wheel solid). Five numbered
markers sit on the car where each job is: 1 spark plugs (front of the bonnet, green = done),
2 battery (bonnet near the windscreen, amber = can do now, with an amber ring = selected),
3 front tyre (on the dashed wheel, grey outline = you need a part), 4 fuel (rear quarter above the
rear wheel, amber), 5 ignition (behind the windscreen, amber). A small "front" label at the left.
Below: the numbered job list, one row each, same numbers and colours: "1 Spark plugs · Fitted";
"2 Battery · in your bag · Fitting… 12 s" with a thin amber progress bar; "3 Front tyre · none on
you"; "4 Fuel · jerrycan in hand · [A] Pour"; "5 Ignition · no keys · [X] Hotwire". Footer: "Up/down
picks · A does it · B closes" left, and a disabled "[Y] Start the engine" button right.
Selecting a row moves the ring to its marker on the car.

SVG of the car (viewBox 0 0 360 120):
<path d="M52 88 L50 76 Q50 68 60 66 L118 58 L150 35 Q154 32 160 32 L226 32 Q232 32 236 36 L264 56 L304 60 Q312 61 312 70 L312 86 L306 88 L282 88 A20 20 0 0 0 242 88 L112 88 A20 20 0 0 0 72 88 Z" fill="#171c1f" stroke="#7b8790" stroke-width="2"/>
windows: M124 58 L152 37 L194 37 L194 57 Z and M200 37 L229 37 L255 56 L200 57 Z (stroke #56626a);
pillars x=197 and x=122 from the window line to y=86; mirror M120 56 L113 52 L113 58 Z;
headlight ellipse at (56,71) rx 5 ry 4 #E8E2D0; tail light rect at (308,64) 4x9 #C8352B;
front wheel dashed circle (92,90) r16; rear wheel circle (262,90) r16 with hub r6; ground line y=107.
Markers: 1 (76,55), 2 (102,51), 3 (92,90), 4 (286,70), 5 (160,49), r=9, ring r=13.

## 2. Talk (dialogue)

The world dimmed above. A panel across the bottom (deeper panel, a 3 px top border in the person's
faction colour): the name in the faction colour (15 px, bold) then "The Haven · council head ·
trusts you" (dim, 12 px); their line in bright bone at 18 px, line height 1.5, in quotes ("You're
back. Good. The well in Beauregard has gone quiet, and quiet worries me more than screaming.");
then the replies as rows at 14 px, each with a pad key number (1, 2, 3...), the selected one with a
lighter background and a 3 px accent bar on the left; a reply may carry a dim tag on the right
("a job · Favor", "trade"); the last reply is always a way out ("I'll be going.").

## 3. Subtitles

In play, at the bottom centre (max 80% wide), on a solid dark backing (rgba(12,14,15,.88), 4 px
radius): the speaker's name on its own line in their faction colour (13 px, bold), then what they
say in bright bone at 18 px, line height 1.45 ("Stop there. Hands where I can see them."). The notice
cards stay where they are (top right), so the two never mix.

## 4. Reader (notes, letters, books)

The world dimmed (rgba(10,12,13,.72)); a sheet of paper in the middle (#EDE7D6, ink #1c1a16), about
half the screen's height or more: the title (17 px, bold: "Note on the dashboard"), a dim line on the
paper (#5f5c55, 12 px: "Folded twice, in pencil"), the text at 16 px, line height 1.6; at the foot
"1 / 1" left (pages) and "B close" right. You can't move while it's open.

## 5. Quartermaster's shop

Header: "Haven stores · Ada Reyes, quartermaster in Morton" left; right "You're Trusted · 64 Favor"
(Favor in amber); a 2 px accent line under the header. Category tabs (LB/RB): Food and water,
Medical, Gear and tools, Weapons and ammo, Services (no vehicles, no car parts). Left: the items
(icon, name, price "4 Favor" in amber), the selected row lighter with an accent bar; items above
your standing are dim with a lock and the standing they need ("Champion"). Right: a details panel,
topped by a picture of the selected item (the author asked for it: "show an image of the thing
you're buying"): a dark box (#0f1113, 1 px #22262a border, about 130 px tall in the 680 px sample)
with a slightly lighter floor strip at the bottom and the item in the middle, labelled "3D preview"
in the corner; in game it's vanilla's item preview of the prefab (as the inventory draws items),
turnable with the right stick or the mouse; a locked item's picture is darkened. Then the details
(name, a short description with the weight, the price and "you have 64", and an "[A] Buy" button, or
"Locked" with "Needs Champion with the Haven"). Footer: "LB/RB category · up/down item · A buy ·
B close · prices in Haven Favor; locked items show the standing they need".
Sample stock: canned food 4, bottle of water 3, army crackers 2, bandage 3, fever tablets 8, field
surgery kit 30 (Champion), backpack 15, hand radio 12, full jerrycan 10, crowbar 6, fortification
kit 30 (Champion), hunting rifle 40, box of rifle rounds 8, rest at the safehouse 5 (no time skip).
