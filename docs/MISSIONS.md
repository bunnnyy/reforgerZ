# Missions

Every mission ReforgerZ has or plans, with its requirements and status. How the systems
behind them work is in [DESIGN.md](DESIGN.md); standing, Fame and Infamy are in
[REPUTATION.md](REPUTATION.md); the prefabs each mission needs are in
[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md), "The prefab manifest".

## Counts

| Kind | In code | Designed, not built |
|---|---|---|
| [Faction contracts](#faction-contracts) (20 designed per faction) | 80 | — |
| [World events](#world-events) | 25 | — (3 more are covered by other systems) |
| [Puzzle missions](#puzzle-missions) | 17 | — |
| [Unarmed missions](#unarmed-missions) | 11 | — |
| [Perimeter missions](#perimeter-missions) | 3 | — |
| [Haven house contracts](#haven-house-contracts) | 3, plus the Courier's letters | — |
| [Leader missions](#leader-missions) | 32, plus Rein In | — |
| [Story arc missions](#story-arc-missions) | 3 | — |
| **Total** | **176** | **—** |

## Who gets offered what

**World events** are announced to everyone, and anyone who takes part shares the reward.
**Contracts** are offered only to players who meet all of their requirements:

1. **Tier with the giver.** Known, Trusted or Champion. Big jobs need trust.
2. **Standing with others.** For example *not Trusted by the Red Hand*, or *at least Watched by
   the Haven*. Some open *because* someone hates you.
3. **Background.** Some contracts are locked to or from backgrounds (🔒), and some backgrounds
   get a better outcome (★).
4. **World state.** Outbreak stage, cure progress, whether Bastion is still around, night.

How the code does it:

- **No "accept" step.** A contract is offered by radio (at most one new offer per player a
  minute, briefed in a leader's voice) or by the faction's quartermaster when you walk up.
  Anyone can join in, but only players who meet the requirements when it ends are paid.
  Leader jobs are offered only in person.
- **Built from a few blocks:** hold an area, kill what the mission spawned, visit points, destroy
  a prop, carry an item to a drop point, or choose which drop point gets it; escort people,
  capture a special alive, drive a vehicle into a zone, pay in health, or follow a chain of
  steps ([DESIGN.md](DESIGN.md#mission-building-blocks)). Puzzle-style missions add clues and
  answer actions.
- **Missions that need an optional prefab** never appear until it's set.
- **Needs and morale** make matching contracts more likely ([REPUTATION.md](REPUTATION.md),
  "Morale, needs, truces and feuds").
- **Supply trucks aren't contracts.** Escorting, driving in or stealing one pays directly
  ([DESIGN.md](DESIGN.md), "Supply trucks"). Convoy Escort and Hijack the Convoy ask for
  Bastion's next real truck, so those truck rewards are paid on top of the contract.
- **Rescues** add population at the nearest Haven safehouse ([DESIGN.md](DESIGN.md), "Haven safehouses").

**Reading the tables:**
- **Needs:** tier with the giver first, then other requirements. *Any* = Stranger or better and not Hunted.
- **Bg:** 🔒 background lock, ★ background bonus.
- **Result:** what you gain, and in brackets what it costs with other factions. Every contract also pays Favor.
- **Status:** ✅ in code (with any simplification in *italics*), *planned*, or *covered* by another system.

Backgrounds: Nurse, Mechanic, Hunter, Police, Ex-Con, Lab Tech, Radio Op, Unemployed; unlockable
Soldier, Doctor, Raider, Elder, Drifter.

---

## Faction contracts

### The Haven

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Supply Cache | Any | | Hold a cache in a town for 60 s | Haven Fame | ✅ |
| 2 | Survivors Trapped | Any | ★ Nurse: healthier survivors, bonus Fame | Reach a barricaded family and survive 3 waves | Fame; the family joins a safehouse (+3, +4 with ★) | ✅ |
| 3 | Radio Relay | Any | ★ Radio Op: half the hold time | Keep a loud relay on high ground running for 3 min | Fame | ✅ |
| 4 | Medicine Run | Any | ★ Nurse/Doctor: the pharmacy is pre-marked, bonus Fame | Bring medical supplies from a pharmacy town to a Haven safehouse | Fame | ✅ |
| 5 | The Bitten Friend | Known | ★ Nurse: slows his infection | A Haven member is bitten. Bring an antidote before he turns, or end it | Fame; mercy = half the Fame | ✅ |
| 6 | Fuel for the Lights | Known | ★ Mechanic: generator runs twice as long | Bring 2 jerrycans to a safehouse generator. The lights raise heat there | Fame | ✅ *the lights run 10 min (20 with a Mechanic) and it pays when they go out* |
| 7 | Take Back the Town | Known, Bastion gone (Overrun+) | | Kill 8 × size infected in a town | Fame, town infection −35 | ✅ |
| 8 | Harvest Guard | Known | ★ Hunter: farmers stay hidden longer | Protect farmers in the fields for 10 min | Fame; +3 food at the nearest safehouse | ✅ *farmers are real only with the survivor group prefab; ★ delays the first wave by 2 min* |
| 9 | Missing Scout | Known | | Find a scout's body and bring his notes home | Fame; the notes mark a stash (a reward crate in a random town) for whoever brought them | ✅ |
| 10 | Clear the Road | Known | ★ Mechanic: bonus Fame | Kill the infected on the road between two Haven towns | Fame | ✅ *no wrecks to clear* |
| 11 | Burn the Nest | Trusted | | Destroy a hive in an infested town | Fame, infection −30 | ✅ |
| 12 | Refugee Column | Trusted | | Escort a group walking a road to a safe town | Big Fame; three join the safehouse | ✅ *to a safehouse 0.8–3.5 km away, in a straight line through a midpoint* |
| 13 | Signal Fires | Trusted, 3+ players online | | Light beacons on 3 hills within 2 minutes of each other | Fame for each player; every town within 2 km −10 infection | ✅ |
| 14 | Water Purifier | Trusted | ★ Mechanic/Lab Tech: 2 of 3 parts | Find 3 purifier parts in a town and bring them to a safehouse | Fame | ✅ *the parts are the mission item, lying in the streets with two infected each* |
| 15 | Fortify the Safehouse | Trusted | ★ Mechanic: 1.5× faster | Hold the area for 3 min (3 waves) while barricades go up | Fame; the town counts as Barricaded | ✅ |
| 16 | Trader Caravan | Trusted, not Trusted by the Red Hand | | Defend a Haven trader at a crossroads from a Red Hand raid | Fame (Red Hand Infamy) | ✅ |
| 17 | Rescue the Kids | Trusted | ★ Hunter: quieter approach | Hold a school for 90 s without making noise. Too loud and a horde comes | Big Fame; the kids join a safehouse (+3) | ✅ |
| 18 | The Informant | Trusted, Watched+ by the Red Hand | 🔒 not Police/Soldier | Pose as a buyer at a Red Hand dead drop and report who shows up | Fame; the Red Hand loses a little strength (+10 Red Hand Infamy for the buyer if spotted) | ✅ *wait 60 s for the seller, buy, then reach any safehouse; spotted 20%, +30% carrying a gun* |
| 19 | Recover the Bus | Champion | ★ Mechanic: tunes it to half a tank | Get a bus or truck running and drive it to a safehouse | Fame | ✅ *it starts with a splash of fuel; any bus, truck or van counts; the Haven keeps it (it leaves the world)* |
| 20 | Memorial | Champion | | Collect ID tags of the dead from an overrun city | Fame | ✅ *five bodies, 8 s each, then any safehouse* |

**Amends:** Medicine Run and Survivors Trapped work off Haven Infamy while you're Watched.

### Task Force Bastion

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Retake the Town | Any, Bastion present | ★ Police/Soldier: +25% Fame | Kill 8 × size infected in a town | Fame, infection −35 | ✅ |
| 2 | Last Helicopter | Any, Bastion present, no storm | | Bastion's one helicopter lands at an LZ on its run between its own bases. Keep the LZ clear and be there when it lifts off | Fame, Favor | ✅ |
| 3 | Burn the Bodies | Any | ★ Nurse: faster | Stand at 5 corpse piles in a town for 10 s each | Fame, spread slows there | ✅ |
| 4 | Recon | Any | ★ Hunter/Radio Op: double Fame | Visit 3 observation points on high ground | Fame; Bastion shares where the next horde is forming | ✅ |
| 5 | Checkpoint Duty | Known | ★ Police: holds faster | Hold a checkpoint at a town's edge for 10 min against waves | Fame (Red Hand Infamy) | ✅ |
| 6 | Missing Patrol | Known | | Find a squad's last position and deal with the turned soldiers | Fame, a crate of military loot | ✅ |
| 7 | Crash Site | Known | ★ Mechanic: black box twice as fast | Secure a helicopter wreck for 60 s and take its black box | **Choice:** Bastion Fame, or sell it to the Red Hand | ✅ |
| 8 | Restore Comms | Known | ★ Radio Op: needs no parts | Bring parts and repair a radio tower | Fame; Bastion intel on the radio | ✅ *the parts are the mission item in a nearby town; then 60 s by the humming mast; the intel goes to everyone Known with Bastion* |
| 9 | Curfew Patrol | Known, night | ★ Police: 1.5× faster | Patrol a town at night and keep it clear | Fame | ✅ *four patrol points (15 s each) and six infected killed in town; until dawn* |
| 10 | Weapons Cache | Trusted | | Retrieve weapons from an armory in an overrun base | Fame; Bastion gets stronger | ✅ *a barracks or police station in an overrun town if the map has one; search 30 s, carry the case to a Bastion drop* |
| 11 | Convoy Escort | Trusted | ★ Mechanic: fixes a broken-down truck in a third of the time | Escort supply trucks between two towns | Big Fame (Red Hand Infamy if raiders came) | ✅ *Bastion's own supply truck from its stronghold to an outpost; it breaks down once halfway (30 s to fix); raiders only with the Red Hand group prefab; paid for staying with it 40% of the way. Needs supply trucks* |
| 12 | Rescue the Pilot | Trusted | ★ Nurse/Doctor: patches him up, no clock | Find a downed pilot and bring them out | Fame | ✅ *he follows you to a Bastion seat; 20 min before he bleeds out* |
| 13 | Quarantine Line | Trusted | | Hold the edge of an infested town for 10 min | Fame; the town stops spreading for 30 min | ✅ |
| 14 | Clear the Airfield | Trusted | | Clear an airfield so the Last Helicopter can land there | Fame; the Last Helicopter runs twice as often for 2 hours | ✅ *a flat field outside a town, not a real airfield; it doesn't land there* |
| 15 | Raid the Red Hand | Trusted, not Trusted by the Red Hand | 🔒 not Ex-Con/Raider unless Champion with Bastion | Assault a Red Hand camp | Big Fame (big Red Hand Infamy) | ✅ |
| 16 | Hunt the Deserter | Trusted | | Track down a deserter who stole supplies. Bring him back or kill him | Fame; killed = 60% | ✅ *a two-step trail (his pack, his fire) to his hideout; arrested, he follows you to a Bastion seat* |
| 17 | Requisition | Trusted | 🔒 not Elder | "Collect" supplies from a Haven safehouse. Deliver them to Bastion, or give them back | To Bastion: Fame (Haven Infamy), the safehouse loses 4 food. Back: Bastion Infamy | ✅ |
| 18 | Secure the Shipment | Champion | | Escort a Halcyon sample shipment | **Choice:** Bastion Fame (Halcyon Infamy) or let it reach the lab (Halcyon Fame, research) | ✅ *a courier on foot; divert her by talking to her* |
| 19 | Blow the Bridge | Champion | ★ Mechanic/Soldier: two charges, not three | Plant charges to stop a horde heading for a safe town | Fame; the horde turns back. It arrives: +10 infection there | ✅ *no real bridge: a choke point on its way; the horde is data until a player is within 600 m* |
| 20 | Fire Mission | Champion | | Mark three hordes with smoke for artillery | Fame | ✅ *10 s within 80 m marks one; 30 s later the strike kills it outright* |

**Amends:** Burn the Bodies and Recon work off Bastion Infamy while you're Watched.
Surrendering at a checkpoint is planned.

### The Red Hand

The Red Hand doesn't care who you were, but it does care who you've crossed.

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Dead Drop | Any | ★ Ex-Con: 25% better pay | Sell infected samples for loot and Favor | Fame (Halcyon Infamy; the cure slows) | ✅ |
| 2 | Fence the Goods | Any | ★ Ex-Con: +50% Fame, extra Favor | Carry stolen goods to a fence in another town | Fame, mostly Favor | ✅ |
| 3 | Car Theft | Any | ★ Mechanic: +25% Fame | Deliver a specific vehicle to the chop shop | Fame | ✅ *a car from Cars > Car prefabs parked in a town, driven to a Red Hand drop and left there (it's stripped: it leaves the world); spawned cars aren't locked, so no hotwiring* |
| 4 | Pharmacy Heist | Any | | Hold a pharmacy for 90 s before the Haven gets there | Fame (Haven Infamy) | ✅ |
| 5 | Fight Pit | Known | | Survive 4 minutes of waves in a ring while the Red Hand bets | Fame; the bets pay Favor | ✅ *the bets are a fixed, larger Favor reward* |
| 6 | Toll Road | Known | | Set up a roadblock between two towns and hold it for 10 min | Fame (Haven Infamy) | ✅ |
| 7 | Smuggler's Run | Known | ★ Hunter: fewer location reports | Carry a package across the map. Everyone is told a courier is out | Big Favor | ✅ |
| 8 | Raid the Cache | Known, Watched or Hunted by the Haven | | Beat the Haven to a supply cache | Fame (Haven Infamy) | ✅ |
| 9 | Loot the Armory | Known | ★ Police: knows the layout (40 s, not 60) | Break into a police station armory | Fame, mostly Favor | ✅ *the gun cage at a police station (the town centre if the map has none): cut it open (loud), carry the case to a Red Hand drop; nobody keeps the guns* |
| 10 | Protection Racket | Trusted | 🔒 not Elder | "Collect" from a Haven safehouse. Pay or fight | Fame (Haven Infamy); the safehouse loses 3 food | ✅ *the keeper pays 55% of the time; refused, lean on them: a fight with the keeper's people if the survivor group prefab is set, else they pay and you take +5 more Haven Infamy* |
| 11 | Hijack the Convoy | Trusted, not Trusted by Bastion | | Ambush a Bastion convoy | Big Fame (big Bastion Infamy) | ✅ *Bastion's own supply truck to an outpost: robbed, burnt or driven off counts, for anyone within 300 m when it happens. Needs supply trucks* |
| 12 | Zombie Bait | Trusted | ★ Radio Op: twice the noise | Make enough noise at a Bastion checkpoint that a horde comes for it | Fame (Bastion Infamy) | ✅ |
| 13 | Bust Out | Trusted | ★ Ex-Con: cuts the lock in 8 s, not 20 | Free Red Hand prisoners from a Bastion stockade | Fame (Bastion Infamy) | ✅ *a gate near a Bastion seat; its guards (Contracts > Bastion squad) see you unless you're in Bastion's uniform ([disguises](REPUTATION.md#disguises)); the prisoners walk with you to a Red Hand drop* |
| 14 | Arson | Trusted | | Burn a Bastion fuel depot | Fame (Bastion Infamy) | ✅ |
| 15 | Sabotage the Relay | Trusted, not Trusted by the Haven | | Destroy a Haven radio relay | Fame (Haven Infamy) | ✅ |
| 16 | Steal the Antidotes | Trusted, the lab has doses in stock | ★ Lab Tech: twice as fast | Hold the lab's store room for 90 s while the crew loads up | Big Fame, Favor; up to two of the lab's doses move to the Red Hand's black market (Halcyon Infamy) | ✅ |
| 17 | Double Agent | Trusted, Known by Bastion | ★ Police: Bastion believes you (caught 15%, not 35%) | Feed fake intel to Bastion | Fame with both (Bastion +5); caught = +20 Bastion Infamy | ✅ *a dossier from a Red Hand seat to a Bastion drop; believed, Bastion loses a little strength* |
| 18 | Collect the Bounty | Champion | | Kill a player or AI with a bounty on them | Fame, Favor | ✅ *a player Kill on Sight with another faction (not a Red Hand friend), paid to the player who kills them (not for a death to the infected, a fall or suicide); else an armed AI outside a town (Contracts > Bounty target)* |
| 19 | Kill the Officer | Champion | | Assassinate a Bastion outpost commander (the stronghold's commanding officer never dies) | Huge Fame (+75 Bastion Infamy: Kill on Sight) | ✅ *needs a killable commander body (Leaders > Bastion outpost commander or Bastion leader); a new one takes the chair after 2 hours* |
| 20 | Take the Lab | Champion, final stand running | | Hold the lab's clean room for 90 s during the final stand while the crew steals the batch | Huge Fame (Halcyon Infamy); the final stand ends, research drops to 90%, two doses reach the black market | ✅ *reframed: the batch is stolen, which only delays the cure. The lab never falls and nobody wins the cure by taking it* |

**Amends:** Fence the Goods works off Red Hand Infamy. Paying the fence a Favor "tax" is planned.

### Halcyon

| # | Contract | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Alpha Sighting | Any | | Kill a special that's been spotted and bring its samples | Fame, samples | ✅ |
| 2 | Field Samples | Any | ★ Lab Tech/Nurse: each kill counts double | Kill infected in an infested town while Halcyon takes readings | Fame, research | ✅ |
| 3 | Water Survey | Any | ★ Lab Tech: +50% Fame | Sample wells in 3 towns (10 s at each) | Fame; reveals the town most at risk | ✅ *the most infected town still under 50, anywhere on the map* |
| 4 | Power the Lab | Any | ★ Mechanic: +50% Fame | Bring fuel and a generator part to the lab | Fame; antidote doses come twice as fast for 2 hours | ✅ *the part is the mission item at a factory, petrol station or power station in a nearby town* |
| 5 | Lost Colleague | Known | | Find a missing researcher. Half the time he's turned | Fame, research | ✅ |
| 6 | Research Data | Known | | Recover a hard drive from an overrun hospital (hold 45 s) | **Choice:** Halcyon research, or sell it to Bastion (Halcyon Infamy) | ✅ |
| 7 | Install Sensors | Known | ★ Radio Op/Lab Tech: twice as fast | Place 3 sensors in infested towns | Fame; twice the specials there for 2 hours | ✅ *15 s at each spot* |
| 8 | Blood Drive | Known | 🔒 not infected | Donate blood at the lab (costs health) | Fame, small research | ✅ *a quarter of your health, refused under 70%, once per life; amends* |
| 9 | Cold Chain | Known, cure 50%+ | ★ Nurse/Doctor: 25 min, not 15 | Get an antidote batch to a Haven clinic before it spoils | Fame with Halcyon and the Haven | ✅ *the clock starts when it's picked up; the clinic is a Haven drop at a safehouse* |
| 10 | Escort the Scientist | Trusted | | Escort a scientist to take readings at 3 sites | Fame, research | ✅ *she follows whoever leads her; 20 s within 15 m of each site* |
| 11 | Specimen from the Nest | Trusted | | Get a sample from inside a hive without destroying it | Big research | ✅ *20 s within 4 m of the nest, then carry it to the lab. Needs the nest prefab* |
| 12 | Hunt the Screamer | Trusted | ★ Hunter: +50% Fame | Kill a Screamer before it alerts the town (4 min) | Fame, rare sample | ✅ |
| 13 | Quarantine Breach | Trusted | | Kill the infected inside a safe town within 10 minutes | Fame (Haven Fame too) | ✅ |
| 14 | Decontamination | Trusted | ★ Nurse: faster | Burn four corpse piles around the lab | Fame; the lab's town less infected | ✅ |
| 15 | Lab Defence | Trusted | | Hold the lab for 4 minutes against waves (and Red Hand raiders if their prefab is set) | Fame (Red Hand Infamy if raiders) | ✅ |
| 16 | Medical Drone | Trusted | ★ Mechanic/Radio Op: the exact grid within 600 m | Recover a crashed prototype drone | Fame, a little research | ✅ *somewhere within 250 m of its last ping; carry its core to the lab* |
| 17 | Patient Zero | Champion | ★ Lab Tech: clues also give the grid | Follow clues through 4 towns to the outbreak's origin | Huge research | ✅ *a body in each town names the next; at the last, 45 s searching the clinic while infected come* |
| 18 | Live Capture | Champion | | Lure a special into a cage trap alive | Huge research | ✅ *or wound it and tie it; hold it 30 s, then carry the case to the lab within 12 min* |
| 19 | Serum Trial | Champion | 🔒 infected only | Take an experimental serum while bitten: it cures you or speeds up the turn | Huge Fame; research either way | ✅ *50/50; the turn comes twice as fast. Separate from the everyday trial dose at Halcyon's quartermaster ([DESIGN.md](DESIGN.md#treatments))* |
| 20 | Mobile Lab | Champion | | Escort the lab truck to a safer town | Fame; a field annex there takes samples for 3 hours | ✅ *the lab never moves (it's the stronghold); players drive the truck; the annex is a second lab prefab* |

**Amends:** Field Samples and Blood Drive work off Halcyon Infamy while you're Watched.

---

## World events

Announced to everyone, open to everyone. Most pay in loot found on the spot rather than Fame.
Code: `Missions/Events/`; numbers and optional prefabs in **Missions > Events**
([WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)). Nothing comes in from outside: every reward was
already in the Zone, and every crate is earned on the spot.

| # | Event | What happens | Why it's interesting | Status |
|---|---|---|---|---|
| 1 | Crashed Helicopter | The wreck of one of Bastion's lost helicopters, its cargo still aboard | Infected nest around it; a horde comes when someone arrives | ✅ |
| 2 | Horde Migration | A huge horde walks from one town to another | Get out of the way, or thin it out: 10+ kills leaves a crate | ✅ |
| 3 | Night of the Dead | For 20 minutes, twice the infected and twice the specials everywhere | High risk, specials everywhere | ✅ |
| 4 | Fog Bank | A region drops to low visibility for 20 min | Hiding is easy. So are ambushes | ✅ *the engine's fog is global and never changed: within 700 m of a town, heat is ×0.6 and infected spawn twice as close; an optional mist prop is the look* |
| 5 | Distress Call | A survivor asks for help on the radio | Could be real, could be a trap (see 19) | ✅ *two people cornered at a town's edge; infected surround the house once someone is within 300 m. Talk them out with nothing within 20 m: they join the nearest safehouse and leave a crate. 25 min. Radio Operators hear that it's live* |
| 6 | Abandoned Convoy | Bastion trucks left on a highway | Loot, but it may be booby-trapped or watched | ✅ *Bastion's own trucks, inside the Zone since the first week, halfway between two towns (not snapped to a road); two crates and a few infected. 40% a tripwire: a Mechanic, Soldier or Police within 25 m cuts it, else the first to the crates sets it off (an explosion if the prefab is set, 80 heat) and one crate is lost. 35% a Red Hand crew watches it and fights all but its friends. The wrecks are optional props* |
| 7 | Power Surge | A town's power comes back for 10 min | Alarms go off (heat); electronics loot | ✅ *a dark town outside the grid's region is Powered through the power grid and town memory; three alarms make heat for 90 s; the grid's electronics search bonus applies* |
| 8 | Siren Test | Air-raid sirens pull every infected into one town for 5 minutes | The towns around it are briefly safer to loot | ✅ |
| 9 | Wandering Trader | A neutral merchant parks somewhere for 20 min | Rare items; protect him or rob him | ✅ |
| 10 | Gas Station Fire | A station explodes and burns | Infection drops; loot is destroyed | ✅ *a real petrol station (by name) in a town nobody lives in; it goes up 3 min after the warning: infected within 30 m die, the station and up to two buildings beside it burn (the fire system), town infection −8. Needs fire on* |
| 11 | Prison Break | Escaped convicts roam the area | Hostile; the Red Hand recruits them | ✅ *two groups from a town's police station walk to the nearest Red Hand camp (data until a player is within 600 m). They fight anyone near them except Red Hand friends (Known+). All dead: a crate where the last fell. Any reach the camp, or 30 min pass: the Red Hand gains strength, a headline, and its friends who walked in with them +4 Fame* |
| 12 | Bastion Shelling | Bastion's own mortars shell an infested town after a 5-minute warning | Get out, or loot fast. Infection there drops sharply | ✅ |
| 13 | Bell Tower | Someone rings church bells nonstop | A horde converges | *covered* by The Preacher's Flock arc: he rings bells in every town (twice the heat) and small hordes come for him |
| 14 | The Sniper | A crazed survivor shoots from a tower | Take him down; his stash is up there | ✅ *on the high ground above a town, not up a tower: one armed AI, there only while a player is within 900 m, who fights everyone (Red Hand friends too); his shots make heat. Dead: his stash. 30 min, then he moves on* |
| 15 | Stash Note | A note with coordinates turns up | Treasure hunt | ✅ |
| 16 | Mass Grave | A pit full of infected bodies | Lots of samples, and a horde underneath | ✅ |
| 17 | Lost Child | A child wanders alone | Escort them to the Haven | *covered* by the Kid wanderer: show them the way and they follow you to a safehouse (+1 person, +5 Haven Fame) |
| 18 | Burning Town | A town burns for 15 min | Infection there drops sharply | ✅ *one building every 100 s through the fire system, at most 8 and never more than 40% of the town; −3 infection and two infected killed per building. Only infested towns (50+) nobody lives in. Needs fire on* |
| 19 | Fake Distress Call | A Red Hand ambush | Radio Operators are warned | ✅ *announced exactly like Distress Call (never both at once): a tape on a crate, two Red Hand groups in the trees (they appear within 400 m and close in at 60 m) who spare Red Hand friends. All dead: their kit. Radio Operators are told it's a loop* |
| 20 | Storm | For 15 minutes, noise carries less and heat fades three times as fast | A good time to move | ✅ |
| 21 | Last Broadcast | A station is still on the air | Reach it and broadcast | *covered* by the radio station: always on the map rather than an event; take it over and broadcast a preset (cosmetic, no Fame) |
| 22 | Rival Survivors | An unaligned AI group | They trade or fight depending on your reputation | ✅ *a group on `SURV` camps outside a town for 30 min. Hunted by anyone, or Watched by two factions: they fight you (only you). Trusted by someone and Watched by nobody: a crate of their kit for 2 food; anyone else 3 food; two trades in all. Gunning them down when they'd fought nobody: +6 Haven Infamy; if they had: their kit* |
| 23 | Locked Bunker | A sealed bunker, code split over 3 notes | Teamwork or a race between strangers | ✅ *a real locked building in a town: its door lock becomes a keypad (no key or crowbar while it runs), two crates inside, six digits on three bodies around town. The right code unlocks it for everyone; three wrong ones lock the keypad for 2 min (noise). No lockable building, or door locks off: a steel hatch and the crates come up when it opens. 40 min* |
| 24 | Turned Player | A player who turned roams with their gear (started when someone turns) | Their loot is up for grabs | ✅ |
| 25 | Tunnel Rats | Infected flood out of tunnels | A town's infection spikes | ✅ *three culverts at the town's edge (the generic puzzle object); each open one adds 1 infection a minute (at most +20) and sends two infected into town while a player is within 700 m. Collapse all three (no infected within 12 m) for a crate and −5 infection* |
| 26 | Military Remnant Camp | An abandoned Bastion camp with gear | Bastion Infamy if they're still around | ✅ *outside an infested town, never within 1.5 km of a Bastion holding: two crates, turned soldiers and a special. While Bastion is still around (military presence 0.1+), coming within 4 m of a crate is taking: +10 Bastion Infamy, once per player. The camp is an optional prop* |
| 27 | Wedding Party | A horde in formal wear by a church | Dark humour, jewellery for trading | ✅ *six guests and the bride by a town's church, there once a player is within 500 m. About half drop a piece of jewellery, the bride always (at most 6); a Trader wanderer takes one instead of Favor. No jewellery prefab: a crate once they're all down* |
| 28 | Collapse Shelter | A town's shelter is overrun and survivors scatter | Save them before they turn (Haven Fame) | ✅ *four people hide round a town's school or church, each with infected close by; they turn after 15–22 min. Tell each the way (nothing within 15 m) and they join the nearest safehouse; the Haven's 12 Fame and 8 Favor go to the rescuers, scaled by how many were saved* |

---

## Puzzle missions

Contracts you solve by reading, deducing and sneaking rather than shooting; most low-tier ones
can be done unarmed. Each works on any map (built from settlements and real landmark buildings
found by prefab name, with a fallback prop) and deletes everything it spawned when it ends.
Code: `Puzzles/`. ✅ all 17.

| # | Puzzle | From | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|---|
| 1 | The Numbers Station | Haven | Any | ★ Radio Op: hears it without the note or mast | A radio log gives a frequency. Of three radio masts, one carries a voice reading a map grid, every third number a decoy | Fame; stash at the grid |
| 2 | Two Bearings | Bastion | Any | ★ Hunter/Soldier: more Fame | Two wreck pieces have bearings scratched on them. Reach where the lines cross | Fame; the pilot's kit |
| 3 | Paces from the Tower | Haven | Any | ★ Hunter: paces also in metres | "From the church, face the water tower, turn right, 400 paces." | Fame; stash |
| 4 | Hot and Cold | Haven | Any | ★ Radio Op: numeric signal strength | With a radio, find a beacon in the fields by how loud it sounds, within 10 m | Fame; stash |
| 5 | The Rat | Red Hand | Known | ★ Police/Ex-Con: more Fame | Three suspects, exactly one lying. Point at the rat at the boss | Fame. Wrong: fails, Red Hand Infamy |
| 6 | Patient Zero's Ledger | Halcyon | Known, a lab | ★ Lab Tech/Doctor: the method spelled out | A ledger plus hospital bracelets on three bodies. Name who was first, at the lab | Fame, research. Wrong: locked 5 min; twice: fails |
| 7 | The Poisoned Well | Halcyon | Any | ★ Lab Tech/Nurse: can test the water | Three wells, two notes ruling one out each. Seal the poisoned one | Fame, research, that village −10 infection. Wrong: infection up; twice: fails |
| 8 | The Smuggler's Ledger | Red Hand | Known | ★ Ex-Con: more Fame | A Caesar-shifted ledger line; the key is on a memorial plaque by the church | Fame; stash |
| 9 | The Clockwork Horde | Haven | Any | ★ Hunter: harder to notice | A stuck siren sends a horde back and forth through a square. Take the strongbox unnoticed | Fame; the strongbox. Noticed: the horde turns on you |
| 10 | Time-Lock Safe | Red Hand | Known | ★ Ex-Con: window twice as long | A bank safe opens only at 03:00 game time, for 20 min. Opening it is loud | Fame; the safe. Offered only when 03:00 is at most 3 real hours away |
| 11 | The Bell Order | Haven | Known | ★ Elder: more Fame | Ring three village bells in the order on a hymn sheet, at most 15 min apart | Fame; heat and infection drop at the safehouse. Wrong: a horde, start over |
| 12 | Lights in the Window | Bastion | Known, night | ★ Radio Op: Morse on the radio too | A lamp blinks a code word in Morse. Decode it and reach the right meeting place | Fame; a pack |
| 13 | Junction Boxes | Bastion | Any | ★ Mechanic: one more switch named | Four fuse boxes, two switches each, lights-out wiring. Get all eight green | Fame; the substation's contents |
| 14 | The Weighing Vault | Haven | Known | ★ Mechanic: reads to 0.1 kg | Fill a scale crate to the weight in a riddle (±0.2 kg) | Fame; the vault. *Simplified option:* count items instead of weight (a setting) |
| 15 | The Last Experiment | Halcyon | Trusted | ★ Lab Tech: first letter given | Each missing researcher's belongings carry one letter of a password. Type it on the keypad | Big research. Three wrong: wiped |
| 16 | The Silent Village | Haven | Trusted | ★ Elder: more Fame | A village with no infected. Find what's under the church, then **burn it**, **seal it**, or **let Halcyon study it** | Depends on the choice |
| 17 | The Witness | Haven | Known, night | ★ Police: talks by day too | A wanderer saw a Haven courier killed. His story points to a camp. **Report it to the Haven** or **sell it to the Red Hand** | Depends on the choice |

How they work:

- **Clues** are notes, bodies, plaques, wreck pieces or people with `RZ_ClueComponent`; reading
  one sends its text to that player only.
- **Answers** go through actions on one generic puzzle object (keypad, choices, uses); the
  mission labels them.
- **Wrong answers** cost something: heat, a horde, a lock-out, infection, Infamy or failure.
- **Night missions** appear only at or near nightfall and last until dawn. Time is never skipped.

## Unarmed missions

Low-level work for new characters, unarmed backgrounds or anyone who'd rather not shoot. All
are open to anyone not Hunted by the giver, pay small Fame, and are built on **noise that
leads** and **being noticed** ([DESIGN.md](DESIGN.md), "Heat and noise"). Code: `Missions/Unarmed/`.

| # | Mission | From | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | Pharmacy Lure | Haven | ★ Nurse/Doctor: more Fame | Four infected stand in a pharmacy. Make noise away from the door to draw them out, take the medicine to a safehouse | Fame, Favor. Noticed: they come for you, 60% pay | ✅ |
| 2 | Silent Courier | Haven (a Courier wanderer's job) | ★ Hunter: heat limit 45 | Carry a letter through an infested town; heat where you stand must stay under 35 | Fame, Favor. Too loud: fails. Went round: half pay | ✅ |
| 3 | Lead Them Away | Haven | ★ Radio Op: more Fame | A horde drifts toward a safehouse. Lead it with noise to a quarry 550–750 m away | Fame, Favor. It arrives: fails, two people die. Killed instead: half pay | ✅ *the quarry is a grid reference* |
| 4 | Rooftop Rescue | Haven | ★ Mechanic: 15 m is clear enough | A survivor is stuck up a building. Talk them down when no infected is within 25 m | Fame, Favor; they join a safehouse | ✅ *they wait at the foot of the building* |
| 5 | Trail of Clues | Haven (for a wanderer) | ★ Police: grid references | Follow a note, then a bag, then a third clue to a missing person | Alive (half the time): they join a safehouse. Dead: their last letter, 70% pay | ✅ |
| 6 | Water Run | Haven | | Carry water to a safehouse 0.8–2.5 km away; the fast road goes through an infested town | Fame, Favor, +2 food. Fast and unnoticed: ×1.33. Noticed: ×0.7 | ✅ |
| 7 | Barter Chain | Haven, through three wanderers | | Each wants what the next has; the last wants food | Small Fame, Favor; all three remember you | ✅ *only the food changes hands* |
| 8 | Distraction | Any faction | | Someone makes noise across the street while someone else searches a guarded cellar for 3 minutes | Fame, Favor, a reward crate. Doable alone with alarm clocks | ✅ |
| 9 | Medicine Before Dark | Haven (for a Doctor wanderer) | ★ Nurse/Doctor: more Fame | Bring medicine to a bitten survivor before 20:30 | Fame, Favor; they join a safehouse. Dark first: fails | ✅ |
| 10 | Quiet Count | Halcyon | ★ Lab Tech: more Fame | Count the specials in a town from outside and write it in a notebook | Fame, Favor, +1 research. Too close: half pay. Wrong: fails | ✅ |
| 11 | Car Keys | Haven, from a Mechanic wanderer ("Ask about cars") | | A car the Mechanic fixed stands in a town 0.5–2.5 km away; its keys are in one of the 5 houses nearest it that have no lock of their own. Search them quietly and drive it back to where you met the Mechanic | Fame, Favor, for the one who asked and whoever went with them. Heat 45+ at the house when the keys turn up: a horde, ×0.7. Hotwired: half | ✅ *a fixable car that already runs (no parts); searching the right house always finds the keys; the Mechanic keeps the car (it leaves the world once nobody sits in it). One at a time, one per player per 30 min* |

Four more designed ideas are covered by existing missions: The Safe Code (The Last Experiment,
Time-Lock Safe), Breaker Sequence (Junction Boxes), Scout the Town (Recon) and Night Lantern
(Lights in the Window, Signal Fires).

## Perimeter missions

Jobs at the quarantine line ([DESIGN.md](DESIGN.md), "The Perimeter"). Nothing crosses the line in
either direction; these happen at it. Rare and dangerous on purpose; nothing is built for them. They never appear while the Perimeter
is off. Code: `Missions/Perimeter/`.

| # | Mission | From | Needs | What you do | Result | Status |
|---|---|---|---|---|---|---|
| 1 | The Blind Spot | Red Hand | Known, night | Ten minutes after the offer, a 250 m stretch of the edge goes uncovered for ten minutes. A Red Hand runner waits in the kill zone with a package from the cache it hides there. Take it to a Red Hand drop 500–700 m inland before the drones come back | Fame, Favor. The window closing: fails, and anyone still in the kill zone is shot | ✅ *window in real minutes; offered only if it ends before dawn* |
| 2 | The Crossing | Haven | Any, night | A Haven runner dropped a medicine package at the waterline (or any dry edge) when a drone found her. Take it out of the warning zone before the drone locks on (45 s with the package), and deliver it | Fame, Favor. Lock-on: the package is shot, fails | ✅ |
| 3 | Shot at the Line | Any faction | Any | A body lies in the warning zone. Search it without going further; its papers name a stash | Fame, Favor; the stash (a reward crate) | ✅ |

## Haven house contracts

Morally grey work around the safehouses and their rules ([LORE.md](LORE.md), "The Haven";
[REPUTATION.md](REPUTATION.md), "Haven house rules and exile"). Nobody in them is a villain.
They need at least one safehouse. Code: `Missions/Haven/RZ_HavenHouseMissions.c`.

| # | Contract | From | Needs | Bg | What you do | Result | Status |
|---|---|---|---|---|---|---|---|
| 1 | Walk Him Out | Haven | Known, dusk | ★ Nurse: reads the bite; ★ Nurse/Elder: more Fame | A bitten member declared it himself. At dusk he waits at the edge of town. **End it here** or **let him walk on** | End it: Fame. Walk on: 60% Fame, town infection +5. Nobody comes in 40 min: he turns | ✅ *he waits rather than walking with you* |
| 2 | The Ledger | Haven | Known | ★ Elder: more Fame | A member has been taking extra from the stores. **Threaten** them, **pay their debt** (20 Haven Favor), or **tell them to disappear** | Threaten or pay: Fame. Warn off: no pay, +3 Haven Infamy. They remember you | ✅ |
| 3 | A Quiet Arrangement | Red Hand | Known | ★ Ex-Con: lookouts spot you at 25 m | A safehouse keeper pays the Red Hand to be left alone. Carry the envelope to a Red Hand drop without being seen by two Haven lookouts or a wanderer | Small Red Hand Fame, Favor. Seen: +12 Haven Infamy. Never delivered: the safehouse loses 2 people | ✅ *the lookouts are props* |
| – | Letters (the Courier's job) | Haven, via a Courier wanderer | Any | | Carry letters to a safehouse or quiet town 0.8–4 km away | Small Haven Fame, Favor: only the one who asked, and anyone with them at the drop (taking the letters off them pays nobody) | ✅ |

## Leader missions

Personal jobs that Independent and Rogue leaders give **in person**, off their stronghold's books
([REPUTATION.md](REPUTATION.md), "Faction leaders"). Code: `Missions/Leaders/`.

- **Who gets offered what.** The leader's attitude decides who may take it, not your tier, so a
  Rogue leader can hire someone their stronghold hunts. Traits pick the job's tone.
- **Pay.** Independent jobs pay a little Fame and Favor; Rogue jobs pay Favor only, plus any Fame
  with the other faction involved. Rewards never include loot.
- **Rogue jobs can be discovered**, and most rogue deliveries have a second drop at the stronghold
  where you can **turn the leader in** instead.

| # | Job | From | What you do | Result | Status |
|---|---|---|---|---|---|
| 1 | Our Own First | Haven, Independent | Supplies meant for another safehouse wait in a town. Bring them here, or where they were meant to go | Here: +4 food. There: that one +4 food, half pay | ✅ |
| 2 | Bring Them Home | Haven, Independent | Find a local's family in another town. One of them is bitten | All three: +3 people, infection +5. The two who are well: +2 people, less pay | ✅ |
| 3 | Over the Fence | Haven, Independent | Take bread to a Bastion post for medicine | Bastion Fame +3; infection −3 | ✅ |
| 4 | The Old Clinic | Haven, Independent | Hold a clinic in the next town for 3 min (2 waves) | That town −10 infection; +1 person | ✅ |
| 5 | Ration Run | Haven, Rogue | Three days of the stores go to the Red Hand for antibiotics | Stores −3 food; Red Hand Fame +3; infection −3 | ✅ |
| 6 | Names for Bastion | Haven, Rogue | Deliver a "troublemakers" list to Bastion for protection | Bastion Fame +3; players the Haven exiled take +5 Bastion Infamy | ✅ |
| 7 | Shut the Door | Haven, Rogue | A family at the edge of town asks to come in. Turn them away | Turned away: paid. Let in: +3 people, no pay | ✅ |
| 8 | Hidden Stores | Haven, Rogue | Move the council head's hidden stash before the count (20 min) | Paid. Fail it: the leader is discovered. Turn it in: +5 food | ✅ |
| 9 | Hearts and Minds | Bastion, Independent | Rations to a Haven safehouse, against orders | +4 food; Haven Fame +4 | ✅ |
| 10 | The Real Orders | Bastion, Independent | Recover command's last orders from a dead courier (guarded) | To the commander or to the stronghold, with different loyalty effects | ✅ |
| 11 | Training Day | Bastion, Independent | Stand with raw recruits at a town's edge for 4 min (3 waves) | Paid | ✅ *recruits are real only with the survivor group prefab* |
| 12 | Local Law | Bastion, Independent | Two families fight over a well. Hear both, then decide | Split: full pay. One side: 80%, the other leaves | ✅ |
| 13 | Rifles for Sale | Bastion, Rogue | Army rifles to a Red Hand buyer | Red Hand Fame +4 | ✅ |
| 14 | Off-Book Requisition | Bastion, Rogue | Collect a "contribution" from a village | All: +6 Haven Infamy, −4 food. Half: 60% pay. Pay it yourself: 15 Bastion Favor | ✅ |
| 15 | Burn the Ledger | Bastion, Rogue | Burn the supply ledger before the audit (15 min) | Burn it: paid. Keep it: the commander is turned in | ✅ |
| 16 | Desertion Route | Bastion, Rogue | Walk two deserters to a safehouse, or march them to the stronghold instead | Paid; they join the safehouse. Marched in: the commander is turned in | ✅ *a safehouse only, not the Blind Spot* |
| 17 | Our Crossroads | Red Hand, Independent | Hold the crew's crossroads for 5 min (3 waves) | Paid | ✅ |
| 18 | Honour Among Thieves | Red Hand, Independent | Recover what a crew member stole | Take it: paid, loud. Talk it back: 60%. Let him keep it: fails | ✅ |
| 19 | House Rules | Red Hand, Independent | The crew's own fight pit (4 waves) | Mostly Favor (the bets) | ✅ |
| 20 | Debt Collection | Red Hand, Independent | Collect from a debtor (a real wanderer if free): talk, threaten or pay | Talk: 60%. Threaten: full, +3 Haven Infamy. Pay it: 15 Red Hand Favor | ✅ |
| 21 | The Soft Touch | Red Hand, Rogue | Secret medicine to a Haven safehouse | Haven Fame +5; infection −5 | ✅ |
| 22 | Coup Talk | Red Hand, Rogue | Carry a word to two other crews | Never fully works: laughed off, or a purge | ✅ |
| 23 | Sell Out a Rival Crew | Red Hand, Rogue | A map of another crew's camp, to Bastion | Bastion Fame +4; another crew boss dies | ✅ |
| 24 | Skim the Toll | Red Hand, Rogue | Take the boss's toll on the road for 5 min | Paid; +3 Haven Infamy | ✅ |
| 25 | Field Station | Halcyon, Independent | Hold a house in a quiet village for 3 min | Research +1 | ✅ |
| 26 | Second Opinion | Halcyon, Independent | Notes to a Haven medic | Research +1; Haven Fame +3 | ✅ |
| 27 | Patients Not Samples | Halcyon, Independent | A dying man: sit with him, or take the sample | Sit: Haven Fame +3. Sample: research +1, half pay | ✅ |
| 28 | The Notebook | Halcyon, Independent | A dead researcher's notebook (guarded) | To the field lead: research +1. To the director: research +2, +10 Fame | ✅ |
| 29 | Unsanctioned Trial | Halcyon, Rogue | A willing, dying volunteer and an untested serum | Half the time he lives (research +2), half he turns (research +1, a special) | ✅ |
| 30 | Leak the Data | Halcyon, Rogue | Research data to a Haven safehouse for nothing | Haven Fame +5; infection −3 | ✅ |
| 31 | Private Collection | Halcyon, Rogue | Capture a special alive for the field lead | Paid. Taken to the lab instead: the field lead is turned in, research +3 | ✅ |
| 32 | Sell the Formula | Halcyon, Rogue | The antidote formula to the Red Hand | Red Hand Fame +5 | ✅ |
| – | Rein In | The stronghold (radio, Known) | A leader turned Rogue. Read them the terms, or take their counter-offer | Terms: loyalty +25. Counter-offer: Favor, 35% the stronghold finds out (+10 Infamy) | ✅ |

*Simplified:* "in person" means at the leader's seat. Families are one stand-in figure; people
join or leave as numbers. The other side of a trade is a drop point at their nearest seat (within
5 km), else outside a town. Rein In is offered a few minutes after a leader turns Rogue, then at
most hourly.

## Story arc missions

Story arcs ([DESIGN.md](DESIGN.md), "Story arcs and headlines") mostly reuse the missions above:
Lost Colleague (as "The missing courier"), Lab Defence, the supply trucks and the Preacher. Three
missions exist only inside arcs and are never picked at random:

| Mission | Arc | Kind | What you do | Result | Status |
|---|---|---|---|---|---|
| The Deserter's Map | The Deserter's Map | World event | Take a map piece from a dead deserter; get the other two from the wanderers it names; reach the bunker (four infected) | The bunker's stores (a reward crate). A holder dies first: fails | ✅ |
| Fever Town: the water | Fever Town | Halcyon contract, Any, ★ Lab Tech | The Water Survey, starting at the sick safehouse's well | Fame, Favor; the arc goes on | ✅ |
| Fever Town: get them out | Fever Town | Haven contract, Any, ★ Nurse/Doctor: faster | Hold the safehouse gate for 2 minutes (2 waves) | Fame, Favor; two people move, the added infection goes. Fails: two die | ✅ |

## Backgrounds at a glance

Every background has contracts it shines at, and some it's shut out of.

| Background | Shines at | Shut out of |
|---|---|---|
| Nurse | Medicine Run, Bitten Friend, Walk Him Out, Pharmacy Lure, Medicine Before Dark, Rescue the Pilot, Cold Chain | — |
| Mechanic | Clear the Road, Crash Site, Junction Boxes, Rooftop Rescue, Fuel for the Lights, Water Purifier, Fortify the Safehouse, Recover the Bus, Convoy Escort, Blow the Bridge, Car Theft | — |
| Hunter | Rescue the Kids, Smuggler's Run, Hunt the Screamer, Silent Courier, Harvest Guard | — |
| Police | Checkpoint Duty, Trail of Clues, The Rat, Curfew Patrol, Loot the Armory, Double Agent | The Informant |
| Ex-Con | Dead Drop, Fence the Goods, A Quiet Arrangement, Time-Lock Safe, Bust Out | Raid the Red Hand (until Champion with Bastion) |
| Lab Tech | Field Samples, Water Survey, Quiet Count, Steal the Antidotes, Water Purifier, Install Sensors, Patient Zero | — |
| Radio Op | Radio Relay, Recon, Zombie Bait, Lead Them Away, Restore Comms, Install Sensors, Medical Drone | — |
| Unemployed | +50% Fame from every contract (and Infamy worked off by amends) | — |
| Elder (unlock) | Walk Him Out, The Ledger, The Bell Order, The Silent Village | Requisition, Protection Racket |
