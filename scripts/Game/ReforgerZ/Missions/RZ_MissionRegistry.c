//! Every built-in mission type, grouped by faction. Other mods can add their
//! own through RZ_MissionManager.RegisterType(). RZ_Mission_TurnedPlayer is not
//! listed: the game mode starts it directly when a player turns. Neither is
//! RZ_Mission_CarKeys: a Mechanic wanderer starts it.
class RZ_MissionRegistry
{
	//------------------------------------------------------------------------------------------------
	static void RegisterAll(RZ_MissionManager mgr)
	{
		// The Haven
		mgr.RegisterType(RZ_Mission_SupplyCache);
		mgr.RegisterType(RZ_Mission_SurvivorHoldout);
		mgr.RegisterType(RZ_Mission_RadioRelay);
		mgr.RegisterType(RZ_Mission_MedicineRun);
		mgr.RegisterType(RZ_Mission_BittenFriend);
		mgr.RegisterType(RZ_Mission_TakeBackTown);
		mgr.RegisterType(RZ_Mission_MissingScout);
		mgr.RegisterType(RZ_Mission_ClearTheRoad);
		mgr.RegisterType(RZ_Mission_BurnTheNest);
		mgr.RegisterType(RZ_Mission_SignalFires);
		mgr.RegisterType(RZ_Mission_TraderCaravan);
		mgr.RegisterType(RZ_Mission_RescueTheKids);
		// Built on the mission building blocks (Missions/Haven/RZ_HavenContracts.c)
		mgr.RegisterType(RZ_Mission_FuelForTheLights);
		mgr.RegisterType(RZ_Mission_HarvestGuard);
		mgr.RegisterType(RZ_Mission_RefugeeColumn);
		mgr.RegisterType(RZ_Mission_WaterPurifier);
		mgr.RegisterType(RZ_Mission_FortifySafehouse);
		mgr.RegisterType(RZ_Mission_TheInformant);
		mgr.RegisterType(RZ_Mission_RecoverTheBus);
		mgr.RegisterType(RZ_Mission_Memorial);

		// Haven house contracts (Missions/Haven/RZ_HavenHouseMissions.c). The Courier's
		// job (RZ_Mission_CourierJob) isn't listed: a Courier wanderer starts it.
		mgr.RegisterType(RZ_Mission_WalkHimOut);
		mgr.RegisterType(RZ_Mission_TheLedger);
		mgr.RegisterType(RZ_Mission_QuietArrangement);

		// Task Force Bastion
		mgr.RegisterType(RZ_Mission_ClearSettlement);
		mgr.RegisterType(RZ_Mission_LastHelicopter);
		mgr.RegisterType(RZ_Mission_BurnTheBodies);
		mgr.RegisterType(RZ_Mission_Recon);
		mgr.RegisterType(RZ_Mission_CheckpointDuty);
		mgr.RegisterType(RZ_Mission_MissingPatrol);
		mgr.RegisterType(RZ_Mission_CrashSite);
		mgr.RegisterType(RZ_Mission_QuarantineLine);
		mgr.RegisterType(RZ_Mission_RaidRedHand);
		mgr.RegisterType(RZ_Mission_Requisition);
		// Built on the mission building blocks (Missions/Bastion/RZ_BastionContracts.c)
		mgr.RegisterType(RZ_Mission_RestoreComms);
		mgr.RegisterType(RZ_Mission_CurfewPatrol);
		mgr.RegisterType(RZ_Mission_WeaponsCache);
		mgr.RegisterType(RZ_Mission_ConvoyEscort);
		mgr.RegisterType(RZ_Mission_RescueThePilot);
		mgr.RegisterType(RZ_Mission_ClearTheAirfield);
		mgr.RegisterType(RZ_Mission_HuntTheDeserter);
		mgr.RegisterType(RZ_Mission_SecureTheShipment);
		mgr.RegisterType(RZ_Mission_BlowTheBridge);
		mgr.RegisterType(RZ_Mission_FireMission);
		// Bastion's hard edges (Missions/Bastion/RZ_BastionLoreMissions.c)
		mgr.RegisterType(RZ_Mission_NoReply);
		mgr.RegisterType(RZ_Mission_StruckOff);

		// The Red Hand
		mgr.RegisterType(RZ_Mission_BanditDeadDrop);
		mgr.RegisterType(RZ_Mission_FenceTheGoods);
		mgr.RegisterType(RZ_Mission_PharmacyHeist);
		mgr.RegisterType(RZ_Mission_FightPit);
		mgr.RegisterType(RZ_Mission_TollRoad);
		mgr.RegisterType(RZ_Mission_SmugglersRun);
		mgr.RegisterType(RZ_Mission_RaidTheCache);
		mgr.RegisterType(RZ_Mission_ZombieBait);
		mgr.RegisterType(RZ_Mission_Arson);
		mgr.RegisterType(RZ_Mission_SabotageRelay);
		mgr.RegisterType(RZ_Mission_StealAntidotes);
		// Built on the mission building blocks (Missions/RedHand/RZ_RedHandContracts.c)
		mgr.RegisterType(RZ_Mission_CarTheft);
		mgr.RegisterType(RZ_Mission_LootTheArmory);
		mgr.RegisterType(RZ_Mission_ProtectionRacket);
		mgr.RegisterType(RZ_Mission_HijackTheConvoy);
		mgr.RegisterType(RZ_Mission_BustOut);
		mgr.RegisterType(RZ_Mission_DoubleAgent);
		mgr.RegisterType(RZ_Mission_CollectTheBounty);
		mgr.RegisterType(RZ_Mission_KillTheOfficer);
		mgr.RegisterType(RZ_Mission_TakeTheLab);
		// The Red Hand's hard edges (Missions/RedHand/RZ_RedHandLoreMissions.c); Tied Down is the Haven's
		mgr.RegisterType(RZ_Mission_LearningTheDrones);
		mgr.RegisterType(RZ_Mission_PitsDebtor);
		mgr.RegisterType(RZ_Mission_TiedDown);

		// Halcyon
		mgr.RegisterType(RZ_Mission_SampleHunt);
		mgr.RegisterType(RZ_Mission_FieldSamples);
		mgr.RegisterType(RZ_Mission_WaterSurvey);
		mgr.RegisterType(RZ_Mission_LostColleague);
		mgr.RegisterType(RZ_Mission_ResearchData);
		mgr.RegisterType(RZ_Mission_HuntScreamer);
		mgr.RegisterType(RZ_Mission_QuarantineBreach);
		mgr.RegisterType(RZ_Mission_Decontamination);
		mgr.RegisterType(RZ_Mission_LabDefence);
		// Built on the mission building blocks (Missions/Halcyon/RZ_HalcyonContracts.c)
		mgr.RegisterType(RZ_Mission_PowerTheLab);
		mgr.RegisterType(RZ_Mission_InstallSensors);
		mgr.RegisterType(RZ_Mission_BloodDrive);
		mgr.RegisterType(RZ_Mission_ColdChain);
		mgr.RegisterType(RZ_Mission_EscortTheScientist);
		mgr.RegisterType(RZ_Mission_SpecimenFromTheNest);
		mgr.RegisterType(RZ_Mission_MedicalDrone);
		mgr.RegisterType(RZ_Mission_PatientZero);
		mgr.RegisterType(RZ_Mission_LiveCapture);
		mgr.RegisterType(RZ_Mission_SerumTrial);
		mgr.RegisterType(RZ_Mission_MobileLab);

		// Hard edges as contracts (Missions/Halcyon/RZ_HalcyonLoreMissions.c, Missions/Haven/RZ_HavenLoreMissions.c)
		mgr.RegisterType(RZ_Mission_TheThreeWeeks);
		mgr.RegisterType(RZ_Mission_TheAuthorList);
		mgr.RegisterType(RZ_Mission_TheList);

		// Puzzle missions (Puzzles/), each a contract from one faction
		mgr.RegisterType(RZ_Mission_NumbersStation);
		mgr.RegisterType(RZ_Mission_TwoBearings);
		mgr.RegisterType(RZ_Mission_PacesFromTheTower);
		mgr.RegisterType(RZ_Mission_HotAndCold);
		mgr.RegisterType(RZ_Mission_TheRat);
		mgr.RegisterType(RZ_Mission_PatientZeroLedger);
		mgr.RegisterType(RZ_Mission_PoisonedWell);
		mgr.RegisterType(RZ_Mission_SmugglersLedger);
		mgr.RegisterType(RZ_Mission_ClockworkHorde);
		mgr.RegisterType(RZ_Mission_TimeLockSafe);
		mgr.RegisterType(RZ_Mission_BellOrder);
		mgr.RegisterType(RZ_Mission_LightsInTheWindow);
		mgr.RegisterType(RZ_Mission_JunctionBoxes);
		mgr.RegisterType(RZ_Mission_WeighingVault);
		mgr.RegisterType(RZ_Mission_LastExperiment);
		mgr.RegisterType(RZ_Mission_SilentVillage);
		mgr.RegisterType(RZ_Mission_TheWitness);

		// Unarmed missions (Missions/Unarmed/): stealth, noise and thinking, small Fame
		mgr.RegisterType(RZ_Mission_PharmacyLure);
		mgr.RegisterType(RZ_Mission_SilentCourier);
		mgr.RegisterType(RZ_Mission_LeadThemAway);
		mgr.RegisterType(RZ_Mission_RooftopRescue);
		mgr.RegisterType(RZ_Mission_TrailOfClues);
		mgr.RegisterType(RZ_Mission_WaterRun);
		mgr.RegisterType(RZ_Mission_BarterChain);
		mgr.RegisterType(RZ_Mission_Distraction);
		mgr.RegisterType(RZ_Mission_MedicineBeforeDark);
		mgr.RegisterType(RZ_Mission_QuietCount);

		// Perimeter missions (Missions/Perimeter/)
		mgr.RegisterType(RZ_Mission_BlindSpot);
		mgr.RegisterType(RZ_Mission_TheCrossing);
		mgr.RegisterType(RZ_Mission_ShotAtTheLine);

		// World events
		mgr.RegisterType(RZ_Event_CrashedHelicopter);
		mgr.RegisterType(RZ_Event_HordeMigration);
		mgr.RegisterType(RZ_Event_NightOfTheDead);
		mgr.RegisterType(RZ_Event_SirenTest);
		mgr.RegisterType(RZ_Event_Storm);
		mgr.RegisterType(RZ_Event_WanderingTrader);
		mgr.RegisterType(RZ_Event_BastionShelling);
		mgr.RegisterType(RZ_Event_MassGrave);
		mgr.RegisterType(RZ_Event_StashNote);
		// Events that change a town or a region (Missions/Events/RZ_WorldEventsTown.c)
		mgr.RegisterType(RZ_Event_FogBank);
		mgr.RegisterType(RZ_Event_PowerSurge);
		mgr.RegisterType(RZ_Event_GasStationFire);
		mgr.RegisterType(RZ_Event_BurningTown);
		mgr.RegisterType(RZ_Event_TunnelRats);
		// Events about people (Missions/Events/RZ_WorldEventsPeople.c)
		mgr.RegisterType(RZ_Event_DistressCall);
		mgr.RegisterType(RZ_Event_FakeDistress);
		mgr.RegisterType(RZ_Event_PrisonBreak);
		mgr.RegisterType(RZ_Event_TheSniper);
		mgr.RegisterType(RZ_Event_RivalSurvivors);
		mgr.RegisterType(RZ_Event_CollapseShelter);
		// Events with something to take (Missions/Events/RZ_WorldEventsLoot.c)
		mgr.RegisterType(RZ_Event_AbandonedConvoy);
		mgr.RegisterType(RZ_Event_RemnantCamp);
		mgr.RegisterType(RZ_Event_LockedBunker);
		mgr.RegisterType(RZ_Event_WeddingParty);
	}
}
