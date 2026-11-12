//! One real choice (docs/DESIGN.md, "Story blocks"; the one-choice jobs in docs/MISSIONS.md): when a
//! contract's work is done, someone it concerns (a stand-in person, the puzzle civilian prefab) asks
//! one question before it ends. Options are its choice slots; picking one belongs to someone the job
//! was offered to who took part (RZ_StoryKit.MayDecide) and runs the decision's 5 s hold, which anyone
//! eligible may object to once. Nobody in 90 s, or no civilian prefab: the default. The mission says
//! what each option does in RZ_Mission.OnChosen. Server only.
class RZ_OneChoice
{
	static const int OBJECT_INDEX = 90;		// the person's object index (a mission's own objects stay below it)
	static const float ASK_SECONDS = 90;

	string m_sPrompt;		// what the person says, to the participants
	string m_sLabels;		// "A|B|C", the choice slots
	string m_sWhat;			// the hold's line: "decide where the food goes"
	int m_iDefault;			// taken when nobody chooses
	int m_iPicked = -1;		// what was chosen (the default if nobody did)
	int m_iDecider = -1;	// who chose, -1 nobody
	bool m_bAsking;
	bool m_bDone;
	bool m_bManual;			// the mission asks itself (mid-job), not when its work is done
	bool m_bEnds = true;	// settling it ends the job as done; false: OnChosen says what happens
	protected float m_fAsked;
	protected IEntity m_Person;
	protected ref array<int> m_aAsset = {};	// per option: the finale route its asset belongs to, -1 none

	//------------------------------------------------------------------------------------------------
	//! Option `option` gives the decider a route asset (RZ_RouteAssets) for `route`.
	void Asset(int option, int route)
	{
		while (m_aAsset.Count() <= option)
		{
			m_aAsset.Insert(-1);
		}

		m_aAsset[option] = route;
	}

	//------------------------------------------------------------------------------------------------
	protected int AssetOf(int option)
	{
		if (option < 0 || option >= m_aAsset.Count())
			return -1;

		return m_aAsset[option];
	}

	//------------------------------------------------------------------------------------------------
	void Set(string prompt, string labels, string what, int def)
	{
		m_sPrompt = prompt;
		m_sLabels = labels;
		m_sWhat = what;
		m_iDefault = def;
	}

	//------------------------------------------------------------------------------------------------
	//! The work is done: the person turns up beside `at` and asks. True while asking; false if it
	//! settled on the default at once (no prefab).
	bool Ask(RZ_GameModeComponent gm, RZ_Mission m, vector at)
	{
		if (m_bAsking || m_bDone || !m.AsksChoice())
			return m_bAsking;

		ResourceName prefab = gm.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);
		if (!prefab.IsEmpty())
			m_Person = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(at, 3, 6)));

		if (m_Person)
			RZ_PuzzleObjectComponent.Tag(m_Person, m.m_iId, OBJECT_INDEX, "Leave it to them|Object|Stand with", m_sLabels);

		if (!m_Person)
		{
			Settle(gm, m, -1, m_iDefault);
			return false;
		}

		m_bAsking = true;
		m_fAsked = 0;
		m.m_fTimeLeft = Math.Max(m.m_fTimeLeft, ASK_SECONDS + 60);
		m.NotifyParticipants(gm, m.m_sTitle, m_sPrompt);
		if (m_sLabels.Contains("["))
		{
			foreach (int part : m.m_aParticipants)
			{
				RZ_Guidance.Report(part, "tags");
			}
		}
		RZ_Log.Info(string.Format("Story: %1 asks (%2)", m.m_sTitle, m_sLabels));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! While asking: the default after 90 s. True once settled.
	bool Tick(RZ_GameModeComponent gm, RZ_Mission m, float dt)
	{
		if (!m_bAsking)
			return m_bDone;

		m_fAsked += dt;
		if (m_fAsked >= ASK_SECONDS && (!m.m_Story || m.m_Story.m_Deciding.Holder() < 0))
			Settle(gm, m, -1, m_iDefault);

		return m_bDone;
	}

	//------------------------------------------------------------------------------------------------
	//! A choice slot picked on the person: the decider rule, then the hold.
	void Handle(RZ_GameModeComponent gm, RZ_Mission m, int playerId, int option)
	{
		if (!m_bAsking || m_bDone || option < 0)
			return;

		if (!RZ_StoryKit.MayDecide(gm, m, playerId))
			return;

		// The clever way closes the honest one (Missions/Story/RZ_Grey.c)
		string closed = RZ_GreyChoice.Closed(gm, m, option);
		if (!closed.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m.m_sTitle, closed);
			return;
		}

		// After the lock: an option whose asset belongs to another route refuses a locked player
		string refusal = RZ_RouteAssets.Refuse(gm, playerId, AssetOf(option));
		if (!refusal.IsEmpty())
		{
			gm.NotifyPlayer(playerId, m.m_sTitle, refusal);
			return;
		}

		if (!m.m_Story)
		{
			Settle(gm, m, playerId, option);
			return;
		}

		m.m_Story.m_Deciding.StartHold(gm, m, playerId, option, m_sWhat);
	}

	//------------------------------------------------------------------------------------------------
	//! The hold ran out with nobody objecting.
	void HoldDone(RZ_GameModeComponent gm, RZ_Mission m, int decider, int option)
	{
		if (m_bAsking && !m_bDone)
			Settle(gm, m, decider, option);
	}

	//------------------------------------------------------------------------------------------------
	protected void Settle(RZ_GameModeComponent gm, RZ_Mission m, int decider, int option)
	{
		m_bAsking = false;
		m_bDone = true;
		m_iPicked = option;
		m_iDecider = decider;
		string who = "nobody";
		if (decider > 0)
			who = RZ_Util.PlayerName(decider);

		RZ_Log.Info(string.Format("Story: %1 chose %2 (by %3)", m.m_sTitle, option, who));
		if (decider > 0 && AssetOf(option) >= 0)
			RZ_RouteAssets.Add(gm, decider, AssetOf(option), m.m_sTitle, RZ_FinaleAssets.ALLY);

		m.OnChosen(gm, decider, option);
		// The shared layer: nobody chose, the reaction, the callback deed, the others' line
		RZ_GreyChoice.Settled(gm, m, this, decider, option);
		if (m_bEnds)
			m.m_eState = RZ_EMissionState.SUCCEEDED;

		RZ_Util.DeleteUnit(m_Person);
		m_Person = null;
	}

	//------------------------------------------------------------------------------------------------
	//! The players with the decider (their group within 150 m and those who stood with them), or the
	//! participants when nobody decided.
	void With(RZ_GameModeComponent gm, RZ_Mission m, notnull array<int> result)
	{
		if (m_iDecider < 0 || !m.m_Story)
		{
			result.InsertAll(m.m_aParticipants);
			return;
		}

		m.m_Story.m_Deciding.WithDecider(gm, m_iDecider, m.m_vPos, result);
	}

	//------------------------------------------------------------------------------------------------
	//! Favor from `faction` to those with the decider (or the participants), at once.
	void PayWith(RZ_GameModeComponent gm, RZ_Mission m, int faction, int favor)
	{
		array<int> with = {};
		With(gm, m, with);
		foreach (int playerId : with)
		{
			if (!RZ_Reputation.IsHunted(gm.GetTier(playerId, faction)))
				gm.AddFavor(playerId, faction, favor);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Infamy with `faction` for those with the decider (or the participants).
	void BlameWith(RZ_GameModeComponent gm, RZ_Mission m, int faction, int infamy, string why)
	{
		array<int> with = {};
		With(gm, m, with);
		foreach (int playerId : with)
		{
			gm.AddInfamy(playerId, faction, infamy, why);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's morale, through the errands' capped path when they're on.
	static void Morale(RZ_GameModeComponent gm, int f, int delta, string why)
	{
		if (gm.GetErrands())
		{
			gm.GetErrands().Morale(f, delta, why);
			return;
		}

		if (gm.GetFactionLife())
			gm.GetFactionLife().AddMorale(f, delta);
	}

	//------------------------------------------------------------------------------------------------
	//! The lived-in safehouse with the least food, -1 if none.
	static int HungriestHouse(RZ_GameModeComponent gm)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int best = -1;
		int lowest = 1000000;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (h && h.m_iPopulation > 0 && h.m_iFood < lowest)
			{
				lowest = h.m_iFood;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	void Cleanup()
	{
		RZ_Util.DeleteUnit(m_Person);
		m_Person = null;
	}
}

//------------------------------------------------------------------------------------------------
//! Where the horde goes (Blow the Bridge, Zombie Bait, Lead Them Away): one shared choice. Turned
//! isn't gone, and by morning they drift somewhere: an empty town (it grows worse there), past
//! Bastion's nearest post or past the Red Hand's nearest camp (that side's morale, and its Infamy to
//! those with the decider). Every answer is grey. A side with no holding within 3 km is an empty
//! town instead.
class RZ_HordeWhere
{
	static const int EMPTY = 0;
	static const int BASTION = 1;
	static const int REDHAND = 2;

	//------------------------------------------------------------------------------------------------
	static void Set(RZ_OneChoice c, int def)
	{
		c.Set("They're turned, not gone. By morning they'll drift somewhere, and the noise you make now decides where. Choose at the spot.",
			"An empty town|Past Bastion's post|Past a Red Hand camp", "decide where the horde drifts", def);
	}

	//------------------------------------------------------------------------------------------------
	//! What it did; returns which it really was (a side with no holding near is an empty town).
	static int Apply(RZ_GameModeComponent gm, RZ_Mission m, int option)
	{
		int faction = -1;
		if (option == BASTION)
			faction = RZ_EFaction.MILITARY;
		else if (option == REDHAND)
			faction = RZ_EFaction.BANDITS;

		RZ_Holding h = null;
		if (faction >= 0)
			h = RZ_ErrandCard.HoldingNear(gm, faction, m.m_vPos, 3000);

		if (!h)
		{
			int town = RZ_Mission.PickEmptySettlementAway(gm, m.m_vPos, 400, 3000, 0, 100);
			RZ_Settlement s = gm.GetSettlement(town);
			if (s)
			{
				s.AddInfection(5);
				m.NotifyParticipants(gm, m.m_sTitle, string.Format("They drift into %1 by morning. Nobody lives there. Nobody will for a while.", s.m_sName));
			}

			RZ_StoryKit.Record(m, "horde", RZ_WorldFacts.Town(m.m_iSettlement), "empty", m.m_aParticipants);
			return EMPTY;
		}

		RZ_OneChoice.Morale(gm, faction, -2, "a horde sent past");
		if (m.m_Choice)
			m.m_Choice.BlameWith(gm, m, faction, 4, "sent a horde past their post");

		m.NotifyParticipants(gm, m.m_sTitle, string.Format("They drift past %1 by morning. Somebody there has a long night, and knows which way the noise came from.", h.m_sName));
		string value = "bastion";
		if (option == REDHAND)
			value = "redhand";

		RZ_StoryKit.Record(m, "horde", RZ_WorldFacts.Town(m.m_iSettlement), value, m.m_aParticipants);
		return option;
	}
}
