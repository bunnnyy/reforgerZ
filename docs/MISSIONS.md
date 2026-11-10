# Missions

There are two kinds of missions:

- **World events** (28) are announced to everyone, and anyone can take part.
  They are what makes strangers meet: a horde, a helicopter wreck, a town about to be shelled.
- **Faction contracts** (20 per faction, plus 17 puzzle missions, 10 unarmed missions, 3 Perimeter missions, 3 Haven house contracts and 32 leader missions, 30 of them in code) are offered only to players who
  qualify, by radio or at the faction's contact. A contract belongs to the
  person or group that accepts it. Others can still turn up and interfere.

## Who gets offered what

A contract has up to four requirements, and you must meet all of them:

1. **Tier with the giver.** Known, Trusted or Champion. Big jobs need trust.
2. **Standing with others.** For example, *not Hunted by Bastion*, or *at least Watched by Haven*.
   Some only open *because* someone hates you, like the Red Hand recruiting you after Bastion turned on you.
3. **Character type.** Some contracts only exist for certain backgrounds, and
   some backgrounds get an extra option or a better outcome on a shared contract.
4. **World state.** Outbreak stage, cure progress, whether the military is still around.

Reading the tables:
- **Needs:** tier with the giver first, then any other requirements. *Any* = Stranger or better and not Hunted.
- **Bg:** background lock (🔒) or bonus (★).
- **Result:** what you gain, and in brackets what it costs you with other factions. Every contract also pays Favor.
- ✅ = already implemented in code.

Backgrounds: Nurse, Mechanic, Hunter, Police, Ex-Con, Lab Tech, Radio Op, Unemployed;
unlockable: Soldier, Doctor, Raider, Elder, Drifter.

### How the code does it

- There's no "accept" step. A contract is offered by radio (one new offer per player a
  minute) or by the faction's quartermaster when you walk up. Anyone can join in, but only
  players who meet the requirements when it ends are paid.
- Missions marked ✅ are built from a few blocks: hold an area, kill what the mission spawned,
  visit points, destroy a prop, carry an item to a drop point, or choose which drop point gets it.
  Some are simpler than the table says. For example Clear the Road kills the infected on the
  road (no wrecks), Toll Road and Checkpoint Duty are 10-minute holds with waves, Zombie Bait
  is "make enough noise at the checkpoint", and Field Samples counts kills in the town.
- Missions that need an optional prefab (drop points, mission item, helicopter wreck, Red Hand
  AI group, fuel depot, relay, trader, explosion) never appear until that prefab is set.
  See [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md).
- ★ bonuses in code: shorter holds (Radio Relay, Crash Site, Checkpoint Duty, Rescue the Kids,
  Steal the Antidotes), more Fame (Retake, Survivors Trapped, Medicine Run, Clear the Road,
  Recon, Fence, Water Survey, Hunt the Screamer), faster burning (Burn the Bodies,
  Decontamination), double kills (Field Samples), a slower infection (Bitten Friend),
  better pay (Dead Drop), fewer courier reports (Smuggler's Run), more noise (Zombie Bait),
  reading the bite and more Fame (Walk Him Out), more Fame (The Ledger), harder to spot
  (A Quiet Arrangement).
- **Rescues and safehouses.** Rescued people become population at the nearest Haven safehouse
  ([WORLD.md](WORLD.md), "People"): Survivors Trapped adds 3 (4 with a ★), Rescue the Kids adds 3.
  Medicine Run's drop point is at a real safehouse when there are any. Requisition's supplies
  are real: delivered to Bastion, the nearest safehouse loses 4 food.
- **Supply trucks aren't contracts.** Escorting one, driving one in after its crew died, or
  stealing one to a rival's stronghold pays directly ([WORLD.md](WORLD.md), "Supply lines").
  Convoy Escort and Hijack the Convoy are still planned. What a faction needs right now (lost
  trucks, deaths, research) makes matching contracts more likely ([REPUTATION.md](REPUTATION.md),
  "Morale, needs, truces and feuds").

---

## The Haven (Survivors): 20 contracts

| # | Contract | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|
| 1 | Supply Cache ✅ | Any | | Hold a cache in a town for 60 s | Haven Fame |
| 2 | Survivors Trapped ✅ | Any | ★ Nurse: survivors leave healthier, bonus Fame | Reach a barricaded family and survive 3 waves | Haven Fame; the family joins a safehouse (+3, +4 with ★) |
| 3 | Radio Relay ✅ | Any | ★ Radio Op: half the hold time | Keep a relay on high ground running for 3 min | Haven Fame |
| 4 | Medicine Run ✅ | Any | ★ Nurse/Doctor: knows the pharmacy; it's pre-marked | Bring medical supplies from a pharmacy town to a Haven safehouse | Haven Fame |
| 5 | The Bitten Friend ✅ | Known | ★ Nurse: can slow his infection | A Haven member is bitten. Bring an antidote before he turns, or end it | Fame; mercy kill = less Fame |
| 6 | Fuel for the Lights | Known | ★ Mechanic: generator runs twice as long | Bring 2 jerrycans to a safehouse generator. The lights raise heat there | Haven Fame |
| 7 | Take Back the Town ✅ | Known, Bastion gone (Overrun+) | | Kill 8 × size infected in town | Fame, town infection -35 |
| 8 | Harvest Guard | Known | ★ Hunter: farmers stay hidden longer | Protect farmers in the fields for 10 min | Haven Fame, food crate |
| 9 | Missing Scout ✅ | Known | | Search an area for a scout's body and recover his notes | Fame, reveals a stash |
| 10 | Clear the Road ✅ | Known | ★ Mechanic: can tow wrecks away instead of blowing them | Clear wrecks and infected off a route between two Haven towns | Fame, safer travel event |
| 11 | Burn the Nest ✅ | Trusted | | Destroy a hive in an infested town | Fame, infection -30 |
| 12 | Refugee Column | Trusted | | Escort a group walking a road to a safe town | Big Fame (Bastion Infamy if you go through a checkpoint) |
| 13 | Signal Fires ✅ | Trusted | needs 3 players | Light beacons on 3 hills within 2 minutes of each other | Fame for each player; nearby towns' infection drops |
| 14 | Water Purifier | Trusted | ★ Mechanic/Lab Tech: only need 2 of 3 parts | Find 3 purifier parts scattered around a town | Fame, the safehouse becomes a spawn point |
| 15 | Fortify the Safehouse | Trusted | ★ Mechanic | Hold the area while barricades go up (or build them with the building mod) | Fame; the safehouse resists attacks |
| 16 | Trader Caravan ✅ | Trusted, not Trusted by the Red Hand | | Defend a Haven trader at a crossroads from a Red Hand raid | Fame (Red Hand Infamy) |
| 17 | Rescue the Kids ✅ | Trusted | ★ Hunter: quieter approach | Get children out of a school without making noise. Too loud and a horde comes | Big Fame; the kids join a safehouse (+3) |
| 18 | The Informant | Trusted, Watched+ by the Red Hand | 🔒 not Police/Soldier | Pose as a buyer at a Red Hand dead drop and report who shows up | Fame (Red Hand Infamy if spotted) |
| 19 | Recover the Bus | Champion | ★ Mechanic | Get a bus or truck running and drive it to the safehouse | Fame; the Haven gains a vehicle |
| 20 | Memorial | Champion | | Collect ID tags of the dead from an overrun city | Fame; unlocks the Haven Champion vehicle shop |

**Amends with the Haven:** *Medicine Run* and *Survivors Trapped* can be done
while Watched by the Haven. They remove Infamy instead of adding Fame.

---

## Task Force Bastion (Military): 20 contracts

| # | Contract | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|
| 1 | Retake the Town ✅ | Any, Bastion present | ★ Police/Soldier: +25% Fame | Kill 8 × size infected in a town | Bastion Fame, infection -35 |
| 2 | Last Helicopter ✅ | Any, Bastion still around, no storm | | Bastion's one working helicopter lands at an LZ on its run between its own bases. Keep the LZ clear and be there when it lifts off. Storms ground it ([WORLD.md](WORLD.md), section 5) | Fame, Favor |
| 3 | Burn the Bodies ✅ | Any | ★ Nurse: faster | Burn 5 corpse piles in a town to slow infection | Fame, spread slows there |
| 4 | Recon ✅ | Any | ★ Hunter/Radio Op: reports count double | Visit 3 observation points and report horde sizes | Fame; the next horde is announced earlier |
| 5 | Checkpoint Duty ✅ | Known | ★ Police | Hold a checkpoint 10 min. Let survivors pass, stop armed bandits | Fame (Red Hand Infamy for each bandit you stop) |
| 6 | Missing Patrol ✅ | Known | | Find a squad's last position, recover dog tags, deal with the turned soldiers | Fame, military loot |
| 7 | Crash Site ✅ | Known | ★ Mechanic: recovers the black box faster | Secure the wreck of a Bastion helicopter lost in the first week, and its black box | **Choice:** Bastion Fame, or sell the box to the Red Hand |
| 8 | Restore Comms | Known | ★ Radio Op: needs no parts | Bring parts and repair a radio tower | Fame; Bastion intel on the radio |
| 9 | Curfew Patrol | Known, night only | ★ Police | Patrol a town at night and keep it clear | Fame |
| 10 | Weapons Cache | Trusted | | Retrieve weapons from an armory in an overrun base | Fame; unlocks the Bastion rifle shop |
| 11 | Convoy Escort | Trusted | ★ Mechanic: can fix a broken-down truck | Escort supply trucks between two towns | Big Fame (Red Hand Infamy if you kill raiders) |
| 12 | Rescue the Pilot | Trusted | ★ Nurse/Doctor | Find a downed pilot behind the lines and bring them out | Fame |
| 13 | Quarantine Line ✅ | Trusted | | Hold the edge of a town for 10 min while it's sealed off | Fame; that town stops spreading for a while |
| 14 | Clear the Airfield | Trusted | | Clear an airfield so the Last Helicopter can land there | Fame; the Last Helicopter runs more often |
| 15 | Raid the Red Hand ✅ | Trusted, not Trusted by the Red Hand | 🔒 not Ex-Con/Raider unless Champion with Bastion | Assault a Red Hand camp | Big Fame (big Red Hand Infamy) |
| 16 | Hunt the Deserter | Trusted | | Track down a deserter who stole supplies. Bring him back or kill him | Fame; capture = more Fame |
| 17 | Requisition ✅ | Trusted | 🔒 not Elder | "Collect" supplies from a Haven safehouse | Fame (Haven Infamy); a nasty choice. Taken to Bastion, the safehouse loses 4 food |
| 18 | Secure the Shipment | Champion | | Escort a Halcyon sample shipment and deliver it to Bastion | **Choice:** Bastion Fame (Halcyon Infamy) or let it reach the lab |
| 19 | Blow the Bridge | Champion | ★ Mechanic/Soldier: needs fewer charges | Plant charges to stop a horde migrating toward a safe town | Fame; the horde is diverted |
| 20 | Fire Mission | Champion | | Mark three hordes with smoke for artillery | Fame; unlocks the Fire Mission perk |

**Amends with Bastion:** *Burn the Bodies* and *Recon* can be done while Watched.
Surrendering at a checkpoint (dropping your weapons) also removes some Infamy.

---

## The Red Hand (Bandits): 20 contracts

The Red Hand doesn't care who you were, but it does care who you've crossed.
Many contracts open up *because* Bastion or the Haven turned on you.

| # | Contract | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|
| 1 | Dead Drop ✅ | Any | ★ Ex-Con: 25% better pay | Sell infected samples for loot | Red Hand Fame (Halcyon Infamy, the cure slows) |
| 2 | Fence the Goods ✅ | Any | ★ Ex-Con | Deliver loot worth a set value to a fence | Fame, Favor |
| 3 | Car Theft | Any | ★ Mechanic: hotwires faster | Deliver a specific vehicle to the chop shop | Fame |
| 4 | Pharmacy Heist ✅ | Any | | Clear a pharmacy before the Haven gets there | Fame (Haven Infamy if Haven AI is present) |
| 5 | Fight Pit ✅ | Known | | Survive waves of infected in a ring while the Red Hand bets | Fame; bets pay Favor |
| 6 | Toll Road ✅ | Known | | Set up a roadblock and hold it for 15 min | Fame (Haven Infamy for each traveller you rob) |
| 7 | Smuggler's Run ✅ | Known | ★ Hunter: quiet route | Carry a package across the map. Everyone else is told a courier is out | Big Favor |
| 8 | Raid the Cache ✅ | Known, Hunted or Watched by the Haven | | Beat the Haven to a supply cache | Fame (Haven Infamy) |
| 9 | Loot the Armory | Known | ★ Police: knows the station layout | Break into a police station armory | Fame, weapons |
| 10 | Protection Racket | Trusted | 🔒 not Elder | "Collect" from a Haven safehouse. Pay or fight | Fame (Haven Infamy) |
| 11 | Hijack the Convoy | Trusted, not Trusted by Bastion | | Ambush a Bastion convoy | Big Fame (big Bastion Infamy) |
| 12 | Zombie Bait ✅ | Trusted | ★ Radio Op: uses a radio to lure | Lure a horde into a Bastion checkpoint | Fame (Bastion Infamy) |
| 13 | Bust Out | Trusted | ★ Ex-Con | Free Red Hand prisoners from a Bastion stockade | Fame (Bastion Infamy) |
| 14 | Arson ✅ | Trusted | | Burn a Bastion fuel depot | Fame (Bastion Infamy); Bastion convoys are fewer for a while |
| 15 | Sabotage the Relay ✅ | Trusted, not Trusted by the Haven | | Destroy a Haven radio relay | Fame (Haven Infamy) |
| 16 | Steal the Antidotes ✅ | Trusted, cure 50%+ | ★ Lab Tech: knows where they're stored | Steal an antidote batch from the lab | Big Fame; Red Hand shop gets antidotes (big Halcyon Infamy) |
| 17 | Double Agent | Trusted, Known by Bastion | ★ Police: Bastion believes you | Feed fake intel to Bastion | Fame with both. If caught, big Bastion Infamy |
| 18 | Collect the Bounty | Champion | | Kill a player or AI with a bounty on them | Fame, Favor |
| 19 | Kill the Officer | Champion | | Assassinate a Bastion commander | Huge Fame (Kill on Sight with Bastion) |
| 20 | Take the Lab | Champion, final stand active | | Seize the lab during the final stand so the cure goes to the Red Hand | Season ending: the Red Hand wins the cure |

**Amends with the Red Hand:** pay a Favor "tax" to the fence, or do a *Fence the Goods* run.

---

## Halcyon (Institute): 20 contracts

| # | Contract | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|
| 1 | Alpha Sighting ✅ | Any | | Kill a mutated special and bring its samples | Halcyon Fame, samples |
| 2 | Field Samples ✅ | Any | ★ Lab Tech/Nurse: faster collection | Take tissue samples from 5 regular infected corpses | Fame, research |
| 3 | Water Survey ✅ | Any | ★ Lab Tech | Take water samples from 3 wells in different towns | Fame; reveals where infection spreads next |
| 4 | Power the Lab | Any | ★ Mechanic | Bring fuel and a generator part to the lab | Fame; the lab makes antidotes faster |
| 5 | Lost Colleague ✅ | Known | | Find a missing researcher. They may already have turned | Fame, their data = research |
| 6 | Research Data ✅ | Known | | Recover a hard drive from an overrun hospital | **Choice:** Halcyon research, or sell it to Bastion (Halcyon Infamy) |
| 7 | Install Sensors | Known | ★ Radio Op/Lab Tech | Place 3 sensors in infested towns | Fame; specials show up more there (more samples) |
| 8 | Blood Drive | Known | 🔒 not infected | Donate blood at the lab (costs health) | Fame, small research |
| 9 | Cold Chain | Known, cure 50%+ | ★ Nurse | Get an antidote batch to a Haven clinic before it spoils | Fame with Halcyon and the Haven |
| 10 | Escort the Scientist | Trusted | | Escort a scientist to take readings at 3 sites | Fame, research |
| 11 | Specimen from the Nest | Trusted | | Get a sample from inside a hive without destroying it | Big research |
| 12 | Hunt the Screamer ✅ | Trusted | ★ Hunter | Kill a Screamer before it alerts the town | Fame, rare sample |
| 13 | Quarantine Breach ✅ | Trusted | | Infection has started inside a safe town. Contain it within the time limit | Fame (Haven Fame too) |
| 14 | Decontamination ✅ | Trusted | ★ Nurse | Burn infected corpses around the lab | Fame; fewer zombies at the lab |
| 15 | Lab Defence ✅ | Trusted | | Repel a raid on the lab | Fame (Red Hand Infamy if raiders) |
| 16 | Medical Drone | Trusted | ★ Mechanic/Radio Op | Recover a crashed prototype drone | Fame; unlocks the Sample Scanner perk |
| 17 | Patient Zero | Champion | ★ Lab Tech: the clues are clearer | Follow clues through 4 towns to the outbreak's origin | Huge research |
| 18 | Live Capture | Champion | | Lure a special into a cage trap alive | Huge research |
| 19 | Serum Trial | Champion | 🔒 infected only | Take an experimental serum while bitten. It cures you or speeds up the turn | Huge Fame; research either way |
| 20 | Mobile Lab | Champion | | Escort the lab truck to a safer town; the lab moves | Fame; the lab relocates (the final stand happens there) |

**Amends with Halcyon:** *Blood Drive* and *Field Samples* can be done while Watched.

---

## World events: 28 (anyone)

| # | Event | What happens | Why it's interesting |
|---|---|---|---|
| 1 | Crashed Helicopter ✅ | The wreck of one of Bastion's lost helicopters, its military cargo still aboard | Infected nest around it; a horde comes when someone arrives |
| 2 | Horde Migration ✅ | A huge horde crosses the region on a road | Get out of the way, or take advantage of it |
| 3 | Night of the Dead ✅ | One night with double density and runners | High risk, specials everywhere |
| 4 | Fog Bank | A region drops to low visibility for 20 min | Hiding is easy. So are ambushes. |
| 5 | Distress Call | A survivor asks for help on the radio | Could be real, could be a Red Hand trap (see 19) |
| 6 | Abandoned Convoy | Military trucks left on a highway | Loot, but it may be booby-trapped or watched |
| 7 | Power Surge | A town's power comes back for 10 min | Alarms go off (heat); electronics loot |
| 8 | Siren Test ✅ | Air-raid sirens pull every infected into one town | The towns around it are briefly safe to loot |
| 9 | Wandering Trader ✅ | A neutral merchant parks somewhere for 20 min | Rare items; protect him or rob him |
| 10 | Gas Station Fire | A station explodes and burns | Infection drops; loot is destroyed |
| 11 | Prison Break | Escaped convicts roam the area | They're hostile; the Red Hand recruits them |
| 12 | Bastion Shelling ✅ | Bastion's own mortars shell an infested town after a 5-minute warning | Get out, or loot fast before it hits. Infection there drops sharply |
| 13 | Bell Tower | Someone rings church bells nonstop | A horde converges; who's ringing them? |
| 14 | The Sniper | A crazed survivor shoots from a tower | Take him down; his stash is up there |
| 15 | Stash Note ✅ | A note with coordinates turns up on a body | Treasure hunt |
| 16 | Mass Grave ✅ | A pit full of infected bodies | Lots of samples, and a horde underneath |
| 17 | Lost Child | A child wanders alone | Escort to the Haven (Fame) or just leave |
| 18 | Burning Town | A town burns for 15 min | Infection there drops sharply |
| 19 | Fake Distress Call | It's a Red Hand ambush | Radio Operators are warned |
| 20 | Storm ✅ | Rain and wind; noise carries less | Heat decays faster; good time to move |
| 21 | Last Broadcast | A TV station is still running | Reach it and broadcast. Everyone gets a small Fame boost with the faction you name |
| 22 | Rival Survivors | An unaligned AI group | They trade or fight depending on your overall reputation |
| 23 | Locked Bunker | A sealed bunker, code split over 3 notes | Teamwork or a race between strangers |
| 24 | Turned Player ✅ | A player who turned roams with their gear | Their loot is up for grabs; their friends may disagree |
| 25 | Tunnel Rats | Infected flood out of tunnels or a subway | A town's infection spikes |
| 26 | Military Remnant Camp | An abandoned Bastion camp with gear | Taking it gives Bastion Infamy if they're still active |
| 27 | Wedding Party | A horde in formal wear by a church | Dark humour, good jewellery loot for trading |
| 28 | Collapse Shelter | A town's shelter is overrun and survivors scatter | Save them before they turn (Haven Fame) |

## Puzzle missions: 17 contracts

Contracts you solve by reading, deducing and sneaking rather than shooting. Most low-tier
ones can be done unarmed. Each is a normal contract from one faction (requirements, ★,
Fame and Favor as above), works on any map (it's built from settlements and real landmark
buildings it finds by prefab name, with a fallback prop), and deletes everything it
spawned when it ends. Code: `Scripts/Game/ReforgerZ/Puzzles/`.

| # | Puzzle | From | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|---|
| 1 | The Numbers Station ✅ | Haven | Any | ★ Radio Op: hears the broadcast without the note or the mast | A radio log gives a frequency. Of three radio masts, one carries it: a voice reading a map grid, every third number a decoy | Fame; stash at the grid |
| 2 | Two Bearings ✅ | Bastion | Any | ★ Hunter/Soldier: more Fame | Two helicopter wreck pieces have bearings scratched on them. Draw both lines; reach where they cross | Fame; the pilot's kit |
| 3 | Paces from the Tower ✅ | Haven | Any | ★ Hunter: the paces also in metres | A dead man's note: "From the church, face the water tower, turn right, 400 paces." Built from two real landmarks in the town | Fame; stash |
| 4 | Hot and Cold ✅ | Haven | Any | ★ Radio Op: numeric signal strength | With a radio, a beacon in the fields sounds faint, clear, loud. Find it within 10 m | Fame; stash |
| 5 | The Rat ✅ | Red Hand | Known | ★ Police/Ex-Con: more Fame | Three suspects, exactly one lying (generated, always one solution). Point at the rat at the boss | Fame. Wrong: the rat escapes, the mission fails and you get Red Hand Infamy |
| 6 | Patient Zero's Ledger ✅ | Halcyon | Known, lab exists | ★ Lab Tech/Doctor: the method is spelled out | Ledger (ages, admissions) plus hospital bracelets on three bodies (days of death). Adults die 4 days after the bite, children 2. Name who was first, at the lab | Fame, research. One wrong answer locks it for 5 min; two fail |
| 7 | The Poisoned Well ✅ | Halcyon | Any | ★ Lab Tech/Nurse: can test the water | Three wells in three villages. A clinic note and a farmer's note each rule one out; a dead courier lies by the poisoned one. Seal it | Fame, small research, that village's infection -10. A wrong seal raises that village's infection; two fail |
| 8 | The Smuggler's Ledger ✅ | Red Hand | Known | ★ Ex-Con: more Fame | One ledger line is Caesar-shifted. The key is the number on a memorial plaque by the church. The line names a landmark (or an edge) of another town | Fame; stash |
| 9 | The Clockwork Horde ✅ | Haven | Any | ★ Hunter: harder to notice | A stuck siren wails every 2 min at alternate ends of a town; a horde walks between them through the square. Take the strongbox there without being noticed | Fame; the strongbox. Noticed: the horde turns on you and you must wait a minute |
| 10 | Time-Lock Safe ✅ | Red Hand | Known | ★ Ex-Con: window twice as long | A bank safe opens only at 03:00 game time, for 20 min. Opening it is loud (heat and a horde) | Fame; the safe's contents. Offered only when 03:00 is at most 3 real hours away |
| 11 | The Bell Order ✅ | Haven | Known | ★ Elder: more Fame | Ring three village church bells in the order on a hymn sheet at the safehouse, at most 15 min apart. Doable solo, faster in a group | Fame; heat and infection drop at the safehouse. Wrong bell: a horde at the ringer, start over |
| 12 | Lights in the Window ✅ | Bastion | Known | ★ Radio Op: also gets the Morse on the radio | At night a lamp in a distant building blinks a code word in Morse. The signal card at the scout post has the alphabet and four code words with meeting places. Reach the right one | Fame; a pack at the meeting place. Offered only at or near nightfall |
| 13 | Junction Boxes ✅ | Bastion | Any | ★ Mechanic: the note names one more switch | Four fuse boxes, two switches each; every switch flips its own lamp and one or two others (lights-out, shortest fix 3-6 presses). Each box shows its lamps and wiring. Get all eight green | Fame; the substation's contents |
| 14 | The Weighing Vault ✅ | Haven | Known | ★ Mechanic: reads the needle to 0.1 kg | Put items in a scale crate until they match the weight in a riddle (±0.2 kg). Feedback is "too heavy / too light" | Fame; the vault's contents. *Simplified option:* if the storage weight API doesn't work, the crate counts items instead (game mode setting) |
| 15 | The Last Experiment ✅ | Halcyon | Trusted | ★ Lab Tech: the screen gives the first letter | Each missing researcher's belongings (4-5 towns) carry one letter of the terminal password. Unscramble it and type it on the keypad (phone letters: 2 = ABC...). Without a keypad prefab, pick it from four words | Big research. Three wrong tries wipe it |
| 16 | The Silent Village ✅ | Haven | Trusted | ★ Elder: more Fame | A village with no infected at all. Notes, a locked cellar and a list of the missing point under the church. Then choose: **burn it** (infection drops around, a fight), **seal it** (half Fame, nothing happens), or **let Halcyon study it** (Halcyon pays, big research, half the time specials and a horde come up) | Depends on the choice |
| 17 | The Witness ✅ | Haven | Known | ★ Police: he talks by day too | A Haven courier was killed on a road. A wanderer saw it but only talks at night, at that spot. His story points to a camp (a Red Hand group if that prefab is set). At the camp, **report it to the Haven** (Fame; Red Hand Infamy if their group is there) or **sell it to the Red Hand** (Red Hand Fame, Haven Infamy) | Depends on the choice. Offered only at or near nightfall |

How they work in code:

- **Clues** are notes, bodies, plaques, wreck pieces or people with `RZ_ClueComponent`. Reading
  one sends its text to that player only. The text lives on the server.
- **Answers** go through actions on puzzle objects: a keypad (`RZ_KeypadAction`), choices
  (`RZ_PuzzleChoiceAction`: accuse, decide) and uses (`RZ_PuzzleUseAction`: ring, seal, flip,
  tune, take). One generic object prefab serves every puzzle; the mission sets the action labels.
- **Wrong answers** cost something: heat, a horde, a lock-out timer, infection, Infamy or failure.
- **Being noticed** (Clockwork Horde): heat where you stand above a threshold, or a zombie within
  12 m of you for more than 6 s.
- **Noisemakers** (bottle, firecracker, alarm clock) add heat where they land or when their timer
  runs out. They're general items, not tied to one mission.
- **Night**: missions that need the dark only appear at or near nightfall and last until dawn
  (at most 3 hours). Time is never skipped; the clock speed is measured on the server.
- Rewards are what you find at the end (a stash, a safe, a vault) plus Fame, Favor or research.
  Clue and object positions are given as town names and grid references.
- Missions turn themselves off when a prefab they need is missing: see
  [WORKBENCH_SETUP.md](WORKBENCH_SETUP.md), "Puzzle prefabs".

## Unarmed missions: 10 contracts

Low-level work for new characters, unarmed backgrounds (Nurse, Lab Tech, Unemployed) or anyone
who'd rather not shoot ([WORLD.md](WORLD.md), section 2). All are open to anyone not Hunted by
the giver, pay small Fame, and are built on **noise** and **being noticed**:

- **Noise that leads:** any loud noise (a thrown bottle, a firecracker, an alarm clock, a burst
  of fire: 5+ heat at once) within range of infected a mission controls makes them walk to it.
  They stay about 90 s, then drift back.
- **Being noticed:** the same check as the puzzles: heat where you stand over a threshold, or one
  of them within 10–12 m of you for 6–8 s. Missions that are about making noise only use the
  second part.

Code: `Scripts/Game/ReforgerZ/Missions/Unarmed/`.

| # | Mission | From | Bg | What you do | Result |
|---|---|---|---|---|---|
| 1 | Pharmacy Lure ✅ | Haven | ★ Nurse/Doctor: more Fame | Four infected stand in a pharmacy (a building named like one, else near the town centre). Make noise away from the door to draw them out, take the medicine, bring it to the nearest safehouse | Fame, Favor. Noticed when you take it: they come for you and the pay is cut to 60% |
| 2 | Silent Courier ✅ | Haven (a Courier wanderer's job, named if one is alive) | ★ Hunter: heat limit 45 | Carry a letter from one town through an infested town to people hiding on the far side. In the town, heat where you stand must stay under 35 | Fame, Favor. Too loud: fails. Went round the town: half pay |
| 3 | Lead Them Away ✅ | Haven | ★ Radio Op: more Fame | A horde spawns 750–950 m from a safehouse and drifts toward it. Each loud noise within 200 m of it turns it there; lead it to a quarry pit 550–750 m from the safehouse. Noticed near it, it turns on you | Fame, Favor. It reaches the safehouse: fails, two people there die. Killed instead: half pay |
| 4 | Rooftop Rescue ✅ | Haven | ★ Mechanic: 15 m is clear enough | A survivor is stuck up a building (a school, station, bank or church). Every 90 s three infected wander in. Talk them down the ladder when none is within 25 m and you aren't noticed | Fame, Favor; they join the nearest safehouse. Too early: "not now" and noise |
| 5 | Trail of Clues ✅ | Haven (for a wanderer) | ★ Police: grid references | A note in one town leads to a bag in a second, which leads to a third. Each clue appears only after the one before was read. Half the time the missing person is alive | Alive: bring them home (they join a safehouse). Dead: their last letter, 70% pay |
| 6 | Water Run ✅ | Haven | | Carry water from a well or water tower to a safehouse 0.8–2.5 km away. The fast road goes through an infested town | Fame, Favor, the safehouse +2 food. Through the town unnoticed: ×1.33 pay; noticed there: ×0.7 (it spilled) |
| 7 | Barter Chain ✅ | Haven, through three wanderers | | The first wants something the second has; the second wants something the third has; the third wants food. Talk to them in the right order (share a food item with the third) | Small Fame, Favor; all three remember you helped. One of them dies: fails |
| 8 | Distraction ✅ | Any faction (picked each time) | | Infected guard a cellar. Someone makes noise at a marked spot across the street; someone else searches the cellar. The search takes 3 minutes and only runs while there's been noise there in the last 15 s. Noticed while searching: it starts again | Fame, Favor, the cellar (reward crate). Doable alone with alarm clocks |
| 9 | Medicine Before Dark ✅ | Haven (for a Doctor wanderer, named if one is alive) | ★ Nurse/Doctor: more Fame | A bitten survivor waits in one town; medicine is in a pharmacy in another. Bring it before nightfall (20:30). Offered only 15–90 real minutes before dark | Fame, Favor; they join a safehouse. Dark first: fails, and a special rises there if anyone's near |
| 10 | Quiet Count ✅ | Halcyon | ★ Lab Tech: more Fame | Two to four specials (and a few others) in a town. Count them from outside and write the number in the notebook at an observation post (one or none / two / three / four or more). The answer is how many are alive when you write it | Fame, Favor, +1 research. Within 50 m of one: half pay and they come for you. Wrong count: fails |

Simplifications in code:

- **Car Keys** (WORLD.md #8) isn't built: there's no fixable car in code yet.
- **The Safe Code, Breaker Sequence, Scout the Town and Night Lantern** (WORLD.md #3, #4, #9, #10) are
  covered by existing missions: The Last Experiment's keypad and the Time-Lock Safe, Junction
  Boxes, Recon, and Lights in the Window and Signal Fires.
- **Rooftop Rescue:** the survivor stands at the foot of the building (the ladder), not on the
  roof: the code can't tell where a roof is.
- **Water Run:** "heavy" is the mission item prefab's weight. The fast road counts as taken if the
  carrier passes through the infested town.
- **Barter Chain:** only the food changes hands; the other items are words.
- **Lead Them Away:** the quarry is a grid reference, not a detected quarry.
- Needs: Pharmacy Lure, Silent Courier, Water Run and Medicine Before Dark the mission item and
  the Haven drop point; Rooftop Rescue and Medicine Before Dark the puzzle civilian; Trail of Clues
  the note and the civilian; Distraction and Quiet Count the generic puzzle object; Pharmacy Lure,
  Lead Them Away and Distraction the zombie **Move waypoint**; Lead Them Away and Water Run a
  safehouse; Barter Chain three living wanderers. Each is off while what it needs is missing.

## Perimeter missions: 3 contracts

The only links to the outside, at the edge of the map (the Perimeter, see
[LORE.md](LORE.md)). They're rare and dangerous on purpose, and nothing is built for them:
no fences, no towers. Each is a normal contract (requirements, Fame and Favor as above),
works on any map (positions come from the edge rectangle, found automatically), and deletes
what it spawned when it ends. They never appear while the Perimeter is off. Code:
`Scripts/Game/ReforgerZ/Missions/Perimeter/`.

| # | Mission | From | Needs | What you do | Result |
|---|---|---|---|---|---|
| 1 | The Blind Spot ✅ | Red Hand | Known, night | The drones run a rotation. Ten minutes after the offer, a 250 m stretch of the edge goes uncovered for ten minutes: no drone, no shots there. A smuggler waits just inside the line (in the kill zone). Reach him, take the package, and hand it in at the Red Hand drop 500-700 m inland before the drones come back | Fame, Favor. The window closing fails it, and anyone still in the kill zone gets the warning shot, then the fatal one |
| 2 | The Crossing ✅ | Haven | Any, night | Someone outside left a sealed medicine package at the edge, inside the warning zone: at the waterline if the edge there is coast, else at the treeline. Take it out of the warning zone before the drone locks on, and deliver it to the Haven drop in the safest town | Fame, Favor. Lock-on: one shot destroys the package (not you) and the mission fails |
| 3 | Shot at the Line ✅ | Any faction (picked each time) | Any | A body lies in the warning zone, 20-50 m short of the kill zone. Search it without going further: the papers name where the dead person left what they couldn't carry. Go there | Fame, Favor; the stash (reward crate) |

Simplifications in code:

- **Night** is the game clock's night (20:30 to 05:00). The Blind Spot is only offered when the
  whole window fits before dawn.
- **The Blind Spot's window** counts in real minutes from the offer, not game time.
- **"Treeline"** is just any dry edge: trees aren't detected. The waterline is the first dry
  ground walking in from the edge after water.
- **Lock-on** counts the carrier's total time in the warning zone with the package (45 s),
  not one continuous stay. Dropping it stops the count.
- **The smuggler** is scenery: reaching him hands over the package. The first player to reach
  him gets it, hired or not.

## Haven house contracts: 3 contracts, plus the Courier's job

Morally grey work around the Haven's safehouses and their rules ([WORLD.md](WORLD.md),
"House rules and the dark side"). Nobody in them is a villain. Each is a normal contract
(requirements, ★, Fame and Favor as above) and works on any map: they need at least one
safehouse. Walk Him Out and The Ledger need the puzzle civilian (or the generic puzzle object);
A Quiet Arrangement and the Courier's job need the mission item and a drop point. Code:
`Scripts/Game/ReforgerZ/Missions/Haven/RZ_HavenHouseMissions.c`.

| # | Contract | From | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|---|
| 1 | Walk Him Out ✅ | Haven | Known, dusk | ★ Nurse: can read how far the bite has gone; ★ Nurse/Elder: more Fame | A bitten Haven member told the council himself. At dusk he waits at the edge of his safehouse's town (he's no longer counted in its population). Reach him and choose: **end it here** or **let him walk on** | End it: Fame. Walk on: 60% Fame, and the town's infection +5. Nobody comes within 40 min: he turns there |
| 2 | The Ledger ✅ | Haven | Known | ★ Elder: more Fame | A safehouse council wants a member who's been taking extra from the stores "reminded". They're in another town, with a reason. Choose: **threaten** them, **pay their debt** (20 of your Haven Favor), or **tell them to disappear** | Threaten: Fame; they remember it (wronged). Pay: Fame; they remember (helped). Warn off: no pay, +3 Haven Infamy; they remember you saved them. Uses a real Haven wanderer (Trader, Scavenger or Courier) when one is free |
| 3 | A Quiet Arrangement ✅ | Red Hand | Known | ★ Ex-Con: lookouts spot you at 25 m instead of 35 m | A safehouse keeper pays the Red Hand to be left alone and can't be seen carrying it. Take the envelope from the safehouse to a Red Hand drop 0.8–2.5 km away without being seen by the two Haven lookouts on the way or a Haven wanderer | Small Red Hand Fame, Favor. Seen: +12 Haven Infamy on delivery. Never delivered: the Red Hand collects in person (the safehouse loses 2 people) |
| – | Letters (the Courier's job) ✅ | Haven, via the Courier wanderer | Any, one at a time | | Ask a Courier for work: carry a bundle of letters to a safehouse (or a quiet town) 0.8–4 km away. No fighting needed | Small Haven Fame, Favor. Only the player who asked (and anyone with them at the drop) is paid |

Simplifications in code:

- **Walk Him Out**: he waits at the edge rather than walking there with you (a prop can't
  walk), and "dusk" means the offer only appears between 16:30 and 21:00 game time (any time
  if the clock can't be read).
- **The Ledger**: the member is a separate figure; when a real wanderer plays the part, that
  wanderer is kept off the roads until the contract ends, then walks on from there.
- **A Quiet Arrangement**: the lookouts are mission props, not wanderers. Being seen right
  at the safehouse doesn't count (the keeper handed it over in private).

The **bitten wanderer** (a wanderer bitten on the road, saved by an antidote) is part of the
wanderer simulation, not a mission: see [WORLD.md](WORLD.md), "Wanderers".

## Leader missions: 32 personal jobs, plus Rein In

Work that Independent and Rogue leaders give **in person**, off their stronghold's books
(who they are and how loyalty works: [REPUTATION.md](REPUTATION.md), "Faction leaders").
Code: `Scripts/Game/ReforgerZ/Missions/Leaders/`.

- **Who gets offered what.** Only a leader of the matching faction and loyalty offers a job,
  and only in person: ask them for work, or walk up to one who has a job for you. They're never
  on the radio. The leader's attitude decides who may take it (not your tier), so a Rogue
  leader can hire someone their stronghold hunts.
- **Traits pick the job.** Each job has a tone; Ruthless leaders lean to dark ones, Idealists
  and Merciful ones to hopeful ones, Pragmatists (and a little, the Greedy) to cross-faction
  ones, Veterans to hard ones, Talkers to talking ones.
- **Pay.** Independent jobs pay a little faction Fame and Favor; Rogue jobs pay Favor only (the
  leader pays from their own stores), plus any Fame with the other faction involved. The
  leader's traits scale it, and they remember you helped.
- **Rogue jobs can be discovered** (+10 Infamy with the faction; the leader is removed or pulled
  back into line), and most rogue deliveries have a second drop at the stronghold: hand it in
  there to **turn the leader in** (Fame and Favor from the stronghold; the leader remembers).
- Item jobs need the mission item and the drop points they name; people need the puzzle civilian.

| # | Job | From | What you do | Result |
|---|---|---|---|---|
| 1 | Our Own First ✅ | Haven, Independent | Supplies meant for another safehouse wait in a town. Bring them here, or where they were meant to go | Here: this safehouse +4 food. There: that one +4 food, half pay, no credit with the council head |
| 2 | Bring Them Home ✅ | Haven, Independent | Find a local's family in another town. One of them is bitten | All three: +3 people, the town's infection +5. The two who are well: +2 people, less pay. Leave them: fails |
| 3 | Over the Fence ✅ | Haven, Independent | Take bread to a nearby Bastion post for medicine | Bastion Fame +3; the safehouse town's infection −3 |
| 4 | The Old Clinic ✅ | Haven, Independent | Hold a clinic in the next town for 3 min (2 waves) so it can be an annex | That town's infection −10; +1 person at the safehouse |
| 5 | Ration Run ✅ | Haven, Rogue | Three days of the stores go to the Red Hand for antibiotics | Stores −3 food (back if it fails or you turn it in); Red Hand Fame +3; the town's infection −3 |
| 6 | Names for Bastion ✅ | Haven, Rogue | Deliver a "troublemakers" list to Bastion for protection | Bastion Fame +3; online players the Haven exiled take +5 Bastion Infamy |
| 7 | Shut the Door ✅ | Haven, Rogue | A family at the edge of town asks to come in. Turn them away | Turned away: paid, they take to the roads. Let in: +3 people, no pay, the council head remembers |
| 8 | Hidden Stores ✅ | Haven, Rogue | Move the council head's hidden stash before the count (20 min) | Paid. Fail it once you've started: the count finds it (the leader is discovered). Turn it in: +5 food |
| 9 | Hearts and Minds ✅ | Bastion, Independent | Rations to a Haven safehouse, against orders | That safehouse +4 food; Haven Fame +4 |
| 10 | The Real Orders ✅ | Bastion, Independent | Recover command's last orders from a dead courier (guarded). Picking them up shows what they say | To the commander: if they reveal the garrison was written off (60%), the commander drifts away from the stronghold, else toward it. To the stronghold: +10 Fame, they're burned, the commander remembers |
| 11 | Training Day ✅ | Bastion, Independent | Stand with raw recruits at a town's edge for 4 min (3 waves) | Paid. With the survivor group prefab, the recruits are real and must live |
| 12 | Local Law ✅ | Bastion, Independent | Two families fight over a well. Hear both sides, then decide. One shot near them and it's over | Split the days: full pay. Give it to one: 80%, the other family leaves |
| 13 | Rifles for Sale ✅ | Bastion, Rogue | Army rifles to a Red Hand buyer | Red Hand Fame +4 |
| 14 | Off-Book Requisition ✅ | Bastion, Rogue | Collect a "contribution" from a village (a safehouse village if one is near) | All: +6 Haven Infamy, the safehouse −4 food. Half and lie: 60% pay, half the harm. Pay it yourself: 15 Bastion Favor, nobody hurt |
| 15 | Burn the Ledger ✅ | Bastion, Rogue | Burn the supply ledger behind the post before the audit (15 min) | Burn it: paid, heat. Keep it for the auditor: the commander is turned in |
| – | Desertion Route | Bastion, Rogue | Get deserters to a Haven safehouse or the Blind Spot | *Not built:* nothing in the framework can walk a group of people to a place yet |
| 16 | Our Crossroads ✅ | Red Hand, Independent | Hold the crew's crossroads outside town for 5 min (3 waves) | Paid |
| 17 | Honour Among Thieves ✅ | Red Hand, Independent | Recover what a crew member stole | Take it back: paid, loud. Talk it back: 60% (always for a Talker), else he runs. Let him keep it: fails |
| 18 | House Rules ✅ | Red Hand, Independent | The crew's own fight pit behind the garages (4 waves) | Mostly Favor (the bets) |
| 19 | Debt Collection ✅ | Red Hand, Independent | Collect from a debtor (a real wanderer when one is free): talk, threaten or pay | Talk: 60% pay. Threaten: full pay, +3 Haven Infamy if a wanderer. Pay it yourself: 15 Red Hand Favor |
| 20 | The Soft Touch ✅ | Red Hand, Rogue | Secret medicine to a Haven safehouse | Haven Fame +5; that town's infection −5 |
| 21 | Coup Talk ✅ | Red Hand, Rogue | Carry a word to two other crews (other Red Hand posts, else camps outside two towns), 20 s at each | Never fully works: laughed off (the crew boss is pulled back toward the boss) or a purge (the crew boss is discovered; carriers take Red Hand Infamy, and if the boss is removed nobody pays) |
| 22 | Sell Out a Rival Crew ✅ | Red Hand, Rogue | A map of another crew's camp, to Bastion | Bastion Fame +4; another Red Hand crew boss, if there is one, dies in the raid (replaced later) |
| 23 | Skim the Toll ✅ | Red Hand, Rogue | Take the boss's toll on the road out for 5 min | Paid; +3 Haven Infamy (the travellers) |
| 24 | Field Station ✅ | Halcyon, Independent | Hold a house in a quiet village for 3 min (1 wave) while a station goes up | Research +1 |
| 25 | Second Opinion ✅ | Halcyon, Independent | Notes to a Haven medic (a safehouse with one if in reach) | Research +1; Haven Fame +3 |
| 26 | Patients Not Samples ✅ | Halcyon, Independent | A dying man. Sit with him, or take the sample the protocol wants | Sit: Haven Fame +3. Sample: research +1, half pay, the field lead remembers |
| 27 | The Notebook ✅ | Halcyon, Independent | A dead researcher's notebook (guarded): half research, half what she thought of the director | To the field lead: research +1. To the director: research +2, +10 Fame, the field lead remembers |
| 28 | Unsanctioned Trial ✅ | Halcyon, Rogue | A willing, dying volunteer and an untested serum | Half the time he lives (research +2), half the time he turns (research +1, a special). Refuse: fails |
| 29 | Leak the Data ✅ | Halcyon, Rogue | Research data to a Haven safehouse for nothing | Haven Fame +5; that town's infection −3 |
| – | Private Collection | Halcyon, Rogue | Capture a special alive | *Not built:* needs a way to trap and hold a special |
| 30 | Sell the Formula ✅ | Halcyon, Rogue | The antidote formula to the Red Hand | Red Hand Fame +5 |
| – | Rein In ✅ | The stronghold (radio contract, Known) | A leader turned Rogue. Go to them and read the terms, or take their counter-offer (25 Favor ×traits) | Terms: their loyalty +25. Counter-offer: no stronghold pay, the leader remembers, 35% the stronghold finds out (+10 Infamy) |

Simplifications in code:

- **"In person"** means at the leader's seat (within 80 m for listing, 15 m for the nudge).
- **Bring Them Home, Shut the Door**: the families are one stand-in figure; the people join or
  leave as numbers.
- **Over the Fence, Hearts and Minds, the Red Hand and Bastion buyers**: the other side is a
  drop point at their nearest seat (within 5 km), else outside a town 0.8–3 km away.
- **The Real Orders**: the orders are read out to whoever first picks up the satchel.
- **Training Day**: without the survivor group prefab, the recruits are only in the brief.
- **Coup Talk**: "other crews" are other Red Hand seats (its outposts) when there are any;
  otherwise they're camps outside towns.
- **Rein In** is offered by radio a few minutes after a leader is Rogue (including one who starts
  the season Rogue), then at most once an hour while they stay Rogue. The rogue leader is shown by a stand-in while it runs.
- Rewards never include loot: Favor, Fame, research, food and population only.

## Character-type summary

Every background has contracts it shines at, and at least one it's shut out of.

| Background | Shines at | Shut out of |
|---|---|---|
| Nurse | Medicine Run, Bitten Friend, Cold Chain, Pilot rescue, Walk Him Out, Pharmacy Lure, Medicine Before Dark | — |
| Mechanic | Fuel, Clear the Road, Recover the Bus, Car Theft, Convoy repairs, Rooftop Rescue | — |
| Hunter | Harvest Guard, Rescue the Kids, Smuggler's Run, Hunt the Screamer, Silent Courier | — |
| Police | Checkpoint Duty, Curfew Patrol, Loot the Armory, Double Agent, Trail of Clues | The Informant |
| Ex-Con | Dead Drop, Fence, Bust Out, A Quiet Arrangement | Raid the Red Hand (until Champion with Bastion) |
| Lab Tech | Field Samples, Water Survey, Patient Zero, Steal the Antidotes, Quiet Count | — |
| Radio Op | Radio Relay, Recon, Restore Comms, Zombie Bait, Install Sensors, Lead Them Away | — |
| Unemployed | +50% reputation from everything | — |
| Elder (unlock) | Haven contracts, Walk Him Out, The Ledger | Requisition, Protection Racket |
