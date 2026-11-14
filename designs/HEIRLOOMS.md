# Heirlooms: one of a kind

**Designed, not built.** Approved by the author on 2026-11-14. Written for
[PERSISTENCE.md](PERSISTENCE.md) (a character lasts across deaths).

The review's gap: a long walk pays nothing personal, and loot is vanilla items by building type.
Heirlooms are **things only one person can hold at a time**: each belonged to someone in the Zone's
story, each is found by following what the world already says (a trace, a door mark, a puzzle), and
each does one small thing.

## The rules

- **18 a season**, from the list below, each placed once per season by the server at a fitting real
  place (PLACES.md's needs), never twice in the same place two seasons running.
- **Found by reading, not by luck.** Each heirloom has a trail: a door mark with a story (TRACES.md,
  "Reading a door"), a scene (TRACES.md, "Scenes"), a note in a puzzle (the clue registry,
  `RZ_TraceClues`), or a person who talks about it. The trail's last step is the thing, standing
  where its words say. Never a random loot roll.
- **Held, not hoarded.** An heirloom is a real item in your inventory (a base-game prop's look,
  named). It can't go in the locker ([LOCKER.md](LOCKER.md)) and isn't a quest item: **at your death
  it stays on your body** (PERSISTENCE.md, "Death"), where anyone can take it. If the body goes with
  it still there, the heirloom is set down where the body was, as data, and shows to players who
  come near (a small prop, near players only). It never despawns in a season.
- **Small and capped.** Each opens a reply in talk (a check, DIALOGUE.md "Checks and tags") or gives
  one small effect, capped like every effect. Nothing that shoots better, nothing that pays Fame.
- **Known by the world.** Who holds one is in the season record by name: people mention it (*"That's
  Rudi's key on your belt."*), and the hall of fame lists the heirlooms brought to their ends.
- **An end each.** Most heirlooms have somewhere they belong. Taking it there (a reply) ends it for
  the season, pays small Favor with that faction and writes a line on the season's story. Kept, it
  goes on doing its small thing.

## The eighteen

Every owner is a man or a place; women are only mentioned.

| Heirloom | Whose | The trail | What it does | Where it belongs |
|---|---|---|---|---|
| Rudi's shutter key | Rudi Kasper, the garage at the crossroads | the shutter's marks; the book's first line | opens the garage's back room (a real door's lock); Red Hand old hands: **[Rudi's key]** *"Where did you get that?"* | Rudi's shutter (set it down there) |
| The Janda lorry's ignition key | the Janda brothers | a lorry's cab, a note in a toll hut | the lorry at the crossroads moves once; a Red Hand toll waves you through once a day | the brothers |
| The bus driver's clipboard | the Haven's bus driver | the first Haven safehouse's board; a door mark "the bus stopped here" | Haven: one persuade a conversation one step easier | the first safehouse |
| The priest's church key | the priest who opened his church on Day 4 | a church door's mark; the Haven's story | a Haven safehouse that was a church takes you in even when full | his church |
| Corporal Vrana's badge | Corporal Radek Vrana, Day 5 on the north road | a roadblock's scene | Bastion: **[Vrana's badge]** at a gate, searched once instead of twice | the captain |
| The captain's log page | Bastion's captain | a burnt office; the roll call | Bastion: reads the Day 3 orders aloud (Await Relief, a reply) | the stockade's board |
| Lieutenant Laurent's compass | the Last Pilot | the wreck (The Last Pilot) | your map shows the tracked objective's bearing without a map item | his crew, if any live; else the wreck |
| The roster's spare pencil | Bastion's roll | the duty roster | the roll call: you can add a name once a season | the sergeant |
| The lab's first sample tag | Halcyon | the case's runner (OPENING.md) | Halcyon: **[The tag]** the director won't lie to you once | the director |
| The site log's binder clip | the director's site log | Proof's pages | holds a page of the site log you find without it counting as carried | the lab's records room |
| The courier's armband | Halcyon's first courier | the courier's route; a dead courier | Halcyon checkpoints let you pass as a courier once a day | the lab gate |
| The vet's bag | the vet at the farm | a farm's door mark "vet, gone" | treating animals and the bitten 10% quicker | the vet's farm |
| The well rota | a village's well | the duty's board | one village's well never fails you | the village |
| The radio ham's crystal | a radio ham, Day 6 | a mast's hut | one secure channel a season day, heard without cracking it | the mast |
| The Long Table's place card | the Long Table | the guestbook (TRACES.md, "The Long Table: absence") | the Long Table's people are polite to you; nothing more, which is worse | burn it |
| The column's road atlas | the Day 2 column | the column's echo (INFECTED.md, "Who they are") | a column tribe's tracks read to anyone, not just a Hunter | the line, where the column stopped |
| The schoolroom bell | the Haven's school room | a school's mark | ring it at a Haven safehouse: the evening meal comes an hour early once a day | the school room |
| An alpha's tag | a named alpha ([ALPHA_HUNT.md](ALPHA_HUNT.md)) | its hunt | the factions pay differently for it | its own (ALPHA_HUNT.md) |

## Many players

One of each per season on the whole server: whoever holds it holds it. Heirlooms make PvP stories
(*"He's got Rudi's key."*), and people on the server can be asked where one was last seen. Two
players on the same trail race; the trail's steps are readable by anyone.

## What it uses

Traces (marks, scenes, the clue registry), puzzles, places' needs, the dialogue engine's checks,
items as base-game props with names, the season record, the hall of fame, corpses as built.

## Order of work

1. The heirloom item (a named prop, its owner by name, data when set down) and its rules at death.
2. The trails: six first (Rudi's key, the clipboard, Vrana's badge, the compass, the tag, the vet's
   bag), one per faction and two for the road.
3. The effects and the replies, each capped; the ends.
4. The rest of the eighteen; landmarks get theirs ([LANDMARKS.md](LANDMARKS.md)).

## Settled with the author (2026-11-14)

- A held heirloom is known only by rumour, never shown on the map.
