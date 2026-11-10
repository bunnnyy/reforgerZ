# Missions

There are two kinds of missions:

- **World events** (30) are announced to everyone, and anyone can take part.
  They are what makes strangers meet: an airdrop, a horde, a downed helicopter.
- **Faction contracts** (20 per faction) are offered only to players who
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

---

## The Haven (Survivors): 20 contracts

| # | Contract | Needs | Bg | What you do | Result |
|---|---|---|---|---|---|
| 1 | Supply Cache ✅ | Any | | Hold a cache in a town for 60 s | Haven Fame |
| 2 | Survivors Trapped ✅ | Any | ★ Nurse: survivors leave healthier, bonus Fame | Reach a barricaded family and survive 3 waves | Haven Fame |
| 3 | Radio Relay ✅ | Any | ★ Radio Op: half the hold time | Keep a relay on high ground running for 3 min | Haven Fame |
| 4 | Medicine Run ✅ | Any | ★ Nurse/Doctor: knows the pharmacy; it's pre-marked | Bring medical supplies from a pharmacy town to a safehouse | Haven Fame |
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
| 17 | Rescue the Kids ✅ | Trusted | ★ Hunter: quieter approach | Get children out of a school without making noise. Too loud and a horde comes | Big Fame |
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
| 2 | Evac Flight ✅ | Any, Collapse/Overrun | | Be at the LZ when the helicopter leaves | Fame, crates |
| 3 | Burn the Bodies ✅ | Any | ★ Nurse: faster | Burn 5 corpse piles in a town to slow infection | Fame, spread slows there |
| 4 | Recon ✅ | Any | ★ Hunter/Radio Op: reports count double | Visit 3 observation points and report horde sizes | Fame; the next horde is announced earlier |
| 5 | Checkpoint Duty ✅ | Known | ★ Police | Hold a checkpoint 10 min. Let survivors pass, stop armed bandits | Fame (Red Hand Infamy for each bandit you stop) |
| 6 | Missing Patrol ✅ | Known | | Find a squad's last position, recover dog tags, deal with the turned soldiers | Fame, military loot |
| 7 | Crash Site ✅ | Known | ★ Mechanic: recovers the black box faster | Secure a downed helicopter and its black box | **Choice:** Bastion Fame, or sell the box to the Red Hand |
| 8 | Restore Comms | Known | ★ Radio Op: needs no parts | Bring parts and repair a radio tower | Fame; Bastion intel on the radio |
| 9 | Curfew Patrol | Known, night only | ★ Police | Patrol a town at night and keep it clear | Fame |
| 10 | Weapons Cache | Trusted | | Retrieve weapons from an armory in an overrun base | Fame; unlocks the Bastion rifle shop |
| 11 | Convoy Escort | Trusted | ★ Mechanic: can fix a broken-down truck | Escort supply trucks between two towns | Big Fame (Red Hand Infamy if you kill raiders) |
| 12 | Rescue the Pilot | Trusted | ★ Nurse/Doctor | Find a downed pilot behind the lines and bring them out | Fame |
| 13 | Quarantine Line ✅ | Trusted | | Hold the edge of a town for 10 min while it's sealed off | Fame; that town stops spreading for a while |
| 14 | Clear the Airfield | Trusted | | Clear an airfield so evac flights can land there | Fame; evac flights appear more often |
| 15 | Raid the Red Hand ✅ | Trusted, not Trusted by the Red Hand | 🔒 not Ex-Con/Raider unless Champion with Bastion | Assault a Red Hand camp | Big Fame (big Red Hand Infamy) |
| 16 | Hunt the Deserter | Trusted | | Track down a deserter who stole supplies. Bring him back or kill him | Fame; capture = more Fame |
| 17 | Requisition ✅ | Trusted | 🔒 not Elder | "Collect" supplies from a Haven safehouse | Fame (Haven Infamy); a nasty choice |
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

## World events: 30 (anyone)

| # | Event | What happens | Why it's interesting |
|---|---|---|---|
| 1 | Airdrop ✅ | A supply crate lands somewhere; everyone sees where | Everyone races there; PvP magnet |
| 2 | Crashed Helicopter ✅ | A smoking wreck with military loot | The crash noise brings a horde |
| 3 | Horde Migration ✅ | A huge horde crosses the region on a road | Get out of the way, or take advantage of it |
| 4 | Night of the Dead ✅ | One night with double density and runners | High risk, specials everywhere |
| 5 | Fog Bank | A region drops to low visibility for 20 min | Hiding is easy. So are ambushes. |
| 6 | Distress Call | A survivor asks for help on the radio | Could be real, could be a Red Hand trap (see 21) |
| 7 | Abandoned Convoy | Military trucks left on a highway | Loot, but it may be booby-trapped or watched |
| 8 | Power Surge | A town's power comes back for 10 min | Alarms go off (heat); electronics loot |
| 9 | Siren Test ✅ | Air-raid sirens pull every infected into one town | The towns around it are briefly safe to loot |
| 10 | Wandering Trader ✅ | A neutral merchant parks somewhere for 20 min | Rare items; protect him or rob him |
| 11 | Gas Station Fire | A station explodes and burns | Infection drops; loot is destroyed |
| 12 | Prison Break | Escaped convicts roam the area | They're hostile; the Red Hand recruits them |
| 13 | Bombing Run ✅ | Bastion strikes an infested town in 5 min | Get out, or loot fast before it hits |
| 14 | Bell Tower | Someone rings church bells nonstop | A horde converges; who's ringing them? |
| 15 | The Sniper | A crazed survivor shoots from a tower | Take him down; his stash is up there |
| 16 | Stash Note ✅ | A note with coordinates turns up on a body | Treasure hunt |
| 17 | Mass Grave ✅ | A pit full of infected bodies | Lots of samples, and a horde underneath |
| 18 | Lost Child | A child wanders alone | Escort to the Haven (Fame) or just leave |
| 19 | Supply Plane Crash ✅ | Cargo is scattered over 1 km | Several small contested crates |
| 20 | Burning Town | A town burns for 15 min | Infection there drops sharply |
| 21 | Fake Distress Call | It's a Red Hand ambush | Radio Operators are warned |
| 22 | Storm ✅ | Rain and wind; noise carries less | Heat decays faster; good time to move |
| 23 | Last Broadcast | A TV station is still running | Reach it and broadcast. Everyone gets a small Fame boost with the faction you name |
| 24 | Rival Survivors | An unaligned AI group | They trade or fight depending on your overall reputation |
| 25 | Locked Bunker | A sealed bunker, code split over 3 notes | Teamwork or a race between strangers |
| 26 | Turned Player ✅ | A player who turned roams with their gear | Their loot is up for grabs; their friends may disagree |
| 27 | Tunnel Rats | Infected flood out of tunnels or a subway | A town's infection spikes |
| 28 | Military Remnant Camp | An abandoned Bastion camp with gear | Taking it gives Bastion Infamy if they're still active |
| 29 | Wedding Party | A horde in formal wear by a church | Dark humour, good jewellery loot for trading |
| 30 | Collapse Shelter | A town's shelter is overrun and survivors scatter | Save them before they turn (Haven Fame) |

## Character-type summary

Every background has contracts it shines at, and at least one it's shut out of.

| Background | Shines at | Shut out of |
|---|---|---|
| Nurse | Medicine Run, Bitten Friend, Cold Chain, Pilot rescue | — |
| Mechanic | Fuel, Clear the Road, Recover the Bus, Car Theft, Convoy repairs | — |
| Hunter | Harvest Guard, Rescue the Kids, Smuggler's Run, Hunt the Screamer | — |
| Police | Checkpoint Duty, Curfew Patrol, Loot the Armory, Double Agent | The Informant |
| Ex-Con | Dead Drop, Fence, Bust Out | Raid the Red Hand (until Champion with Bastion) |
| Lab Tech | Field Samples, Water Survey, Patient Zero, Steal the Antidotes | — |
| Radio Op | Radio Relay, Recon, Restore Comms, Zombie Bait, Install Sensors | — |
| Unemployed | +50% reputation from everything | — |
| Elder (unlock) | Haven contracts | Requisition, Protection Racket |
