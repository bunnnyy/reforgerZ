//! World events: announced to everyone, open to everyone. See docs/MISSIONS.md, "World events".
//! Most pay in loot found on the spot rather than Fame. Nothing comes in from outside the Zone.

//------------------------------------------------------------------------------------------------
//! Base for events that change a world setting for a while and put it back afterwards.
class RZ_Event_Timed : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		Restore(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void Restore(RZ_GameModeComponent gm)
	{
	}
}

//------------------------------------------------------------------------------------------------
//! #8 Siren Test: air-raid sirens pull every infected into one town for 5 minutes.
//! The towns around it are briefly safer to loot.
class RZ_Event_SirenTest : RZ_Event_Timed
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 300;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		gm.GetSpawner().SetFocusSettlement(m_iSettlement);

		// Packs come only if they've learned the bells; the rest keep away
		if (gm.GetTribes())
			gm.GetTribes().GetPeople().BellsCome(m_vPos);

		m_sTitle = "Siren Test";
		m_sBrief = string.Format("The air-raid sirens in %1 have started wailing. Every infected around is heading there. Everywhere else is quieter for the next five minutes.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		gm.GetHeat().AddHeat(m_vPos, 3 * dt);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		gm.GetSpawner().SetFocusSettlement(-1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The sirens in " + GetLocationName(gm) + " have died down.";
	}
}

//------------------------------------------------------------------------------------------------
//! #20 Storm: for 15 minutes, noise carries less and heat fades three times as fast.
class RZ_Event_Storm : RZ_Event_Timed
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_bGlobal = true;
		m_fTimeLeft = 900;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		gm.GetHeat().SetEventModifiers(3, 0.6);
		m_sTitle = "Storm";
		m_sBrief = "A storm is rolling in. Wind and rain will cover your noise for the next 15 minutes. A good time to move.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		gm.GetHeat().SetEventModifiers(1, 1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The storm has passed. Sound carries again.";
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Wandering Trader: a neutral merchant parks somewhere for 20 minutes.
//! Needs the trader prefab (e.g. a Shop System trader).
class RZ_Event_WanderingTrader : RZ_Event_Timed
{
	protected IEntity m_Trader;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1200;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetTraderPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 50);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.8, s.m_fRadius + 100);
		m_Trader = RZ_Util.SpawnPrefab(gm.GetTraderPrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Trader)
			return false;

		m_sTitle = "Wandering Trader";
		m_sBrief = string.Format("A trader has parked outside %1 for the next 20 minutes. Rare goods. Protect him, or rob him.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Trader);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The trader has packed up and moved on.";
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Bastion Shelling: Bastion's own mortar crews shell an infested town after a
//! five-minute warning. Infection there drops sharply. Nothing comes from outside the
//! Zone: only while Bastion is still around. Needs the explosion (mortar impact) prefab.
class RZ_Event_BastionShelling : RZ_Mission
{
	protected bool m_bWarned;

	protected static const float SAFE_DISTANCE = 300;	// m from where people live to the nearest shell

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 300;
		m_fParticipationRadius = 0;
		m_Req.m_fMinMilitary = 0.2;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (gm.GetExplosionPrefab().IsEmpty())
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		// Never where people live: no town with a safehouse, a holding or the lab within SAFE_DISTANCE
		// of where the shells fall. Filtered first, so a map where every town in range is lived in
		// just has none
		array<int> towns = {};
		RZ_WorldFacts facts = gm.GetFacts();
		foreach (int i, RZ_Settlement t : gm.GetSettlements())
		{
			if (t.m_fInfection < 60 || t.m_bHasLab || gm.GetMissions().IsSettlementBusy(i, this))
				continue;

			if (facts.IsNearLivedIn(t.m_vCenter, t.m_fRadius * 0.8 + SAFE_DISTANCE))
				continue;

			towns.Insert(i);
		}

		if (towns.IsEmpty())
			return false;

		m_iSettlement = towns.GetRandomElement();
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		m_vPos = s.m_vCenter;
		m_sTitle = "Bastion Shelling";
		m_sBrief = string.Format("Bastion's mortar crews will shell %1 in five minutes. They don't check for survivors. Get out, or loot fast.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bWarned && m_fTimeLeft <= 60)
		{
			m_bWarned = true;
			gm.NotifyAll("Bastion Shelling", "One minute until the mortars open up on " + GetLocationName(gm) + ".");
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (s)
		{
			for (int i = 0; i < 6; i++)
			{
				RZ_Util.SpawnPrefab(gm.GetExplosionPrefab(), LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.8));
			}

			s.AddInfection(-40);
			gm.GetHeat().AddHeat(s.m_vCenter, 100);
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return GetLocationName(gm) + " has been shelled. Not much is moving there now.";
	}
}

//------------------------------------------------------------------------------------------------
//! #16 Mass Grave: a pit full of infected bodies. Lots of samples, and a horde
//! underneath that wakes when someone gets close.
class RZ_Event_MassGrave : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 60;
		m_Req.m_bNotCured = true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 40, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 300);
		gm.SpawnSamples(m_vPos, 3);
		m_sTitle = "Mass Grave";
		m_sBrief = string.Format("Someone found a mass grave outside %1. There are samples in it. Something underneath is still moving.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (CountPlayersNear(players, m_vPos, 30) == 0)
			return;

		SpawnGuards(gm, m_vPos, 4, 5, 15);
		gm.GetSpawner().SpawnHorde(m_vPos, 150);

		// The nearest pack has eaten here (docs/INFECTED.md, the changed missions)
		if (gm.GetTribes())
			gm.GetTribes().GetPeople().FeedNearest(m_vPos, 3000, 20);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The mass grave near " + GetLocationName(gm) + " has woken up.";
	}
}

//------------------------------------------------------------------------------------------------
//! #15 Stash Note: a note with coordinates turns up. Treasure hunt.
class RZ_Event_StashNote : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1800;
		m_fParticipationRadius = 15;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius + 200);
		gm.SpawnRewardCrate(m_vPos);
		m_sTitle = "Stash Note";
		m_sBrief = string.Format("A note was found on a body: 'Buried the good stuff at %1. Don't tell anyone.' Too late for that.", RZ_SettlementScanner.GridReference(m_vPos));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (CountPlayersNear(players, m_vPos, 10) > 0)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Someone found the stash near " + GetLocationName(gm) + ".";
	}
}

//------------------------------------------------------------------------------------------------
//! #24 Turned Player: a player who turned roams with their gear. Started by the
//! game mode when someone turns (never picked at random). Their body and loot are
//! where they fell; the turned zombie is marked as the event.
class RZ_Mission_TurnedPlayer : RZ_Mission_KillTagged
{
	protected string m_sName;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1200;
		m_fParticipationRadius = 60;
	}

	//------------------------------------------------------------------------------------------------
	void Prepare(string playerName, vector pos)
	{
		m_sName = playerName;
		m_vPos = pos;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_iSettlement = -1;
		if (!gm.GetSpawner().SpawnZombie(m_vPos, false, -1, m_iId))
			return false;

		m_bSpawned = true;
		m_sTitle = "Turned: " + m_sName;
		m_sBrief = string.Format("%1 has turned, still carrying everything they had. Whoever reaches them first decides what happens to it.", m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetLocationName(RZ_GameModeComponent gm)
	{
		return RZ_SettlementScanner.GridReference(m_vPos);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return m_sName + " has been put to rest.";
	}
}
