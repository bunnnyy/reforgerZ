//! The packs' world events (docs/MISSIONS.md, "World events"; docs/INFECTED.md, "People"). Every
//! Ridge Calls replaces Night of the Dead, The Tribe Moves replaces Horde Migration, and Red Dusk is
//! new. With the tribes off, or no pack on the map, the first two keep the old behaviour. The Vigil,
//! the Clash and Border Nights are the life cycle's own (Zombies/RZ_TribeLife.c, RZ_TribeBorders.c);
//! Furious is a pack's state after its leader is caught alive (RZ_TribePeople.MakeFurious).

//------------------------------------------------------------------------------------------------
//! #3 Every Ridge Calls (Night of the Dead, reworked): a frenzy night everywhere. Announced at dusk;
//! all night every pack hunts three trails with its range doubled, and the commitment rule still
//! holds, so it's many hunts, not one endless one. For its first 20 minutes the towns hold twice the
//! bodies and specials, as before. One frenzy at a time, none in the first 2 season days, one every
//! 3 at most. With no packs: the 20 minutes only, at any hour.
class RZ_Event_EveryRidgeCalls : RZ_Event_Timed
{
	protected bool m_bFrenzy;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_bGlobal = true;
		m_fTimeLeft = 1200;
		m_fParticipationRadius = 0;
		m_Req.m_iMinStage = RZ_EOutbreakStage.COLLAPSE;
	}

	//------------------------------------------------------------------------------------------------
	//! With packs on the map it's announced at dusk only.
	protected static bool PacksOn(RZ_GameModeComponent gm)
	{
		RZ_TribePeople people = RZ_TribeContract.People(gm);
		return people && !gm.GetTribes().GetTribeList().IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!PacksOn(gm))
			return 0.5;

		float h = RZ_GameTime.GetHour();
		if (h < 18.5 || h >= 20.5 || !gm.GetTribes().GetPeople().CanFrenzy())
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		gm.GetSpawner().SetEventModifiers(2, 2);
		m_sTitle = "Every Ridge Calls";
		if (PacksOn(gm))
		{
			m_bFrenzy = true;
			gm.GetTribes().GetPeople().Frenzy(vector.Zero, 0, 2, false);
			m_sBrief = "Every ridge is calling tonight, and they're answering each other all down the valley. Whatever's out there is coming further than it ever has. Get indoors and stay there till light.";
			m_sObjective = "Get indoors and stay there till first light.";
			return true;
		}

		m_sBrief = "Something has stirred them up. For the next 20 minutes there are twice as many infected, and the changed ones are everywhere. Find somewhere safe.";
		m_sObjective = "Find shelter for the next 20 minutes.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void Restore(RZ_GameModeComponent gm)
	{
		gm.GetSpawner().SetEventModifiers(1, 1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_bFrenzy)
			return "The towns have settled. The ridges haven't: they'll call till dawn.";

		return "It's over. They're settling down again.";
	}
}

//------------------------------------------------------------------------------------------------
//! #29 Red Dusk: at a clear dusk, at most once every 7 season days, from Collapse: the packs within
//! 3 km of one point frenzy (range x1.75, three trails, the nearest wire first). Simplified: the
//! engine's sky isn't changed; it runs only at a clear dusk.
class RZ_Event_RedDusk : RZ_Event_Timed
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 900;
		m_fParticipationRadius = 0;
		m_Req.m_iMinStage = RZ_EOutbreakStage.COLLAPSE;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		RZ_TribePeople people = RZ_TribeContract.People(gm);
		RZ_Environment env = gm.GetEnvironment();
		float h = RZ_GameTime.GetHour();
		if (!people || !people.CanRedDusk() || h < 18.5 || h >= 19.5 || (env && env.IsRaining()))
			return 0;

		return 0.4;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		if (!tribes || tribes.GetTribeList().IsEmpty())
			return false;

		RZ_Tribe t = tribes.GetTribeList().GetRandomElement();
		m_vPos = t.RangeCentre();
		if (tribes.GetPeople().Frenzy(m_vPos, 3000, 1.75, true) == 0)
			return false;

		m_sTitle = "Red Dusk";
		m_sBrief = string.Format("The sun's going down red over the hills by %1, and they're all calling. They'll come to the wire tonight.", t.m_sPlace);
		m_sObjective = string.Format("Stay clear of the hills by %1 after dark.", t.m_sPlace);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The red's gone out of the sky. The calling hasn't.";
	}
}

//------------------------------------------------------------------------------------------------
//! #2 The Tribe Moves (Horde Migration, reworked): a pack's lair moves out of its range, it's pushed
//! off, or a nomad moves on (RZ_TribeLife.MoveTribe starts it): it walks across the map as a herd,
//! shown as two groups within 600 m of a player. Lying low as it passes works. 10 or more kills leave
//! its old lair's finds to whoever follows it back. With no packs it's the old migration between
//! two towns. Its player title is "On the Move" (player text never says "tribe").
class RZ_Event_TribeMoves : RZ_Mission
{
	protected int m_iTribe = -1;
	protected vector m_vFrom;
	protected vector m_vTo;
	protected string m_sWas;
	protected int m_iKills;
	protected vector m_vLastKill;
	protected bool m_bShown;
	protected float m_fWalked;

	protected static const float WALK = 1.5;	// m/s as data

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 1500;
		m_fParticipationRadius = 250;
	}

	//------------------------------------------------------------------------------------------------
	//! A pack's move (before Setup): from its old lair to its new one.
	void SetMove(RZ_Tribe t, vector from, vector to, string was)
	{
		m_iTribe = t.m_iId;
		m_vFrom = from;
		m_vTo = to;
		m_sWas = was;
	}

	//------------------------------------------------------------------------------------------------
	//! Picked at random only with no packs (Collapse on); with packs, the life cycle starts it.
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_TribeContract.Tribes(gm) || gm.GetDirector().GetStage() < RZ_EOutbreakStage.COLLAPSE)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_sTitle = "On the Move";
		RZ_Tribes tribes = RZ_TribeContract.Tribes(gm);
		if (m_iTribe >= 0 && tribes)
		{
			RZ_Tribe t = tribes.Find(m_iTribe);
			if (!t)
				return false;

			m_vPos = m_vFrom;
			RZ_TribePeople people = tribes.GetPeople();
			m_sBrief = string.Format("%1 are moving %2, from %3 toward %4. Stay off their way, or make them pay for it.", RZ_Tribes.Capitalize(people.Name(t, RZ_EFaction.SURVIVORS)), RZ_Tribes.CompassWord(m_vFrom, m_vTo), m_sWas, t.m_sPlace);
			m_sObjective = string.Format("Keep off the way of %1, from %2 to %3.", people.Name(t, RZ_EFaction.SURVIVORS), m_sWas, t.m_sPlace);
			return true;
		}

		// No packs: the old migration from a bad town to another
		m_iSettlement = PickSettlement(gm, 50, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		int target = PickSettlementAway(gm, s.m_vCenter, 1500, 3500, 0, 100);
		RZ_Settlement dest = gm.GetSettlement(target);
		if (!dest)
			return false;

		m_vFrom = s.m_vCenter;
		m_vTo = dest.m_vCenter;
		m_vPos = m_vFrom;
		m_sBrief = string.Format("A great crowd of them is leaving %1 and heading for %2. Stay off that road, or make them pay for it.", s.m_sName, dest.m_sName);
		m_sObjective = string.Format("Keep off the road from %1 to %2.", s.m_sName, dest.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The herd walks as data; two groups appear when a player is within 600 m (passing through, not
	//! coming for anyone: stealth can lower its sight).
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		float total = RZ_Util.DistXZ(m_vFrom, m_vTo);
		if (!m_bShown)
		{
			m_fWalked = Math.Min(total, m_fWalked + WALK * dt);
			if (total > 1)
				m_vPos = RZ_Util.GroundPos(m_vFrom + (m_vTo - m_vFrom) * (m_fWalked / total));
		}

		if (!m_bShown && RZ_Util.NearestDistance(m_vPos, players) <= 600)
		{
			m_bShown = true;
			ZombieSpawner(gm).SpawnHordeAt(LandPointNear(m_vPos, 0, 50), m_vTo, m_iId, null, false);
			ZombieSpawner(gm).SpawnHordeAt(LandPointNear(m_vPos, 30, 80), m_vTo, m_iId, null, false);
		}

		if (!m_bShown && m_fWalked >= total)
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_ZombieSpawner ZombieSpawner(RZ_GameModeComponent gm)
	{
		return gm.GetSpawner();
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (missionId != m_iId)
			return;

		m_iKills += count;
		m_vLastKill = pos;
		m_vPos = pos;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	//! 10 or more kills: its old lair's finds (a crate where it lay up, or where the last one fell).
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		// A horde passed: the town nearest its way posts the reason (Survivors Trapped, Retake the Town)
		vector mid = (m_vFrom + m_vTo) * 0.5;
		int town = gm.FindNearestSettlement(mid);
		RZ_Settlement s = gm.GetSettlement(town);
		if (s && RZ_Util.DistXZ(s.m_vCenter, mid) <= 1500)
			gm.GetFacts().Memory().PostReason("horde.passed", RZ_WorldFacts.Town(town), string.Format("A horde passed through %1.", s.m_sName), s.m_vCenter, -1, 0.5);

		if (m_iKills < 10)
			return;

		if (m_iTribe >= 0)
			gm.SpawnRewardCrate(LandPointNear(m_vFrom, 2, 8));
		else
			gm.SpawnRewardCrate(m_vLastKill);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_iKills >= 10 && m_iTribe >= 0)
			return string.Format("They've moved on. Survivors put down %1 of them on the way, and whatever was left where they used to lie up is anyone's now.", m_iKills);

		return string.Format("They've moved on. Survivors put down %1 of them on the way.", m_iKills);
	}

	//------------------------------------------------------------------------------------------------
	//! Started by the life cycle when a pack moves out of its range.
	static void Start(RZ_GameModeComponent gm, RZ_Tribe t, vector from, vector to, string was)
	{
		RZ_MissionManager mm = gm.GetMissions();
		if (!mm || RZ_Util.DistXZ(from, to) < 500)
			return;

		RZ_Event_TribeMoves m = new RZ_Event_TribeMoves();
		m.Configure(gm);
		m.SetMove(t, from, to, was);
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		if (!ids.IsEmpty() && mm.CanRunNow(m, ids.Count(), false))
			mm.StartMission(m);
	}
}

//------------------------------------------------------------------------------------------------
//! The Gathering (docs/INFECTED.md, "The great one"): every 4 season days, and on frenzy nights, each
//! pack of the great one's clan sends a group along its way to the nearest lived-in place's wire, and
//! they wait there until dawn (Zombies/RZ_TribeGreat.c moves them; this event is what everyone is
//! told, and the wire's spot). The bodies stay inside the per-cluster shares, so it comes in waves.
//! 10 or more of them put down within 300 m of the wire: what they carried is left there at dawn.
//! Never picked at random.
class RZ_Event_Gathering : RZ_Mission
{
	protected int m_iKills;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 300;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Started by the great one at dusk (or on a frenzy night): from place, to town's wire at wire.
	static void Start(RZ_GameModeComponent gm, vector wire, string place, string town)
	{
		RZ_MissionManager mm = gm.GetMissions();
		if (!mm)
			return;

		RZ_Event_Gathering m = new RZ_Event_Gathering();
		m.Configure(gm);
		m.m_vPos = wire;
		m.m_sTitle = "The Gathering";
		m.m_sBrief = string.Format("Every ridge answered the call from %1 tonight, and now they're all coming down toward the wire at %2 together. Not one hunt: all of them, in waves, until first light.", place, town);
		m.m_sObjective = string.Format("Hold the wire at %1 until first light.", town);
		float secs = RZ_GameTime.RealSecondsUntil(5.5);
		if (secs > 0)
			m.m_fTimeLeft = Math.Clamp(secs, 600, 10800);

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		if (!ids.IsEmpty() && mm.CanRunNow(m, ids.Count(), false))
			mm.StartMission(m);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnZombiesKilled(RZ_GameModeComponent gm, int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		if (RZ_Util.DistXZ(pos, m_vPos) <= 300)
			m_iKills += count;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_iKills >= 10)
			gm.SpawnRewardCrate(LandPointNear(m_vPos, 5, 20));
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		if (m_iKills >= 10)
			return string.Format("First light. The wire held: %1 of them are lying in front of it, and what they had on them is still there.", m_iKills);

		return "First light. Whatever came down to the wire last night has gone back up into the hills.";
	}
}

//------------------------------------------------------------------------------------------------
//! The Night of Many Calls (docs/INFECTED.md, "The great one"): the dusk after the great one dies, every
//! border reopens for 2 counted nights (Zombies/RZ_TribeGreat.c); this event is what everyone is told.
//! Never picked at random.
class RZ_Event_NightOfManyCalls : RZ_Mission
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetEvent(false);
		m_bGlobal = true;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	static void Start(RZ_GameModeComponent gm)
	{
		RZ_MissionManager mm = gm.GetMissions();
		if (!mm)
			return;

		RZ_Event_NightOfManyCalls m = new RZ_Event_NightOfManyCalls();
		m.Configure(gm);
		float secs = RZ_GameTime.RealSecondsUntil(5);
		if (secs > 0)
			m.m_fTimeLeft = Math.Clamp(secs, 600, 10800);

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		if (!ids.IsEmpty() && mm.CanRunNow(m, ids.Count(), false))
			mm.StartMission(m);
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_sTitle = "The Night of Many Calls";
		m_sBrief = "Every ridge is calling at once tonight, and none of them is answering the same voice. Whatever held them together is gone. Stay away from the rivers and the bridges for a night or two.";
		m_sObjective = "Keep away from rivers and bridges tonight.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTimeUp(RZ_GameModeComponent gm)
	{
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The calling has thinned out. Most of them know whose ground is whose again.";
	}
}
