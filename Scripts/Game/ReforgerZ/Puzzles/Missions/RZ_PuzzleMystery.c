//! Puzzle missions: mysteries. See docs/MISSIONS.md, "Puzzle missions".

//------------------------------------------------------------------------------------------------
//! #16 The Silent Village (Haven): a village with no infected at all (spawns are
//! suppressed there for the mission). Notes, a locked cellar and a list of the
//! missing point to something under the church that draws people in. Once the
//! cellar is open, decide at the church:
//!  - Burn it: infection drops in the area; what's down there comes up burning (a fight).
//!  - Seal it: nothing happens, for now. Half the Haven Fame.
//!  - Let Halcyon study it: big research, Halcyon pays instead; half the time a pack
//!    of specials comes up the steps afterwards.
//! ★ Elder: more Fame (they knew this village).
class RZ_Mission_SilentVillage : RZ_PuzzleMission
{
	protected vector m_vChurch;
	protected vector m_vCellar;
	protected bool m_bCellarOpen;
	protected bool m_bFighting;
	protected float m_fFightGrace;
	protected int m_iCellarClue = -1;

	protected static const int CHOICE_BURN = 0;
	protected static const int CHOICE_SEAL = 1;
	protected static const int CHOICE_STUDY = 2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.TRUSTED, 20, 20);
		m_Req.Bonus("elder");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 3600;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	//! A small village if there is one free, otherwise any quiet settlement.
	protected int PickVillage(RZ_GameModeComponent gm)
	{
		array<int> villages = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (s.m_iSize == 1 && s.m_fInfection <= 70 && !gm.GetMissions().IsSettlementBusy(i))
				villages.Insert(i);
		}

		if (villages.IsEmpty())
			return PickSettlement(gm, 0, 60);

		return villages[m_Rng.Int(0, villages.Count())];
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickVillage(gm);
		RZ_Settlement v = gm.GetSettlement(m_iSettlement);
		if (!v)
			return false;

		m_vPos = v.m_vCenter;
		m_fParticipationRadius = v.m_fRadius + 100;

		array<string> churchKinds = {"church"};
		m_vChurch = LandPointNear(v.m_vCenter, 0, v.m_fRadius * 0.3);
		RZ_Landmark church = FindOrPlaceLandmark(gm, v.m_vCenter, v.m_fRadius, churchKinds, m_vChurch);
		string churchName = "the church";
		if (church)
		{
			m_vChurch = church.m_vPos;
			if (church.m_bSpawned)
				churchName = church.m_sName;
		}

		array<string> names = {};
		RZ_NoteText.DistinctFirstNames(m_Rng, 9, names);
		if (names.Count() < 9)
			return false;

		string priest = "Father " + RZ_NoteText.FirstName(m_Rng);
		int n1 = AddClue("A diary page", string.Format("%1, %2. %3 says the bell at %4 rang by itself last night. Nobody was pulling the rope. He went down to the crypt to look. That was this morning.\n%5",
			v.m_sName, RZ_NoteText.Day(m_Rng.Int(4, 8)), priest, churchName, RZ_NoteText.Signoff(m_Rng, names[0])));
		int n2 = AddClue("A letter, never sent", string.Format("The sick ones aren't turning here. They get up in the night and walk to %1 and go down the steps, quietly, like it's Mass. %2 followed her husband. I'm locking us in the cellar.\n%3",
			churchName, names[1], RZ_NoteText.Signoff(m_Rng, names[2])));
		int n3 = AddClue("A list nailed to a door", string.Format("MISSING: %1, %2, %3, %4, %5.\nThe children first. Then anyone who went looking.", names[3], names[4], names[5], names[6], names[7]));

		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(v.m_vCenter, v.m_fRadius * 0.2, v.m_fRadius * 0.7), n1, "Read the diary page");
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(v.m_vCenter, v.m_fRadius * 0.2, v.m_fRadius * 0.7), n2, "Read the letter");
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(v.m_vCenter, v.m_fRadius * 0.1, v.m_fRadius * 0.5), n3, "Read the list");

		m_iCellarClue = AddClue("The cellar", string.Format("Inside, %1, alive, barely, holding a kitchen knife. \"Don't go under %2. It isn't them down there. It's one thing, and it's using them. It hums. You can hear it in your teeth.\"", names[2], churchName));
		m_vCellar = LandPointNear(v.m_vCenter, v.m_fRadius * 0.3, v.m_fRadius * 0.8);
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vCellar, 0, "Force the cellar door", "");

		int steps = AddClue("The crypt steps", "Steps go down behind the altar. The air coming up is warm and sweet. Something below breathes, slowly, a lot of somethings, in time.");
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, RZ_Util.RandomPointInRing(m_vChurch, 3, 6), 1, "", "Burn it|Seal the steps|Call Halcyon to study it", steps, "Listen at the steps");

		gm.GetSpawner().SetSuppressed(m_iSettlement, true);

		m_sTitle = "The Silent Village";
		m_sBrief = string.Format("Nobody has seen a single infected in %1 for days. Not one. The Haven wants to know why before anyone moves in. Find out what happened there, then decide what to do about it.", v.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx != 0 || m_bCellarOpen)
			return;

		m_bCellarOpen = true;
		SetObjectLabels(0, "", "");
		gm.GetHeat().AddHeat(m_vCellar, 5);
		gm.NotifyPlayer(playerId, m_aClueTitles[m_iCellarClue], m_aClueTexts[m_iCellarClue]);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 1 || m_bFighting)
			return;

		if (!m_bCellarOpen)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You don't know what you'd be deciding about yet. Someone in this village locked themselves away. Find them first.");
			return;
		}

		RZ_Settlement v = gm.GetSettlement(m_iSettlement);
		if (option == CHOICE_BURN)
		{
			NotifyParticipants(gm, m_sTitle, "The fire catches on the old pews. Something under the floor screams, and they come up the steps burning.");
			if (v)
				v.AddInfection(-30);

			foreach (int i, RZ_Settlement other : gm.GetSettlements())
			{
				if (i != m_iSettlement && v && RZ_Util.DistXZ(other.m_vCenter, v.m_vCenter) < 1500)
					other.AddInfection(-10);
			}

			gm.GetSpawner().SetSuppressed(m_iSettlement, false);
			SpawnGuards(gm, m_vChurch, 5, 5, 20);
			SpawnGuards(gm, m_vChurch, 1, 5, 15, true);
			m_bFighting = true;
			m_fFightGrace = 10;
			SetObjectLabels(1, "", "");
			return;
		}

		if (option == CHOICE_SEAL)
		{
			m_fRewardScale = 0.5;
			NotifyParticipants(gm, m_sTitle, "You drag stones over the steps until the humming is only a feeling. Nothing happens. For now.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == CHOICE_STUDY)
		{
			m_eGiver = RZ_EFaction.INSTITUTE;
			gm.GetCure().AddResearch(6, "Halcyon is studying something found under a village church.");
			NotifyParticipants(gm, m_sTitle, "Halcyon's people arrive at dusk in masks, very quiet, very interested. They thank you and ask you to leave.");
			if (m_Rng.Chance(0.5))
			{
				SpawnGuards(gm, m_vChurch, 3, 5, 25, true);
				gm.GetSpawner().SpawnHorde(m_vChurch, 250, m_iId);
				gm.NotifyHorde(m_vChurch);
			}

			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bFighting)
			return;

		m_fFightGrace -= dt;
		if (m_fFightGrace > 0)
			return;

		if (gm.GetSpawner().CountForMission(m_iId) == 0)
		{
			NotifyParticipants(gm, m_sTitle, "The church burns down to its stones. The humming has stopped.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		gm.GetSpawner().SetSuppressed(m_iSettlement, false);
		super.Cleanup(gm);
	}
}

//------------------------------------------------------------------------------------------------
//! #17 The Witness (Haven): a Haven courier was killed on the road. A wanderer saw
//! it, but only talks at night at the spot where it happened. His testimony points
//! to a camp (a Red Hand AI group if that prefab is set, otherwise just their stash).
//! At the camp, decide: report it to the Haven (Fame) or sell what you know to the
//! Red Hand (Red Hand Fame, Haven Infamy). ★ Police: he talks to you by day too.
class RZ_Mission_TheWitness : RZ_PuzzleMission
{
	protected vector m_vSpot;
	protected vector m_vCamp;
	protected string m_sTestimony;
	protected string m_sCampName;
	protected int m_iWitness = -1;
	protected bool m_bHeard;
	protected IEntity m_Group;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 20, 15);
		m_Req.Bonus("police");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !IsNightSoon(1200))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vSpot = LandPointNear(s.m_vCenter, s.m_fRadius + 100, s.m_fRadius + 500);
		m_vPos = m_vSpot;

		array<int> used = {m_iSettlement};
		array<int> candidates = {};
		CollectSettlements(gm, m_vSpot, 600, 2500, used, candidates);
		if (candidates.IsEmpty())
			return false;

		RZ_Settlement y = gm.GetSettlement(candidates[0]);
		array<string> kinds = {"church", "watertower", "school", "station", "fuel", "tower", "monument"};
		RZ_Landmark mark = RZ_LandmarkFinder.FindNearest(y.m_vCenter, y.m_fRadius, kinds);
		if (mark)
		{
			m_vCamp = LandPointNear(mark.m_vPos, 20, 60);
			m_sCampName = string.Format("%1 in %2", mark.m_sName, y.m_sName);
		}
		else
		{
			float dir = m_Rng.Int(0, 8) * 45;
			m_vCamp = RZ_Compass.PointAt(y.m_vCenter, dir, y.m_fRadius + 80);
			m_sCampName = string.Format("the %1 edge of %2", RZ_Compass.DirectionWord(dir), y.m_sName);
		}

		string courier = RZ_NoteText.FirstName(m_Rng);
		int body = AddClue("The courier", string.Format("A Haven courier, %1. The satchel has been cut open and emptied. Someone tied a red rag around the strap, like a joke.", courier));
		SpawnClue(gm, RZ_EPuzzleProp.BODY, m_vSpot, body, "Look at the body");

		m_sTestimony = string.Format("He keeps his eyes on the dark road while he talks. \"Three of them. Red rags on their arms. She gave them the bag and they shot her anyway. They went off toward %1, laughing. I followed them as far as the last fence. I'm not going any closer, and neither should you.\"", m_sCampName);
		m_iWitness = AddClue("The wanderer", "");
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, RZ_Util.RandomPointInRing(m_vSpot, 5, 8), 0, "", "", m_iWitness, "Talk to the wanderer");

		int letters = AddClue("The courier's letters", string.Format("In a crate under a tarp: the Haven post, still sealed. Letters from safehouses to families in other towns. %1 died for these.", courier));
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vCamp, 1, "", "Radio the Haven|Sell what you know to the Red Hand", letters, "Search the crate");

		ResourceName groupPrefab = gm.GetRedHandGroupPrefab();
		if (!groupPrefab.IsEmpty())
		{
			m_Group = RZ_Util.SpawnPrefab(groupPrefab, RZ_Util.RandomPointInRing(m_vCamp, 8, 15));
			TrackProp(m_Group);
		}

		m_fTimeLeft = NightDuration(2400, 10800, 4800);
		m_sTitle = "The Witness";
		m_sBrief = string.Format("A Haven courier was killed on the road outside %1. A wanderer saw it happen, they say, but he only talks after dark, and only where she died. Find out who did it.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		if (idx != m_iWitness)
			return super.GetClueText(gm, playerId, idx);

		if (RZ_GameTime.IsNight() || HasBonus(gm, playerId))
		{
			m_bHeard = true;
			return m_sTestimony;
		}

		return "He won't look at you. \"Not in daylight. They watch the road in daylight. Come back when it's dark.\"";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 1)
			return;

		if (!m_bHeard)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "A crate of letters. You don't know whose they are or how they got here. Somebody out there does.");
			return;
		}

		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		if (option == 0)
		{
			if (m_Group)
				SetRival(RZ_EFaction.BANDITS, 10);

			gm.NotifyPlayer(playerId, m_sTitle, "You read out the grid on the Haven channel. A long pause, then: \"Thank you. We'll come for the letters, and for her.\"");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			m_eGiver = RZ_EFaction.BANDITS;
			SetRival(RZ_EFaction.SURVIVORS, 15);
			gm.NotifyPlayer(playerId, m_sTitle, "The Red Hand pays well to know who's been talking, and who saw. The wanderer won't be on the road tomorrow.");
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Selling to the Red Hand is open to anyone the Red Hand deals with, not only Haven contractors.
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (m_eGiver == RZ_EFaction.BANDITS && m_eState != RZ_EMissionState.ACTIVE)
			return !RZ_Reputation.IsHunted(gm.GetTier(playerId, RZ_EFaction.BANDITS));

		return super.IsEligible(gm, playerId);
	}
}
