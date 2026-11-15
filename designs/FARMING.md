# Farming: growing your own food

**Designed, not built.** Approved by the author on 2026-11-15. Super basic on purpose: no story yet.

The map's greenhouses and garden plots are where you grow food. Four crops, a few steps by hand,
ready in one to two hours.

## The crops

| Crop | Grows in | Harvest |
|---|---|---|
| Tomatoes | 1 h | a few tomatoes |
| Cucumbers | 1 h 15 min | a few cucumbers |
| Bell peppers | 1 h 30 min | a few peppers |
| Pumpkins | 2 h | one pumpkin (heavy, feeds more) |

Each harvest is food you can eat (vanilla's eating, as the game's own food). The times are real play
time on the server, with no time skip; tuned in the settings.

## Where

- **Greenhouses** on the map (the vanilla greenhouse props): a few beds each.
- **Garden plots** by farmhouses: the soft ground beside them.
- Found from the map the way kitchens and work sites are (by the prop's name and the ground), so it
  works on any map. Nothing is placed for it.

## Doing it

All in the world, one short prompt per step, gamepad ready:

1. **Turn the soil** with a shovel (vanilla's entrenching tool, or a found shovel).
2. **Plant** a seed packet (found in sheds, farms and kitchens; one packet per crop).
3. **Water** it from a bottle or a canteen. A dry bed stops growing until it's watered again.
4. **Harvest** when it's ripe: the bed shows it (a plant model at its stage).

## How it runs

- Each bed is data on the server (crop, planted at, last watered): no plant models while nobody is
  near. Near a player, the bed shows a plant model for its stage (sprout, half-grown, ripe).
- Anyone can harvest a ripe bed: guard it or lose it.
- A ripe bed left alone goes over after a while and the bed is empty again.
- Saved with the world, so a restart keeps the beds.

## Not in this design

Story, faction ties (selling food to the Haven), gardeners in camp life, animals eating crops,
seasons and cold. Each can come later as its own design.

## Order of work

1. Seeds in loot, the four crops as items, the beds found in greenhouses, the four steps, growth as
   data, plant models near players.
2. Garden plots by farmhouses.
