# Borrowed kit: what's left

**Status: designed; disguise phases 1 to 3 built.** Approved by the author on 2026-11-11. Kit, claims,
the look, caught and held, again looks, roll call, word gets round, vouching at barriers, the Rogue's
price, kit that remembers whose it was, the nail list, Bastion's board, colours and every abuse
case's rule are built and live in [REPUTATION.md](../docs/REPUTATION.md#borrowed-kit). This file
keeps only what isn't built yet.

## Not built yet

| What | Where it goes |
|---|---|
| **Claim lines in conversations:** a claim spoken as a choice on their people, its tags on the choice, one claim line per speaker per life, Bastion's word at the wire picked from three (a wrong word fails at once), `pass:` and `catch:` effects. Today the barrier, the post's speaker and missions' `PassAs` are the claims | [DIALOGUE.md](DIALOGUE.md#12-claims-in-someone-elses-kit) |
| **Looks:** "something of theirs from a mission on you" (−1) | The look ([REPUTATION.md](../docs/REPUTATION.md#borrowed-kit)) |
| **Breakers:** a mission's sneak approach that spots someone passing is a catch | [MISSION_STYLE.md](MISSION_STYLE.md) |
| **Missions:** a stage that offers a trick sets `<key>.disguised` *(you)* for later stages; trick objectives at least 25 m from a leader's seat | [MISSION_STYLE.md](MISSION_STYLE.md) |
| **The majors:** Await Relief's Act IV roster read in Bastion kit at night (the gate sentry at difficulty 2 and the duty board at 3, a name off the nail as the cover detail, the muster's callback *"And whoever signed the roster as {dead_name}: he's still dead."*); Proof's records room in a Halcyon coat at night (the duty officer at 3, a real notebook number) | [MAJORS.md](MAJORS.md) |
| **The finale:** after the 90% lock every look with every faction whose route you didn't choose is −1, by identity, until the cure; the stand's spare rule for a route's colours | [FINALE.md](FINALE.md) |
| **The HUD:** a look-coming widget, `RZ_Hud_Look` (the Watched eye with "2 of 2", owner-only through `RZ_UIFeed`, sent only when it changes), and the standing screen's per-faction lines (your face: unknown, known, caught 1 of 3, blown for 40 more minutes; their tells; what their enemies do to their colours). Today the notices carry it | docs/UI.md and UI/LAYOUTS.md when built |
| **Haven gossip** about impostors, sideways and about food: *"Somebody came to Morton in a soldier's coat asking for bread. We gave him bread. The coat didn't fit."* | The look's word gets round |
| **The last week** ([below](#the-last-week)) | [EPILOGUE.md](EPILOGUE.md) |

## Cards for the errands deck

These small jobs are **errand cards**, dealt by [ERRANDS.md](ERRANDS.md) under its rules: a live Why, the dealer, the
interruption budget, and the small-pay ledger (errand Fame at most 2 a card, never above 39 with a faction, 10 a faction
a season day; Favor inside the hourly ledger; amends instead at 15+ Infamy). Leaders never give them: a named person
does. Each is offered only when the season's state makes it relevant, by someone who would really ask. The conditions
count events over time and across different players, so one friend's cheap catch can't open a card for the whole group.

| Card | Giver, and when | What happens |
|---|---|---|
| **The Name on the Tape** (Bastion's variant of the "bring their effects home" family) | A Bastion named person, for players Known with Bastion. One of Bastion's people died near a post this season day (to the infected or in a raid), and his body is still out there | The card puts his body where he fell (a prop, removed afterwards). Bring his tag and jacket to that post's quartermaster: 4 Favor, +1 Fame, his tag on the nail, and his name on the nightly call. Or sell the jacket to the Red Hand's fence for 10 Red Hand Favor (*"A name the army can't check twice"*), and the post's board lists him as "kit lost". It refuses anything a quartermaster sold, and kit off a man you killed (*"You'd know. You were there."*, +3 Bastion Infamy). Once per dead man, 3 times a life |
| **Spot the Tells** | A Bastion named person, for Known. Two or more different players were caught in Bastion kit near a post in the last 2 season days | A Red Hand scout in Bastion kit stands among the post's three named people. Talk to each and find him by his tells: the wrong boots, an AK, no cap. Right: +2 Fame, 5 Favor, and he goes to the stockade (Bust Out runs a little more often). Wrong: *"Private Hollis spends the night in the stockade for wearing the wrong boots. He's furious. He's also, technically, out of uniform."* No pay. Or take the scout's 10 Red Hand Favor to point at Hollis (35% it comes out: +5 Bastion Infamy) |
| **Paint** (with Name Day's Brush) | A crew boss's man, for Known with the Red Hand. A Bastion checkpoint stands at night within 2.5 km of a Red Hand camp | *"New faces paint a red hand on."* Paint one on the barrier or on a Bastion sign in sight of the checkpoint: 20 s at the barrier prop. The post comes down within 45 minutes, so nothing stays. Trick: in Bastion kit, ask the crew for a light. Sneak: from the back, unseen. Force: it's a fight. Pays a crew name for the life (with the majors on, the same `book.named` the pit gives), picked from whatever went wrong, and +2 Fame. Bastion's board: *"Civilian with a paint tin. Day 14."* |
| **Masks Off** | A Halcyon named person, for Known. A safehouse took a bite declaration in the last season day | A beak (a man in a coat, hood and respirator) has been taking "samples" from the bitten. He's a Haven orderly selling them to the Red Hand for his daughter's antidote. Turn him in: +2 Halcyon Fame, and the coat goes back in the record. Let him go: +2 Haven Fame. Or take his coat: *"It's a good coat. It's also number 17, and number 17 is signed out to Dr Brandt."* It's useless at Halcyon's looks, and there's a 35% chance the lab finds out: +3 Halcyon Infamy |
| **Notebook 31** (merged with His Notebook and Into the Record) | A Halcyon named person, for anyone. A Halcyon courier or researcher died this season (The Missing Courier, Lost Colleague, a field lead's death) | His numbered coat and notebook are where he fell (a prop). Return both: +2 Halcyon Fame, the notebook read into the record, and his name on the authors' list at the change of shift. Sell the notebook to the fence: 15 Red Hand Favor (35% it's found out: +5 Halcyon Infamy). Keep the notebook: its number is a real cover detail for one Halcyon look. The coat is a dead man's coat, so it's no good at Halcyon's looks |
| **Wrong Way Round** | A crew boss's man, for Trusted with the Red Hand. A Haven house sits on a road the Red Hand doesn't hold | The house's lads take tolls under painted red hands to keep raiders off, and they painted them the wrong way round. Tell the boss: the lads are "settled", the house loses 1 person, +2 Red Hand Fame, +5 Haven Infamy. Take a cut: 6 Haven Favor from the lads' takings, once (35% the house finds out: +5 Haven Infamy). Warn them off: +2 Haven Fame. Or recruit them: +2 Red Hand Fame, and the house loses 1 person |
| **Civvies** (re-angled) | A Haven occupant, for anyone. A deserter struck off Bastion's strength as KIA, his name still on the board, is sheltering at a safehouse (a Deserter wanderer resting there, or a man generated for the card; never the finale's canon deserter) | The disguise in reverse: he wants to stop looking like a soldier. Bring him civilian clothes (nothing with a faction's keywords): +2 Haven Fame, and he stays as one of the house. What happens to him after that is Struck Off's story, not this card's: if a player takes his tag to a Bastion post, Struck Off opens with him as its subject, and Bastion's way ends with him handing his tag to the sergeant and answering "Here" at the roll. If you sell where he sleeps to the Red Hand (10 Red Hand Favor), he's gone by morning (a headline). It never overlaps Desertion Route, which is about the men a commander lets go |
| **Laundry Day** | A Haven occupant, for Known. A Haven and Bastion truce, a house near a Bastion post, and a Red Hand roadblock in between | The house washes Bastion's uniforms for bread. Get the sack (a mission item, not wearable) past the roadblock. Wear a uniform from it on top: that's Bastion colours, so you're turned round, and during a feud you get the grudge. Or go round, more than 150 m from the roadblock. Or pay the toll twice (20 Red Hand Favor). Or sell the sack to the fence for 15 Red Hand Favor: the house loses its bread (−2 food), and there's a 35% chance it's found out (+3 Haven Infamy). Delivered: +2 Haven Fame, +1 Bastion Fame, and the house gets +2 food |

**Built:** Dress Rehearsal and Borrowed Green ([docs/ERRANDS.md](../docs/ERRANDS.md#the-cards)), and The
Officer's Coat (the stockade mother, for a player disguised in Bastion kit). **Next:** The Name on the Tape and
Spot the Tells, then the rest.


## The last week

After the cure the epilogue lasts **one week** (168 hours; decision 12). [EPILOGUE.md](EPILOGUE.md) owns the closure set;
the disguise system adds these, all finishable within the week:
- **The lock's −1 ends** with the locks at the cure, and every grudge the contest started stands down once. Looks go
  back to their season rules.
- **Bring Them Home.** A dead man's tag, jacket or notebook carried back to his post or station is taken in: Bastion puts
  the tag on the nail and reads his name at the roll; Halcyon reads the notebook into the record and adds him to the
  authors; the Red Hand writes his line in the book as "sent off". Kit off the turned from a military tribe goes back to
  the lost post's roster: *"He's home. Mark him."* Pay is the epilogue deck's, never anything that carries on.
- **Give back borrowed kit** (part of Settle Accounts). Signed-for kit worn by someone else can be handed in at its
  buyer's quartermaster: no catch, no Infamy, and the buyer hears it came back (*"Your jacket came home. Clean, mostly."*).
  Clean kit from the Red Hand's shelf can be sold back to the fence at half.
- **Lines**, built from the season's counts:
  - Bastion, at each holding: *"Eleven jackets on the wire at Morton. None of them fit anyone."*
  - The Red Hand, from the lorry at the crossroads: *"Best-dressed of the season: {player}, through nine of Bastion's
    checkpoints in green and never caught. Drinks are on Bastion."* It's an honour line: the name is frozen and
    filtered, and an `/anonymous` player is "somebody".
  - Halcyon, with the paper: *"Coats returned: 31 of 34. The rest are listed as authors."*
  - The Haven: *"The ribbon only ever went on the people it was given to. Mostly."*
- **The hall of fame:** "Caught in someone else's kit" (a count, with no names) and "Never caught" (the most looks passed
  without a catch, as an honour line).

