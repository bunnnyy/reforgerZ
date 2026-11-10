//! Settings for the newer world events (docs/MISSIONS.md, "World events"). On
//! RZ_GameModeComponent > Missions > Events. Every prefab here is optional: an event that needs
//! one it doesn't have falls back (named below) or never appears.
[BaseContainerProps()]
class RZ_EventSettings
{
	// ---------------------------------------------------------------- Fog Bank
	[Attribute("700", desc: "Fog Bank: radius of the fog (m)")]
	float m_fFogRadius;

	[Attribute("0.6", desc: "Fog Bank: heat added inside the fog is multiplied by this (it muffles you)")]
	float m_fFogHeatMult;

	[Attribute("0.5", desc: "Fog Bank: inside the fog, the closest distance infected spawn from players is multiplied by this")]
	float m_fFogSpawnMult;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fog Bank: a low fog or mist effect, spawned at five points while it lasts (look only). Empty = no effect; the engine's own fog is never changed", "et")]
	ResourceName m_sFogEffectPrefab;

	// ---------------------------------------------------------------- Power Surge
	[Attribute("10", desc: "Power Surge: real minutes a town's power stays on")]
	float m_fSurgeMinutes;

	// ---------------------------------------------------------------- Fire (Gas Station Fire, Burning Town)
	[Attribute("8", desc: "Gas Station Fire: infection taken off the town when the station goes up")]
	float m_fStationInfectionDrop;

	[Attribute("8", desc: "Burning Town: most buildings that burn (and never more than 40% of the town)")]
	int m_iBurnMax;

	[Attribute("3", desc: "Burning Town: infection taken off the town for each building that burns (on top of the fire's own)")]
	float m_fBurnInfectionDrop;

	// ---------------------------------------------------------------- People
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Prison Break: escaped convicts, an armed AI group on BAND (the Red Hand takes them in). Empty = Missions > Red Hand group", "et")]
	ResourceName m_sConvictGroupPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "The Sniper: one armed AI with a scoped rifle, on BAND (he fights everyone near him through the hostility system). Empty = Contracts > Bounty target", "et")]
	ResourceName m_sSniperPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Rival Survivors: an armed AI group on SURV, nobody's colours. Empty = Missions > Survivor group", "et")]
	ResourceName m_sRivalGroupPrefab;

	// ---------------------------------------------------------------- Loot (Abandoned Convoy, Military Remnant Camp, Locked Bunker, Wedding Party)
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Abandoned Convoy: a military truck wreck prop, not drivable (two are placed; look only). Empty = just the cargo by the road", "et")]
	ResourceName m_sTruckWreckPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Military Remnant Camp: a tent or small camp composition (look only). Empty = just the crates", "et")]
	ResourceName m_sCampPrefab;

	[Attribute("10", desc: "Military Remnant Camp: Bastion Infamy for taking from its stores while Bastion is still around")]
	int m_iCampInfamy;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Locked Bunker: a steel hatch prop, used when no locked building with doors is found. Empty = Puzzles > Puzzle object", "et")]
	ResourceName m_sHatchPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Wedding Party: infected in formal wear (one at random per guest). Empty = Zombies > Common prefabs", "et")]
	ref array<ResourceName> m_aWeddingPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Wedding Party: the bride. Empty = Zombies > Special prefabs", "et")]
	ResourceName m_sBridePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Wedding Party: a piece of jewellery (a small item whose name has one of the jewellery keywords). Empty = a crate once the party is down", "et")]
	ResourceName m_sJewelleryPrefab;

	[Attribute("jewel,necklace,bracelet,weddingring,wedding_ring", desc: "Jewellery keywords: a Trader wanderer takes one such item instead of Favor")]
	string m_sJewelleryWords;

	// ---------------------------------------------------------------- Tunnel Rats
	[Attribute("20", desc: "Tunnel Rats: most infection the open culverts add to the town")]
	float m_fTunnelInfectionCap;
}
