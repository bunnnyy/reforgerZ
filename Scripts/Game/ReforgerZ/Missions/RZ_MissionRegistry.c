//! Every built-in mission type, grouped by faction. Other mods can add their
//! own through RZ_MissionManager.RegisterType().
class RZ_MissionRegistry
{
	//------------------------------------------------------------------------------------------------
	static void RegisterAll(RZ_MissionManager mgr)
	{
		// The Haven
		mgr.RegisterType(RZ_Mission_SupplyCache);
		mgr.RegisterType(RZ_Mission_SurvivorHoldout);
		mgr.RegisterType(RZ_Mission_RadioRelay);
		mgr.RegisterType(RZ_Mission_TakeBackTown);
		mgr.RegisterType(RZ_Mission_BurnTheNest);

		// Task Force Bastion
		mgr.RegisterType(RZ_Mission_ClearSettlement);
		mgr.RegisterType(RZ_Mission_EvacWindow);

		// The Red Hand
		mgr.RegisterType(RZ_Mission_BanditDeadDrop);

		// Halcyon
		mgr.RegisterType(RZ_Mission_SampleHunt);
	}
}
