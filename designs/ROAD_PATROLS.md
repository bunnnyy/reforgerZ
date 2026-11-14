# Road patrols

**Designed, not built.** Approved by the author on 2026-11-14 ("build it, but not too many car
patrols"). Today a site sends its people only a short way out ([SITES.md](../docs/SITES.md#patrols):
the wire, the approach, the threat side, the courier, the road post run). Nobody moves along the roads
around a faction's ground, so a highway between two holdings is empty except for a supply truck.

## What it adds

| | What | Who |
|---|---|---|
| **Foot patrols** | 3–5 people walk out from a holding along the real road network, 1–3 km, and back the way they came | Bastion: a fire team in step, checking papers. The Red Hand: a crew looking for anyone to charge. The Haven: a scavenging party with sacks. Halcyon: a sample team in hazmat with one armed guard |
| **Vehicle patrols, rare** | Bastion's jeep or the Red Hand's pickup drives the main road between two of its holdings and back | At most **one on the whole server at a time**, and each faction no more than once every 45–60 minutes of play |

## How they run

- **Data until someone's near.** A patrol is a point moving along its road points at a walk (a vehicle
  at a road speed). Its people (and the vehicle) appear only while a player is within 400 m, and go
  when everyone is past 500 m. With a body the AI walks its remaining points; its position is read
  back.
- **How many.** A few at once on the server, more with more players online (two, plus one per four
  players, at most five). A faction leaves a holding only now and then (every 20–35 minutes of play),
  fewer when its strength or morale is low this season. The Haven and Halcyon don't go out at night.
- **Routes.** From the road point nearest the holding, along connected roads, turning at junctions,
  until the walk is 1–3 km long; never into water, never near the line, never through another
  faction's holding. A holding with no road within 300 m sends nobody.

## Meeting one

What they do goes by your standing with their faction ([REPUTATION.md](../docs/REPUTATION.md)), said by
their leader (rule 29 of [MISSION_RULES.md](../docs/MISSION_RULES.md)) and talked through on the talk
screen:

| Standing | What happens |
|---|---|
| **Known or better** | A nod and road news. Now and then a small ask: walk the next stretch with them (a little Fame, once a patrol) |
| **Below Known, not Hunted** | Bastion: papers (in their kit it's a look, as at a road post). The Red Hand: the road's toll, as at their posts. The Haven and Halcyon: questions, and they go on |
| **Hunted** | They open fire (the hostility rules as built) |

## What they leave

- **A fight on the road.** A patrol that meets the infected on its way leaves spent cases on the verge
  where it happened, a live trace that ages out ([TRACES.md](../docs/TRACES.md#live-traces)).
- **What others hear.** An up mast's warnings in its region say a patrol is due on a road; overheard
  traffic names one (the masts, [DESIGN.md](../docs/DESIGN.md#the-masts)). A player can use that to
  keep clear of one, or to wait for it.
- **One that doesn't come back.** A patrol wiped out on the road is the faction's Missing Patrol
  ([MISSIONS.md](../docs/MISSIONS.md)) when that mission can start there.

## Hooks

Tonight's list ([TONIGHT.md](TONIGHT.md)) can name a patrol due near you once it's built.
