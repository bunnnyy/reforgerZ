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
- **The admin read-out:** `/rep <name>` in chat (server admins; empty for yourself) prints a
  player's cells, Earned Fame and Feared, what's left to forgive, deeds, tallies and threats used,
  to the admin and to the log.

### What people call you

✅ Code: `Factions/RZ_RepVoice.c`. A name opens nothing: every rule still reads the tier. What's
said aloud (the openings, what they make of you, the picture) is said only with **Dialogue > Trees**
on ([DIALOGUE.md](DIALOGUE.md)); the word and crew names are kept either way.

**Manners** are a writing tool, never a rule: the cell picks how a line sounds. Stranger is
*unknown*; Known and Trusted *warm*; Champion *devoted*; Watched and Mixed Bag *wary*; Useful
Trouble and Favourite Rascal *fond*; Wildcard, Necessary Evil and Wild Child *needed*; the Hunted
cells *hostile*. A Pragmatist, or an Independent or Rogue leader who holds nothing against you,
reads *hostile* as *wary*.

**The word for you**, to your face, this life (`RZ_RepVoice.Word`, the `{word}` token):

| Manner | The Haven | Bastion | The Red Hand | Halcyon |
|---|---|---|---|---|
| unknown | love | civilian | you | visitor |
| warm | a friend of the house (Trusted: the one with the ribbon) | civilian, cleared (Trusted: auxiliary) | your crew name, else new face | courier (Trusted: field associate) |
| devoted | one of ours | on the roster | your crew name | colleague |
| wary | a worry | uncleared | you | subject of concern |
| fond, needed | a worry we feed | flagged, useful / flagged | your crew name | under review |
| hostile | put out | on the list | that one | excluded from the study |

Kin of a fallen Champion are "family" to the Haven. A word the majors or questlines give
(`SetWord`, "Sergeant Novak") wins unless they're hostile.

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

Bailiff (a pack's leader laid down) waits for the tribes, and Smoke (a signal smoke thrown) for a
throw hook; the majors' and Name Day's names are planned
([REPUTATION_NAMES.md](../designs/REPUTATION_NAMES.md)). Log line:
`Reputation: crew name <name> for <player>`.

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
exists: a **local deed** at this seat ("You're the one who brought medicine to Levie.", written when
you finish that leader's personal job); this life's memory (saved: "Here's the one I owe. Sit
down."; crossed: "You. I remember."); the **name from an earlier life**, once per leader per life
("There was a Lukas who crossed me once. Different face. Same name. Should I be worried?"); the
greeting for your manner in the leader's voice, from the voice bank
([WRITING.md](WRITING.md#the-voice-bank)); the built greeting.

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

The parley, the knock, settling up, the company you keep and the season record are planned:
[REPUTATION_NAMES.md](../designs/REPUTATION_NAMES.md).


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
| Known | Canned food, water, bandages, crowbars | A welcome message ✅ (safehouse services, including bite declarations and expired antibiotics for 5 Favor, are open to anyone the Haven hasn't exiled) |
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
| Known | Military rations, morphine, flares | **Waved through** ✅: Bastion's checkpoints inside the Zone let you pass unsearched ([Checkpoints](#checkpoints-roadblocks-and-camps)) |
| Trusted | Service rifles, body armour, helmets, NVGs | **Requisition** ✅: once an hour, ammo or rations at half the Favor cost, only from what the quartermaster sells at Trusted or below ✅ (never a Champion item). While Bastion's trucks run, it's drawn at a Bastion outpost's quartermaster from the stock its trucks brought, and refused while that outpost is short |
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
| Known | Knives, pistols, lockpick-style tools, the **cut dose** (cheap: cures half the time, else a faster turn and a fever ✅) | **Toll pass** ✅: Red Hand roadblocks take a toll instead of shooting ([Checkpoints](#checkpoints-roadblocks-and-camps)) |
| Trusted | Suppressors, scoped rifles, explosives | **Fence** ✅: sell found loot for Favor; roadblocks wave you through ✅; the **red armband** ✅ ([clues](#reputation-clues)) |
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
it comes down and the poster gets half back. Postings aren't saved. *Simplified:* "friends" means
the same vanilla player group; a poster in no group excludes only themselves.

| Hunted | — | Red Hand AI shoots you ✅; hunter squads come for you ✅ |

### Halcyon

| Tier | Shop (lab stores) | Perk / service |
|---|---|---|
| Stranger | The **trial dose** for anyone bitten (free, once per life, 80% cure ✅) | — |
| Known | Antiseptic, painkillers, sample containers, **stabilisers** (from a small stock; double price until 50% research ✅) | — |
| Trusted | Antidotes (only while the lab has doses in stock ✅), stim kits, gas masks | **Sample scanner** ✅: special infected within 300 m are revealed every minute. **The medic** ✅: 20 Favor to be patched up, and a fresh bite washed out |
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
  two squads are after one player at a time. When that player dies, their squads stand down
  (walk off, or vanish if nobody is near): a new life starts with nobody on its trail.
- **Bounties** ✅: Kill on Sight with any faction puts a public bounty on you, announced every
  10 minutes with a rough location (one notice, however many factions want you). One bounty is paid per death, only to the player who killed
  you (50 Favor and a little Fame), and only for a player kill: dying to the infected, a fall or
  your own hand pays nobody. With several factions after you, the one with the most Infamy pays;
  a faction never pays someone it hunts too. If none pays, a posting on the Red Hand's
  [bounty board](#the-red-hand) can. *The killer is the last attacker the body's damage manager
  recorded; if that wasn't a player, no bounty is paid.*
- **Choice missions** ✅: you decide which faction's drop point gets the item. The winner gives
  Fame; the one you double-crossed gives Infamy. Taken to the other side, it pays everyone with
  you whom that side doesn't hunt, and the leader who gave the job neither counts it as help nor
  scales its pay.
- **Disguises** ✅: a faction's uniform gets you past its AI while you're not Hunted by it;
  caught, your Infamy gain doubles ([Disguises](#disguises)).

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

Food delivered to a safehouse pays 2 Haven Favor per item, plus 1 Fame per 3 items (at most 5).
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

**When the budget is spent:**
- An amends contract pays its Fame as ordinary Fame, rival shares and all: "Bastion has forgiven you
  all it will this life. The work counts as Fame instead."
- Surrender is a search and the hold, with no Infamy off.
- Paying is refused with the same line.

So a little trouble (up to 30) can be cleaned completely, and more can only be softened. A Watched
player and a mixed one are treated the same. Every forgiveness is logged. The later routes (the
parley, settling up in Fame, Make it right, errand amends, the epilogue's Come In and the finale's
Hand it in) are planned and will draw on the same budget
([REPUTATION_NAMES.md](../designs/REPUTATION_NAMES.md)).

## Checkpoints, roadblocks and camps

Code: `Factions/RZ_Checkpoints.c`. ✅ all. The Perimeter's sealed checkpoints at the map edge are a
different thing ([DESIGN.md](DESIGN.md#the-perimeter)): they stay sealed to everyone.

**Road posts.** Every 15 minutes, if fewer than two are up, a Bastion checkpoint or a Red Hand
roadblock may go up on a road 350–1,000 m from a random player, within 2.5 km of one of that
faction's holdings (not near the line, a safehouse, a holding or another post). It stays 45
minutes. It's data: the barrier and its crew (the territory guard prefab unless set) exist only
while a player is within 600 m, and its crew counts for retaliation. It needs territory and the
road network. Its fire barrel and nest beside the barrier, and the crew leaning there, are camp
life's ([DESIGN.md](DESIGN.md#camp-life)); none of it changes who is waved through.

| Post | You are | What happens |
|---|---|---|
| Bastion checkpoint | Known or better, not the Wildcard tier | Waved through, unsearched |
| | The Wildcard tier (Wildcard, Necessary Evil, Wild Child) | Searched like a stranger: "Arms out." They search you because they don't trust you, so running this search costs +6 Bastion Infamy, not +3 (**Reputation > Wildcard at posts**, **Wildcard run Infamy**) |
| | Anyone else | Told at 150 m to walk to the barrier. At the barrier (35 m) for 15 s: searched, and up to three contraband items taken (suppressors, explosives: a setting). Leave the barrier before it's done and you've **run** it: +3 Bastion Infamy and the post fires on you for a few minutes |
| | Hunted | Shot (as everywhere); or [surrender](#surrender) |
| Red Hand roadblock | Trusted or better | Waved through |
| | Known | Pay the **toll** at the barrier (10 Red Hand Favor) and pass. Can't pay: treated as a stranger. The Wildcard tier pays danger money: 20 (**Reputation > Wildcard toll**, ×2) |
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
The lab lets a bitten player in unarmed for the trial dose, Watched or not, until they've had it.
Hunted players are simply shot. The Haven's holdings follow its house rules instead.

**Safe zones.** Strongholds and the lab: within 80 m you're told the rule as you come in
("weapons slung"), and firing there, with no infected within 40 m, costs 2 Infamy with the
faction (at most once every 20 s) and draws their fire for a few minutes. Safehouses have their
own [house rules](#haven-house-rules-and-exile).

Every "fired on for a few minutes" here is the same timed grudge as
[shooting back](#what-makes-ai-shoot-you): it restarts, never adds up.

What the posts and camps say to you (the warnings, the search, the toll, the gate, the safe-zone
rule) shows as a subtitle at the bottom of the screen, not a notice card; the outcomes with
numbers (a surrender's Infamy, the toll paid) stay cards ([UI.md](UI.md#subtitles-and-the-reader)).

## Reputation clues

Something other players can see ✅. At **Trusted** the Red Hand gives you a **red armband** and
the Haven an **amber ribbon** (once per life, into your pack, or at your feet if it's full).
Wearing it is up to you; worn, anyone can read whose you are. The armband's name carries the Red
Hand disguise keyword, so it counts as one piece of a Red Hand [disguise](#disguises). Both need
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
| The ambush in Fake Distress Call, the convicts in Prison Break | Players within 300 m (250 m for the convicts) who aren't at least Known with the Red Hand |
| The Sniper | Every player within 500 m, Red Hand friends included |
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

### Disguises

Wear enough of a faction's uniform (two pieces by default: say, jacket and trousers) and its AI
looks past you as long as it isn't Hunting you. With the default stances that only matters
where a faction is set to shoot from Watched or Strangers, or in a mission that asks for it
(Bust Out walks you past the stockade guards). ✅

- **What counts** is read from the prefab names of what the character wears, every 10 seconds,
  against keyword lists per faction (Bastion, the Red Hand, Halcyon; the Haven has no uniform).
- **Caught:** any Infamy you earn with that faction while wearing its uniform is doubled, and the
  disguise stops working for 15 minutes ("they know your face now"). Shares don't count, since
  nobody saw you: the rival share of Fame from another faction's job, a mission's rival Infamy
  and a questline's standing are never doubled and never blow the disguise.
- **It doesn't hide a fight.** Mission fights, hunter squads and retaliation still shoot you.
- *Simplified:* one faction at a time, nothing partial, nothing saved.

## The season's winner

When the final stand is won, the cure goes to whichever faction the server gave the most Fame
this season ✅. What that means is in [LORE.md](LORE.md), "Endings". The winner shapes the next season's
starting rules ✅: in the Red Hand's season the fence opens at Known, and in Bastion's its
checkpoints keep a curfew ([DESIGN.md](DESIGN.md#seasons)).

## How it maps to code

- `RZ_PlayerProfile`: Fame, Infamy and Favor per faction for the current life, plus the
  persistent Champion list and background unlocks. Per life also: Fame gained and direct Infamy
  gained (`EarnedFame`, `Feared`), the budget used (`m_aForgiven`, `ForgiveLeft`), threats used, the
  name last told, and the life's deeds (`Core/RZ_Deeds.c`).
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
  `RZ_Retaliation` handles shooting back; `RZ_Disguise` reads uniforms and doubles Infamy when caught.
- The season's winner: Fame per faction summed over all players (`RZ_SaveData.seasonFame`),
  written to `last_season.json`.
