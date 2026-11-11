//! Halcyon's hard edges as contracts (docs/LORE.md, "Halcyon"; docs/MISSIONS.md, "Halcyon" #21-22):
//! The Three Weeks (the Institute's notes from the closed site in the hills) and The Author List
//! (a failed trial volunteer's name, carried back from the Haven).

//------------------------------------------------------------------------------------------------
//! #21 The Three Weeks: on Day 3 Dr Wolff went back up to the closed site and didn't come down.
//! A step chain (RZ_StepChain): her van at the edge of the last town (her notebook gives the
//! site's grid and how the store code works), then the site's gate (the gate log gives the
//! date). The store's terminal takes the code (the day the Institute was told, day then month);
//! inside are the site's notes. Then the choice: the lab (+2 research, Halcyon Fame), the Haven
//! (Haven Fame, Halcyon Infamy, a headline that the Institute knew), or the Red Hand (Red Hand
//! Fame and Favor, Halcyon Infamy). The site is a remote spot between two towns, at a real
//! building when there is one. ★ Lab Tech: the gate log's right line is pointed out, more Fame.
//! Needs the note, the terminal (or keypad), the mission item and the three drop points.
class RZ_Mission_TheThreeWeeks : RZ_PuzzleMission
{
	protected ref RZ_StepChain m_Chain = new RZ_StepChain();
	protected int m_iLogClue = -1;
	protected int m_iScreen = -1;
	protected string m_sCode;
	protected string m_sAnchor;
	protected vector m_vVan;
	protected vector m_vGate;
	protected vector m_vStore;
	protected vector m_vHavenDrop;
	protected string m_sHavenPlace;
	protected vector m_vRedDrop;
	protected string m_sRedPlace;
	protected int m_iPhase;
	protected bool m_bStoreShown;

	// The notes, once they're out of the store (kept like RZ_Mission_Delivery keeps its item)
	protected IEntity m_Item;
	protected bool m_bItemWasCarried;
	protected vector m_vItemLastPos;
	protected int m_iItemReturns;
	protected bool m_bDelivered;
	protected ref array<IEntity> m_aDrops = {};
	protected ref array<int> m_aDropFactions = {};

	protected static const int PHASE_TRAIL = 0;		// the van, then the gate (the step chain)
	protected static const int PHASE_STORE = 1;		// the locked store
	protected static const int PHASE_CARRY = 2;		// the notes are out: whose are they?
	protected static const float RESEARCH = 2;
	protected static const int CROSS_INFAMY = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 25, 25);
		m_Req.Bonus("labtech");
		m_Req.m_bNotCured = true;
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!gm.HasLab() || !HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.TERMINAL) || gm.GetMissionItemPrefab().IsEmpty())
			return 0;

		if (gm.GetDropPointPrefab(RZ_EFaction.INSTITUTE).IsEmpty() || gm.GetDropPointPrefab(RZ_EFaction.SURVIVORS).IsEmpty() || gm.GetDropPointPrefab(RZ_EFaction.BANDITS).IsEmpty())
			return 0;

		if (gm.GetSettlements().Count() < 4)
			return 0;

		return 0.5;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		int anchor = -1;
		if (!FindSite(gm, anchor))
			return false;

		RZ_Settlement a = gm.GetSettlement(anchor);
		if (!a || !PickDrops(gm, anchor))
			return false;

		m_iSettlement = anchor;
		m_sAnchor = a.m_sName;
		float toSite = RZ_Compass.Bearing(a.m_vCenter, m_vStore);
		m_vVan = LandPointNear(RZ_Compass.PointAt(a.m_vCenter, toSite, a.m_fRadius + 30), 0, 15);
		m_vGate = LandPointNear(RZ_Compass.PointAt(m_vStore, RZ_Compass.Bearing(m_vStore, m_vVan), 45), 0, 8);

		// The store code is the day the Institute was told, day then month. The log around it has
		// other dates, so it has to be read.
		int month = m_Rng.Int(1, 13);
		int told = m_Rng.Int(3, 24);
		m_sCode = RZ_NoteText.Pad(told, 2) + RZ_NoteText.Pad(month, 2);
		string mm = "/" + RZ_NoteText.Pad(month, 2);
		string clinic = ClinicTown(gm, anchor);
		string patient = RZ_NoteText.Surname(m_Rng);

		m_Chain.SetMission(m_iId);
		RZ_ChainStep van = m_Chain.AddStep(gm, anchor, m_vVan, 8, 0);
		van.m_iGuards = 2;
		van.m_sReadLabel = "Search the van";
		van.m_iClue = AddClue("Dr Wolff's notebook", string.Format("Dr G. Wolff's notebook, on the passenger seat. The last page, Day 3:\n\"He won't drive past %1. Fair enough. I'll walk the rest: the site is %2 from here, grid %3. The store code is the date we told the Institute, day then month, four figures. Somebody thought that was funny. If I'm not back by dark, he's not to come looking.\"\nUnder it, in somebody else's hand: \"Waited till dark.\"",
			m_sAnchor, RZ_Util.DirectionText(m_vVan, m_vGate), RZ_SettlementScanner.GridReference(m_vGate)));

		RZ_ChainStep gate = m_Chain.AddStep(gm, -1, m_vGate, 10, 0);
		gate.m_iGuards = 4;
		gate.m_sReadLabel = "Read the gate log";
		m_iLogClue = AddClue("The gate log", string.Format("Pinned up in the gate hut, the last page of the log:\n%1%5: Fence walked. Nothing to report.\n%2%5: The clinic in %6 phoned about %7, the fever. Message passed to the Institute, 14:10.\n%3%5: The Institute replies: carry on, say nothing.\n%4%5: Store code changed, by order.\nThen, in another pen, after the dates stop: \"Day 3. G. Wolff in, 16:40.\" There is no \"out\" after it.",
			RZ_NoteText.Pad(told - 2, 2), RZ_NoteText.Pad(told, 2), RZ_NoteText.Pad(told + 1, 2), RZ_NoteText.Pad(told + 3, 2), mm, clinic, patient));
		gate.m_iClue = m_iLogClue;

		m_iScreen = AddClue("The store door", "INSTITUTE FIELD SITE. STORE 2. LOCKED.\nFour figures, then Enter.");

		m_vPos = m_vVan;
		m_sTitle = "The Three Weeks";
		m_sBrief = string.Format("On Day 3 Dr Wolff went back up to the closed site in the hills for the rest of the samples. Her driver came back alone. He left her van at the %1 edge of %2, and that's all he'll say. Her notebook should be in it, and the site's notes are still up there. They belong to the Institute. Bring back whatever is left, and please don't read them on the way.",
			RZ_Compass.DirectionWord(toSite), m_sAnchor);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The closed site: a remote spot between two towns, well clear of every settlement and the
	//! lab, at a real factory, mast or barracks there if there is one. anchor = the nearest town.
	protected bool FindSite(RZ_GameModeComponent gm, out int anchor)
	{
		array<ref RZ_Settlement> all = gm.GetSettlements();
		int n = all.Count();
		array<string> kinds = {"military", "industrial", "radio", "power", "tower"};
		for (int t = 0; t < 30; t++)
		{
			float need = 450;
			if (t >= 20)
				need = 300;

			int ia = m_Rng.Int(0, n);
			int ib = m_Rng.Int(0, n);
			if (ia == ib)
				continue;

			vector ca = all[ia].m_vCenter;
			vector cb = all[ib].m_vCenter;
			float gap = RZ_Util.DistXZ(ca, cb);
			if (gap < 1400 || gap > 6000)
				continue;

			vector line = cb - ca;
			vector side = Vector(-line[2], 0, line[0]) * (1 / gap);
			vector mid = ca + line * m_Rng.Range(0.4, 0.6) + side * m_Rng.Range(-gap * 0.2, gap * 0.2);
			vector p = HighPointNear(RZ_Util.GroundPos(mid), 0, 200, 12);
			if (!IsRemote(gm, p, need))
				continue;

			RZ_Landmark l = RZ_LandmarkFinder.FindNearest(p, 400, kinds);
			if (l && IsRemote(gm, l.m_vPos, need - 100))
				p = LandPointNear(l.m_vPos, 4, 10);

			int nearest = gm.FindNearestSettlement(p);
			if (nearest < 0 || nearest == gm.GetLabSettlement() || gm.GetMissions().IsSettlementBusy(nearest))
				continue;

			m_vStore = p;
			anchor = nearest;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Dry, inside the line, at least `gap` past every settlement's edge and 1.5 km from the lab.
	protected static bool IsRemote(RZ_GameModeComponent gm, vector p, float gap)
	{
		if (RZ_Util.IsWater(p) || RZ_Perimeter.IsNearLineAt(p) || RZ_LockedSpots.IsBlocked(p))
			return false;

		if (gm.HasLab() && RZ_Util.DistXZ(p, gm.GetLabPosition()) < 1500)
			return false;

		foreach (RZ_Settlement s : gm.GetSettlements())
		{
			if (RZ_Util.DistXZ(p, s.m_vCenter) - s.m_fRadius < gap)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the clinic was: another town on the map (Patient Zero's records are in one like it).
	protected string ClinicTown(RZ_GameModeComponent gm, int anchor)
	{
		array<int> options = {};
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (i != anchor)
				options.Insert(i);
		}

		if (options.IsEmpty())
			return "town";

		return gm.GetSettlement(options[m_Rng.Int(0, options.Count())]).m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven's drop at the safehouse nearest the site (the safest town without one), the Red
	//! Hand's outside a contested town 0.8-4 km away. Data only: spawned once the notes are out.
	protected bool PickDrops(RZ_GameModeComponent gm, int anchor)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Safehouse h;
		if (houses && houses.Count() > 0)
			h = houses.Get(houses.FindNearest(m_vStore));

		if (h)
		{
			m_vHavenDrop = LandPointNear(h.m_vPos, 4, 8);
			m_sHavenPlace = "the safehouse in " + h.m_sName;
		}
		else
		{
			RZ_Settlement hs = gm.GetSettlement(PickSafestSettlement(gm, gm.GetLabSettlement()));
			if (!hs)
				return false;

			m_vHavenDrop = LandPointNear(hs.m_vCenter, 0, hs.m_fRadius * 0.4);
			m_sHavenPlace = hs.m_sName;
		}

		int camp = -1;
		for (int i = 0; i < 6 && camp < 0; i++)
		{
			int pick = PickSettlementAway(gm, m_vStore, 800, 4000, 20, 90, anchor);
			if (pick < 0)
				pick = PickSettlementAway(gm, m_vStore, 500, 8000, 0, 100, anchor);

			if (pick != gm.GetLabSettlement())
				camp = pick;
		}

		RZ_Settlement rs = gm.GetSettlement(camp);
		if (!rs)
			return false;

		m_vRedDrop = LandPointNear(rs.m_vCenter, rs.m_fRadius, rs.m_fRadius + 60);
		m_sRedPlace = "outside " + rs.m_sName;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iLogClue && HasBonus(gm, playerId))
			text += "\n(You know Institute paperwork. \"Message passed\" is the moment a thing becomes the Institute's problem. That's the date that matters.)";

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		m_Chain.OnClueRead(idx);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iPhase == PHASE_TRAIL)
		{
			int done = m_Chain.Update(gm, dt, ids, players, m_aParticipants);
			if (m_Chain.IsDone())
			{
				m_iPhase = PHASE_STORE;
				m_vPos = m_vStore;
				NotifyParticipants(gm, m_sTitle, string.Format("The store is up past the hut, %1 from the gate. Its door has a keypad, and it still has power.", RZ_Util.DirectionText(m_vGate, m_vStore)));
			}
			else
			{
				RZ_ChainStep st = m_Chain.GetCurrent();
				if (st)
					m_vPos = st.m_vPos;

				if (done == 0)
					NotifyParticipants(gm, m_sTitle, string.Format("Wolff walked the rest. The site's gate is at grid %1.", RZ_SettlementScanner.GridReference(m_vGate)));
			}
		}

		if (m_iPhase == PHASE_STORE && !m_bStoreShown && RZ_ContractHelp.AnyNear(players, m_vStore, 250))
		{
			m_bStoreShown = true;
			SpawnObject(gm, RZ_EPuzzleProp.TERMINAL, m_vStore, 0, "", "", m_iScreen, "Read the door");
			SpawnGuards(gm, m_vStore, 1, 6, 12, true);
			SpawnGuards(gm, m_vStore, 3, 10, 35);
		}

		if (m_iPhase == PHASE_CARRY)
			TrackNotes(gm, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCodeEntered(RZ_GameModeComponent gm, int playerId, int objIdx, string code, IEntity source)
	{
		if (objIdx != 0 || m_iPhase != PHASE_STORE)
			return;

		if (code == m_sCode)
		{
			OpenStore(gm);
			return;
		}

		// Loud, and the door sulks for a while. It never wipes: the notes are paper.
		m_iWrong++;
		SoundAlarm(gm, m_vStore, 20, m_iWrong == 2);
		LockOut(120);
		gm.NotifyPlayer(playerId, m_sTitle, "The keypad buzzes, long and loud, and goes dead for two minutes. Somewhere past the fence, something answers it.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenStore(RZ_GameModeComponent gm)
	{
		if (!SpawnNotes(gm, RZ_Util.RandomPointInRing(m_vStore, 1, 2)))
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		AddDrop(gm, RZ_EFaction.INSTITUTE, RZ_HalcyonContractHelp.DropPos(gm));
		AddDrop(gm, RZ_EFaction.SURVIVORS, m_vHavenDrop);
		AddDrop(gm, RZ_EFaction.BANDITS, m_vRedDrop);
		m_iPhase = PHASE_CARRY;
		m_vPos = m_vStore;
		NotifyParticipants(gm, m_sTitle, string.Format("The lock gives. Inside is a cold room that isn't cold any more: shelves of numbered notebooks, and a data drive in a sealed bag marked \"For the Institute only\". The top notebook is dated three weeks before the gates closed. You take the drive and the notebooks that are still dry.\nThe lab wants them (grid %1). The Haven would read them out loud: %2, grid %3. The Red Hand would pay for them: %4, grid %5.",
			RZ_SettlementScanner.GridReference(DropPos(RZ_EFaction.INSTITUTE)), m_sHavenPlace, RZ_SettlementScanner.GridReference(m_vHavenDrop), m_sRedPlace, RZ_SettlementScanner.GridReference(m_vRedDrop)));
	}

	//------------------------------------------------------------------------------------------------
	protected bool SpawnNotes(RZ_GameModeComponent gm, vector pos)
	{
		m_Item = RZ_Util.SpawnPrefab(gm.GetMissionItemPrefab(), RZ_Util.GroundPos(pos) + Vector(0, 0.3, 0));
		if (!m_Item)
			return false;

		RZ_MissionItemComponent.Tag(m_Item, m_iId);
		m_vItemLastPos = m_Item.GetOrigin();
		m_bItemWasCarried = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddDrop(RZ_GameModeComponent gm, RZ_EFaction faction, vector pos)
	{
		IEntity drop = RZ_Util.SpawnPrefab(gm.GetDropPointPrefab(faction), RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
		if (!drop)
			return;

		m_aDrops.Insert(drop);
		m_aDropFactions.Insert(faction);
		TrackProp(drop);
	}

	//------------------------------------------------------------------------------------------------
	protected vector DropPos(RZ_EFaction faction)
	{
		int idx = m_aDropFactions.Find(faction);
		if (idx < 0 || !m_aDrops[idx])
			return m_vPos;

		return m_aDrops[idx].GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	//! Who carries the notes. Gone with a carrier who left (or a body that was cleared): put
	//! back where they were last seen, a few times at most. Lost from the ground: lost.
	protected void TrackNotes(RZ_GameModeComponent gm, array<int> ids, array<IEntity> players)
	{
		if (m_bDelivered)
			return;

		if (!m_Item)
		{
			if (m_bItemWasCarried && m_iItemReturns < 3 && SpawnNotes(gm, LandPointNear(m_vItemLastPos, 0, 2)))
			{
				m_iItemReturns++;
				NotifyInterested(gm, m_sTitle, string.Format("Whoever had the site's notes is gone. They're lying where they were last seen (grid %1).", RZ_SettlementScanner.GridReference(m_vItemLastPos)));
				return;
			}

			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		IEntity root = m_Item.GetRootParent();
		m_bItemWasCarried = root && root != m_Item;
		if (m_bItemWasCarried)
			m_vItemLastPos = root.GetOrigin();
		else
			m_vItemLastPos = m_Item.GetOrigin();

		AddParticipant(RZ_Util.GetCarrierPlayerId(m_Item));
	}

	//------------------------------------------------------------------------------------------------
	//! Only at this mission's own drops, and only from someone Halcyon sent: whoever gets the
	//! notes pays, Halcyon remembers who else got them.
	override bool OnItemDelivered(RZ_GameModeComponent gm, int playerId, RZ_EFaction faction, IEntity drop)
	{
		if (m_bDelivered || !drop || m_iPhase != PHASE_CARRY)
			return false;

		int dropIdx = m_aDrops.Find(drop);
		if (dropIdx < 0 || m_aDropFactions[dropIdx] != faction)
			return false;

		if (!IsEligible(gm, playerId))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "Whoever these notes go to will want to know who Halcyon sent for them. It wasn't you.");
			return false;
		}

		m_bDelivered = true;
		m_aParticipants.Clear();
		m_aParticipants.Insert(playerId);
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		RZ_ContractHelp.AddNear(ids, players, drop.GetOrigin(), 50, m_aParticipants);

		if (faction == RZ_EFaction.INSTITUTE)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The director signs for them herself and doesn't open them in front of you. \"Thank you. These are... thank you.\" The door closes. Nobody at the lab says anything about three weeks.");
		}
		else if (faction == RZ_EFaction.SURVIVORS)
		{
			m_eGiver = RZ_EFaction.SURVIVORS;
			m_iFameReward = 20;
			SetRival(RZ_EFaction.INSTITUTE, CROSS_INFAMY);
			gm.NotifyPlayer(playerId, m_sTitle, "The council reads them at the long table by candlelight, one page each, out loud. Nobody eats. When they get to the reply from the Institute, somebody laughs, once, and stops.");
			if (gm.GetLeaders())
				gm.GetLeaders().AddHeadline("The Haven has read the notes from the Institute's closed site in the hills. The Institute knew about the first case three weeks before the gates closed, and told the site to carry on.");
		}
		else
		{
			m_eGiver = RZ_EFaction.BANDITS;
			m_iFameReward = 12;
			m_iFavorReward = 35;
			SetRival(RZ_EFaction.INSTITUTE, CROSS_INFAMY);
			gm.NotifyPlayer(playerId, m_sTitle, "\"Lovely. Halcyon will pay for every page, and then again for the copies.\" They don't read them. They count them.");
			if (gm.GetLeaders())
				gm.GetLeaders().AddHeadline("The Red Hand is selling pages from somewhere in the hills, one at a time. Halcyon is buying.", false);
		}

		m_eState = RZ_EMissionState.SUCCEEDED;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		if (m_eGiver == RZ_EFaction.INSTITUTE)
			gm.GetCure().AddResearch(RESEARCH, "Notes from the Institute's closed site reached the lab.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = super.GetWhereText(gm, from);
		if (m_iPhase == PHASE_TRAIL && m_Chain.GetCurrentIndex() == 0)
			return text + "\nDr Wolff's van: the edge of " + m_sAnchor + ", grid " + RZ_SettlementScanner.GridReference(m_vVan);

		if (m_iPhase != PHASE_CARRY)
			return text + "\nThe site's gate: grid " + RZ_SettlementScanner.GridReference(m_vGate);

		return text + string.Format("\nThe lab: grid %1. The Haven: %2, grid %3. The Red Hand: %4, grid %5.",
			RZ_SettlementScanner.GridReference(DropPos(RZ_EFaction.INSTITUTE)), m_sHavenPlace, RZ_SettlementScanner.GridReference(m_vHavenDrop), m_sRedPlace, RZ_SettlementScanner.GridReference(m_vRedDrop));
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The Three Weeks: the site's notes are still up in the hills, or lost on the way down. Nobody from the lab is going back for them.";
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		m_Chain.GetSpots(result);
		result.Insert(m_vStore);
		if (m_Item && !m_bItemWasCarried)
			result.Insert(m_Item.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		m_Chain.Cleanup();
		m_aDrops.Clear();
		m_aDropFactions.Clear();
		if (m_Item && !m_bDelivered)
			RZ_Util.DeleteUnit(m_Item);

		m_Item = null;
	}
}

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
