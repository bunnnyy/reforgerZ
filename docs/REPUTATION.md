# Reputation, factions and unlocks

How the four factions treat you. Inspired by New Vegas: every faction remembers two
separate things about you, **Fame** (what you did for them) and **Infamy** (what you did
to them). You can be a hero to the Haven and a nightmare to the Red Hand, or loved and
feared by the same faction at once.

Missions are the main source ([MISSIONS.md](MISSIONS.md)). Most help one faction at the cost
of another, and some make you **choose who gets the prize**. Who the factions are as people is
in [LORE.md](LORE.md); how they hold territory and move supplies is in [DESIGN.md](DESIGN.md),
"Factions in the background". ✅ = in code; everything else is planned ([ROADMAP.md](ROADMAP.md)).

## The four factions

| Faction | Who they are | Wants | Rival of |
|---|---|---|---|
| **The Haven** | A network of safehouses run by ordinary people stuck in the Zone | Food, medicine, safe towns, people | The Red Hand. Uneasy with Bastion |
| **Task Force Bastion** | Soldiers abandoned inside the line, who look after their own and hold a few posts | Their own first; order, control, the cure as a strategic asset | The Red Hand. Distrusts Halcyon |
| **The Red Hand** | Opportunists who saw their chance when the world broke | Profit, leverage, fear | Everyone, Bastion most of all |
| **Halcyon** | The researchers running the only lab | Samples, data, protection, the cure going public | Wary of Bastion seizing it |

The code uses the enum names `SURVIVORS`, `MILITARY`, `BANDITS` and `INSTITUTE`, and the
faction keys `SURV`, `MIL`, `BAND` and `INST`. Only the display names differ.

## Fame and Infamy

Each faction tracks **Fame 0–100** and **Infamy 0–100** for your current life.

- Helping a faction gives Fame with it, and usually Infamy with its rival.
- Hurting a faction (killing its people, stealing, sabotage, siding against it) gives Infamy.
- Neither goes down on its own. You work Infamy off with **amends** ✅, all from one budget
  of 30 per faction per life ([Working off Infamy](#working-off-infamy)).

### Standing tiers

| Fame ↓ / Infamy → | Low (< 15) | Some (15–39) | High (40–74) | Extreme (75+) |
|---|---|---|---|---|
| **Low (< 15)** | Stranger | Watched | Hunted | Kill on Sight |
| **Some (15–39)** | Known | Uneasy | Hunted | Kill on Sight |
| **High (40–74)** | Trusted | Tolerated | Wildcard | Hunted |
| **Extreme (75+)** | **Champion** | Respected | Wildcard | Wildcard |

- **Positive tiers:** Known → Trusted → Champion. They unlock shops, perks and missions.
- **Mixed tiers** count one step lower for access: Respected = Trusted; Tolerated and
  Wildcard = Known; Uneasy = Stranger.
- **Negative tiers:** Watched → Hunted → Kill on Sight. The faction comes after you.

**The tier is the only rule word.** Shops, perks, contract requirements, persuade ranks and the AI
hostility bitmask all read the tier. What people call you is the cell's name
([Reputations](#reputations)), and a name opens nothing.

**The credit runs out (Fame 75+)** ✅. While your Fame with a faction is 75 or more, every Infamy
gain with it also takes the same amount off that Fame: the final amount, after a uniform's doubling
and the stronghold leader's temper. It stops once Fame is below 75. So Wild Child (75+/75+) who
kills one more guard slides toward Soft-Hearted Devil, which is Hunted, and a Champion's accidental
3 Infamy costs 3 Fame. A Champion background unlock is kept: it's recorded when it's earned. A
Champion → Trusted drop says why: "Your credit ran out." (**Reputation > Credit rule**, from
**Credit from**, 75.)

Players are told when their tier changes, not on every point. The tier notice leads with the name
when the name isn't the tier word: "They call you Useful Trouble - counts as Known now."

### Reputations

✅ Each of the 16 cells has a name. The 7 plain cells keep their tier word; the 9 mixed cells get
their own (**Reputation > Names**, on; off shows the tier words only).

| Fame ↓ / Infamy → | Low | Some | High | Extreme |
|---|---|---|---|---|
| **Low** | Stranger | Watched | Hunted | Kill on Sight |
| **Some** | Known | **Mixed Bag** *(Uneasy)* | **Bad News** *(Hunted)* | **Merciful Thug** *(Kill on Sight)* |
| **High** | Trusted | **Useful Trouble** *(Tolerated)* | **Wildcard** *(Wildcard)* | **Soft-Hearted Devil** *(Hunted)* |
| **Extreme** | Champion | **Favourite Rascal** *(Respected)* | **Necessary Evil** *(Wildcard)* | **Wild Child** *(Wildcard)* |

- **Name first, rule second:** "Useful Trouble - counts as Known", "Soft-Hearted Devil - Hunted".
  The mixed tier words Uneasy, Tolerated and Respected stay in code and in the tier table as access
  classes; players see "counts as" instead. Requirements always say tiers ("Trusted with Bastion").
- **Notoriety** (0-3) is the higher of your Fame band and Infamy band with a faction: how well its
  people know your name. It's in the standing feed; the voices and the disguise look will read it.
- **Earned this life.** The profile keeps two totals per faction for this life: every Fame gain, and
  every direct Infamy gain (rival shares left out). **Earned Fame** is the lower of current Fame and
  Fame gained; **Feared** the lower of current Infamy and direct Infamy gained. A background's start
  and Word gets around count for neither, and amends lower both. The Fear check reads them
  ([DESIGN.md](DESIGN.md#checks-and-ways)), so a fresh Ex-Con has nothing to threaten Bastion with.
- **Deeds.** The life keeps, per faction, its best deed (the biggest single Fame gain with a phrase)
  and its worst (the biggest direct Infamy gain), how many of their people you killed and how many
  jobs you did for them; the season keeps the same by identity ([DESIGN.md](DESIGN.md#deeds)).
  Phrases come from contracts ("brought medicine to Levie"; "did a job for us near Morton" for one
  without its own), a leader's killing ("shot Vlasak") and running a checkpoint. The worst deed reads
  as the kill tally ("put 9 of ours down") once three times the kills outweigh it.
- **The change notice.** A new name is told once it has held 30 s (a quick swing back says
  nothing), at most once per faction per 5 minutes; a tier change is always told at once. The first
  mixed name also gets a guidance tip: "People can like you and fear you at once." (**Reputation >
  Change hold seconds**, **Change gap minutes**.) A log line on every change:
  `Reputation: <player> <faction> <name> (<tier>)`.
- **The card** on the notebook's Standing page shows the name, what it counts as, and how much is
  left to forgive; its 16-cell grid is filled in by code, with each faction's initial in the cell you
  stand in ([UI.md](UI.md#the-notebook)). Without the layouts, **Ask what they think of you** at a
  quartermaster gives the same, one line a faction: "The Haven: Useful Trouble (counts as Known;
  Fame 46, Infamy 22, 30 of 30 left to forgive; Favor 30)".
- **The admin read-out:** `/rep <name>` in chat (server admins; empty for yourself), or the
  **[Admin] Standing of the nearest player** reply when an admin talks to anyone, prints a
  player's cells, Earned Fame and Feared, what's left to forgive, deeds, tallies and threats used,
  to the admin and to the log.

### What people call you

✅ Code: `Factions/RZ_RepVoice.c`; the gate barks, others hear and the bounty's words in
`Factions/RZ_RepNamesGates.c`. A name opens nothing: every rule still reads the tier. What's
said aloud (the openings, what they make of you, the picture) is said only with **Dialogue > Trees**
on ([DIALOGUE.md](DIALOGUE.md)); the word and crew names are kept either way.

**Manners** are a writing tool, never a rule: the cell picks how a line sounds. Stranger is
*unknown*; Known and Trusted *warm*; Champion *devoted*; Watched and Mixed Bag *wary*; Useful
Trouble and Favourite Rascal *fond*; Wildcard, Necessary Evil and Wild Child *needed*; the Hunted
cells *hostile*, or *owed* when they still owe you a hearing ([the parley](#the-parley-and-the-knock)).
A Pragmatist, or an Independent or Rogue leader who holds nothing against you, reads *hostile* as
*wary*.

**The word for you**, to your face, this life (`RZ_RepVoice.Word`, the `{word}` token):

| Manner | The Haven | Bastion | The Red Hand | Halcyon |
|---|---|---|---|---|
| unknown | love | civilian | you | visitor |
| warm | a friend of the house (Trusted: the one with the ribbon) | civilian, cleared (Trusted: auxiliary) | your crew name, else new face | courier (Trusted: field associate) |
| devoted | one of ours | on the roster | your crew name | colleague |
| wary | a worry | uncleared | you | subject of concern |
| fond, needed | a worry we feed | flagged, useful / flagged | your crew name | under review |
| hostile | put out | on the list | that one | excluded from the study |
| owed | put out, prayed for | on the list, with a note | in the book, both pages | excluded, with thanks |

Kin of a fallen Champion are "family" to the Haven. A word the majors or questlines give
(`SetWord`, "Sergeant Novak") wins unless they're hostile.

**After the 90% lock**, until the cure, the word is the lock's ([FINALE.md](FINALE.md#the-lock)),
unless they're hostile. Names don't change.

| You locked to | The Haven | Bastion | The Red Hand | Halcyon |
|---|---|---|---|---|
| Their route | with the house | attached | in on it | signed on |
| A rival route | the other lot's | hostile element | the other crew | a confound |
| Independent | nobody's | unaffiliated | freelance | an uncontrolled variable |

**Red Hand crew names** are what the crews hold on you: leverage, not an honour. The first of these
this life, while you're Known or better with the Red Hand, becomes your name, and a notice tells
you; until then you're "new face" (**Reputation > Crew names**, on).

| Did | Name |
|---|---|
| Survived the cut dose | Lucky |
| Bitten twice in one life | Snack |
| Ran a Bastion checkpoint and reached a Red Hand roadblock the same season day | Legs |
| Paid a toll with exactly the Favor you had | Skint |
| Had three things taken at one Bastion search | Pockets |
| Killed one of theirs, then did a Red Hand job the same season day | Sorry |
| Laid a pack's leader down (the nearest player within 150 m when it falls) | Bailiff |
| Threw a smoke grenade | Smoke |

**Smoke's throw hook** (`Factions/RZ_RepNamesThrows.c`): every alive player's character is listened
to for the character's `OnGrenadeThrown` event, as the molotov's trap is (VERIFY: it fires on the
server for a thrown smoke); a throw counts when the thrown thing's prefab name says "smoke" (vanilla's
M18 and RDG-2, Bastion's signal smoke), not by weapon type, because the bottles and the tin rattle are
smoke grenades underneath. Log line: `Reputation: <player> threw a smoke`.

The Name Day card's dares give theirs ([ERRANDS.md](ERRANDS.md#the-cards)). A pack's leader laid down
is also a deed in each faction's own words, unless a bigger one stands: "laid the big one down at its
lair" (the Haven), "put the heavy down" (Bastion), "evicted the boss up the hill" (the Red Hand),
"brought in the index" (Halcyon). Log lines: `Reputation: crew name <name> for <player>`,
`Reputation: <player> laid a pack's leader down`.

**What they make of you.** With Trees on, a leader's **What do you make of me?** is one template
per faction in four variants (both deeds, best only, worst only, neither), then what the leader
remembers of this life, then the rule in brackets:

| Faction | Both deeds | Neither |
|---|---|---|
| Haven | "People have started calling you {rep}. You {best}. You also {worst}. I'd rather they didn't have to." | "People call you {rep}. Nobody can say why yet." |
| Bastion | "Your file says {rep}. Commendation: {best}. Incident: {worst}. Both signed for." | "Your file says {rep}. The rest of the page is blank." |
| Red Hand | "Crews call you {word}. The book says {rep}. Owed: {best}. Owing: {worst}. The book's never wrong, only late." | "The book says {rep}. Short entry." |
| Halcyon | "We have you as {rep}. It's a category, not a judgement. Contribution: {best}. Incident: {worst}. Both are in the record." | "We have you as {rep}. Insufficient data." |

"(The Haven: Useful Trouble - counts as Known. Fame 46, Infamy 22. 30 of 30 left to forgive.)"

**The opening.** With Trees on, a leader greets someone they've met with one line, the first that
exists: the **crossing opener**, once a life per leader, at a seat of the faction a major was about
when you were one of those who decided its ending (*"The Closed Book. That was you. The book's got a
line about it, in red."*); at a Haven seat, once a life, kin of a fallen Champion are greeted as family; a **local deed** at this seat ("You're the one who brought medicine to Levie.", written when
you finish that leader's personal job); this life's memory (saved: "Here's the one I owe. Sit
down."; crossed: "You. I remember."); the **name from an earlier life**, once per leader per life
("There was a Lukas who crossed me once. Different face. Same name. Should I be worried?"); a
**route line** the first visit each real day once routes are open or you've locked
([WRITING.md](WRITING.md#the-race-and-the-lock)); then, unless they're hostile, one time in four
**who you work for** when it's another faction (the one you've given the most Fame, 15 or more:
"You run for Bastion. Noted. Arms out.") and one time in three your **best deed** ("You're the one
who held the lab."); one time in four, when no worst deed says it, the **tally** ("You've put 4 of
ours in the ground. I count. Somebody has to."); the greeting for your manner in the leader's voice, from the voice bank
([WRITING.md](WRITING.md#the-voice-bank)), with the owed's own ("Halt. Hands. You {best}. You also
{worst}. Both go in the log. Talk.") and a set word said the way each faction says a rank; the built
greeting.

**The change notice in voice.** With Trees on, a new name is told in the faction's voice (the voice
bank's `change` lines, by manner), with the rule in brackets: *"Your file's been updated. Useful
Trouble. Initialled. (Useful Trouble - counts as Known)"*; for the Hunted, "(Hunted: their people shoot
you. They still owe you a hearing at a gate.)" when they're owed. With a leader of that faction within
30 m, the leader says it (a subtitle); else it's the faction's notice. Without a line, the plain notice.

**Gate barks** (**Reputation > Gate barks**, on). At a holding's gate, a road post or a safehouse door,
within 25 m, the place says one line about you, only on a change: your first visit this life, or a new
name or best deed with that faction since the last bark there (*"Here's the one who carried a family
into Levie. Mind the step."*; *"Known again. Something's changed about you."*). From the place, never
a body, so it never gives away a lookout you can't see; never while you're in someone else's kit,
never in the Hostile manner; once per place per session (the approach lane). **Others hear**
(**Reputation > Others hear**, on): at notoriety 3, in your own colours, other players within 30 m of
the gate hear it too, once per 10 minutes per place for everyone (*"Someone at the gate nods at Mara.
..."*), under the names policy ("that one" for the anonymous). Log line: `Reputation: gate bark at
<place> for <player>`.

**On the standing page** (`Factions/RZ_RepNamesPage.c`) the header carries the whole picture after
your name, background and life (*"MARA - NURSE - LIFE 2   "Word is you're the Haven's Champion and
Bastion's Bad News. I'll believe the better half.""*). Each faction's leaders column opens with two
lines: the word as a sentence (*"The crews call you Lucky."*, *"Bastion has you down as civilian,
cleared."*; "The crews don't call you anything yet." before a crew name) and the nearest leader of
theirs you've met, with what they make of you. For the owed the tier's note reads *"Hunted - they owe
you a hearing"* (the feed's `F` record carries the word and the owed flag; `H` the picture; `W` the two
lines). With **Reputation > Names** off, neither the picture nor the two lines are sent. A contract the
parley reserved for you reads **"Amends: works off Infamy · finishes where the job says"** in the
journal's requirements.

**Bounty notices** carry the target's name with the Red Hand and a dry note for the owed: *"The Red
Hand board: 40 Favor on Mara, Merciful Thug. The book says they're owed, too."*

**Deed phrases from missions** (`Factions/RZ_RepNamesDeeds.c`): every contract and world event has
its own phrase, set on the mission (`m_sDeed`) or, for the rest, by class (*"took back Morton"*,
*"held the lab"*, *"answered a distress call near Levie"*); a class missing from both says *"did a job
for us near {town}"*. Each questline's, major's and finale route's ending has a few words, written as
the deed of the Fame the ending pays, online or not (*"buried the list"*, *"put the paper under
seal"*, *"brought the cure home"*). War deeds between locked players are the finale's (`NoteWar`,
[FINALE.md](FINALE.md)).

**Deed phrases** beyond the missions: firing at a house (*"fired at the door in Levie"*), killing
or robbing a wanderer (*"shot Old Marta, who walked the roads"*, *"robbed the Trader on the road"*),
and shooting the farm's people (*"shot a farmer who'd done nobody any harm"*).

**The whole picture** (`RZ_RepVoice.Picture`): the two factions that know your name best, in a
wanderer's voice: "Word is you're the Haven's Champion and Bastion's Bad News. I'll believe the
better half." Unknown everywhere: "Nobody's heard of you. Enjoy it."; notoriety 2 or more with all
four: "Everybody's got an opinion about you. None of them agree." With Trees on, a wanderer says it
one time in four, after the road news. Its kinds (`picture:` in conditions), first match wins:
WANTED (Hunted by two or more), TORN (Hunted by one, Known or better with another), OUTLAW, MIXED (any
mixed cell), SIDED, FRIEND_OF_ALL, ONE_SIDE, WATCHED, UNKNOWN.

**Tokens** for any line (`RZ_RepVoice.FillFor`): `{rep}`, `{word}`, `{best}`, `{worst}`, `{kills}`,
`{jobs}`, `{picture}`, `{name}`, `{crew}`.

### Face and name

✅ **Leader and wanderer memory is life-stamped** (**Reputation > Life-stamped memory**, on). Each
entry keeps the life it was written in. Memory from this life counts as built; memory from an
earlier life counts at half, rounded toward zero (crossed −3 becomes −1, saved +2 becomes +1, helped
+1 becomes 0), and met doesn't carry, so a new character isn't refused by people who never met them.
Old saves load their entries as an earlier life. Questline participation stays by identity.

**Regard.** With Trees on, how you talk to a leader or wanderer adds regard ÷ 2 to their attitude,
at most ±2 and fading a step a real day ([DIALOGUE.md](DIALOGUE.md#memory)). Prices still read
only the built memory.

**The name outlives the face** this season: [the season record](#the-season-record). A leader who
never met this life's face, but whose faction an earlier life stood out with, greets it once with
the name: "Don't know the face. Know the name. It's in the file as Useful Trouble."

### The season record

✅ Code: `Factions/RZ_RepNamesSeason.c` (`RZ_RepSeason`). The name a player made this season, by
identity across every life, wiped with the season. Sampled every 30 s for players online; saved in
its own file, `$profile:ReforgerZ/season_record.json`, at the 2-minute save when changed, and dropped
when the season number changes. Nothing in it opens anything.

| Kept | Per |
|---|---|
| The best and the worst cell reached (by access rank, then the cell's place on the grid) and the life the best was reached in | faction |
| The deed that stood at the best and at the worst (the life's best deed and worst deed then) | faction |
| Kills, jobs and Infamy worked off, summed across lives; the last Infamy seen | faction |
| The cell at the 90% lock | faction |
| The route at the lock and when it was taken; one war deed (`NoteWar`); the latest crew name; the name as first recorded | player |

**Titles.** A player's title with a faction is the name of the best cell they reached this season
("Necessary Evil"), whichever life reached it. Conversations read it as `title:bastion=...` and
`{title:bastion}`, and `seasoned:bastion` is true when an earlier life stood out with them (Known or
better at its best, or Watched or worse at its worst).

**What the ending reads:** `TitleFor`, `SeasonCell`, `SeasonDeed`, `SeasonPicture` ("Necessary Evil
to Bastion": the title with the faction you stood highest with), `RouteAtLock`, `LockDays`,
`LockCell`, `CrewName`, `WarDeed`, `Kills`, `Jobs`, `WorkedOff`, `NameOf`, and `IsSquare` (the last
Infamy seen with them is under 15). Log line: `Reputation: season record: <player> locked to <route>`.

`SeasonDeed` is the deed that stood at the best (or worst) cell; `BiggestDeed` the biggest across
every life this season (the season's deeds store). Square with the Red Hand also means no unpaid line
in its book (a dose on credit, the finale's price door) until the epilogue's *Pay into the book* marks
it paid. The epilogue reads it for titles ([EPILOGUE.md](EPILOGUE.md#at-the-cure)).


### Small pay and switched sides

Two kinds of Fame don't count toward the season's Fame (the hall of fame's "Zone's Fame", who the
cure goes to with nobody in the room, next season's rules), through flags on `AddFame`:

- **Errands** pay small (`RZ_RepFlag.SMALL`): at most 2 an errand, never taking Fame with a faction
  above 39 (they can make you Known, never Trusted), at most 10 a faction a season day, no rival
  shares, and left out of the hall of fame's Fame lines and the best deed too. Errand Favor is at
  most 40 an hour, with diminishing returns ([ERRANDS.md](ERRANDS.md#pay-nothing-for-free)).
  Their dark ways cost Infamy only when someone saw (the witness rule) or you knew: selling a woman
  to a toll or a crew lad seen by the Haven's +12 Haven Infamy, robbing the dead in front of the
  digger +3; killing a stranger or a local costs what killing a wanderer does (+15). Their lies go
  into the one lie store and cost 2–5 when found ([ERRANDS.md](ERRANDS.md#strangers)).
- **A switched side** in a choice mission (`RZ_RepFlag.SWITCH`): the side that got the result pays
  at most the mission's base; its Fame counts for you and your tier but not for the season
  ([DESIGN.md](DESIGN.md#story-blocks)).

### Favor (currency)

Every faction pays in its own **Favor**, earned from its missions and spent at its
quartermaster. Reputation decides **what** you can buy; Favor decides **how much**. Favor
is per life, like reputation.

### Rivals

Fame with one faction also adds a share as Infamy with its rival: Haven → Red Hand 15%,
Bastion → Red Hand 25%, Red Hand → Bastion 20% and Haven 15%, Halcyon → Red Hand 10%.
A mission with its own rival uses that instead. A truce halves it, a feud makes it ×1.5
([Truces and feuds](#morale-needs-truces-and-feuds)).

### Lives and carry-over

Reputation is per character, so a new life starts fresh with only its background's starting
standing ([DESIGN.md](DESIGN.md), "Backgrounds and personal goals"). Two things carry over:

- **Word gets around** ✅: if any past character reached Champion with a faction, every new
  character starts at least Known with it (at least 15 Fame, and any starting Infamy the
  background gives with it is cut to 14).
- **Unlocks** ✅: Champion with a faction unlocks a background (Soldier, Doctor, Raider, Elder);
  10 Legacy points unlock the Drifter.
- **Remembered** ✅: the next character after a Haven Champion dies is kin of the fallen
  ([The Haven](#the-haven)).

## What each faction gives you

Shops are the faction's own **quartermasters**: any item or crate, sold for Favor at a required
tier ✅. Never a vehicle or a car part: cars are only ever found, and the server refuses a shop or
requisition action set to one ✅. The items below are a suggested starting list. Prices are the base cost ×
the local leader's traits and memory (within ±20%), × a shortage (+25–50% where a supply truck
didn't arrive), × the faction's morale (0.95–1.1). The server checks tier and Favor when you buy.

**Trade** at a quartermaster opens the shop screen ✅ ([UI.md](UI.md#the-shop-screen)): the same
stock in tabs (food and water, medical, gear and tools, weapons and ammo; Requisition and the
quartermaster's services on a Services tab), each item with the price it costs *you* there now and,
when you can't have it, why ("Needs Champion with the Haven", "Not until the next truck"). Buying
there runs the same checks as the Buy action. Without the screen's layout the Buy actions show as
before.

### The Haven

| Tier | Shop | Perk / service |
|---|---|---|
| Known | Canned food, water, bandages, crowbars | A welcome message ✅ (safehouse services, including bite declarations and expired antibiotics for 5 Favor, are open to anyone the Haven hasn't exiled). **Ask about the roads** ✅: the three nearest packs, the Haven's names for them and their mood ([INFECTED.md](INFECTED.md#knowing)) |
| Trusted | Hunting rifles, ammo, backpacks, radios | **Neighbours** ✅: the direction to the nearest job you'd be paid for, on spawn and every 10 min |
| Champion | Fortification kits (barricade planks), saline bags | **Safe Harbour** ✅ and **Remembered** ✅: see below |
| Hunted | — | **Exile** ✅: see below |

**Safe Harbour.** While a Haven Champion is inside any safehouse (within 60 m of it), no hunter
squad is sent after them, and a squad already hunting them loses the trail: it walks 400 m away
and gives up within 2 minutes (the player is told once a life). Bounty notices give only a
vague area: a town 1.5–3 km from where they really are.

**Remembered.** When a Haven Champion dies, a memorial headline goes out to everyone and the
nearest safehouse's council head reads the name at supper (players Known with the Haven hear
it). That player's next character is **kin of the fallen**: at their first safehouse they're
greeted as family and the medic sees to them for nothing, once (a full heal; a bite is slowed as
if declared at a Town, which counts as declaring it). Nothing more: the "word gets around" Known
start is separate.

Each safehouse's quartermaster stocks by its size: a Shelter sells Known items, a Community or
Settlement up to Trusted, a Town up to Champion ([DESIGN.md](DESIGN.md), "Haven safehouses").

### Task Force Bastion

| Tier | Shop | Perk / service |
|---|---|---|
| Known | Military rations, morphine, flares | **The contact board** ✅: every pack Bastion has counted ([INFECTED.md](INFECTED.md#knowing)). **Waved through** ✅: Bastion's checkpoints inside the Zone let you pass unsearched ([Checkpoints](#checkpoints-roadblocks-and-camps)) |
| Trusted | Service rifles, body armour, helmets, NVGs | **Requisition** ✅: once an hour, ammo or rations at half the Favor cost, only from what the quartermaster sells at Trusted or below ✅ (never a Champion item); anything that isn't ammo or rations by its name is refused, and so is anyone in Red Hand colours, as at the shop. Bastion kit signed out is marked as the shop marks it ([borrowed kit](#borrowed-kit)). While Bastion's trucks run, it's drawn at a Bastion outpost's quartermaster from the stock its trucks brought, and refused while that outpost is short |
| Champion | Machine guns, grenades, the **signal smoke** | **Fire Mission** ✅: see below |
| Hunted | — | Bastion AI shoots you ✅; hunter squads get your last position ✅ |

**Fire Mission.** A Bastion Champion throws a signal smoke (bought on the Champion shelf) at a
horde: at least 6 infected within 50 m of where it lands. 30 seconds later the guns hit it: four
shells, and up to 25 infected there die. Once an hour per player; never within 250 m of a
safehouse, a faction holding or the lab; the shells don't check who's standing there. It pays
nothing: the reward is the horde gone. A non-Champion's smoke gets no answer. It shares the
shelling with the Fire Mission contract ([MISSIONS.md](MISSIONS.md)).

### The Red Hand

| Tier | Shop (black market) | Perk / service |
|---|---|---|
| Known | Knives, pistols, lockpick-style tools, the **cut dose** (cheap: cures half the time, else a faster turn and a fever ✅) | **Toll pass** ✅: Red Hand roadblocks take a toll instead of shooting ([Checkpoints](#checkpoints-roadblocks-and-camps)). **The map of the crews** ✅ (15 Favor): every pack, with its crew name ([INFECTED.md](INFECTED.md#knowing)) |
| Trusted | Suppressors, scoped rifles, explosives | **Fence** ✅: sell found loot for Favor; roadblocks wave you through ✅; the **red armband** ✅ ([clues](#reputation-clues)). **The tag frequencies** ✅ (25 Favor): Halcyon's tagged packs, and a stolen tag |
| Champion | Rare military gear, stolen antidotes (only the doses stolen from the lab ✅) | **Bounty board** ✅: a bounty on whoever last killed you |

**Fence.** "Fence what you're carrying" at a Red Hand quartermaster buys found loot by keyword
(watches, jewellery, binoculars, compasses, cameras, lighters: a setting) for 3 Favor an item,
at most 8 a sale and 30 Favor an hour. It never buys anything a quartermaster sold (the last 300
sales are remembered), so buying and selling back can't make Favor.

**Bounty board.** A Champion can post a bounty on **the last player who killed them** (60 Red
Hand Favor). It's announced to everyone, and with the Kill on Sight bounties every 10 minutes, with
the same rough "last seen". It's paid the same way as any bounty: once, to the killer only, for a
player kill. The killer gets 40 Red Hand Favor, less than it cost, so a pair of friends can't farm
it; and never the poster, anyone in the poster's or the target's group, or someone the Red Hand
hunts. One posting per poster and per target, three on the server at once; unclaimed after 2 hours,
it comes down and the poster gets half back. Postings, who last killed whom, the Requisition
cooldown and the fence's hourly cap are saved by identity and real time, so a restart changes none
of them (the 2 hours run on in real time while the server is down). *Simplified:* "friends" means
the same vanilla player group; a poster in no group excludes only themselves.

| Hunted | — | Red Hand AI shoots you ✅; hunter squads come for you ✅ |

### Halcyon

| Tier | Shop (lab stores) | Perk / service |
|---|---|---|
| Stranger | The **trial dose** for anyone bitten (free, once per life, 80% cure ✅) | — |
| Known | Antiseptic, painkillers, sample containers, **stabilisers** (from a small stock; double price until 50% research ✅) | **The range map** ✅ from 25%: the packs' ground Halcyon has read or tagged ([INFECTED.md](INFECTED.md#knowing)) |
| Trusted | Antidotes (only while the lab has doses in stock ✅), stim kits, gas masks | **Sample scanner** ✅: special infected within 300 m are revealed every minute. **The medic** ✅: 20 Favor to be patched up, and a fresh bite washed out. **The call box** ✅ from 50% ([INFECTED.md](INFECTED.md#the-cure)). Tag readings in the journal |
| Champion | The **immunity booster** ✅ (80 Favor, once per life: the next bite doesn't take) | **Lab access** ✅: your samples count 25% more |

The medic and the booster are services at Halcyon's quartermaster, not items. The medic is the
safehouse medic's treatment (a full heal; a fresh bite is cleaned as if declared at a safehouse);
a bite that has taken needs the trial dose or an antidote. The booster can't be taken while
infected, and it's used up by the first bite that would have infected you. *Simplified:* you
visit the medic; calling one out isn't built.
| Hunted | — | The lab takes your samples without Fame ✅, and Halcyon AI shoots you ✅ |

### Consequences for everyone

- **Hunter squads** ✅: every 10 minutes, Bastion or the Red Hand sends an AI group 400 m from
  a player they Hunt, toward where they were. It's hostile to that player within 800 m. At most
  two squads are after one player at a time, and four on the server. A squad only appears where
  no player within 700 m has a clear line over the terrain to it (six tries; else none this round). When that player dies, their squads stand down
  (walk off, or vanish if nobody is near): a new life starts with nobody on its trail.
- **Bounties** ✅: Kill on Sight with any faction puts a public bounty on you, announced every
  10 minutes with a rough location (one notice, however many factions want you). One bounty is paid per death, only to the player who killed
  you (50 Favor and a little Fame), and only for a player kill: dying to the infected, a fall or
  your own hand pays nobody. With several factions after you, the one with the most Infamy pays;
  a faction never pays someone it hunts too, nor a killer in the victim's own group. If none pays, a posting on the Red Hand's
  [bounty board](#the-red-hand) can. *The killer is the last attacker the body's damage manager
  recorded; if that wasn't a player, no bounty is paid.*
- **A tagged pack's leader** ✅: killing one in its prime or older (Halcyon's tag in it) costs the
  nearest player within 150 m +2 Halcyon Infamy: *"a year of data"* ([INFECTED.md](INFECTED.md#the-factions-sides)).
- **Choice missions** ✅: you decide which faction's drop point gets the item. The winner gives
  Fame; the one you double-crossed gives Infamy. Taken to the other side, it pays everyone with
  you whom that side doesn't hunt, and the leader who gave the job neither counts it as help nor
  scales its pay.
- **Borrowed kit** ✅: a claim in a faction's kit, judged by a fixed look, buys passage and a
  moment, never a living; caught, it costs the kit and +3 Infamy ([Borrowed kit](#borrowed-kit)).

## Haven house rules and exile

Who the Haven are, and why they keep these rules, is in [LORE.md](LORE.md), "The Haven". ✅ all.

| Rule | What happens |
|---|---|
| Firing within 50 m of a safehouse | +3 Haven Infamy (at most once every 20 s), unless an infected is within 40 m of the safehouse: then you're defending it. Drawing a weapon is fine |
| Declaring a bite at a safehouse | At any safehouse, once per life: the infection's time left ×1.15 (Shelter), ×1.25 (Community), ×1.4 (Settlement, a medic), ×1.5 (Town). A fresh bite is cleaned instead (90% it never takes). Either way you can't be caught hiding that bite; a later bite in the same life can be declared too, but isn't slowed again. A bite that's been cleaned already uses nothing: come back if it takes |
| Hiding a bite | Inside a safehouse without having declared it, once it shows: 15% a minute in Weakness, 30% in the Final stage, once per life, +15 Haven Infamy. A fresh bite, the Fever stage and a fever suppressant don't show |
| Shooting at, robbing or killing a wanderer | +4 (shot at, once per wanderer), +8 (robbed), +15 (killed) Haven Infamy; they remember you |
| Hunted by the Haven (**exile**) | Safehouse services and the Haven quartermaster refuse you ("the door stays shut"); Haven wanderers refuse you and warn other players near you. The guards shoot an exile only while they carry a firearm within 100 m of a safehouse (with a minute's grace after spawning at one); an unarmed exile just finds the door shut |
| A hungry safehouse and an exile | A safehouse short on food may sell an exile's rough location to the Red Hand (25% every 10 minutes): a hunter squad that really shoots them, and a message to players the Red Hand knows |

Food delivered to a safehouse pays 2 Haven Favor per item, plus 1 Haven Fame per 3 items (at most 2
a hand-in) as small pay through the [errands ledger](ERRANDS.md#pay-nothing-for-free): inside its
Fame ceiling and day's cap, left out of season Fame and records. Food a quartermaster sold or
handed out (requisitioned rations too) counts for nothing at a house, an errand or the fence: *"That's off a
quartermaster's shelf."*
Wanderers' attitude is your Haven standing (Bastion, reversed, for the Deserter) plus their
memory of you ([DESIGN.md](DESIGN.md), "Wanderers"). The Mechanic wanderer sells car parts for
12 Haven Favor (a quarter off if they remember you helped) and gives a fixable car's keys to
players they like.

## Working off Infamy

Infamy never fades on its own. ✅ **One forgiveness budget:** each faction forgives at most **30
Infamy per life** (**Reputation > Forgiveness budget**), whatever the route. Every route goes through
one call, `Forgive()`: the stronghold leader's temper first (a Merciful leader makes each amount
count ×1.2, never the total), then what's left of the budget:

| Way | What it takes | How much | Limits besides the budget |
|---|---|---|---|
| **Amends contracts** | Watched with the giver: the contract works Infamy off instead of paying Fame ([MISSIONS.md](MISSIONS.md)) | the contract's Fame, as Infamy off | — |
| **Paying** | "Pay to make amends" at any quartermaster, in that faction's own Favor | 5 Infamy for 15 Favor; each payment this life costs ×1.5 more | at most 15 of the 30, once every 30 minutes, never below 15 (the Watched line). Kill on Sight: refused |
| **Surrender** | Bastion only, at one of its checkpoints ([below](#checkpoints-roadblocks-and-camps)) | 8 Bastion Infamy | once an hour, by identity |
| **Errand amends** | At 15+ Infamy with the faction, an errand pays amends instead of Fame; the amends family (Make It Right, the Latrine, Clean Slate) is for the Watched ([ERRANDS.md](ERRANDS.md#pay-nothing-for-free)) | 2 Infamy | at most 8 a life per faction from errands, never below 15 |
| **Settling up** | "Settle up" at any quartermaster: spent Fame for Infamy (**Reputation > Settling up**) | 5 Infamy for 10 Fame, the stronghold leader's temper counting as on amends | at most 10 a real day per faction; closed with a faction whose route is fighting yours after the 90% lock. Spent Fame is gone: it can drop your band and your tier (the Fame comes off first, so the tier notice reads both) |
| **Parley amends** | The owed, at a gate ([below](#the-parley-and-the-knock)): one amends contract reserved for 60 minutes though they're Hunted | the contract's Fame, as Infamy off | one offer per parley |
| **Make it right** | A leader who won't deal with you on attitude (−2 or below, not Hunted), with Dialogue > Trees: *"What would it take?"* reserves that faction's amends contract for an hour ([DIALOGUE.md](DIALOGUE.md#make-it-right)) | the contract's Fame, as Infamy off (when Watched or worse) | done, that leader's regard goes to 0 and their crossed memory of you (this life) is cleared, once a season per leader |

**When the budget is spent:**
- An amends contract pays its Fame as ordinary Fame, rival shares and all: "Bastion has forgiven you
  all it will this life. The work counts as Fame instead."
- Surrender is a search and the hold, with no Infamy off.
- Paying is refused with the same line.

So a little trouble (up to 30) can be cleaned completely, and more can only be softened. A Watched
player and a mixed one are treated the same. Every forgiveness is logged. The epilogue week's
*Pay into the book* at Rudi's crossroads works off 5 Red Hand Infamy through it, and **square** with
all four (Infamy under 15 with each, no unpaid line in the book) is checked at processing and the
wipe for the hall's line ([EPILOGUE.md](EPILOGUE.md#the-closing-work)); its dose outlets read standing,
never clothes. The epilogue's **Come In** (an errand card, [ERRANDS.md](ERRANDS.md)) works off 2 Infamy
through it too. *Simplified:* the finale's "Hand it in at a checkpoint" takes no Infamy off.

## Checkpoints, roadblocks and camps

Code: `Factions/RZ_Checkpoints.c`. ✅ all. The Perimeter's sealed checkpoints at the map edge are a
different thing ([DESIGN.md](DESIGN.md#the-perimeter)): they stay sealed to everyone.

**Road posts.** Every 15 minutes, if fewer than two are up, a Bastion checkpoint or a Red Hand
roadblock may go up on a road 350–1,000 m from a random player, within 2.5 km of one of that
faction's holdings (not near the line, a safehouse, a holding or another post). Bastion's turn
comes up as often as Bastion is still out on the roads: its post goes up on a chance of
0.3 + 0.7 × Bastion presence (every time in Early Outbreak, about a third of the time in
Aftermath), else the Red Hand tries; a curfew's night posts always go up. It stays 45
minutes. It's data: the barrier and its crew (the territory guard prefab unless set) exist only
while a player is within 600 m, and its crew counts for retaliation. It needs territory and the
road network. Its fire barrel and nest beside the barrier, and the crew leaning there, are camp
life's ([DESIGN.md](DESIGN.md#camp-life)); none of it changes who is waved through.

| Post | You are | What happens |
|---|---|---|
| Bastion checkpoint | Known or better, not the Wildcard tier, not in Red Hand colours, not in a searched car | Waved through, unsearched |
| | In a vanilla group arriving together (within 50 m) where one member would be searched (a member in Bastion's kit stands their own look) | **The company you keep:** "We search the whole car." Everyone there is searched, and nobody in the car gets a look (**Reputation > Group rules**). Running is still per player |
| | In Red Hand colours (or a worn red armband) | Searched whatever your tier; the worn red is taken: "Evidence." |
| | In Bastion's kit, below Known | A claim: the look at the barrier ([Borrowed kit](#borrowed-kit)). Pass: waved through as one of theirs, unsearched. Caught: held and searched |
| | Signed through by a friend ([vouching](#borrowed-kit)), not in Red Hand colours | Waved through, unsearched; the friend answers for you |
| | The Wildcard tier (Wildcard, Necessary Evil, Wild Child) | Searched like a stranger: "Arms out." They search you because they don't trust you, so running this search costs +6 Bastion Infamy, not +3 (**Reputation > Wildcard at posts**, **Wildcard run Infamy**) |
| | Anyone else | Told at 150 m to walk to the barrier. At the barrier (35 m) for 15 s: searched, and up to three contraband items taken (suppressors, explosives: a setting), plus any worn dead man's kit or kit signed out to someone else ([whose kit it was](#whose-kit-it-was)). Leave the barrier before it's done and you've **run** it: +3 Bastion Infamy and the post fires on you for a few minutes |
| | Hunted | Shot (as everywhere); or [surrender](#surrender) |
| Red Hand roadblock | In Bastion's colours | Turned round at 150 m whatever your tier ("We don't sell passes to the army."), fired on at the barrier |
| | Trusted or better | Waved through |
| | Known, or signed through by a Trusted friend ([vouching](#borrowed-kit)) | Pay the **toll** at the barrier (10 Red Hand Favor) and pass. Can't pay: treated as a stranger. The Wildcard tier pays danger money: 20 (**Reputation > Wildcard toll**, ×2). In Halcyon's colours the toll doubles to 20 ("Beaks pay double."); with both, the higher, never both. The **night rate**: at night within 1.5 km of a road a pack hunts at its hour, one Favor more (*"Night rate. The Postmen are out."*, [INFECTED.md](INFECTED.md#voices)) |
| | In the Red Hand's kit, below Known | A claim: the look at the barrier. Pass: waved through like crew. Caught: the toll for liars |
| | Anyone else | Told at 150 m to turn around. At the barrier: fired on for a few minutes |
| | Hunted | Shot |

A cleared pass lasts while the post is up; walk well away without clearing it and the next approach
starts again.

### Surrender

At a Bastion checkpoint, a player with Bastion Infamy (15 or more: Watched or Hunted) who walks to
the barrier **with no firearm** and waits 10 seconds gives themselves up: searched, 8 Bastion
Infamy off (from the [forgiveness budget](#working-off-infamy); once it's spent, nothing comes off),
and Bastion's AI holds its fire on them for 5 minutes, whatever their standing,
fights or grudges say (and forgets those fights). A Hunted player walking up unarmed within 150 m
isn't shot while they come in: only while they get closer (or wait at the barrier), until they've
hung back for 30 s in one approach, with their surrender hour free, and not while a Bastion squad sent after them
is within 1 km. A post is no place to sit out a hunt. Once an hour per player; within the hour it's just a search.
Armed and Hunted, they shoot.

**Camps.** A faction's outposts and stronghold (and the lab, for Halcyon) warn off a player the
faction watches (Watched) at 150 m, and at the gate
(50 m) turn them away: unarmed, the gate stays shut; armed, they open fire for a few minutes.
Bastion's and Halcyon's gates treat anyone in the Red Hand's colours the same way. A friend in the
same group whom the camp would let in is told "Your friend can come in. You wait outside." In the
camp's own kit while Watched, the gate is a look instead: pass and you're inside for 10 minutes
(**Hostility > Gate pass minutes**); services still go by your real standing.
The lab lets a bitten player in unarmed for the trial dose, Watched or not and whatever they wear,
until they've had it. Hunted players are simply shot, unless they're owed a
[parley](#the-parley-and-the-knock). The Haven's holdings follow its house rules instead.

**Safe zones.** Strongholds and the lab: within 80 m you're told the rule as you come in
("weapons slung"), and firing there, with no infected within 40 m, costs 2 Infamy with the
faction (at most once every 20 s) and draws their fire for a few minutes. Safehouses have their
own [house rules](#haven-house-rules-and-exile).

Every "fired on for a few minutes" here is the same timed grudge as
[shooting back](#what-makes-ai-shoot-you): it restarts, never adds up.

### The parley and the knock

Code: `Factions/RZ_Parley.c`, on the checkpoints' tick. ✅ (**Reputation > Parley**)

**The owed:** Hunted or Kill on Sight with a faction, with Earned Fame 15+ with it this life and a
best deed with it this life. At a Bastion checkpoint or camp gate, a Red Hand roadblock or camp gate,
or Halcyon's lab or field-station gate, an owed player gets one hearing instead of a bullet:

1. **The shout** at 150 m: "Stop there. Empty hands, or not at all."
2. **Walking in** empty-handed: no firearm in any slot or anywhere in the pack, and no grenade or
   charge (`RZ_Util.IsEmptyHanded`, every tick). They hold fire only while you're closing, 90 s at
   most, a few seconds at a time (`StandDown`, never forgiving).
3. **At the barrier** (35 m), 10 s searched. Nobody walks out to meet you. The nearest leader of that
   faction speaks over the radio from inside: one deed callback ("You {best}. You also {worst}. Both
   go in the log. Talk.") and one **amends offer**: the Haven's Medicine Run (else Survivors Trapped),
   Bastion's Burn the Bodies (else Recon), the Red Hand's Fence the Goods or Halcyon's Field Samples,
   reserved for you for 60 minutes though you're Hunted. It finishes by its own rules where it says;
   nothing is handed in at the gate. If none can be placed: "Come back later", and the 2 hours aren't used.
4. **Walking away**, the hold lasts while you're leaving, to 200 m or 90 s.

**Closed** (no shout at all, so they shoot as usual): a hunter squad or fight of that faction has you
within 1 km; another player is within 150 m of the gate; this identity parleyed with that faction in
the last 2 real hours (server time: a reconnect or a restart resets nothing). **Breaks:** arming
yourself, firing, coming back past the barrier, a squad closing in ("They've got friends coming. So
have we."). **Pauses:** another player arriving within 150 m ("Back off. We're talking."), or anyone
else's fire within 150 m of the gate (20 s, and the shooter takes that faction's grudge). That faction's
hunter squads skip you while it runs, and no threat is made through talk during one. Elsewhere
that faction's AI still shoots you, and player-against-player damage is unchanged. *Simplified:* the
engine's hold is faction-wide; it's made local only by its conditions.

**The Haven's knock.** The Haven shoots only an exile carrying a firearm near a safehouse, so there's
no hold: an owed exile talks to a safehouse's named person within 15 m, empty-handed and alone, and
the council head answers through the shut door with one callback and the offer. The same 2-hour
limit. *Simplified:* the knock is the named person's talk, not a separate action on the door.

What the posts and camps say to you (the warnings, the search, the toll, the gate, the safe-zone
rule) shows as a subtitle at the bottom of the screen, not a notice card; the outcomes with
numbers (a surrender's Infamy, the toll paid) stay cards ([UI.md](UI.md#subtitles-and-the-reader)).

## Reputation clues

Something other players can see ✅. At **Trusted** the Red Hand gives you a **red armband** and
the Haven an **amber ribbon** (once per life, into your pack, or at your feet if it's full).
Wearing it is up to you; worn, anyone can read whose you are. The armband's name carries the Red
Hand disguise keyword, so it counts as one piece of the Red Hand's [kit](#borrowed-kit). Both need
their prefabs set; without them nothing is given. *Simplified:* it's an item to wear, not worn
automatically.

## Faction leaders

Every place a faction holds has a named leader ([LORE.md](LORE.md), "Who's in charge"), so
reputation is with people as well as factions. Where they sit and how they're simulated is in
[DESIGN.md](DESIGN.md), "Leaders". Code: `Factions/RZ_Leaders.c`, `Factions/RZ_LeaderVoice.c`. ✅ all.

Each leader has a seeded name, a title, a one-line backstory and **two traits**, and is saved.
Stronghold leaders never die, are never replaced and are always Loyal.

### Traits

Every effect is small and capped: one leader's combined multiplier never goes past ±20%.

| Trait | On contracts they give | Elsewhere |
|---|---|---|
| Greedy | Favor ×1.2; cross-faction trade jobs a little more often | Their quartermaster charges ×1.15 |
| Paranoid | Fame ×0.85 | Stronghold: Infamy with the faction ×1.15. Remembers being crossed harder |
| Merciful | Amends work off ×1.15 more Infamy | Stronghold: Infamy gained ×0.9, worked off ×1.2; amends contracts more often. Forgives faster |
| Ruthless | Darker personal jobs more often | Stronghold: contracts that hurt a rival more often |
| Idealist | Hopeful personal jobs more often, dark ones less | Stronghold: harmless contracts more often. Halcyon Idealists remember anyone who sells samples to the Red Hand |
| Pragmatist | Cross-faction personal jobs more often | Deals with people their faction hates (bad standing counts as Watched at worst). Stronghold: choice missions more often |
| Veteran | Favor ×1.1; Fame ×1.15 for Soldiers and Police; tough jobs more often | Stronghold: Trusted+ contracts more often |
| Talker | Talky jobs more often | Two rumours instead of one; their rogue jobs leak more easily |

"Stronghold" effects come from the stronghold leader and apply faction-wide. Contract weights
change by at most ±20%.

### Loyalty

Outpost and safehouse leaders have a loyalty: **Loyal** (60+ on a hidden 0–100 score),
**Independent**, or **Rogue** (below 25).

| Loyalty | Their attitude to you | Work |
|---|---|---|
| Loyal | Your faction standing plus their memory of you. Hunted by the faction = no | Voice the stronghold's radio contracts; no personal jobs |
| Independent | Half your standing plus double their memory | Off-book personal jobs |
| Rogue | Double their memory; bad standing doesn't count if you've helped them | Off-book jobs against the stronghold; their quartermaster sells you Known items even if the stronghold hunts you |

**Memory** (per player, saved): *helped* (+1), *saved* (killed infected at their door: +2),
*crossed* (shot near them, turned them in, worked against their faction: −3; −2 for a Merciful
leader, −4 for a Paranoid one). At −2 or below they won't talk to you. Their quartermaster's
prices move ×0.9 for people they owe and ×1.1 for people who crossed them (doubled for
Independent and Rogue leaders), within the ±20% cap.

**What moves loyalty** (at most 15 points an hour, except the big events):

| Event | Change |
|---|---|
| A safehouse runs out of food / is fed again | −5 / +2 |
| The faction's contracts near them fail / succeed | −2 / +1 |
| They finish a rogue job without getting caught | −3 |
| Their own off-book delivery against orders (Our Own First, Hearts and Minds) | −2 |
| Every hour, back toward their temperament (55; Idealist +10, Paranoid +5, Pragmatist −10, Ruthless −5) | ±1–2 |
| Rein In: terms read / counter-offer taken | +25 / −5 |
| A rogue job is discovered and they keep their place | +25 |
| A new leader | rolled fresh: 40% Loyal, 40% Independent, 20% Rogue |

At most one leader per faction is Rogue at a time, so no faction ever splits. When a leader
turns Rogue, the stronghold puts out **Rein In** ([MISSIONS.md](MISSIONS.md), "Leader missions").

**Discovery.** A rogue job is discovered with a 15% chance, +35% if the faction's own people saw
you doing it, +10% for a Talker leader, −5% for a Paranoid one. Discovered: everyone paid takes
**+10 Infamy** with the faction, and the leader is removed (half the time) or pulled back into
line. You can also turn a rogue leader in: most rogue deliveries have a drop at the stronghold.

### Talking to leaders

Talk to a leader (their body, or "Talk to whoever's in charge" on the quartermaster) and the
talk screen opens ([DIALOGUE.md](DIALOGUE.md#the-talk-screen)): **Ask for work** (their personal
job, else the stronghold's contracts), **Heard anything?** (other leaders' loyalties, hungry
safehouses, noise, rumours) and **What do you make of me?**. Without the talk screen these are
actions on the leader. Radio
contracts are briefed in the voice of the nearest Loyal leader of the giver, whose traits set
the pay.

**First contact.** The first time you talk to a leader they say who their faction is, who
they are, and what they think of one other faction (two for a Talker), with where its nearest
leader sits. Hooks: `RZ_Leaders.GetFirstContactLine(faction, leaderId, playerId)` and
`RZ_Leaders.DescribeStronghold(faction, fromPos)`.

**Deaths.** Off-screen, every half hour, an outpost or safehouse leader can be bitten (1–4%) or
killed in a raid (1%, doubled at a hungry safehouse). Killing one yourself is +15 Infamy with
their faction. A new leader takes the chair after 2 hours. All of it is headline news.

## Morale, needs, truces and feuds

Factions change with the world (code: `Factions/RZ_FactionLife.c`). The radio chatter, life
events, remembered moments and rumours this feeds are in [DESIGN.md](DESIGN.md), "Faction life".
Every effect is small, capped and recovers. ✅ all.

**Needs.** Each faction has one need at a time, worked out every 5 minutes. Leaders and named
people mention it, and contracts that answer it are picked more often (×1.2):

| Need | When | Contracts it favours |
|---|---|---|
| Food (Haven) | a safehouse is short, or a Haven truck was lost in the last hour or so | deliveries and choice missions |
| Supplies (Bastion, the Red Hand) | one of its trucks was lost recently | deliveries and choice missions |
| Security | a leader died or an outpost was lost recently (the Haven: a death) | holds, kill missions, work against a rival |
| Revenge (Bastion, the Red Hand) | a feud; for the Red Hand also an outpost lost | work that costs a rival Infamy |
| Samples (Halcyon) | the cure isn't done | Halcyon's contracts |
| People (Haven) | its safehouses hold fewer than 6 people each on average | rescues and escorts |

**Morale** (5–95, starts at 60, recovers 2 points every 10 minutes toward 60): a lost truck −10,
a lost outpost −10, a dead leader −8, Under Pressure −5 (+5 when it ends), a lost final stand
−15 (Halcyon); a delivered truck +4, a new or won outpost +5, a breakthrough +10 (Halcyon), a
contract for them +1 (−1 if it failed).

| Morale | Their people | Their shops | The Haven |
|---|---|---|---|
| Under 30 | jumpier: retaliation reaches ×1.25 as far and lasts ×1.25 as long | ×1.1 | rescued survivors walk off 2.5× as often (at most half) |
| Over 75 | calmer: ×0.85 | ×0.95 | half as often |

A demoralised faction also puts out a little less work (×0.9). Contract weights from needs and
morale stay within 0.85–1.2.

**Truces and feuds.** Temporary: 45–90 minutes each, at most two at once, one per pair of
factions, and a 30-minute pause after one ends.

| | Starts | Rival Infamy between the two | Territory |
|---|---|---|---|
| Truce | every 30 minutes, 30% for a pair of rivals with low morale (both under 45) or a bad outbreak | halved | neither takes the other's towns |
| Feud | a truck lost near the other's holding (50%), a stolen truck delivered to them (60%), an outpost taken by them (60%); or the 30-minute roll for rivals who've been losing | ×1.5 | the stronger takes the weaker's towns with no strength margin |

They're headlines, never alerts.

## The Long Table

A tiny cannibal family in one farmhouse ([LORE.md](LORE.md#the-long-table)), some seasons. Where
the house is and what it does in the world is in [DESIGN.md](DESIGN.md#the-long-table); its
questlines are in [QUESTLINES.md](QUESTLINES.md#the-long-table). ✅ all (`World/RZ_LongTable.c`).

**Not a fifth faction.** They're four to seven people with nothing to sell that a player should
want. A Fame ladder would make eating people a way up, and a shop would be loot paid for in
people. So there's no Fame, Infamy or Favor with them, no tiers, no shop, no perks, and no bit
in the hostility mask. They remember each character (for this life) in three words: *guest* (ate
with them), *family* (A Place at the Table) or *crossed* (refused the host, was set on, shot one of
them, or took proof to someone). The crossed get nothing at the farm but a shut door.

| Dealing | What happens |
|---|---|
| **Eat with them** ("Sit down and eat", on any of the grown-ups) | A bowl at the long table, grace (the name read is the last one who never arrived) and true road news (a truck on the road or about to leave, a hungry safehouse, where the infected are stirred up). Once a visit; the first meal of a life shows the boots by the door. Family also take a jar away: one food item from the loot pool, once a real day. Safe, except under the guest rule |
| **Trade** ("Trade for a jar") | One piece of ammunition or medicine (Long Table > Trade keywords) for a jar: a food item from the loot pool, a day written on the lid. Once a visit |
| **Declare your bite** | Infected: a blessing, and they'll never set on you while you're infected. Not: "Good. Then sit." |
| **Expose them** | Proof to a faction's leader ([C1](QUESTLINES.md#c1-the-house-that-went-quiet), stage 4): +10 Fame with that faction (the Red Hand +4; +5 more for [C2](QUESTLINES.md#c2-a-place-at-the-table)'s undercover guest) |
| **Break them up** | One of C1's endings, C2's fight at the table, or any fight they lose |

On the third meal of a life the host makes his offer (C2), to anyone not Trusted with the Haven.

**The guest rule.** A player who ate with them on this visit and is still within 30 m of the house
between 20:30 and 05:00 (from a minute after the meal), with no other player within 100 m and not
infected, is set on by the family: a fight ([DESIGN.md](DESIGN.md#the-long-table)). Once a life,
and never for family; for someone who refused the host's stray, at any time of day, meal or not.
Anyone else, at any other time, is a guest. Without the fight prefab it's a narrow escape. Either
way the house is crossed with them, and suspicion rises 15.

**Shooting them.** Until they're found out they're a quiet farm family, so killing one costs +6
Haven Infamy, as for gunning down Rival Survivors who'd fought nobody. Once they've set on you,
once they're found out, with the infected on them, or for a questline's player who has found the
proof or come to end it, it costs nothing with anyone.

**When they're found out,** every faction reacts in its own way, within the usual caps:

| Faction | Reacts | For players |
|---|---|---|
| The Haven | Morale −8 (shame), and a rumour that it was one of theirs, and no more; wants them walked out (C1) | Proof: +10 Haven Fame. Family of the house when it's found out: Haven Infamy +40 |
| Bastion | A rumour: an infected cluster, containment. If nobody else ends it within a real day, it burns the farm | Proof: +10 Bastion Fame. Family: Bastion Infamy +20 |
| The Red Hand | On the next round the boss purges the crew that bought from them (the crew boss of the Red Hand outpost nearest the farm is replaced; Red Hand morale −3), once, unless the buyer is already gone; the open channel says "pork" and laughs | Proof: +4 Red Hand Fame |
| Halcyon | A rumour about whether they ate the bitten; wants one of them alive, and the mother's kitchen notebook (C1) | Proof: +10 Halcyon Fame; the volunteer brought in: research +2 |

Family who are online are named in the headline; those offline get their Infamy on their saved
profile. Trading with them before anyone knew costs nothing. Only what a player did knowingly
(the questlines' *(you)* flags) is held against them.

## What makes AI shoot you

AI shoots individual players, not everyone on the player faction. ✅ all. Each faction's AI
fires on a player when one of these is true for that player:

| Faction (AI) | Default stance | Shoots you when |
|---|---|---|
| The Haven (guards, occupants, survivor groups, wanderers) | Peaceful | You're Hunted by the Haven **and** carry a firearm within 100 m of a safehouse; or you're near an armed wanderer with a grudge against you |
| Task Force Bastion | Wary: no shooting (the Perimeter works as before) | You're Hunted or Kill on Sight by Bastion |
| The Red Hand | Neutral: they leave you alone | You're Hunted or Kill on Sight by the Red Hand; or you're near one of their crews in a fight |
| Halcyon | Peaceful | You're Hunted or Kill on Sight by Halcyon |

The stance is a setting per faction (**Shoots from**: Hunted, Watched, Strangers or Never).
Setting the Red Hand to *Strangers* makes its crews shoot anyone below Known, which makes the
Known toll pass the difference between being shot and not.

**Fights.** Some AI is spawned to fight, whatever your standing:

| Who | Shoots |
|---|---|
| A hunter squad | The player it was sent after, within 800 m |
| A road post or camp | For a few minutes: a player who ran a Bastion checkpoint, came up to a Red Hand roadblock without paying, or came armed past a camp's gate after the warning ([Checkpoints](#checkpoints-roadblocks-and-camps)) |
| The Red Hand camp in Raid the Red Hand, the raiders in Trader Caravan and Lab Defence | Every player within 350 m of them while they live |
| The killers' camp in The Witness | Players within 150 m who aren't at least Known with the Red Hand |
| The crew in the trees at a Distress Call (its loop and bait truths), the carriers at a Crash Site already taken, the convicts in Prison Break | Players within 300 m (250 m for the convicts) who aren't at least Known with the Red Hand |
| The Sniper | Every player within 500 m, Red Hand friends included |
| A faction's group at the cure finale's stand | Within 1.5 km of the lab during the stand, players locked to a side fighting that faction's route ([FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room)); never Halcyon's, and not in Tally mode |
| Rival Survivors (an unaligned group on `SURV`) | Only a player Hunted by any faction or Watched by two, once they come within 120 m (the Haven bit, like a wanderer's grudge) |

In Trader Caravan the trader is put on a variant the Red Hand is hostile to, so the raiders go
for him while he stays friendly to players.

**Wanderers fight back.** A wanderer whose body is an armed AI group shoots, within 80 m, a
player who robbed or shot at them; the Deserter draws on Bastion's Champions. The Kid never does.

**Shooting back.** Shoot or kill a faction's AI that ReforgerZ spawned (guards and named people
at holdings, supply truck crews, safehouse guards) and that faction's AI within 300 m turns on
you for 3 minutes. Another hit restarts the timer, never adds to it, so it can't become lasting
hostility. A kill also costs +3 Infamy with the faction, at most 12 an hour. It doesn't count
with an infected within 40 m of the victim, near a mission fight or hunter squad, or if that
faction already shoots you. Leaders, wanderers and mission fights keep their own rules. A
setting turns it off.

**Players and each other.** Player-against-player damage is unchanged. At the AI level, players
stay friendly to each other, with one exception: while the Haven shoots you, you also count as
hostile to players nobody is shooting, which can show in name tags and vanilla team-kill accounting.

**How it works.** Reforger decides hostility per faction, so each player's *character* is moved
onto one of 15 hidden copies of the player faction (`SURV_H1` to `SURV_H15`) whose number is a
bitmask of the AI factions that shoot them: 1 the Haven, 2 Bastion, 4 the Red Hand, 8 Halcyon.
A modded faction check decodes the key. The player controller stays on `SURV`, so the deploy
menu, chat, groups and map markers don't change. Standing is re-read when a tier changes or you
spawn; nearness is checked on the server tick.

**Fallback.** If AI doesn't follow the modded check, set **Mode** to *Outlaw*: anyone any
faction would shoot goes onto one `SURV_OUTLAW` faction, hostile to every AI faction. It's
blunter (a Bastion-Hunted player is then shot by the Haven and Halcyon too) but uses only
vanilla relations. Setup is in [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md), "Factions and loadouts".

### Borrowed kit

Code: `Factions/RZ_Disguise.c` (kit, claims, caught), `Factions/RZ_DisguiseWatch.c` (while you pass,
word gets round, vouching), `Factions/RZ_DisguiseMarks.c` (whose kit it was, the nail list, the
board), `Factions/RZ_DisguiseClaims.c` (cover details spoken, the look's last minuses, the HUD, the
majors' tricks, the last week); the look in `RZ_Checks.LookScore`. ✅ (all of borrowed kit; settings
**Factions > Hostility > Disguises**). Wearing a faction's kit does nothing by itself: you **claim** to be one of
them, and they look at you. A pass buys passage and a moment, never goods, standing, services,
contracts or the cure.

**Kit**, read from the loadout slots every 10 s (never from a pack; VERIFY on a player character):

| Term | Means |
|---|---|
| Their colours | 2 pieces matching a faction's keywords (Bastion, the Red Hand, Halcyon; the Haven has no uniform) |
| Full kit | 3 pieces, one on the head: a helmet, cap or beret (Bastion), a bandana (the Red Hand), the beak (Halcyon: a hood or mask with a respirator, plus the coat; the mask is never a tell) |
| Own colours | Their kit while you're Known or better with them (access rank 1+): just your gear, never a disguise |
| Issued kit | Their kit signed out to you by their own quartermaster: still your own colours for a real day after your standing slips below Known (**Issued grace hours**, 24), never while Hunted |
| In their kit | Their colours while below Known with them. Nothing happens until you claim |
| Tells (−1 each; two and you can't claim) | another faction's colours worn at the same time; the wrong weapon (Bastion: any firearm that isn't a service weapon, by keyword; Halcyon: any long gun) |

**Claims.** Walking up to a road post's barrier in full kit (**Claim in full kit**), or talking to a
post's speaker in their colours; at an outpost's gate while Watched; a mission's trick
(`RZ_Ways.PassAs`, once per player per person per job, one mission trick per life per faction per
season day). The Hunted and Kill on Sight can't claim. At 150 m you're shown what they'll see as one
notice: "[Full kit +2] [Background +1] [They know your face −1] Pass as one of them: 2 of 2."

**The look** (difficulty 2 at barriers, gates, again looks and roll call; a mission sets 1–3):

| Plus | Minus (each) |
|---|---|
| Their colours +1, full kit +2 | They know your face: Watched or Uneasy, or a leader of theirs met this life |
| A matching background +1: Soldier or Police (Bastion), Ex-Con or Raider (Red Hand), Lab Tech or Doctor (Halcyon) | Caught by them earlier this life |
| One cover detail +1: Bastion's **word at the wire** (changes daily, read on Bastion's secure net when a Radio Operator cracks it), a crew name (Red Hand), a name you've heard (a leader of theirs you've met, or a dead man's from Bastion's nightly call), a dead researcher's notebook number (Halcyon, one look), a mission's paper | On the bounty notices (Kill on Sight with anyone) |
| Dark or fog +1 (your light level as stealth reads it, else 20:30–05:00) | Each look already passed at this place; each tell; each point-out (two at most); under half health; the infection showing |
| | **Word's round** there; **seen putting it on** (below); running; a gun in your hands (both VERIFY); a red armband the Red Hand gave someone else |
| | **Something of theirs on you**: a mission item from a job against them (its rival) in your inventory; **sworn to another side**: after the 90% lock, every faction whose route you didn't choose, by identity, until the cure |

Instant fail: the infection showing at Halcyon's look ("They see bites for a living."), three
catches this life ("Your description is on the board at every post."), blown, kit that remembers
whose it was ([below](#whose-kit-it-was)), and a dead man's name given at his own post.
**Claim lines** (in conversations, [DIALOGUE.md](DIALOGUE.md#claims-in-someone-elses-kit)): Bastion's
word at the wire is a choice of three (a wrong one fails at once). Other cover details are spoken as
choices too, each shown only when you really have it: a leader of theirs you've met (`cover:name`), a
dead man's name heard on the nightly call (`cover:dead`), your crew name (`cover:crew`), a dead
researcher's notebook number (`cover:notebook`), a mission's paper (`cover:paper`, when a stage hands
one: `RZ_DisguiseClaims.SetPaper`). A spoken detail counts at your next look with them for 10 minutes
and names that very detail (so a dead man's name given at his own post still fails); one you don't
have is a lie, caught at once. **One claim line per speaker per life.** Without a conversation the
best detail you have counts at the look, as before.

**Passing.** At a barrier: cleared for that post while it stands. At a gate: 10 minutes inside. While
passing, that faction's AI looks past what it holds against you (not a fight's bit), your persuade
score with them counts as 0, and you can't threaten their people.

**While you pass** ✅ (`RZ_DisguiseWatch`):
- **Again looks.** Every 3 minutes (**Again look minutes**) that one of theirs within 15 m can see you
  (one sight trace from their eyes to your chest, VERIFY), they look again, one step harder each time
  (the looks passed at this place). A notice comes 10 s before: "Someone's looking at you again." If
  nobody of theirs sees you when it's due, the look is **owed** for 30 minutes: claim again and it
  comes 10 s after the claim. The state is kept by identity, so logging out doesn't clear it.
- **Roll call** (**Roll call**). At the shift change (05:30 and 17:30; the Red Hand's dusk meal at
  18:30) everyone passing within 150 m of one of their holdings (or the lab) is looked at.
- **Seen putting it on.** When their kit goes on with one of theirs within 30 m who can see you,
  every look within 150 m of there is one step harder for an hour.
- **Leaders and the wire.** A leader of theirs within 15 m, or being inside a stronghold's or the
  lab's wire, is a catch. **The Rogue's price** (**Rogue's price**, 20): a Rogue outpost or safehouse
  leader says "Nice jacket. Twenty Favor and I never saw it." Still within 15 m at the next check,
  you pay his faction's Favor and pass at his seat for 10 minutes; walk off first and nothing happens;
  can't pay and you're caught. He writes it down: if his rogue work is found out
  ([Loyalty](#loyalty)), everyone who paid takes +5 Infamy (offline: when they're next on). Loyal and
  Independent leaders never play along.
- **A hit on their people** (retaliation sees it) always gives you away, with only an infected within
  3 m of the victim as an excuse: within 35 m of their people it's a catch, else you're exposed.

**Caught** (face to face: a failed look, a leader of theirs within 15 m, inside a stronghold's or
the lab's wire): blown with them for 60 minutes of play; one of 3 catches per faction per life;
+3 Infamy (never doubled, at most 6 an hour, and on its own it never takes you from below 15 to 15);
the kit taken (**Confiscation**; VERIFY deleting worn items); held for a moment (a short stand-down,
never over one of their fights, which carries on). Then Bastion searches you, the Red Hand takes the
toll for liars (10 Favor, else a beating: 20% of your health, never below the floor; with infected
near, +2 Infamy instead), and Halcyon walks you off the gate politely. With a mixed name they say
it to your face ("Hang on. You're that Merciful Thug."). **Walking off while held** at a Bastion
barrier (30 m before the hold ends) is running the checkpoint: its Infamy and the post's fire.
**At a post that would shoot you anyway** (their AI wants you by standing; only the kit held them),
a failed look goes **straight to the guns**: no hold, no search, no kit taken, one urgent line and
their grudge, with the catch's Infamy as usual ([WRITING.md](WRITING.md#faction-kit-and-disguises)).
Log line: `Disguise: <player> caught by <faction> at a hostile post: straight to the guns`.
**Exposed** at range (firing within 150 m of their people with no infected within 40 m of you, or
a hit on them): blown, nothing else. Surrender comes first: a player who qualifies for surrender at a
Bastion barrier is never caught there.

**Word gets round** ✅ (**Word round minutes**, **Word round radius**). A catch, or a kill of one of
theirs that retaliation counts (never a hit or gunfire), makes every look at that faction's nearest
holding within 1.5 km, and anywhere within 1.5 km of it (its road posts), one step harder for
everyone for 30 minutes. One per holding: another trigger restarts it and never queues. The first
trigger is said on their open channel, without a name (*"All stations: civilian in our kit at
Morton. Check faces. Bastion out."*; the Red Hand: *"Someone's wearing hands we didn't paint. Ask
names at the toll."*; Halcyon reads a security notice), and leaders and wanderers repeat it as a
rumour for an hour. With no holding in reach, the open channel tells the catch as before.

**Vouching** ✅ (**Vouching**). The one vouch ([DIALOGUE.md](DIALOGUE.md#several-players)'s "Vouch", with its
lie liability) also signs a friend through: Known with Bastion or Halcyon
(Trusted with the Red Hand), not Wildcard, in your own colours, at a road post's barrier or
Halcyon's field station or lab gate, with your friend (not Hunted) within 10 m. Bastion waves them
through unsearched; the Red Hand charges them the Known toll instead of turning them round; Halcyon
lets a Watched visitor in for 10 minutes. You **answer for them** for 30 minutes and 1 km: if
they're caught in that faction's kit, or hit or kill its people, you take +5 Infamy with it
(**Vouch Infamy**, once per vouch) and can't vouch for anyone for 2 hours. A lie found out under a
vouch bans you for 2 hours too. Without a vouch everyone is checked on their own.

#### Whose kit it was

✅ (`RZ_DisguiseMarks`, **Kit marks**). Kit remembers whose it was (at most 200 marks). **Across a
restart:** the marks on pieces players wear are saved by the wearer's identity and the piece's prefab,
and go back on a worn piece of that prefab and faction (without a mark) when that identity is next read
wearing it (vanilla persistence brings the kit back; VERIFY a worn piece's root parent is the character).
A piece in storage or on the ground can't be found again on its entity, so it doesn't launder by a
restart: each mark of that kind (a dead man's, off the turned, signed for) leaves a slot by faction and
prefab, and the first unmarked piece of that prefab read on someone after the restart is of **unknown
origin**: −1 at their look (*[Unknown origin -1]*), for 3 real days. Bought fair or signed for since,
it carries its new mark instead.

| Kind | How a piece gets it | At their look | Elsewhere |
|---|---|---|---|
| A dead man's | Worn by one of their people when he died (any death retaliation sees; VERIFY the clothes stay on the body) | Fails: *"Kral? Kral went on the nail on Day 12. Hands on the barrier."* The Red Hand's passes only if bought fair | Put on: *"The name tape says KRAL."* Taken at any Bastion search |
| Off the turned | Worn by an infected whose kill the spawner counts (VERIFY) | Fails: *"That's off one of ours. Where is he?"* | Put on: *"Bastion green, burnt at the cuff. Off one of the turned."* Only good as armour |
| Signed for | Their kit, sold by their own quartermaster: signed out to the buyer | Someone else's fails: *"Signed out to Mara. You're not Mara."* The buyer's own is issued kit | Taken at any Bastion search; the buyer is told it came back |
| Bought fair | Anything the Red Hand's quartermaster sells | Clean kit | The clean way in: the Red Hand's Trusted shelf, one Bastion set and one Halcyon set a life, 40 Favor each (shop actions with **Once per life**) |
| Given | The Red Hand's armband and the Haven's ribbon, to whoever reached Trusted | A red armband the Red Hand gave someone else: −1 at its look | A ribbon given to someone else: Haven people won't talk to you ("Where did you get that ribbon?"; VERIFY a worn piece's parent is the character) |

**The nail list** (saved). Every Bastion, Red Hand or Halcyon person retaliation sees die gets a
seeded name (never LORE's or the finale's canon people) and goes on the list: Bastion's nail, the
Red Hand's book, Halcyon's authors (with a coat number). The last 30 per faction are kept, with where
and when, and whether a player killed him. At 20:30 Bastion's open channel reads the names that went
on the nail since the last call; whoever hears it (a radio, and Known with Bastion or a Radio
Operator) has a dead man's name to give at a look. At the change of shift Halcyon reads the coat
count (*"Coats returned: 7 of 9."*) and the new authors. The cards that bring a man's tag home mark
him "home" or "kit lost" ([ERRANDS.md](ERRANDS.md#the-cards)).

**Bastion's board** (saved, the last 20). Each Bastion catch goes on it: the day, the place, the
kit and how many things were wrong, and the name given (a crew name or a dead man's), never the
player's. Its newest line is read at the barrier's speaker and in a Bastion leader's news. Every
third Bastion catch of the season, the Red Hand's channel laughs about tailoring. The season's counts
(catches per faction, jackets home, coats out and back, looks passed by identity and who was ever
caught) are saved for the hall of fame.

**Doubling only for what they saw:** Infamy done in sight (not a share) while passing, within 150 m
of their people or holdings, doubles; within 35 m it's a catch, otherwise you're exposed. Rival
shares never double. **Never fooled:** fights (hunter squads, mission groups, grudges), and within
1.5 km of the lab during the final stand no disguise clears anything.

**Colours** (**Colours**): their enemies react to anyone in a faction's colours, friends included,
at their posts and camps ([Checkpoints](#checkpoints-roadblocks-and-camps)): refusals and the timed
grudge only, never a standing bit. Also: the Red Hand's quartermaster and fence won't deal with
anyone in Bastion colours (*"Take the costume off, soldier."*), and Bastion's won't in the Red Hand's;
the Haven's people, council heads and wanderers won't talk to anyone in Red Hand colours, and the
Deserter not to anyone in Bastion's jacket; during a Bastion and Red Hand feud, a Red Hand crew
within 150 m that can see you in Bastion colours opens up (the timed grudge). Under a strict stance
(**Shoots from** Watched or Strangers) a faction's AI holds its fire on someone in its kit who is
more than 35 m from its people, without a claim, until they come closer.

#### The look on the HUD and the standing page

✅ (`RZ_Hud_Look`, [UI.md](UI.md#screens)). While a look is coming (the 150 m notice, an again
look's 10 s warning), the HUD shows the Watched eye and **"PASS AS BASTION: 2 OF 2"** (or "SEEN
THROUGH"), owner-only, sent only when it changes; it goes once the look is done, the kit comes off or
you're 200 m on. The notices still carry the tags. The notebook's Standing page has, above the
leaders of Bastion, the Red Hand and Halcyon, **IN THEIR KIT**: whether they know your face, catches
this life (of 3), blown minutes left, their tells, and what their enemies do to their colours.

#### Other people's stories, the stand and the majors

- **The Haven's gossip.** A catch is told sideways, about food, at most once in six hours of play,
  with the nearest town, as a rumour wanderers and leaders repeat for 12 hours: *"Somebody came to
  Morton in a soldier's coat asking for bread. We gave him bread. The coat didn't fit."*
- **The stand's spare rule.** A fight zone that spares a faction's friends (Known or better) also
  spares a player locked on that faction's route who wears its colours.
- **The majors' tricks** (hooks the majors call: `RZ_DisguiseClaims.RosterTrick`, `RecordsTrick`,
  `MusterCallback`). Await Relief's Act IV roster in Bastion kit at night: the gate sentry's look at 2,
  then the duty board's at 3, a name off the nail as the cover detail; passing the board signs the
  roster as him, and the muster can say *"And whoever signed the roster as Kral: he's still dead."*
  Proof's records room in a Halcyon coat at night: the duty officer's look at 3, a real notebook number
  as the cover. Night is 20:30–05:00. Which act offers them is the majors' ([QUESTLINES.md](QUESTLINES.md#the-majors)).

#### The last week

✅ After the cure (the epilogue, [EPILOGUE.md](EPILOGUE.md)):
- **The lock's −1 ends** at the cure: looks go back to their season rules.
- **Bring them home.** A dead man's kit worn within 40 m of his post is taken in: Bastion puts his tag
  on the nail and reads his name at the roll, the Red Hand writes his line in the book as "sent off",
  Halcyon reads his notebook into the record and adds him to the authors (his fate on the list goes to
  home). Kit off the turned is taken in at any Bastion holding: *"He's home. Mark him."*
- **Give back borrowed kit.** Signed-for kit worn by someone else is handed back within 8 m of its
  faction's quartermaster: no catch, no Infamy, and the buyer hears *"Your jacket came home. Clean,
  mostly."* if they're on.
- **Lines**, once, at the first kit read after the cure, on each faction's open channel from the
  season's counts: Bastion's jackets on the wire, the Red Hand's best-dressed of the season (the most
  looks passed by someone never caught, at least 3: an honour line, its name frozen and filtered, an
  `/anonymous` player is "somebody"), Halcyon's coats returned, the Haven's ribbon line.
- **The hall of fame** gets "Caught in someone else's kit: N times." and "Never caught: …" (the same
  honour line).
- *Simplified:* clean kit from the Red Hand's shelf isn't sold back at the fence (that needs a shop
  action on the fence's prefab); the factions' lines are told once, not at each holding.

#### Why it can't be abused

The author's rule: disguises and faction armour are fine, but very hard to abuse. Each attempt, the
rule that stops it, and what the log shows ([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md#7-log-lines-to-check)):

| Attempt | The rule | Log |
|---|---|---|
| Walk in and help yourself | Nothing a claim buys pays: shops, services, leaders and hand-ins go by real standing. The wire is a catch | `caught by` |
| Kill one of them and wear his kit | It's a dead man's kit: told when you put it on, an instant fail at their look, taken at a Bastion search | `marked as <name>'s (a dead man's)`, the look's `[A dead man's kit]` |
| Loot uniforms off military infected | Kit off the turned: an instant fail | `kit off the turned marked`, `[Off the turned]` |
| A friend buys kit for you | Signed out to them: an instant fail, taken at a Bastion search | `signed out to them`, `search took` |
| Guess the word at the wire | The word counts only once you've heard it on the secure net (or been told) | `heard the word at the wire` |
| Farm standing | No Fame or Favor ever comes from a disguise; Infamy is always yours | the look's line |
| Retry until it works | The check is fixed and shown first. Catches count: three a life, then no claim | `caught by ... (catch N this life)` |
| Stay inside | A gate pass is 10 minutes, each again look is one step harder, roll call at the shift change, leaders within 15 m | `again look at`, `roll call for` |
| Friends take turns | Word gets round at the holding, for everyone | `word gets round at` |
| Grief a friend's plan | Only catches and kills spread word, one per holding, restarted and never queued. A point-out is −1, twice at most, and a wrong one costs the pointer regard | `word gets round ... restarted`, `pointed out to` |
| Shoot guards from inside a horde | A hit always gives you away; only an infected within 3 m excuses it | `exposed to`, `caught by` |
| A Hunted team's opening volley | The Hunted can't claim: they're seen through | `can't claim: Hunted` |
| Log out before a look | Passes, owed looks and looks passed are kept by identity; catches and blown time are saved with the life | `owed a look` |
| Swap kits to reset | Blown time, catches and word round belong to you and the place, not the kit | the look's tags (`[Caught before -1]`) |
| Walk behind a friend at a barrier | Everyone is checked on their own unless signed through, and the voucher answers for them | `signed ... through`, `voucher ... answers` |
| Hide in a searched car | "We search the whole car" comes before any look: nobody in a searched car claims | `searched with the whole car` |
| Get intel from lines | A pass says nothing their open channel doesn't | the look's line |
| Say a name you never heard | A cover choice shows only when you have the detail; said without it, it's a lie, caught at once | `a cover detail they don't have` |
| Try every cover line on one speaker | One claim line per speaker per life | `gave ... a cover detail` |
| Carry their stolen papers through their gate | Something of theirs from a job against them is −1 at their look | the look's `[Something of theirs on you -1]` |
| Farm the cards | Card pay is the errands' small-pay ledger, each card once a season; The Name on the Tape and Notebook 31 are never dealt to whoever killed the dead, once per name; Spot the Tells needs two different players caught over 2 days | `Errands: <card> taken by` |
| Use colours as a licence to kill | Colours are refusals and timed grudges, never standing bits: retaliation still counts every kill | retaliation's lines |
| Hide Infamy behind Fame | Tolerated and Wildcard players aren't disguised: they're treated by their real standing | — |
| Use it on the cure | Nothing is cleared within 1.5 km of the lab during the stand | `counts for nothing near the lab` |
| Keep issued kit after going bad | A real day's grace, then it's a disguise like any other; never while Hunted | the look's line |
| Pay the Rogue and keep going | 10 minutes at his seat only; everyone who paid is named if he's found out | `paid Rogue`, `Rogue ... found out` |
| Walk off while held | Running the checkpoint | `walked off while held` |
| Dump Infamy on a voucher | +5 once per vouch, then 2 hours without vouching; the voucher chooses whom to vouch for | `voucher ... answers` |

## The cure finale: locks and war

The last tenth of the cure is a race between five routes ([FINALE.md](FINALE.md)) ✅. What it does to
standing:

- **Locking.** You lock to one route at its desk, for the season; dying doesn't undo it. A faction
  route locks you unless you're Hunted by its faction; the Hunted lock if they're owed (the parley,
  [above](#the-parley-and-the-knock)) or pay its **price door** (the Haven's *In pencil*, Bastion's *Sign
  on, attached*, the Red Hand's *line in the book*, Halcyon's *volunteers' book*). A price door opens
  the route only: Infamy stays. The Independent route is always open.
- **War deeds.** Killing a player locked to a faction route costs **+3 Infamy** with that faction (an
  Independent-locked victim: the Haven), at most **12 an hour**, tagged war. At the stand, within
  150 m of the lab, fire between locked players of different sides costs nothing, and the lab's
  safe-zone rule doesn't apply to locked players; killing an unlocked player there costs +10 Halcyon.
  Shooting your own side: +10 with your route's faction. Nothing new is paid for these kills.
- **Parley** closes with any faction whose route is fighting yours (a hostile act in the last 2 real
  days, or the stand). It reopens at the cure.
- **What they call you** after the lock is the lock's word ([above](#what-people-call-you)), and leaders
  greet you on the first visit each real day with a route line in their voice
  ([WRITING.md](WRITING.md#the-race-and-the-lock)). The season record keeps the route and the cell
  with each faction at the lock ([above](#the-season-record)).
- **Pay.** All finale content pays at most 50 Fame per faction per player, 60 Favor and 40 Infamy.

## The season's winner

When the final stand is won, **whoever holds the clean room controls the cure** ✅
([FINALE.md](FINALE.md#the-stand-a-fight-for-the-clean-room)), and the faction whose rules apply is the
controller's (an Independent cure: only by its "one side" terms). **Season Fame**, the Fame the
server gave each faction this season summed over all players plus the finisher's capped chunk, is the
fallback: when nobody holds the room the cure is **handed over** to the faction with the most, and
with the finale off it picks the winner as it always did. Season Fame also gives the hall of fame its
line (*"The cure: the Haven, by force. The Zone's Fame: Bastion, 1640."*) and, when the Fame leader
isn't the controller, its rules at half next season. What each ending means is in [LORE.md](LORE.md),
"Endings". The winner shapes the next season's starting rules ✅: in the Red Hand's season the fence
opens at Known, and in Bastion's its checkpoints keep a curfew ([DESIGN.md](DESIGN.md#seasons)).

## How it maps to code

- `RZ_PlayerProfile`: Fame, Infamy and Favor per faction for the current life, plus the
  persistent Champion list and background unlocks. Per life also: Fame gained and direct Infamy
  gained (`EarnedFame`, `Feared`), the budget used (`m_aForgiven`, `ForgiveLeft`), threats used, the
  name last told, and the life's deeds (`Core/RZ_Deeds.c`). Borrowed kit's catches and blown time
  (`m_aCatches`, `m_aBlownUntil`, per life), and the parley's and settling's server times. Borrowed
  kit's nail list, Bastion's board and the season's counts are `RZ_DisguiseSave` in the world save.
- `RZ_Reputation`: the tier table, access ranks, standing checks, the rival table, display names.
  `RZ_RepNames` (`Factions/RZ_RepNames.c`): the 16 names (`RZ_ERep`, append-only), "counts as", the
  card's note, notoriety; `RZ_RepWatch`, the debounced change notice and `/rep`;
  `RZ_ReputationSettings` (**Factions > Reputation**).
- `RZ_GameModeComponent`: `AddFame(player, faction, amount, rivals, flags, deed)` and
  `AddInfamy(player, faction, amount, why, flags, deed)`, with the flags (`RZ_RepFlag`) `SHARE` (a
  rival share: never doubled, never Feared; the rival table, a mission's rival, questline standing
  and siding pass it), `ACT` (done in sight) and `SMALL` (errands' small pay: left out of season Fame
  and the hall of fame); the credit rule; `Forgive(player, faction, amount, why)`, the one budget;
  `AddFavor`, `SpendFavor`, `GetTier`. Every Infamy gain goes through the stronghold leader's temper
  (`RZ_Leaders.ScaleInfamy`). *Simplified:* a direct gain without SHARE counts as done in sight, as
  before, so a uniform still doubles it; call sites aren't yet marked ACT one by one.
- `RZ_QuartermasterComponent` marks a faction contact; `RZ_FactionShopAction` sells one item
  (`Quote` is every check and the price for one player; `Buy` takes the Favor; `RZ_SupplyLines.FindShopState`
  for shortages, `PriceMult` for leaders and morale). `RZ_ShopOpenAction` ("Trade") and
  `RZ_ShopScreen` (`Factions/RZ_ShopScreen.c`) put a quartermaster's actions on the shop screen.
- Missions: `m_eGiver` gets Fame, an optional `m_eRival` gets Infamy, `m_iFavorReward` pays out,
  `m_iLeaderId` is the leader whose traits scale it.
- `RZ_FactionPerks`: perks, hunter squads, bounties and the bounty board, the safehouse location
  leak (`LeakLocation`). House rules and exile are in `RZ_Safehouses` and `RZ_Wanderers`.
- `RZ_FactionServices`: the Fire Mission perk (`RZ_FireMissionSmokeComponent`), the fence, the
  medic and booster, paying for amends, reputation clues; `RZ_FactionServiceAction` on quartermasters.
- `RZ_LongTable` (`World/RZ_LongTable.c`): the Long Table's memory of each character, its
  dealings, the guest rule, shooting them and the reactions when it's found out.
- `RZ_Checkpoints`: road posts, surrender, camps and safe zones. A surrender is
  `RZ_Hostility.StandDown`; every "fired on for a few minutes" is `RZ_Hostility.AddGrudge`.
- AI hostility: `RZ_Hostility` works out each player's bitmask; `RZ_Factions` builds and decodes
  the keys; the modded `SCR_Faction` in `RZ_FactionRelations.c` turns them into relations;
  `RZ_Retaliation` handles shooting back; `RZ_Disguise` reads kit, runs claims and looks, catches and
  exposures, and doubles an act seen while passing; `RZ_Parley` runs the parley, the knock, amends
  reservations (`RZ_Mission.Reserve`) and settling up.
- The season's winner: the cure finale's controller (`RZ_CureFinale.ControllerFaction`), else Fame
  per faction summed over all players (`RZ_SaveData.seasonFame`, plus `AddFactionBonus` for the
  finisher's chunk), written to `last_season.json`. The finale's locks, price doors, war Infamy and
  caps are in `Cure/RZ_CureFinale.c`.
