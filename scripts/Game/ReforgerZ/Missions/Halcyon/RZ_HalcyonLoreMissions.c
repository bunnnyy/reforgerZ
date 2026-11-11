//! Halcyon's hard edge as a contract (docs/LORE.md, "Halcyon"; docs/MISSIONS.md, "Halcyon" #21):
//! The Author List (a failed trial volunteer's name, carried back from the Haven). The Three Weeks
//! is a questline now (Missions/Questlines/Halcyon/RZ_Quest_ThreeWeeks.c).

//------------------------------------------------------------------------------------------------
//! #22 The Author List: a volunteer who didn't walk out of the trial dose is on the list of
//! authors by number. Halcyon gives the number, the day and the safehouse they came from. At the
//! safehouse, the trial list by the door names who went up that day, and the man who walked them
//! up says who came back. The one who went up that day and didn't come back is the name: give
//! it to the trial lead at the lab. Wrong: she checks the consent form's initials and waits five
//! minutes; twice wrong: fails. Small Halcyon Fame, a little Haven Fame. The volunteer is
//! generated (the trial dose keeps no record of who it failed). ★ Elder: reads the council's
//! pencil mark on the list. Needs a lab, a safehouse, the note and the civilian.
class RZ_Mission_TheAuthorList : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected string m_sHouse;
	protected vector m_vHouse;
	protected vector m_vLead;
	protected int m_iNumber;
	protected int m_iDay;
	protected string m_sAnswer;
	protected string m_sHelper;
	protected ref array<string> m_aOptions = {};
	protected int m_iListClue = -1;
	protected int m_iHelperClue = -1;
	protected int m_iLeadClue = -1;
	protected bool m_bListRead;
	protected bool m_bHelperHeard;
	protected bool m_bHouseShown;
	protected bool m_bLeadShown;
	protected bool m_bToldLab;

	protected static const int HAVEN_FAME = 4;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 8, 12);
		m_Req.Need(RZ_EFaction.SURVIVORS, RZ_EStandingCheck.NOT_HUNTED);
		m_Req.Bonus("elder");
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || RZ_ContractHelp.CountSafehouses(gm) == 0 || !HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = RZ_ContractHelp.PickFreeSafehouse(gm);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_iSettlement = h.m_iSettlement;
		m_sHouse = h.m_sName;
		m_vHouse = h.m_vPos;
		m_vLead = LandPointNear(gm.GetLabPosition(), 5, 10);

		// The trials started on Day 6; the volunteer went up on a day before today, and so did
		// someone else from the house on another day
		int today = 12;
		if (gm.GetBroadcast())
			today = Math.Max(8, gm.GetBroadcast().GetDay());

		m_iDay = m_Rng.Int(6, today);
		int otherDay = m_iDay;
		for (int g = 0; g < 20 && otherDay == m_iDay; g++)
		{
			otherDay = m_Rng.Int(6, today);
		}

		if (otherDay == m_iDay)
			otherDay = m_iDay + 1;

		m_iNumber = m_Rng.Int(11, 90);

		// Three volunteers with different initials (the lab checks the consent form's), and the helper
		array<string> names = {};
		PickVolunteers(names);
		m_sAnswer = names[0];
		string walked = names[1];
		string other = names[2];
		m_sHelper = names[3];

		// The day's two names in either order; the days in order
		string sameDay = string.Format("%1. %2.", m_sAnswer, walked);
		if (m_Rng.Chance(0.5))
			sameDay = string.Format("%1. %2.", walked, m_sAnswer);

		string lines = string.Format("Day %1: %2\nDay %3: %4.", m_iDay, sameDay, otherDay, other);
		if (otherDay < m_iDay)
			lines = string.Format("Day %1: %2.\nDay %3: %4", otherDay, other, m_iDay, sameDay);

		m_iListClue = AddClue("The trial list", string.Format("Pinned inside the door, in pencil, in three different hands: \"Walked up to the lab for the trial, by %1:\"\n%2\nUnderneath, a row of little candles drawn in the same pencil, and no names next to them.", m_sHelper, lines));
		m_iHelperClue = AddClue(m_sHelper, string.Format("\"I walked them up. All of them, every time. You hold their hand up the hill, and then you wait outside, because they won't let you in.\" %1 looks at the door, not at you. \"%2 came back down. So did %3. I carried %3's bag. That's all I know, and it's more than the lab knows.\"",
			m_sHelper, walked, other));
		m_iLeadClue = AddClue("The trial lead", "");

		m_aOptions.Insert(m_sAnswer);
		m_aOptions.Insert(walked);
		m_aOptions.Insert(other);
		m_Rng.ShuffleStrings(m_aOptions);

		m_vPos = m_vHouse;
		m_sTitle = "The Author List";
		m_sBrief = string.Format("Volunteer %1 came to us on Day %2 from the safehouse in %3. A second volunteer came up with them that day and walked out. Volunteer %1 didn't. We have a number, a pulse chart and a time of death, and no name. The Haven wants them named, and so do we. Find out who it was, and bring the name to the trial lead at the lab.",
			m_iNumber, m_iDay, m_sHouse);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Four full names; the first three (the volunteers) with different initials.
	protected void PickVolunteers(notnull array<string> result)
	{
		array<string> firsts = {};
		RZ_NoteText.DistinctFirstNames(m_Rng, 4, firsts);
		while (firsts.Count() < 4)
		{
			firsts.Insert("Dana");
		}

		array<string> initials = {};
		for (int i = 0; i < 4; i++)
		{
			string full = firsts[i] + " " + RZ_NoteText.Surname(m_Rng);
			for (int tries = 0; tries < 10 && i < 3 && initials.Contains(Initials(full)); tries++)
			{
				full = firsts[i] + " " + RZ_NoteText.Surname(m_Rng);
			}

			initials.Insert(Initials(full));
			result.Insert(full);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string Initials(string full)
	{
		int gap = full.IndexOf(" ");
		if (gap < 0 || gap + 1 >= full.Length())
			return full.Substring(0, 1);

		return full.Substring(0, 1) + full.Substring(gap + 1, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HouseDone()
	{
		return m_bListRead && m_bHelperHeard;
	}

	//------------------------------------------------------------------------------------------------
	protected string NameLabels()
	{
		if (!HouseDone())
			return "";

		string labels = "";
		foreach (int i, string n : m_aOptions)
		{
			if (i > 0)
				labels += "|";

			labels += string.Format("Volunteer %1 was %2", m_iNumber, n);
		}

		return labels;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bHouseShown && RZ_ContractHelp.AnyNear(players, m_vHouse, 250))
		{
			m_bHouseShown = true;
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vHouse, 2, 4), m_iListClue, "Read the list by the door");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vHouse, 4, 8), 0, "", "", m_iHelperClue, "Talk to " + m_sHelper);
		}

		if (!m_bLeadShown && RZ_ContractHelp.AnyNear(players, m_vLead, 250))
		{
			m_bLeadShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vLead, 1, "", NameLabels(), m_iLeadClue, "Talk to the trial lead");
		}
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iListClue && HasBonus(gm, playerId))
			text += string.Format("\n(You know how a council keeps a list. The pencil has gone over %1 twice, the way they mark someone who didn't come home.)", m_sAnswer);

		if (idx == m_iLeadClue)
		{
			text = string.Format("\"Volunteer %1. Day %2. The safehouse in %3.\" She turns the notebook round so you can see the number, and nothing next to it. \"If you find out, I'll write it in. In ink.\"", m_iNumber, m_iDay, m_sHouse);
			if (!HouseDone())
				text += "\nYou don't have a name to give her yet.";
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iListClue)
			m_bListRead = true;

		if (idx == m_iHelperClue)
			m_bHelperHeard = true;

		if (!HouseDone() || m_bToldLab)
			return;

		m_bToldLab = true;
		m_vPos = m_vLead;
		SetObjectLabels(1, "", NameLabels());
		NotifyParticipants(gm, m_sTitle, string.Format("You know who went up on Day %1, and who came back down. Take the name to the trial lead at the lab, grid %2.", m_iDay, RZ_SettlementScanner.GridReference(m_vLead)));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 1 || option < 0 || option >= m_aOptions.Count())
			return;

		if (!HouseDone())
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You don't have a name yet. The safehouse in %1 does.", m_sHouse));
			return;
		}

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"I didn't ask you.\" She keeps the pen capped.");
			return;
		}

		if (m_aOptions[option] == m_sAnswer)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("She checks it against the initials on the consent form, nods, and writes it in over the number, in capitals: %1. Then she sits with the page for a while and doesn't say anything, and you don't either.", m_sAnswer));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		m_iWrong++;
		if (m_iWrong >= 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("She checks it against the consent form and closes the notebook. \"A number, then. I won't write a guess in ink.\" Volunteer %1 stays a number.", m_iNumber));
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		LockOut(300);
		gm.NotifyPlayer(playerId, m_sTitle, "She checks it against the initials on the consent form and shakes her head. \"Not those initials. Go and ask again. I'll be here.\"");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (gm.GetLeaders())
			gm.GetLeaders().AddHeadline(string.Format("Halcyon's list of authors has one number fewer. Volunteer %1 was %2, from the safehouse in %3.", m_iNumber, m_sAnswer, m_sHouse), false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.SURVIVORS, HAVEN_FAME);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + string.Format("\nThe safehouse in %1: grid %2. The trial lead at the lab: grid %3.",
			m_sHouse, RZ_SettlementScanner.GridReference(m_vHouse), RZ_SettlementScanner.GridReference(m_vLead));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("The Author List: Volunteer %1 is still a number.", m_iNumber);
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vHouse);
		result.Insert(m_vLead);
	}
}
