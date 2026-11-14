// Test-only (not part of the mod): the mission rules the static check can't see
// (docs/MISSION_RULES.md, tools/mission_rules.py):
//   - rule 2: what a step's words name stands there when you arrive. For each errand step done at a
//     work site, 10 s after the tester first stands at its place its prop must be up
//     (RZ_WorkSites.PropOf); else "RZT rules FAIL thing ...".
//   - rule 19: a hand-in is a reply picked on the talk screen (RZ_Errands.HandIn). Every errand
//     resolved after its step and before its way back is counted as by reply or by walking up; a
//     walk-up while the talk screen is on is "RZT rules FAIL walk-up ...".
// Rule 9 (a person's marker on their body) is RZT_PersonObj.c's.
// Lines: "RZT rules FAIL ..." and "RZT rules: things N checked, N missing; hand-ins N, by reply N, by walking up N".

class RZT_Rules
{
	static int s_iThings;
	static int s_iMissing;
	static int s_iHandIns;
	static int s_iByReply;
	static int s_iWalkUps;
	static ref map<int, float> s_mArrived = new map<int, float>();	// errand id -> world time (ms) the tester first stood at its step
	static ref set<int> s_aChecked = new set<int>();
	static ref set<int> s_aReplied = new set<int>();
	static const float WAIT_MS = 10000;

	//------------------------------------------------------------------------------------------------
	static string Summary()
	{
		return string.Format("things %1 checked, %2 missing; hand-ins %3, by reply %4, by walking up %5",
			s_iThings, s_iMissing, s_iHandIns, s_iByReply, s_iWalkUps);
	}
}

modded class RZ_QuestTracker
{
	//------------------------------------------------------------------------------------------------
	override protected void ErrandObjectives(int playerId, int errandId, RZ_ObjList list)
	{
		super.ErrandObjectives(playerId, errandId, list);
		if (RZT_Rules.s_aChecked.Contains(errandId) || !m_GM.GetErrands())
			return;

		RZ_Errand e = m_GM.GetErrands().FindById(errandId);
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!e || e.m_bStepDone || !sites || !sites.Has(RZ_Errands.WorkKey(e)))
			return;

		RZ_ErrandWay w = e.GetWay();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!w || !me || !w.IsAtPlace(me, e.m_vPlace))
			return;

		float now = GetGame().GetWorld().GetWorldTime();
		float since;
		if (!RZT_Rules.s_mArrived.Find(errandId, since))
		{
			RZT_Rules.s_mArrived.Set(errandId, now);
			return;
		}

		if (now - since < RZT_Rules.WAIT_MS)
			return;

		RZT_Rules.s_aChecked.Insert(errandId);
		RZT_Rules.s_iThings++;
		if (!sites.PropOf(RZ_Errands.WorkKey(e)))
		{
			RZT_Rules.s_iMissing++;
			Print(string.Format("RZT rules FAIL thing: errand %1 '%2' at %3: nothing stands at its work site 10 s after arriving",
				errandId, e.Fill(w.StepText()), RZ_SettlementScanner.GridReference(e.m_vPlace)));
		}

		Print("RZT rules (so far): " + RZT_Rules.Summary());
	}
}

modded class RZ_Errands
{
	//------------------------------------------------------------------------------------------------
	override protected void HandIn(RZ_Errand e, RZ_ErrandWay w, int playerId)
	{
		RZT_Rules.s_aReplied.Insert(e.m_iId);
		super.HandIn(e, w, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void Resolve(RZ_Errand e, RZ_ErrandWay w)
	{
		if (e.m_eState != RZ_EErrandState.DONE && w && w.HasStep() && e.m_bStepDone && !w.m_bEndsThere)
		{
			RZT_Rules.s_iHandIns++;
			if (RZT_Rules.s_aReplied.Contains(e.m_iId))
			{
				RZT_Rules.s_iByReply++;
			}
			else
			{
				RZT_Rules.s_iWalkUps++;
				if (RZ_Dialogue.IsOn())
					Print(string.Format("RZT rules FAIL walk-up: errand %1 '%2' was handed in by walking up to %3 (handed %4), not by a reply",
						e.m_iId, e.Fill(e.m_Def.m_sTitle), e.m_sGiver, e.m_Def.m_bHanded));
			}

			Print("RZT rules (so far): " + RZT_Rules.Summary());
		}

		super.Resolve(e, w);
	}
}

//------------------------------------------------------------------------------------------------
//! The rules check's own errand (test-only): a named person's ask done at a work site near them,
//! then handed in on their talk, so the check counts a thing and a hand-in every run.
class RZT_RulesDef : RZ_ErrandDef
{
	//------------------------------------------------------------------------------------------------
	void RZT_RulesDef()
	{
		m_sKey = "rzt.rules";
		m_sTitle = "Tins for the Shelf";
		m_sFamily = "rzt";
		Over("Never mind.");
		RZ_ErrandWay w = Way("do");
		w.m_ePlace = RZ_EErrandPlace.HERE;
		w.m_fSeconds = 8;
		w.m_fRadius = 6;
		w.m_sStep = "Sort the tins in the crate at {place}, then tell {giver}.";
		w.Work(RZ_EWork.HANDS, "Sort the tins", 8);
		w.m_eProp = RZ_EErrandProp.CRATE;
		w.Take("The tins are in a heap. Sort them for me.", "Sort the tins, would you?");
		w.Done("Sorted. Thank you.", "Good. That's the shelf done.");
	}
}

modded class RZ_Errands
{
	//------------------------------------------------------------------------------------------------
	//! A taken errand from a real named person (key), its step at `place`, as Hand does for a handed
	//! one: the work site and its prop put out, the giver's take line, the tracker.
	RZ_Errand RZT_Give(RZ_ErrandDef d, int playerId, string giverKey, vector place)
	{
		RZ_NamedPerson giver = m_People.Get(giverKey);
		if (!giver || d.m_aWays.IsEmpty())
			return null;

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		RZ_ErrandWay w = d.m_aWays[0];
		RZ_Errand e = new RZ_Errand();
		e.m_Def = d;
		e.m_iAsker = playerId;
		e.m_sIdent = profile.m_sIdentity;
		e.m_iLife = profile.m_iCharacter;
		e.m_sGiver = giverKey;
		e.m_sGiverName = giver.m_sName;
		e.m_sGiverTitle = giver.m_sTitle;
		e.m_iFaction = giver.m_iFaction;
		if (!RZ_Factions.IsValid(e.m_iFaction))
			e.m_iFaction = RZ_EFaction.SURVIVORS;

		e.m_vGiver = giver.m_vPos;
		e.m_vOffer = giver.m_vPos;
		e.m_iSettlement = m_GM.FindNearestSettlement(giver.m_vPos);
		e.SetText("giver", giver.m_sName);
		RZ_Settlement town = m_GM.GetSettlement(e.m_iSettlement);
		if (town)
			e.SetText("here", town.m_sName);

		e.SetText("place", "the step");
		e.m_iId = m_iNextId;
		m_iNextId++;
		e.m_fOfferedAt = m_fClock;
		e.m_iWay = 0;
		e.m_fTakenAt = m_fClock;
		e.m_bStepDone = false;
		e.m_eState = RZ_EErrandState.HELD;
		e.m_vPlace = place;
		m_aErrands.Insert(e);
		PlaceProp(e, w);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! The talk's hand-in reply ("[Errand] It's done: ...", Ask: HandIn), as the talk screen calls it.
	void RZT_ReplyHandIn(RZ_Errand e, int playerId)
	{
		RZ_ErrandWay w = e.GetWay();
		if (w)
			HandIn(e, w, playerId);
	}
}
