//! Puzzle missions: logic. See docs/MISSIONS.md, "Puzzle missions".

//------------------------------------------------------------------------------------------------
//! #5 The Rat (Red Hand): three suspects, exactly one lying, and the liar is the rat.
//! The statements are generated and checked by brute force so exactly one suspect
//! fits. Accuse at the boss. Wrong: the real rat runs and you gain Red Hand Infamy.
//! ★ Police/Ex-Con: more Fame.
class RZ_Mission_TheRat : RZ_PuzzleMission
{
	protected int m_iRat = -1;
	protected ref array<string> m_aNames = {};

	protected static const int STMT_ACCUSE = 0;		// "X is the rat"
	protected static const int STMT_CLEAR = 1;		// "X is not the rat"
	protected static const int STMT_SELF = 2;		// "I'm not the rat"

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 20);
		m_Req.Bonus("police");
		m_Req.Bonus("excon");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 2400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Truth of suspect i's statement if `rat` were the rat.
	protected static bool IsTrue(int type, int speaker, int target, int rat)
	{
		if (type == STMT_ACCUSE)
			return target == rat;

		if (type == STMT_CLEAR)
			return target != rat;

		return speaker != rat;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 10, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		RZ_NoteText.DistinctFirstNames(m_Rng, 3, m_aNames);
		if (m_aNames.Count() < 3)
			return false;

		// Random statements until exactly one suspect can be the (only) liar
		array<int> types = {0, 0, 0};
		array<int> targets = {0, 0, 0};
		bool found = false;
		for (int attempt = 0; attempt < 300 && !found; attempt++)
		{
			for (int i = 0; i < 3; i++)
			{
				types[i] = m_Rng.Int(0, 3);
				targets[i] = (i + m_Rng.Int(1, 3)) % 3;
			}

			int fits = 0;
			int fitRat = -1;
			for (int c = 0; c < 3; c++)
			{
				bool consistent = true;
				for (int k = 0; k < 3; k++)
				{
					if (IsTrue(types[k], k, targets[k], c) != (k != c))
						consistent = false;
				}

				if (consistent)
				{
					fits++;
					fitRat = c;
				}
			}

			if (fits == 1)
			{
				found = true;
				m_iRat = fitRat;
			}
		}

		if (!found)
			return false;

		string boss = RZ_NoteText.FirstName(m_Rng);
		int intro = AddClue(boss, "One of these three sold our route to Bastion. The truck's gone and so is Teo. I asked them each where they were. One of them lied to my face. Only one. Listen to them, then point at the rat and I'll do the rest.");
		string choices = string.Format("Point at %1|Point at %2|Point at %3", m_aNames[0], m_aNames[1], m_aNames[2]);
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", choices, intro, "Talk to " + boss);

		for (int j = 0; j < 3; j++)
		{
			int clue = AddClue(m_aNames[j], Statement(types[j], j, targets[j]));
			SpawnClue(gm, RZ_EPuzzleProp.CIVILIAN, RZ_Util.RandomPointInRing(m_vPos, 4, 8), clue, "Listen to " + m_aNames[j]);
		}

		m_sTitle = "The Rat";
		m_sBrief = string.Format("Someone in the Red Hand crew at %1 talked to Bastion. %2 wants the rat found before anyone else disappears. Hear them out. Exactly one of them is lying.", s.m_sName, boss);
		m_sObjective = string.Format("Find the rat in the Red Hand crew at %1 for %2.", s.m_sName, boss);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string Statement(int type, int speaker, int target)
	{
		string other = m_aNames[target];
		if (type == STMT_ACCUSE)
			return string.Format("It was %1. Ask %1 where they were when the truck left. Go on, ask.", other);

		if (type == STMT_CLEAR)
			return string.Format("%1 was with me at the fuel dump all night, moaning about the cold. It wasn't %1.", other);

		return "I've been with the Hand since Day 2. I've bled for it. It's not me.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option < 0 || option > 2)
			return;

		if (option == m_iRat)
		{
			m_aParticipants.Clear();
			m_aParticipants.Insert(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 goes white. Nobody says anything. Nobody needs to.", m_aNames[m_iRat]));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("While everyone stares at %1, %2 is already over the fence and gone. That's on you.", m_aNames[option], m_aNames[m_iRat]));
		gm.AddInfamy(playerId, RZ_EFaction.BANDITS, 10);
		m_eState = RZ_EMissionState.FAILED;
	}
}

//------------------------------------------------------------------------------------------------
//! #6 Patient Zero's Ledger (Halcyon): a ledger lists patients, ages and admission
//! days; dog tags give days of death. Adults died 4 days after the bite, children
//! 2. Work back to who was infected first and answer at the lab. Data is generated
//! so neither the earliest admission nor the earliest death is the answer.
//! ★ Lab Tech/Doctor: the ledger spells the method out.
class RZ_Mission_PatientZeroLedger : RZ_PuzzleMission
{
	protected int m_iZero = -1;
	protected int m_iLedger = -1;
	protected ref array<string> m_aNames = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, 20, 15);
		m_Req.Bonus("labtech");
		m_Req.Bonus("doctor");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || !gm.HasLab())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 20, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		RZ_NoteText.DistinctFirstNames(m_Rng, 3, m_aNames);
		if (m_aNames.Count() < 3)
			return false;

		array<int> ages = {0, 0, 0};
		array<int> infected = {0, 0, 0};
		array<int> admitted = {0, 0, 0};
		array<int> died = {0, 0, 0};
		bool ok = false;
		for (int attempt = 0; attempt < 200 && !ok; attempt++)
		{
			// One child, two adults (or the other way round)
			int child = m_Rng.Int(0, 3);
			array<int> days = {1, 2, 3, 4, 5, 6};
			m_Rng.ShuffleInts(days);
			for (int i = 0; i < 3; i++)
			{
				int incubation = 4;
				if (i == child)
				{
					ages[i] = m_Rng.Int(5, 13);
					incubation = 2;
				}
				else
				{
					ages[i] = m_Rng.Int(24, 71);
				}

				infected[i] = days[i];
				died[i] = days[i] + incubation;
				admitted[i] = days[i] + m_Rng.Int(1, incubation);
			}

			int zero = ArgMin(infected);
			if (ArgMin(admitted) != zero && ArgMin(died) != zero && IsUniqueMin(infected) && IsUniqueMin(died) && IsUniqueMin(admitted))
			{
				ok = true;
				m_iZero = zero;
			}
		}

		if (!ok)
			return false;

		string doctor = "Dr. " + RZ_NoteText.Surname(m_Rng);
		string ledger = string.Format("Admissions, isolation ward, %1.\n", s.m_sName);
		for (int j = 0; j < 3; j++)
		{
			ledger += string.Format("%1, age %2: admitted %3\n", m_aNames[j], ages[j], RZ_NoteText.Day(admitted[j]));
		}

		ledger += string.Format("\nIn the margin, %1: \"The fever takes a grown man four days from the bite to the end. The little ones go in half that. The families bring them in late, always late. The admission dates mean nothing.\"", doctor);
		m_iLedger = AddClue("Ward ledger", ledger);
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3), m_iLedger, "Read the ledger");

		for (int k = 0; k < 3; k++)
		{
			int tag = AddClue("Identity tag", string.Format("A plastic hospital bracelet, still on the wrist: %1. Someone has written on it in marker: died %2.", m_aNames[k], RZ_NoteText.Day(died[k])));
			SpawnClue(gm, RZ_EPuzzleProp.BODY, LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.9), tag, "Check the wrist");
		}

		vector lab = gm.GetLabPosition();
		int ask = AddClue("Halcyon", "Three patients, one ward, one of them brought it in. Which one was bitten first? Be sure. We only get to write the report once.");
		string choices = string.Format("%1 was first|%2 was first|%3 was first", m_aNames[0], m_aNames[1], m_aNames[2]);
		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, RZ_Util.RandomPointInRing(lab, 3, 6), 0, "", choices, ask, "Read Halcyon's question");

		m_sTitle = "Patient Zero's Ledger";
		m_sBrief = string.Format("Halcyon wants to know who brought the infection into the isolation ward in %1. The ward ledger is still there, and so are the patients. Work it out and give your answer at the lab.", s.m_sName);
		m_sObjective = string.Format("Work out who infected the ward in %1, then answer at the lab.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static int ArgMin(array<int> values)
	{
		int best = 0;
		foreach (int i, int v : values)
		{
			if (v < values[best])
				best = i;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsUniqueMin(array<int> values)
	{
		int minIdx = ArgMin(values);
		foreach (int i, int v : values)
		{
			if (i != minIdx && v == values[minIdx])
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iLedger && HasBonus(gm, playerId))
			text += "\n(Your training kicks in: day of death minus four for adults, minus two for children, gives the day of the bite.)";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (option < 0 || option > 2)
			return;

		if (option == m_iZero)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The researcher checks your working twice, then nods. \"That's the first link. Thank you.\"");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		m_iWrong++;
		if (m_iWrong >= 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"No. And now the report is wrong.\" She tears the page out.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		LockOut(300);
		gm.NotifyPlayer(playerId, m_sTitle, "That doesn't fit the dates. Go back over it. You get one more try, in five minutes.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(3, "Patient zero of the " + GetLocationName(gm) + " ward was identified from the ledger: the first bite, and who carried it in.");

		// The ward's first case is known: The Three Weeks can begin
		RZ_Quest_ThreeWeeks.NoteClinicRecords(gm);
	}
}

//------------------------------------------------------------------------------------------------
//! #7 The Poisoned Well (Halcyon): three wells in three villages. A clinic note rules
//! one out, a farmer's note rules another out, and a dead courier lies by the
//! poisoned one. Seal the right well. A wrong seal costs that village its water
//! (infection up); two wrong seals fail. ★ Lab Tech/Nurse: can test the water.
class RZ_Mission_PoisonedWell : RZ_PuzzleMission
{
	protected int m_iPoisoned = -1;
	protected ref array<int> m_aVillages = {};
	protected ref array<vector> m_aWells = {};
	protected ref array<bool> m_aSealed = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("labtech");
		m_Req.Bonus("nurse");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.WELL) || gm.GetSettlements().Count() < 4)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		array<int> used = {m_iSettlement};
		array<int> candidates = {};
		CollectSettlements(gm, s.m_vCenter, 400, 3000, used, candidates);
		if (candidates.Count() < 3)
			return false;

		array<string> wellKinds = {"well"};
		for (int i = 0; i < 3; i++)
		{
			int idx = candidates[i];
			RZ_Settlement v = gm.GetSettlement(idx);
			m_aVillages.Insert(idx);
			vector pos = LandPointNear(v.m_vCenter, 0, v.m_fRadius * 0.3);
			RZ_Landmark realWell = RZ_LandmarkFinder.FindNearest(v.m_vCenter, v.m_fRadius, wellKinds);
			if (realWell)
				pos = RZ_Util.RandomPointInRing(realWell.m_vPos, 1.5, 2.5);

			m_aWells.Insert(pos);
			m_aSealed.Insert(false);
			SpawnObject(gm, RZ_EPuzzleProp.WELL, pos, i, "Seal the well|Test the water", "");
		}

		m_iPoisoned = m_Rng.Int(0, 3);
		int ruledOutA = (m_iPoisoned + 1) % 3;
		int ruledOutB = (m_iPoisoned + 2) % 3;

		string clinic = RZ_NoteText.FullName(m_Rng);
		int sick = AddClue("Clinic note", string.Format("%1, %2. Eleven people with cramps and blue lips. Not the bite: nobody turns. They all carted water this week. We asked every one of them twice and none of them had been to %3.\n%4",
			s.m_sName, RZ_NoteText.Day(m_Rng.Int(6, 12)), VillageName(gm, ruledOutA), RZ_NoteText.Signoff(m_Rng, clinic)));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4), sick, "Read the clinic note");

		string farmer = RZ_NoteText.FirstName(m_Rng);
		int farm = AddClue("Farmer's note", string.Format("Pinned to a gate: \"We've drunk from our well in %1 all week, us and the goats. Nobody here is sick. Whatever it is, it isn't us. Stop asking. - %2\"", VillageName(gm, ruledOutB), farmer));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, RZ_Util.RandomPointInRing(m_aWells[ruledOutB], 6, 15), farm, "Read the note on the gate");

		int courier = AddClue("Dead courier", string.Format("A Haven courier, curled up on his side. His canteen smells of bitter almonds. In his water log, this morning's line: \"Filled up at the well in %1.\"", VillageName(gm, m_iPoisoned)));
		SpawnClue(gm, RZ_EPuzzleProp.BODY, RZ_Util.RandomPointInRing(m_aWells[m_iPoisoned], 8, 20), courier, "Search the body");

		m_sTitle = "The Poisoned Well";
		m_sBrief = string.Format("People who drank from one of three wells, in %1, %2 or %3, are dying, and it isn't the virus. Find out which one and seal it. Seal the wrong one and that village loses its water. The clinic in %4 kept notes.",
			VillageName(gm, 0), VillageName(gm, 1), VillageName(gm, 2), s.m_sName);
		m_sObjective = string.Format("Find which well, in %1, %2 or %3, is poisoned and seal it.", VillageName(gm, 0), VillageName(gm, 1), VillageName(gm, 2));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string VillageName(RZ_GameModeComponent gm, int i)
	{
		RZ_Settlement v = gm.GetSettlement(m_aVillages[i]);
		if (v)
			return v.m_sName;

		return RZ_SettlementScanner.GridReference(m_aWells[i]);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx < 0 || objIdx > 2)
			return;

		if (value == 1)
		{
			if (!HasBonus(gm, playerId))
			{
				gm.NotifyPlayer(playerId, "The water", "It looks like water. You don't know what you'd be testing for.");
				return;
			}

			if (objIdx == m_iPoisoned)
				gm.NotifyPlayer(playerId, "The water", "A drop on the test strip turns it the wrong colour fast. This is the one.");
			else
				gm.NotifyPlayer(playerId, "The water", "Hard, cold, a bit of iron. Clean.");

			return;
		}

		if (m_aSealed[objIdx])
		{
			gm.NotifyPlayer(playerId, "The well", "It's already sealed.");
			return;
		}

		m_aSealed[objIdx] = true;
		SetObjectLabels(objIdx, "", "");
		if (objIdx == m_iPoisoned)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You drag the lid over and wedge it shut. Nobody else drinks from this one.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		RZ_Settlement v = gm.GetSettlement(m_aVillages[objIdx]);
		if (v)
			v.AddInfection(15);

		m_iWrong++;
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("The well is sealed. Then a farmer from %1 comes running: that was the clean one. They'll have to fetch water from the river now, through the infected.", VillageName(gm, objIdx)));
		if (m_iWrong >= 2)
			m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement v = gm.GetSettlement(m_aVillages[m_iPoisoned]);
		if (v)
			v.AddInfection(-10);

		gm.GetCure().AddResearch(1, "A water sample from a poisoned well reached the lab. It wasn't the virus killing the village. It was something in the water.");
	}
}

//------------------------------------------------------------------------------------------------
//! #8 The Smuggler's Ledger (Red Hand): one line of a ledger is Caesar-shifted.
//! The key is a number on a memorial plaque by a church. The decoded line names
//! the stash: a landmark in another town, or one edge of it. ★ Ex-Con: more Fame.
class RZ_Mission_SmugglersLedger : RZ_PuzzleMission
{
	protected vector m_vStash;
	protected vector m_vLedger;
	protected vector m_vPlaque;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 15, 20);
		m_Req.Bonus("excon");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || gm.GetSettlements().Count() < 3)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Shifts A-Z forward by `shift`; everything else is kept.
	static string Caesar(string text, int shift)
	{
		string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		string upper = text;
		upper.ToUpper();
		string result = "";
		int len = upper.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = upper.Substring(i, 1);
			int idx = alphabet.IndexOf(ch);
			if (idx < 0)
			{
				result += ch;
				continue;
			}

			int shifted = (idx + shift) % 26;
			if (shifted < 0)
				shifted += 26;

			result += alphabet.Substring(shifted, 1);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement a = gm.GetSettlement(m_iSettlement);
		if (!a)
			return false;

		m_vPos = a.m_vCenter;
		m_vLedger = LandPointNear(a.m_vCenter, 0, a.m_fRadius * 0.4);

		// The plaque: by the church in this town (or a fallback landmark)
		array<string> churchKinds = {"church"};
		RZ_Landmark church = FindOrPlaceLandmark(gm, a.m_vCenter, a.m_fRadius, churchKinds, LandPointNear(a.m_vCenter, 0, a.m_fRadius * 0.5));
		if (!church)
			return false;

		// The stash: another town, by a landmark or at one edge
		array<int> used = {m_iSettlement};
		array<int> candidates = {};
		CollectSettlements(gm, a.m_vCenter, 500, 3500, used, candidates);
		if (candidates.IsEmpty())
			return false;

		RZ_Settlement t = gm.GetSettlement(candidates[0]);
		string tName = t.m_sName;
		string plain;
		array<string> kinds = {"watertower", "school", "station", "fuel", "tower", "monument", "castle", "lighthouse"};
		RZ_Landmark mark = RZ_LandmarkFinder.FindNearest(t.m_vCenter, t.m_fRadius, kinds);
		if (mark)
		{
			m_vStash = LandPointNear(mark.m_vPos, 5, 15);
			plain = string.Format("STASH BEHIND %1 IN %2", mark.m_sName, tName);
		}
		else
		{
			float dir = m_Rng.Int(0, 8) * 45;
			m_vStash = RZ_Compass.PointAt(t.m_vCenter, dir, t.m_fRadius * 0.8);
			plain = string.Format("STASH AT THE %1 EDGE OF %2 BY THE LAST HOUSE", RZ_Compass.DirectionWord(dir), tName);
		}

		int key = m_Rng.Int(3, 23);
		string cipher = Caesar(plain, key);

		string smuggler = RZ_NoteText.FirstName(m_Rng);
		int ledger = AddClue("A smuggler's ledger", string.Format("Columns of numbers: fuel, ammo, cigarettes, people. One line is written differently:\n\"%1\"\nIn the margin: \"Same key as always. Count the drowned on the plaque at %2 in %3. Every letter walks forward that many steps.\" - %4",
			cipher, church.m_sName, a.m_sName, smuggler));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vLedger, ledger, "Read the ledger");

		int plaque = AddClue("Memorial plaque", string.Format("Bronze, green at the edges:\n\"In memory of the %1 men of %2 who were lost to the river in the flood year. The water took them; the town remembers.\"", key, a.m_sName));
		m_vPlaque = RZ_Util.RandomPointInRing(church.m_vPos, 3, 6);
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, m_vPlaque, plaque, "Read the plaque");

		m_sTitle = "The Smuggler's Ledger";
		m_sBrief = string.Format("A dead smuggler's ledger turned up in %1, near grid %2. One line is in code, and it's the line that matters. Decode it, find what he hid, and the Red Hand will make it worth your while.", a.m_sName, RZ_SettlementScanner.GridReference(m_vLedger));
		m_sObjective = string.Format("Decode the smuggler's ledger in %1 and find the stash.", a.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (ReachedBy(ids, players, m_vStash, 25))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vStash);
		NotifyParticipants(gm, m_sTitle, "Loose bricks, a canvas bag behind them. The Red Hand will want to hear you found it.");
	}
}
