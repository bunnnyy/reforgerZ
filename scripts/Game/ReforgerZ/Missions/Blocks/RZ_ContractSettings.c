//! Settings for the mission building blocks (docs/DESIGN.md, "Mission building blocks") and the
//! optional prefabs of the contracts built on them. On RZ_GameModeComponent > Missions > Contracts.
//! Every prefab here is optional: a contract that needs one it doesn't have falls back (named
//! below) or never appears.
[BaseContainerProps()]
class RZ_ContractSettings
{
	// ---------------------------------------------------------------- Rough areas (RZ_Mission)
	[Attribute("500", desc: "Contracts give a rough area: a circle of this radius (m) that holds the real spot, centred off it. Radio Operators and players Trusted with the giver get the exact spot. 0 = always exact")]
	float m_fRoughRadius;

	[Attribute("500", desc: "The exact spot is revealed to a player who comes this close (m) to it")]
	float m_fRevealDistance;

	// ---------------------------------------------------------------- Arriving (RZ_Arrive)
	[Attribute("15", desc: "Arrive radius (m): a step done by getting to a spot (a job's points, an errand's or an opening's place, a chain's step, a meeting) counts this close, or within the step's own radius if that's wider. Kept to 5-40")]
	float m_fArriveRadius;

	[Attribute("25", desc: "Arrive margin (m): a step done by getting to a town, a camp or a building counts inside it or this close to its edge. Kept to 0-50")]
	float m_fArriveAreaMargin;

	// ---------------------------------------------------------------- Escorts (RZ_Escort)
	[Attribute("3", desc: "Most escorts (people walking with players) at once, server-wide")]
	int m_iMaxEscorts;

	[Attribute("150", desc: "Escorted people only walk on while a player is this close (m)")]
	float m_fEscortLeash;

	[Attribute("300", desc: "Escorted people get a body only while a player is this close (m); otherwise they're data")]
	float m_fEscortBodyRadius;

	[Attribute("1.3", desc: "How fast escorted people move as data, without a body (m/s)")]
	float m_fEscortSpeed;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Escorted civilians: an AI group on SURV, 2-4 unarmed people (Refugee Column, Bust Out, Desertion Route). Empty = Missions > Survivor group, then data with a Puzzles > Civilian figure", "et")]
	ResourceName m_sEscortCivilians;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Escorted scientist: a one-man AI group on INST (Escort the Scientist, Secure the Shipment). Empty = the escorted civilians", "et")]
	ResourceName m_sEscortScientist;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Escorted soldier: a one-man AI group on MIL (Rescue the Pilot, Struck Off). Empty = the escorted civilians", "et")]
	ResourceName m_sEscortSoldier;

	// ---------------------------------------------------------------- Live capture (RZ_LiveCapture)
	[Attribute("2", desc: "Most live captures running at once")]
	int m_iMaxCaptures;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Cage trap: a cage or box-trap prop with the generic puzzle object's components. Empty = Puzzles > Puzzle object", "et")]
	ResourceName m_sCagePrefab;

	[Attribute("4", desc: "The special must be this close to the cage (m) when the door drops")]
	float m_fCaptureRadius;

	[Attribute("0.3", desc: "A special at or under this share of its health, with a player within 3 m, is pinned and tied instead of killed")]
	float m_fRestrainShare;

	[Attribute("12", desc: "Real minutes before a crated specimen wakes up and breaks out")]
	float m_fSpecimenWakeMinutes;

	// ---------------------------------------------------------------- Vehicles (RZ_VehicleZone)
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A drivable bus or truck (Recover the Bus). Empty = Supply > Haven truck, then Cars > Car prefabs", "et")]
	ResourceName m_sBusPrefab;

	[Attribute("bus,truck,van,ural,kamaz,lorry", desc: "Keywords of vehicles that count as a bus or truck (Recover the Bus also takes any of these)")]
	string m_sBusWords;

	// ---------------------------------------------------------------- Health cost (RZ_HealthCost)
	[Attribute("0.25", desc: "Share of full health the Blood Drive takes")]
	float m_fBloodCost;

	[Attribute("0.7", desc: "Nobody gives blood under this share of full health")]
	float m_fBloodMinHealth;

	// ---------------------------------------------------------------- Optional contract prefabs
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bounty target: an armed AI group or character on SURV, a loner who owes the Red Hand (Collect the Bounty when no player has a bounty; it's made hostile as SURV). Empty = Missions > Survivor group", "et")]
	ResourceName m_sBountyTargetPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion squad: an AI group on MIL (the stockade guards in Bust Out, Crash Site's recovery team). Empty = Factions > Bastion hunter squad, then Supply > Bastion crew", "et")]
	ResourceName m_sBastionSquadPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Crashed drone: a small wreck prop (Medical Drone; look only). Empty = Puzzles > Puzzle object", "et")]
	ResourceName m_sDroneWreckPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Smoke marker: a coloured smoke effect (Fire Mission; look only). Empty = no smoke, the message says it", "et")]
	ResourceName m_sSmokePrefab;
}
