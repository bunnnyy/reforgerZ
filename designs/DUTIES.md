# Duties

**Designed, not built.** The duties themselves are built and are in the docs: work from the first
day ([docs/MISSIONS.md](../docs/MISSIONS.md#who-gets-offered-what)), the duty rules, the trust
ladder and its four rungs, the sorting duties played at their place, the duties' people, things
that happen, the Trusted rung, and each faction's own five
([docs/ERRANDS.md](../docs/ERRANDS.md#duties)). The author's rule that games are played in the
world is in [docs/UI.md](../docs/UI.md#default-first-then-menus). This file holds what the
author asked for that isn't built yet.

## What waits

### Real things on the table

Suspicious Item Props' things (`PrefabsEditable/Auto/Props/Civilian/Suspicious Items/`: the stack
of cash, the gold bar, the pearl necklace, the credit cards, the forged passports, the soldier's
IDs, the pistol in parts, the copper wire, the pill bottle, the drug bricks, the blindfold; never
`E_Smartphone.et`) are named in the duties' words (the swap table, the count, the toll, gate duty,
kit inspection, the roll call, the incinerator) but don't stand there as objects. Two things are
needed first:

- **Prop copies with nothing to use.** They're pick-up items, so one standing as a work site's
  look could be taken: free loot. Each needs a copy without its pick-up action.
- **Our own child prefabs for the papers, the IDs and the pills**, under `Prefabs/ReforgerZ/`,
  inheriting theirs with an era-right name and description (as `RZ_Water_Dirty.et` does), written
  as text with a `.meta`, for the PC to open: the RU passport and ID say "Russian Federation", the
  pills name a 1996 brand, and the passports, IDs, wiring and drug bricks name an inventory icon
  the mod doesn't ship.

JTF Bronze Props' `Detainee` and `InterrogationRoom` scenes (`tools/camp_kit/world_kit.txt`) would
dress the lock-up, with the blindfold.

### The Trusted rung's other versions

| Duty | Waits for |
|---|---|
| The Red Hand's debt book, Halcyon's authors' list (the names) | Written the way the casualty return is: evidence at a place, a word that follows someone |
| Who gets a dose: the house's one dose, the sergeant's list, who the crew pays for | Doses that exist outside the lab. It must never hand a dose to the player, and what it writes needs the finale's rules read first |
| Halcyon's isolation (the lock-up), the animal house (a cull) | Their own beats, on the ward's people |

### By faction

- **The Haven.** The garden's beds seen as beds (today a bed is a spot and its words; the dug,
  planted and ripe looks need props). The roll call's reason followed up (someone sent after the
  one who's out and not back).
- **Bastion.** Kit inspection's missing rifle bought back at a Red Hand toll by the player (today
  it's reported, and the routine Fell Off the Inventory is the buying back). Stag at the wire with
  sectors to choose between.
- **The Red Hand.** The toll weighted by feuds, truces and who holds the road (today: the stage,
  the hour, the weather, the town's sickness). Spotter's board changing what the crew does (today
  it's recorded, rumoured and kept as a saved counter that nothing reads). A night watch by sound.
  Escorting someone through the territory with a body walking beside you (today it's a walk to the
  far marker).
- **Halcyon.** The decon line's stations running short by count (today the water fails once, after
  the third arrival). The ward's research counted where it's done well (today a true chart is a
  record and the giver's regard; it never touches the cure). The cold chain's rules felt in the
  body: running, tilting, time (today the rule decides which choices are right).

### Two errands from the props work

To build as errands or cards if they fit cleanly: a soldier's ID brought back from a patrol that
didn't return; a forger selling false hope at the line. Both appear inside duties today (gate
duty's man with a missing soldier's card, the swap table's forged travel papers), not as errands
of their own.

## What it must not do

- No new reputation system, no pay outside the ledger's caps, no loot or doses handed out beyond
  what a duty names (the still's bottle, a snare's meat).
- No bodies or props away from players; a squad only while its player is near (the escort block's
  rules and cap).
- Nothing a pad can't do, nothing that needs reading at speed.
- A duty never writes a leader's memory or vote, like every errand.
