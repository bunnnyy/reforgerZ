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

		// Task Force Bastion

		// The Red Hand

		// Halcyon

		// Unaligned groups
		// The Long Table (World/RZ_LongTable.c; only in a season that has one)
		q.Register(RZ_Quest_HouseWentQuiet);
	}
}
