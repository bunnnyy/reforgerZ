# ReforgerZ

A zombie survival game mode for **Arma Reforger**. You're a lone civilian:
survive alone, team up with strangers, or turn on them. The whole server
shares one outbreak that gets worse over time, and one way out: **research
the cure.**

- **Pick a background in the spawn menu**, like an occupation in Project Zomboid:
  Nurse, Mechanic, Hunter, Police Officer, Ex-Con, Lab Technician, Radio Operator, Unemployed.
  Each has a real perk and a real flaw. Five more (Soldier, Doctor, Raider, Elder, Drifter)
  are unlocked through Legacy.
- **The Director** moves the server from Early Outbreak → Collapse → Overrun → Aftermath,
  spreads infection from town to town, and sends hordes toward noise.
- **World events and faction contracts** in real towns on whatever map you load: 10 world
  events anyone can join (horde migrations, Night of the Dead, Bastion shelling) and 42 contracts
  offered only to players the faction trusts, some of which make you choose who gets the prize.
- **Cure research**: kill special infected, take their samples to the lab, unlock
  server-wide breakthroughs, then hold the lab through the final stand.
- **Fame and Infamy**: the Haven, Task Force Bastion, the Red Hand and Halcyon each remember
  what you did for them and to them. Standing unlocks quartermaster shops and perks; Infamy
  brings hunter squads, bounties, and AI of that faction that really shoots *you* (and nobody
  else). The faction the server helps most gets the cure.
- **Named faction leaders**: a person runs every place a faction holds, with a name, a past
  and two traits that colour their briefs and nudge their prices and pay. Outpost and safehouse
  leaders can be Loyal, Independent or Rogue, give off-book jobs that can be discovered, die,
  and are replaced; stronghold leaders never fall.
- **Haven safehouses and wanderers**: a few Haven-held buildings in abandoned towns grow
  from Shelter to Town as you rescue people and bring food, with house rules (no shooting,
  declare your bite) and exile for those who break them. Named wanderers walk between them,
  remember what you did, and their deaths make the news.
- **A world that teaches itself**: no tutorial pop-ups. You wake next to a note, a flyer or a
  body that points somewhere, carry a wind-up radio that picks up the Haven's open channel, and
  hear tips from whoever is nearest, in their own voice. It fades as you gain experience, but
  never quite goes away.
- **Any map**: towns are detected from map labels and building clusters.
- **Built on the Workshop**: zombies, hunger and thirst, traders and base building come from
  existing mods. ReforgerZ ties them together.

## Docs

- [docs/PLAN.md](docs/PLAN.md): design, systems and roadmap
- [docs/WORLD.md](docs/WORLD.md): the living world: safehouses, wanderers, territory, supply trucks
- [docs/LORE.md](docs/LORE.md): the sealed Zone, tone and writing guide
- [docs/REPUTATION.md](docs/REPUTATION.md): factions, Fame and Infamy, perks, unlocks
- [docs/MISSIONS.md](docs/MISSIONS.md): every contract and world event (✅ = in code)
- [docs/MOD_STACK.md](docs/MOD_STACK.md): which Workshop mods the server runs and why
- [docs/WORKBENCH_SETUP.md](docs/WORKBENCH_SETUP.md): turning these scripts into a playable mod

## Status

Phases 1 to 3 are written in Enforce Script under `Scripts/Game/ReforgerZ/`.
They **haven't been compiled in Workbench yet**. Start with the `VERIFY` list in
the setup guide.

```
Scripts/Game/ReforgerZ/
  Core/         game mode component (hub), helpers
  World/        settlement detection, manual markers, the Perimeter, safehouses, wanderers
  Director/     outbreak stages, infection spread, heat
  Zombies/      spawner (works with any zombie mod's prefabs)
  Backgrounds/  civilian backgrounds, player profiles
  Factions/     Fame/Infamy tiers, quartermasters and shops, perks, hunter squads, bounties, leaders, AI hostility
  Missions/     framework, requirements, bases; Haven/ Bastion/ RedHand/ Halcyon/ Events/ Leaders/
  Cure/         research, lab, samples, actions
  Guidance/     new-player guidance: experience levels, contextual tips, spawn scenarios, open radio channels
  Persistence/  save/load of mode state
```
