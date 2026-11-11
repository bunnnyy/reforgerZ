# Errands: what's left

**Status: designed, not built.** Approved by the author on 2026-11-11. The rest of errands is built and
described in [docs/ERRANDS.md](../docs/ERRANDS.md) (givers, the dealer, keeping them true, ways, pay, the
faults table, 34 routines, the wanderers' asks, 63 cards, fallout, strangers, locals, twists, board and
radio asks); the lie store and the witness rule are in [docs/DESIGN.md](../docs/DESIGN.md#lies-and-witnesses).
The epilogue deck's last cards (Processing, What the Cure Costs Here and the rest) are designed in
[EPILOGUE.md](EPILOGUE.md#the-epilogue-deck-last-errands). This file keeps only the parts not in code yet.

## The dealer

- **A per-player context cache**, refreshed every 30 s: settlement and owner, holding within 600 m, a
  pack's range within 1.5 km, road, vehicle, fight, near a mission or set piece, minutes into the life,
  the nearest plausible spot, tiers, title halves, route lock.
- **Freshness and nearness** factors in the score; **locals coming to you** (a local you helped walks up
  near their house); a radio **contract** offer goes before a radio ask; a 10-minute first-life wait after
  Find the Haven's first leg.
- **A suspect variant** under the arsonist rule: *"Where were you Tuesday?"*, and fit ×0.5.
- **"The last one who said that lied":** a lie found out on a dead life makes that giver kind's next
  errand for that player open with the line, fit ×0.8 for a real day.

## Chores at holdings

Chore slots per holding: 1 + 1 per 5 players online, 3 at most, and one chore per player per holding.

## Pay

- **Tagged mission-paid items:** a post's ration or magazine paid in kind, from its stock and never while
  short, tagged so quartermasters, food delivery, the fence and every errand refuse it for pay.
- **A meal as pay:** a fed house above one upkeep pays −1 food, a little healing and 2 Favor.
- **"When the truck comes":** deferred pay at the next delivery there, to the life that earned it.
- **Carried items** (a letter, a tag, a cut, a cold box) as mission items that can't be equipped, pay only
  when the asker or a helper hands them in, and are refused at other drops; taken by another player, the
  errand is spoiled and the thief takes the card's Infamy.
- At most 2 errand headlines a real day.

## Who asks

- **A Champion card per faction** (the Haven's is You're the One); the hall of fame read for a line
  (*"My brother saw your name on the list at the lab."*).
- **Kill on Sight:** a wanderer who recognises you and would sell where you sleep.

## Strangers

- **Out of sight:** spawned 60–90 m off where the player can't see (a door, a hedge line, a bend) and
  removed out of sight when passed. VERIFY: the out-of-sight spawn.
- **Every crossroads:** junctions of two used roads from the road network, not only road posts.
- **Harm beyond killing:** shot at +4, robbed +8 Haven Infamy, and robbing one stops strangers for that
  player for a real day.
- **The run-ups left as words:** Still Walking's board note at his house when he got the dose; The Last
  Tin's watch as a real watch.

## Locals, callbacks and threads

- **Locals** shown only while a player who knows them is within 100 m; three lines a memo; a local's spot
  used as a place (the gate at dusk, the stove at supper).
- **Callbacks from errands** through the one callback queue (words only, on played time, near the subject).
- **Threads:** up to 12 per player-season tags that lines read, and leaders *saying* one (*"The cook says
  you brought turnips."*).
- **Hand to Hand's promises read by the finale** (a promised local in the same house as one of the eight
  makes that yes one step easier; each promised local at the Independent's lab-gate sit-in takes a minute
  off it). The finale owns the reading ([FINALE.md](FINALE.md)).

## The catalogue

- **His Effects in full:** the effects as a carried item; the jacket and the photo frame at the fence (3
  and 10 Red Hand Favor) under the witness rule; a kit piece off a man you killed refused (*"You'd know.
  You were there."*, +3 Infamy with his faction); once per dead man, 3 times a life.
- **The Name on the Tag's named body** at a job's site: carry the ID to the house or post it names, 4
  Favor from that faction. Red Hand crews don't bury their dead, so it's thanks only.
- **Margin cards** for Await Relief and Proof: 2–4 each, in the major's voice, never on its set-piece
  evenings or inside its 600 m, changing only a giver's memory and a rumour.

## Settings and telemetry

- Settings for what's fixed today: Stranger gap (15–25 min), Strangers on the server (6), Footfall
  half-life (6 real hours), Pinned and unpinned locals (30 / 20), and Majors read errand memory (off).
- Telemetry: lies told against lies found, and twists.
- The `mission_errand` icon for the journal row (the journal uses the unarmed icon today).
