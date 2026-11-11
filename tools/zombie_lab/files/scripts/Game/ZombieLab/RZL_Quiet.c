// Test-only (not part of the mod). Bacon's faction lets its AI mark spotted targets on the map; the
// lab world has no map, so the vanilla marking system gets no marker back and throws on every
// sighting. Map markers don't change what the AI does, so the lab skips them.
modded class SCR_AIEnemyMarkingSystem
{
	override void MarkTarget(notnull SCR_AITargetInfo targetInfo, notnull SCR_AIGroup spottingGroup)
	{
	}
}
