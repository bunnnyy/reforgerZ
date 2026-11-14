# The map's fog

**Designed, not built.** Approved by the author on 2026-11-14 ("fog of war", and build it now).

The map shows only what this character has seen. In a sealed Zone you know the streets you walked
and the roads someone told you about; the rest is grey. It gives the long walk a reward and the
landmarks, heirlooms and tonight's list a map to fill.

## What you see

| Part | How |
|---|---|
| **The fog** | Unseen ground is under a dark grey fog. Roads and the coast still show faintly through it, so you're never lost |
| **Walking** | Reveals about 150 m round you as you go |
| **High ground** | On a tower or a roof (6 m above the ground under you), or on a hill well above the land round it (35 m), you see about 400 m |
| **A found map** | A paper map found as loot in a town reveals that town and the land round it (500 m) once you carry it. The map every life starts with doesn't count |
| **A notice board** | Reading a town's board reveals that town the same way |
| **Asking about the roads** | The Haven's service (Known) shows the way from where you stand to the job you follow: a strip along the straight line |
| **Friends' camps** | A faction you're Known with shares the ground round its camps (300 m): its holdings, and for the Haven its safehouses |

Seen ground stays seen.

## Markers

The mod's own places (camps, the lab, the stations, the checkpoints) show as a pin once their
ground is seen. Before that, the places everyone knows of show as a rumour: a rough circle about
350 m across, somewhere round the place, never on it, labelled "... somewhere here". The tracked
objective always shows, as today. The packs' rings stay as they are (they're already rumours).

## Whose it is

The fog belongs to the player's profile, so it is kept across deaths today, and per character once
characters last (designs/PERSISTENCE.md) with one change in one place. A season's wipe starts a
fresh state, so the fog starts fresh. A character reset (persistence) clears it.

## How it works

- **The server keeps it:** for each profile, one bit per 100 m square of the map (128 × 128 on
  Everon: about 2 KB), saved with the profile.
- **Cheap:** a player's reveal is worked out only when they enter a new square (and the high-ground
  look every 10 s while they stand still); camps are shared every 30 s; found maps are looked for
  every 5 s.
- **Sent as runs:** the owning client gets the whole fog as run lengths (a few hundred numbers)
  when the map opens and at most every 3 s while it changes.
- **Drawn on the client:** an overlay on the game's own map (the icon layer's canvas), unseen
  squares merged into rectangles, redrawn only when the map moves, zooms or the fog changes.
- **Settings:** Map > Fog (on, radii, square size, how dark). Off: today's map.
- **Admin:** `/fog all`, `/fog reset`, and the talk screen's [Admin] rows, for a pad.

## Later (planned)

- **Selling a survey:** an explored town's survey sold to the Haven or Bastion for Favor (capped).
- **Sharing maps:** copy your map onto another player's by talking, or hand over a marked paper map.
- **A notebook page** on the first visit to a place, pointing onward.
- **Roads, not lines:** asking about the roads and a found map follow the real roads, not a strip
  and a disc.

## Order of work

1. The fog, walking and high ground, the payload, the drawing, the settings, the admin tools.
2. Found maps, boards, asking about the roads, friends' camps; markers as rumours.
3. The later parts above.
