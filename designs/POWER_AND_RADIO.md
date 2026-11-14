# Power and radio: the masts, the grid and the loop

**Designed, not built.** Approved by the author on 2026-11-14: every power and radio idea, with the
relay network reworked so it isn't "more radio range" ("just expanding the radio kind of sucks, not
worth the effort"). It builds on what's built: one power station a map
([DESIGN.md](../docs/DESIGN.md#the-power-grid)), one radio station
([DESIGN.md](../docs/DESIGN.md#the-radio-station)), radios open and secure, the map's fog
([UI.md](../docs/UI.md#the-maps-fog)), the Distress Call ([MISSIONS.md](../docs/MISSIONS.md)) and
Await Relief ([QUESTLINES.md](../docs/QUESTLINES.md#await-relief)).

The rules it keeps: the Zone is sealed (no radio reaches past the line, nobody outside ever
answers); nothing is overpowered (every effect small, capped, and lost when the mast or the power
goes); bodies and props only near players; any map; a pad and a TV; games played in the world;
people say things ([MISSION_RULES.md](../docs/MISSION_RULES.md), rule 29); everyone met is a man.

## A. The masts

### What a mast is

Three to five of the map's own masts and aerials on high ground, each over a region (2.5 km round
it). Each is **a place with a story and an owner**, not a range number. Down, it's a dead aerial
with a tale at its foot. Up, it's a voice over its region, and **whoever puts it up decides whose
voice it is**. Factions want them; players bargain them; a rival can cut one.

| | |
|---|---|
| Where | the map's radio masts and aerials (by prefab name, the radio station's finder), on dry mainland, off the line, 1.5 km apart, never the radio station's own; too few, the highest ground by a mid-sized town; three at least, five at most |
| Its name | for what's there and the town below: "the TV mast above Levie", "the relay mast above Morton" |
| Its foot | one prompt, **The mast**, on the cabinet at its foot (the map's own cabinet when one stands within 25 m, else the console prop the radio station uses), which opens the object menu |

### Putting one up: a small expedition

Each mast is missing **one part**, a different one each (dealt from the mast's seed, the same all
season), and has **one trouble** at its foot. The tag on the cabinet says what's wrong in plain
words ("Feed cable cut. Somebody took the battery.").

| Part | What you carry up |
|---|---|
| A battery | a car battery (the cars' part) or a big battery |
| Wire | a coil of wire (the crafting junk) |
| A radio set | a portable radio, its valves for the relay |
| Cells | torch cells, two of them, for the beacon |
| Fuel | a jerrycan, for the mast's own generator |

| Trouble | What it is | Its ways |
|---|---|---|
| **A man lives there** | an old radio man in the hut at its foot, who's kept it in his head all season | He wants one thing first (two tins, something medical, or fuel), given by a reply on his talk screen. He'll ask you to keep it open; keep his word and he stays, and tells you things. |
| **The crew holds it** | a Red Hand crewman by the cabinet, collecting for the book | Pay him (two tins), or, Known with the Red Hand, he waves you through. Or promise him it's the Red Hand's when it's up: the promise binds (it's sold when it's up). |
| **A pack lies below** | a tribe's lair within 800 m | Nothing to talk round: climbing makes noise (heat at the foot), and at night the pack comes. Go by day, go quiet. |
| **Nothing** | an empty hill and the wind | Just the climb. |

**The climb.** With the trouble settled and the part on you: **Climb and fit the part**, a 25 s hold
at the foot that takes the part when it's done. It makes a little heat. The mast is up.

### Who it serves

At the top of the menu, once it's up, the one who put it up (or anyone, while nobody has said) picks:

| Choice | What it does in play |
|---|---|
| **Hand it to the Haven, Bastion or Halcyon** | It's that faction's mast. Their people in the region talk about you (a deed: "put the mast above Levie up for us"); you get a little Fame with them, once a mast a season; their open channel's lines in the region come through it. Its warnings go to those Known with them. |
| **Keep it open** | Everyone's mast. Everyone with a radio in its region hears its **warnings** as spoken radio lines: a pack moving near, a truck due, a storm. The old radio man, if there is one, stays. |
| **Sell it to the Red Hand** | Favor with the Red Hand. Its warnings go to those Known with them; anyone else in the region hears the crew's offer to listen in for a price. |

A mast held by a faction is that faction's **foothold** in the region: small and capped (its people
mention it, its warnings, the deed). A rival can **cut it** (phase B's sabotage): then it's down
again, with a new part missing, and the faction that held it offers a job to put it back.

### What a mast up gives the region

| | |
|---|---|
| **The land** | Whoever puts it up sees the land round it: the map's fog cleared for 1 km. |
| **Warnings** | (open, or to its faction's people) a spoken radio line now and then: a pack moving within 1 km of someone in the region, a truck due through it, a storm. |
| **Listening in** | With a radio in an up mast's region, now and then you overhear a faction's working traffic: a truck's route, a patrol's change, a toll's take. Something to act on or sell. |
| **The perimeter's chatter** | Near a mast within 3 km of the line, sometimes the drone operators outside: dry, bored, never answering. Flavour. |
| **Finding a voice** | With two masts up, now and then a faint voice on the band: a trapped man with a transmitter. Take a bearing at two masts (a 10 s hold at each, **Take a bearing on the voice**); two bearings fix the spot, and a Distress Call starts there, a house nobody would have looked in. |

**All masts up** opens phase C's loop's relay.

## B. The grid

| Idea | How it plays |
|---|---|
| **Substations, region by region** | The station feeds the substations, not the towns. Each of the map's transformer yards near a town is switched on in person (its own small panel: a breaker, a fuse to fit), so lighting the map is a chain of trips. |
| **What the factions want from it** | Halcyon's research runs faster while the lab's region is lit (×1.2, capped). The Haven's cold room keeps samples and medicine longer. Bastion's radios reach their posts. The Red Hand can cut a town's line and charge to switch it back: errands and leader jobs come from each. |
| **Lines that break** | Storms and the infected break the overhead line between a substation and a town. Walk the line to find the break (a sagging wire at a pole) and splice it with wire and pliers: a quiet job in the open at night. |
| **Generators you carry** | A portable generator (a farm's, a garage's) on a car's back lights one building: a camp, a locker, a workshop. It hums: heat. |
| **Lit towns at night** | The infected shy from lit streets (a little), but the light draws packs from further away. |
| **Sabotage** | A contract to cut a rival's substation or mast: their region goes dark, at a price in Infamy. |

## C. The loop's relay and Bastion's last call

| Idea | How it plays |
|---|---|
| **The loop's relay** | With every mast up, the one that carries the government loop can be reached. Cut it: "Aid is on its way" goes silent for the whole server for a night. Or swap the tape: your own words go out for one evening. Rare, and every faction says something about it. |
| **Bastion's last call** | A Bastion duty: hold the tallest mast's power while the radio officer calls Command. He calls; the band hisses; nobody answers. Ties to Await Relief. |

## Order of work

1. **A, the masts:** finding them, the expedition (parts, troubles, the climb), who it serves, the
   fog cleared, warnings, listening in, the perimeter's chatter, finding a voice.
2. **B, the grid:** substations, the factions' stakes, lines that break, generators, lit towns,
   sabotage (and a cut mast's job to put it back).
3. **C:** the loop's relay, Bastion's last call.
