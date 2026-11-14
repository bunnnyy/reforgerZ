# ReforgerZ

A zombie survival game mode for **Arma Reforger**, set in a sealed government
**Quarantine Zone**. Eleven days ago the region was closed off: drones over the line,
roads ending in concrete, snipers nobody sees. Nothing comes in and nobody gets out.
You're a civilian who was inside when the gates closed. Survive alone, team up with
strangers, or turn on them. The whole server shares one outbreak that slowly gets worse,
and one way to end it: **research the cure.**

## Features

- **Backgrounds** picked on our own spawn screen (Nurse, Mechanic, Hunter, Police Officer, Ex-Con,
  Lab Technician, Radio Operator, Unemployed), each with a real perk and flaw. Five more are
  unlocked through Legacy.
- **Openings:** every life starts somewhere random that suits who you were, coming out of a door
  for a reason (the last bottle empty, your group gone three days, the fever broken), and whoever's
  outside calls you over: someone who knew you before, a stranger, or a wanderer who met an earlier
  life of yours. Your first objective is to talk to them (nobody opens the talk screen on you). Their small asks move you from place to place, never back: they walk with you, or send you on to the next person (the second ask your background's own: a fever to treat, a pump, a
  gate to hold, a locked door) teach the Zone, tell its story and walk you to the Haven or the
  faction your background belongs with; experienced players get less of it. Every place a task
  sends you is true to its words (a house down the road, the remains by the road, the well in the
  town it names), never on an island, checked live and by a place audit on the map.
- **The Director** moves the server through four outbreak stages over a season of about 45
  real days, spreads infection from town to town and sends hordes toward noise. The infected adapt
  a little as the cure advances. Research is scaled to how much the server really plays over the
  last week, so the season takes about as long with two regulars as with twenty, and it waits at
  the lab, which works through about 3% a day even with nobody online and refuses more when its
  cold store is full, so no server can rush the cure.
- **Two kinds of infected** ([INFECTED.md](docs/INFECTED.md)): the towns hold many slow ones
  that lie up by day, come out at night, drift to noise, remember what you killed and give up on a
  locked door; the countryside's few walk somewhere, and a horde comes from a town's side. Bodies
  only near players, counted in a fixed budget, never spawned in your sight. The wild land belongs to
  **tribes**: packs with a lair, scouts on the ridges, two or three out by day, a call at dusk that
  tells you how they are, and a hunt at night that flanks, stalks, rings, flushes, baits or ambushes
  you, breaks off when it loses too many, and walks home at dawn. They live: leaders age, are
  challenged and die, guards fight for the call, packs split, quarrel over bridges, move when
  disturbed, starve, form from towns gone dark, and remember where they died, where they fed and who
  hurt them. Our code moves them; the zombie mod fights. Every faction has its own name for each
  pack, always with the place (the Haven's "Quarry lot" is Bastion's "Contact Papa-two" and the Red
  Hand's "Postmen"); you learn them, from a rumour to a ring on your map, and your own dead character
  may run with one until somebody brings them home. Some seasons a clan of them answers one deep voice
  at dusk, and comes down to the wire together.
- **Stealth**: stance, speed, light, weather and noise make you more or less visible. The
  infected near you see the most visible of you, lose a player who lies low, search where they
  last had them and give up; the HUD shows how visible you are.
- **Bites you can survive**: most hits are scratches, a bite can be cleaned in its first two
  minutes, and the infection runs through fever, weakness and a final stage. Antidotes, Halcyon's
  trial dose, stabilisers and five low-grade meds with side effects exist from day one; research
  makes them more plentiful.
- **Four factions that remember you**: the Haven, Task Force Bastion, the Red Hand and Halcyon.
  Fame and Infamy unlock quartermaster shops and perks (artillery on a horde, a fence, a bounty
  board, an immunity booster), or bring hunter squads, bounties and AI that shoots *you* and
  nobody else. Each of the 16 Fame x Infamy cells has a name ("Merciful Thug", "Useful Trouble"),
  shown with what it counts as; each faction forgives at most 30 Infamy a life, and at 75 Fame
  every new wrong costs Fame too. Bastion checkpoints and Red Hand roadblocks on the roads search you, take a toll
  or let you surrender. A faction's uniform gets you past its people, until you're caught.
- **A living background**: strongholds that never fall, a few outposts, real supply trucks with
  AI crews, patrols walking the roads round each faction's holdings (and a rare one in a vehicle), Haven safehouses that grow as you rescue people, named wanderers and leaders with
  loyalties of their own, story arcs and session headlines.
- **Camps that look lived in**: each faction moves into the building in town that suits it
  (Bastion the school or town hall on the road, the Red Hand the garage at the crossroads, the
  Haven the church, Halcyon the lab or a house), works in its yard and barricades its other road
  entries; with no such building, a small compound on rings. Fires, tables, washing, firewood,
  clutter and each faction's own things stand round its posts, camps, safehouses and field stations
  (scenery only: nothing to loot or use; each piece at its props' real size, fitted on the site's
  ground read on a half-metre grid, or left out rather than squeezed in, and every site scored by an
  audit), and people keep their faction's routine at the site's own posts and on its own rounds: the
  sentry at the gate and the lookouts, the walk round the inside of the wire and out along the road,
  a wary or alerted camp manning every post or lining the wire, the morning wash, chopping wood, cooking before the meal, Bastion's shift change and
  the roll at 20:30, the Red Hand's toll, the Haven's supper, Halcyon's courier, and the off shift
  asleep. Each site tells what happened to it: a pyre at the wire after the infected came close, new
  graves by the church, the old owner's mark beside the new one at a taken gate, and a line from its
  people when you walk in. Nobody looks the same: each faction's people wear their own kind of clothes in several
  looks, and the same person always wears the same one.
- **Sites built by hand**: a server admin or game master builds any camp site in Game Master and
  saves it with the site builder (a menu that lists every site and whether it's automated or
  hand-built, takes you there, and keeps backups); the game lays that site out from the build, near
  players only, and every other site stays automated. Role spots put down there (seats, the canteen,
  the gate, watch posts, cook, wash, chop, sleep, the walk) run its people's day, with a checklist of
  what each site still needs and a pin to keep it in place every season.
- **Questlines** that run over many sessions: each stage is a contract or a wait, you can talk,
  persuade, sneak, fight, bribe, betray or side with another faction, and what you chose changes
  the ending, the rewards and the world. Fifteen: three per faction, two for the Long Table and
  the example Short Weight, crossing each other through shared flags ([QUESTLINES.md](docs/QUESTLINES.md)).
- **Four majors**, one season-long story per faction (The Book Has to Balance, Await Relief, Every
  Name, Proof), each a page its stronghold leader would rather burn, with climaxes on the cure's
  milestones in order, council votes and a muster, and codas at the north road after the cure. Each
  ending leaves one small story rule for next season, a line in the hall and a page in the notebook's
  season tab. Their
  defaults fire from what the server is really doing (research pace, hours played, who's engaging,
  the gap since the last beat), never on fixed days ([QUESTLINES.md](docs/QUESTLINES.md#the-majors)).
- **The cure's finale** ([FINALE.md](docs/FINALE.md)): at 90% the lab's batch is missing one thing,
  and five routes race to bring it (the man who didn't turn, the captain's Day 2 tubes, the cook's
  still, the lower store's reference stock, the eight who walked out of the Day 6 trial). Each needs
  one thing a rival faction holds, and every door has a slower way round. You lock to one route for
  the season; the first to finish makes the batch and earns its faction a capped chunk of season
  Fame; and the final stand is a fight for the clean room: whoever holds it when the stand is won
  controls the cure and chooses its terms. Season Fame decides only when nobody holds the room. You
  commit at a leader's [Point of no return]; what you did earlier in the season can count as a route
  asset; every route has ways to talk, threaten, trade, trick, force or betray its way through, a
  branch when its subject is lost, and rivals can spoil, drain, take or ransom what it needs.
- **The epilogue week** ([EPILOGUE.md](docs/EPILOGUE.md)): for a week after the cure the Zone tells
  its season in the world, not in slides: a board by the lab gate with the names of who held it,
  candles on a safehouse step with a number chalked beside them, a queue on the north road that
  doesn't move until the last day. The cure is given on the spot, the way the winner would give it
  (Bastion's roster and a firearm, the Red Hand's two tins, Halcyon's order on the door). Every
  story still open can end inside the week (loose ends, names brought home, towns taken back, the
  last broadcast, your own line on the Names board), and on the last day your character can walk
  out at the north road, one name at a time.
- **164 missions** (plus about 90 questline stages): faction contracts (a few about what a faction would rather not say), world
  events, puzzle missions, unarmed stealth jobs, missions at the quarantine line, morally grey
  Haven and Red Hand work and leaders' off-book jobs. **Jobs only by interaction:** every job
  starts from a person you ask or a thing you use (the radio only says who's asking, and world
  events are news, not jobs); only the people a job was offered to, and their group, are paid. Contracts
  name a rough area until you're close, unless you're a Radio Operator or trusted by the giver.
  Some start because the world asked for them (a hungry house, a lost truck, a town gone bad), and
  the choice missions pay fairly whichever side the result goes to. Some givers leave one thing out of
  the brief, a different thing each season; anyone at the scene can find it.
- **Errands**: small, true jobs named people ask in person: the cook whose pot is thin, the
  sergeant whose relief is asleep in a car, the technician whose freezers are short of fuel, the
  Doctor on the road with a bitten stranger before dark. 34 routines at safehouses, posts, camps and
  stations, the wanderers' asks, 18 strangers met only where one makes sense (a road people use, a
  campfire, a stopped car, a town edge at dusk), locals who remember what you did, board and radio
  asks, and 70 hand-written cards, each true only while its reason is; they pay little, and remember you ([ERRANDS.md](docs/ERRANDS.md)).
- **Work from the first day, and duties**: a faction's giver with no open contract for you makes one
  when asked, and its named people always have a duty: the place's own work, taken over for a shift.
  Chores at first (the kitchen, wood and water, washing down), then, as they come to know your face,
  the line at the gate and the dead (sorted in person: go to each, look where you choose with fewer
  looks than places, carry it to the pyre, the table or the grave, and a bitten one let through
  costs you), a watch, a patrol with their people, for the Known a hunt with the hunter, the
  stores and Halcyon's sample bench, and for the Trusted the lock-up, the names (of the dead, in the
  Red Hand's debt book, on Halcyon's week report), the trial list, who gets a dose, a cull and the
  animal house. Each faction has five of its own besides (the Haven's garden and swap
  table, Bastion's gate duty and kit inspection, the Red Hand's toll and still, Halcyon's decon line
  and observation ward...), most of them worked out from what you find, with the call yours.
  Something happens on the way (a mark on a door, someone
  hiding, a truck nobody has found), and what you do about it is remembered. Each run is drawn from
  the world as it is
  ([ERRANDS.md](docs/ERRANDS.md#duties)).
- **Hands-on work**: a step that says dig, build, board up, fix, search, fill, paint or burn is a thing
  at its place with one prompt you hold (*Dig the latrine*), and the world shows the result: the
  latrine screen, a grave, the boarded door ([work sites](docs/DESIGN.md#work-site)).
- **Seasons**: the faction that got the cure shapes the next season's start, admins can add rule
  sets (Winter, Fog, Military Collapse, No Vehicles, or their own), the server wipes itself only
  after the cure's epilogue week (the line stands down, the gates open with the wipe; a timed wipe is
  there for admins who pick it, off by default), and a hall of fame, frozen at the cure, remembers who
  carried samples, held the final stand and rescued the most, season after season, with the week's
  lines.
- **One world, one story**: missions, arcs, questlines and voices read the same world facts
  (the cure, the day, who holds a town, fixed facts of the setting), so the line doesn't shoot
  after the cure and two stories don't use the same safehouse at once. Contracts pick their towns
  by who holds them, the origin story, the last orders and the Red Hand's pit are the same all
  season, and a contract waits while the questline that tells its story runs.
- **The Long Table**: some seasons, a small family in a farmhouse past the emptiest village. They
  set a place for whoever comes, the host on the road is the friendliest man in the Zone, and
  people who take that road don't always arrive. Eat with them, trade, find out, tell someone or
  end it yourself ([LORE.md](docs/LORE.md#the-long-table)).
- **People who talk back** ([DIALOGUE.md](docs/DIALOGUE.md)): with **Dialogue > Trees** on (off
  by default until it's play-tested), the people you talk to have topics in their own voices, with
  what each answer needs shown before you choose (persuade, a threat that always costs, a price, your
  background) and no dice; they remember how you spoke to them this life. Anyone with work for you
  shows it first, one reply a job tagged [Quest], [Job] or [Errand], explained and taken in the same
  conversation (with a card when it starts and ends); work you can't take yet shows locked with what
  it needs. Around that, they talk about what's on their mind, and what they've heard is what's really going on
  (a town that fell, a truck that didn't come), told their way, and not always truly. Each faction has its own
  word for you, the Red Hand's crews give you a name you'd rather they hadn't, and leaders tell you
  what they make of you. People lie to you (and the truth is out there to catch them with), you
  can make promises they'll hold you to, people say a line as you pass, strangers tell you who they
  are, and anyone can call on the open channel. The season keeps the name you made, whatever face
  you come back with. Talk never pays Fame.
- **Doors that talk** ([TRACES.md](docs/TRACES.md)): chalk, paint and stencils beside a few doors in each
  town, written from the town's real history: Bastion's search code from the first days, the Haven's
  bears and candles, the Red Hand's hands and tallies, Halcyon's beak, residents' last words, the
  infected's scratches. Stand at one and anyone reads the gist; a Police Officer reads Bastion's code,
  a Hunter how old the scratches are. Graves, ash and candles where players died or burned the dead.
  The first days frozen in place (the column's cars on the north road, the gym, the Day 4 church, the
  cordon's roadblock with its radio still hissing, the hill where two men watched the relief go by),
  small scenes where something happened (a table laid for four, a chair facing out at a village's
  edge, IOUs on a shop door), wrecks, scorch and crosses that follow the season, and after dark
  lanterns where people live, dogs that go quiet before a hunt, one stroke of a church bell and the
  town loudspeaker. Every prop is optional: the words always come. Nothing pays.
- **Notes between players**: a notice board at every Haven safehouse.
- **Masts you put up and give away** ([DESIGN.md](docs/DESIGN.md#the-masts)): three to five of the
  map's own masts, each missing a part, with an old radio man, a Red Hand crewman or a pack at its
  foot. Climb it with the part and say whose voice it is (a faction's, everyone's, or the Red Hand's
  to sell): its region hears warnings, overhears factions' traffic, and with two up a trapped man's
  voice can be found by taking bearings.
- **Loot the world**: a few things lie loose in every building, on worktops and floors and suited
  to it (medicine in a pharmacy, tools in a garage), and you pick them up the game's way; picked-over
  towns are thin; bodies looted like any body, car wrecks' boots to open like any storage; treating each other is the
  game's own, and a Nurse or Doctor also slows a bite; locked doors that keep the infected out (they bang on them)
  and keys you can lose or trade, letters that point somewhere, rare fixable cars, crafting (nineteen recipes at the bench and the stove, with junk to find for them; a lockpick for locked doors),
  traps, fire (a full fuel can in your hands at the door), water and secure radio channels.
- **Teaches itself**: no tutorial pop-ups, just a note where you wake, a wind-up radio and
  people who tell you things.
- **Any map**: towns, the quarantine line and its checkpoints are found automatically.
- **A UI that looks like the Zone**: quarantine signage and survivor marks, original faction
  emblems, a spawn screen in place of vanilla's deploy menu (with a short card of the life just
  ended over it at each death), an infection timer with the golden window, styled notice cards in each faction's
  colour that come a few at a time by importance (one card at spawn, the rest into the notebook),
  a small quest tracker top left with the tracked job's objectives, each pointing where its own step is done, a you-are-here
  arrow on the map, a notebook (standing, quests, radio, a journal of
  past jobs and what you know, the season with the hall of fame) opened with N, the pause menu or the notebook every life carries, a talk screen
  for everyone who talks (one Talk prompt a person, their lines typed out a box at a time, small
  nametags over the people near you), a repair screen for the cars, a shop screen at the quartermasters, a
  crafting screen at workbenches, a panel at the power station (a breaker board to set from the card
  inside its door) and a cooking screen at stoves, a radio screen with every channel and
  the log (View radio), and one small object menu for props and places, with a strip of readouts (POWER OFF, FIRE LIT)
  and the options you can't use yet locked with a word saying why (the game's own
  inventory, looting, healing, doors and fuel first, a menu for anything with options, a single
  prompt only for what's done in a hurry),
  spoken lines as subtitles that say who is talking and from where (radio voices in their own box), notes on a paper reader, and on the map only what you know (of the jobs, only the tracked one's objective; under a fog of war, only the ground this character has seen, and places as rumours until then), with its key in the corner. Gamepad
  and console as well as keyboard and mouse: every screen works with a pad, nothing needs typing
  or a chat command, and no text is under 18 px for a TV (the HUD and notice cards show in Workbench play,
  and the spawn screen, the map key and the map icons with the keyboard, pad not yet; the notebook, talk,
  repair, shop, crafting, cooking and radio screens, the object menu, the power panel, reader, death brief and subtitles are written and not yet played).

## Status

The game scripts, about 246,500 lines of Enforce Script under `scripts/Game/ReforgerZ/`, **compile
without errors or warnings** against Arma Reforger 1.9 and run with no script exceptions in the
server's runtime test and in Workbench play on Everon and Arland; every approved design is built
(the last, the sites design: each site's ground read on a grid with real footprints and its surroundings, the placement rules and the solver, building in layers by each faction's doctrine, the camps' people at their posts and on their rounds, each site's story, and the site audit), simplified as each doc says; none waits in `designs/`. The repo holds 285 prefabs (many on stand-in bases), the worlds and the
UI; before it can be called working it needs real prefabs in place of the stand-ins, the `VERIFY`
behaviours checked and a play-test with a game client (on a Linux server, Special Zombies stops the
mode loading: [MOD_STACK.md](docs/MOD_STACK.md#special-zombies-stops-reforgerz-loading-on-a-linux-server)).
What's built, next and to test first is in [ROADMAP.md](docs/ROADMAP.md); start with
[PC_SETUP.md](docs/PC_SETUP.md), then [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md).

## Docs

| Doc | What's in it |
|---|---|
| [LORE.md](docs/LORE.md) | The sealed Zone, the factions as people, endings |
| [WRITING.md](docs/WRITING.md) | How player text is written: tone, the voices, the season mood and the voice bank, conversations, grey choices, the humour line, the text lint's rules |
| [DESIGN.md](docs/DESIGN.md) | How every system works, by system |
| [PLACES.md](docs/PLACES.md) | Task places and things: every task's place and the things its words name checked as data, live in play and by the place audit on a map |
| [SITES.md](docs/SITES.md) | Camp sites: each site's ground read on a grid, the props' real footprints, the surroundings and threat, the rules and the solver, building in layers by doctrine (the wire and its gates, the boards, the watch, valuables indoors), the people (posts, shifts, patrol routes, everyday movement, alert states), each site's story, the site audit and its drawing, hand-built sites and the site builder |
| [DIALOGUE.md](docs/DIALOGUE.md) | Talking to people: the talk screen, conversation trees, checks, memory, the `.rzd` format and its tools |
| [ROADMAP.md](docs/ROADMAP.md) | Built, next, play-test priorities, dropped ideas |
| [REPUTATION.md](docs/REPUTATION.md) | Factions, Fame/Infamy/Favor, shops, perks, leaders, AI hostility |
| [INFECTED.md](docs/INFECTED.md) | The infected: towns' crawlers, loose ones, hordes, the land grid, bodies and the agent budget, the tribes and their life cycle, borders, memory, who they are, how they move and hunt, the crawlers' life, the people's side (words, knowing, skills, your dead run, the moat, the cure, the last week), the great one, signs and sounds, the simulator's numbers |
| [TRACES.md](docs/TRACES.md) | Traces: door marks written by the people who were there (who wrote what, how to read them by background, where they come from), doors you can see, player traces, the scenes (the first days, the line, vignettes, live traces), sound and light, the handover to the epilogue |
| [MISSION_RULES.md](docs/MISSION_RULES.md) | The rules every task follows, where each is kept, and the checklist for a new task |
| [MISSIONS.md](docs/MISSIONS.md) | Every mission and event, with requirements and status |
| [ERRANDS.md](docs/ERRANDS.md) | The small jobs named people ask in person: givers, the dealer, pay, the faults table, the routines and cards, and the duties (a place's own work, from the first day, on a trust ladder) |
| [QUESTLINES.md](docs/QUESTLINES.md) | The four majors (one season-long story per faction) and the long questlines over many sessions, three per faction and two for the Long Table |
| [FINALE.md](docs/FINALE.md) | The cure's finale: the five routes, the lock, the race, the stand as a fight for the clean room, terms, Fame and fallbacks |
| [EPILOGUE.md](docs/EPILOGUE.md) | The week after the cure: its phases, beats and carriers, the dose outlets, the queue and the lab gate, the grey choices, the closing work, the hall's freeze, names |
| [MOD_STACK.md](docs/MOD_STACK.md) | Which Workshop mods the server runs and why |
| [PC_SETUP.md](docs/PC_SETUP.md) | Setting up your Windows PC with Claude Code: Steam apps, Workbench, local tests, play-testing |
| [UI.md](docs/UI.md) | Art direction, the icon set, the screens, the UI scripts, built and planned |
| [WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md) | Turning the scripts into a playable mod |

## Code

```
scripts/Game/ReforgerZ/
  Core/          game mode component (the hub), helpers, map markers, loose-item cleanup,
                 seasons (the winner's rules, rule sets, the wipe cycle), AI groups' move
                 orders (one held waypoint each); the shared engine: the clocks, deeds, the
                 callback queue, names in public text; the photo tour (Workbench play only)
  World/         settlements, the map's fog (what each character has seen), the Perimeter, safehouses and their notice boards, wanderers,
                 deer herds in the country and their meat, the things players build cleared
                 when unused (fireplaces, tents),
                 town memory, the power grid (where its station is, the control at it, its
                 dressing and generator sound, the power panel's server side and its breakers), the radio station, the masts (put up, given
                 away, their warnings, listening in and a trapped voice's bearings), the world facts (what content
                 reads: season flags, canon, claims and soft holds, text tokens) and its world
                 memory (reasons, signals, the town log, world records, the truth deck) with its
                 writers, the marks spawner, the Long Table, the land grid (water, height, trees,
                 roads, bridges and wild land), named people, the finale's people and the faults
                 table; task places (every task's place need and the things its words name, the
                 checks, the finder and the place audit)
    HandBuilt/   hand-built sites: the saves, the site builder menu, its Game Master row,
                 camera and guides, role spots and each site's checklist, and the shipped
                 saves (generated by tools/hand_built.py)
    Traces/      Traces: the door-mark grammar, the first days, doors on the map, the writers
                 and the watch, reading, decals' list and boards, player traces, the scenes,
                 sound and light, the placer camp life shares, the clue registry
  Director/      outbreak stages, spread, heat, adaptation, time and weather,
                 the broadcast, the season mood and grief, story arcs, the questline engine,
                 the majors' engine (acts, votes, the dynamic fallbacks, the season story, the
                 stand's inputs), next season's story rules, headlines, the season budget and the
                 set-piece calendar; Epilogue/: the epilogue week (the record, beats, the dose
                 outlets, its places and scenes, the closing work, the evening calendar)
  Zombies/       spawner (any zombie mod's prefabs), the towns' crawlers, bodies and the agent
                 budget (roles, data parts, the sight rule, runtime groups), the tribes (seeds,
                 the daily rhythm, hunts, scouts, their bodies), their life cycle, borders and
                 memory, the people's side (words, knowing, your dead run, the moat, the last
                 week; the tag and the call box), the great one, signs and sounds (props,
                 call sounds, the moon), the small rules (prey, stragglers, patrols), bite
                 detection, stealth
                 (visibility, the infected's senses, losing them), the player effects component
                 (the close moves played on the player's machine)
    Behaviour/   the behaviour layer: Bacon's hooks, steering modes, senses, hearing, blood
                 and shrieks, formations, hunt strategies, reactions, doors, navigation, the
                 kinds' attacks and their effects (server side), the crawlers' daily life
  Backgrounds/   backgrounds, personal goals, player profiles
  Factions/      reputation and its 16 names (and the season record and titles), quartermasters, shops and the shop screen's server
                 side, perks, services, hunter squads, bounties,
                 road posts and camps, leaders, AI hostility, disguises and retaliation,
                 territory, supply trucks, road patrols, faction life, camp life (scenery and routines; each site's
                 ground on a grid, its surroundings, its people's posts and routes,
                 its story and its audit),
                 reputation's voice (the word for you, crew names, the picture), the voice bank
  Missions/      framework, requirements, bases, building blocks (Blocks/: escorts, live
                 capture, vehicle zones and convoys, health cost, step chains, work sites, carrying); Haven/ Bastion/ RedHand/ Halcyon/
                 Events/ Leaders/ Unarmed/ Perimeter/ Tribes/ (the packs' contracts); the shared checks (Trust, Fear, prices) and
                 ways through a job; Story/ (the story kit, who decides, who benefits, reasons);
                 Errands/ (the dealer, the routines, the duties, Cards/ with the epilogue deck); Questlines/ (the author's base class,
                 approach helpers, the majors' scenes, the registry, one folder per faction with
                 its major, one for the Long Table, and Finale/: the cure finale's five routes,
                 desks, tissue hunts, items and route assets)
  Puzzles/       clues, puzzle objects and actions, noisemakers, landmarks; Missions/
  Cure/          research, lab, samples, antidotes, infection stages, treatments and meds; the
                 cure finale's controller (tracks, locks, pacts, the batch, the stand's clean room)
  Guidance/      experience levels, tips, openings, open radio channels
  Interactions/  loose loot, wrecks' boots, door locks, doorway blockers and keys, building queries (mission
                 spots kept out of locked buildings), cars (a car's room to stand, the hotwire's
                 loom), a work site's hold (its animation and movement lock), petrol pumps as vanilla fuel, crafting and
                 the stove (and their screen's server side), the object menu's server side and grouped
                 actions, its status strip and locked rows, water, traps, generators, fire, secure radio and the radio screen's server
                 side (View radio), a medic slowing a bite
  Persistence/   save and load of mode state, the hall of fame
  Dialogue/      the talk screen's engine: conversations, adapters for each kind of person
                 (strangers, the open channel, the quartermasters, the medic and a duty's people too), what a reply
                 needs, who's near each player for the nametags; the trees (hubs,
                 nodes, checks, effects), conditions, memory (dialogue.json: promises, callbacks,
                 lies told), scenes and sittings, barks, Make it right, greets that open, the secure
                 nets and distress calls, bindings and world predicates, the notebook's talk rows;
                 Pack/: the conversations, generated from dialogue/
  UI/            the per-player data feed, HUD (notice cards, subtitles), the notebook and its
                 pages, the pause menu button and notebook item, the talk screen and the nametags, the reader,
                 the car repair, shop, crafting and cooking screens (and the item picture), the radio screen, the object menu and the status strip, the power panel, the street lamps (dark but where there's power), the screen guard, the notice board, the spawn screen and loadout lock, the
                 death brief, the map's icon layer, its fog and key, notice styles, notice lanes (the interruption budget), chat commands
scripts/WorkbenchGame/ReforgerZ/
                 the Workbench plugins: ReforgerZ: Create prefabs, Write prefab checklist
                 (RZ_CreatePrefabsPlugin.c); the PC's helpers for building prefabs as text:
                 Write resource index, Dump prefabs and classes, Register or rebuild resources
                 (RZ_ResourceIndexPlugin.c)
Worlds/RZ_Everon/
                 the main world: a sub-scene of Eden with the game mode, the faction and
                 loadout managers, Eden's AI world and vanilla's world managers
Worlds/RZ_Arland/
                 the small test world, the same on Arland
Missions/        the scenarios that play them (RZ_Everon.conf, RZ_Arland.conf)
Prefabs/         ReforgerZ/: the prefabs made from the manifest; the player controller
                 (Characters/Core/) and the faction and loadout managers (MP/Managers/)
Configs/ReforgerZ/PrefabManifest.json
                 every prefab, slot, settings object, faction and loadout to set up
Configs/System/  the vanilla menu and input configs, with the ReforgerZ screens and keys added
Configs/Map/MapFullscreen.conf
                 the vanilla full-screen map config, with the ReforgerZ icon layer and map key added
tools/prefab_manifest.py
                 checks the manifest against the scripts and writes the setup doc's tables
tools/camp_kit/  pieces.py: the camp-life pieces, their kits' plans and the Traces compositions; build.py
                 writes their prefabs and the game mode's slots, with each piece's real footprint from
                 footprints.json (the props' measured boxes); render.py draws them and the faction
                 bases' plans (bases.py) into tools/camp_kit/camp_plans.html, and with --placed the
                 sites a site audit laid out into sites_placed.html; world_kit.txt: the world-building
                 kit, every prop, group and building to dress places with and where each suits
tools/people_looks.py
                 writes the faction people's look prefabs (Prefabs/ReforgerZ/People/) and checks their GUIDs
tools/hand_built.py
                 ships the site builder's saves: checks them and writes RZ_HandBuiltPack.c (--check: check only)
UI/              icon sources and PNGs (Icons/), HTML mockups (Mockups/), previews (Previews/),
                 the layout guide (LAYOUTS.md), the layouts (layouts/ReforgerZ/), the icon
                 atlas and the car picture (Textures/ReforgerZ/) and its imageset (Imagesets/ReforgerZ/)
tools/render_ui.js
                 renders the icon PNGs, the mockup previews and the textures (Node and Playwright)
tools/icon_atlas.py
                 packs the icons into the atlas and writes the imageset (Python and Pillow)
tools/pacing_sim.py
                 simulates a season's research, the lab and the stages for 2, 5 and 10 players
tools/tribe_sim.py
                 simulates the infected tribes as data over a season on that pacing, on the
                 code's rules and the measured wild land of Arland, Everon and Cain and a
                 Chernarus-size map, 2, 5 and 10 players
dialogue/        the conversations as .rzd text (leaders, wanderers, residents, posts, table,
                 strangers, radio, barks, finale)
                 and shipped.lock, the ids that have shipped
tools/dialogue_pack.py, tools/dialogue_sim.py
                 the dialogue lint and packer; walks a conversation as one character sees it
tools/voice_sim.py
                 replays the voice bank's picker: pool sizes and joke shares by mood
tools/strings_inventory.py
                 lists every line of player text in the scripts and the conversations
tools/mission_rules.py, tools/mission_rules_allow.txt
                 the mission rules check: every task against docs/MISSION_RULES.md from the
                 source; the allowlist holds the hits that are right, and the pending ones
tools/voice_lint.py, tools/voice_lint_baseline.txt
                 the text lint: checks player text against the writing rules; the baseline
                 holds today's findings, so only new or changed text fails
tools/compile_check.sh
                 compiles the scripts on the dedicated server, no Workbench needed (Linux,
                 or Windows in Git Bash)
tools/runtime_test/
                 runs the game mode on that server with a fake player, (run.sh sites) the site
                 audit and (run.sh tasks) the task place audit on Arland, Everon or Cain, and
                 (run.sh layer) the behaviour layer test: a pack of Bacon's bodies hunting the
                 fake player on Arland; places.py draws the task places (test only, not the mod)
tools/zombie_lab/
                 measures Bacon Zombies' own AI on that server, on Everon: groups, waypoints,
                 sight, sharing, speeds, health, the clock, frame time (test only, not the mod)
tools/land_survey/
                 reads the land grid on Arland, Everon and Cain on that server and reports what
                 it found (test only, not the mod)
.claude/skills/pc-setup/
                 /pc-setup for Claude Code on your PC: points it at docs/PC_SETUP.md
addon.gproj      the project file (mod ID, dependencies)
```
