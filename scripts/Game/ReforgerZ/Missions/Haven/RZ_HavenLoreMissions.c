//! The Haven's hard edges as contracts (docs/LORE.md, "The Haven"; docs/MISSIONS.md, "The Haven" #21):
//! The List.

//------------------------------------------------------------------------------------------------
//! #21 The List: a name is crossed off a safehouse's list and nobody says why. The house list and
//! a council member at the safehouse point somewhere; what's there tells one of four truths: they
//! were walked out after a bite (a cairn at the edge of town), put out by the council (a bedroll
//! on the road out), left on their own (a packed bag at a landmark), or are alive in another
//! town (a Haven wanderer when one is free, else a generated person). Then: tell the family
//! (full pay; +3 Haven Infamy if the council put them out; if they're alive, the family goes
//! after them and the house is one fewer), tell the council (75%), or keep it quiet (40%; a
//! wanderer who asked for it remembers). ★ Police/Elder: more Fame, and the list says more.
//! Needs a safehouse, the note and the civilian. One name is this contract; who crosses names out,
//! and why, is the questline The List, which can open once it's done (world flag "list.contract",
//! Missions/Questlines/Haven/RZ_Quest_TheList.c).
class RZ_Mission_TheList : RZ_PuzzleMission
{
	protected int m_iHouse = -1;
	protected string m_sHouse;
	protected vector m_vHouse;
	protected int m_iTruth;
	protected string m_sName;			// the name crossed out
	protected string m_sKin;			// who asked
	protected string m_sKinWord;		// what the crossed-out one is to them: "brother"
	protected string m_sCouncil;		// the council member who answers sideways
	protected int m_iDay;
	protected int m_iWanderer = -1;
	protected vector m_vTruth;
	protected string m_sTruthPlace;
	protected int m_iListClue = -1;
	protected int m_iCouncilClue = -1;
	protected int m_iKinClue = -1;
	protected int m_iTruthClue = -1;
	protected bool m_bListRead;
	protected bool m_bCouncilHeard;
	protected bool m_bTruthFound;
	protected bool m_bHouseShown;
	protected bool m_bTruthShown;

	protected static const int TRUTH_WALKED_OUT = 0;
	protected static const int TRUTH_EXILED = 1;
	protected static const int TRUTH_LEFT = 2;
	protected static const int TRUTH_ALIVE = 3;
	protected static const int OBJ_KIN = 0;
	protected static const int OBJ_COUNCIL = 1;
	protected static const int TOLD_INFAMY = 3;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 12, 12);
		m_Req.Bonus("police");
		m_Req.Bonus("elder");
		// Not while the questline The List asks the same question at length
		if (m_iQuestRun < 0)
			m_Req.NotWhileQuest("list");

		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (RZ_ContractHelp.CountSafehouses(gm) == 0 || !HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return 0;

		return 0.7;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iHouse = RZ_ContractHelp.PickFreeSafehouse(gm);
		RZ_Safehouse h = RZ_ContractHelp.Safehouse(gm, m_iHouse);
		if (!h)
			return false;

		m_iSettlement = h.m_iSettlement;
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_sHouse = h.m_sName;
		m_vHouse = h.m_vPos;

		int today = 12;
		if (gm.GetBroadcast())
			today = Math.Max(4, gm.GetBroadcast().GetDay());

		m_iDay = m_Rng.Int(2, today);

		// The names: who asked, who answers, and (unless a wanderer is it) who was crossed out
		array<string> firsts = {};
		RZ_NoteText.DistinctFirstNames(m_Rng, 3, firsts);
		while (firsts.Count() < 3)
		{
			firsts.Insert("Dalibor");
		}

		m_sName = firsts[0] + " " + RZ_NoteText.Surname(m_Rng);
		m_sKin = firsts[1];
		m_sCouncil = firsts[2];
		array<string> kinWords = {"brother", "son", "father", "cousin"};
		m_sKinWord = m_Rng.Pick(kinWords);

		m_iTruth = m_Rng.Int(0, 4);
		if (m_iTruth == TRUTH_ALIVE && !PlaceAlive(gm))
			m_iTruth = TRUTH_LEFT;

		float dir = m_Rng.Int(0, 8) * 45;
		string dirWord = RZ_Compass.DirectionWord(dir);
		if (m_iTruth == TRUTH_WALKED_OUT)
		{
			m_vTruth = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, dir, s.m_fRadius + 50), 0, 15);
			m_sTruthPlace = string.Format("the %1 edge of %2", dirWord, s.m_sName);
		}
		else if (m_iTruth == TRUTH_EXILED)
		{
			m_vTruth = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, dir, s.m_fRadius + 150), 0, 25);
			m_sTruthPlace = string.Format("the road %1 out of %2", dirWord, s.m_sName);
		}
		else if (m_iTruth == TRUTH_LEFT)
		{
			array<string> kinds = {"church", "school", "station", "watertower", "well", "monument"};
			RZ_Landmark mark = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
			if (mark)
			{
				m_vTruth = LandPointNear(mark.m_vPos, 3, 8);
				m_sTruthPlace = string.Format("%1 in %2", mark.m_sName, s.m_sName);
			}
			else
			{
				m_vTruth = LandPointNear(RZ_Compass.PointAt(s.m_vCenter, dir, s.m_fRadius * 0.6), 0, 20);
				m_sTruthPlace = string.Format("an empty house on the %1 side of %2", dirWord, s.m_sName);
			}
		}

		WriteClues(gm);
		m_vPos = m_vHouse;
		m_sTitle = "The List";
		m_sBrief = string.Format("The safehouse in %1 has crossed %2 off its list, and nobody will tell %3 why. %2 is %3's %4. It isn't really our business. It's somebody's. Ask around, quietly, and then decide who ought to hear it.",
			m_sHouse, m_sName, m_sKin, m_sKinWord);
		m_sObjective = string.Format("Find out why the safehouse in %1 crossed %2 off its list.", m_sHouse, m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Alive in another town, 0.6-2.5 km away: a Haven wanderer when one is free (they're held
	//! there, and their name is the one crossed out), else a generated person. False if no town.
	protected bool PlaceAlive(RZ_GameModeComponent gm)
	{
		int town = PickSettlementAway(gm, m_vHouse, 600, 2500, 0, 80, m_iSettlement);
		RZ_Settlement t = gm.GetSettlement(town);
		if (!t)
			return false;

		m_vTruth = LandPointNear(t.m_vCenter, t.m_fRadius * 0.2, t.m_fRadius * 0.6);
		m_sTruthPlace = t.m_sName;

		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers)
			return true;

		int mask = RZ_Wanderers.RoleBit(RZ_EWandererRole.TRADER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.MECHANIC);
		m_iWanderer = wanderers.FindForMission(mask);
		if (m_iWanderer < 0)
			return true;

		wanderers.Hold(m_iWanderer, m_iId);
		m_sName = wanderers.GetName(m_iWanderer);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void WriteClues(RZ_GameModeComponent gm)
	{
		m_iListClue = AddClue("The house list", string.Format("The house list, on the back of a calendar: everyone who sleeps here, in pencil, one line each. %1 is crossed out, one hard line, in the council's pen. Next to it someone wrote a day, Day %2, and then rubbed it out with a thumb.",
			m_sName, m_iDay));

		string council = "";
		string found = "";
		if (m_iTruth == TRUTH_WALKED_OUT)
		{
			council = string.Format("\"%1 did what the rules say. Most people manage it.\" %2 doesn't look up from the potatoes. \"If you want to bother somebody, bother %3. Take your hat off when you get there.\"", m_sName, m_sCouncil, m_sTruthPlace);
			found = string.Format("A low cairn of stones just past the last fence, and a slate with %1 scratched on it, and Day %2. Under the top stone, folded small: a coat sleeve, cut off at the elbow, and the bite mark still on it. %1 declared it. The house walked them out at dusk, and somebody came back later and built this, alone.", m_sName, m_iDay);
		}
		else if (m_iTruth == TRUTH_EXILED)
		{
			council = string.Format("\"We don't talk about people who've been put out. That's the rule, and it's a kind one, whatever you think.\" %1 wipes the same cup twice. \"%2. That's as far as the council walked %3. Don't bring anything back.\"", m_sCouncil, RZ_Util.Capitalize(m_sTruthPlace), m_sName);
			found = string.Format("A bedroll and an empty rifle sling, left where somebody sat for a long time. Nailed to a post: a council chit. \"%1. Put out, Day %2, for firing at the gate. No weapon, as the rules say.\" Underneath, in a shakier hand: \"I was aiming at the thing behind you.\"", m_sName, m_iDay);
		}
		else if (m_iTruth == TRUTH_LEFT)
		{
			council = string.Format("\"Nobody put %1 out. %1 went.\" %2 shrugs, and it's not a kind shrug. \"Their things are still at %3. Go and look, if it matters to you.\"", m_sName, m_sCouncil, m_sTruthPlace);
			string town = "the next town";
			int other = PickSettlementAway(gm, m_vHouse, 800, 5000, 0, 100, m_iSettlement);
			RZ_Settlement os = gm.GetSettlement(other);
			if (os)
				town = os.m_sName;

			found = string.Format("A packed bag nobody came back for, and a letter on top for the council: \"I'm going to find my mother in %1. Don't cross me out. I'll be back before the rota changes. %2, Day %3.\" The rota by the door has changed four times since.", town, m_sName, m_iDay);
		}
		else
		{
			council = string.Format("\"Crossed out means not here. It doesn't mean dead, whatever %1 thinks.\" %2 lowers their voice. \"Somebody on the roads said they'd seen %3 in %4. Somebody always says that.\"", m_sKin, m_sCouncil, m_sName, m_sTruthPlace);
			found = string.Format("\"Yes, it's me. No, I'm not coming back.\" %1 doesn't get up. \"Food was short. There were mouths that needed it more than mine, and I did the sums before the council had to. Tell them whatever you like. Just don't tell them where.\"", m_sName);
		}

		m_iCouncilClue = AddClue(m_sCouncil, council);
		m_iTruthClue = AddClue(m_sName, found);
		m_iKinClue = AddClue(m_sKin, "");
	}

	//------------------------------------------------------------------------------------------------
	protected bool Pointed()
	{
		return m_bListRead && m_bCouncilHeard;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iListClue && HasBonus(gm, playerId))
			text += string.Format("\n(You've read enough lists to see it: the line through %1 is newer than the pencil, and the rubbed-out day is the day it was drawn.)", m_sName);

		if (idx != m_iKinClue)
			return text;

		if (!m_bTruthFound)
			return string.Format("\"%1 is my %2. Was. Is. Nobody will say which.\" %3 has a copy of the list folded small, the crossed-out line gone soft from being touched. \"Ask %4. %4 knows. Everybody knows but me.\"", m_sName, m_sKinWord, m_sKin, m_sCouncil);

		return string.Format("%1 sees your face and puts the cup down. \"Well?\"", m_sKin);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bHouseShown && RZ_ContractHelp.AnyNear(players, m_vHouse, 250))
		{
			m_bHouseShown = true;
			SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(m_vHouse, 2, 4), m_iListClue, "Read the house list");
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vHouse, 4, 8), OBJ_KIN, "", KinLabels(), m_iKinClue, "Talk to " + m_sKin);
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, LandPointNear(m_vHouse, 5, 10), OBJ_COUNCIL, "", CouncilLabels(), m_iCouncilClue, "Talk to " + m_sCouncil);
		}

		if (!Pointed() || m_bTruthShown || !RZ_ContractHelp.AnyNear(players, m_vTruth, 250))
			return;

		m_bTruthShown = true;
		if (m_iTruth == TRUTH_WALKED_OUT)
			SpawnClue(gm, RZ_EPuzzleProp.OBJECT, m_vTruth, m_iTruthClue, "Look at the cairn");
		else if (m_iTruth == TRUTH_EXILED)
			SpawnClue(gm, RZ_EPuzzleProp.BODY, m_vTruth, m_iTruthClue, "Look at the bedroll");
		else if (m_iTruth == TRUTH_LEFT)
			SpawnClue(gm, RZ_EPuzzleProp.BODY, m_vTruth, m_iTruthClue, "Look in the bag");
		else
			SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vTruth, 2, "", "", m_iTruthClue, "Talk to " + m_sName);

		// The edge of town and the road out have their infected
		if (m_iTruth == TRUTH_WALKED_OUT || m_iTruth == TRUTH_EXILED)
			SpawnGuards(gm, m_vTruth, 2, 25, 60);
	}

	//------------------------------------------------------------------------------------------------
	protected string KinLabels()
	{
		if (!m_bTruthFound)
			return "";

		return string.Format("Tell %1 the truth|Keep it to yourself", m_sKin);
	}

	//------------------------------------------------------------------------------------------------
	protected string CouncilLabels()
	{
		if (!m_bTruthFound)
			return "";

		return "Tell the council what you found";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iListClue || idx == m_iCouncilClue)
		{
			bool before = Pointed();
			if (idx == m_iListClue)
				m_bListRead = true;
			else
				m_bCouncilHeard = true;

			if (!before && Pointed())
			{
				m_vPos = m_vTruth;
				NotifyParticipants(gm, m_sTitle, string.Format("The list and %1 both point the same way: %2, grid %3.", m_sCouncil, m_sTruthPlace, RZ_SettlementScanner.GridReference(m_vTruth)));
			}

			return;
		}

		if (idx != m_iTruthClue || m_bTruthFound)
			return;

		m_bTruthFound = true;
		m_vPos = m_vHouse;
		SetObjectLabels(OBJ_KIN, "", KinLabels());
		SetObjectLabels(OBJ_COUNCIL, "", CouncilLabels());
		NotifyParticipants(gm, m_sTitle, string.Format("Now you know why %1 was crossed out. Back at the safehouse in %2, %3 is waiting, and so is the council. Or nobody has to know.", m_sName, m_sHouse, m_sKin));
	}

	//------------------------------------------------------------------------------------------------
	//! Only someone the Haven sent may settle it, once they know.
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_KIN && objIdx != OBJ_COUNCIL)
			return;

		if (!m_bTruthFound)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You don't know anything yet.");
			return;
		}

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 looks at you and shakes their head. \"I didn't ask you.\"", m_sKin));
			return;
		}

		RZ_Wanderers wanderers = gm.GetWanderers();
		if (objIdx == OBJ_COUNCIL && option == 0)
		{
			m_fRewardScale = 0.75;
			gm.NotifyPlayer(playerId, m_sTitle, CouncilText());
			if (m_iTruth == TRUTH_ALIVE)
				Remember(wanderers, playerId, RZ_Wanderers.MEM_WRONGED);

			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (objIdx != OBJ_KIN)
			return;

		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, KinText());
			if (m_iTruth == TRUTH_EXILED)
			{
				gm.AddInfamy(playerId, RZ_EFaction.SURVIVORS, TOLD_INFAMY);
				gm.NotifyPlayer(playerId, "The Haven", string.Format("The council hears by supper who told. (+%1 Haven Infamy)", TOLD_INFAMY));
			}

			if (m_iTruth == TRUTH_ALIVE && gm.GetSafehouses())
				gm.GetSafehouses().RemovePopulation(m_iHouse, 1);

			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			m_fRewardScale = 0.4;
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You tell %1 you couldn't find out. %1 nods as if that was the answer they expected, and goes back to the list. They'll keep asking. You'll keep not saying.", m_sKin));
			if (m_iTruth == TRUTH_ALIVE)
				Remember(wanderers, playerId, RZ_Wanderers.MEM_HELPED);

			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string KinText()
	{
		if (m_iTruth == TRUTH_WALKED_OUT)
			return string.Format("%1 listens all the way through, and then says, \"Thank you. I'd rather know.\" There's one more candle in the window tonight.", m_sKin);

		if (m_iTruth == TRUTH_EXILED)
			return string.Format("%1 goes very quiet, and then very loud, and the whole house hears it. The rule was kept. Nobody at supper looks at the council.", m_sKin);

		if (m_iTruth == TRUTH_LEFT)
			return string.Format("%1 reads the letter twice and laughs, which is worse than crying. \"That's %2. Always going to be back before the rota changes.\"", m_sKin, m_sName);

		return string.Format("%1 has a bag packed before you've finished. \"%2? Alive?\" By morning %1 is gone too, down the road to %3. The house is one fewer.", m_sKin, m_sName, m_sTruthPlace);
	}

	//------------------------------------------------------------------------------------------------
	protected string CouncilText()
	{
		if (m_iTruth == TRUTH_LEFT)
			return string.Format("The council reads the letter and, after a while, rubs out the line through %1 and writes \"away\" next to it, in pencil. Nobody tells %2 why the list changed.", m_sName, m_sKin);

		if (m_iTruth == TRUTH_ALIVE)
			return string.Format("The council thanks you for coming to them first. They write \"%1\" next to the name, in pencil, and nothing else. Somebody will go and look. %2 won't be told.", m_sTruthPlace, m_sKin);

		return string.Format("The council thanks you for coming to them first, the way people thank you for closing a door. The line through %1 stays where it is. %2 won't be told.", m_sName, m_sKin);
	}

	//------------------------------------------------------------------------------------------------
	protected void Remember(RZ_Wanderers wanderers, int playerId, int flag)
	{
		if (wanderers && m_iWanderer >= 0)
			wanderers.RememberPlayer(m_iWanderer, playerId, flag);
	}

	//------------------------------------------------------------------------------------------------
	//! Somebody asked, and found out: the questline The List can open at this house (its council
	//! head asks the bigger question). The flag holds the safehouse, + 1.
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (gm.GetQuestlines())
			gm.GetQuestlines().SetWorldFlag("list.contract", m_iHouse + 1);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from) + string.Format("\nThe safehouse in %1: grid %2.", m_sHouse, RZ_SettlementScanner.GridReference(m_vHouse));
		if (Pointed() && !m_bTruthFound)
			text += string.Format("\nWhere it points: %1, grid %2.", m_sTruthPlace, RZ_SettlementScanner.GridReference(m_vTruth));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Nobody found out why %1 was crossed off the list in %2. %3 has stopped asking, which is worse.", m_sName, m_sHouse, m_sKin);
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		result.Insert(m_vHouse);
		result.Insert(m_vTruth);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (wanderers && m_iWanderer >= 0)
			wanderers.Release(m_iWanderer, m_vTruth);

		m_iWanderer = -1;
	}
}
