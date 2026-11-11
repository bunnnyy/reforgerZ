# The mission style: what's left

**Status: designed, not built.** Approved by the author on 2026-11-11. Everything else of the mission
style is built and described in [docs/DESIGN.md](../docs/DESIGN.md#story-blocks) (the story kit, ways,
checks, who decides, who benefits, truths, records, lies, fallout, reasons, route assets and the lock)
and [docs/MISSIONS.md](../docs/MISSIONS.md) (the six story jobs, the Tier 0 audit, the one-choice jobs
and the work rows). This file keeps only the parts not in code yet.

## The other 23 story jobs

Each gets the story kit as the six built ones have it ([DESIGN.md](../docs/DESIGN.md#story-blocks)): 3+
ways (one needs no gun, one needs no standing), 2+ parties the result can go to (at least one that isn't
the giver, sometimes "nobody"), 2–4 truths with "as they said" never under 30%, a fallout (an errand or a
line), at most 2 world records, a callback, and the reasons it answers. Decency usually pays least, and
never pays the Haven by default. Each has one darkly funny line, spoken by the living
([WRITING.md](../docs/WRITING.md#dark-humour-not-edginess)).

- **The Haven (6):** The Cache, Medicine Run, Missing Scout, Refugee Column, Trader Caravan, The Informant.
- **Bastion (4):** Missing Patrol, Rescue the Pilot, Raid the Red Hand, The Armory.
- **The Red Hand (4):** Dead Drop, Smuggler's Run, Bust Out, Double Agent.
- **Halcyon (3):** Research Data, Cold Chain, Escort the Scientist.
- **World events (3):** Wandering Trader, Prison Break, Collapse Shelter.
- **Unarmed, house and perimeter (3):** Trail of Clues, Letters, The Crossing.

## Talk scenes

- **Nodes:** a scene person has 2–3 nodes, each a line and up to four options (the choice slots); the last
  is always "Leave it", back to the first node.
- **The speaker** holds the person: the first eligible participant to pick an option keeps the floor for
  60 s after each pick, 3 minutes at most. Others in the room may pick options that only reveal something.
  An objector becomes the speaker for 60 s.
- **Lines:** a node's line goes to the speaker and anyone within 8 m (overheard), on the owner-only
  payload.
- **Fallback** if the replicated-labels play-test fails: the line lists numbered options, and the
  choices read "Option 1" to "Option 4".

## Who decides

- **Leave**, per player: you stop taking part, with no pay and no blame.
- **Offline participants** (a step done, or 5 minutes in the area) are paid on their saved profile at
  their next join, through the questlines' path.
- **Amends need clean hands:** Press, Trick, a switch or hiding something voids that job's Infamy
  work-off for that player.

## Trade

- **Background services** on the spot (a Nurse dresses a wound, a Mechanic fixes a pump) are worth 2 on
  the price table, plus the time, as a timed stand-by.
- **Mission goods** (anything a mission spawned) are tagged, never count for Trade, and quartermasters,
  food delivery, the fence and errands refuse them: *"These are Levie's tins. Take them back to Levie."*
  VERIFY: how to tag an item.

## Truths

- **Gates:** a truth whose gate fails (a season flag, a free wanderer, a Rogue leader, a prefab, a place)
  is never drawn.
- **The giver's traits** move the weights by at most ±50%: Greedy and Paranoid hide more, Merciful and
  Idealist less, a Pragmatist says it straight, Ruthless and Veteran leave things out "by procedure", and a
  Talker's secrets leak into road news. Radio contracts use the voicing leader's traits.
- **Canon wins:** a truth about a fixed fact (`bastion.orders`, `origin.*`) reads canon and never rolls.

## Lies and callbacks

- **Caught lying:** a Paranoid giver won't give you their in-person jobs for 2 hours of your play.
- **Callbacks** can also be a person at the door (a stand-in within 300 m) or a note on that house's
  board signed with a name (the notice board taking system notes signed with a person's name).

## Reasons

- **Still to post:** `house.water`, `house.dark`, `house.feud`, `house.borrowed`, `post.pressure`,
  `leader.rogue`, `leader.bitten`, `wanderer.owes`, `wanderer.lost`, `crew.skimmed`, `racket.ran`,
  `lab.power`, `lab.short`, the tribes' and the majors'; the conditions (Every Ridge Calls, Fog Bank, Power
  Surge, Siren Test, Gas Station Fire, Bastion Shelling, Storm, Turned Player) post theirs.
- **Order per player:** the journal and the quartermaster's list put first the jobs near you, those
  answering a reason where you have a record, and those from a giver you've helped. Only the order
  changes, never the odds.

## Teaching it

Four contextual guidance tips, each shown once: the first tag (*"Tags say what an option needs. [Trusted
· Haven] means your standing with the Haven."*), the first truth found (*"The brief is only what they
told you."*), the first background read (*"Your background notices things others don't."*), the first
hold (*"Someone is about to decide. You can object once."*).

## The tribes and the conversion

- **The tribes' contracts on the kit:** Bring Them Home as a story job (find out who the named young was,
  then the ID card or keepsake to the family, or the fence buys it); Cull's choice (where the displaced
  pack goes, through the "where the horde drifts" block); Bastion's lost posts as a one-choice job (their
  tags to the nail, or left walking).
- **`walk.out` read by the tribes:** a man walked on feeds the nearest pack within 2 km as a named young.
- **Tier 0:** check tags on the audited choices that have none.

## Log lines and tests

- An hourly summary: Fame and Favor per player-hour by way; how often switches and "nobody" are picked per
  job; food spent on trades; fallout starts and their pay.
- Runtime test additions: each check result, each truth revealed and one kept after a restart, each
  beneficiary with an objection, a fallout errand, a record written and read, a callback, a reason
  starting a contract, a lie found out by each kind of event, a locked player refused another route's
  option.
