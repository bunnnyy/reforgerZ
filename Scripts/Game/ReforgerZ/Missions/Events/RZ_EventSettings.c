//! Settings for the newer world events (docs/MISSIONS.md, "World events"). On
//! RZ_GameModeComponent > Missions > Events. Every prefab here is optional: an event that needs
//! one it doesn't have falls back (named below) or never appears.
[BaseContainerProps()]
class RZ_EventSettings
{
	// ---------------------------------------------------------------- Fog Bank
	[Attribute("700", desc: "Fog Bank: radius of the fog (m)")]
	float m_fFogRadius = 700;

	[Attribute("0.6", desc: "Fog Bank: heat added inside the fog is multiplied by this (it muffles you)")]
	float m_fFogHeatMult = 0.6;

	[Attribute("0.5", desc: "Fog Bank: inside the fog, the closest distance infected spawn from players is multiplied by this")]
	float m_fFogSpawnMult = 0.5;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fog Bank: a low fog or mist effect, spawned at five points while it lasts (look only). Empty = no effect; the engine's own fog is never changed", "et")]
	ResourceName m_sFogEffectPrefab;

	// ---------------------------------------------------------------- Power Surge
	[Attribute("10", desc: "Power Surge: real minutes a town's power stays on")]
	float m_fSurgeMinutes = 10;

	// ---------------------------------------------------------------- Fire (Gas Station Fire, Burning Town)
	[Attribute("8", desc: "Gas Station Fire: infection taken off the town when the station goes up")]
	float m_fStationInfectionDrop = 8;

	[Attribute("8", desc: "Burning Town: most buildings that burn (and never more than 40% of the town)")]
	int m_iBurnMax = 8;

	[Attribute("3", desc: "Burning Town: infection taken off the town for each building that burns (on top of the fire's own)")]
	float m_fBurnInfectionDrop = 3;

	// ---------------------------------------------------------------- Tunnel Rats
	[Attribute("20", desc: "Tunnel Rats: most infection the open culverts add to the town")]
	float m_fTunnelInfectionCap = 20;
}
