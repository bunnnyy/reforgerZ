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

		// Task Force Bastion (Missions/Questlines/Bastion/)
		q.Register(RZ_Quest_AwaitRelief);
		q.Register(RZ_Quest_Census);
		q.Register(RZ_Quest_LastPilot);

		// The Red Hand

		// Halcyon

		// Unaligned groups
	}
}
