# Openings: the rest

**Designed, not built.** Approved by the author on 2026-11-12. Phase 1 is built and lives in
[DESIGN.md](../docs/DESIGN.md#openings-how-every-life-starts): the place roll with no start picker,
the hiding place, someone from before, one chain shape for every background, and where it ends by
background. What follows is the rest. When it's built, it moves into DESIGN.md ("Openings"),
[DIALOGUE.md](../docs/DIALOGUE.md) (the conversations) and [WRITING.md](../docs/WRITING.md), and
this file is deleted.

## Why today: the other two reasons

Two reasons besides the hiding place, for variety (the hiding place keeps 70%):

| Reason | Weight | The scene | The contact |
|---|---|---|---|
| **Your group is gone** | 18% | Bedrolls for four, one of them yours, a map with a circled town, a note: "Back by dark." Three days of dark | Someone who saw them go, or the one who came back |
| **The fever broke** | 12% | A barricaded room, a bucket, a bite on your arm that's only a scar now. On the door: "If you're reading this, you're you. Come find us." | Whoever wrote it, waiting outside |

**Other contacts.** Someone from before in about three of four openings; otherwise a stranger who
heard the door.

**The earlier-life contact.** Later lives can bring back someone you met in an earlier life (a
wanderer or occupant who remembers your old name, [memory](../docs/DIALOGUE.md#memory)): "You look
like someone I knew."

## Per background: the chain's step 2

Each background gets its own system as step 2:

| Background | Step 2 (the system) |
|---|---|
| Nurse | treat someone (treatments, infection); boil water if the pharmacy was empty |
| Mechanic | get a car to start, or the hand pump (cars, fuel) |
| Hunter | track and avoid a pack, quietly (stealth, tribes) |
| Police Officer | open a locked door with the station's keys (locks, keys) |
| Ex-Con | get through a Red Hand toll without a fight (standing, disguise) |
| Lab Technician | take a sample from a dead special (samples, the cure) |
| Radio Operator | find the open channel and answer (radio, channels) |
| Unemployed | trade with a wanderer (Favor, trading) |
| Soldier | hold the gate for a minute (fights, the infected's heat) |
| Doctor | stabilise someone bitten (antidotes, the clock) |
| Raider | collect a debt (Infamy, the Red Hand's ways) |
| Elder | settle an argument between two families (talk, checks) |
| Drifter | three people, three small asks (a sample of everything) |

The design's example, for a Nurse: *"My boy's burning. There's a pharmacy two streets over. They'll
hear you if you run."* Search the pharmacy; treat the boy with what you found; walk them both to the
safehouse in the next town, where the cook knows the mother. Halcyon's flyer can lie in the pharmacy
(the cure beat).

## By experience

| Level | Opening |
|---|---|
| 0, brand new | The full opening and all three steps; brand-new players lean to the hiding place |
| 1, learning | The full opening; step 1 may be shorter |
| 2, familiar | The scene and the contact; one step, then the tracker picks the nearest job |
| 3, veteran | The scene as a one-line read and a tracked nearby job; a contact only one time in three |

## The scenes

- Compositions in `tools/camp_kit/pieces.py`, three per reason, plus per-background props, built as
  the camp pieces are.
- The six spawn scenarios' clues come back as props and lines inside the scenes (the ditch car, the
  basement door, the church flyer, the farmhouse radio, the attic raider, the dead soldier), and the
  scenarios themselves go.

## Lines

Lines for every background × reason, and a voice pass ([WRITING.md](../docs/WRITING.md)).

## Build order

2. Per-background steps 2 and the scenes' per-background props.
3. Group gone and fever broke, the earlier-life contact, and the experience levels.
4. Lines for every background × reason, and the voice pass.
