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

		// The Red Hand

		// Halcyon
		q.Register(RZ_Quest_ThreeWeeks);
		q.Register(RZ_Quest_OtherTwo);
		q.Register(RZ_Quest_Authors);

		// Unaligned groups
	}
}
