//! Puzzle missions: timing and stealth. See docs/MISSIONS.md, "Puzzle missions".

//------------------------------------------------------------------------------------------------
//! #9 The Clockwork Horde: a broken siren wails every two minutes, alternating
//! between the two ends of town, and a horde walks back and forth through the
//! square where the strongbox is. Take it without being noticed.
//! ★ Hunter: harder to notice.
class RZ_Mission_ClockworkHorde : RZ_PuzzleMission
{
	protected vector m_vBox;
	protected vector m_vSirenA;
	protected vector m_vSirenB;
	protected bool m_bAtA = true;
	protected float m_fSirenTimer;
	protected ref array<IEntity> m_aHorde = {};

	protected static const float SIREN_INTERVAL = 120;
	protected static const int MAX_ATTEMPTS = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("hunter");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2400;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.OBJECT) || gm.GetSpawner().GetMoveWaypointPrefab().IsEmpty())
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		// A town and an axis through it with dry ground at both ends (coastal towns need a few goes)
		RZ_Settlement s = null;
		bool found = false;
		for (int attempt = 0; attempt < MAX_ATTEMPTS && !found; attempt++)
		{
			m_iSettlement = PickSettlement(gm, 20, 90);
			s = gm.GetSettlement(m_iSettlement);
			if (!s)
				return false;

			m_vBox = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.15);
			float axis = m_Rng.Range(0, 360);
			m_vSirenA = RZ_Compass.PointAt(m_vBox, axis, s.m_fRadius * 0.8);
			m_vSirenB = RZ_Compass.PointAt(m_vBox, axis + 180, s.m_fRadius * 0.8);
			if (!RZ_Util.IsWater(m_vSirenA) && !RZ_Util.IsWater(m_vSirenB))
				found = true;
		}

		if (!found)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 100;

		SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vBox, 0, "Take the strongbox", "");
		// Not one that must keep coming: staying unseen is the job, so stealth may dull its senses (RZ_Stealth)
		gm.GetSpawner().SpawnHordeAt(LandPointNear(m_vSirenA, 0, 30), m_vSirenA, m_iId, m_aHorde, false);
		m_fSirenTimer = SIREN_INTERVAL;

		RZ_NoticeTracker stealth = GetNotice();
		stealth.m_fHeatThreshold = 40;
		stealth.m_fZombieRadius = 12;
		stealth.m_fZombieSeconds = 6;

		m_sTitle = "The Clockwork Horde";
		m_sBrief = string.Format("The air-raid siren in %1 is stuck on a timer. Every two minutes one end of town wails, then the other, and a horde walks back and forth between them, straight through the square. A Haven family's strongbox is still in the middle. Time it, and don't get noticed.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		float radius = 300;
		if (s)
			radius = s.m_fRadius + 50;

		GetNotice().Track(gm, ids, players, dt, m_vBox, radius);

		m_fSirenTimer -= dt;
		if (m_fSirenTimer > 0)
			return;

		m_fSirenTimer = SIREN_INTERVAL;
		m_bAtA = !m_bAtA;
		vector siren = m_vSirenB;
		string side = RZ_Compass.DirectionWord(RZ_Compass.Bearing(m_vBox, m_vSirenB));
		if (m_bAtA)
		{
			siren = m_vSirenA;
			side = RZ_Compass.DirectionWord(RZ_Compass.Bearing(m_vBox, m_vSirenA));
		}

		gm.GetHeat().AddHeat(siren, 15);
		SendHorde(gm, siren);

		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), m_vBox) < 900)
				gm.NotifyPlayer(ids[i], "Siren", string.Format("The siren at the %1 end of town starts to wail.", side));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHorde(RZ_GameModeComponent gm, vector target)
	{
		ResourceName wp = gm.GetSpawner().GetMoveWaypointPrefab();
		for (int i = m_aHorde.Count() - 1; i >= 0; i--)
		{
			IEntity unit = m_aHorde[i];
			if (!unit)
			{
				m_aHorde.Remove(i);
				continue;
			}

			TrackProp(RZ_Util.RedirectGroup(unit, target, wp));
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		RZ_NoticeTracker stealth = GetNotice();
		float threshold = 40;
		if (HasBonus(gm, playerId))
			threshold = 60;

		stealth.m_fHeatThreshold = threshold;
		if (stealth.IsNoticed(gm, user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, stealth.GetReason(gm, user) + " They turn toward you. Get out and try again later.");
			SendHorde(gm, user.GetOrigin());
			LockOut(60);
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "You lift the strongbox and walk away slowly. Nothing follows.");
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vBox);
	}
}

//------------------------------------------------------------------------------------------------
//! #10 Time-Lock Safe (Red Hand): a bank safe whose time lock opens at 03:00 game
//! time, for 20 minutes. Opening it is loud. Offered only when 03:00 is at most
//! three real hours away, and lasts until the window has passed.
//! ★ Ex-Con: knows the lock's slack, so the window is twice as long.
class RZ_Mission_TimeLockSafe : RZ_PuzzleMission
{
	protected vector m_vSafe;

	protected static const float OPEN_HOUR = 3;
	protected static const float WINDOW_HOURS = 0.334;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 20, 20);
		m_Req.Bonus("excon");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.SAFE) || !RZ_GameTime.IsCalibrated())
			return 0;

		float secs = RZ_GameTime.RealSecondsUntil(OPEN_HOUR);
		if (secs < 300 || secs > 10800)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_vSafe = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3);
		array<string> kinds = {"bank", "station", "school"};
		RZ_Landmark building = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius, kinds);
		if (building)
			m_vSafe = RZ_Util.RandomPointInRing(building.m_vPos, 2, 5);

		int plate = AddClue("Brass plate", "HARTMANN & SONS, SAFE MAKERS.\nTIME LOCK. Opens daily at 03:00 for twenty minutes.\nDo not force. The bolts will not care.");
		SpawnObject(gm, RZ_EPuzzleProp.SAFE, m_vSafe, 0, "Try the handle", "", plate, "Read the brass plate");

		float until = RZ_GameTime.RealSecondsUntil(OPEN_HOUR);
		float window = RZ_GameTime.RealSecondsFor(WINDOW_HOURS * 2);
		if (until < 0 || window < 0)
			return false;

		m_fTimeLeft = Math.Max(1800, until + window + 600);
		m_sTitle = "Time-Lock Safe";
		m_sBrief = string.Format("The bank safe in %1 still keeps time. Its lock opens at 03:00 and not a minute before. Be there in the dark, and be ready to leave fast: it's loud.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		float h = RZ_GameTime.GetHour();
		float window = WINDOW_HOURS;
		if (HasBonus(gm, playerId))
			window = WINDOW_HOURS * 2;

		if (h < 0 || !RZ_GameTime.IsWithin(h, OPEN_HOUR, window))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The little clock in the door reads %1. The handle doesn't move.", RZ_GameTime.Clock(h)));
			return;
		}

		gm.NotifyPlayer(playerId, m_sTitle, "Something inside clunks, three times. The door swings open with a shriek you hear in your teeth. Grab it and go.");
		SoundAlarm(gm, m_vSafe, 70, true);
		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vSafe);
	}
}

//------------------------------------------------------------------------------------------------
//! #11 The Bell Order (Haven): ring three village church bells in the order a
//! hymn sheet gives. Right order: the horde around the Haven safehouse (the one in the least
//! infected town; with no safehouses, simply the safest town) follows the sound away (heat and
//! infection drop there). Wrong bell: a horde comes for the
//! ringer and the order starts over. Up to 15 minutes between bells, so it works
//! solo; a group is faster. ★ Elder: more Fame.
class RZ_Mission_BellOrder : RZ_PuzzleMission
{
	protected int m_iHaven = -1;
	protected bool m_bSafehouse;		// m_iHaven has a real Haven safehouse
	protected ref array<int> m_aOrder = {};
	protected ref array<int> m_aVillages = {};
	protected ref array<vector> m_aBells = {};
	protected int m_iStep;
	protected float m_fSinceRing;

	protected static const float RING_WINDOW = 900;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 20, 20);
		m_Req.Bonus("elder");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.BELL) || gm.GetSettlements().Count() < 4)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		vector sheetAt;
		RZ_Safehouse house = RZ_ContractHelp.Safehouse(gm, RZ_SafehousePick.Safest(gm, true));
		if (house)
		{
			m_bSafehouse = true;
			m_iHaven = house.m_iSettlement;
		}
		else
		{
			m_iHaven = PickSafestSettlement(gm);
		}

		RZ_Settlement haven = gm.GetSettlement(m_iHaven);
		if (!haven || IsTownBusy(gm, m_iHaven))
			return false;

		if (house)
			sheetAt = LandPointNear(house.m_vPos, 3, 8);
		else
			sheetAt = LandPointNear(haven.m_vCenter, 0, haven.m_fRadius * 0.3);

		m_iSettlement = m_iHaven;
		m_vPos = haven.m_vCenter;
		array<int> used = {m_iHaven};
		array<int> candidates = {};
		CollectSettlements(gm, haven.m_vCenter, 500, 3000, used, candidates);
		if (candidates.Count() < 3)
			return false;

		array<string> churchKinds = {"church"};
		for (int i = 0; i < 3; i++)
		{
			RZ_Settlement v = gm.GetSettlement(candidates[i]);
			m_aVillages.Insert(candidates[i]);
			vector pos = LandPointNear(v.m_vCenter, 0, v.m_fRadius * 0.3);
			RZ_Landmark church = RZ_LandmarkFinder.FindNearest(v.m_vCenter, v.m_fRadius, churchKinds);
			if (church)
				pos = RZ_Util.RandomPointInRing(church.m_vPos, 4, 8);

			m_aBells.Insert(pos);
			SpawnObject(gm, RZ_EPuzzleProp.BELL, pos, i, "Ring the bell", "");
		}

		m_aOrder.Clear();
		m_aOrder.Insert(0);
		m_aOrder.Insert(1);
		m_aOrder.Insert(2);
		m_Rng.ShuffleInts(m_aOrder);

		int hymn = AddClue("Hymn sheet", string.Format("Hymn 114. Someone has pencilled new words over the old ones:\n\"Ring first in %1, so they look away,\nthen in %2, so they start walking,\nand last in %3, so they keep going.\nNever out of turn. They come to the wrong bell.\"\nUnderneath, smaller: \"Father Anselm's trick. It worked once.\"",
			VillageName(gm, m_aOrder[0]), VillageName(gm, m_aOrder[1]), VillageName(gm, m_aOrder[2])));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, sheetAt, hymn, "Read the hymn sheet");

		gm.GetHeat().AddHeat(haven.m_vCenter, 30);
		m_sTitle = "The Bell Order";
		if (m_bSafehouse)
			m_sBrief = string.Format("The dead are circling the Haven safehouse in %1. The old preacher had a trick: ring three village bells in the right order and they follow the sound away. The order is on a hymn sheet at the safehouse. Ring one out of turn and they come to you instead.", haven.m_sName);
		else
			m_sBrief = string.Format("The dead are circling %1, and there are still people holding out there. The old preacher had a trick: ring three village bells in the right order and they follow the sound away. The order is on a hymn sheet in the town. Ring one out of turn and they come to you instead.", haven.m_sName);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string VillageName(RZ_GameModeComponent gm, int i)
	{
		RZ_Settlement v = gm.GetSettlement(m_aVillages[i]);
		if (v)
			return v.m_sName;

		return RZ_SettlementScanner.GridReference(m_aBells[i]);
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return super.GetWhereText(gm, from) + string.Format("\nBells: %1, %2, %3", VillageName(gm, 0), VillageName(gm, 1), VillageName(gm, 2));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (m_iStep == 0)
			return;

		m_fSinceRing += dt;
		if (m_fSinceRing < RING_WINDOW)
			return;

		m_iStep = 0;
		NotifyParticipants(gm, m_sTitle, "The last bell's echo died away long ago. They've lost interest. Start again from the first bell.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx < 0 || objIdx > 2)
			return;

		if (objIdx != m_aOrder[m_iStep])
		{
			m_iStep = 0;
			gm.NotifyPlayer(playerId, m_sTitle, "Wrong bell. The note hangs in the air, and everything that heard it turns toward you.");
			SoundAlarm(gm, m_aBells[objIdx], 30, true);
			return;
		}

		m_iStep++;
		m_fSinceRing = 0;
		gm.GetHeat().AddHeat(m_aBells[objIdx], 20);
		if (m_iStep >= 3)
		{
			if (m_bSafehouse)
				NotifyParticipants(gm, m_sTitle, "The third bell rings out. Far off, the crowd around the safehouse starts to drift after it.");
			else
				NotifyParticipants(gm, m_sTitle, "The third bell rings out. Far off, the crowd around the town starts to drift after it.");

			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		NotifyParticipants(gm, m_sTitle, string.Format("The bell in %1 rings out over the roofs (%2 of 3). The next one, quickly.", VillageName(gm, objIdx), m_iStep));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Settlement haven = gm.GetSettlement(m_iHaven);
		if (!haven)
			return;

		gm.GetHeat().AddHeat(haven.m_vCenter, -200);
		haven.AddInfection(-8);
		if (m_bSafehouse)
			gm.NotifyAll("The Haven", string.Format("Bells were heard across the valley tonight. The safehouse in %1 says the dead have drifted away.", haven.m_sName));
		else
			gm.NotifyAll("The Haven", string.Format("Bells were heard across the valley tonight. People in %1 say the dead have drifted away.", haven.m_sName));
	}
}

//------------------------------------------------------------------------------------------------
//! #12 Lights in the Window (Bastion): at night a lamp in a distant building blinks a
//! code word in Morse. The signal card at the scout post has the alphabet and the
//! code words for four meeting places. Reach the right one: reaching a wrong one first fails it
//! (the observer won't wait for someone who can't read the light). Each meeting place is the
//! middle of the grid square the card gives (RZ_HiddenStash).
//! ★ Radio Operator: catches the word on the radio as well.
class RZ_Mission_LightsInTheWindow : RZ_PuzzleMission
{
	protected ref RZ_BlinkingLight m_Light;
	protected vector m_vLight;
	protected vector m_vMeeting;
	protected ref array<vector> m_aWrongPlaces = {};
	protected string m_sWord;
	protected ref array<int> m_aToldOps = {};

	protected static const float SEE_RANGE = 2500;
	protected static const float MEETING_GAP = 300;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.KNOWN, 15, 15);
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.LIGHT) || !HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !IsNightSoon(900))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement post = gm.GetSettlement(m_iSettlement);
		if (!post)
			return false;

		m_vPos = post.m_vCenter;
		array<int> used = {m_iSettlement};
		array<int> candidates = {};
		CollectSettlements(gm, post.m_vCenter, 600, 2000, used, candidates);
		if (candidates.IsEmpty())
			return false;

		RZ_Settlement farTown = gm.GetSettlement(candidates[0]);
		m_vLight = LandPointNear(farTown.m_vCenter, 0, farTown.m_fRadius * 0.5);
		array<string> tall = {"church", "tower", "watertower", "school"};
		RZ_Landmark building = RZ_LandmarkFinder.FindNearest(farTown.m_vCenter, farTown.m_fRadius, tall);
		if (building)
			m_vLight = building.m_vPos;

		m_vLight = m_vLight + Vector(0, 6, 0);	// an upper window

		// Four code words, four meeting places
		array<string> words = {"OWL", "FOX", "ELK", "CROW", "WOLF", "HARE", "MOLE", "WREN"};
		m_Rng.ShuffleStrings(words);
		int right = m_Rng.Int(0, 4);
		string book = "";
		array<vector> places = {};
		for (int i = 0; i < 4; i++)
		{
			// Grid square centres, on land, far enough apart that no two can be mistaken
			vector place;
			bool found = false;
			for (int tries = 0; tries < 10 && !found; tries++)
			{
				place = RZ_HiddenStash.GridSquareCentre(LandPointNear(post.m_vCenter, 500, 2500));
				found = !RZ_Util.IsWater(place) && !IsNearAnyPoint(place, places, MEETING_GAP);
			}

			if (!found)
				return false;

			places.Insert(place);
			book += string.Format("%1: %2\n", words[i], PlaceText(gm, place));
			if (i == right)
				m_vMeeting = place;
			else
				m_aWrongPlaces.Insert(place);
		}

		m_sWord = words[right];
		int card = AddClue("Signal card", "Bastion scout post. If the window lights up, it's one of ours. Read it, then go.\n\n" + RZ_Morse.AlphabetCard() + "\n\nCode words:\n" + book + "\nShort flash dot, long flash dash. A long dark gap means it starts again.");
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(post.m_vCenter, 0, post.m_fRadius * 0.3), card, "Read the signal card");

		m_Light = new RZ_BlinkingLight(gm.GetPuzzlePrefab(RZ_EPuzzleProp.LIGHT), m_vLight, m_sWord, 1.5);
		m_fTimeLeft = NightDuration(1800, 10800, 3600);

		m_sTitle = "Lights in the Window";
		m_sBrief = string.Format("A Bastion observer is signalling from a window in %1 after dark. The signal card is at the old scout post in %2. Read the light, find the meeting place, get there.", farTown.m_sName, post.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		bool night = RZ_GameTime.IsNight();
		bool watched = CountPlayersNear(players, m_vLight, SEE_RANGE) > 0;
		if (m_Light)
		{
			if (night && watched)
				m_Light.Start();
			else
				m_Light.Stop();
		}

		if (night)
		{
			foreach (int i, IEntity p : players)
			{
				int playerId = ids[i];
				if (m_aToldOps.Contains(playerId) || !IsInvolved(playerId) || !RZ_Radio.IsRadioOperator(gm, playerId))
					continue;

				if (RZ_Util.DistXZ(p.GetOrigin(), m_vLight) > SEE_RANGE)
					continue;

				m_aToldOps.Insert(playerId);
				gm.NotifyPlayer(playerId, "On your set", "Someone is keying the same letters on a weak carrier: " + RZ_Morse.Encode(m_sWord) + ".");
			}
		}

		// Guessing doesn't work: a player who has read the card and turns up at a wrong place first ends it
		foreach (int j, IEntity q : players)
		{
			if (!m_aParticipants.Contains(ids[j]))
				continue;

			foreach (vector wrong : m_aWrongPlaces)
			{
				if (RZ_Util.DistXZ(q.GetOrigin(), wrong) > RZ_HiddenStash.FIND_RADIUS)
					continue;

				gm.NotifyPlayer(ids[j], m_sTitle, "Nobody here, and nobody coming. Wrong place. Somewhere out there, a lamp goes dark for good.");
				m_eState = RZ_EMissionState.FAILED;
				return;
			}
		}

		if (ReachedBy(ids, players, m_vMeeting, RZ_HiddenStash.FIND_RADIUS))
		{
			m_vMeeting = RZ_HiddenStash.FoundAt(players, m_vMeeting);
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsNearAnyPoint(vector pos, array<vector> others, float dist)
	{
		foreach (vector o : others)
		{
			if (RZ_Util.DistXZ(pos, o) < dist)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vMeeting);
		NotifyParticipants(gm, m_sTitle, "Nobody's there. Just a pack under a groundsheet and a note: \"Thanks. Don't come looking for me.\"");
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		if (m_Light)
			m_Light.Stop();

		m_Light = null;
		super.Cleanup(gm);
	}
}

//------------------------------------------------------------------------------------------------
//! A real Haven safehouse for missions that name one (The Bell Order, The Crossing).
class RZ_SafehousePick
{
	//------------------------------------------------------------------------------------------------
	//! The safehouse in the least infected town, -1 if there are none. skipBusy: not in a town
	//! another mission holds.
	static int Safest(RZ_GameModeComponent gm, bool skipBusy)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return -1;

		int best = -1;
		float bestInfection = 1000;
		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse h = houses.Get(i);
			if (!h || h.m_iSettlement < 0)
				continue;

			if (skipBusy && gm.GetMissions().IsSettlementBusy(h.m_iSettlement))
				continue;

			RZ_Settlement s = gm.GetSettlement(h.m_iSettlement);
			if (!s || s.m_fInfection >= bestInfection)
				continue;

			best = i;
			bestInfection = s.m_fInfection;
		}

		return best;
	}
}
