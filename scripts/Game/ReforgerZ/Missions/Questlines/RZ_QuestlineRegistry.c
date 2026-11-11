//! Every questline, registered in one place (docs/DESIGN.md, "Questlines"). Add a line here for
//! each new questline class. Stage missions are never registered anywhere: their questline
//! starts them.
class RZ_QuestlineRegistry
{
	//------------------------------------------------------------------------------------------------
	static void RegisterAll(RZ_Questlines q)
	{
		// The Haven
		q.Register(RZ_Quest_ShortWeight);
		q.Register(RZ_Quest_TheList);
		q.Register(RZ_Quest_PriceOfTheDoor);
		q.Register(RZ_Quest_EdgeOfTown);
		q.Register(RZ_Major_EveryName);

		// Task Force Bastion (Missions/Questlines/Bastion/)
		q.Register(RZ_Major_Orders);
		q.Register(RZ_Quest_Census);
		q.Register(RZ_Quest_LastPilot);

		// The Red Hand (Missions/Questlines/RedHand/)
		q.Register(RZ_Major_Book);
		q.Register(RZ_Quest_Cargo);
		q.Register(RZ_Quest_WellStory);

		// Halcyon
		q.Register(RZ_Quest_ThreeWeeks);
		q.Register(RZ_Quest_OtherTwo);
		q.Register(RZ_Quest_Authors);
		q.Register(RZ_Major_Proof);

		// The cure finale's five routes (Missions/Questlines/Finale/; docs/FINALE.md): research stops
		// at the 90 percent gate while they're registered and the finale is on
		q.Register(RZ_Route_Haven);
		q.Register(RZ_Route_Bastion);
		q.Register(RZ_Route_RedHand);
		q.Register(RZ_Route_Halcyon);
		q.Register(RZ_Route_OneEach);

		// Unaligned groups
		// The Long Table (World/RZ_LongTable.c; only in a season that has one)
		q.Register(RZ_Quest_HouseWentQuiet);
		q.Register(RZ_Quest_PlaceAtTable);
	}

	//------------------------------------------------------------------------------------------------
	//! The fixed keys of the questlines designed in docs/QUESTLINES.md and not built yet. Content
	//! may gate on them already (RZ_MissionRequirements.NotWithQuest); the questline uses the same
	//! key when it's written, so the gate starts working by itself.
	static bool IsDesigned(string key)
	{
		array<string> keys = {
			"list", "door", "walk",
			"orders", "census", "pilot",
			"book", "cargo", "well",
			"weeks", "other_two", "authors",
			"table", "place",
			"every_name", "proof",
			"route_haven", "route_bastion", "route_redhand", "route_halcyon", "route_one_each"};
		return keys.Contains(key);
	}
}
