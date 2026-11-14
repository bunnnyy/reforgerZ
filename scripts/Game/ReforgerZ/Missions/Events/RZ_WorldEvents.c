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
		m_sObjective = string.Format("Keep away from the sirens in %1 for five minutes.", s.m_sName);
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
		m_sObjective = "Move while the storm covers your noise.";
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
		return "The storm has passed. Sound carries again, and so does every footstep.";
	}
}

//------------------------------------------------------------------------------------------------
//! #9 Wandering Trader: a neutral merchant parks somewhere for 20 minutes.
//! Needs the trader prefab (e.g. a Shop System trader).
//! A story job (key `trader`, kept per town): honest goods, as they said; a dead house's stores (a
//! house within 6 km: its name on the crates); or he sells the roads (who passed, to a crew within 6
//! km). His brother on the wagon (the talk scene): stolen: send the crates home (the house +3 food, 8
//! Haven Favor, he leaves early, a Haven asset); roads: tell the house he's selling them (Haven morale
//! +1, +4 Red Hand Infamy, he leaves) or two tins for his silence about you (nothing else changes).
class RZ_Event_WanderingTrader : RZ_Event_Timed
{
	protected IEntity m_Trader;
	protected int m_iHouse = -1;
	protected string m_sCrew = "a crew";

	protected static const int WAY_HOME = 0;
	protected static const int WAY_TELL = 1;
	protected static const int WAY_QUIET = 2;

	//------------------------------------------------------------------------------------------------
	void RZ_Event_WanderingTrader()
	{
		m_Story = new RZ_MissionStory("trader");
		m_Job = new RZ_StoryJob();
	}

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
		m_sBrief = string.Format("A wandering trader has pulled his wagon up outside %1 for the next 20 minutes, with things nobody has seen on a shelf since the gates closed. Everyone on the open channel knows he's there. Protect him, or rob him.", s.m_sName);
		m_sObjective = string.Format("Protect or rob the trader outside %1.", s.m_sName);
		Truth(gm);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Truth(RZ_GameModeComponent gm)
	{
		array<string> values = {"honest", "stolen", "roads"};
		array<float> weights = {0.5, 0, 0};
		if (gm.GetSafehouses())
			m_iHouse = gm.GetSafehouses().FindNearest(m_vPos, 6000);

		if (m_iHouse >= 0)
			weights[1] = 0.3;

		RZ_Holding crew = RZ_ErrandCard.HoldingNear(gm, RZ_EFaction.BANDITS, m_vPos, 6000);
		if (crew)
		{
			weights[2] = 0.2;
			m_sCrew = crew.m_sName;
		}

		string truth = m_Job.Draw(this, RZ_WorldFacts.Town(m_iSettlement), values, weights);
		string house = "a house";
		if (m_iHouse >= 0)
			house = gm.GetSafehouses().Get(m_iHouse).m_sName;

		m_Job.m_vReadAt = m_vPos;
		m_Job.m_fReadRange = 20;
		m_Job.Read("stolen", "elder", string.Format("The crates are stencilled with a house's name: %1's. That house had stores, once.", house));
		m_Job.Read("stolen", "labtech", "His price list has tins at a quarter of what they're worth. Goods you didn't pay for are cheap to sell.");
		m_Job.Read("roads", "excon,raider", string.Format("A notebook in his coat with times and number plates. %1 buys that kind of list.", m_sCrew));

		string reveal = "Bought, swapped, found. Mostly found. Nobody minds where.";
		if (truth == "stolen")
			reveal = string.Format("Off a cart outside %1. The people who owned it weren't using it any more. Not in the way you'd think.", house);
		else if (truth == "roads")
			reveal = "He writes down who passes. Somebody pays for that. Don't look at his notebook, he's shy.";

		RZ_TalkScene sc = m_Job.Scene("The trader's brother", "trader.wife", LandPointNear(m_vPos, 4, 7));
		int n0 = sc.Node("A man sits on the wagon's step, shelling peas. \"Everything's half price. The other half's what it costs you.\"");
		int n1 = sc.Node("So you know. Now what?");
		sc.Ask(n0, "Where's it all from?", n1, reveal, true);
		if (truth == "stolen" && m_iHouse >= 0)
			sc.Decide(n1, "Send the crates home", "send the trader's crates back to the house", WAY_HOME, "He sighs and starts loading the crates back. \"He'll sulk for a week.\"", RZ_EFaction.SURVIVORS);
		else if (truth == "roads")
		{
			sc.Decide(n1, "Tell the house he sells roads", "tell the house the trader sells the roads", WAY_TELL, "\"Then we'll be going.\" He whistles for him and the wagon's moving in a minute.");
			sc.Way(n1, "Two tins and I wasn't here", RZ_TalkScene.FOOD, -1, 2, WAY_QUIET, "He tears a page out of his notebook and gives it to you. Your page.");
		}
		else
			sc.Ask(n1, "Where are you headed?", -1, "Wherever the road's least shot at. It moves about.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnStoryWay(RZ_GameModeComponent gm, int playerId, int way)
	{
		array<int> who = {playerId};
		string town = RZ_WorldFacts.Town(m_iSettlement);
		if (way == WAY_QUIET)
		{
			RZ_StoryKit.Record(this, "fate", town, "quiet", who);
			return;
		}

		if (way == WAY_HOME)
		{
			gm.GetSafehouses().AddFood(m_iHouse, 3);
			RZ_StoryKit.PayDecider(gm, this, playerId, RZ_EFaction.SURVIVORS, 8);
			RZ_StoryKit.Record(this, "fate", town, "home", who);
			RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
			RZ_StoryKit.Later(gm, playerId, h.m_sName, "The crates are stacked by the house's door with the stencil facing out. Somebody's chalked 'HOME' under it.", h.m_vPos);
		}
		else
		{
			RZ_OneChoice.Morale(gm, RZ_EFaction.SURVIVORS, 1, "a trader who sold the roads, found out");
			RZ_StoryKit.BlameDecider(gm, this, playerId, RZ_EFaction.BANDITS, 4, "spoiled a crew's road list");
			RZ_StoryKit.Record(this, "fate", town, "told", who);
		}

		// He packs up early
		m_fTimeLeft = Math.Min(m_fTimeLeft, 5);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		RZ_Util.DeleteUnit(m_Trader);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The trader has packed up the wagon and moved on down the road, to a town that hasn't heard of the prices yet.";
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
		m_sBrief = string.Format("Bastion's mortar crews will shell %1 in five minutes: the town has gone over, and containment says it goes. The loudspeaker gave the warning once. They don't check for survivors after. Get out, or loot fast.", s.m_sName);
		m_sObjective = string.Format("Get out of %1 before the shells land.", s.m_sName);
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
		m_sBrief = string.Format("Someone found a mass grave outside %1: the first week's dead, from before anyone had time to dig them separate holes. Halcyon pays for samples from a site like that. Something underneath is still moving.", s.m_sName);
		m_sObjective = string.Format("Take samples from the mass grave outside %1.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (CountPlayersNear(players, m_vPos, 30) == 0)
			return;

		SpawnGuards(gm, m_vPos, 4, 5, 15);
		gm.GetSpawner().SpawnHorde(m_vPos, 150, m_iId);

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
		m_sObjective = string.Format("Dig up the stash at grid %1.", RZ_SettlementScanner.GridReference(m_vPos));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (RZ_Arrive.CountAtSpot(players, m_vPos, 10) > 0)
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
		m_sObjective = string.Format("Reach the turned %1 before anyone else does.", m_sName);
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
