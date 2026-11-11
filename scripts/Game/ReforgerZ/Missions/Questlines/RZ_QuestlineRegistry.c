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

		// Task Force Bastion (Missions/Questlines/Bastion/)
		q.Register(RZ_Quest_AwaitRelief);
		q.Register(RZ_Quest_Census);
		q.Register(RZ_Quest_LastPilot);

		// The Red Hand (Missions/Questlines/RedHand/)
		q.Register(RZ_Quest_TheBook);
		q.Register(RZ_Quest_Cargo);
		q.Register(RZ_Quest_WellStory);

		// Halcyon
		q.Register(RZ_Quest_ThreeWeeks);
		q.Register(RZ_Quest_OtherTwo);
		q.Register(RZ_Quest_Authors);

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
			"table", "place"};
		return keys.Contains(key);
	}
}
